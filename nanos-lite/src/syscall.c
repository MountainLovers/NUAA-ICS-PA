#include "common.h"
#include "syscall.h"


extern ssize_t fs_read(int fd, void *buf, size_t len);
extern ssize_t fs_write(int fd, const void *buf, size_t len);
extern int fs_open(const char *pathname, int flags, int mode);
extern off_t fs_lseek(int fd, off_t offset, int whence);
extern int fs_close(int fd);


size_t sys_write(int fd, void *buf, size_t count) {
	/*int i;
	if ((fd == 1) || (fd == 2)) {
		for (i = 0; i < count; i++) _putc(((char *)buf)[i]);
	}*/
	size_t len = fs_write(fd, buf, count);
	return len;
}


_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);
  a[1] = SYSCALL_ARG2(r);
	a[2] = SYSCALL_ARG3(r);
	a[3] = SYSCALL_ARG4(r);
  switch (a[0]) {
		case SYS_none:
						SYSCALL_ARG1(r) = 1;
						break;
		case SYS_exit:
						_halt(a[1]);
						break;
		case SYS_write:
						Log("Call SYS_write!!");
						SYSCALL_ARG1(r) = sys_write(a[1], (void *)a[2], a[3]);
						break;
		case SYS_brk:
						SYSCALL_ARG1(r) = 0;
						break;
		case SYS_read:
						SYSCALL_ARG1(r) = fs_read(a[1], (void *)a[2], a[3]);
						break;
		case SYS_open:
						SYSCALL_ARG1(r) = fs_open((char *)a[1], a[2], a[3]);
						break;
		case SYS_close:
						SYSCALL_ARG1(r) = fs_close(a[1]);
						break;
		case SYS_lseek:
						SYSCALL_ARG1(r) = fs_lseek(a[1], a[2], a[3]);
						break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
