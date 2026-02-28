.PHONY: all clean push

### Target Platform ####################
TARGET := zxn

### Project Name #######################
APPNAME := scrnshot

### Binary type ########################
APPTYPE := dotn

### Tool Commands ######################
CC := zcc
LD := zcc

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

### Directories ########################
SRC_DIR := ../src
INC_DIR := ../inc
BLD_DIR := .
LIB_DIR := $(if $(wildcard ../../../lib),../../../lib,../lib)

### Source Files #######################
SRCS := $(wildcard $(SRC_DIR)/*.c)
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BLD_DIR)/%.o,$(SRCS))

### Compiler Options ###################
CFLAGS := -compiler=sdcc --vc -SO3 --opt-code-size
CFLAGS += -I$(INC_DIR) 
CFLAGS += -I$(LIB_DIR)/libzxn/inc

# verbose output
# CFLAGS += -v

ifeq ($(BUILD), debug)
# create list files
CFLAGS += --list

# add C code to list files
CFLAGS += --c-code-in-asm

# create debug code
CFLAGS += -D__DEBUG__
else ifeq ($(BUILD), release)
# speed vs. code quality
# CFLAGS += --max-allocs-per-node10000
endif

### Linker Options #####################
LDFLAGS := -subtype=$(APPTYPE) -clib=sdcc_iy -Cz"--clean" -o$(BLD_DIR)/$(APPNAME) -create-app
LDFLAGS += -pragma-include:$(INC_DIR)/zpragma.inc
LDFLAGS += -L$(LIB_DIR)/libzxn/build -llibzxn

# Selecting CRT
LDFLAGS += -startup=30

ifeq ($(BUILD), debug)
# create map file
LDFLAGS += -m

# create symbol files
LDFLAGS += -s
endif

### Create build target ################
all: libzxn $(OBJS)
	$(LD) +$(TARGET) $(LDFLAGS) $(OBJS)
  # Why fucking hell does ZCC not use the full name ?? 
ifeq ($(APPTYPE), dotn)
ifeq ($(OS),Windows_NT)
	@$(RM) $(BLD_DIR)/$(APPNAME)
	@$(MV) $(BLD_DIR)/SCRNSH $(BLD_DIR)/$(APPNAME)
endif
endif

libzxn:
	$(MAKE) -C $(LIB_DIR)/libzxn/build BUILD=$(BUILD)

$(BLD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) +$(TARGET) $(CFLAGS) -c $< -o $@

### Push to emulator image #############
push:
	$(HDF) put "$(ZXN_IMAGE_PATH)/$(ZXN_IMAGE_NAME)" $(APPNAME) "/dot"

### Cleanup build files ################
clean:
	@$(RM) $(BLD_DIR)/$(APPNAME)
	@$(RM) $(wildcard $(BLD_DIR)/*.o)
	@$(RM) $(wildcard $(BLD_DIR)/*.lis)
	@$(RM) $(wildcard $(BLD_DIR)/*.map)
	@$(RM) $(wildcard $(BLD_DIR)/*.sym)
	@$(RM) $(wildcard $(SRC_DIR)/*.lis)
	@$(RM) $(wildcard $(SRC_DIR)/*.sym)
	@$(MAKE) -C $(LIB_DIR)/libzxn/build clean
