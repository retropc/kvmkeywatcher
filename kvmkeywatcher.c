#define _DEFAULT_SOURCE
#define _GNU_SOURCE

#include <sys/time.h>
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>
#include <poll.h>

#define OTHER_PORT 2
#define VENDOR_ID   0x10d5
#define PRODUCT_ID  0x55a2

#define MAX_DEVICES 50

static struct pollfd fds[MAX_DEVICES];
static int fd_count;
static int fds_device_map[MAX_DEVICES];
static int watched_devices[MAX_DEVICES];

struct input_event {
  struct timeval time;
  unsigned short type;
  unsigned short code;
  unsigned int value;
};

extern int switch_port(int vendor, int product, int port);

static void trigger_switch_port(int code) {
#ifdef DEBUG
  fprintf(stderr, "switch port\n");
#else
  switch_port(VENDOR_ID, PRODUCT_ID, OTHER_PORT);
#endif
}

#ifdef DEBUG
static void trigger_debug(int code) {
  fprintf(stderr, "pressed %d\n", code);
}
#endif

static void handle_event(struct input_event *ie) {
  static time_t last;
  void (*fn)(int);

  switch(ie->code) {
    case 69: /* numlock */
      fn = trigger_switch_port;
      break;
    default:
#ifdef DEBUG
      fn = trigger_debug;
#else
      return;
#endif
  }

  if(ie->value == 1 && ie->type == 1) {
    if(last == ie->time.tv_sec || last + 1 == ie->time.tv_sec) {
      last = 0;

      pid_t pid = fork();
      if(pid == -1)
        return;

      if(pid == 0) {
        fn(ie->code);
        _exit(0);
      }
    } else {
      last = ie->time.tv_sec;
    }
  }
}

/*
static void dump(void) {
  printf("\n");
  printf("fd_count: %d\n", fd_count);
  printf("fds: [");

  int i;
  for(i=0;i<fd_count;i++) {
    printf("{fd: %d, events: %d, [dm: %d]}, ", fds[i].fd, fds[i].events, fds_device_map[i]);
  }
  printf("]\n");

  printf("watched_devices: {");
  for(i=0;i<MAX_DEVICES;i++) {
    if(watched_devices[i] != -1)
      printf("%d: %d, ", i, watched_devices[i]);
  }

  printf("}\n");
  printf("\n");
}
*/

static void open_device(int device) {
  char buf[512];
  if(device < 0 || device >= MAX_DEVICES || watched_devices[device] >= 0)
    return;

  snprintf(buf, sizeof(buf), "/dev/input/event%d", device);
  int f = open(buf, O_RDONLY);
  if(f < 0)
    return;

  watched_devices[device] = f;
  fds[fd_count].fd = f;
  fds[fd_count].events = POLLIN;
  fds[fd_count].revents = 0;
  fds_device_map[fd_count] = device;

  fd_count++;
}

static void rescan(void) {
  int want, device;
  char buf[1024];

  FILE *fd = fopen("/proc/bus/input/devices", "r");
  if(!fd)
    return;

  for(;;) {
    if(!fgets(buf, sizeof(buf), fd))
      break;

    switch(buf[0]) {
      case 'I': { /* new device */
        want = 0;
        device = -1;
        break;
      }
      case 'H': { /* H: Handlers=sysrq event1 */
        if(strncmp(buf, "H: Handlers=", 12))
          break;

        char *token = strtok(buf + 12, " ");
        while(token) {
          if(!strncmp(token, "event", 5)) {
            device = atoi(token + 5);
            break;
          }

          token = strtok(NULL, " ");
        }
      }
      case 'B': { /* B: EV=120013 */
        if(!strcmp(buf, "B: EV=120013\n"))
          want = 1;
        break;
      }
      case '\n': {
        /* end section */
        if(device != -1 && want)
          open_device(device);

        break;
      }
    }
  }

  fclose(fd);
}

static void close_device(int position) {
  int fd = fds[position].fd;
  int device = fds_device_map[position];
  watched_devices[device] = -1;

  close(fd);

  if(position != fd_count - 1) {
    memcpy(&fds[position], &fds[fd_count - 1], sizeof(struct pollfd));
    fds_device_map[position] = fds_device_map[fd_count - 1];
  }

  fd_count--;
}

int main(void) {
  time_t last_scan = 0;
  int i;

  if(getuid() != 0) {
    fprintf(stderr, "run as root\n");
    return 1;
  }

  for(i=0;i<MAX_DEVICES;i++)
    watched_devices[i] = -1;

#ifdef DEBUG
  open_device(0);
#else
  if(daemon(0, 0) < 0) {
    fprintf(stderr, "unable to daemonise\n");
    return 1;
  }
#endif

  for(;;) {
    time_t now = time(NULL);
    if(now - last_scan > 5) {
      rescan();
      last_scan = now;
    }

    int ret = poll(fds, fd_count, 2000);
    for(i=0;ret>0;i++) {
      if(fds[i].revents == 0)
        continue;

      ret--;

      struct input_event ie;
      fds[i].revents = 0;

      int r = read(fds[i].fd, &ie, sizeof(ie));
      if(r <= 0 || r < sizeof(ie)) {
        close_device(i);
        i--;
      } else {
        handle_event(&ie);
      }
    }
  }

  return 0;
}
