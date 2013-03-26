//interrupts.h

#ifndef _INTERRUPTS_H
#define _INTERRUPTS_H

#include <iolpc2388.h>
#include "types.h"


extern DWORD _reg_R13, _reg_R14, _reg_CPSR, _reg_SPSR;
extern DWORD _reg_R0, _reg_R1, _reg_R2, _reg_R3,_reg_R4, _reg_R5, _reg_R6, _reg_R7,_reg_R8, _reg_R9, _reg_R10, _reg_R11, _reg_R12;


__arm void Interrupts_get_Registers(void);
__arm void exception(const char* pModuleName,const char* pFuncName, int LineNum, const char* pDesc);
__arm int endfunction(void);

//removed __nested
 __irq __nested __arm void irq_handler(void);
void Interrupts_Init(void);


__arm void __program_start(void);


__irq __arm void undef_handler(void);

__swi __arm void swi_handler(void);

__irq __arm void prefetch_handler(void);

__irq __arm void data_handler(void);

__irq __nested __arm void fiq_handler(void);


#endif  //#ifndef _INTERRUPTS_H
