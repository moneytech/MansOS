/*
 * Copyright (c) 2008-2013 the MansOS team. All rights reserved.
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *  * Redistributions of source code must retain the above copyright notice,
 *    this list of  conditions and the following disclaimer.
 *  * Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS OR
 * CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
 * EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
 * PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS;
 * OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY,
 * WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
 * OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF
 * ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */

#ifndef _MANSOS_DEFINES_H_
#define _MANSOS_DEFINES_H_

/// \file
/// Platform independent defines and constants.
/// Also includes platform dependent defines.
///

#include "stdtypes.h"
#include <stdlib.h> // include file where NULL is defined
#include <limits.h> // INT_MAX etc
#include <string.h> // memcpy()

//
// Compiler specific function and data attributes
//
#if defined __GNUC__
#include "defines_gcc.h"
#elif defined __IAR_SYSTEMS_ICC__
#include "defines_iar.h"
#elif defined SDCC
#include "defines_sdcc.h"
#else
#error Unsupported compiler
#endif

//! Compile time assert. Useful for type size checking and similar checks
#define COMPILE_TIME_ASSERT(e, name)                 \
    struct CTA_##name { char name[(e) - 1]; }


//
// Build a single uint16_t from 2 bytes
#define MAKE_U16(a, b)         ((uint16_t) (((a) << 8) | (b)))
// Build a single uint32_t from 4 bytes. Handy for creating IP addresses
#define MAKE_U32(a, b, c, d)   (((uint32_t) (a) << 24) | ((uint32_t) (b) << 16ul) | ((c) << 8) | (d))

//! MIN of two values
#define MIN(a, b) ((a) < (b) ? (a) : (b))
//! MAX of two values
#define MAX(a, b) ((a) < (b) ? (b) : (a))

//! Determine the number of parameters passed to a macro 
#define COUNT_PARMS2(_1, _2, _3, _4, _5, _6, _7, _8, _9, _10, _, ...) _
#define COUNT_PARMS(...) \
    COUNT_PARMS2(__VA_ARGS__, 10, 9, 8, 7, 6, 5, 4, 3, 2, 1)

//! Determine the length of an array at compile-time
#define ARRAYLEN(a) (sizeof(a) / sizeof(*(a)))


//! Is 'a' is after 'b'? Use for comparing time values. Handles overflow correctly.
#define timeAfter16(a, b) ((int16_t) ((b) - (a)) < 0)
//! Is 'a' is after 'b'? 32-bit version.
#define timeAfter32(a, b) ((int32_t) ((b) - (a)) < 0)
//! Is 'a' is after 'b'? 64-bit version.
#define timeAfter64(a, b) ((int64_t) ((b) - (a)) < 0)

#ifdef USE_LONG_LIFETIME
//! Define jiffies type as 64-bit number
typedef uint64_t ticks_t;
#define timeAfter(a, b) timeAfter64(a, b)
#else
//! Define jiffies type as 32-bit number (correct for up to 40 days)
typedef uint32_t ticks_t;
#define timeAfter(a, b) timeAfter32(a, b)
#endif


// Alignment check and ops: work for powers of 2

//! Check if 16-bit addr is aligned to align, which must be a power of 2
#define IS_ALIGNED(addr, align) (!((uint16_t)(addr) & (align - 1)))
//! Align 16-bit addr down to align, which must be a power of 2
#define ALIGN_DOWN(addr, align) ((uint16_t)(addr) & ~(align - 1))
//! Align 16-bit addr up to align, which must be a power of 2
#define ALIGN_UP(addr, align) (((uint16_t)(addr) + (align - 1)) & ~(align - 1))
//! Align 32-bit addr down to align, which must be a power of 2
#define ALIGN_DOWN_U32(addr, align) ((uint32_t)(addr) & ~(align - 1))
//! Align 32-bit addr up to align, which must be a power of 2
#define ALIGN_UP_U32(addr, align) (((uint32_t)(addr) + (align - 1)) & ~(align - 1))

//! Get the most significant byte (MSB) of a 2-byte word
#define MSB(a) ((a & 0xFF00) >> 8)
//! Get the least significant byte (LSB) of a 2-byte word
#define LSB(a) ((a & 0xFF))


#ifndef SERIAL_BAUDRATE
//! Serial port default baudrate
#define SERIAL_BAUDRATE 38400
#endif


#ifndef CPU_MHZ
#error CPU_MHZ must be defined!
#endif

#if defined MCU_MSP430 || defined USE_FATFS
#define MANSOS_STDIO 1
#else
#define MANSOS_STDIO 0
#endif

#if MCU_AVR
//! Atmel has straight decimal MHz
#define CPU_HZ (CPU_MHZ * 1000000ul)
#else
//! msp430-based devices usually have binary MHz
#define CPU_HZ (CPU_MHZ * 1024ul * 1024)
#endif

#ifndef TIMER_INTERRUPT_HZ
//! timer frequency, times per second. 100 Hz is also a supported value
#define TIMER_INTERRUPT_HZ  1000
#endif

//! timer A feeds from from ACLK (32'768 Hz)
#define TIMER_SECOND ACLK_SPEED 
#define TIMER_100_MS (ACLK_SPEED / 10 + 1)

#if MCU_MSP430
//! MSP430 has binary timer frequency that introduces rounding error in milliseconds
# if TIMER_INTERRUPT_HZ == 100
# define CORRECTION_PER_WRAPAROUND   -4
# elif TIMER_INTERRUPT_HZ == 1000
# define CORRECTION_PER_WRAPAROUND   -48
# else
# warning Strange alarm timer frequency!
# define CORRECTION_PER_WRAPAROUND   0
# endif
#else
//! Atmega has decimal timer Hz, correction is not needed
# define CORRECTION_PER_WRAPAROUND   0
#endif

///
/// This idiom should be used for reading active timers
///
#define ACTIVE_TIMER_READ(name, timer)             \
static inline uint16_t name ## _TIMER_READ(void)   \
{                                                  \
    uint16_t t1 = timer;                           \
    uint16_t t2;                                   \
    do {                                           \
        t2 = timer;                                \
    } while (t1 == t2);                            \
    return t2;                                     \
}


#endif