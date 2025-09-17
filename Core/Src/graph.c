/*
 * graph.c
 *
 *  Created on: Sep 12, 2025
 *      Author: Andrei Trif
 */

#include "graph.h"
#include "../../../Drivers/BSP/STM324x9I_EVAL/stm324x9i_eval_lcd.h"
#include "../../../Utils/Fonts/fonts.h"
#include <string.h>

#define TEXT_8BIT_LEN(s) (strlen(s) * 5)
#define TEXT_8BIT_HEIGHT 8
#define TEXT_8BIT_WIDTH 4

#define FIXED_POINT_SHIFT 16

static uint32_t x_size = 0;
static uint32_t y_size = 0;
static uint32_t x_gap = 10;
static uint32_t y_gap = 20;
static uint32_t pixels_per_sample = 0;
static uint32_t y_pixels_per_sample = 0;

static char x_title[40];
static char y_title[40];
static char graph_title[40] = { 0 };

static uint32_t w;
static uint32_t h;

static graph_display_mode_t mode = MODE_BAR;

static uint32_t bg_color = LCD_COLOR_DARKGRAY;
static uint32_t line_color[6] = {
    LCD_COLOR_GREEN, LCD_COLOR_ORANGE, LCD_COLOR_RED, LCD_COLOR_RED, LCD_COLOR_RED, LCD_COLOR_RED,
};
static int intensity_div = 0;

void graph_init(uint32_t x_samples, uint32_t y_samples, char *x_axis_title, char *y_axis_title,
                char *title)
{
    x_size = x_samples;
    y_size = y_samples;

    w = BSP_LCD_GetXSize();
    h = BSP_LCD_GetYSize();

    pixels_per_sample = (w - TEXT_8BIT_WIDTH * 2) / x_samples;
    x_gap = (w - pixels_per_sample * x_samples) / 2;

    y_pixels_per_sample = ((h - y_gap * 2) << FIXED_POINT_SHIFT) / y_samples;

    intensity_div = y_samples / (sizeof(line_color) / sizeof(*line_color));

    strcpy(x_title, x_axis_title);
    strcpy(y_title, y_axis_title);
    strcpy(graph_title, title);
}

void draw_slope1(int x, int y, int dx, int dy, int len)
{
    BSP_LCD_DrawLine(x, y, x + len * dx, y + len * dy);
}

void graph_draw_axis(void)
{
    BSP_LCD_Clear(bg_color);

    BSP_LCD_SetTextColor(line_color[0]);
    BSP_LCD_SetBackColor(bg_color);
    BSP_LCD_SetFont(&Font8);

    // Vertical arrow
    BSP_LCD_DrawVLine(x_gap - 1, y_gap / 2, h - y_gap);
    draw_slope1(x_gap - 1, y_gap / 2, -1, 1, 5);
    draw_slope1(x_gap - 1, y_gap / 2, 1, 1, 5);

    // Horizontal arrow
    BSP_LCD_DrawHLine(x_gap / 2, h - y_gap, w - x_gap);
    draw_slope1(w - x_gap / 2, h - y_gap, -1, -1, 5);
    draw_slope1(w - x_gap / 2, h - y_gap, -1, 1, 5);

    // Vertical Text
    int height = (TEXT_8BIT_HEIGHT + 1) * strlen(y_title);
    for (size_t i = 0; i < strlen(y_title); ++i)
    {
        BSP_LCD_DisplayChar(x_gap / 2, h / 2 - height / 2 + i * (TEXT_8BIT_HEIGHT + 1), y_title[i]);
    }

    // Horizontal text
    BSP_LCD_DisplayStringAt(w / 2 - TEXT_8BIT_LEN(x_title) / 2, h - y_gap / 2, (uint8_t *)x_title,
                            LEFT_MODE);

    // Graph title
    BSP_LCD_DisplayStringAt(w - TEXT_8BIT_LEN(graph_title), 0, (uint8_t *)graph_title, LEFT_MODE);
}

void graph_change_mode(graph_display_mode_t display_mode)
{
    BSP_LCD_SetTextColor(bg_color);
    BSP_LCD_FillRect(x_gap, y_gap, w - 2 * x_gap, h - 2 * y_gap);
    mode = display_mode;
}

void graph_update_x_value(uint32_t x, uint32_t y)
{
    uint32_t screen_x = x_gap + x * pixels_per_sample;
    uint32_t screen_y = h - ((y * y_pixels_per_sample) >> FIXED_POINT_SHIFT) - y_gap;

    BSP_LCD_SetTextColor(bg_color);
    switch (mode)
    {
        case MODE_BAR:
        {
            BSP_LCD_FillRect(screen_x, y_gap, pixels_per_sample, h - 2 * y_gap);

            BSP_LCD_SetTextColor(line_color[y / intensity_div]);
            BSP_LCD_FillRect(screen_x, screen_y, pixels_per_sample, h - screen_y - y_gap);
            break;
        }
        case MODE_POINT:
        {
            BSP_LCD_FillRect(screen_x + pixels_per_sample / 2 - 1, y_gap, 2, h - 2 * y_gap);

            BSP_LCD_DrawPixel(screen_x + pixels_per_sample / 2, screen_y, line_color[0]);
            break;
        }
        default:
            break;
        }
    }
}
