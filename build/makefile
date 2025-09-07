# Zielplattform
TARGET = zxn

# Name der Ausgabe-Datei
APPNAME = scrnshot

# Verzeichnisse
SRC_DIR = ../src
INC_DIR = ../inc
BUILD_DIR = .

# Quellcode-Dateien
SRCS = $(SRC_DIR)/main.c
# SRCS += $(SRC_DIR)/zxn_uart.c
# SRCS += $(SRC_DIR)/zxn_utils.c

# Compiler-Flags
CFLAGS  = -compiler=sdcc -clib=sdcc_iy -SO3 --max-allocs-per-node200000 --opt-code-size -I$(INC_DIR) -pragma-include:$(INC_DIR)/zpragma.inc
CFLAGS += -startup=30
# CFLAGS += -v           # verbose output
# CFLAGS += --list       # create list files
# CFLAGS += -D__DEBUG__  # create debug code

# Linker-Flags
LDFLAGS  = -subtype=dot -Cz"--clean" -create-app -o $(BUILD_DIR)/$(APPNAME) 
# LDFLAGS += -m  # create map file
# LDFLAGS += -s  # create symbol files

# Compiler-Befehl
CC = zcc +$(TARGET) $(CFLAGS) $(SRCS) $(LDFLAGS)

# Standard-Build-Ziel
all:
	$(CC)

# Bereinigen der Build-Dateien
# rm -f $(BUILD_DIR)/$(APPNAME) *.o *.bin *.sym
# rm -f $(BUILD_DIR)/$(APPNAME).map
clean:
	rm -f $(BUILD_DIR)/$(APPNAME)
	rm -f $(BUILD_DIR)/$(APPNAME).lis
	rm -f $(BUILD_DIR)/$(APPNAME).map
	rm -f $(BUILD_DIR)/$(APPNAME).sym
	rm -f $(SRC_DIR)/*.lis
	rm -f $(SRC_DIR)/*.sym
