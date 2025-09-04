/*
 * wm8994.c
 *
 *  Created on: Sep 2, 2025
 *      Author: Andrei Trif
 */

#include "wm8994.h"
#include "wm8994_regs.h"

#include "sai.h"

#define WM8994_ID 0x8994

static void i2c_write_u16(wm8994_t *w, uint16_t reg, uint16_t data);
static uint16_t i2c_read_u16(wm8994_t *w, uint16_t reg);
static uint16_t set_bit_reg(uint16_t reg_data, uint16_t reg_bit);
static uint16_t unset_bit_reg(uint16_t reg_data, uint16_t reg_bit);
static uint16_t set_bits_reg(uint16_t reg_data, uint16_t val, uint16_t start, uint16_t len);
static uint16_t get_bit_reg(uint16_t reg_data, uint16_t reg_bit);
static void wm8994_enable_path(wm8994_t *w, wm8994_output_t out);
static void wm8994_startup_headphones(wm8994_t *w);

/**
 * @brief Initializes the driver, returns non-zero integer upon configuration
 * error. On I2C errors, it calls the error_callback
 * @retval None
 */
int wm8994_init_driver(wm8994_t *w, I2C_HandleTypeDef *hi2c, uint16_t addr,
                       void (*error_callback)(void))
{
    w->hi2c = hi2c;
    w->addr = addr;
    w->error_callback = error_callback;

    // Check that the provided address on the provided I2C peripheral is our
    // codec
    if (i2c_read_u16(w, REG_ID_RESET) != WM8994_ID)
    {
        return -1;
    }

    // Reset all registers to default values
    i2c_write_u16(w, REG_ID_RESET, 1);
    HAL_Delay(100);

    uint16_t reg;

    // Before running the cold-start sequence, enable antipop
    reg = set_bit_reg(0, ANTIPOP_2_VMID_BUF_ENA);
    reg =
        set_bits_reg(reg, VMID_RAMP_SOFT_FAST_START, ANTIPOP_2_VMID_RAMP, ANTIPOP_2_VMID_RAMP_LEN);
    i2c_write_u16(w, REG_ANTIPOP_2, reg);

    // Enable vmid
    reg = set_bits_reg(0, PWR_1_VMID_SEL_2x40k, PWR_1_VMID_SEL, PWR_1_VMID_SEL_LEN);
    reg = set_bit_reg(reg, PWR_1_BIAS_ENA);
    i2c_write_u16(w, REG_PWR_1, reg);
    HAL_Delay(50);

    // AIF1 Clock enable
    reg = set_bit_reg(0, AIF1_CLOCK_1_ENA);
    i2c_write_u16(w, REG_AIF1_CLOCK_1, reg);

    // Set frequency to 16kHz and rate to 256, our clock is 4.096MHz = 16kHz *
    // 256
    reg = set_bits_reg(0, AIF1_RATE_256, AIF1_RATE_CLK_RATE, AIF1_RATE_CLK_RATE_LEN);
    reg = set_bits_reg(reg, AIF1_SR_16K, AIF1_RATE_SR, AIF1_RATE_SR_LEN);
    i2c_write_u16(w, REG_AIF1_RATE, reg);

    // Enable DSP clock, the signal has to pass through the DSP
    reg = set_bit_reg(0, CLOCK_1_SYSDSPCLK_ENA);
    reg = set_bit_reg(reg, CLOCK_1_AIF1DSPCLK_ENA);
    reg = set_bit_reg(reg, CLOCK_1_AIF2DSPCLK_ENA);
    reg = set_bit_reg(reg, CLOCK_1_TOCLK_ENA);
    i2c_write_u16(w, REG_CLOCK_1, reg);

    // Enable paths for HPOUT1 and LINE1L
    wm8994_enable_path(w, HEADPHONE_OUTPUT);
    wm8994_enable_path(w, HEADPHONE_MIC);

    // I2S protocol with a 16 bit data size. 16 bit * 4 channels = 64 bits
    reg = set_bits_reg(0, AIF1_CTRL_1_WL_16BIT, AIF1_CTRL_1_WL, AIF1_CTRL_1_WL_LEN);
    reg = set_bits_reg(reg, AIF1_CTRL_1_FMT_I2S, AIF1_CTRL_1_FMT, AIF1_CTRL_1_FMT_LEN);
    reg = set_bit_reg(reg, AIF1_CTRL_1_ADCR_SRC);
    i2c_write_u16(w, REG_AIF1_CTRL_1, reg);

    // cold start
    wm8994_startup_headphones(w);

    // Set outputs to 0dB
    i2c_write_u16(w, REG_R_OUT_VOL, 0x17C);
    i2c_write_u16(w, REG_L_OUT_VOL, 0x17C);

    // Set DAC1 to 0dB
    i2c_write_u16(w, REG_DAC1_L_VOL, 0x0C0);
    i2c_write_u16(w, REG_DAC1_R_VOL, 0x0C0);

    // ADC volume to 0dB
    i2c_write_u16(w, REG_AIF1_ADC1_L_VOL, 0x1C0);

    // Set GPIO1 to DRC1 signal detect
    // The default function of GPIO1 is AIF1LRCLK which won't let us receive
    // microphone data, due to the pin being unused. So we set the function to
    // anything other than 0.
    i2c_write_u16(w, REG_GPIO1, 1);

    return 0;
}

uint16_t wm8994_get_revision(wm8994_t *w)
{
    return i2c_read_u16(w, REG_CHIP_REVISION);
}

// Init sequence for headphones, after path is enabled
static void wm8994_startup_headphones(wm8994_t *w)
{
    uint16_t reg = 0;

    // initialize Control Write Sequence at addr 0
    reg = set_bit_reg(0, WR_SEQ_CTRL_1_WSEQ_ENA);
    reg = set_bit_reg(reg, WR_SEQ_CTRL_1_WSEQ_START);
    i2c_write_u16(w, REG_WR_SEQ_CTRL_1, reg);

    HAL_Delay(300);

    // Delay 1 ms until CWR is done
    while (get_bit_reg(i2c_read_u16(w, REG_WR_SEQ_CTRL_2), WR_SEQ_CTRL_2_WSEQ_BUSY))
    {
        HAL_Delay(1);
    }

    // Unmute DAC1
    i2c_write_u16(w, REG_AIF1_DAC1_FILTERS_1, 0);
}

static void wm8994_enable_path(wm8994_t *w, wm8994_output_t out)
{
    uint16_t reg;
    switch (out)
    {
    case HEADPHONE_OUTPUT:
    {
        // Enable DAC1 and AIF1DAC1
        reg = set_bit_reg(0, PWR_5_DAC1L_ENA);
        reg = set_bit_reg(reg, PWR_5_DAC1R_ENA);
        reg = set_bit_reg(reg, PWR_5_AIF1DAC1L_ENA);
        reg = set_bit_reg(reg, PWR_5_AIF1DAC1R_ENA);
        i2c_write_u16(w, REG_PWR_5, reg);

        // Direct DAC1 -> HPOUT1
        reg = set_bit_reg(0, OUT_MIX_1_DAC1L_TO_HPOUT1L);
        i2c_write_u16(w, REG_OUT_MIX_1, reg);
        reg = set_bit_reg(0, OUT_MIX_2_DAC1R_TO_HPOUT1R);
        i2c_write_u16(w, REG_OUT_MIX_2, reg);

        // AIF1 signal through Left/Right mixer and then DAC1
        reg = set_bit_reg(0, DAC1_LMR_DAC1L_TO_DAC1L);
        i2c_write_u16(w, REG_DAC1_LMR, reg);

        reg = set_bit_reg(0, DAC1_RMR_DAC1R_TO_DAC1R);
        i2c_write_u16(w, REG_DAC1_RMR, reg);

        break;
    }
    case HEADPHONE_MIC:
    {
        // Enable input PGA for IN1L single-ended mic, p -> VMID, n -> input
        reg = set_bit_reg(0, IN_MIX_2_IN1LN_TO_IN1L);
        i2c_write_u16(w, REG_IN_MIX_2, reg);

        // Enable ADC and AIF1ADC1
        reg = set_bit_reg(0, PWR_4_ADCL_ENA);
        reg = set_bit_reg(reg, PWR_4_AIF1ADC1L_ENA);
        i2c_write_u16(w, REG_PWR_4, reg);

        // Enable MIXINL_ENA, IN1L is the line from the jack
        reg = set_bit_reg(0, PWR_2_MIXINL_ENA);
        reg = set_bit_reg(reg, PWR_2_IN1L_ENA);
        i2c_write_u16(w, REG_PWR_2, reg | i2c_read_u16(w, REG_PWR_2));

        // Enable MICBIAS2
        reg = set_bit_reg(0, PWR_1_MICB2_ENA);
        i2c_write_u16(w, REG_PWR_1, i2c_read_u16(w, REG_PWR_1) | reg);

        // Set IN1L_TO_MIXINL and set volume to 0dB
        reg = set_bit_reg(0, IN_MIX_3_IN1L_TO_MIXINL);
        i2c_write_u16(w, REG_IN_MIX_3, reg);

        // IN1L PGA unmute and set it to 0dB
        i2c_write_u16(w, REG_L_IN12_VOL, 0xB);

        // ADC1 -> ADC1R_TO_AIF1
        reg = set_bit_reg(0, ADC1_LMR_ADC1L_TO_AIF1ADC1L);
        i2c_write_u16(w, REG_ADC1_LMR, reg);

        i2c_write_u16(w, 0x410, 0x7000);

        break;
    }
    default:
        break;
    }
}

static uint16_t i2c_read_u16(wm8994_t *w, uint16_t reg)
{
    uint16_t c;
    if (HAL_I2C_Mem_Read(w->hi2c, w->addr << 1, reg, I2C_MEMADD_SIZE_16BIT, (uint8_t *)&c, 2, 10) !=
        HAL_OK)
    {
        w->error_callback();
    }

    return __builtin_bswap16(c);
}

static void i2c_write_u16(wm8994_t *w, uint16_t reg, uint16_t data)
{
    data = __builtin_bswap16(data);
    if (HAL_I2C_Mem_Write(w->hi2c, w->addr << 1, reg, I2C_MEMADD_SIZE_16BIT, (uint8_t *)&data, 2,
                          0xFFFF) != HAL_OK)
    {
        w->error_callback();
    }
}

static uint16_t set_bit_reg(uint16_t reg_data, uint16_t reg_bit)
{
    return reg_data | (1 << reg_bit);
}

static uint16_t unset_bit_reg(uint16_t reg_data, uint16_t reg_bit)
{
    return reg_data & ~(1 << reg_bit);
}

static uint16_t set_bits_reg(uint16_t reg_data, uint16_t val, uint16_t start, uint16_t len)
{
    uint16_t len_mask = ((1 << len) - 1);
    return (reg_data & ~(len_mask << start)) | ((val & len_mask) << start);
}

static uint16_t get_bit_reg(uint16_t reg_data, uint16_t reg_bit)
{
    return reg_data & (1 << reg_bit);
}
