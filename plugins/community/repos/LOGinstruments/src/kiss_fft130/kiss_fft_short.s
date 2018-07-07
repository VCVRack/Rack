	.file	"kiss_fft.c"
# GNU C11 (Ubuntu 5.4.0-6ubuntu1~16.04.4) version 5.4.0 20160609 (x86_64-linux-gnu)
#	compiled by GNU C version 5.4.0 20160609, GMP version 6.1.0, MPFR version 3.1.4, MPC version 1.0.3
# GGC heuristics: --param ggc-min-expand=100 --param ggc-min-heapsize=131072
# options passed:  -imultiarch x86_64-linux-gnu -D FIXED_POINT kiss_fft.c
# --param l1-cache-size=32 --param l1-cache-line-size=64
# --param l2-cache-size=3072 -mtune=ivybridge -march=x86-64
# -auxbase-strip kiss_fft_short.s -O3 -ffast-math -fomit-frame-pointer
# -fverbose-asm -fstack-protector-strong -Wformat -Wformat-security
# options enabled:  -faggressive-loop-optimizations -falign-labels
# -fassociative-math -fasynchronous-unwind-tables -fauto-inc-dec
# -fbranch-count-reg -fcaller-saves -fchkp-check-incomplete-type
# -fchkp-check-read -fchkp-check-write -fchkp-instrument-calls
# -fchkp-narrow-bounds -fchkp-optimize -fchkp-store-bounds
# -fchkp-use-static-bounds -fchkp-use-static-const-bounds
# -fchkp-use-wrappers -fcombine-stack-adjustments -fcommon -fcompare-elim
# -fcprop-registers -fcrossjumping -fcse-follow-jumps -fcx-limited-range
# -fdefer-pop -fdelete-null-pointer-checks -fdevirtualize
# -fdevirtualize-speculatively -fdwarf2-cfi-asm -fearly-inlining
# -feliminate-unused-debug-types -fexpensive-optimizations
# -ffinite-math-only -fforward-propagate -ffunction-cse -fgcse
# -fgcse-after-reload -fgcse-lm -fgnu-runtime -fgnu-unique
# -fguess-branch-probability -fhoist-adjacent-loads -fident -fif-conversion
# -fif-conversion2 -findirect-inlining -finline -finline-atomics
# -finline-functions -finline-functions-called-once
# -finline-small-functions -fipa-cp -fipa-cp-alignment -fipa-cp-clone
# -fipa-icf -fipa-icf-functions -fipa-icf-variables -fipa-profile
# -fipa-pure-const -fipa-ra -fipa-reference -fipa-sra -fira-hoist-pressure
# -fira-share-save-slots -fira-share-spill-slots
# -fisolate-erroneous-paths-dereference -fivopts -fkeep-static-consts
# -fleading-underscore -flifetime-dse -flra-remat -flto-odr-type-merging
# -fmerge-constants -fmerge-debug-strings -fmove-loop-invariants
# -fomit-frame-pointer -foptimize-sibling-calls -foptimize-strlen
# -fpartial-inlining -fpeephole -fpeephole2 -fpredictive-commoning
# -fprefetch-loop-arrays -freciprocal-math -free -freg-struct-return
# -freorder-blocks -freorder-blocks-and-partition -freorder-functions
# -frerun-cse-after-loop -fsched-critical-path-heuristic
# -fsched-dep-count-heuristic -fsched-group-heuristic -fsched-interblock
# -fsched-last-insn-heuristic -fsched-rank-heuristic -fsched-spec
# -fsched-spec-insn-heuristic -fsched-stalled-insns-dep -fschedule-fusion
# -fschedule-insns2 -fsemantic-interposition -fshow-column -fshrink-wrap
# -fsplit-ivs-in-unroller -fsplit-wide-types -fssa-phiopt
# -fstack-protector-strong -fstdarg-opt -fstrict-aliasing -fstrict-overflow
# -fstrict-volatile-bitfields -fsync-libcalls -fthread-jumps
# -ftoplevel-reorder -ftree-bit-ccp -ftree-builtin-call-dce -ftree-ccp
# -ftree-ch -ftree-coalesce-vars -ftree-copy-prop -ftree-copyrename
# -ftree-cselim -ftree-dce -ftree-dominator-opts -ftree-dse -ftree-forwprop
# -ftree-fre -ftree-loop-distribute-patterns -ftree-loop-if-convert
# -ftree-loop-im -ftree-loop-ivcanon -ftree-loop-optimize
# -ftree-loop-vectorize -ftree-parallelize-loops= -ftree-partial-pre
# -ftree-phiprop -ftree-pre -ftree-pta -ftree-reassoc -ftree-scev-cprop
# -ftree-sink -ftree-slp-vectorize -ftree-slsr -ftree-sra
# -ftree-switch-conversion -ftree-tail-merge -ftree-ter -ftree-vrp
# -funit-at-a-time -funsafe-math-optimizations -funswitch-loops
# -funwind-tables -fverbose-asm -fzero-initialized-in-bss
# -m128bit-long-double -m64 -m80387 -malign-stringops
# -mavx256-split-unaligned-load -mavx256-split-unaligned-store
# -mfancy-math-387 -mfp-ret-in-387 -mfxsr -mglibc -mlong-double-80 -mmmx
# -mno-sse4 -mpush-args -mred-zone -msse -msse2 -mtls-direct-seg-refs
# -mvzeroupper

	.section	.text.unlikely,"ax",@progbits
.LCOLDB0:
	.text
.LHOTB0:
	.p2align 4,,15
	.type	kf_work, @function
kf_work:
.LFB77:
	.cfi_startproc
# BLOCK 2 freq:44 seq:0
# PRED: ENTRY [100.0%]  (FALLTHRU)
	pushq	%r15	#
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	pushq	%r14	#
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	pushq	%r13	#
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	movq	%rdi, %r13	# Fout, Fout
	pushq	%r12	#
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	movq	%rsi, %r12	# f, f
	pushq	%rbp	#
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	movq	%r9, %rbp	# st, st
	pushq	%rbx	#
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	subq	$168, %rsp	#,
	.cfi_def_cfa_offset 224
	movl	(%r8), %eax	# *factors_10(D), p
	movl	4(%r8), %ebx	# MEM[(int *)factors_10(D) + 4B], m
	movq	%rdx, 48(%rsp)	# fstride, %sfp
	movl	%ecx, 8(%rsp)	# in_stride, %sfp
	movl	%eax, %edx	# p, D.6174
	movl	%eax, 16(%rsp)	# p, %sfp
	imull	%ebx, %edx	# m, D.6174
	movl	%ebx, 76(%rsp)	# m, %sfp
	movslq	%edx, %rdx	# D.6174, D.6175
	leaq	(%rdi,%rdx,4), %r14	#, Fout_end
	cmpl	$1, %ebx	#, m
# SUCC: 17 [28.0%]  (CAN_FALLTHRU) 3 [72.0%]  (FALLTHRU,CAN_FALLTHRU)
	je	.L59	#,
# BLOCK 3 freq:32 seq:1
# PRED: 2 [72.0%]  (FALLTHRU,CAN_FALLTHRU)
	movslq	16(%rsp), %rax	# %sfp, D.6175
	movq	%r13, %r15	# Fout, Fout
	movq	%r13, 64(%rsp)	# Fout, %sfp
	movq	%rsi, %r13	# f, f
	movq	48(%rsp), %rbx	# %sfp, fstride
	movq	%r15, %r12	# Fout, Fout
	movq	%rax, %rdi	# D.6175, D.6175
	movq	%rax, 88(%rsp)	# D.6175, %sfp
	movq	%rbx, %rax	# fstride, D.6175
	imulq	%rdi, %rax	# D.6175, D.6175
	movq	%rax, 24(%rsp)	# D.6175, %sfp
	movq	%rbx, %rax	# fstride, fstride
	salq	$2, %rax	#, D.6175
	movq	%rax, %rbx	# D.6175, D.6175
	movq	%rax, 40(%rsp)	# D.6175, %sfp
	movslq	8(%rsp), %rax	# %sfp, D.6175
	imulq	%rbx, %rax	# D.6175, D.6175
	movq	%rax, 32(%rsp)	# D.6175, %sfp
	movslq	76(%rsp), %rax	# %sfp, D.6175
	movq	%rax, 56(%rsp)	# D.6175, %sfp
	leaq	0(,%rax,4), %rbx	#, D.6175
	movq	%rax, 80(%rsp)	# D.6175, %sfp
	leaq	8(%r8), %rax	#, factors
# SUCC: 4 [100.0%]  (FALLTHRU,CAN_FALLTHRU)
	movq	%rax, %r15	# factors, factors
# BLOCK 4 freq:352 seq:2
# PRED: 3 [100.0%]  (FALLTHRU,CAN_FALLTHRU) 4 [91.0%]  (DFS_BACK,CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L5:
	movl	8(%rsp), %ecx	# %sfp,
	movq	%r13, %rsi	# f,
	movq	%r12, %rdi	# Fout,
	movq	%rbp, %r9	# st,
	movq	24(%rsp), %rdx	# %sfp,
	movq	%r15, %r8	# factors,
	addq	%rbx, %r12	# D.6175, Fout
	call	kf_work	#
	addq	32(%rsp), %r13	# %sfp, f
	cmpq	%r12, %r14	# Fout, Fout_end
# SUCC: 4 [91.0%]  (DFS_BACK,CAN_FALLTHRU) 5 [9.0%]  (FALLTHRU,CAN_FALLTHRU,LOOP_EXIT)
	jne	.L5	#,
# BLOCK 5 freq:32 seq:3
# PRED: 4 [9.0%]  (FALLTHRU,CAN_FALLTHRU,LOOP_EXIT)
	cmpl	$3, 16(%rsp)	#, %sfp
	movq	64(%rsp), %r13	# %sfp, Fout
# SUCC: 45 [20.0%]  (CAN_FALLTHRU) 6 [80.0%]  (FALLTHRU,CAN_FALLTHRU)
	je	.L31	#,
# BLOCK 6 freq:26 seq:4
# PRED: 5 [80.0%]  (FALLTHRU,CAN_FALLTHRU)
# SUCC: 7 [62.5%]  (FALLTHRU,CAN_FALLTHRU) 13 [37.5%]  (CAN_FALLTHRU)
	jle	.L60	#,
# BLOCK 7 freq:16 seq:5
# PRED: 6 [62.5%]  (FALLTHRU,CAN_FALLTHRU)
	movl	16(%rsp), %eax	# %sfp, p
	cmpl	$4, %eax	#, p
# SUCC: 39 [40.0%]  (CAN_FALLTHRU) 8 [60.0%]  (FALLTHRU,CAN_FALLTHRU)
	je	.L32	#,
# BLOCK 8 freq:10 seq:6
# PRED: 7 [60.0%]  (FALLTHRU,CAN_FALLTHRU)
	cmpl	$5, %eax	#, p
# SUCC: 9 [66.7%]  (FALLTHRU,CAN_FALLTHRU) 26 [33.3%]  (CAN_FALLTHRU)
	jne	.L6	#,
# BLOCK 9 freq:9 seq:7
# PRED: 22 [66.7%]  (CAN_FALLTHRU) 8 [66.7%]  (FALLTHRU,CAN_FALLTHRU)
.L11:
	leaq	264(%rbp), %rax	#, twiddles
	addq	%r13, %rbx	# Fout, Fout1
	movq	%rbx, %r10	# Fout1, Fout1
	movl	76(%rsp), %ebx	# %sfp, m
	movq	%rax, %r11	# twiddles, twiddles
	movq	%rax, 136(%rsp)	# twiddles, %sfp
	movq	48(%rsp), %rax	# %sfp, D.6175
	imulq	80(%rsp), %rax	# %sfp, D.6175
	leal	(%rbx,%rbx), %edi	#, D.6174
	movslq	%edi, %r8	# D.6174, D.6175
	addl	%ebx, %edi	# m, D.6174
	leaq	0(%r13,%r8,4), %r15	#, Fout2
	movslq	%edi, %rdi	# D.6174, D.6175
	leaq	0(%r13,%rdi,4), %rdi	#, Fout3
	salq	$2, %rax	#, D.6175
	leaq	(%r11,%rax), %rcx	#, D.6178
	movq	%rdi, %r14	# Fout3, Fout3
	leal	0(,%rbx,4), %edi	#, D.6174
	addq	%rcx, %rax	# D.6178, D.6178
	movswl	(%rcx), %esi	# MEM[(struct kiss_fft_cpx *)_455],
	movslq	%edi, %rdi	# D.6174, D.6175
	movzwl	2(%rcx), %edx	# MEM[(struct kiss_fft_cpx *)_455 + 2B], ya$i
	leaq	0(%r13,%rdi,4), %rdi	#, Fout4
	movzwl	(%rax), %ecx	# MEM[(struct kiss_fft_cpx *)_459], yb$r
	movswl	2(%rax), %eax	# MEM[(struct kiss_fft_cpx *)_459 + 2B],
	testl	%ebx, %ebx	# m
# SUCC: 10 [91.0%]  (FALLTHRU,CAN_FALLTHRU) 12 [9.0%]  (CAN_FALLTHRU)
	jle	.L1	#,
# BLOCK 10 freq:8 seq:8
# PRED: 9 [91.0%]  (FALLTHRU,CAN_FALLTHRU)
	movl	%eax, 64(%rsp)	# D.6174, %sfp
	movq	40(%rsp), %rax	# %sfp, D.6175
	xorl	%r12d, %r12d	# ivtmp.197
	movl	%esi, 32(%rsp)	# D.6174, %sfp
	movswl	%cx, %esi	# yb$r, D.6174
	movl	%esi, 48(%rsp)	# D.6174, %sfp
	movswl	%dx, %esi	# ya$i, D.6174
	movq	%r10, %rdx	# Fout1, Fout1
	movl	%esi, 56(%rsp)	# D.6174, %sfp
	leaq	(%rax,%rax,2), %rax	#, tmp1120
	movq	%r11, 24(%rsp)	# twiddles, %sfp
	movq	%rax, 152(%rsp)	# tmp1120, %sfp
	leal	-1(%rbx), %eax	#, D.6182
	leaq	4(%r15,%rax,4), %rax	#, D.6178
	movq	%r12, 8(%rsp)	# ivtmp.197, %sfp
	movq	%rax, 144(%rsp)	# D.6178, %sfp
	movq	%r15, %rax	# Fout2, Fout2
# SUCC: 11 [100.0%]  (FALLTHRU,CAN_FALLTHRU)
	movq	%rdi, %r15	# Fout4, Fout4
# BLOCK 11 freq:89 seq:9
# PRED: 10 [100.0%]  (FALLTHRU,CAN_FALLTHRU) 11 [91.0%]  (DFS_BACK,CAN_FALLTHRU)
.L19:
	movswl	0(%r13), %ecx	# MEM[base: Fout_241, offset: 0B], D.6174
	movq	8(%rsp), %r10	# %sfp, ivtmp.197
	imull	$6553, %ecx, %ecx	#, D.6174, D.6174
	addl	$16384, %ecx	#, D.6174
	sarl	$15, %ecx	#, D.6174
	movw	%cx, 0(%r13)	# D.6174, MEM[base: Fout_241, offset: 0B]
	movswl	2(%r13), %ecx	# MEM[base: Fout_241, offset: 2B], D.6174
	imull	$6553, %ecx, %ecx	#, D.6174, D.6174
	addl	$16384, %ecx	#, D.6174
	sarl	$15, %ecx	#, D.6174
	movw	%cx, 2(%r13)	# D.6174, MEM[base: Fout_241, offset: 2B]
	movswl	(%rdx), %ecx	# MEM[base: Fout1_246, offset: 0B], D.6174
	imull	$6553, %ecx, %ecx	#, D.6174, D.6174
	addl	$16384, %ecx	#, D.6174
	sarl	$15, %ecx	#, D.6174
	movw	%cx, (%rdx)	# D.6174, MEM[base: Fout1_246, offset: 0B]
	movswl	2(%rdx), %ecx	# MEM[base: Fout1_246, offset: 2B], D.6174
	imull	$6553, %ecx, %ecx	#, D.6174, D.6174
	addl	$16384, %ecx	#, D.6174
	sarl	$15, %ecx	#, D.6174
	movw	%cx, 2(%rdx)	# D.6174, MEM[base: Fout1_246, offset: 2B]
	movswl	(%rax), %ecx	# MEM[base: Fout2_92, offset: 0B], D.6174
	imull	$6553, %ecx, %ecx	#, D.6174, D.6174
	addl	$16384, %ecx	#, D.6174
	sarl	$15, %ecx	#, D.6174
	movw	%cx, (%rax)	# D.6174, MEM[base: Fout2_92, offset: 0B]
	movswl	2(%rax), %ecx	# MEM[base: Fout2_92, offset: 2B], D.6174
	imull	$6553, %ecx, %ecx	#, D.6174, D.6174
	addl	$16384, %ecx	#, D.6174
	sarl	$15, %ecx	#, D.6174
	movw	%cx, 2(%rax)	# D.6174, MEM[base: Fout2_92, offset: 2B]
	movswl	(%r14), %ecx	# MEM[base: Fout3_98, offset: 0B], D.6174
	imull	$6553, %ecx, %ecx	#, D.6174, D.6174
	addl	$16384, %ecx	#, D.6174
	sarl	$15, %ecx	#, D.6174
	movw	%cx, (%r14)	# D.6174, MEM[base: Fout3_98, offset: 0B]
	movswl	2(%r14), %ecx	# MEM[base: Fout3_98, offset: 2B], D.6174
	imull	$6553, %ecx, %ecx	#, D.6174, D.6174
	addl	$16384, %ecx	#, D.6174
	sarl	$15, %ecx	#, D.6174
	movw	%cx, 2(%r14)	# D.6174, MEM[base: Fout3_98, offset: 2B]
	movswl	(%r15), %esi	# MEM[base: Fout4_561, offset: 0B], D.6174
	movswl	2(%r15), %ecx	# MEM[base: Fout4_561, offset: 2B], D.6174
	imull	$6553, %esi, %esi	#, D.6174, D.6174
	imull	$6553, %ecx, %ecx	#, D.6174, D.6174
	addl	$16384, %esi	#, D.6174
	addl	$16384, %ecx	#, D.6174
	sarl	$15, %esi	#, D.6174
	sarl	$15, %ecx	#, D.6174
	movw	%si, (%r15)	# D.6174, MEM[base: Fout4_561, offset: 0B]
	movw	%cx, 2(%r15)	# D.6174, MEM[base: Fout4_561, offset: 2B]
	movswl	(%rdx), %edi	# MEM[base: Fout1_246, offset: 0B], D.6174
	movzwl	2(%r13), %ebx	# MEM[base: Fout_241, offset: 2B], scratch$0$i
	movl	%edi, 76(%rsp)	# D.6174, %sfp
	movq	136(%rsp), %rdi	# %sfp, twiddles
	movw	%bx, 16(%rsp)	# scratch$0$i, %sfp
	movswl	2(%rdx), %ebx	# MEM[base: Fout1_246, offset: 2B], D.6174
	movswl	(%rdi,%r10), %r11d	# MEM[base: twiddles_46, index: ivtmp.197_320, offset: 0B], D.6174
	movswl	2(%rdi,%r10), %ebp	# MEM[base: twiddles_46, index: ivtmp.197_320, offset: 2B], D.6174
	movl	%r11d, %r12d	# D.6174, D.6174
	movswl	(%rax), %r11d	# MEM[base: Fout2_92, offset: 0B], D.6174
	movl	%r11d, 120(%rsp)	# D.6174, %sfp
	movq	%r10, %r11	# ivtmp.197, ivtmp.197
	movswl	(%rdi,%r10,2), %r10d	# MEM[base: twiddles_46, index: ivtmp.197_320, step: 2, offset: 0B], D.6174
	movl	%r10d, 124(%rsp)	# D.6174, %sfp
	movswl	2(%rax), %r9d	# MEM[base: Fout2_92, offset: 2B], D.6174
	movswl	(%r14), %r8d	# MEM[base: Fout3_98, offset: 0B], D.6174
	movl	%r12d, 116(%rsp)	# D.6174, %sfp
	movl	%r9d, 80(%rsp)	# D.6174, %sfp
	movswl	2(%rdi,%r11,2), %r9d	# MEM[base: twiddles_46, index: ivtmp.197_320, step: 2, offset: 2B], D.6174
	movl	%r8d, 88(%rsp)	# D.6174, %sfp
	movq	24(%rsp), %r11	# %sfp, ivtmp.202
	movswl	2(%r14), %r8d	# MEM[base: Fout3_98, offset: 2B], D.6174
	movswl	(%r11), %r10d	# MEM[base: _1111, offset: 0B], D.6174
	movl	%r8d, 104(%rsp)	# D.6174, %sfp
	movswl	2(%r11), %r8d	# MEM[base: _1111, offset: 2B], D.6174
	movq	8(%rsp), %r11	# %sfp, ivtmp.197
	movl	%r10d, 96(%rsp)	# D.6174, %sfp
	movswl	(%rdi,%r11,4), %r10d	# MEM[base: twiddles_46, index: ivtmp.197_320, step: 4, offset: 0B], D.6174
	movswl	2(%rdi,%r11,4), %r11d	# MEM[base: twiddles_46, index: ivtmp.197_320, step: 4, offset: 2B], D.6174
	movl	76(%rsp), %edi	# %sfp, D.6174
	imull	%r12d, %edi	# D.6174, D.6174
	movl	%ebx, %r12d	# D.6174, D.6174
	imull	%ebp, %r12d	# D.6174, D.6174
	subl	%r12d, %edi	# D.6174, D.6174
	movl	%esi, %r12d	# D.6174, D.6174
	imull	%r10d, %r12d	# D.6174, D.6174
	addl	$16384, %edi	#, D.6174
	sarl	$15, %edi	#, D.6174
	imull	%r11d, %esi	# D.6174, D.6174
	imull	76(%rsp), %ebp	# %sfp, D.6174
	movl	%edi, 112(%rsp)	# D.6174, %sfp
	imull	116(%rsp), %ebx	# %sfp, D.6174
	movl	%r12d, %edi	# D.6174, D.6174
	movl	%ecx, %r12d	# D.6174, D.6174
	imull	%r11d, %r12d	# D.6174, D.6174
	movl	124(%rsp), %r11d	# %sfp, D.6174
	imull	%r10d, %ecx	# D.6174, D.6174
	leal	16384(%rbp,%rbx), %ebx	#, D.6174
	subl	%r12d, %edi	# D.6174, D.6174
	movl	%ebx, %ebp	# D.6174, D.6174
	movl	104(%rsp), %ebx	# %sfp, D.6174
	leal	16384(%rsi,%rcx), %ecx	#, D.6174
	addl	$16384, %edi	#, D.6174
	sarl	$15, %ebp	#, D.6174
	movl	80(%rsp), %esi	# %sfp, D.6174
	sarl	$15, %edi	#, D.6174
	movl	%ebp, 76(%rsp)	# D.6174, %sfp
	movl	120(%rsp), %ebp	# %sfp, D.6174
	sarl	$15, %ecx	#, D.6174
	movl	%edi, %r12d	# D.6174, D.6176
	addw	112(%rsp), %r12w	# %sfp, D.6176
	movl	%edi, 128(%rsp)	# D.6174, %sfp
	movl	%ecx, 132(%rsp)	# D.6174, %sfp
	imull	%r8d, %ebx	# D.6174, D.6174
	imull	%r9d, %esi	# D.6174, D.6174
	imull	%ebp, %r9d	# D.6174, D.6174
	movswl	%r12w, %edi	# D.6176,
	movl	%ecx, %r12d	# D.6174, D.6176
	movl	%ebp, %ecx	# D.6174, D.6174
	addw	76(%rsp), %r12w	# %sfp, D.6176
	imull	%r11d, %ecx	# D.6174, D.6174
	subl	%esi, %ecx	# D.6174, D.6174
	movl	%r12d, %r10d	# D.6176, D.6176
	addl	$16384, %ecx	#, D.6174
	sarl	$15, %ecx	#, D.6174
	movl	%ecx, 116(%rsp)	# D.6174, %sfp
	movl	88(%rsp), %ecx	# %sfp, D.6174
	imull	96(%rsp), %ecx	# %sfp, D.6174
	subl	%ebx, %ecx	# D.6174, D.6174
	addl	$16384, %ecx	#, D.6174
	sarl	$15, %ecx	#, D.6174
	movl	%ecx, 120(%rsp)	# D.6174, %sfp
	movl	%ecx, %r12d	# D.6174, D.6176
	addw	116(%rsp), %r12w	# %sfp, D.6176
	movl	80(%rsp), %ecx	# %sfp, D.6174
	imull	88(%rsp), %r8d	# %sfp, D.6174
	movl	96(%rsp), %esi	# %sfp, D.6174
	imull	104(%rsp), %esi	# %sfp, D.6174
	movl	%r12d, %ebx	# D.6176, D.6176
	movswl	%r12w, %ebp	# D.6176, D.6174
	imull	%r11d, %ecx	# D.6174, D.6174
	movzwl	0(%r13), %r11d	# MEM[base: Fout_241, offset: 0B], D.6176
	movl	32(%rsp), %r12d	# %sfp, D.6174
	leal	16384(%r8,%rsi), %esi	#, D.6174
	leal	16384(%r9,%rcx), %ecx	#, D.6174
	leal	(%rbx,%r11), %r8d	#, D.6176
	sarl	$15, %ecx	#, D.6174
	movl	48(%rsp), %ebx	# %sfp, D.6174
	sarl	$15, %esi	#, D.6174
	addl	%edi, %r8d	# D.6176, tmp1204
	movw	%r8w, 0(%r13)	# tmp1204, MEM[base: Fout_241, offset: 0B]
	movzwl	16(%rsp), %r8d	# %sfp, scratch$0$i
	leal	(%rsi,%rcx), %r9d	#, D.6176
	subl	%esi, %ecx	# D.6174, D.6176
	movswl	%cx, %ecx	# D.6176, D.6174
	addl	%r9d, %r8d	# D.6176, D.6176
	addl	%r10d, %r8d	# D.6176, tmp1206
	movw	%r8w, 2(%r13)	# tmp1206, MEM[base: Fout_241, offset: 2B]
	movl	%edi, %r8d	# D.6174, D.6174
	movl	%ebx, %edi	# D.6174, D.6174
	imull	%ebp, %edi	# D.6174, D.6174
	movl	%r8d, 80(%rsp)	# D.6174, %sfp
	imull	%r12d, %r8d	# D.6174, D.6174
	addl	$16384, %edi	#, D.6174
	sarl	$15, %edi	#, D.6174
	addl	$16384, %r8d	#, D.6174
	addl	%r11d, %edi	# D.6176, D.6176
	sarl	$15, %r8d	#, D.6174
	addl	%r8d, %edi	# D.6174, D.6176
	movw	%di, 88(%rsp)	# D.6176, %sfp
	movswl	%r10w, %edi	# D.6176, D.6174
	movl	%edi, %r10d	# D.6174, D.6174
	movswl	%r9w, %edi	# D.6176, D.6174
	movl	%edi, 104(%rsp)	# D.6174, %sfp
	imull	%ebx, %edi	# D.6174, D.6174
	movl	%r10d, %r9d	# D.6174, D.6174
	movl	56(%rsp), %ebx	# %sfp, D.6174
	imull	%r12d, %r9d	# D.6174, D.6174
	movzwl	76(%rsp), %r12d	# %sfp, D.6176
	movl	%r10d, 96(%rsp)	# D.6174, %sfp
	subw	132(%rsp), %r12w	# %sfp, D.6176
	addl	$16384, %edi	#, D.6174
	sarl	$15, %edi	#, D.6174
	addw	16(%rsp), %di	# %sfp, D.6176
	addl	$16384, %r9d	#, D.6174
	sarl	$15, %r9d	#, D.6174
	movswl	%r12w, %r8d	# D.6176,
	movzwl	112(%rsp), %r12d	# %sfp, D.6176
	subw	128(%rsp), %r12w	# %sfp, D.6176
	movl	%r8d, 76(%rsp)	# D.6174, %sfp
	addl	%edi, %r9d	# D.6176, D.6176
	movl	%r8d, %edi	# D.6174, D.6174
	imull	%ebx, %edi	# D.6174, D.6174
	leal	16384(%rdi), %r10d	#, D.6174
	movl	%r10d, %esi	# D.6174, D.6174
	sarl	$15, %esi	#, D.6174
	movl	%esi, %edi	# D.6174, D.6174
	movl	64(%rsp), %esi	# %sfp, D.6174
	imull	%ecx, %esi	# D.6174, D.6174
	leal	16384(%rsi), %r10d	#, D.6174
	movl	%r10d, %esi	# D.6174, D.6174
	sarl	$15, %esi	#, D.6174
	leal	(%rdi,%rsi), %r10d	#, D.6176
	movswl	%r12w, %esi	# D.6176,
	movl	%ebx, %edi	# D.6174, D.6174
	movzwl	116(%rsp), %r12d	# %sfp, D.6176
	imull	%esi, %edi	# D.6174, D.6174
	subw	120(%rsp), %r12w	# %sfp, D.6176
	addl	$16384, %edi	#, D.6174
	movswl	%r12w, %r8d	# D.6176,
	movl	%edi, %r12d	# D.6174, D.6174
	movl	64(%rsp), %edi	# %sfp, D.6174
	sarl	$15, %r12d	#, D.6174
	imull	%r8d, %edi	# D.6174, D.6174
	addl	$16384, %edi	#, D.6174
	sarl	$15, %edi	#, D.6174
	addl	%r12d, %edi	# D.6174, D.6176
	movzwl	88(%rsp), %r12d	# %sfp, D.6176
	movl	%r12d, %ebx	# D.6176, tmp1237
	subl	%r10d, %ebx	# D.6176, tmp1237
	movw	%bx, (%rdx)	# tmp1237, MEM[base: Fout1_246, offset: 0B]
	leal	(%rdi,%r9), %ebx	#, tmp1238
	subl	%edi, %r9d	# D.6176, tmp1240
	movw	%bx, 2(%rdx)	# tmp1238, MEM[base: Fout1_246, offset: 2B]
	movl	%r12d, %ebx	# D.6176, D.6176
	movl	%ebp, %r12d	# D.6174, D.6174
	movw	%r9w, 2(%r15)	# tmp1240, MEM[base: Fout4_561, offset: 2B]
	movl	32(%rsp), %r9d	# %sfp, D.6174
	addl	%ebx, %r10d	# D.6176, tmp1239
	movw	%r10w, (%r15)	# tmp1239, MEM[base: Fout4_561, offset: 0B]
	movl	80(%rsp), %r10d	# %sfp, D.6174
	movl	48(%rsp), %ebx	# %sfp, D.6174
	movl	104(%rsp), %ebp	# %sfp, D.6174
	imull	%r9d, %r12d	# D.6174, D.6174
	imull	%ebx, %r10d	# D.6174, D.6174
	leal	16384(%r12), %edi	#, D.6174
	imull	%r9d, %ebp	# D.6174, D.6174
	sarl	$15, %edi	#, D.6174
	addl	%edi, %r11d	# D.6174, D.6176
	movl	%r10d, %edi	# D.6174, D.6174
	movl	96(%rsp), %r10d	# %sfp, D.6174
	addl	$16384, %edi	#, D.6174
	movl	%ebp, %r9d	# D.6174, D.6174
	movl	56(%rsp), %ebp	# %sfp, D.6174
	sarl	$15, %edi	#, D.6174
	addl	$16384, %r9d	#, D.6174
	addl	%r11d, %edi	# D.6176, D.6176
	movl	64(%rsp), %r11d	# %sfp, D.6174
	sarl	$15, %r9d	#, D.6174
	imull	%ebx, %r10d	# D.6174, D.6174
	movl	76(%rsp), %ebx	# %sfp, D.6174
	addw	16(%rsp), %r9w	# %sfp, D.6176
	imull	%ebp, %ecx	# D.6174, D.6174
	imull	%ebp, %r8d	# D.6174, D.6174
	addl	$16384, %r10d	#, D.6174
	imull	%r11d, %esi	# D.6174, D.6174
	imull	%r11d, %ebx	# D.6174, D.6174
	sarl	$15, %r10d	#, D.6174
	addl	$16384, %ecx	#, D.6174
	addl	%r10d, %r9d	# D.6174, D.6176
	sarl	$15, %ecx	#, D.6174
	addl	$16384, %r8d	#, D.6174
	addl	$16384, %esi	#, D.6174
	leal	16384(%rbx), %r10d	#, D.6174
	sarl	$15, %esi	#, D.6174
	movq	152(%rsp), %rbx	# %sfp, tmp1120
	sarl	$15, %r10d	#, D.6174
	subl	%r10d, %ecx	# D.6174, D.6176
	sarl	$15, %r8d	#, D.6174
	addq	$4, %r13	#, Fout
	subl	%r8d, %esi	# D.6174, D.6176
	addq	$4, %rdx	#, Fout1
	addq	$4, %rax	#, Fout2
	leal	(%rdi,%rcx), %r8d	#, tmp1267
	subl	%ecx, %edi	# D.6176, tmp1269
	addq	$4, %r14	#, Fout3
	addq	%rbx, 24(%rsp)	# tmp1120, %sfp
	movw	%r8w, -4(%rax)	# tmp1267, MEM[base: Fout2_92, offset: 0B]
	leal	(%r9,%rsi), %r8d	#, tmp1268
	subl	%esi, %r9d	# D.6176, tmp1270
	movw	%r8w, -2(%rax)	# tmp1268, MEM[base: Fout2_92, offset: 2B]
	movq	40(%rsp), %rsi	# %sfp, D.6175
	addq	$4, %r15	#, Fout4
	addq	%rsi, 8(%rsp)	# D.6175, %sfp
	movw	%di, -4(%r14)	# tmp1269, MEM[base: Fout3_98, offset: 0B]
	movw	%r9w, -2(%r14)	# tmp1270, MEM[base: Fout3_98, offset: 2B]
	cmpq	144(%rsp), %rax	# %sfp, Fout2
# SUCC: 11 [91.0%]  (DFS_BACK,CAN_FALLTHRU) 12 [9.0%]  (FALLTHRU,CAN_FALLTHRU,LOOP_EXIT)
	jne	.L19	#,
# BLOCK 12 freq:35 seq:10
# PRED: 16 [100.0%]  (CAN_FALLTHRU,LOOP_EXIT) 9 [9.0%]  (CAN_FALLTHRU) 42 [9.0%]  (CAN_FALLTHRU,LOOP_EXIT) 11 [9.0%]  (FALLTHRU,CAN_FALLTHRU,LOOP_EXIT) 48 [100.0%]  (CAN_FALLTHRU,LOOP_EXIT)
.L1:
	addq	$168, %rsp	#,
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	popq	%rbx	#
	.cfi_def_cfa_offset 48
	popq	%rbp	#
	.cfi_def_cfa_offset 40
	popq	%r12	#
	.cfi_def_cfa_offset 32
	popq	%r13	#
	.cfi_def_cfa_offset 24
	popq	%r14	#
	.cfi_def_cfa_offset 16
	popq	%r15	#
	.cfi_def_cfa_offset 8
# SUCC: EXIT [100.0%] 
	ret
# BLOCK 13 freq:10 seq:11
# PRED: 6 [37.5%]  (CAN_FALLTHRU)
.L60:
	.cfi_restore_state
	cmpl	$2, 16(%rsp)	#, %sfp
# SUCC: 14 [66.7%]  (FALLTHRU,CAN_FALLTHRU) 26 [33.3%]  (CAN_FALLTHRU)
	jne	.L6	#,
# BLOCK 14 freq:9 seq:12
# PRED: 24 [66.7%]  (CAN_FALLTHRU) 13 [66.7%]  (FALLTHRU,CAN_FALLTHRU)
.L9:
	movl	76(%rsp), %eax	# %sfp, m
	addq	$264, %rbp	#, tw1
	addq	%r13, %rbx	# Fout, Fout2
	movq	40(%rsp), %rcx	# %sfp, D.6175
	subl	$1, %eax	#, D.6182
# SUCC: 15 [100.0%]  (FALLTHRU,CAN_FALLTHRU)
	leaq	4(%r13,%rax,4), %rsi	#, D.6178
# BLOCK 15 freq:98 seq:13
# PRED: 14 [100.0%]  (FALLTHRU,CAN_FALLTHRU) 15 [91.0%]  (DFS_BACK,CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L12:
	movswl	0(%r13), %edx	# MEM[base: Fout_54, offset: 0B], D.6174
	addq	$4, %rbx	#, Fout2
	movl	%edx, %eax	# D.6174, tmp902
	sall	$14, %eax	#, tmp902
	subl	%edx, %eax	# D.6174, D.6174
	movswl	2(%r13), %edx	# MEM[base: Fout_54, offset: 2B], D.6174
	addl	$16384, %eax	#, D.6174
	sarl	$15, %eax	#, D.6174
	movw	%ax, 0(%r13)	# D.6174, MEM[base: Fout_54, offset: 0B]
	movl	%edx, %eax	# D.6174, tmp908
	sall	$14, %eax	#, tmp908
	subl	%edx, %eax	# D.6174, D.6174
	addl	$16384, %eax	#, D.6174
	sarl	$15, %eax	#, D.6174
	movw	%ax, 2(%r13)	# D.6174, MEM[base: Fout_54, offset: 2B]
	movswl	-4(%rbx), %eax	# MEM[base: Fout2_67, offset: 0B], D.6174
	movswl	-2(%rbx), %edx	# MEM[base: Fout2_67, offset: 2B], D.6174
	movl	%eax, %edi	# D.6174, tmp914
	sall	$14, %edi	#, tmp914
	subl	%eax, %edi	# D.6174, D.6174
	movl	%edx, %eax	# D.6174, tmp919
	sall	$14, %eax	#, tmp919
	addl	$16384, %edi	#, D.6174
	subl	%edx, %eax	# D.6174, D.6174
	sarl	$15, %edi	#, D.6174
	addl	$16384, %eax	#, D.6174
	movw	%di, -4(%rbx)	# D.6174, MEM[base: Fout2_67, offset: 0B]
	movl	%edi, %edx	# D.6174, D.6174
	sarl	$15, %eax	#, D.6174
	movw	%ax, -2(%rbx)	# D.6174, MEM[base: Fout2_67, offset: 2B]
	movswl	0(%rbp), %r8d	# MEM[base: tw1_81, offset: 0B], D.6174
	movl	%eax, %r10d	# D.6174, D.6174
	movswl	2(%rbp), %r9d	# MEM[base: tw1_81, offset: 2B], D.6174
	addq	%rcx, %rbp	# D.6175, tw1
	imull	%r8d, %edx	# D.6174, D.6174
	imull	%r9d, %r10d	# D.6174, D.6174
	imull	%r9d, %edi	# D.6174, D.6174
	imull	%r8d, %eax	# D.6174, D.6174
	subl	%r10d, %edx	# D.6174, D.6174
	movzwl	0(%r13), %r10d	# MEM[base: Fout_54, offset: 0B], D.6176
	addl	$16384, %edx	#, D.6174
	leal	16384(%rdi,%rax), %eax	#, D.6174
	sarl	$15, %edx	#, D.6174
	sarl	$15, %eax	#, D.6174
	subl	%edx, %r10d	# D.6174, D.6176
	movw	%r10w, -4(%rbx)	# D.6176, MEM[base: Fout2_67, offset: 0B]
	movzwl	2(%r13), %edi	# MEM[base: Fout_54, offset: 2B], D.6176
	subl	%eax, %edi	# D.6174, D.6176
	movw	%di, -2(%rbx)	# D.6176, MEM[base: Fout2_67, offset: 2B]
	addw	%dx, 0(%r13)	# D.6174, MEM[base: Fout_54, offset: 0B]
	addw	%ax, 2(%r13)	# D.6174, MEM[base: Fout_54, offset: 2B]
	addq	$4, %r13	#, Fout
	cmpq	%rsi, %r13	# D.6178, Fout
# SUCC: 15 [91.0%]  (DFS_BACK,CAN_FALLTHRU) 16 [9.0%]  (FALLTHRU)
	jne	.L12	#,
# BLOCK 16 freq:9 seq:14
# PRED: 15 [9.0%]  (FALLTHRU)
# SUCC: 12 [100.0%]  (CAN_FALLTHRU,LOOP_EXIT)
	jmp	.L1	#
# BLOCK 17 freq:12 seq:15
# PRED: 2 [28.0%]  (CAN_FALLTHRU)
.L59:
	movq	48(%rsp), %rax	# %sfp, fstride
	movslq	%ecx, %rsi	#, D.6175
	movq	%rdi, %rdx	# Fout, Fout
	salq	$2, %rax	#, D.6175
	imulq	%rax, %rsi	# D.6175, D.6175
# SUCC: 18 [100.0%]  (FALLTHRU,CAN_FALLTHRU)
	movq	%rax, 40(%rsp)	# D.6175, %sfp
# BLOCK 18 freq:137 seq:16
# PRED: 17 [100.0%]  (FALLTHRU,CAN_FALLTHRU) 18 [91.0%]  (DFS_BACK,CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L3:
	movl	(%r12), %eax	# MEM[base: f_3, offset: 0B], MEM[base: f_3, offset: 0B]
	addq	$4, %rdx	#, Fout
	addq	%rsi, %r12	# D.6175, f
	movl	%eax, -4(%rdx)	# MEM[base: f_3, offset: 0B], MEM[base: Fout_1, offset: 0B]
	cmpq	%rdx, %r14	# Fout, Fout_end
# SUCC: 18 [91.0%]  (DFS_BACK,CAN_FALLTHRU) 19 [9.0%]  (FALLTHRU,CAN_FALLTHRU,LOOP_EXIT)
	jne	.L3	#,
# BLOCK 19 freq:12 seq:17
# PRED: 18 [9.0%]  (FALLTHRU,CAN_FALLTHRU,LOOP_EXIT)
	cmpl	$3, 16(%rsp)	#, %sfp
# SUCC: 49 [20.0%]  (CAN_FALLTHRU) 20 [80.0%]  (FALLTHRU,CAN_FALLTHRU)
	je	.L34	#,
# BLOCK 20 freq:10 seq:18
# PRED: 19 [80.0%]  (FALLTHRU,CAN_FALLTHRU)
# SUCC: 21 [62.5%]  (FALLTHRU,CAN_FALLTHRU) 24 [37.5%]  (CAN_FALLTHRU)
	jle	.L61	#,
# BLOCK 21 freq:6 seq:19
# PRED: 20 [62.5%]  (FALLTHRU,CAN_FALLTHRU)
	movl	16(%rsp), %eax	# %sfp, p
	movl	$4, %ebx	#, D.6175
	movq	$1, 80(%rsp)	#, %sfp
	cmpl	$4, %eax	#, p
# SUCC: 40 [40.0%]  (CAN_FALLTHRU) 22 [60.0%]  (FALLTHRU,CAN_FALLTHRU)
	je	.L10	#,
# BLOCK 22 freq:4 seq:20
# PRED: 21 [60.0%]  (FALLTHRU,CAN_FALLTHRU)
	cmpl	$5, %eax	#, p
# SUCC: 9 [66.7%]  (CAN_FALLTHRU) 23 [33.3%]  (FALLTHRU,CAN_FALLTHRU)
	je	.L11	#,
# BLOCK 23 freq:2 seq:21
# PRED: 22 [33.3%]  (FALLTHRU,CAN_FALLTHRU) 25 [100.0%]  (CAN_FALLTHRU)
.L29:
	movslq	16(%rsp), %rdi	# %sfp, D.6175
	leaq	264(%rbp), %r15	#, twiddles
	movl	0(%rbp), %ebp	# st_32(D)->nfft, Norig
	salq	$2, %rdi	#, D.6175
	call	malloc	#
	movq	$1, 56(%rsp)	#, %sfp
	movq	%rax, 24(%rsp)	#, %sfp
# SUCC: 27 [100.0%]  (CAN_FALLTHRU)
	jmp	.L20	#
# BLOCK 24 freq:4 seq:22
# PRED: 20 [37.5%]  (CAN_FALLTHRU)
.L61:
	cmpl	$2, 16(%rsp)	#, %sfp
	movl	$4, %ebx	#, D.6175
# SUCC: 14 [66.7%]  (CAN_FALLTHRU) 25 [33.3%]  (FALLTHRU)
	je	.L9	#,
# BLOCK 25 freq:1 seq:23
# PRED: 24 [33.3%]  (FALLTHRU)
# SUCC: 23 [100.0%]  (CAN_FALLTHRU)
	jmp	.L29	#
# BLOCK 26 freq:7 seq:24
# PRED: 13 [33.3%]  (CAN_FALLTHRU) 8 [33.3%]  (CAN_FALLTHRU)
.L6:
	movq	88(%rsp), %rdi	# %sfp, D.6175
	leaq	264(%rbp), %r15	#, twiddles
	movl	0(%rbp), %ebp	# st_32(D)->nfft, Norig
	salq	$2, %rdi	#, D.6175
	call	malloc	#
	movl	76(%rsp), %edx	# %sfp,
	movq	%rax, 24(%rsp)	#, %sfp
	movq	%rax, %rdi	#,
	testl	%edx, %edx	#
# SUCC: 38 [12.4%]  (CAN_FALLTHRU) 27 [87.6%]  (FALLTHRU,CAN_FALLTHRU)
	jle	.L21	#,
# BLOCK 27 freq:8 seq:25
# PRED: 26 [87.6%]  (FALLTHRU,CAN_FALLTHRU) 23 [100.0%]  (CAN_FALLTHRU)
.L20:
	movq	56(%rsp), %rax	# %sfp, D.6175
	movq	%r13, 64(%rsp)	# Fout, %sfp
	movl	16(%rsp), %ebx	# %sfp, p
	movl	$0, 80(%rsp)	#, %sfp
	movq	24(%rsp), %rdi	# %sfp, scratch
	movl	$0, 56(%rsp)	#, %sfp
	movq	48(%rsp), %rsi	# %sfp, fstride
	salq	$2, %rax	#, D.6175
	movq	%rax, 32(%rsp)	# D.6175, %sfp
	movl	%ebx, %eax	# p, p
	subl	$1, %eax	#, D.6182
	leaq	4(%rdi,%rax,4), %rax	#, D.6183
	movl	%esi, 88(%rsp)	# tmp1629, %sfp
	movq	%rax, 96(%rsp)	# D.6183, %sfp
	movl	76(%rsp), %eax	# %sfp, D.6179
	imull	%esi, %eax	# fstride, D.6179
	movl	%eax, 48(%rsp)	# D.6179, %sfp
	movl	%ebx, %eax	# p, p
	subl	$2, %eax	#, D.6182
	leaq	8(%rdi,%rax,4), %r14	#, D.6183
# SUCC: 28 [100.0%]  (FALLTHRU,CAN_FALLTHRU)
# BLOCK 28 freq:89 seq:26
# PRED: 27 [100.0%]  (FALLTHRU,CAN_FALLTHRU) 36 [91.0%]  (DFS_BACK,CAN_FALLTHRU)
.L28:
	movl	16(%rsp), %ebx	# %sfp, p
	testl	%ebx, %ebx	# p
# SUCC: 29 [91.0%]  (FALLTHRU,CAN_FALLTHRU) 36 [9.0%]  (CAN_FALLTHRU)
	jle	.L25	#,
# BLOCK 29 freq:81 seq:27
# PRED: 28 [91.0%]  (FALLTHRU,CAN_FALLTHRU)
	movl	$32767, %eax	#, tmp1305
	movq	24(%rsp), %rsi	# %sfp, ivtmp.104
	cltd
	movq	64(%rsp), %rdi	# %sfp, ivtmp.102
	idivl	%ebx	# p
	movq	32(%rsp), %r8	# %sfp, D.6175
# SUCC: 30 [100.0%]  (FALLTHRU,CAN_FALLTHRU)
	movq	96(%rsp), %r9	# %sfp, D.6183
# BLOCK 30 freq:900 seq:28
# PRED: 30 [91.0%]  (DFS_BACK,CAN_FALLTHRU) 29 [100.0%]  (FALLTHRU,CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L22:
	movl	(%rdi), %edx	# MEM[base: _814, offset: 0B], MEM[base: _814, offset: 0B]
	addq	$4, %rsi	#, ivtmp.104
	addq	%r8, %rdi	# D.6175, ivtmp.102
	movswl	%dx, %ecx	# MEM[base: _814, offset: 0B], D.6174
	sarl	$16, %edx	#, D.6174
	imull	%eax, %ecx	# tmp1305, D.6174
	imull	%eax, %edx	# tmp1305, D.6174
	addl	$16384, %ecx	#, D.6174
	addl	$16384, %edx	#, D.6174
	sarl	$15, %ecx	#, D.6174
	sarl	$15, %edx	#, D.6174
	movw	%cx, -4(%rsi)	# D.6174, MEM[base: _1194, offset: 0B]
	movw	%dx, -2(%rsi)	# D.6174, MEM[base: _1194, offset: 2B]
	cmpq	%r9, %rsi	# D.6183, ivtmp.104
# SUCC: 30 [91.0%]  (DFS_BACK,CAN_FALLTHRU) 31 [9.0%]  (FALLTHRU,CAN_FALLTHRU,LOOP_EXIT)
	jne	.L22	#,
# BLOCK 31 freq:81 seq:29
# PRED: 30 [9.0%]  (FALLTHRU,CAN_FALLTHRU,LOOP_EXIT)
	movq	24(%rsp), %rax	# %sfp, scratch
	movl	$0, 8(%rsp)	#, %sfp
	movl	80(%rsp), %r13d	# %sfp, ivtmp.93
	movq	64(%rsp), %rbx	# %sfp, ivtmp.91
	movl	(%rax), %eax	# *scratch_1004, *scratch_1004
# SUCC: 32 [100.0%]  (FALLTHRU,CAN_FALLTHRU)
	movl	%eax, 40(%rsp)	# *scratch_1004, %sfp
# BLOCK 32 freq:900 seq:30
# PRED: 31 [100.0%]  (FALLTHRU,CAN_FALLTHRU) 35 [91.0%]  (DFS_BACK,CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L24:
	movl	40(%rsp), %eax	# %sfp, *scratch_1004
	cmpl	$1, 16(%rsp)	#, %sfp
	movl	%eax, (%rbx)	# *scratch_1004, MEM[base: _1199, offset: 0B]
# SUCC: 33 [91.0%]  (FALLTHRU,CAN_FALLTHRU) 35 [9.0%]  (CAN_FALLTHRU)
	je	.L27	#,
# BLOCK 33 freq:819 seq:31
# PRED: 32 [91.0%]  (FALLTHRU,CAN_FALLTHRU)
	movq	24(%rsp), %rax	# %sfp, scratch
	movzwl	(%rbx), %r11d	# MEM[base: _1199, offset: 0B], D.6177
	movzwl	2(%rbx), %r10d	# MEM[base: _1199, offset: 2B], D.6177
	leaq	4(%rax), %r9	#, ivtmp.80
# SUCC: 34 [100.0%]  (FALLTHRU,CAN_FALLTHRU)
	xorl	%eax, %eax	# twidx
# BLOCK 34 freq:9100 seq:32
# PRED: 33 [100.0%]  (FALLTHRU,CAN_FALLTHRU) 34 [91.0%]  (DFS_BACK,CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L26:
	addl	%r13d, %eax	# ivtmp.93, D.6179
	movswl	(%r9), %r8d	# MEM[base: _1243, offset: 0B], D.6174
	movl	%eax, %edx	# D.6179, twidx
	movswl	2(%r9), %ecx	# MEM[base: _1243, offset: 2B], D.6174
	subl	%ebp, %edx	# Norig, twidx
	cmpl	%eax, %ebp	# D.6179, Norig
	cmovle	%edx, %eax	# D.6179,, twidx, twidx
	addq	$4, %r9	#, ivtmp.80
	movslq	%eax, %rdx	# twidx, D.6175
	movl	%ecx, %r12d	# D.6174, D.6174
	leaq	(%r15,%rdx,4), %rdx	#, D.6178
	movswl	(%rdx), %edi	# _832->r, D.6174
	movswl	2(%rdx), %esi	# _832->i, D.6174
	movl	%r8d, %edx	# D.6174, D.6174
	imull	%edi, %edx	# D.6174, D.6174
	imull	%esi, %r12d	# D.6174, D.6174
	imull	%edi, %ecx	# D.6174, D.6174
	imull	%r8d, %esi	# D.6174, D.6174
	subl	%r12d, %edx	# D.6174, D.6174
	addl	$16384, %edx	#, D.6174
	sarl	$15, %edx	#, D.6174
	addl	%edx, %r11d	# D.6174, D.6177
	leal	16384(%rsi,%rcx), %edx	#, D.6174
	movw	%r11w, (%rbx)	# D.6177, MEM[base: _1199, offset: 0B]
	sarl	$15, %edx	#, D.6174
	addl	%edx, %r10d	# D.6174, D.6177
	movw	%r10w, 2(%rbx)	# D.6177, MEM[base: _1199, offset: 2B]
	cmpq	%r9, %r14	# ivtmp.80, D.6183
# SUCC: 34 [91.0%]  (DFS_BACK,CAN_FALLTHRU) 35 [9.0%]  (FALLTHRU,CAN_FALLTHRU,LOOP_EXIT)
	jne	.L26	#,
# BLOCK 35 freq:900 seq:33
# PRED: 32 [9.0%]  (CAN_FALLTHRU) 34 [9.0%]  (FALLTHRU,CAN_FALLTHRU,LOOP_EXIT)
.L27:
	addl	$1, 8(%rsp)	#, %sfp
	addq	32(%rsp), %rbx	# %sfp, ivtmp.91
	movl	8(%rsp), %eax	# %sfp, q1
	addl	48(%rsp), %r13d	# %sfp, ivtmp.93
	cmpl	%eax, 16(%rsp)	# q1, %sfp
# SUCC: 32 [91.0%]  (DFS_BACK,CAN_FALLTHRU) 36 [9.0%]  (FALLTHRU,CAN_FALLTHRU,LOOP_EXIT)
	jne	.L24	#,
# BLOCK 36 freq:89 seq:34
# PRED: 28 [9.0%]  (CAN_FALLTHRU) 35 [9.0%]  (FALLTHRU,CAN_FALLTHRU,LOOP_EXIT)
.L25:
	addl	$1, 56(%rsp)	#, %sfp
	movl	88(%rsp), %edi	# %sfp, D.6179
	movl	56(%rsp), %eax	# %sfp, k
	addq	$4, 64(%rsp)	#, %sfp
	addl	%edi, 80(%rsp)	# D.6179, %sfp
	cmpl	%eax, 76(%rsp)	# k, %sfp
# SUCC: 28 [91.0%]  (DFS_BACK,CAN_FALLTHRU) 37 [9.0%]  (FALLTHRU,CAN_FALLTHRU,LOOP_EXIT)
	jg	.L28	#,
# BLOCK 37 freq:8 seq:35
# PRED: 36 [9.0%]  (FALLTHRU,CAN_FALLTHRU,LOOP_EXIT)
# SUCC: 38 [100.0%]  (FALLTHRU,CAN_FALLTHRU)
	movq	24(%rsp), %rdi	# %sfp,
# BLOCK 38 freq:9 seq:36
# PRED: 26 [12.4%]  (CAN_FALLTHRU) 37 [100.0%]  (FALLTHRU,CAN_FALLTHRU)
.L21:
	addq	$168, %rsp	#,
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	popq	%rbx	#
	.cfi_def_cfa_offset 48
	popq	%rbp	#
	.cfi_def_cfa_offset 40
	popq	%r12	#
	.cfi_def_cfa_offset 32
	popq	%r13	#
	.cfi_def_cfa_offset 24
	popq	%r14	#
	.cfi_def_cfa_offset 16
	popq	%r15	#
	.cfi_def_cfa_offset 8
# SUCC: EXIT [100.0%]  (ABNORMAL,SIBCALL)
	jmp	free	#
# BLOCK 39 freq:6 seq:37
# PRED: 7 [40.0%]  (CAN_FALLTHRU)
.L32:
	.cfi_restore_state
	movq	56(%rsp), %rax	# %sfp, D.6175
# SUCC: 40 [100.0%]  (FALLTHRU,CAN_FALLTHRU)
	movq	%rax, 80(%rsp)	# D.6175, %sfp
# BLOCK 40 freq:9 seq:38
# PRED: 21 [40.0%]  (CAN_FALLTHRU) 39 [100.0%]  (FALLTHRU,CAN_FALLTHRU)
.L10:
	leaq	264(%rbp), %rax	#, tw3
	leaq	0(%r13,%rbx), %r11	#, ivtmp.168
	movq	%rax, %rdi	# tw3, tw3
	movq	48(%rsp), %rax	# %sfp, fstride
	movq	%rdi, 8(%rsp)	# tw3, %sfp
	movq	%rdi, %rbx	# tw3, tw3
	movq	%rdi, %r15	# tw3, tw3
	leaq	0(,%rax,8), %rsi	#, D.6175
	leaq	(%rax,%rax,2), %rax	#, tmp1016
	movq	%rsi, 88(%rsp)	# D.6175, %sfp
	salq	$2, %rax	#, tmp1017
	movq	%rax, 104(%rsp)	# tmp1017, %sfp
	movl	4(%rbp), %eax	# st_32(D)->inverse, D.6174
	movl	%eax, 96(%rsp)	# D.6174, %sfp
	movq	80(%rsp), %rax	# %sfp, k
	leaq	0(%r13,%rax,8), %rbp	#, ivtmp.170
	leaq	0(%rbp,%rax,4), %r8	#, ivtmp.172
# SUCC: 43 [100.0%]  (CAN_FALLTHRU)
	jmp	.L18	#
# BLOCK 41 freq:49 seq:39
# PRED: 43 [50.0%]  (CAN_FALLTHRU)
.L62:
	movzwl	64(%rsp), %r10d	# %sfp, D.6176
	movzwl	76(%rsp), %ecx	# %sfp, D.6176
	movl	%r10d, %eax	# D.6176, tmp1098
	subl	%esi, %eax	# D.6176, tmp1098
	movw	%ax, (%r11)	# tmp1098, MEM[base: _1157, offset: 0B]
	movl	%ecx, %eax	# D.6176, D.6176
	addl	%edi, %eax	# D.6176, tmp1099
	movw	%ax, 2(%r11)	# tmp1099, MEM[base: _1157, offset: 2B]
	movl	%r10d, %eax	# D.6176, D.6176
	addl	%eax, %esi	# D.6176, tmp1100
	movl	%ecx, %eax	# D.6176, tmp1101
	subl	%edi, %eax	# D.6176, tmp1101
	movw	%si, (%r8)	# tmp1100, MEM[base: _1137, offset: 0B]
# SUCC: 42 [100.0%]  (FALLTHRU,CAN_FALLTHRU)
	movw	%ax, 2(%r8)	# tmp1101, MEM[base: _1137, offset: 2B]
# BLOCK 42 freq:98 seq:40
# PRED: 41 [100.0%]  (FALLTHRU,CAN_FALLTHRU) 44 [100.0%]  (CAN_FALLTHRU)
.L17:
	addq	$4, %r13	#, Fout
	addq	$4, %r11	#, ivtmp.168
	addq	$4, %rbp	#, ivtmp.170
	addq	$4, %r8	#, ivtmp.172
	subq	$1, 80(%rsp)	#, %sfp
# SUCC: 43 [91.0%]  (FALLTHRU,DFS_BACK,CAN_FALLTHRU) 12 [9.0%]  (CAN_FALLTHRU,LOOP_EXIT)
	je	.L1	#,
# BLOCK 43 freq:98 seq:41
# PRED: 42 [91.0%]  (FALLTHRU,DFS_BACK,CAN_FALLTHRU) 40 [100.0%]  (CAN_FALLTHRU)
.L18:
	movswl	0(%r13), %edx	# MEM[base: Fout_278, offset: 0B], D.6174
	movl	%edx, %eax	# D.6174, tmp1022
	sall	$13, %eax	#, tmp1022
	subl	%edx, %eax	# D.6174, D.6174
	movswl	2(%r13), %edx	# MEM[base: Fout_278, offset: 2B], D.6174
	addl	$16384, %eax	#, D.6174
	sarl	$15, %eax	#, D.6174
	movw	%ax, 0(%r13)	# D.6174, MEM[base: Fout_278, offset: 0B]
	movl	%edx, %eax	# D.6174, tmp1028
	sall	$13, %eax	#, tmp1028
	subl	%edx, %eax	# D.6174, D.6174
	addl	$16384, %eax	#, D.6174
	sarl	$15, %eax	#, D.6174
	movw	%ax, 2(%r13)	# D.6174, MEM[base: Fout_278, offset: 2B]
	movswl	(%r11), %edx	# MEM[base: _1157, offset: 0B], D.6174
	movl	%edx, %eax	# D.6174, tmp1034
	sall	$13, %eax	#, tmp1034
	subl	%edx, %eax	# D.6174, D.6174
	movswl	2(%r11), %edx	# MEM[base: _1157, offset: 2B], D.6174
	addl	$16384, %eax	#, D.6174
	sarl	$15, %eax	#, D.6174
	movw	%ax, (%r11)	# D.6174, MEM[base: _1157, offset: 0B]
	movl	%edx, %eax	# D.6174, tmp1040
	sall	$13, %eax	#, tmp1040
	subl	%edx, %eax	# D.6174, D.6174
	addl	$16384, %eax	#, D.6174
	sarl	$15, %eax	#, D.6174
	movw	%ax, 2(%r11)	# D.6174, MEM[base: _1157, offset: 2B]
	movswl	0(%rbp), %edx	# MEM[base: _1142, offset: 0B], D.6174
	movl	%edx, %eax	# D.6174, tmp1046
	sall	$13, %eax	#, tmp1046
	subl	%edx, %eax	# D.6174, D.6174
	movswl	2(%rbp), %edx	# MEM[base: _1142, offset: 2B], D.6174
	addl	$16384, %eax	#, D.6174
	sarl	$15, %eax	#, D.6174
	movw	%ax, 0(%rbp)	# D.6174, MEM[base: _1142, offset: 0B]
	movl	%edx, %eax	# D.6174, tmp1052
	sall	$13, %eax	#, tmp1052
	subl	%edx, %eax	# D.6174, D.6174
	addl	$16384, %eax	#, D.6174
	sarl	$15, %eax	#, D.6174
	movw	%ax, 2(%rbp)	# D.6174, MEM[base: _1142, offset: 2B]
	movswl	(%r8), %eax	# MEM[base: _1137, offset: 0B], D.6174
	movswl	2(%r8), %ecx	# MEM[base: _1137, offset: 2B], D.6174
	movl	%eax, %edx	# D.6174, tmp1058
	sall	$13, %edx	#, tmp1058
	subl	%eax, %edx	# D.6174, D.6174
	movl	%ecx, %eax	# D.6174, tmp1063
	sall	$13, %eax	#, tmp1063
	addl	$16384, %edx	#, D.6174
	subl	%ecx, %eax	# D.6174, D.6174
	sarl	$15, %edx	#, D.6174
	movq	8(%rsp), %rcx	# %sfp, tw3
	addl	$16384, %eax	#, D.6174
	movw	%dx, (%r8)	# D.6174, MEM[base: _1137, offset: 0B]
	sarl	$15, %eax	#, D.6174
	movw	%ax, 2(%r8)	# D.6174, MEM[base: _1137, offset: 2B]
	movswl	(%r11), %edi	# MEM[base: _1157, offset: 0B], D.6174
	movswl	(%rcx), %r10d	# MEM[base: tw3_376, offset: 0B], D.6174
	movswl	0(%rbp), %r14d	# MEM[base: _1142, offset: 0B], D.6174
	movswl	(%rbx), %r12d	# MEM[base: tw3_356, offset: 0B], D.6174
	movl	%edi, 56(%rsp)	# D.6174, %sfp
	movswl	(%r15), %edi	# MEM[base: tw3_335, offset: 0B], D.6174
	movl	%r10d, 32(%rsp)	# D.6174, %sfp
	movswl	2(%rcx), %r10d	# MEM[base: tw3_376, offset: 2B], D.6174
	movswl	2(%rbx), %r9d	# MEM[base: tw3_356, offset: 2B], D.6174
	movl	%r14d, %ecx	# D.6174, D.6174
	movswl	2(%r11), %esi	# MEM[base: _1157, offset: 2B], D.6174
	imull	%r12d, %ecx	# D.6174, D.6174
	movl	%edi, 16(%rsp)	# D.6174, %sfp
	movswl	2(%rbp), %edi	# MEM[base: _1142, offset: 2B], D.6174
	movl	%r10d, 48(%rsp)	# D.6174, %sfp
	movl	%esi, 24(%rsp)	# D.6174, %sfp
	movswl	2(%r15), %esi	# MEM[base: tw3_335, offset: 2B], D.6174
	movl	%edi, %r10d	# D.6174, D.6174
	imull	%r12d, %edi	# D.6174, D.6174
	imull	%r9d, %r10d	# D.6174, D.6174
	imull	%r14d, %r9d	# D.6174, D.6174
	subl	%r10d, %ecx	# D.6174, D.6174
	movzwl	0(%r13), %r10d	# MEM[base: Fout_278, offset: 0B], D.6176
	leal	16384(%r9,%rdi), %r14d	#, D.6174
	addl	$16384, %ecx	#, D.6174
	movl	16(%rsp), %r9d	# %sfp, D.6174
	sarl	$15, %ecx	#, D.6174
	movzwl	2(%r13), %edi	# MEM[base: Fout_278, offset: 2B], D.6176
	sarl	$15, %r14d	#, D.6174
	movl	%ecx, %r12d	# D.6174, D.6176
	addw	0(%r13), %r12w	# MEM[base: Fout_278, offset: 0B], D.6176
	subl	%ecx, %r10d	# D.6174, D.6176
	movw	%r10w, 64(%rsp)	# D.6176, %sfp
	movl	56(%rsp), %r10d	# %sfp, D.6174
	subl	%r14d, %edi	# D.6174, D.6176
	addw	2(%r13), %r14w	# MEM[base: Fout_278, offset: 2B], D.6176
	movw	%di, 76(%rsp)	# D.6176, %sfp
	movl	24(%rsp), %edi	# %sfp, D.6174
	movw	%r12w, 0(%r13)	# D.6176, MEM[base: Fout_278, offset: 0B]
	imull	%r10d, %r9d	# D.6174, D.6174
	movw	%r14w, 2(%r13)	# D.6176, MEM[base: Fout_278, offset: 2B]
	imull	%esi, %edi	# D.6174, D.6174
	movl	%r9d, %ecx	# D.6174, D.6174
	movl	32(%rsp), %r9d	# %sfp, D.6174
	imull	%r10d, %esi	# D.6174, D.6174
	movl	16(%rsp), %r10d	# %sfp, D.6174
	imull	24(%rsp), %r10d	# %sfp, D.6174
	subl	%edi, %ecx	# D.6174, D.6174
	addl	$16384, %ecx	#, D.6174
	imull	%edx, %r9d	# D.6174, D.6174
	sarl	$15, %ecx	#, D.6174
	imull	48(%rsp), %edx	# %sfp, D.6174
	leal	16384(%rsi,%r10), %esi	#, D.6174
	movl	%ecx, %r10d	# D.6174, D.6176
	sarl	$15, %esi	#, D.6174
	movl	%r9d, %edi	# D.6174, D.6174
	movl	48(%rsp), %r9d	# %sfp, D.6174
	imull	%eax, %r9d	# D.6174, D.6174
	imull	32(%rsp), %eax	# %sfp, D.6174
	subl	%r9d, %edi	# D.6174, D.6174
	leal	16384(%rdx,%rax), %eax	#, D.6174
	addl	$16384, %edi	#, D.6174
	sarl	$15, %edi	#, D.6174
	sarl	$15, %eax	#, D.6174
	leal	(%rdi,%rcx), %r9d	#, D.6176
	movl	96(%rsp), %ecx	# %sfp,
	subl	%edi, %r10d	# D.6174, D.6176
	leal	(%rax,%rsi), %edx	#, D.6176
	subl	%r9d, %r12d	# D.6176, tmp1096
	subl	%eax, %esi	# D.6174, D.6176
	subl	%edx, %r14d	# D.6176, tmp1097
	movw	%r12w, 0(%rbp)	# tmp1096, MEM[base: _1142, offset: 0B]
	addq	40(%rsp), %r15	# %sfp, tw3
	movl	%r10d, %edi	# D.6176, D.6176
	movw	%r14w, 2(%rbp)	# tmp1097, MEM[base: _1142, offset: 2B]
	addq	88(%rsp), %rbx	# %sfp, tw3
	movq	104(%rsp), %r14	# %sfp, tmp1017
	addw	%r9w, 0(%r13)	# D.6176, MEM[base: Fout_278, offset: 0B]
	addq	%r14, 8(%rsp)	# tmp1017, %sfp
	addw	%dx, 2(%r13)	# D.6176, MEM[base: Fout_278, offset: 2B]
	testl	%ecx, %ecx	#
# SUCC: 41 [50.0%]  (CAN_FALLTHRU) 44 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
	jne	.L62	#,
# BLOCK 44 freq:49 seq:42
# PRED: 43 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
	movzwl	64(%rsp), %edx	# %sfp, D.6176
	movzwl	76(%rsp), %ecx	# %sfp, D.6176
	movl	%edx, %eax	# D.6176, D.6176
	addl	%esi, %eax	# D.6176, tmp1102
	movw	%ax, (%r11)	# tmp1102, MEM[base: _1157, offset: 0B]
	movl	%ecx, %eax	# D.6176, tmp1103
	addl	%edi, %ecx	# D.6176, tmp1105
	subl	%edi, %eax	# D.6176, tmp1103
	movw	%ax, 2(%r11)	# tmp1103, MEM[base: _1157, offset: 2B]
	movl	%edx, %eax	# D.6176, tmp1104
	subl	%esi, %eax	# D.6176, tmp1104
	movw	%cx, 2(%r8)	# tmp1105, MEM[base: _1137, offset: 2B]
	movw	%ax, (%r8)	# tmp1104, MEM[base: _1137, offset: 0B]
# SUCC: 42 [100.0%]  (CAN_FALLTHRU)
	jmp	.L17	#
# BLOCK 45 freq:6 seq:43
# PRED: 5 [20.0%]  (CAN_FALLTHRU)
.L31:
	movq	56(%rsp), %rax	# %sfp, D.6175
	movq	%rax, 16(%rsp)	# D.6175, %sfp
# SUCC: 46 [100.0%]  (FALLTHRU,CAN_FALLTHRU)
	movq	%rax, %rsi	# D.6175, m
# BLOCK 46 freq:9 seq:44
# PRED: 45 [100.0%]  (FALLTHRU,CAN_FALLTHRU) 49 [100.0%]  (CAN_FALLTHRU)
.L7:
	movq	48(%rsp), %rdi	# %sfp, fstride
	leaq	264(%rbp), %r12	#, tw2
	addq	%r13, %rbx	# Fout, ivtmp.144
	leaq	0(%r13,%rsi,8), %r9	#, ivtmp.146
	movq	%r12, %r15	# tw2, tw2
	movq	%rdi, %rax	# fstride, D.6175
	imulq	%rsi, %rax	# m, D.6175
	movswl	266(%rbp,%rax,4), %eax	# MEM[(struct  *)_124 + 2B], D.6174
	movl	%eax, 24(%rsp)	# D.6174, %sfp
	movq	%rdi, %rax	# fstride, D.6175
	salq	$3, %rax	#, D.6175
# SUCC: 47 [100.0%]  (FALLTHRU,CAN_FALLTHRU)
	movq	%rax, 32(%rsp)	# D.6175, %sfp
# BLOCK 47 freq:98 seq:45
# PRED: 46 [100.0%]  (FALLTHRU,CAN_FALLTHRU) 47 [91.0%]  (DFS_BACK,CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L14:
	movswl	0(%r13), %eax	# MEM[base: Fout_127, offset: 0B], D.6174
	imull	$10922, %eax, %eax	#, D.6174, D.6174
	addl	$16384, %eax	#, D.6174
	sarl	$15, %eax	#, D.6174
	movw	%ax, 0(%r13)	# D.6174, MEM[base: Fout_127, offset: 0B]
	movswl	2(%r13), %eax	# MEM[base: Fout_127, offset: 2B], D.6174
	imull	$10922, %eax, %eax	#, D.6174, D.6174
	addl	$16384, %eax	#, D.6174
	sarl	$15, %eax	#, D.6174
	movw	%ax, 2(%r13)	# D.6174, MEM[base: Fout_127, offset: 2B]
	movswl	(%rbx), %eax	# MEM[base: _1187, offset: 0B], D.6174
	imull	$10922, %eax, %eax	#, D.6174, D.6174
	addl	$16384, %eax	#, D.6174
	sarl	$15, %eax	#, D.6174
	movw	%ax, (%rbx)	# D.6174, MEM[base: _1187, offset: 0B]
	movswl	2(%rbx), %eax	# MEM[base: _1187, offset: 2B], D.6174
	imull	$10922, %eax, %eax	#, D.6174, D.6174
	addl	$16384, %eax	#, D.6174
	sarl	$15, %eax	#, D.6174
	movw	%ax, 2(%rbx)	# D.6174, MEM[base: _1187, offset: 2B]
	movswl	(%r9), %edx	# MEM[base: _1183, offset: 0B], D.6174
	movswl	2(%r9), %eax	# MEM[base: _1183, offset: 2B], D.6174
	imull	$10922, %edx, %edx	#, D.6174, D.6174
	imull	$10922, %eax, %eax	#, D.6174, D.6174
	addl	$16384, %edx	#, D.6174
	addl	$16384, %eax	#, D.6174
	sarl	$15, %edx	#, D.6174
	sarl	$15, %eax	#, D.6174
	movw	%dx, (%r9)	# D.6174, MEM[base: _1183, offset: 0B]
	movw	%ax, 2(%r9)	# D.6174, MEM[base: _1183, offset: 2B]
	movswl	(%rbx), %ebp	# MEM[base: _1187, offset: 0B], D.6174
	movswl	2(%rbx), %r8d	# MEM[base: _1187, offset: 2B], D.6174
	movswl	(%r12), %r11d	# MEM[base: tw2_170, offset: 0B], D.6174
	movswl	2(%r12), %r10d	# MEM[base: tw2_170, offset: 2B], D.6174
	movl	%ebp, %ecx	# D.6174, D.6174
	movswl	(%r15), %esi	# MEM[base: tw2_190, offset: 0B], D.6174
	movl	%r8d, %r14d	# D.6174, D.6174
	movswl	2(%r15), %edi	# MEM[base: tw2_190, offset: 2B], D.6174
	imull	%r11d, %ecx	# D.6174, D.6174
	imull	%r10d, %r14d	# D.6174, D.6174
	imull	%r11d, %r8d	# D.6174, D.6174
	imull	%ebp, %r10d	# D.6174, D.6174
	subl	%r14d, %ecx	# D.6174, D.6174
	movl	%eax, %r14d	# D.6174, D.6174
	addl	$16384, %ecx	#, D.6174
	imull	%edi, %r14d	# D.6174, D.6174
	leal	16384(%r10,%r8), %r8d	#, D.6174
	sarl	$15, %ecx	#, D.6174
	movl	%ecx, 8(%rsp)	# D.6174, %sfp
	movl	%edx, %ecx	# D.6174, D.6174
	sarl	$15, %r8d	#, D.6174
	imull	%esi, %ecx	# D.6174, D.6174
	subl	%r14d, %ecx	# D.6174, D.6174
	movzwl	8(%rsp), %r14d	# %sfp, tmp1426
	addl	$16384, %ecx	#, D.6174
	sarl	$15, %ecx	#, D.6174
	addl	%ecx, %r14d	# D.6174, D.6176
	imull	%edi, %edx	# D.6174, D.6174
	movzwl	0(%r13), %edi	# MEM[base: Fout_127, offset: 0B], D.6176
	addq	$4, %r9	#, ivtmp.146
	imull	%esi, %eax	# D.6174, D.6174
	addq	40(%rsp), %r12	# %sfp, tw2
	addq	32(%rsp), %r15	# %sfp, tw2
	leal	16384(%rdx,%rax), %eax	#, D.6174
	movl	%r14d, %edx	# D.6176, D.6177
	sarw	%dx	# D.6177
	sarl	$15, %eax	#, D.6174
	leal	(%rax,%r8), %esi	#, D.6176
	subl	%edx, %edi	# D.6177, D.6176
	subl	%eax, %r8d	# D.6174, D.6176
	movw	%di, (%rbx)	# D.6176, MEM[base: _1187, offset: 0B]
	movzwl	2(%r13), %edi	# MEM[base: Fout_127, offset: 2B], D.6176
	movl	%esi, %edx	# D.6176, D.6177
	sarw	%dx	# D.6177
	subl	%edx, %edi	# D.6177, D.6176
	movswl	%r8w, %edx	# D.6176, D.6174
	movw	%di, 2(%rbx)	# D.6176, MEM[base: _1187, offset: 2B]
	movl	24(%rsp), %edi	# %sfp, D.6174
	addw	%r14w, 0(%r13)	# D.6176, MEM[base: Fout_127, offset: 0B]
	addw	%si, 2(%r13)	# D.6176, MEM[base: Fout_127, offset: 2B]
	addq	$4, %r13	#, Fout
	imull	%edi, %edx	# D.6174, D.6174
	addl	$16384, %edx	#, D.6174
	sarl	$15, %edx	#, D.6174
	movl	%edx, %eax	# D.6174, D.6176
	addw	(%rbx), %ax	# MEM[base: _1187, offset: 0B], D.6176
	movw	%ax, -4(%r9)	# D.6176, MEM[base: _1183, offset: 0B]
	movzwl	8(%rsp), %eax	# %sfp, D.6176
	subl	%ecx, %eax	# D.6174, D.6176
	movzwl	2(%rbx), %ecx	# MEM[base: _1187, offset: 2B], D.6176
	cwtl
	imull	%edi, %eax	# D.6174, D.6174
	addl	$16384, %eax	#, D.6174
	sarl	$15, %eax	#, D.6174
	subl	%eax, %ecx	# D.6174, D.6176
	movw	%cx, -2(%r9)	# D.6176, MEM[base: _1183, offset: 2B]
	subw	%dx, (%rbx)	# D.6174, MEM[base: _1187, offset: 0B]
	addw	%ax, 2(%rbx)	# D.6174, MEM[base: _1187, offset: 2B]
	addq	$4, %rbx	#, ivtmp.144
	subq	$1, 16(%rsp)	#, %sfp
# SUCC: 47 [91.0%]  (DFS_BACK,CAN_FALLTHRU) 48 [9.0%]  (FALLTHRU)
	jne	.L14	#,
# BLOCK 48 freq:9 seq:46
# PRED: 47 [9.0%]  (FALLTHRU)
# SUCC: 12 [100.0%]  (CAN_FALLTHRU,LOOP_EXIT)
	jmp	.L1	#
# BLOCK 49 freq:2 seq:47
# PRED: 19 [20.0%]  (CAN_FALLTHRU)
.L34:
	movq	$1, 16(%rsp)	#, %sfp
	movl	$4, %ebx	#, D.6175
	movq	16(%rsp), %rsi	# %sfp, m
# SUCC: 46 [100.0%]  (CAN_FALLTHRU)
	jmp	.L7	#
	.cfi_endproc
.LFE77:
	.size	kf_work, .-kf_work
	.section	.text.unlikely
.LCOLDE0:
	.text
.LHOTE0:
	.section	.text.unlikely
.LCOLDB11:
	.text
.LHOTB11:
	.p2align 4,,15
	.globl	kiss_fft_alloc
	.type	kiss_fft_alloc, @function
kiss_fft_alloc:
.LFB79:
	.cfi_startproc
# BLOCK 2 freq:501 seq:0
# PRED: ENTRY [100.0%]  (FALLTHRU)
	leal	-1(%rdi), %eax	#, D.6238
	pushq	%r15	#
	.cfi_def_cfa_offset 16
	.cfi_offset 15, -16
	cltq
	pushq	%r14	#
	.cfi_def_cfa_offset 24
	.cfi_offset 14, -24
	pushq	%r13	#
	.cfi_def_cfa_offset 32
	.cfi_offset 13, -32
	pushq	%r12	#
	.cfi_def_cfa_offset 40
	.cfi_offset 12, -40
	pushq	%rbp	#
	.cfi_def_cfa_offset 48
	.cfi_offset 6, -48
	pushq	%rbx	#
	.cfi_def_cfa_offset 56
	.cfi_offset 3, -56
	movl	%edi, %ebx	# nfft, nfft
	leaq	268(,%rax,4), %rdi	#, memneeded
	subq	$56, %rsp	#,
	.cfi_def_cfa_offset 112
	testq	%rcx, %rcx	# lenmem
# SUCC: 72 [6.7%]  (CAN_FALLTHRU) 3 [93.3%]  (FALLTHRU,CAN_FALLTHRU)
	je	.L157	#,
# BLOCK 3 freq:468 seq:1
# PRED: 2 [93.3%]  (FALLTHRU,CAN_FALLTHRU)
	movq	%rdx, %r14	# mem, mem
	testq	%rdx, %rdx	# mem
# SUCC: 4 [85.0%]  (FALLTHRU,CAN_FALLTHRU) 5 [15.0%]  (CAN_FALLTHRU)
	je	.L66	#,
# BLOCK 4 freq:397 seq:2
# PRED: 3 [85.0%]  (FALLTHRU,CAN_FALLTHRU)
	cmpq	(%rcx), %rdi	# *lenmem_13(D), memneeded
	movl	$0, %eax	#, tmp331
# SUCC: 5 [100.0%]  (FALLTHRU,CAN_FALLTHRU)
	cmova	%rax, %r14	# mem,, tmp331, mem
# BLOCK 5 freq:468 seq:3
# PRED: 3 [15.0%]  (CAN_FALLTHRU) 4 [100.0%]  (FALLTHRU,CAN_FALLTHRU)
.L66:
# SUCC: 6 [100.0%]  (FALLTHRU,CAN_FALLTHRU)
	movq	%rdi, (%rcx)	# memneeded, *lenmem_13(D)
# BLOCK 6 freq:501 seq:4
# PRED: 5 [100.0%]  (FALLTHRU,CAN_FALLTHRU) 72 [100.0%]  (CAN_FALLTHRU)
.L65:
	testq	%r14, %r14	# mem
# SUCC: 7 [89.9%]  (FALLTHRU,CAN_FALLTHRU) 24 [10.1%]  (CAN_FALLTHRU)
	je	.L136	#,
# BLOCK 7 freq:451 seq:5
# PRED: 6 [89.9%]  (FALLTHRU,CAN_FALLTHRU)
	movl	%ebx, (%r14)	# nfft, MEM[(struct kiss_fft_state *)mem_2].nfft
	movl	%esi, 4(%r14)	# inverse_fft, MEM[(struct kiss_fft_state *)mem_2].inverse
	testl	%ebx, %ebx	# nfft
# SUCC: 8 [91.0%]  (FALLTHRU,CAN_FALLTHRU) 71 [9.0%]  (CAN_FALLTHRU)
	jle	.L158	#,
# BLOCK 8 freq:410 seq:6
# PRED: 7 [91.0%]  (FALLTHRU,CAN_FALLTHRU)
	pxor	%xmm3, %xmm3	# D.6237
	cvtsi2sd	%ebx, %xmm3	# nfft, D.6237
	xorl	%r15d, %r15d	# i
	movsd	.LC6(%rip), %xmm0	#, tmp271
	movsd	.LC8(%rip), %xmm4	#, tmp329
	leaq	264(%r14), %rbp	#, D.6243
	leaq	40(%rsp), %r13	#, tmp324
	movsd	%xmm4, (%rsp)	# tmp329, %sfp
	leaq	32(%rsp), %r12	#, tmp323
	movsd	.LC9(%rip), %xmm4	#, tmp330
	divsd	%xmm3, %xmm0	# D.6237, D.6237
	movsd	%xmm3, 24(%rsp)	# D.6237, %sfp
	movsd	%xmm4, 8(%rsp)	# tmp330, %sfp
	mulsd	.LC7(%rip), %xmm0	#, D.6237
	movsd	%xmm0, 16(%rsp)	# D.6237, %sfp
	testl	%esi, %esi	# inverse_fft
# SUCC: 9 [50.0%]  (FALLTHRU,CAN_FALLTHRU) 20 [50.0%]  (CAN_FALLTHRU)
	jne	.L109	#,
# BLOCK 9 freq:2278 seq:7
# PRED: 8 [50.0%]  (FALLTHRU,CAN_FALLTHRU) 9 [91.0%]  (DFS_BACK,CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L105:
	pxor	%xmm0, %xmm0	# D.6237
	cvtsi2sd	%r15d, %xmm0	# i, D.6237
	movq	%r12, %rsi	# tmp323,
	mulsd	16(%rsp), %xmm0	# %sfp, phase
	movq	%r13, %rdi	# tmp324,
	call	sincos	#
	movsd	32(%rsp), %xmm1	#, D.6237
	pxor	%xmm2, %xmm2	# tmp284
	movsd	(%rsp), %xmm3	# %sfp, tmp329
	movsd	8(%rsp), %xmm5	# %sfp, tmp330
	mulsd	%xmm3, %xmm1	# tmp329, D.6237
	movsd	40(%rsp), %xmm0	#, D.6242
	mulsd	%xmm3, %xmm0	# tmp329, D.6237
	addsd	%xmm5, %xmm1	# tmp330, D.6237
	addsd	%xmm5, %xmm0	# tmp330, D.6237
	cvttsd2si	%xmm1, %eax	# D.6237, tmp283
	cvtsi2sd	%eax, %xmm2	# tmp283, tmp284
	leal	-1(%rax), %edx	#, tmp339
	comisd	%xmm1, %xmm2	# D.6237, tmp284
	pxor	%xmm1, %xmm1	# tmp292
	cmova	%edx, %eax	# tmp283,, tmp339, tmp283
	movw	%ax, 0(%rbp)	# tmp283, MEM[base: _83, offset: 0B]
	cvttsd2si	%xmm0, %eax	# D.6237, tmp291
	cvtsi2sd	%eax, %xmm1	# tmp291, tmp292
	leal	-1(%rax), %edx	#, tmp341
	comisd	%xmm0, %xmm1	# D.6237, tmp292
	cmova	%edx, %eax	# tmp291,, tmp341, tmp291
	addl	$1, %r15d	#, i
	addq	$4, %rbp	#, ivtmp.229
	movw	%ax, -2(%rbp)	# tmp291, MEM[base: _83, offset: 2B]
	cmpl	%r15d, %ebx	# i, nfft
# SUCC: 9 [91.0%]  (DFS_BACK,CAN_FALLTHRU) 10 [9.0%]  (FALLTHRU,CAN_FALLTHRU,LOOP_EXIT)
	jne	.L105	#,
# BLOCK 10 freq:451 seq:8
# PRED: 21 [100.0%]  (CAN_FALLTHRU,LOOP_EXIT) 9 [9.0%]  (FALLTHRU,CAN_FALLTHRU,LOOP_EXIT) 71 [100.0%]  (CAN_FALLTHRU)
.L106:
	movsd	.LC4(%rip), %xmm1	#, tmp180
	sqrtsd	24(%rsp), %xmm0	# %sfp, D.6237
	movsd	.LC3(%rip), %xmm2	#, tmp176
	leaq	8(%r14), %rcx	#, facbuf
	andpd	%xmm0, %xmm1	# D.6237, tmp178
	comisd	%xmm1, %xmm2	# tmp178, tmp176
# SUCC: 11 [50.0%]  (FALLTHRU,CAN_FALLTHRU) 54 [50.0%]  (CAN_FALLTHRU)
	ja	.L159	#,
# BLOCK 11 freq:452 seq:9
# PRED: 10 [50.0%]  (FALLTHRU,CAN_FALLTHRU) 54 [100.0%]  (CAN_FALLTHRU)
.L69:
	movsd	.LC5(%rip), %xmm1	#, tmp182
	movl	$4, %esi	#, nfft
	comisd	%xmm0, %xmm1	# D.6237, tmp182
	movsd	.LC2(%rip), %xmm1	#, tmp183
# SUCC: 12 [50.0%]  (FALLTHRU,CAN_FALLTHRU) 25 [50.0%]  (CAN_FALLTHRU)
	jbe	.L147	#,
# BLOCK 12 freq:226 seq:10
# PRED: 11 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
	comisd	%xmm0, %xmm1	# D.6237, tmp183
# SUCC: 48 [50.0%]  (CAN_FALLTHRU) 13 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
	ja	.L74	#,
# BLOCK 13 freq:113 seq:11
# PRED: 12 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
	movl	$1431655766, %r8d	#, tmp332
# SUCC: 16 [100.0%]  (CAN_FALLTHRU)
	jmp	.L82	#
# BLOCK 14 freq:433 seq:12
# PRED: 16 [50.0%]  (CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L160:
# SUCC: 15 [100.0%]  (FALLTHRU,CAN_FALLTHRU)
	movq	%rcx, %rdi	# facbuf, facbuf
# BLOCK 15 freq:973 seq:13
# PRED: 14 [100.0%]  (FALLTHRU,CAN_FALLTHRU) 60 [50.0%]  (CAN_FALLTHRU)
.L81:
	movl	%esi, (%rdi)	# nfft, *facbuf_240
	leaq	8(%rdi), %rcx	#, facbuf
	movl	%ebx, %eax	# nfft, nfft
	cltd
	idivl	%esi	# nfft
	movl	%eax, %ebx	# nfft, nfft
	movl	%eax, 4(%rdi)	# nfft, MEM[(int *)facbuf_240 + 4B]
	cmpl	$1, %eax	#, nfft
# SUCC: 16 [91.0%]  (FALLTHRU,DFS_BACK,CAN_FALLTHRU) 24 [9.0%]  (CAN_FALLTHRU,LOOP_EXIT)
	jle	.L136	#,
# BLOCK 16 freq:866 seq:14
# PRED: 13 [100.0%]  (CAN_FALLTHRU) 19 [100.0%]  (DFS_BACK,CAN_FALLTHRU) 15 [91.0%]  (FALLTHRU,DFS_BACK,CAN_FALLTHRU) 63 [100.0%]  (DFS_BACK,CAN_FALLTHRU)
.L82:
	movl	%ebx, %eax	# nfft, tmp186
	cltd
	idivl	%esi	# nfft
	testl	%edx, %edx	# D.6238
# SUCC: 17 [50.0%]  (FALLTHRU,CAN_FALLTHRU) 14 [50.0%]  (CAN_FALLTHRU)
	je	.L160	#,
# BLOCK 17 freq:433 seq:15
# PRED: 16 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
	cmpl	$2, %esi	#, nfft
# SUCC: 58 [33.3%]  (CAN_FALLTHRU) 18 [66.7%]  (FALLTHRU,CAN_FALLTHRU)
	je	.L79	#,
# BLOCK 18 freq:289 seq:16
# PRED: 17 [66.7%]  (FALLTHRU,CAN_FALLTHRU)
	cmpl	$4, %esi	#, nfft
# SUCC: 19 [50.0%]  (FALLTHRU,CAN_FALLTHRU) 62 [50.0%]  (CAN_FALLTHRU)
	jne	.L78	#,
# BLOCK 19 freq:144 seq:17
# PRED: 18 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
	movl	%ebx, %esi	# nfft, nfft
# SUCC: 16 [100.0%]  (DFS_BACK,CAN_FALLTHRU)
	jmp	.L82	#
# BLOCK 20 freq:2278 seq:18
# PRED: 8 [50.0%]  (CAN_FALLTHRU) 20 [91.0%]  (DFS_BACK,CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L109:
	pxor	%xmm0, %xmm0	# D.6237
	movq	%r12, %rsi	# tmp323,
	movq	%r13, %rdi	# tmp324,
	cvtsi2sd	%r15d, %xmm0	# i, D.6237
	mulsd	16(%rsp), %xmm0	# %sfp, phase
	xorpd	.LC10(%rip), %xmm0	#, phase
	call	sincos	#
	movsd	32(%rsp), %xmm1	#, D.6237
	pxor	%xmm2, %xmm2	# tmp307
	movsd	(%rsp), %xmm4	# %sfp, tmp329
	movsd	8(%rsp), %xmm6	# %sfp, tmp330
	mulsd	%xmm4, %xmm1	# tmp329, D.6237
	movsd	40(%rsp), %xmm0	#, D.6242
	mulsd	%xmm4, %xmm0	# tmp329, D.6237
	addsd	%xmm6, %xmm1	# tmp330, D.6237
	addsd	%xmm6, %xmm0	# tmp330, D.6237
	cvttsd2si	%xmm1, %eax	# D.6237, tmp306
	cvtsi2sd	%eax, %xmm2	# tmp306, tmp307
	leal	-1(%rax), %edx	#, tmp340
	comisd	%xmm1, %xmm2	# D.6237, tmp307
	pxor	%xmm1, %xmm1	# tmp315
	cmova	%edx, %eax	# tmp306,, tmp340, tmp306
	movw	%ax, 0(%rbp)	# tmp306, MEM[base: _153, offset: 0B]
	cvttsd2si	%xmm0, %eax	# D.6237, tmp314
	cvtsi2sd	%eax, %xmm1	# tmp314, tmp315
	leal	-1(%rax), %edx	#, tmp342
	comisd	%xmm0, %xmm1	# D.6237, tmp315
	cmova	%edx, %eax	# tmp314,, tmp342, tmp314
	addl	$1, %r15d	#, i
	addq	$4, %rbp	#, ivtmp.235
	movw	%ax, -2(%rbp)	# tmp314, MEM[base: _153, offset: 2B]
	cmpl	%r15d, %ebx	# i, nfft
# SUCC: 20 [91.0%]  (DFS_BACK,CAN_FALLTHRU) 21 [9.0%]  (FALLTHRU)
	jne	.L109	#,
# BLOCK 21 freq:205 seq:19
# PRED: 20 [9.0%]  (FALLTHRU)
# SUCC: 10 [100.0%]  (CAN_FALLTHRU,LOOP_EXIT)
	jmp	.L106	#
# BLOCK 22 freq:417 seq:20
# PRED: 30 [50.0%]  (CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L118:
# SUCC: 23 [100.0%]  (FALLTHRU,CAN_FALLTHRU)
	movl	$2, %esi	#, nfft
# BLOCK 23 freq:973 seq:21
# PRED: 27 [50.0%]  (CAN_FALLTHRU) 32 [50.0%]  (CAN_FALLTHRU) 22 [100.0%]  (FALLTHRU,CAN_FALLTHRU) 67 [100.0%]  (CAN_FALLTHRU)
.L86:
	movl	%ebx, %eax	# nfft, nfft
	movl	%esi, (%rcx)	# nfft, *facbuf_236
	cltd
	idivl	%esi	# nfft
	movl	%eax, %ebx	# nfft, nfft
	movl	%ebx, 4(%rcx)	# nfft, MEM[(int *)facbuf_236 + 4B]
	leaq	8(%rcx), %rax	#, facbuf
	cmpl	$1, %ebx	#, nfft
# SUCC: 57 [91.0%]  (CAN_FALLTHRU) 24 [9.0%]  (FALLTHRU,CAN_FALLTHRU,LOOP_EXIT)
	jg	.L93	#,
# BLOCK 24 freq:501 seq:22
# PRED: 46 [9.0%]  (CAN_FALLTHRU,LOOP_EXIT) 6 [10.1%]  (CAN_FALLTHRU) 36 [9.0%]  (CAN_FALLTHRU,LOOP_EXIT) 59 [9.0%]  (CAN_FALLTHRU,LOOP_EXIT) 65 [9.0%]  (CAN_FALLTHRU,LOOP_EXIT) 23 [9.0%]  (FALLTHRU,CAN_FALLTHRU,LOOP_EXIT) 15 [9.0%]  (CAN_FALLTHRU,LOOP_EXIT) 45 [100.0%]  (CAN_FALLTHRU,LOOP_EXIT)
.L136:
	addq	$56, %rsp	#,
	.cfi_remember_state
	.cfi_def_cfa_offset 56
	movq	%r14, %rax	# mem,
	popq	%rbx	#
	.cfi_def_cfa_offset 48
	popq	%rbp	#
	.cfi_def_cfa_offset 40
	popq	%r12	#
	.cfi_def_cfa_offset 32
	popq	%r13	#
	.cfi_def_cfa_offset 24
	popq	%r14	#
	.cfi_def_cfa_offset 16
	popq	%r15	#
	.cfi_def_cfa_offset 8
# SUCC: EXIT [100.0%] 
	ret
# BLOCK 25 freq:226 seq:23
# PRED: 11 [50.0%]  (CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L147:
	.cfi_restore_state
	comisd	%xmm0, %xmm1	# D.6237, D.6237
# SUCC: 39 [50.0%]  (CAN_FALLTHRU) 26 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
	ja	.L85	#,
# BLOCK 26 freq:113 seq:24
# PRED: 25 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
	movl	$1431655766, %edi	#, tmp335
# SUCC: 27 [100.0%]  (FALLTHRU,CAN_FALLTHRU)
# BLOCK 27 freq:216 seq:25
# PRED: 26 [100.0%]  (FALLTHRU,CAN_FALLTHRU) 57 [100.0%]  (DFS_BACK,CAN_FALLTHRU)
.L95:
	movl	%ebx, %eax	# nfft, tmp212
	cltd
	idivl	%esi	# nfft
	testl	%edx, %edx	# D.6238
# SUCC: 28 [50.0%]  (FALLTHRU,CAN_FALLTHRU) 23 [50.0%]  (CAN_FALLTHRU)
	je	.L86	#,
# BLOCK 28 freq:108 seq:26
# PRED: 27 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
	cmpl	$2, %esi	#, nfft
# SUCC: 55 [33.3%]  (CAN_FALLTHRU) 29 [66.7%]  (FALLTHRU,CAN_FALLTHRU)
	je	.L88	#,
# BLOCK 29 freq:72 seq:27
# PRED: 28 [66.7%]  (FALLTHRU,CAN_FALLTHRU)
	cmpl	$4, %esi	#, nfft
# SUCC: 30 [50.0%]  (FALLTHRU,CAN_FALLTHRU) 33 [50.0%]  (CAN_FALLTHRU)
	jne	.L87	#,
# BLOCK 30 freq:833 seq:28
# PRED: 29 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
	movl	%ebx, %esi	# nfft, tmp222
	sarl	$31, %esi	#, tmp222
	movl	%esi, %edx	# tmp222, tmp223
	shrl	$31, %edx	#, tmp223
	leal	(%rbx,%rdx), %eax	#, tmp224
	andl	$1, %eax	#, tmp225
	cmpl	%edx, %eax	# tmp223, tmp225
# SUCC: 31 [50.0%]  (FALLTHRU,CAN_FALLTHRU) 22 [50.0%]  (CAN_FALLTHRU)
	je	.L118	#,
# BLOCK 31 freq:417 seq:29
# PRED: 30 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
	comisd	%xmm0, %xmm1	# D.6237, D.6237
# SUCC: 34 [50.0%]  (CAN_FALLTHRU) 32 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
	ja	.L92	#,
# BLOCK 32 freq:835 seq:30
# PRED: 31 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
	movl	%ebx, %eax	# nfft, tmp346
	imull	%edi	# tmp335
	subl	%esi, %edx	# tmp222, tmp228
	movl	$3, %esi	#, nfft
	leal	(%rdx,%rdx,2), %eax	#, tmp234
	cmpl	%eax, %ebx	# tmp234, nfft
# SUCC: 33 [50.0%]  (FALLTHRU,CAN_FALLTHRU) 23 [50.0%]  (CAN_FALLTHRU)
	je	.L86	#,
# BLOCK 33 freq:186 seq:31
# PRED: 32 [50.0%]  (FALLTHRU,CAN_FALLTHRU) 66 [50.0%]  (CAN_FALLTHRU) 29 [50.0%]  (CAN_FALLTHRU)
.L87:
	addl	$2, %esi	#, D.6238
	pxor	%xmm2, %xmm2	# D.6237
	movq	%rcx, %rax	# facbuf, facbuf
	cvtsi2sd	%esi, %xmm2	# D.6238, D.6237
	comisd	%xmm0, %xmm2	# D.6237, D.6237
# SUCC: 34 [50.0%]  (FALLTHRU,CAN_FALLTHRU) 57 [50.0%]  (CAN_FALLTHRU)
	jbe	.L93	#,
# BLOCK 34 freq:278 seq:32
# PRED: 56 [50.0%]  (CAN_FALLTHRU) 31 [50.0%]  (CAN_FALLTHRU) 33 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
.L92:
	movl	%ebx, %esi	# nfft, nfft
	movq	%rcx, %rax	# facbuf, facbuf
# SUCC: 57 [100.0%]  (CAN_FALLTHRU)
	jmp	.L93	#
# BLOCK 35 freq:833 seq:33
# PRED: 41 [50.0%]  (CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L99:
	movl	%ebx, %edx	# nfft, tmp257
	shrl	$31, %edx	#, tmp257
	leal	(%rbx,%rdx), %eax	#, tmp258
	andl	$1, %eax	#, tmp259
	cmpl	%edx, %eax	# tmp257, tmp259
# SUCC: 64 [50.0%]  (CAN_FALLTHRU) 36 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
	jne	.L124	#,
# BLOCK 36 freq:418 seq:34
# PRED: 35 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
	movl	$2, (%rcx)	#, *facbuf_98
	leaq	8(%rcx), %rdi	#, facbuf
	addl	%edx, %ebx	# tmp261, tmp262
	movl	%ebx, %esi	# tmp262, nfft
	sarl	%esi	# nfft
	movl	%esi, 4(%rcx)	# nfft, MEM[(int *)facbuf_98 + 4B]
	cmpl	$1, %esi	#, nfft
# SUCC: 37 [91.0%]  (FALLTHRU,CAN_FALLTHRU) 24 [9.0%]  (CAN_FALLTHRU,LOOP_EXIT)
	jle	.L136	#,
# BLOCK 37 freq:372 seq:35
# PRED: 36 [91.0%]  (FALLTHRU,CAN_FALLTHRU)
	shrl	$31, %ebx	#, tmp266
	leal	(%rsi,%rbx), %eax	#, tmp267
	andl	$1, %eax	#, tmp268
	cmpl	%ebx, %eax	# tmp266, tmp268
# SUCC: 38 [50.0%]  (FALLTHRU,CAN_FALLTHRU) 69 [50.0%]  (CAN_FALLTHRU)
	je	.L125	#,
# BLOCK 38 freq:186 seq:36
# PRED: 37 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
	movq	%rdi, %rcx	# facbuf, facbuf
# SUCC: 39 [100.0%]  (FALLTHRU,DFS_BACK,CAN_FALLTHRU)
	movl	%esi, %ebx	# nfft, nfft
# BLOCK 39 freq:185 seq:37
# PRED: 38 [100.0%]  (FALLTHRU,DFS_BACK,CAN_FALLTHRU) 25 [50.0%]  (CAN_FALLTHRU) 44 [91.0%]  (DFS_BACK,CAN_FALLTHRU) 70 [100.0%]  (DFS_BACK,CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L85:
	movl	%ebx, %eax	# nfft, tmp252
	movq	%rcx, %rdi	# facbuf, facbuf
	cltd
	idivl	%esi	# nfft
	testl	%edx, %edx	# D.6238
# SUCC: 44 [50.0%]  (CAN_FALLTHRU) 40 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
	je	.L101	#,
# BLOCK 40 freq:370 seq:38
# PRED: 39 [50.0%]  (FALLTHRU,CAN_FALLTHRU) 43 [50.0%]  (CAN_FALLTHRU)
.L161:
	cmpl	$2, %esi	#, nfft
# SUCC: 70 [33.3%]  (CAN_FALLTHRU) 41 [66.7%]  (FALLTHRU,CAN_FALLTHRU)
	je	.L122	#,
# BLOCK 41 freq:247 seq:39
# PRED: 40 [66.7%]  (FALLTHRU,CAN_FALLTHRU)
	cmpl	$4, %esi	#, nfft
# SUCC: 35 [50.0%]  (CAN_FALLTHRU) 42 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
	je	.L99	#,
# BLOCK 42 freq:417 seq:40
# PRED: 41 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
	addl	$2, %esi	#, nfft
	pxor	%xmm2, %xmm2	# D.6237
# SUCC: 43 [100.0%]  (FALLTHRU,CAN_FALLTHRU)
	cvtsi2sd	%esi, %xmm2	# nfft, D.6237
# BLOCK 43 freq:556 seq:41
# PRED: 42 [100.0%]  (FALLTHRU,CAN_FALLTHRU) 64 [100.0%]  (CAN_FALLTHRU)
.L100:
	comisd	%xmm0, %xmm2	# D.6237, D.6237
	movl	%ebx, %eax	# nfft, tmp252
	movq	%rcx, %rdi	# facbuf, facbuf
	cmova	%ebx, %esi	# nfft,, nfft, nfft
	cltd
	idivl	%esi	# nfft
	testl	%edx, %edx	# D.6238
# SUCC: 44 [50.0%]  (FALLTHRU,CAN_FALLTHRU) 40 [50.0%]  (CAN_FALLTHRU)
	jne	.L161	#,
# BLOCK 44 freq:833 seq:42
# PRED: 43 [50.0%]  (FALLTHRU,CAN_FALLTHRU) 39 [50.0%]  (CAN_FALLTHRU) 69 [100.0%]  (CAN_FALLTHRU)
.L101:
	movl	%esi, (%rdi)	# nfft, *facbuf_226
	leaq	8(%rdi), %rcx	#, facbuf
	movl	%ebx, %eax	# nfft, nfft
	cltd
	idivl	%esi	# nfft
	movl	%eax, %ebx	# nfft, nfft
	movl	%eax, 4(%rdi)	# nfft, MEM[(int *)facbuf_226 + 4B]
	cmpl	$1, %eax	#, nfft
# SUCC: 39 [91.0%]  (DFS_BACK,CAN_FALLTHRU) 45 [9.0%]  (FALLTHRU)
	jg	.L85	#,
# BLOCK 45 freq:75 seq:43
# PRED: 44 [9.0%]  (FALLTHRU)
# SUCC: 24 [100.0%]  (CAN_FALLTHRU,LOOP_EXIT)
	jmp	.L136	#
# BLOCK 46 freq:1250 seq:44
# PRED: 48 [50.0%]  (CAN_FALLTHRU) 51 [50.0%]  (CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L162:
	movl	%eax, %ebx	# tmp320, nfft
	movl	%esi, (%rcx)	# nfft, *facbuf_57
	leaq	8(%rcx), %rax	#, facbuf
	movl	%ebx, 4(%rcx)	# nfft, MEM[(int *)facbuf_57 + 4B]
	cmpl	$1, %ebx	#, nfft
# SUCC: 47 [91.0%]  (FALLTHRU,CAN_FALLTHRU) 24 [9.0%]  (CAN_FALLTHRU,LOOP_EXIT)
	jle	.L136	#,
# BLOCK 47 freq:1138 seq:45
# PRED: 46 [91.0%]  (FALLTHRU,CAN_FALLTHRU)
# SUCC: 48 [100.0%]  (FALLTHRU,DFS_BACK,CAN_FALLTHRU)
	movq	%rax, %rcx	# facbuf, facbuf
# BLOCK 48 freq:927 seq:46
# PRED: 12 [50.0%]  (CAN_FALLTHRU) 47 [100.0%]  (FALLTHRU,DFS_BACK,CAN_FALLTHRU) 53 [100.0%]  (DFS_BACK,CAN_FALLTHRU)
.L74:
	movl	%ebx, %eax	# nfft, tmp320
	cltd
	idivl	%esi	# nfft
	testl	%edx, %edx	# D.6238
# SUCC: 49 [50.0%]  (FALLTHRU,CAN_FALLTHRU) 46 [50.0%]  (CAN_FALLTHRU)
	je	.L162	#,
# BLOCK 49 freq:463 seq:47
# PRED: 48 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
	cmpl	$2, %esi	#, nfft
# SUCC: 53 [33.3%]  (CAN_FALLTHRU) 50 [66.7%]  (FALLTHRU,CAN_FALLTHRU)
	je	.L126	#,
# BLOCK 50 freq:371 seq:48
# PRED: 49 [66.7%]  (FALLTHRU,CAN_FALLTHRU) 52 [66.7%]  (CAN_FALLTHRU)
.L163:
	cmpl	$4, %esi	#, nfft
# SUCC: 53 [50.0%]  (CAN_FALLTHRU) 51 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
	je	.L126	#,
# BLOCK 51 freq:186 seq:49
# PRED: 50 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
	addl	$2, %esi	#, nfft
	pxor	%xmm1, %xmm1	# D.6237
	movl	%ebx, %eax	# nfft, tmp320
	cvtsi2sd	%esi, %xmm1	# nfft, D.6237
	comisd	%xmm0, %xmm1	# D.6237, D.6237
	cmova	%ebx, %esi	# nfft,, nfft, nfft
	cltd
	idivl	%esi	# nfft
	testl	%edx, %edx	# D.6238
# SUCC: 52 [50.0%]  (FALLTHRU,CAN_FALLTHRU) 46 [50.0%]  (CAN_FALLTHRU)
	je	.L162	#,
# BLOCK 52 freq:93 seq:50
# PRED: 51 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
	cmpl	$2, %esi	#, nfft
# SUCC: 53 [33.3%]  (FALLTHRU,CAN_FALLTHRU) 50 [66.7%]  (CAN_FALLTHRU)
	jne	.L163	#,
# BLOCK 53 freq:371 seq:51
# PRED: 49 [33.3%]  (CAN_FALLTHRU) 50 [50.0%]  (CAN_FALLTHRU) 52 [33.3%]  (FALLTHRU,CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L126:
	movl	%ebx, %esi	# nfft, nfft
# SUCC: 48 [100.0%]  (DFS_BACK,CAN_FALLTHRU)
	jmp	.L74	#
# BLOCK 54 freq:226 seq:52
# PRED: 10 [50.0%]  (CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L159:
	cvttsd2siq	%xmm0, %rax	# D.6237, tmp181
	pxor	%xmm0, %xmm0	# D.6237
	cvtsi2sdq	%rax, %xmm0	# tmp181, D.6237
# SUCC: 11 [100.0%]  (CAN_FALLTHRU)
	jmp	.L69	#
# BLOCK 55 freq:556 seq:53
# PRED: 28 [33.3%]  (CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L88:
	movl	%ebx, %eax	# nfft, tmp345
	imull	%edi	# tmp335
	movl	%ebx, %eax	# nfft, tmp216
	sarl	$31, %eax	#, tmp216
	subl	%eax, %edx	# tmp216, tmp213
	leal	(%rdx,%rdx,2), %eax	#, tmp219
	cmpl	%eax, %ebx	# tmp219, nfft
# SUCC: 56 [50.0%]  (FALLTHRU,CAN_FALLTHRU) 65 [50.0%]  (CAN_FALLTHRU)
	je	.L164	#,
# BLOCK 56 freq:278 seq:54
# PRED: 55 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
	movsd	.LC1(%rip), %xmm2	#, D.6237
	movl	$5, %esi	#, nfft
	movq	%rcx, %rax	# facbuf, facbuf
	comisd	%xmm0, %xmm2	# D.6237, D.6237
# SUCC: 34 [50.0%]  (CAN_FALLTHRU) 57 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
	ja	.L92	#,
# BLOCK 57 freq:207 seq:55
# PRED: 56 [50.0%]  (FALLTHRU,CAN_FALLTHRU) 23 [91.0%]  (CAN_FALLTHRU) 33 [50.0%]  (CAN_FALLTHRU) 34 [100.0%]  (CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L93:
	movq	%rax, %rcx	# facbuf, facbuf
# SUCC: 27 [100.0%]  (DFS_BACK,CAN_FALLTHRU)
	jmp	.L95	#
# BLOCK 58 freq:556 seq:56
# PRED: 17 [33.3%]  (CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L79:
	movl	%ebx, %eax	# nfft, tmp343
	imull	%r8d	# tmp332
	movl	%ebx, %eax	# nfft, tmp192
	sarl	$31, %eax	#, tmp192
	subl	%eax, %edx	# tmp192, tmp189
	leal	(%rdx,%rdx,2), %eax	#, tmp195
	cmpl	%eax, %ebx	# tmp195, nfft
# SUCC: 68 [50.0%]  (CAN_FALLTHRU) 59 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
	jne	.L115	#,
# BLOCK 59 freq:278 seq:57
# PRED: 58 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
	movl	$3, (%rcx)	#, *facbuf_143
	leaq	8(%rcx), %rdi	#, facbuf
	movl	%edx, %ebx	# tmp189, nfft
	movl	%edx, 4(%rcx)	# nfft, MEM[(int *)facbuf_143 + 4B]
	cmpl	$1, %edx	#, nfft
# SUCC: 60 [91.0%]  (FALLTHRU,CAN_FALLTHRU) 24 [9.0%]  (CAN_FALLTHRU,LOOP_EXIT)
	jle	.L136	#,
# BLOCK 60 freq:247 seq:58
# PRED: 59 [91.0%]  (FALLTHRU,CAN_FALLTHRU)
	movl	%edx, %eax	# nfft, tmp344
	movl	$3, %esi	#, nfft
	imull	%r8d	# tmp332
	movl	%ebx, %eax	# nfft, tmp204
	sarl	$31, %eax	#, tmp204
	subl	%eax, %edx	# tmp204, tmp201
	leal	(%rdx,%rdx,2), %eax	#, tmp207
	cmpl	%eax, %ebx	# tmp207, nfft
# SUCC: 15 [50.0%]  (CAN_FALLTHRU) 61 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
	je	.L81	#,
# BLOCK 61 freq:124 seq:59
# PRED: 60 [50.0%]  (FALLTHRU,CAN_FALLTHRU)
	movq	%rdi, %rcx	# facbuf, facbuf
# SUCC: 62 [100.0%]  (FALLTHRU,CAN_FALLTHRU)
	movl	$3, %esi	#, nfft
# BLOCK 62 freq:186 seq:60
# PRED: 61 [100.0%]  (FALLTHRU,CAN_FALLTHRU) 18 [50.0%]  (CAN_FALLTHRU)
.L78:
	addl	$2, %esi	#, D.6238
	pxor	%xmm1, %xmm1	# D.6237
# SUCC: 63 [100.0%]  (FALLTHRU,CAN_FALLTHRU)
	cvtsi2sd	%esi, %xmm1	# D.6238, D.6237
# BLOCK 63 freq:556 seq:61
# PRED: 62 [100.0%]  (FALLTHRU,CAN_FALLTHRU) 68 [100.0%]  (CAN_FALLTHRU)
.L80:
	comisd	%xmm0, %xmm1	# D.6237, D.6237
	cmova	%ebx, %esi	# nfft,, nfft, nfft
# SUCC: 16 [100.0%]  (DFS_BACK,CAN_FALLTHRU)
	jmp	.L82	#
# BLOCK 64 freq:417 seq:62
# PRED: 35 [50.0%]  (CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L124:
	movapd	%xmm1, %xmm2	# D.6237, D.6237
	movl	$3, %esi	#, nfft
# SUCC: 43 [100.0%]  (CAN_FALLTHRU)
	jmp	.L100	#
# BLOCK 65 freq:278 seq:63
# PRED: 55 [50.0%]  (CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L164:
	movl	$3, (%rcx)	#, *facbuf_111
	leaq	8(%rcx), %rsi	#, facbuf
	movl	%edx, %ebx	# tmp213, nfft
	movl	%edx, 4(%rcx)	# nfft, MEM[(int *)facbuf_111 + 4B]
	cmpl	$1, %edx	#, nfft
# SUCC: 66 [91.0%]  (FALLTHRU,CAN_FALLTHRU) 24 [9.0%]  (CAN_FALLTHRU,LOOP_EXIT)
	jle	.L136	#,
# BLOCK 66 freq:62 seq:64
# PRED: 65 [91.0%]  (FALLTHRU,CAN_FALLTHRU)
	movl	%edx, %eax	# nfft, tmp347
	movq	%rsi, %rcx	# facbuf, facbuf
	movl	$3, %esi	#, nfft
	imull	%edi	# tmp335
	movl	%ebx, %eax	# nfft, tmp243
	sarl	$31, %eax	#, tmp243
	subl	%eax, %edx	# tmp243, tmp240
	leal	(%rdx,%rdx,2), %eax	#, tmp246
	cmpl	%eax, %ebx	# tmp246, nfft
# SUCC: 33 [50.0%]  (CAN_FALLTHRU) 67 [50.0%]  (FALLTHRU)
	jne	.L87	#,
# BLOCK 67 freq:31 seq:65
# PRED: 66 [50.0%]  (FALLTHRU)
# SUCC: 23 [100.0%]  (CAN_FALLTHRU)
	jmp	.L86	#
# BLOCK 68 freq:278 seq:66
# PRED: 58 [50.0%]  (CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L115:
	movsd	.LC1(%rip), %xmm1	#, D.6237
	movl	$5, %esi	#, nfft
# SUCC: 63 [100.0%]  (CAN_FALLTHRU)
	jmp	.L80	#
# BLOCK 69 freq:186 seq:67
# PRED: 37 [50.0%]  (CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L125:
	movl	%esi, %ebx	# nfft, nfft
	movl	$2, %esi	#, nfft
# SUCC: 44 [100.0%]  (CAN_FALLTHRU)
	jmp	.L101	#
# BLOCK 70 freq:123 seq:68
# PRED: 40 [33.3%]  (CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L122:
	movl	%ebx, %esi	# nfft, nfft
# SUCC: 39 [100.0%]  (DFS_BACK,CAN_FALLTHRU)
	jmp	.L85	#
# BLOCK 71 freq:41 seq:69
# PRED: 7 [9.0%]  (CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L158:
	pxor	%xmm7, %xmm7	# D.6237
	cvtsi2sd	%ebx, %xmm7	# nfft, D.6237
	movsd	%xmm7, 24(%rsp)	# D.6237, %sfp
# SUCC: 10 [100.0%]  (CAN_FALLTHRU)
	jmp	.L106	#
# BLOCK 72 freq:34 seq:70
# PRED: 2 [6.7%]  (CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L157:
	movl	%esi, (%rsp)	# inverse_fft, %sfp
	call	malloc	#
	movl	(%rsp), %esi	# %sfp, inverse_fft
	movq	%rax, %r14	#, mem
# SUCC: 6 [100.0%]  (CAN_FALLTHRU)
	jmp	.L65	#
	.cfi_endproc
.LFE79:
	.size	kiss_fft_alloc, .-kiss_fft_alloc
	.section	.text.unlikely
.LCOLDE11:
	.text
.LHOTE11:
	.section	.text.unlikely
.LCOLDB12:
	.text
.LHOTB12:
	.p2align 4,,15
	.globl	kiss_fft_stride
	.type	kiss_fft_stride, @function
kiss_fft_stride:
.LFB80:
	.cfi_startproc
# BLOCK 2 freq:10000 seq:0
# PRED: ENTRY [100.0%]  (FALLTHRU)
	cmpq	%rdx, %rsi	# fout, fin
# SUCC: 4 [10.1%]  (CAN_FALLTHRU) 3 [89.9%]  (FALLTHRU,CAN_FALLTHRU)
	je	.L168	#,
# BLOCK 3 freq:8986 seq:1
# PRED: 2 [89.9%]  (FALLTHRU,CAN_FALLTHRU)
	leaq	8(%rdi), %r8	#,
	movq	%rdx, %rax	# fout, fout
	movq	%rdi, %r9	# st,
	movl	$1, %edx	#,
	movq	%rax, %rdi	# fout,
# SUCC: EXIT [100.0%]  (ABNORMAL,SIBCALL)
	jmp	kf_work	#
# BLOCK 4 freq:1014 seq:2
# PRED: 2 [10.1%]  (CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L168:
	pushq	%r13	#
	.cfi_def_cfa_offset 16
	.cfi_offset 13, -16
	pushq	%r12	#
	.cfi_def_cfa_offset 24
	.cfi_offset 12, -24
	movl	%ecx, %r12d	# in_stride, in_stride
	pushq	%rbp	#
	.cfi_def_cfa_offset 32
	.cfi_offset 6, -32
	movq	%rsi, %rbp	# fin, fin
	pushq	%rbx	#
	.cfi_def_cfa_offset 40
	.cfi_offset 3, -40
	movq	%rdi, %rbx	# st, st
	subq	$8, %rsp	#,
	.cfi_def_cfa_offset 48
	movslq	(%rdi), %rdi	# st_4(D)->nfft, D.6271
	salq	$2, %rdi	#, D.6271
	call	malloc	#
	movq	%rbx, %r9	# st,
	movl	%r12d, %ecx	# in_stride,
	movq	%rbp, %rsi	# fin,
	leaq	8(%rbx), %r8	#,
	movq	%rax, %rdi	# tmp103,
	movl	$1, %edx	#,
	movq	%rax, %r13	#, tmp103
	call	kf_work	#
	movslq	(%rbx), %rdx	# st_4(D)->nfft, D.6271
	movq	%rbp, %rdi	# fin,
	movq	%r13, %rsi	# tmp103,
	salq	$2, %rdx	#, D.6271
	call	memcpy	#
	addq	$8, %rsp	#,
	.cfi_def_cfa_offset 40
	movq	%r13, %rdi	# tmp103,
	popq	%rbx	#
	.cfi_restore 3
	.cfi_def_cfa_offset 32
	popq	%rbp	#
	.cfi_restore 6
	.cfi_def_cfa_offset 24
	popq	%r12	#
	.cfi_restore 12
	.cfi_def_cfa_offset 16
	popq	%r13	#
	.cfi_restore 13
	.cfi_def_cfa_offset 8
# SUCC: EXIT [100.0%]  (ABNORMAL,SIBCALL)
	jmp	free	#
	.cfi_endproc
.LFE80:
	.size	kiss_fft_stride, .-kiss_fft_stride
	.section	.text.unlikely
.LCOLDE12:
	.text
.LHOTE12:
	.section	.text.unlikely
.LCOLDB13:
	.text
.LHOTB13:
	.p2align 4,,15
	.globl	kiss_fft
	.type	kiss_fft, @function
kiss_fft:
.LFB81:
	.cfi_startproc
# BLOCK 2 freq:10000 seq:0
# PRED: ENTRY [100.0%]  (FALLTHRU)
	cmpq	%rdx, %rsi	# fout, fin
# SUCC: 4 [10.1%]  (CAN_FALLTHRU) 3 [89.9%]  (FALLTHRU,CAN_FALLTHRU)
	je	.L172	#,
# BLOCK 3 freq:8986 seq:1
# PRED: 2 [89.9%]  (FALLTHRU,CAN_FALLTHRU)
	movq	%rdx, %rax	# fout, fout
	movq	%rdi, %r9	# cfg,
	movl	$1, %ecx	#,
	leaq	8(%rdi), %r8	#,
	movl	$1, %edx	#,
	movq	%rax, %rdi	# fout,
# SUCC: EXIT [100.0%]  (ABNORMAL,SIBCALL)
	jmp	kf_work	#
# BLOCK 4 freq:1014 seq:2
# PRED: 2 [10.1%]  (CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L172:
	pushq	%r12	#
	.cfi_def_cfa_offset 16
	.cfi_offset 12, -16
	pushq	%rbp	#
	.cfi_def_cfa_offset 24
	.cfi_offset 6, -24
	movq	%rsi, %rbp	# fin, fin
	pushq	%rbx	#
	.cfi_def_cfa_offset 32
	.cfi_offset 3, -32
	movq	%rdi, %rbx	# cfg, cfg
	movslq	(%rdi), %rdi	# cfg_2(D)->nfft, D.6292
	salq	$2, %rdi	#, D.6292
	call	malloc	#
	movq	%rbx, %r9	# cfg,
	movq	%rbp, %rsi	# fin,
	movl	$1, %ecx	#,
	leaq	8(%rbx), %r8	#,
	movq	%rax, %rdi	# tmp102,
	movl	$1, %edx	#,
	movq	%rax, %r12	#, tmp102
	call	kf_work	#
	movslq	(%rbx), %rdx	# cfg_2(D)->nfft, D.6292
	movq	%rbp, %rdi	# fin,
	movq	%r12, %rsi	# tmp102,
	salq	$2, %rdx	#, D.6292
	call	memcpy	#
	popq	%rbx	#
	.cfi_restore 3
	.cfi_def_cfa_offset 24
	movq	%r12, %rdi	# tmp102,
	popq	%rbp	#
	.cfi_restore 6
	.cfi_def_cfa_offset 16
	popq	%r12	#
	.cfi_restore 12
	.cfi_def_cfa_offset 8
# SUCC: EXIT [100.0%]  (ABNORMAL,SIBCALL)
	jmp	free	#
	.cfi_endproc
.LFE81:
	.size	kiss_fft, .-kiss_fft
	.section	.text.unlikely
.LCOLDE13:
	.text
.LHOTE13:
	.section	.text.unlikely
.LCOLDB14:
	.text
.LHOTB14:
	.p2align 4,,15
	.globl	kiss_fft_cleanup
	.type	kiss_fft_cleanup, @function
kiss_fft_cleanup:
.LFB82:
	.cfi_startproc
# BLOCK 2 freq:10000 seq:0
# PRED: ENTRY [100.0%]  (FALLTHRU)
# SUCC: EXIT [100.0%] 
	ret
	.cfi_endproc
.LFE82:
	.size	kiss_fft_cleanup, .-kiss_fft_cleanup
	.section	.text.unlikely
.LCOLDE14:
	.text
.LHOTE14:
	.section	.text.unlikely
.LCOLDB15:
	.text
.LHOTB15:
	.p2align 4,,15
	.globl	kiss_fft_next_fast_size
	.type	kiss_fft_next_fast_size, @function
kiss_fft_next_fast_size:
.LFB83:
	.cfi_startproc
# BLOCK 2 freq:81 seq:0
# PRED: ENTRY [100.0%]  (FALLTHRU)
	movl	$1431655766, %r8d	#, tmp154
# SUCC: 3 [100.0%]  (FALLTHRU,CAN_FALLTHRU)
	movl	$1717986919, %esi	#, tmp155
# BLOCK 3 freq:900 seq:1
# PRED: 2 [100.0%]  (FALLTHRU,CAN_FALLTHRU) 12 [100.0%]  (DFS_BACK,CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L182:
	movl	%edi, %ecx	# n, n
	testb	$1, %dil	#, n
# SUCC: 4 [91.0%]  (FALLTHRU,CAN_FALLTHRU) 7 [9.0%]  (CAN_FALLTHRU)
	jne	.L175	#,
# BLOCK 4 freq:9100 seq:2
# PRED: 3 [91.0%]  (FALLTHRU,CAN_FALLTHRU) 4 [91.0%]  (DFS_BACK,CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L176:
	movl	%ecx, %eax	# n, tmp104
	shrl	$31, %eax	#, tmp104
	addl	%eax, %ecx	# tmp104, tmp105
	sarl	%ecx	# n
	testb	$1, %cl	#, n
# SUCC: 4 [91.0%]  (DFS_BACK,CAN_FALLTHRU) 5 [9.0%]  (FALLTHRU)
	je	.L176	#,
# BLOCK 5 freq:819 seq:3
# PRED: 4 [9.0%]  (FALLTHRU)
# SUCC: 7 [100.0%]  (CAN_FALLTHRU,LOOP_EXIT)
	jmp	.L175	#
# BLOCK 6 freq:9100 seq:4
# PRED: 7 [91.0%]  (CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L190:
	movl	%ecx, %eax	# n, tmp157
	sarl	$31, %ecx	#, tmp119
	imull	%r8d	# tmp154
	subl	%ecx, %edx	# tmp119, n
# SUCC: 7 [100.0%]  (FALLTHRU,DFS_BACK,CAN_FALLTHRU)
	movl	%edx, %ecx	# n, n
# BLOCK 7 freq:10000 seq:5
# PRED: 3 [9.0%]  (CAN_FALLTHRU) 6 [100.0%]  (FALLTHRU,DFS_BACK,CAN_FALLTHRU) 5 [100.0%]  (CAN_FALLTHRU,LOOP_EXIT)
.L175:
	movl	%ecx, %eax	# n, tmp156
	imull	%r8d	# tmp154
	movl	%ecx, %eax	# n, tmp111
	sarl	$31, %eax	#, tmp111
	subl	%eax, %edx	# tmp111, tmp108
	leal	(%rdx,%rdx,2), %eax	#, tmp114
	cmpl	%eax, %ecx	# tmp114, n
# SUCC: 6 [91.0%]  (CAN_FALLTHRU) 8 [9.0%]  (FALLTHRU)
	je	.L190	#,
# BLOCK 8 freq:900 seq:6
# PRED: 7 [9.0%]  (FALLTHRU)
# SUCC: 10 [100.0%]  (CAN_FALLTHRU,LOOP_EXIT)
	jmp	.L177	#
# BLOCK 9 freq:9100 seq:7
# PRED: 10 [91.0%]  (CAN_FALLTHRU)
	.p2align 4,,10
	.p2align 3
.L191:
	movl	%ecx, %eax	# n, tmp160
	sarl	$31, %ecx	#, tmp141
	imull	%esi	# tmp155
	sarl	%edx	# tmp140
	subl	%ecx, %edx	# tmp141, n
# SUCC: 10 [100.0%]  (FALLTHRU,DFS_BACK,CAN_FALLTHRU)
	movl	%edx, %ecx	# n, n
# BLOCK 10 freq:10000 seq:8
# PRED: 9 [100.0%]  (FALLTHRU,DFS_BACK,CAN_FALLTHRU) 8 [100.0%]  (CAN_FALLTHRU,LOOP_EXIT)
.L177:
	movl	%ecx, %eax	# n, tmp159
	imull	%esi	# tmp155
	movl	%ecx, %eax	# n, tmp132
	sarl	$31, %eax	#, tmp132
	sarl	%edx	# tmp131
	subl	%eax, %edx	# tmp132, tmp128
	leal	(%rdx,%rdx,4), %eax	#, tmp135
	cmpl	%eax, %ecx	# tmp135, n
# SUCC: 9 [91.0%]  (CAN_FALLTHRU) 11 [9.0%]  (FALLTHRU,CAN_FALLTHRU,LOOP_EXIT)
	je	.L191	#,
# BLOCK 11 freq:900 seq:9
# PRED: 10 [9.0%]  (FALLTHRU,CAN_FALLTHRU,LOOP_EXIT)
	cmpl	$1, %ecx	#, n
# SUCC: 13 [9.0%]  (CAN_FALLTHRU,LOOP_EXIT) 12 [91.0%]  (FALLTHRU,CAN_FALLTHRU)
	jle	.L181	#,
# BLOCK 12 freq:819 seq:10
# PRED: 11 [91.0%]  (FALLTHRU,CAN_FALLTHRU)
	addl	$1, %edi	#, n
# SUCC: 3 [100.0%]  (DFS_BACK,CAN_FALLTHRU)
	jmp	.L182	#
# BLOCK 13 freq:81 seq:11
# PRED: 11 [9.0%]  (CAN_FALLTHRU,LOOP_EXIT)
.L181:
	movl	%edi, %eax	# n,
# SUCC: EXIT [100.0%] 
	ret
	.cfi_endproc
.LFE83:
	.size	kiss_fft_next_fast_size, .-kiss_fft_next_fast_size
	.section	.text.unlikely
.LCOLDE15:
	.text
.LHOTE15:
	.section	.rodata.cst8,"aM",@progbits,8
	.align 8
.LC1:
	.long	0
	.long	1075052544
	.align 8
.LC2:
	.long	0
	.long	1074266112
	.align 8
.LC3:
	.long	0
	.long	1127219200
	.section	.rodata.cst16,"aM",@progbits,16
	.align 16
.LC4:
	.long	4294967295
	.long	2147483647
	.long	0
	.long	0
	.section	.rodata.cst8
	.align 8
.LC5:
	.long	0
	.long	1073741824
	.align 8
.LC6:
	.long	0
	.long	1072693248
	.align 8
.LC7:
	.long	1413754136
	.long	-1072094725
	.align 8
.LC8:
	.long	0
	.long	1088421824
	.align 8
.LC9:
	.long	0
	.long	1071644672
	.section	.rodata.cst16
	.align 16
.LC10:
	.long	0
	.long	-2147483648
	.long	0
	.long	0
	.ident	"GCC: (Ubuntu 5.4.0-6ubuntu1~16.04.4) 5.4.0 20160609"
	.section	.note.GNU-stack,"",@progbits
