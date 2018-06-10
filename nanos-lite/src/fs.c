#include "fs.h"

typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
	off_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

extern void ramdisk_read(void *buf, off_t offset, size_t len);

void init_fs() {
  // TODO: initialize the size of /dev/fb
}

size_t fs_filesz(int fd) {
	return file_table[fd].size;
}

int fs_open(const char *pathname, int flags, int mode) {
	int i;
	for (i = 0; i < NR_FILES; i++) {
		if (strcmp(pathname, file_table[i].name) == 0) {
			file_table[i].open_offset = 0;
			return i;
		}
	}
	panic("fs_open failed!");
	assert(0);
	return -1;
}

ssize_t fs_read(int fd, void *buf, size_t len) {
	if (fd <= 2) {
		panic("fd < 2 in fs_read");
		assert(0);
	}
	int last_len = file_table[fd].size - file_table[fd].open_offset;
	if (last_len < len) len = last_len;
	ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
	file_table[fd].open_offset += len;
	return len;
}

/*ssize_t fs_write(int fd, const void *buf, size_t len) {
}

off_t fs_lseek(int fd, off_t offset, int whence) {
}*/

int fs_close(int fd) {
	return 0;
}
