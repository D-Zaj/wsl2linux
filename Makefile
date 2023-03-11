CC=gcc
TARGET=wsl2win
SRC=wsl2win.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) -o $@ $<
