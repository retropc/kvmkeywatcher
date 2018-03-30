#include <stdio.h>
#include <usb.h>

static struct usb_device *find_device(int vendor, int product) {
  struct usb_bus *bus;
  struct usb_device *device;

  usb_find_busses();
  usb_find_devices();

  for(bus=usb_busses;bus;bus=bus->next)
    for(device=bus->devices;device;device=device->next)
      if ((device->descriptor.idVendor == vendor) && (device->descriptor.idProduct == product))
        return device;

  return NULL;
}

int switch_port(int vendor, int product, int port) {
  static int inited;
  int ret = 1;

  if(!inited) {
    usb_init();
    inited = 1;
  }

  struct usb_device *device = find_device(vendor, product);
  if(!device) {
    fprintf(stderr, "device not found\n");
    goto exit;
  }

  struct usb_dev_handle *handle = usb_open(device);
  if(!handle) {
    fprintf(stderr, "error opening\n");
    goto exit;
  }

  int r = usb_claim_interface(handle, 1);
  if(r < 0) {
    fprintf(stderr, "unable to claim device\n");
    goto exit;
  }

  r = usb_set_configuration(handle, 0);
/*  if(r < 0) {
    fprintf(stderr, "unable to set configuration\n");
    goto exit;
  }
*/

  char data[] = { 1, port - 1, 0, 0, 0, 0, 0, 0 };
  usb_interrupt_write(handle, 2, data, sizeof(data), 5000);

  ret = 0;

exit:
  if(device && handle)
    usb_close(handle);

  return ret;
}
