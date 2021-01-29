/* SPDX-License-Identifier: GPL-2.0 */
/* Copyright (C) 2015-2020 Intel Corporation. All rights reserved. */
#ifndef _NDCTL_BITMAP_H_
#define _NDCTL_BITMAP_H_

#include <linux/const.h>
#include <util/size.h>
#include <util/util.h>
#include <ccan/short_types/short_types.h>

#ifndef _UL
#define _UL(x)		(_AC(x, UL))
#endif
#ifndef _ULL
#define _ULL(x)		(_AC(x, ULL))
#endif

#define DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#define UL(x)		(_UL(x))
#define ULL(x)		(_ULL(x))

/* GENMASK() and its dependencies copied from include/linux/{bits.h, const.h} */
#define __is_constexpr(x) \
	(sizeof(int) == sizeof(*(8 ? ((void *)((long)(x) * 0l)) : (int *)8)))
#define GENMASK_INPUT_CHECK(h, l) \
	(BUILD_BUG_ON_ZERO(__builtin_choose_expr( \
		__is_constexpr((l) > (h)), (l) > (h), 0)))
#define __GENMASK(h, l) \
	(((~UL(0)) - (UL(1) << (l)) + 1) & \
	 (~UL(0) >> (BITS_PER_LONG - 1 - (h))))
#define GENMASK(h, l) \
	(GENMASK_INPUT_CHECK(h, l) + __GENMASK(h, l))

#define BIT(nr)			(1UL << (nr))
#define BIT_MASK(nr)		(1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)		((nr) / BITS_PER_LONG)
#define BITS_PER_BYTE		8
#define BITS_TO_LONGS(nr)	DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))

#define BITMAP_FIRST_WORD_MASK(start) (~0UL << ((start) & (BITS_PER_LONG - 1)))
#define BITMAP_LAST_WORD_MASK(nbits) (~0UL >> (-(nbits) & (BITS_PER_LONG - 1)))

#define small_const_nbits(nbits) \
	(__builtin_constant_p(nbits) && (nbits) <= BITS_PER_LONG)

unsigned long *bitmap_alloc(unsigned long nbits);
void bitmap_set(unsigned long *map, unsigned int start, int len);
void bitmap_clear(unsigned long *map, unsigned int start, int len);
int test_bit(unsigned int nr, const volatile unsigned long *addr);
unsigned long find_next_bit(const unsigned long *addr, unsigned long size,
			    unsigned long offset);
unsigned long find_next_zero_bit(const unsigned long *addr, unsigned long size,
				 unsigned long offset);
int bitmap_full(const unsigned long *src, unsigned int nbits);


#endif /* _NDCTL_BITMAP_H_ */
