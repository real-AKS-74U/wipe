TARGET = build/wipe

SRC_DIR = src
INC_DIR = include
BUILD_DIR = build
LIB_DIR = $(BUILD_DIR)/libs

SRCS = $(wildcard $(SRC_DIR)/*.c)
OBJS = $(SRCS:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

CC = gcc
GTK_VERSION = gtk4
CFLAGS = -I$(INC_DIR) `pkg-config --cflags $(GTK_VERSION) libudev`
LDFLAGS = `pkg-config --libs $(GTK_VERSION) libudev`

all: $(TARGET) copy-libs

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ $(LDFLAGS)

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	@mkdir -p $(BUILD_DIR)
	$(CC) -c $< -o $@ $(CFLAGS)

copy-libs: $(TARGET)
	@mkdir -p $(LIB_DIR)
	@ldd $(TARGET) | awk '{print $$3}' | grep '^/' | xargs -r -I{} cp -u {} $(LIB_DIR)/
	@echo "[+] Copied runtime libraries into $(LIB_DIR)"

clean:
	rm -rf $(BUILD_DIR)/*

.PHONY: all clean copy-libs