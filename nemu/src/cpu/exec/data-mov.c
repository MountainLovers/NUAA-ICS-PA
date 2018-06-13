#include "cpu/exec.h"

make_EHelper(mov) {
  operand_write(id_dest, &id_src->val);
  print_asm_template2(mov);
}

make_EHelper(push) {
  //TODO();
  rtl_push(&id_dest->val);	
	
  print_asm_template1(push);
}

make_EHelper(pop) {
  //TODO();
  rtl_pop(&t0);
	operand_write(id_dest, &t0);
  print_asm_template1(pop);
}

make_EHelper(pusha) {
  //TODO();
  if (decoding.is_operand_size_16) {
		rtl_push((rtlreg_t *)&reg_w(R_AX));
		rtl_push((rtlreg_t *)&reg_w(R_CX));
		rtl_push((rtlreg_t *)&reg_w(R_DX));
		rtl_push((rtlreg_t *)&reg_w(R_BX));
		rtl_push((rtlreg_t *)&reg_w(R_SP));
		rtl_push((rtlreg_t *)&reg_w(R_BP));
		rtl_push((rtlreg_t *)&reg_w(R_SI));
		rtl_push((rtlreg_t *)&reg_w(R_DI));
	}else{
		rtl_push(&reg_l(R_EAX));
		rtl_push(&reg_l(R_ECX));
		rtl_push(&reg_l(R_EDX));
		rtl_push(&reg_l(R_EBX));
		rtl_push(&reg_l(R_ESP));
		rtl_push(&reg_l(R_EBP));
		rtl_push(&reg_l(R_ESI));
		rtl_push(&reg_l(R_EDI));
	}
  print_asm("pusha");
}

make_EHelper(popa) {
  //TODO();
  if (decoding.is_operand_size_16) {
		rtl_pop((rtlreg_t *)&reg_w(R_DI));
		rtl_pop((rtlreg_t *)&reg_w(R_SI));
		rtl_pop((rtlreg_t *)&reg_w(R_BP));
		rtl_pop(&t0);
		rtl_pop((rtlreg_t *)&reg_w(R_BX));
		rtl_pop((rtlreg_t *)&reg_w(R_DX));
		rtl_pop((rtlreg_t *)&reg_w(R_CX));
		rtl_pop((rtlreg_t *)&reg_w(R_AX));
	}else{
		rtl_pop(&reg_l(R_EDI));
		rtl_pop(&reg_l(R_ESI));
		rtl_pop(&reg_l(R_EBP));
		rtl_pop(&t0);
		rtl_pop(&reg_l(R_EBX));
		rtl_pop(&reg_l(R_EDX));
		rtl_pop(&reg_l(R_ECX));
		rtl_pop(&reg_l(R_EAX));
	}
		
  print_asm("popa");
}

make_EHelper(leave) {
  cpu.esp = cpu.ebp;

	rtl_pop(&cpu.ebp);

  print_asm("leave");
}

make_EHelper(cltd) {
  
	if (decoding.is_operand_size_16) {
    if ((int)(cpu.eax & 0xFFFF) < 0) cpu.edx = cpu.edx | 0xFFFF;
		else cpu.edx = cpu.edx & 0xFFFF0000;
  }
  else {
    if ((int)cpu.eax < 0) cpu.edx = 0xFFFFFFFF;
		else cpu.edx = 0x00000000;
  }

  print_asm(decoding.is_operand_size_16 ? "cwtl" : "cltd");
}

make_EHelper(cwtl) {

	if (decoding.is_operand_size_16) {
    rtl_sext(&cpu.eax, &cpu.eax, 1);
  }
  else {
    rtl_sext(&cpu.eax, &cpu.eax, 2);
  }

  print_asm(decoding.is_operand_size_16 ? "cbtw" : "cwtl");
}

make_EHelper(movsx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  rtl_sext(&t2, &id_src->val, id_src->width);
  operand_write(id_dest, &t2);
  print_asm_template2(movsx);
}

make_EHelper(movzx) {
  id_dest->width = decoding.is_operand_size_16 ? 2 : 4;
  operand_write(id_dest, &id_src->val);
  print_asm_template2(movzx);
}

make_EHelper(lea) {
  rtl_li(&t2, id_src->addr);
  operand_write(id_dest, &t2);
  print_asm_template2(lea);
}
