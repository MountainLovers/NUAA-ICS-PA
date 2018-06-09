#include "common.h"
#include "syscall.h"

size_t sys_write(int fd, void *buf, size_t count) {
	int i;
	if ((fd == 1) || (fd == 2)) {
		for (i = 0; i < count; i++) _putc(((char *)buf)[i]);
	}
	return count;
}




_RegSet* do_syscall(_RegSet *r) {
  uintptr_t a[4];
  a[0] = SYSCALL_ARG1(r);

  switch (a[0]) {
		case SYS_none:
						SYSCALL_ARG1(r) = 1;
						break;
		case SYS_exit:
						_halt(SYSCALL_ARG2(r));
						break;
		case SYS_write:{
						int fd = SYSCALL_ARG2(r);
						void *buf = (void *)SYSCALL_ARG3(r);
						size_t count = SYSCALL_ARG4(r);
						SYSCALL_ARG1(r) = sys_write(fd, buf, count);
						break;
		}
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
