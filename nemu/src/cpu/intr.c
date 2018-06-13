#include "cpu/exec.h"
#include "memory/mmu.h"

void raise_intr(uint8_t NO, vaddr_t ret_addr) {
  /* TODO: Trigger an interrupt/exception with ``NO''.
   * That is, use ``NO'' to index the IDT.
   */

  rtl_push((rtlreg_t *)&cpu.extend_flags);
	rtl_push((rtlreg_t *)&cpu.cs);
	rtl_push((rtlreg_t *)&ret_addr);
	uint32_t low = vaddr_read(cpu.idtr.base + 8 * NO, 4);
	uint32_t high = vaddr_read(cpu.idtr.base + 8 * NO + 4, 4);
	if (!(high >> 15 & 0x01)) assert(0);
	uint32_t offset = (low & 0xFFFF) | (high & 0xFFFF0000);
	decoding.is_jmp = true;
	decoding.jmp_eip = offset;
}

void dev_raise_intr() {
}
