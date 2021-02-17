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

#ifndef __GENERIC_ARRAY_H__
#define __GENERIC_ARRAY_H__

#include <sys/compiler.h>
#include <limits.h>
#include <assert.h>

/**
 * ARRAY_SIZE()
 * Get size of array
 *
 * @arr:           Name of array.
 */

#define ARRAY_SIZE(arr) (sizeof(arr)/sizeof(*(arr)))

#define ARRAY_BITS(size) ({ __typeof__(size) __bits = log2(size) + 1; __bits; })

#define FIELD_SIZE(s, m) (sizeof(((__typeof__(s))0)->m))

#define FIELD_INC_AT(ptr, offset, type) \
	(*((type*)(((u8*)ptr) + offset)))++;

#define FIELD_DEC_AT(ptr, offset, type) \
	(*((type*)(((u8*)ptr) + offset))--)

#define FIELD_ADD_AT(ptr, offset, type, val) \
	(*((type*)(((u8*)ptr) + offset)) += (val))

#define FIELD_SET_AT(ptr, offset, type, val) \
	(*((type*)(((u8*)ptr) + offset))) = val

#define FIELD_GET_AT(ptr, offset, type, val) \
	val = (*((type*)(((u8*)ptr) + offset))) 

#define DEFINE_STATIC_ARRAY_ALIGNED_4BIT(type, name, item, ...) \
	_Pragma("clang diagnostic push") \
	_Pragma("clang diagnostic ignored \"-Winitializer-overrides\"") \
	static type name[1<<8] = {[0 ... ((1<<4)-1)] = item, __VA_ARGS__}; \
	_Pragma("clang diagnostic pop") \
	static inline type name##_fetch(u8 index) { return name[index & 0x0f];}\
	static inline u8 name##_index(u8 index) { return index & 0x0f; }

#define DEFINE_ARRAY_ALIGNED_4BIT(type, name, item, ...) \
	_Pragma("clang diagnostic push") \
	_Pragma("clang diagnostic ignored \"-Winitializer-overrides\"") \
	type name[1<<8] = {[0 ... ((1<<4)-1)] = item, __VA_ARGS__}; \
	_Pragma("clang diagnostic pop") \
	type name##_fetch(u8 index) { return name[index & 0x0F]; }

#define DEFINE_STATIC_ARRAY_ALIGNED_8BIT(type, name, item, ...) \
	_Pragma("clang diagnostic push") \
	_Pragma("clang diagnostic ignored \"-Winitializer-overrides\"") \
	static type name[1<<8] = {[0 ... ((1<<8)-1)] = item, __VA_ARGS__}; \
	_Pragma("clang diagnostic pop") \
	static inline type name##_fetch(u8 index) { return name[index]; } \
	static inline u8 name##_index(u8 index) { return index; }

#define DEFINE_ARRAY_ALIGNED_8BIT(type, name, item, ...) \
	_Pragma("clang diagnostic push") \
	_Pragma("clang diagnostic ignored \"-Winitializer-overrides\"") \
	type name[1<<8] = {[0 ... ((1<<8)-1)] = item, __VA_ARGS__}; \
	_Pragma("clang diagnostic pop") \
	type name##_fetch(u8 index) { return name[index]; }

#define DECLARE_ARRAY_ALIGNED_8BIT(type, name) \
	extern type name[1<<8];

/*
 * O(1) time branchless memory access with array[(1<<bits)-1] default element.
 *
 * If the value in it has side effects, the side effects happen only once, not
 * for each initialized field by the range initializer.
 * Requires GNU Extensions (-std=gnu99)
 */

/* O(1) time branchless access for zero-based and one-based assoc. arrays .*/
#define DEFINE_STATIC_ARRAY_POW2(type, name, bits, item, ...) \
	static const unsigned name##_mask = ~((1<<(bits))-1); \
	static const unsigned name##_bits = bits; \
	_Pragma("clang diagnostic push") \
	_Pragma("clang diagnostic ignored \"-Winitializer-overrides\"") \
	static type name[1 << (bits)] = { \
		[0 ... ((1<<(bits))-1)] = item, __VA_ARGS__ \
	}; \
	_Pragma("clang diagnostic pop") \
	/* zero-based array fetcher (0, 1, 2, ..., N − 2) */ \
	/* zero-based array with array[N - 1] default element. */ \
	static inline unsigned name##_index_zb(unsigned index) { \
		unsigned _x = !(name##_mask & index) * index; \
		return _x + ((index && !_x) * ((1<<bits)-1)); \
	} \
	static inline type name##_fetch_zb(unsigned index) { \
		return name[name##_index_zb(index)]; \
	} \
	/* one-based array getter (1, 2, ..., N − 1) */ \
	/* one-based array with array[0] default element. */ \
	static inline unsigned name##_index_ob(unsigned index) { \
		return !(name##_mask & index) * index; \
	} \
	static inline type name##_fetch_ob(unsigned index) { \
		return name[name##_index_ob(index)]; \
	} \
	/* branch based array with array[N - 1] default element. */ \
	static inline unsigned name##_index(unsigned index) { \
		return index < array_size(name) ? index: array_size(name) - 1;\
	} \
	static inline type name##_fetch(unsigned index) { \
		return name[name##_index(index)]; \
	}

/* O(1) time branchless access for zero-based and one-based arrays. */
#define DEFINE_ARRAY_POW2(type, name, bits, item, ...) \
	const unsigned name##_mask = ~((1<<(bits))-1); \
	const unsigned name##_bits = bits; \
	_Pragma("clang diagnostic push") \
	_Pragma("clang diagnostic ignored \"-Winitializer-overrides\"") \
	type name[1 << (bits)] = { \
		[0 ... ((1<<(bits))-1)] = item, __VA_ARGS__ \
	}; \
	_Pragma("clang diagnostic pop") \

/* O(1) time branchless access for zero-based and one-based arrays. */
#define DECLARE_ARRAY_POW2(type, name, bits) \
	extern const unsigned name##_mask; \
	extern const unsigned name##_bits; \
	extern type name[1 << (bits)]; \
	/* zero-based array index (0, 1, 2, ..., N − 2) */ \
	static inline unsigned name##_index_zb(unsigned index) { \
		unsigned _x = !(name##_mask & index) * index; \
		return _x + ((index && !_x) * ((1<<bits)-1)); \
	} \
	/* one-based array with array[0] default element. */ \
	static inline unsigned name##_index_ob(unsigned index) { \
		return !(name##_mask & index) * index; \
	} \
	/* zero-based array fetcher (0, 1, 2, ..., N − 2) */ \
	static inline type name##_fetch_zb(unsigned index) { \
		return name[name##_index_zb(index)]; \
	} \
	/* one-based array getter (1, 2, ..., N − 1) */ \
	static inline type name##_fetch_ob(unsigned index) { \
		return name[name##_index_ob(index)]; \
	} \
	/* branch based array with array[N - 1] default element. */ \
	static inline unsigned name##_index(unsigned index) { \
		return index < ARRAY_SIZE(name) ? index: ARRAY_SIZE(name) - 1;\
	} \
	static inline type name##_fetch(unsigned index) { \
		return name[name##_index(index)]; \
	}


/**
 * ARRAY_INDEX_OB()
 * Get in-bound index for one-based array
 *
 * @name:           Name of array.
 * @index:          Index of array.
 */

#define ARRAY_INDEX_OB(name, index) name##_index_ob(index)

/**
 * ARRAY_INDEX_ZB()
 * Get in-bound index for zero-based array
 *
 * @name:           Name of array.
 * @index:          Index of array.
 */

#define ARRAY_INDEX_ZB(name, index) name##_index_zb(index)

/**
 * ARRAY_FETCH_OB()
 * GET in-bound value for one-based array
 *
 * @name:           Name of array.
 * @fetch:          fetch of array.
 */

#define ARRAY_FETCH_OB(name, fetch) name##_fetch_ob(fetch)

/**
 * ARRAY_FETCH_ZB()
 * Get in-bound value for zero-based array
 *
 * @name:           Name of array.
 * @fetch:          fetch of array.
 */

#define ARRAY_FETCH_ZB(name, fetch) name##_fetch_zb(fetch)

/* Run block on bits of number */
#define VISIT_ARRAY_BITS_NUM(num, bit, block)                \
{                                                                    \
	const unsigned __count = sizeof(num) * 8; \
	const unsigned __typeof__(num) mask = (1 << (count - 1)); \
	do {bit = (num & mask) != 0?1:0; block; mask >>= 1;} while (mask > 0); \
}

#define BSEARCH_FIRST_GE_CMP(ary,N,x,ary_lt_x) \
({ \
	unsigned l = 0, r = (N); \
	while (l < r) { \
		unsigned m = (l+r)/2; \
		if (ary_lt_x(ary,m,x)) \
			l = m+1; \
		else \
			r = m; \
	} \
	l; \
})

#define ARRAY_LT_NUM(ary,i,x) (ary)[i] < (x)
#define BSEARCH_FIRST_GE(ary,N,x) BSEARCH_FIRST_GE_CMP(ary,N,x,ARRAY_LT_NUM)
#define BSEARCH_EQ(ary,N,x) \
({ \
	int i = BSEARCH_FIRST_GE(ary,N,x); \
	if (i >= (N) || (ary)[i] != (x)) i=-1; i; \
})

#endif
