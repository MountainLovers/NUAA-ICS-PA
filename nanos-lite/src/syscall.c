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
						SYSCALL_ARG1(r) = sys_write(a[1], (void *)a[2], a[3]);
						break;
		case SYS_brk:
						SYSCALL_ARG1(r) = 0;
						break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }

  return NULL;
}
