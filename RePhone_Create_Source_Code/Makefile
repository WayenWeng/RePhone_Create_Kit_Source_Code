
# Set your toolchain path
TOOLCHAIN=

# Set your LinkIt sdk path
LINKIT_ASSIST_SDK_PATH=

CC=$(TOOLCHAIN)/bin/arm-none-eabi-gcc
AS=$(TOOLCHAIN)/bin/arm-none-eabi-as
LD=$(TOOLCHAIN)/bin/arm-none-eabi-gcc
OBJCOPY=$(TOOLCHAIN)/bin/arm-none-eabi-objcopy
OBJDUMP=$(TOOLCHAIN)/bin/arm-none-eabi-objdump
SIZE=$(TOOLCHAIN)/bin/arm-none-eabi-size


OBJECTS = action.o actuator.o address_book.o battery.o call_ui.o cjson.o condition.o contact_ui.o font_icon24.o font_icon48.o \
	font_ugui.o gui_glue.o ifttt_book.o ifttt.o ifttt_ui.o input_window.o laudio.o lcd_sitronix_st7789s.o ldlceint.o ldlcgpio.o \
	ldlci2cV2.o led_matrix_window.o lgsmsms.o lledmatrix.o lmotor.o lrgbws2812.o lstorage.o lvoicecall.o main.o music.o \
	sensor.o settings.o sms_book.o sms_ui.o tp_goodix_gt9xx.o tp_i2c.o ugui.o util.o vibrator_window.o windows.o ws2812_window.o

PROJECT = main

WORKSPACE_PATH = ./

SYS_OBJECTS += $(LINKIT_ASSIST_SDK_PATH)/LINKIT_ASSIST_SDK/lib/LINKIT10/src/gccmain.o
INCLUDE_PATHS += -I. -I$(LINKIT_ASSIST_SDK_PATH)/LINKIT_ASSIST_SDK/include -I$(WORKSPACE_PATH)
LIBRARY_PATHS += -L$(LINKIT_ASSIST_SDK_PATH)/LINKIT_ASSIST_SDK/lib/LINKIT10/armgcc
LIBRARIES += $(LINKIT_ASSIST_SDK_PATH)/LINKIT_ASSIST_SDK/lib/LINKIT10/armgcc/percommon.a -lm
LINKER_SCRIPT = $(LINKIT_ASSIST_SDK_PATH)/LINKIT_ASSIST_SDK/lib/LINKIT10/armgcc/scat.ld

###############################################################################


CPU = -mcpu=arm7tdmi-s -mthumb -mlittle-endian
CC_FLAGS = $(CPU) -mthumb-interwork -c -fvisibility=hidden -gdwarf-2 -gstrict-dwarf -fpic -O2 -fno-exceptions -fno-non-call-exceptions
CC_SYMBOLS += -D__HDK_LINKIT_ASSIST_2502__ -D__COMPILER_GCC__

LD_FLAGS = $(CPU) -O2 -Wl,--gc-sections --specs=nosys.specs -fpic -pie -Wl,-Map=$(PROJECT).map  -Wl,--entry=gcc_entry -Wl,--unresolved-symbols=report-all -Wl,--warn-common -Wl,--warn-unresolved-symbols
LD_SYS_LIBS =

# the pack tool https://github.com/mandl/LinkIt_Assist_2502
PACK= ./merge_mtk.py

# the uploader tool https://github.com/mandl/LinkIt_Assist_2502
PUSH= ./uploader.py

all: $(PROJECT).vxp size

clean:
	rm -f $(PROJECT).vxp $(PROJECT).bin $(PROJECT).elf $(PROJECT).hex $(PROJECT).map $(PROJECT).lst $(OBJECTS)

.s.o:
	$(AS) $(CPU) -o $@ $<

.c.o:
	$(CC)  $(CC_FLAGS) $(CC_SYMBOLS) -std=gnu99   $(INCLUDE_PATHS) -o $@ $<

$(PROJECT).elf: $(OBJECTS) $(SYS_OBJECTS)
	$(LD) $(LD_FLAGS) -T$(LINKER_SCRIPT) $(LIBRARY_PATHS) -o $@ -Wl,--start-group $^ $(LIBRARIES) $(LD_SYS_LIBS) -Wl,--end-group

$(PROJECT).bin: $(PROJECT).elf
	@$(OBJCOPY) -O binary $< $@

$(PROJECT).hex: $(PROJECT).elf
	@$(OBJCOPY) -O ihex $< $@

$(PROJECT).vxp: $(PROJECT).elf
	@$(OBJCOPY) --strip-debug $<
	@$(PACK) $< $@

$(PROJECT).lst: $(PROJECT).elf
	@$(OBJDUMP) -Sdh $< > $@

lst: $(PROJECT).lst

size: $(PROJECT).elf
	$(SIZE) $(PROJECT).elf

flash: $(PROJECT).vxp
	$(PUSH)
