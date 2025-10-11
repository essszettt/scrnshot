# Zielplattform
TARGET = zxn

# Name der Ausgabe-Datei
APPNAME = scrnshot

# Verzeichnisse
SRC_DIR = ../src
INC_DIR = ../inc
BUILD_DIR = .

# Quellcode-Dateien
SRCS  = $(SRC_DIR)/main.c
SRCS += $(SRC_DIR)/libzxn.c
SRCS += $(SRC_DIR)/libzxn.asm

# Compiler-Flags
CFLAGS  = -compiler=sdcc -clib=sdcc_iy -SO3 --opt-code-size -I$(INC_DIR) -pragma-include:$(INC_DIR)/zpragma.inc
CFLAGS += -startup=30
CFLAGS += --max-allocs-per-node200000
CFLAGS += -v           # verbose output
# CFLAGS += --list       # create list files
# CFLAGS += -D__DEBUG__  # create debug code

# Linker-Flags
LDFLAGS  = -subtype=dotn -Cz"--clean" -create-app -o $(BUILD_DIR)/$(APPNAME) 
LDFLAGS += -m  # create map file
# LDFLAGS += -s  # create symbol files

# Compiler-Aufruf
CC = zcc +$(TARGET) $(CFLAGS) $(SRCS) $(LDFLAGS)

# Standard-Build-Ziel
all:
	$(CC)
	@rm -f $(BUILD_DIR)/$(APPNAME)
	@mv -f $(BUILD_DIR)/SCRNSH $(BUILD_DIR)/$(APPNAME)

# Bereinigen der Build-Dateien
clean:
	@rm -f $(BUILD_DIR)/$(APPNAME)
	@rm -f $(BUILD_DIR)/$(APPNAME).lis
	@rm -f $(BUILD_DIR)/$(APPNAME).map
	@rm -f $(BUILD_DIR)/$(APPNAME).sym
	@rm -f $(SRC_DIR)/*.lis
	@rm -f $(SRC_DIR)/*.sym
