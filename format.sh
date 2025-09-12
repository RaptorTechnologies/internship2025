for file in Core/Src/* Core/Inc/*; do
	case "${file}" in
		*stm32f4xx*)
			echo "Skip ${file}"
			;;
		*sysmem.c)
			echo "Skip ${file}"
			;;
		*syscalls.c)
			echo "Skip ${file}"
			;;
		*system_stm32f4xx.c)
			echo "Skip ${file}"
			;;
		*)
			echo "Format ${file}"
			clang-format --style=file -i "${file}"
			;;
	esac
done

