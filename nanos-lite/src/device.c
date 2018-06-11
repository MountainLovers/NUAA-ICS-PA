#include "common.h"

#define NAME(key) \
  [_KEY_##key] = #key,

static const char *keyname[256] __attribute__((used)) = {
  [_KEY_NONE] = "NONE",
  _KEYS(NAME)
};

size_t events_read(void *buf, size_t len) {
	int key = _read_key();
  bool down = false;
  if (key & 0x8000) {
    key ^= 0x8000;
    down = true;
  }   
  if (key != _KEY_NONE) {
		if (down) sprintf(buf, "%s %s\n", "kd", keyname[key]);
		else sprintf(buf, "%s %s\n", "ku", keyname[key]);
  }else{
		unsigned long time = _uptime();
	  sprintf(buf, "t %d\n", time);
	}
	return strlen(buf);	
}

static char dispinfo[128] __attribute__((used));

void dispinfo_read(void *buf, off_t offset, size_t len) {
	strncpy(buf, dispinfo + offset, len);	
}

void fb_write(const void *buf, off_t offset, size_t len) {
	int x, y;
	offset /= 4;
	x = offset % _screen.width;
	y = offset / _screen.width;
	_draw_rect(buf, x, y, len/4, 1);
}

void init_device() {
  _ioe_init();
	strcpy(dispinfo, "WIDTH:400\nHEIGHT:300");
	Log("dispinfo:\n%s", dispinfo);
  // TODO: print the string to array `dispinfo` with the format
  // described in the Navy-apps convention
}
