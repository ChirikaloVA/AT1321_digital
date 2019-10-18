;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;;
;;
;; Part one of the system initialization code,
;; contains low-level
;; initialization.
;;
;; Copyright 2006 IAR Systems. All rights reserved.
;;
;; $Revision: 21638 $
;;

        MODULE  ?cstartup

        ;; Forward declaration of sections.
        SECTION IRQ_STACK:DATA:NOROOT(3)
        SECTION FIQ_STACK:DATA:NOROOT(3)
        SECTION SVC_STACK:DATA:NOROOT(3)
        SECTION ABT_STACK:DATA:NOROOT(3)
        SECTION UND_STACK:DATA:NOROOT(3)
        SECTION CSTACK:DATA:NOROOT(3)

;
; The module in this file are included in the libraries, and may be
; replaced by any user-defined modules that define the PUBLIC symbol
; __iar_program_start or a user defined start symbol.
;
; To override the cstartup defined in the library, simply add your
; modified version to the workbench project.

        SECTION .intvec:CODE:NOROOT(2)

        PUBLIC  __vector
        PUBLIC  __iar_program_start
        PUBLIC  __vector_0x14
		PUBLIC	exception
		PUBLIC	Interrupts_get_Registers
		
        EXTERN	undef_handler, swi_handler, prefetch_handler
		    EXTERN	data_handler, irq_handler, fiq_handler
                ARM	; Always ARM mode after reset	
__vector:
		ldr	pc,[pc,#24]	; Absolute jump can reach 4 GByte
__undef_handler:
    ldr	pc,[pc,#24]	; Branch to undef_handler
__swi_handler:
		ldr	pc,[pc,#24]	; Branch to swi_handler
__prefetch_handler:
		ldr	pc,[pc,#24]	; Branch to prefetch_handler
__data_handler
		ldr	pc,[pc,#24]	; Branch to data_handler
__vector_0x14
    dc32 0xFFFFFFFF
__irq_handler:
		ldr   pc,[pc, #24] 	; Branch to irq_handler
__fiq_handler:
		ldr	pc,[pc,#24]	; Branch to fiq_handler

		; Constant table entries (for ldr pc) will be placed at 0x20
      dc32	__iar_program_start
      dc32	undef_handler
      dc32	swi_handler
      dc32	prefetch_handler
      dc32	data_handler
      dc32	0xFFFFFFFF
      dc32	irq_handler
      dc32	fiq_handler

; --------------------
; Mode, correspords to bits 0-5 in CPSR

MODE_MSK DEFINE 0x1F            ; Bit mask for mode bits in CPSR

USR_MODE DEFINE 0x10            ; User mode
FIQ_MODE DEFINE 0x11            ; Fast Interrupt Request mode
IRQ_MODE DEFINE 0x12            ; Interrupt Request mode
SVC_MODE DEFINE 0x13            ; Supervisor mode
ABT_MODE DEFINE 0x17            ; Abort mode
UND_MODE DEFINE 0x1B            ; Undefined Instruction mode
SYS_MODE DEFINE 0x1F            ; System mode

CP_DIS_MASK DEFINE 0xFFFFFFF2

        SECTION .text:CODE:NOROOT(2)

        EXTERN  ?main
        REQUIRE __vector
        EXTERN  low_level_init
		EXTERN _reg_R13, _reg_R14, _reg_SPSR, _reg_CPSR
		EXTERN _reg_R0, _reg_R1, _reg_R2, _reg_R3
		EXTERN _reg_R4, _reg_R5, _reg_R6, _reg_R7
		EXTERN _reg_R8, _reg_R9, _reg_R10, _reg_R11, _reg_R12
		EXTERN _stackVal1,_stackVal2,_stackVal3,_stackVal4

        ARM

__iar_program_start:
?cstartup:

I_Bit    DEFINE 0x80            ; when I bit is set, IRQ is disabled
F_Bit    DEFINE 0x40            ; when F bit is set, FIQ is disabled

#define VIC_INT_ENABLE  0xFFFFF014
; Disable all interrupts
                ldr   r0,=VIC_INT_ENABLE
                mov   r1,#0xFFFFFFFF
                str   r1,[r0]

; Execution starts here.
; After a reset, the mode is ARM, Supervisor, interrupts disabled.
; Initialize the stack pointers.
; The pattern below can be used for any of the exception stacks:
; FIQ, IRQ, SVC, ABT, UND, SYS.
; The USR mode uses the same stack as SYS.
; The stack segments must be defined in the linker command file,
; and be declared above.

                mrs         r0,cpsr                             ; Original PSR value
                bic         r0,r0,#MODE_MSK                     ; Clear the mode bits
                orr         r0,r0,#SVC_MODE                     ; Set Supervisor mode bits
                msr         cpsr_c,r0                           ; Change the mode
                ldr         sp,=SFE(SVC_STACK)                  ; End of SVC_STACK

                bic         r0,r0,#MODE_MSK                     ; Clear the mode bits
                orr         r0,r0,#UND_MODE                     ; Set Undefined mode bits
                msr         cpsr_c,r0                           ; Change the mode
                ldr         sp,=SFE(UND_STACK)                  ; End of UND_MODE

                bic         r0,r0,#MODE_MSK                     ; Clear the mode bits
                orr         r0,r0,#ABT_MODE                     ; Set Data abort mode bits
                msr         cpsr_c,r0                           ; Change the mode
                ldr         sp,=SFE(ABT_STACK)                  ; End of ABT_STACK

                bic         r0,r0,#MODE_MSK                     ; Clear the mode bits
                orr         r0,r0,#FIQ_MODE                     ; Set FIR mode bits
                msr         cpsr_c,r0                           ; Change the mode
                ldr         sp,=SFE(FIQ_STACK)                  ; End of FIQ_STACK

                bic         r0,r0,#MODE_MSK                     ; Clear the mode bits
                orr         r0,r0,#IRQ_MODE                     ; Set IRQ mode bits
                msr         cpsr_c,r0                           ; Change the mode
                ldr         sp,=SFE(IRQ_STACK)                  ; End of IRQ_STACK

                bic         r0,r0,#MODE_MSK | I_Bit | F_Bit     ; Clear the mode bits
                orr         r0,r0,#SYS_MODE                     ; Set System mode bits
                msr         cpsr_c,r0                           ; Change the mode
                ldr         sp,=SFE(CSTACK)                     ; End of CSTACK

#ifdef __ARMVFP__
; Enable the VFP coprocessor.
                mov     r0, #BASE_ARD_EIM                   ; Set EN bit in VFP
                fmxr    fpexc, r0                           ; FPEXC, clear others.

; Disable underflow exceptions by setting flush to zero mode.
; For full IEEE 754 underflow compliance this code should be removed
; and the appropriate exception handler installed.
                mov     r0, #0x01000000		                  ; Set FZ bit in VFP
                fmxr    fpscr, r0                           ; FPSCR, clear others.
#endif

; Add more initialization here


; Continue to ?main for more IAR specific system startup

                ldr     r0,=?main
                bx      r0





;get R14 register to use in C code
		CODE32
exception
		SWI		0
		
Interrupts_get_Registers
		STR      LR,[SP, #-4]!
		MRS      LR,SPSR
		STMDB    SP!,{R0-R2,LR}

		PUSH	{R12}
		LDR		R12, =_reg_R7
		STR		R7,[R12]
		LDR		R12, =_reg_R6
		STR		R6,[R12]
		LDR		R12, =_reg_R5
		STR		R5,[R12]
		LDR		R12, =_reg_R4
		STR		R4,[R12]
		LDR		R12, =_reg_R3
		STR		R3,[R12]
		LDR		R12, =_reg_R2
		STR		R2,[R12]
		LDR		R12, =_reg_R1
		STR		R1,[R12]
		LDR		R12, =_reg_R0
		STR		R0,[R12]
		POP		{R12}
		LDR		R0, =_reg_R12
		STR		R12,[R0]
		
		MRS      R2,CPSR
		MRS      R1,SPSR
		;prepare switch to mode from where exception was
		MOV      R0,R1
		AND      R0,R0,#0x1f
		ORR      R0,R0,#0xc0	;disable INT
		MSR      CPSR_c,R0
		
		LDR		R0, =_reg_R11
		STR		R11,[R0]
		LDR		R0, =_reg_R10
		STR		R10,[R0]
		LDR		R0, =_reg_R9
		STR		R9,[R0]
		LDR		R0, =_reg_R8
		STR		R8,[R0]

		LDR		R0, =_reg_R13
		STR		R13,[R0]
		LDR		R0, =_reg_R14
		STR		R14,[R0]
		LDR		R0, =_reg_SPSR
		STR		R1,[R0]
		MOV		R0, R13
		LDR		R1,[R0,#-4]
		LDR		R0, =_stackVal1
		STR		R1,[R0]
		MOV		R0, R13
		LDR		R1,[R0,#-8]
		LDR		R0, =_stackVal2
		STR		R1,[R0]
		MOV		R0, R13
		LDR		R1,[R0,#-12]
		LDR		R0, =_stackVal3
		STR		R1,[R0]
		MOV		R0, R13
		LDR		R1,[R0,#-16]
		LDR		R0, =_stackVal4
		STR		R1,[R0]

		MSR      CPSR,R2

		LDM      SP!,{R0-R2,LR}
		MSR      SPSR,LR
		LDM      SP!,{PC}        ;; return








        SECTION .endof:CONST:ROOT(2)
        PUBLIC  endfunction
        ARM
endfunction:
        dc32 0xFFFFFFFF
        dc32 0xFFFFFFFF















                END
