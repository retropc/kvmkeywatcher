CFLAGS+=-Wall -Wno-unused
.PHONY: clean all

all: kvmkeywatcher kvmkeywatcher-arm

clean:
	rm -f kvmkeywatcher kvmkeywatcher-arm

kvmkeywatcher: kvmkeywatcher.c usb.c
	gcc $(CFLAGS) -pedantic -DDEBUG $(LDFLAGS) -o $@ $^ -lusb

kvmkeywatcher-arm: kvmkeywatcher.c usb.c
	$(CC) $(CFLAGS) $(LDFLAGS) -o $@ $^ -lusb
