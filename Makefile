.PHONY: clean all

all: kvmkeywatcher kvmkeywatcher-arm

clean:
	rm -f kvmkeywatcher kvmkeywatcher-arm

kvmkeywatcher: kvmkeywatcher.c usb.c
	gcc -Wall -pedantic -DDEBUG -o $@ $^ -lusb

kvmkeywatcher-arm: kvmkeywatcher.c usb.c
	$(CC) $(CFLAGS) $(LDFLAGS) -Wall -g -o $@ $^ -lusb

