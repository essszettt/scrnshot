.PHONY: all clean push

### Target Platform ####################
TARGET := zxn

### Project Name #######################
APPNAME := scrnshot

### Binary type ########################
APPTYPE := dotn

### C-Runtime-Type #####################
CRTTYPE := 30

### Tool Commands ######################
CC := zcc

ifeq ($(OS),Windows_NT)
RM := rm -f
MV := mv -f
else
RM := rm -f
MV := mv -f
endif

HDF := hdfmonkey

### Build Type #########################
BUILD ?= release

### Source Directories #################
SRC_DIR := ../src
INC_DIR := ../inc
BLD_DIR := .
LIB_DIR := $(if $(wildcard ../../../lib),../../../lib,../lib)

### Source Files #######################
SRCS := $(SRC_DIR)/main.c

### Compiler Flags #####################
CFLAGS := -compiler=sdcc --vc -clib=sdcc_iy -SO3 --opt-code-size -pragma-include:$(INC_DIR)/zpragma.inc
CFLAGS += -I$(INC_DIR)
CFLAGS += -I$(LIB_DIR)/libzxn/inc

# verbose output
# CFLAGS += -v

ifeq ($(BUILD), debug)
# create list files
CFLAGS += --list

# create debug code
CFLAGS += -D__DEBUG__
else
# tradeoff speed vs. code quality
CFLAGS += --max-allocs-per-node200000
endif

### Linker Flags #######################
LDFLAGS := -subtype=$(APPTYPE) -startup=$(CRTTYPE) -Cz"--clean" -create-app -o $(BLD_DIR)/$(APPNAME) 
LDFLAGS += -L$(LIB_DIR)/libzxn/build -llibzxn

ifeq ($(BUILD), debug)
# create map file
LDFLAGS += -m

# create symbol files
LDFLAGS += -s
endif

### Build Target #######################
all: libzxn
	$(CC) +$(TARGET) $(CFLAGS) $(SRCS) $(LDFLAGS)
ifeq ($(APPTYPE), dotn)
ifeq ($(OS),Windows_NT)
	@$(RM) -f $(BLD_DIR)/$(APPNAME)
	@$(MV) -f $(BLD_DIR)/SCRNSH $(BLD_DIR)/$(APPNAME)
endif
endif

libzxn:
	$(MAKE) -C $(LIB_DIR)/libzxn/build BUILD=$(BUILD)

### Push to emulator image #############
push:
	$(HDF) put "$(ZXN_IMAGE_PATH)/$(ZXN_IMAGE_NAME)" $(APPNAME) "/dot"

### Cleanup Build Files ################
clean:
	@$(RM) -f $(BLD_DIR)/$(APPNAME)
	@$(RM) $(wildcard $(BLD_DIR)/*.lis)
	@$(RM) $(wildcard $(BLD_DIR)/*.map)
	@$(RM) $(wildcard $(BLD_DIR)/*.sym)
	@$(RM) $(wildcard $(SRC_DIR)/*.lis)
	@$(RM) $(wildcard $(SRC_DIR)/*.sym)
	@$(MAKE) -C $(LIB_DIR)/libzxn/build clean
