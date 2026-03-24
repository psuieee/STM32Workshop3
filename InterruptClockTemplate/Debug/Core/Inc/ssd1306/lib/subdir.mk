################################################################################
# Automatically-generated file. Do not edit!
# Toolchain: GNU Tools for STM32 (13.3.rel1)
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../Core/Inc/ssd1306/lib/ssd1306.c 

OBJS += \
./Core/Inc/ssd1306/lib/ssd1306.o 

C_DEPS += \
./Core/Inc/ssd1306/lib/ssd1306.d 


# Each subdirectory must supply rules for building sources it contributes
Core/Inc/ssd1306/lib/%.o Core/Inc/ssd1306/lib/%.su Core/Inc/ssd1306/lib/%.cyclo: ../Core/Inc/ssd1306/lib/%.c Core/Inc/ssd1306/lib/subdir.mk
	arm-none-eabi-gcc "$<" -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F411xE -c -I../Core/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc -I../Drivers/STM32F4xx_HAL_Driver/Inc/Legacy -I../Drivers/CMSIS/Device/ST/STM32F4xx/Include -I../Drivers/CMSIS/Include -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity -MMD -MP -MF"$(@:%.o=%.d)" -MT"$@" --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -o "$@"

clean: clean-Core-2f-Inc-2f-ssd1306-2f-lib

clean-Core-2f-Inc-2f-ssd1306-2f-lib:
	-$(RM) ./Core/Inc/ssd1306/lib/ssd1306.cyclo ./Core/Inc/ssd1306/lib/ssd1306.d ./Core/Inc/ssd1306/lib/ssd1306.o ./Core/Inc/ssd1306/lib/ssd1306.su

.PHONY: clean-Core-2f-Inc-2f-ssd1306-2f-lib

