################################################################################
# Automatically-generated file. Do not edit!
################################################################################

# Add inputs and outputs from these tool invocations to the build variables 
C_SRCS += \
../MiSistemaDeFicheros.c \
../common.c \
../parse.c \
../util.c 

OBJS += \
./MiSistemaDeFicheros.o \
./common.o \
./parse.o \
./util.o 

C_DEPS += \
./MiSistemaDeFicheros.d \
./common.d \
./parse.d \
./util.d 


# Each subdirectory must supply rules for building sources it contributes
%.o: ../%.c
	@echo 'Building file: $<'
	@echo 'Invoking: GCC C Compiler'
	gcc -O0 -g3 -Wall -c -fmessage-length=0 -MMD -MP -MF"$(@:%.o=%.d)" -MT"$(@:%.o=%.d)" -o"$@" "$<"
	@echo 'Finished building: $<'
	@echo ' '


