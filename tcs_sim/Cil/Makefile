CC ?= cc
CFLAGS ?= -Wall -Wextra -Wpedantic -std=c11 -O2

TARGET = cil_bridge
VENDOR = vendor
CPPFLAGS += -D_GNU_SOURCE -I$(VENDOR)/cil -I$(VENDOR)/cfu \
	-I$(VENDOR)/tim -I$(VENDOR)/sys

OBJS = cil_bridge.o Cil.o CilUtil.o Cfu.o Tim.o

.PHONY: all clean

all: $(TARGET)

$(TARGET): $(OBJS)
	$(CC) -o $@ $^ -lm -lrt

cil_bridge.o: cil_bridge.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

Cil.o: $(VENDOR)/cil/Cil.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

CilUtil.o: $(VENDOR)/cil/CilUtil.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

Cfu.o: $(VENDOR)/cfu/Cfu.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

Tim.o: $(VENDOR)/tim/Tim.c
	$(CC) $(CPPFLAGS) $(CFLAGS) -c -o $@ $<

clean:
	rm -f $(TARGET) $(OBJS)
