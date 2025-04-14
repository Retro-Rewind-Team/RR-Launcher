/*
    exception_catch.S - copies registers and calls into C handler

    This code is adapted from the Homebrew channel.
    See: https://github.com/fail0verflow/hbc/blob/master/wiipax/stub/exception_asm.s

    Copyright (C) fail0verflow and all contributors. Licensed under GPL-2-or-later.
    See <https://www.gnu.org/licenses/> for license details.

    2025-04-14 (Jacherr) - Updated label names and routine name called into in C code.
*/

	.globl exception_catch_start, exception_catch_end

exception_catch_start:
	# store all interesting regs
	mfcr 0 ;  stw 0,0x3580(0)
	mfxer 0 ; stw 0,0x3584(0)
	mflr 0 ;  stw 0,0x3588(0)
	mfctr 0 ; stw 0,0x358c(0)
	mfsrr0 0 ;  stw 0,0x3590(0)
	mfsrr1 0 ;  stw 0,0x3594(0)
	mfdar 0 ;   stw 0,0x3598(0)
	mfdsisr 0 ; stw 0,0x359c(0)

	# switch on FP, DR, IR
	mfmsr 0 ; ori 0,0,0x2030 ; mtsrr1 0

    # go to C handler
	lis 0,_handle_exception@h ; ori 0,0,_handle_exception@l ; mtsrr0 0
	rfi
exception_catch_end: