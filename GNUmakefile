LIBS := wayland-client wayland-protocols

CFLAGS ?= -Wall
CFLAGS += $(shell pkg-config --libs $(LIBS)) -I$(CURDIR)/proto\
	  -I$(CURDIR)/include

PROTOS := $(basename $(wildcard proto/*.xml))
PROTO_DEPS := $(foreach \
	      PROTO,$(PROTOS),$(PROTO)-protocol.c $(PROTO)-client-protocol.h)

%.o: %.c
	$(CC) $(CFLAGS) -c -o $@ $<

wlr-mux: $(PROTO_DEPS) event.o wayland.o main.o
	$(CC) $(CFLAGS) -o $@ $^

proto/%-client-protocol.h: proto/%.xml
	wayland-scanner client-header $< $@

proto/%-protocol.c: proto/%.xml
	wayland-scanner private-code $< $@
