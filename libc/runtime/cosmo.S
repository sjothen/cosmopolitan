/*-*- mode:unix-assembly; indent-tabs-mode:t; tab-width:8; coding:utf-8     -*-│
│vi: set et ft=asm ts=8 tw=8 fenc=utf-8                                     :vi│
╞══════════════════════════════════════════════════════════════════════════════╡
│ Copyright 2020 Justine Alexandra Roberts Tunney                              │
│                                                                              │
│ Permission to use, copy, modify, and/or distribute this software for         │
│ any purpose with or without fee is hereby granted, provided that the         │
│ above copyright notice and this permission notice appear in all copies.      │
│                                                                              │
│ THE SOFTWARE IS PROVIDED "AS IS" AND THE AUTHOR DISCLAIMS ALL                │
│ WARRANTIES WITH REGARD TO THIS SOFTWARE INCLUDING ALL IMPLIED                │
│ WARRANTIES OF MERCHANTABILITY AND FITNESS. IN NO EVENT SHALL THE             │
│ AUTHOR BE LIABLE FOR ANY SPECIAL, DIRECT, INDIRECT, OR CONSEQUENTIAL         │
│ DAMAGES OR ANY DAMAGES WHATSOEVER RESULTING FROM LOSS OF USE, DATA OR        │
│ PROFITS, WHETHER IN AN ACTION OF CONTRACT, NEGLIGENCE OR OTHER               │
│ TORTIOUS ACTION, ARISING OUT OF OR IN CONNECTION WITH THE USE OR             │
│ PERFORMANCE OF THIS SOFTWARE.                                                │
╚─────────────────────────────────────────────────────────────────────────────*/
#include "libc/macros.internal.h"
#include "libc/notice.inc"
#include "libc/sysv/consts/prot.h"
#include "libc/sysv/consts/map.h"
#include "libc/calls/strace.internal.h"
#include "libc/sysv/consts/map.h"
#include "libc/dce.h"
.text.startup

//	Cosmopolitan runtime.
//
//	@param	edi is argc
//	@param	rsi is argv
//	@param	rdx is environ
//	@param	rcx is auxv
//	@noreturn
cosmo:	push	%rbp
	mov	%rsp,%rbp
	mov	%edi,%r12d
	mov	%rsi,%r13
	mov	%rdx,%r14
	mov	%rcx,%r15
#ifdef SYSDEBUG
	call	__strace_init
	mov	%eax,%r12d
#endif /* SYSDEBUG */
#ifdef __FAST_MATH__
	push	%rax
	stmxcsr	(%rsp)
	orl	$0x8040,(%rsp)
	ldmxcsr	(%rsp)
	pop	%rax
#endif
	call	_init
	ezlea	__init_array_start,ax		# static ctors in forward order
	.weak	__init_array_start		# could be called multiple times
	ezlea	__init_array_end,cx		# idempotency recommended
	.weak	__init_array_end		# @see ape/ape.lds
1:	cmp	%rax,%rcx
	je	2f
	push	%rax
	push	%rcx
	call	.Largs
	call	*(%rax)
	pop	%rcx
	pop	%rax
	add	$8,%rax
	jmp	1b
2:	call	.Largs
	.weak	main
	call	main
	xchg	%eax,%edi
	call	exit
.Largs:	mov	%r12d,%edi
	mov	%r13,%rsi
	mov	%r14,%rdx
	mov	%r15,%rcx
	ret
	.endfn	cosmo,weak

#if !IsTiny()
//	Creates deterministically addressed stack we can use
//
//	This helps debugging be more comprehensible, because
//	when diagnosing low-level problems when error report
//	isn't working, sometimes numbers are all you have to
//	go on, and we can't use them if kernel hardening has
//	configured that meaningful data to be randomized.
//
//	Having deterministic addresses is also key to ensure
//	builds, execution, and other things are reproducible
	.init.start 304,_init_stack
	testb	IsWindows()
	jnz	9f
	push	%rdi
	push	%rsi
//	allocate stack
	movabs	$ape_stack_vaddr,%rdi
	mov	$ape_stack_memsz,%esi
	mov	$ape_stack_prot,%edx
	mov	$MAP_STACK,%ecx
	or	MAP_ANONYMOUS,%ecx
	or	$-1,%r8
	xor	%r9d,%r9d
	push	%rsi
	call	mmap
	pop	%r8
	pop	%rsi
	pop	%rdi
	cmp	$-1,%rax
	je	9f
//	switch stacks
	leave
	pop	%rcx
	lea	(%rax,%r8),%rsp
	sub	$ape_stack_align,%rsp		# openbsd:stackbound
	mov	%rbp,(%rsp)
	push	%rcx
	push	%rbp
	mov	%rsp,%rbp
9:	nop
	.init.end 304,_init_stack
	.weak	ape_stack_prot
	.weak	ape_stack_vaddr
	.weak	ape_stack_memsz
	.weak	ape_stack_align
#endif

#ifdef __PG__
	.init.start 306,_init_ftrace
	push	%rdi
	push	%rsi
	call	ftrace_init
	mov	%eax,%r12d
	pop	%rsi
	pop	%rdi
	.init.end 306,_init_ftrace
#endif

#if IsAsan()
	.init.start 306,_init_symbols
	push	%rdi
	push	%rsi
	call	__init_symbols
	pop	%rsi
	pop	%rdi
	.init.end 306,_init_symbols
#endif

#if IsModeDbg()
#ifdef SYSDEBUG
	.init.start 307,_init_printargs
	cmpl	$0,__strace(%rip)
	jz	1f
	push	%rdi
	push	%rsi
	loadstr	STRACE_PROLOGUE,di
	call	__printargs
	pop	%rsi
	pop	%rdi
1:	.init.end 307,_init_printargs
#endif
#endif
