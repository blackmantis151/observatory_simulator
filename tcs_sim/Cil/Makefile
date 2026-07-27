CC ?= gcc
CFLAGS ?= -Wall -Wextra -O2

TARGET = cil_sender
SRC = cil_sender.c

all: $(TARGET)

$(TARGET): $(SRC)
	$(CC) $(CFLAGS) -o $(TARGET) $(SRC)

clean:
	rm -f $(TARGET)
