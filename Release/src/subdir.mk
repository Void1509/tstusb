################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../src/descriptor.c \
../src/disp.c \
../src/hw_init.c \
../src/main.c \
../src/usb.c \
../src/usbcore.c 

OBJS += \
./src/descriptor.o \
./src/disp.o \
./src/hw_init.o \
./src/main.o \
./src/usb.o \
./src/usbcore.o 

C_DEPS += \
./src/descriptor.d \
./src/disp.d \
./src/hw_init.d \
./src/main.d \
./src/usb.d \
./src/usbcore.d 


# Each subdirectory must supply rules for building sources it contributes
src/%.o: ../src/%.c
	@echo 'Building file: $<'
	@echo 'Invoking: Cross ARM C Compiler'
	arm-none-eabi-gcc -mcpu=cortex-m3 -mthumb -Os -fmessage-length=0 -fsigned-char -ffunction-sections -fdata-sections -ffreestanding -Wall -Wextra  -g -DSTM32F10X_MD -DUSE_STDPERIPH_DRIVER -DHSE_VALUE=8000000 -I"../include" -I"../system/include" -I"../system/include/cmsis" -I"../system/include/stm32f1-stdperiph" -I/Users/valeriy/Documents/workspace/machine2/mylibs/inc -std=gnu11 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@)" -c -o "$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


