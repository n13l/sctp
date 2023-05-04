/*
 * The MIT License (MIT)         Copyright (c) 2017 Daniel Kubec <niel@rtfm.cz> 
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy 
 * of this software and associated documentation files (the "Software"),to deal
 * in the Software without restriction, including without limitation the rights 
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

#ifndef __OPTIMIZED_INDEX_H__
#define __OPTIMIZED_INDEX_H__

#include <sys/compiler.h>

/*
 * Branch misprediction is very expensive. But conditional moves are also
 * costly for a few reasons.
 *
 * Latency
 *
 * conditional move instructions typically have higher latency compared to
 * regular  move instructions. In x86-64, a conditional move instruction can
 * have a latency of 2 or more cycles, while a regular MOV instruction typically
 * has a latency of 1 cycle. This increased latency can lead to performance
 * penalties if the instruction is in the critical path of the execution.
 *
 * Throughput
 *
 * The throughput of conditional move instructions can also be lower than that
 * of regular move instructions. For example, on some x86-64 microarchitectures,
 * conditional move instructions have a throughput of 1 cycle, whereas MOV
 * instructions can have a throughput of 0.5 cycles. This means that conditional
 * move instructions can take more time to complete, especially when they are 
 * executed in a sequence.
 *
 * Pipeline Stalls
 *
 * Modern CPUs rely on pipelining and out-of-order execution to
 * achieve high performance. In some cases, the conditional move instruction can
 * cause pipeline stalls or increase the dependency chain between instructions.
 * This can lead to reduced performance as the CPU has to wait for the
 * conditional move instruction to complete before it can execute the following
 * instructions.
 *
 * Branch Prediction
 *
 * While conditional move instructions can help eliminate branch 
 * mispredictions by avoiding the need for explicit branches, 
 * they still depend on the processor's ability to predict the outcome of the
 * condition. If the condition is hard to predict, the processor might not be
 * able to efficiently schedule the conditional move instruction, leading to
 * performance penalties.
 *
 * Using a function based on bitwise operations can help mitigate some of these
 * performance penalties associated with conditional moves. Bitwise operations,
 * such as AND, OR, and XOR, typically have lower latencies and higher
 * throughput compared to conditional move instructions. Additionally, they can
 * be more easily scheduled by the CPU's out-of-order execution engine, which
 * can help reduce pipeline stalls and dependency chains.
 */

#define USE_VERIFY_0_BASED_INDEX32_POW2_BITWISE 0
#define USE_VERIFY_0_BASED_INDEX32_POW2_TERNARY 1
#define USE_VERIFY_1_BASED_INDEX32_POW2_BITWISE 1
#define USE_VERIFY_1_BASED_INDEX32_POW2_TERNARY 0

/*
 * verify_0bi32pow2_bw() should take roughly 7 CPU cycles.
 * However, this is a rough estimate and may vary depending on the CPU 
 * architecture, compiler optimizations, and other factors.
 *
 * xor     edx, edx
 * cmp     esi, edi
 * setb    dl
 * lea     eax, [rdx-1]
 * neg     edx
 * and     eax, edi
 * and     edx, esi
 * or      eax, edx
 */

static inline u32
verify_0bi32pow2_bw(u32 index, u32 mask)
{
	unsigned cond = -((index) > (mask));
	return (((index) & ~cond) | ((mask) & cond));
}

/*
 * verify_0bi32pow2_tn() should take roughly 4 CPU cycles.
 * However, this is a rough estimate and may vary depending on the CPU
 * architecture, compiler optimizations, and other factors.
 *
 * cmp     edi, esi
 * mov     eax, esi
 * cmovbe  eax, edi
 */

static inline u32
verify_0bi32pow2_tn(u32 index, u32 mask)
{
	return index > mask ? mask: index;
}

/*
 * verify_1bi32pow2_bw() should take roughly 5 CPU cycles.
 * However, this is a rough estimate and may vary depending on the CPU
 * architecture, compiler optimizations, and other factors.
 *
 * xor     eax, eax
 * test    edi, edx
 * sete    al
 * neg     eax
 * and     eax, edi
 */

#define verify_1bi32pow2_bw(index, mask, msb) ({ \
	(index & (-(!(index & msb)))); \
})

/*
 * verify_1bi32pow2_tn() should take roughly 5 CPU cycles.
 * However, this is a rough estimate and may vary depending on the CPU
 * architecture, compiler optimizations, and other factors.
 *
 * xor     edx, edx
 * mov     eax, edi
 * cmp     esi, edi
 * cmovb   eax, edx
 */

#define verify_1bi32pow2_tn(index, mask, msb) ({ \
	(index <= mask ? index: 0); \
})

#if USE_VERIFY_0_BASED_INDEX32_POW2_BITWISE == 1
#define verify_0bi32pow2(index, mask) \
	({ verify_0bi32pow2_bw(index, mask); })
#endif
#if USE_VERIFY_0_BASED_INDEX32_POW2_TERNARY == 1
#define verify_0bi32pow2(index, mask) \
	({ verify_0bi32pow2_tn(index, mask); })
#endif

#if USE_VERIFY_1_BASED_INDEX32_POW2_BITWISE == 1
#define verify_1bi32pow2(index, mask, msb) \
	({ verify_1bi32pow2_bw(index, mask, msb); })
#endif
#if USE_VERIFY_1_BASED_INDEX32_POW2_TERNARY == 1
#define verify_1bi32pow2(index, mask, msb) \
	({ verify_1bi32pow2_tn(index, mask, msb); })
#endif

#endif
