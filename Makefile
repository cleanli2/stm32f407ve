TARGET = stm32f407ve
GIT_SHA1="$(shell git log --format='_%h' -1)"
DIRTY="$(shell git diff --quiet || echo 'dirty')"
CLEAN="$(shell git diff --quiet && echo 'clean')"
BRANCH="$(shell git symbolic-ref --short HEAD)"

export CC             = arm-none-eabi-gcc           
export AS             = arm-none-eabi-as
export LD             = arm-none-eabi-ld
export OBJCOPY        = arm-none-eabi-objcopy
export OBJDUMP        = arm-none-eabi-objdump

TOP=$(shell pwd)

INC_FLAGS= \
		   -I $(TOP)/Core/Inc\
		   -I $(TOP)/Drivers/STM32F4xx_HAL_Driver/Inc\
		   -I $(TOP)/Drivers/STM32F4xx_HAL_Driver/Inc/Legacy\
		   -I $(TOP)/Drivers/CMSIS/Device/ST/STM32F4xx/Include\
		   -I $(TOP)/FATFS/Target\
		   -I $(TOP)/FATFS/App\
		   -I $(TOP)/Middlewares/Third_Party/FatFs/src\
		   -I $(TOP)/Drivers/CMSIS/Include

SFLAGS = -mcpu=cortex-m4 -g3 -DDEBUG -x assembler-with-cpp --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb
#CFLAGS =  -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx -fstack-usage -fcyclomatic-complexity --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb
CFLAGS1 = -mcpu=cortex-m4 -std=gnu11 -g3 -DDEBUG -DUSE_HAL_DRIVER -DSTM32F407xx
#CFLAGS2 = $(INC_FLAGS) -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage -fcyclomatic-complexity --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb
CFLAGS2 = $(INC_FLAGS) -O0 -ffunction-sections -fdata-sections -Wall -fstack-usage                          --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb
CFLAGS2 += -DGIT_SHA1=\"$(GIT_SHA1)$(DIRTY)$(CLEAN)\"
#LDFLAGS =  -mthumb -mcpu=cortex-m4 -Wl,--start-group -lc -lm -Wl,--end-group -specs=nano.specs -specs=nosys.specs -static -Wl,-cref,-u,Reset_Handler -Wl,-Map=Project.map -Wl,--gc-sections -Wl,--defsym=malloc_getpagesize_P=0x80
LDFLAGS = -mcpu=cortex-m4 --specs=nosys.specs -Wl,-Map=$(TARGET).map -Wl,--gc-sections -static --specs=nano.specs -mfpu=fpv4-sp-d16 -mfloat-abi=hard -mthumb -Wl,--start-group -lc -lm -Wl,--end-group

C_SRC=$(shell find ./ -name '*.c')
C_OBJ=$(C_SRC:%.c=%.o)
S_SRC=$(shell find ./ -name '*.s')
S_OBJ=$(S_SRC:%.s=%.o)

$(warning t=$(type))
ifeq ($(type),)
	type=notype
endif
ifeq ($(type),svr)
CFLAGS+=-DSVR
$(info type server)
else
$(info type client)
endif

ifeq ($(board),)
board=dftbd
endif
CFLAGS+=-DSTM32F10X_LD
LDFILE=STM32F407VETX_FLASH_legacy

.PHONY: all clean

all:$(C_OBJ) $(S_OBJ)
	$(CC) $(C_OBJ) $(S_OBJ) -T $(LDFILE).ld -o $(TARGET).elf $(LDFLAGS)
	$(OBJCOPY) $(TARGET).elf  $(TARGET).bin -Obinary 
	$(OBJCOPY) $(TARGET).elf  $(TARGET).hex -Oihex
	cp $(TARGET).hex $(TARGET)$(GIT_SHA1)_$(DIRTY)$(CLEAN).hex
	rm $(TARGET)_*.hex
	cp $(TARGET).hex $(TARGET)-$(board)-$(BRANCH)-$(type)-$(GIT_SHA1)-$(DIRTY)$(CLEAN).hex
	$(OBJDUMP) -d -S $(TARGET).elf > $(TARGET).asm

$(C_OBJ):%.o:%.c
	$(CC) $(CFLAGS1) -c $(CFLAGS2) -o $@ $<

$(S_OBJ):%.o:%.s
	$(CC) -c $(SFLAGS) -o $@ $<
clean:
	rm -f $(shell find ./ -name '*.o')
	rm -f $(shell find ./ -name '*.d')
	rm -f $(shell find ./ -name '*.map')
	rm -f $(shell find ./ -name '*.elf')
	rm -f $(shell find ./ -name '*.bin')
	rm -f $(shell find ./ -name '*.asm')
	rm -f $(shell find ./ -name '*.hex')
