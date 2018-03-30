CFLAGS+=-Wall -Wno-unused
.PHONY: clean all

all: kvmkeywatcher S99kvmkeywatcher-arm

clean:
	rm -f kvmkeywatcher S99kvmkeywatcher-arm

kvmkeywatcher: kvmkeywatcher.c usb.c
	gcc $(CFLAGS) -pedantic -DDEBUG $(LDFLAGS) -o $@ $^ -lusb

S99kvmkeywatcher-arm: kvmkeywatcher.c usb.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lusb
