LIBS := wayland-client wayland-protocols

CFLAGS ?= -Wall
CFLAGS += $(shell pkg-config --libs $(LIBS)) -I$(CURDIR)/proto

PROTOS := $(basename $(wildcard proto/*.xml))
PROTO_DEPS := $(foreach \
	      PROTO,$(PROTOS),$(PROTO)-protocol.c $(PROTO)-client-protocol.h)

wlr-mux: main.c $(PROTO_DEPS)
	$(CC) $(CFLAGS) -o $@ $^

proto/%-client-protocol.h: proto/%.xml
	wayland-scanner client-header $< $@

proto/%-protocol.c: proto/%.xml
	wayland-scanner private-code $< $@
