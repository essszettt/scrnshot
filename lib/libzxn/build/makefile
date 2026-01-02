.PHONY: all clean

### Target Platform ####################
TARGET := zxn

### Project Name #######################
LIBNAME := libzxn

### OS specific settings ###############
CC := zcc
AR := z88dk-z80asm

ifeq ($(OS),Windows_NT)
RM := rm -f
MV := mv -f
else
RM := rm -f
MV := mv -f
endif

### Build Type #########################
BUILD ?= release

### Directories ########################
SRC_DIR = ../src
INC_DIR = ../inc
BLD_DIR = .

LIBFILE := $(BLD_DIR)/$(LIBNAME).lib

### Source Files #######################
SRCS_C := $(wildcard $(SRC_DIR)/*.c)
SRCS_A := $(wildcard $(SRC_DIR)/*.asm)
SRCS   := $(SRCS_C)
SRCS   += $(SRCS_A)

### Object Files #######################
OBJS := $(patsubst $(SRC_DIR)/%.c,$(BLD_DIR)/%.o,$(SRCS_C))
OBJS += $(patsubst $(SRC_DIR)/%.asm,$(BLD_DIR)/%.o,$(SRCS_A))

### Compiler Flags #####################
CFLAGS := -compiler=sdcc -clib=sdcc_iy --vc -SO3 --opt-code-size -I$(INC_DIR)

# verbose output
# CFLAGS += -v

ifeq ($(BUILD), debug)
# create list files
CFLAGS += --list

# add C code to list files
CFLAGS += --c-code-in-asm

# create debug code
CFLAGS += -D__DEBUG__
else
# tradeoff speed vs. code quality
CFLAGS += --max-allocs-per-node200000
endif

### Archiver Flags #####################
ARFLAGS := -m=z80n
#ARFLAGS += -v

### Build Targets ######################
all: $(LIBFILE)

$(LIBFILE): $(OBJS)
	$(AR) $(ARFLAGS) -x$(LIBFILE) $(OBJS)

$(BLD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) +$(TARGET) $(CFLAGS) -c $< -o $@

$(BLD_DIR)/%.o: $(SRC_DIR)/%.asm
	$(CC) +$(TARGET) $(CFLAGS) -c $< -o $@

### Cleanup Build Files ################
clean:
	@$(RM) $(LIBFILE)
	@$(RM) $(wildcard $(BLD_DIR)/*.o)
	@$(RM) $(wildcard $(BLD_DIR)/*.lis)
	@$(RM) $(wildcard $(BLD_DIR)/*.map)
	@$(RM) $(wildcard $(BLD_DIR)/*.sym)
	@$(RM) $(wildcard $(SRC_DIR)/*.lis)
	@$(RM) $(wildcard $(SRC_DIR)/*.sym)
