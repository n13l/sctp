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

#ifndef __STREAMLINED_ARRAY_H__
#define __STREAMLINED_ARRAY_H__

#include <sys/compiler.h>
#include <bsd/index.h>
#include <limits.h>
#include <assert.h>

/*
 * A "streamlined array" integrates the principles of alignment based on
 * power-of-2, padding, forming an interconnected and harmoniously arranged 
 * collection of elements with efficient support. The term "streamlined array"
 * conveys the ideas of unity, consistency, and seamless interconnection,
 * adeptly encapsulating the essence of these characteristics for a highly
 * effective and well-organized array of elements.
 *
 * Additionally, a streamlined array with power-of-2 and padding is beneficial
 * for input validation. By adhering to a well-organized structure and optimized
 * padding, the array becomes more predictable and manageable. This uniformity
 * simplifies the validation process, as it allows for easier identification of
 * potential inconsistencies or errors in the input data. As a result, input
 * validation can be performed more effectively, ensuring that the data is
 * accurate and reliable before it is processed further.
 */

/*
 * The ARRAY_SIZE macro calculates the number of elements in a given array.
 *
 * - arr: The array for which the size should be determined.
 *
 * This macro computes the size by dividing the total size of the array
 * by the size of an individual element (in bytes).
 *
 * The result is the number of elements present in the array.
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

/*
 * The ARRAY_SIZE_0B_ARGS macro calculates the number of elements in a
 * zero-based array.
 *
 * - arr: The array for which the size should be determined.
 *
 * This macro computes the size by dividing the total size of the array
 * by the size of an individual element plus one anchor element.
 */

#define ARRAY_SIZE_0B_ARGS(type, ...) \
	((sizeof((type[]){__VA_ARGS__})/sizeof(type)) + 1)

#define ARRAY_BITS_0B_ARGS(type, ...) \
	(BUILD_TIME_NUM_BITS(ARRAY_SIZE_0B_ARGS(type,__VA_ARGS__)))

#define ARRAY_MASK_0B_ARGS(type, ...) \
	((1 << ARRAY_BITS_0B_ARGS(type,__VA_ARGS__)) - 1)

/*
 * The ARRAY_SIZE_1B_ARGS macro calculates the number of elements in a
 * one-based array.
 *
 * - arr: The array for which the size should be determined.
 *
 * This macro computes the size by dividing the total size of the array
 * by the size of an individual element including zero for anchor element.
 */

#define ARRAY_SIZE_1B_ARGS(type, ...) \
	((sizeof((type[]){__VA_ARGS__})/sizeof(type)))

#define ARRAY_BITS_1B_ARGS(type, ...) \
	(BUILD_TIME_NUM_BITS(ARRAY_SIZE_1B_ARGS(type,__VA_ARGS__)))

#define ARRAY_MASK_1B_ARGS(type, ...) \
	((1 << ARRAY_BITS_1B_ARGS(type,__VA_ARGS__)) - 1)

/*
 * The DEFINE_ARRAY_STREAMLINED macro creates a fixed-size, power-of-two,
 * zero-based array with a specified anchor element.
 * This allows for fast, branchless access to the array elements.
 *
 * - type: The data type of the elements in the array.
 * - name: The name of the array variable.
 * - anchor: The default element.
 * - __VA_ARGS__: A list of array elements to be initialized after the anchor.
 *
 * O(1) time branchless access for zero-based arrays.
 *
 * If the value in it has side effects, the side effects happen only once, not
 * for each initialized field by the range initializer.
 * Requires GNU Extensions (-std=gnu99)
 */

#define DEFINE_ARRAY_STREAMLINED(type, name, anchor, ...) \
	const u32 name##_size = ARRAY_SIZE_0B_ARGS(type,__VA_ARGS__);\
	const u32 name##_mask = ARRAY_MASK_0B_ARGS(type,__VA_ARGS__);\
	_Pragma("clang diagnostic push") \
	_Pragma("clang diagnostic ignored \"-Winitializer-overrides\"") \
	type name[1 << (ARRAY_BITS_0B_ARGS(type,__VA_ARGS__))] \
		__attribute__ ((aligned(16))) = { \
		[0 ... (ARRAY_MASK_0B_ARGS(type,__VA_ARGS__))] = \
		anchor, __VA_ARGS__ \
	}; \
	_Pragma("clang diagnostic pop") \

/*
 * The DECLARE_ARRAY_STREAMLINED macro is used to declare an external
 * streamlined array variable.
 *
 * - type: The data type of the elements in the array
 * - name: The name of the array variable.
 */

#define DECLARE_ARRAY_STREAMLINED(type, name) \
	extern const u32 name##_mask; \
	extern const u32 name##_size; \
	extern type name[]; \
	static inline u32 name##_verify(u32 index) { \
		return verify_0bi32pow2(index, name##_mask); \
	} \
	static inline type name##_at(u32 index) { \
		return name[name##_verify(index)]; \
	}

/*
 * The STATIC_ARRAY_STREAMLINED macro creates a static, fixed-size, power-of-two,
 * zero-based array with a specified anchor element.
 *
 * This allows for fast, branchless access to the array elements, while
 * keeping the scope of the array limited to the current source file.
 *
 * - type: The data type of the elements in the array.
 * - name: The name of the array variable.
 * - anchor: The default element.
 * - __VA_ARGS__: A list of array elements to be initialized after the anchor.
 */

#define STATIC_ARRAY_STREAMLINED(type, name, anchor, ...) \
	static const u32 name##_size = ARRAY_SIZE_0B_ARGS(type,__VA_ARGS__);\
	static const u32 name##_mask = ARRAY_MASK_0B_ARGS(type,__VA_ARGS__);\
	_Pragma("clang diagnostic push") \
	_Pragma("clang diagnostic ignored \"-Winitializer-overrides\"") \
	static type name[1 << (ARRAY_BITS_0B_ARGS(type,__VA_ARGS__))] \
		__attribute__ ((aligned(16))) = { \
		[0 ... (ARRAY_MASK_0B_ARGS(type,__VA_ARGS__))] = \
		anchor, __VA_ARGS__ \
	}; \
	_Pragma("clang diagnostic pop") \
	static inline u32 name##_verify(u32 index) { \
		return verify_0bi32pow2(index, name##_mask); \
	} \
	static inline type name##_at(u32 index) { \
		return name[name##_verify(index)]; \
	} \

/*
 * The DEFINE_ARRAY1_STREAMLINED macro creates a fixed-size,
 * power-of-two, one-based array with a specified anchor element.
 * This allows for fast, branchless access to the array elements.
 *
 * - type: The data type of the elements in the array.
 * - name: The name of the array variable.
 * - anchor: The default element.
 * - __VA_ARGS__: A list of array elements to be initialized after the anchor.
 *
 * O(1) time branchless access for zero-based arrays.
 *
 * If the value in it has side effects, the side effects happen only once, not
 * for each initialized field by the range initializer.
 * Requires GNU Extensions (-std=gnu99)
 */

#define DEFINE_ARRAY1_STREAMLINED(type, name, anchor, ...) \
	const u32 name##_size = ARRAY_SIZE_1B_ARGS(type,__VA_ARGS__);\
	const u32 name##_mask = ARRAY_MASK_1B_ARGS(type,__VA_ARGS__);\
	const u32 name##_msb = ~name##_mask; \
	_Pragma("clang diagnostic push") \
	_Pragma("clang diagnostic ignored \"-Winitializer-overrides\"") \
	type name[1 << (ARRAY_BITS_1B_ARGS(type,__VA_ARGS__))] \
		__attribute__ ((aligned(16))) = { \
		[0 ... (ARRAY_MASK_1B_ARGS(type,__VA_ARGS__))] = \
		anchor, __VA_ARGS__ \
	}; \
	_Pragma("clang diagnostic pop") \

/*
 * The DECLARE_ARRAY1_STREAMLINED macro is used to declare an external
 * streamlined array variable.
 *
 * - type: The data type of the elements in the array
 * - name: The name of the array variable.
 */

#define DECLARE_ARRAY1_STREAMLINED(type, name) \
	extern const u32 name##_mask; \
	extern const u32 name##_size; \
	extern const u32 name##_msb; \
	extern type name[]; \
	static inline u32 name##_verify(u32 index) { \
		return verify_1bi32pow2(index, name##_mask, name##_msb); \
	} \
	static inline type name##_at(u32 index) { \
		return name[name##_verify(index)]; \
	}

/*
 * The STATIC_ARRAY1_STREAMLINED macro creates a static, fixed-size,
 * power-of-two, one-based array with a specified anchor element.
 *
 * This allows for fast, branchless access to the array elements, while
 * keeping the scope of the array limited to the current source file.
 *
 * - type: The data type of the elements in the array.
 * - name: The name of the array variable.
 * - anchor: The default element.
 * - __VA_ARGS__: A list of array elements to be initialized after the anchor.
 */

#define STATIC_ARRAY1_STREAMLINED(type, name, anchor, ...) \
	static const u32 name##_size = ARRAY_SIZE_1B_ARGS(type,__VA_ARGS__);\
	static const u32 name##_mask = ARRAY_MASK_1B_ARGS(type,__VA_ARGS__);\
	static const u32 name##_msb = ~name##_mask; \
	_Pragma("clang diagnostic push") \
	_Pragma("clang diagnostic ignored \"-Winitializer-overrides\"") \
	static type name[1 << (ARRAY_BITS_1B_ARGS(type,__VA_ARGS__))] \
		__attribute__ ((aligned(16))) = { \
		[0 ... (ARRAY_MASK_1B_ARGS(type,__VA_ARGS__))] = \
		anchor, __VA_ARGS__ \
	}; \
	_Pragma("clang diagnostic pop") \
	static inline u32 name##_verify(u32 index) { \
		return verify_1bi32pow2(index, name##_mask, name##_msb); \
	} \
	static inline type name##_at(u32 index) { \
		return name[name##_verify(index)]; \
	} \

/* Run block on bits of number */
#define VISIT_ARRAY_BITS_NUM(num, bit, block)                \
{                                                                    \
	const unsigned __count = sizeof(num) * 8; \
	const unsigned __typeof__(num) mask = (1 << (count - 1)); \
	do {bit = (num & mask) != 0?1:0; block; mask >>= 1;} while (mask > 0); \
}

#endif
