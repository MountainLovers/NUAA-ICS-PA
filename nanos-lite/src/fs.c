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
extern void ramdisk_write(const void *buf, off_t offset, size_t len);
extern void dispinfo_read(void *buf, off_t offset, size_t len);
extern void fb_write(const void *buf, off_t offset, size_t len);
extern size_t events_read(void *buf, size_t len);

void init_fs() {
  // TODO: initialize the size of /dev/fb
	file_table[FD_FB].size = _screen.height * _screen.width * 4;
}

size_t fs_filesz(int fd) {
	return file_table[fd].size;
}

int fs_open(const char *pathname, int flags, int mode) {
	Log("fs_open 啦!");
	int i;
	for (i = 0; i < NR_FILES; i++) {
		if (strcmp(pathname, file_table[i].name) == 0) {
			file_table[i].open_offset = 0;
			return i;
		}
	}
	Log("pathname = %s", pathname);
	panic("fs_open failed!");
	assert(0);
	return -1;
}

ssize_t fs_read(int fd, void *buf, size_t len) {
	Log("fs_read 啦!");
	switch (fd) {
		case 0:
		case 1:
		case 2:
			panic("fd < 2 in fs_read");
			assert(0);
			break;
		case FD_EVENTS:
			len = events_read(buf, len);
			break;
		case FD_DISPINFO:
			if (file_table[fd].open_offset > file_table[fd].size) {
				panic("fs_read FD_DISPINFO WRONG!");
			}
			if (file_table[fd].open_offset + len > file_table[fd].size) len = file_table[fd].size - file_table[fd].open_offset;
			dispinfo_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
			file_table[fd].open_offset += len;
			break;
		default:
			if (file_table[fd].open_offset > file_table[fd].size) {
				panic("fs_read default wrong!");
			}
			if (file_table[fd].size - file_table[fd].open_offset < len) len = file_table[fd].size - file_table[fd].open_offset;
			ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
			file_table[fd].open_offset += len;
	}
	return len;
}

ssize_t fs_write(int fd, const void *buf, size_t len) {
	switch (fd) {
		case FD_STDOUT:
		case FD_STDERR:{
			int i;
			for (i = 0; i < len; i++) _putc(((char *)buf)[i]);
			break;
		}
		case FD_FB:
			fb_write(buf, file_table[fd].open_offset, len);
			file_table[fd].open_offset += len;
			break;
		default:
 			if (file_table[fd].size - file_table[fd].open_offset < len) len = file_table[fd].size - file_table[fd].open_offset;
			ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
			file_table[fd].open_offset += len;
	}
	return len;
}

off_t fs_lseek(int fd, off_t offset, int whence) {
	int p = file_table[fd].open_offset, size = file_table[fd].size;
	switch (whence) {
		case SEEK_SET:
			p = offset;
			break;
		case SEEK_CUR:
			p += offset;
			break;
		case SEEK_END:
			p = size + offset;
			break;
	}
	if (whence == SEEK_END) {
		file_table[fd].open_offset = p;
		return file_table[fd].open_offset;
	}else{
		if (p >= 0 && p <= size) {
			file_table[fd].open_offset = p;
			return file_table[fd].open_offset;
		}else{
			if (p < 0) file_table[fd].open_offset = 0;
			if (p > size) file_table[fd].open_offset = size+1;
			return -1;
		}
	}
}

int fs_close(int fd) {
	return 0;
}
