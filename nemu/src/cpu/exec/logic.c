#include "cpu/exec.h"

make_EHelper(test) {
  rtl_and(&t0, &id_dest->val, &id_src->val);

	rtl_update_ZFSF(&t0, id_dest->width);
  uint32_t flag = 0;
	rtl_set_CF(&flag);
  rtl_set_OF(&flag);


  print_asm_template2(test);
}

make_EHelper(and) {
  rtl_and(&t0, &id_dest->val, &id_src->val);
	operand_write(id_dest, &t0);

	rtl_update_ZFSF(&t0, id_dest->width);
  uint32_t flag = 0;
	rtl_set_CF(&flag);
  rtl_set_OF(&flag);

  print_asm_template2(and);
}

make_EHelper(xor) {
  rtl_xor(&t0, &id_dest->val, &id_src->val);
  operand_write(id_dest, &t0);

	rtl_update_ZFSF(&t0, id_dest->width);
	uint32_t flag = 0;
	rtl_set_CF(&flag);
	rtl_set_OF(&flag);
  print_asm_template2(xor);
}

make_EHelper(or) {
  rtl_or(&t0, &id_dest->val, &id_src->val);
	operand_write(id_dest, &t0);

  rtl_update_ZFSF(&t0, id_dest->width);
	uint32_t flag = 0;
	rtl_set_CF(&flag);
	rtl_set_OF(&flag);
  print_asm_template2(or);
}

make_EHelper(sar) {
  //TODO();
  // unnecessary to update CF and OF in NEMU
  rtl_sar(&t0, &id_dest->val, &id_src->val);
	operand_write(id_dest, &t0);

	rtl_update_ZFSF(&t0, id_dest->width);
  print_asm_template2(sar);
}

make_EHelper(shl) {
  //TODO();
  // unnecessary to update CF and OF in NEMU
  rtl_shl(&t0, &id_dest->val, &id_src->val);
	operand_write(id_dest, &t0);

	rtl_update_ZFSF(&t0, id_dest->width);

  print_asm_template2(shl);
}

make_EHelper(shr) {
  //TODO();
  // unnecessary to update CF and OF in NEMU
  rtl_shr(&t0, &id_dest->val, &id_src->val);
	operand_write(id_dest, &t0);

	rtl_update_ZFSF(&t0, id_dest->width);

  print_asm_template2(shr);
}

make_EHelper(setcc) {
  uint8_t subcode = decoding.opcode & 0xf;
  rtl_setcc(&t2, subcode);
  operand_write(id_dest, &t2);

  print_asm("set%s %s", get_cc_name(subcode), id_dest->str);
}

make_EHelper(not) {
  //TODO();
  rtlreg_t num = 0xFFFFFFFF;

  rtl_xor(&t1, &id_dest->val, &num);
	operand_write(id_dest, &t1);

  print_asm_template1(not);
}

make_EHelper(rol) {
	int count = id_src->val;
	int temp = count;
	t0 = id_dest->val;
	while (temp != 0) {
		int tmpcf = (((t0 << (32 - id_dest->width * 8)) >> 31) & 0x01);
	  t0 = (t0 << 1) + tmpcf;
		temp--;
  }
	if (count == 1) {
		if ((((t0 << (32 - id_dest->width * 8)) >> 31) & 0x01) != cpu.eflags.CF) cpu.eflags.OF = 1; else cpu.eflags.OF = 0;
	}
	operand_write(id_dest, &t0);

	//printf_asm_template2(rol);
}
