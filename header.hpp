#ifndef HEADER_HPP
# define HEADER_HPP

#include <limits.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>

/* === CONFIG OPTIONS === */

// #define USE_HW_CURSOR // doesn't support term.set_cursor()

#define PAGESIZE 4096

#define GDT_ENTRIES_ADDRESS 0x800

#define MAX_LINE_LEN 1024

#define IDT_ENTRIES 256

#define KEYBOARD_BUFFER_SIZE 256

#define CLOCK_FREQ 1000                 // Hz             // TODO: benchmark avg process switch time and set CLOCK_FREQ accordingly
#define MS_INTERVAL (1000 / CLOCK_FREQ) // interval in ms

#define KERNEL_STACK_SIZE 16384

#define TASK_NAME_LEN 32


#ifdef DEBUG
// # define DEBUG_GDT
// # define DEBUG_MM
// # define    DEBUG_MM_SWITCH
// # define DEBUG_KHEAP
// # define DEBUG_IDT
// # define DEBUG_INTERRUPTS
// # define DEBUG_TIMER
// # define DEBUG_KBD
// # define DEBUG_SCHED
// # define    DEBUG_SCHED_SWITCH
#endif

/* === THE END === */


#define LOG_HDR "\1"
#define KERN_EMERG   LOG_HDR "0" // Used for emergency messages, usually those that precede a crash.
#define KERN_ALERT   LOG_HDR "1" // A situation requiring immediate action.
#define KERN_CRIT    LOG_HDR "2" // Critical conditions, often related to serious hardware or software failures.
#define KERN_ERROR   LOG_HDR "3" // Used to report error conditions.
#define KERN_WARNING LOG_HDR "4" // Warnings about problematic situations that do not, in themselves, create serious problems with the system.
#define KERN_NOTICE  LOG_HDR "5" // Situations that are normal, but still worthy of note. A number of security-related conditions are reported at this level.
#define KERN_INFO    LOG_HDR "6" // Informational messages. Many drivers print information about the hardware they find at startup time at this level.
#define KERN_DEBUG   LOG_HDR "7" // Used for debugging messages.

// log levels
#define LL_EMERG 0
#define LL_ALERT 1
#define LL_CRIT 2
#define LL_ERROR 3
#define LL_WARNING 4
#define LL_NOTICE 5
#define LL_INFO 6
#define LL_DEBUG 7

#define LOG_MM "%13gmm%g: "
#define LOG_KHEAP "%13gkheap%g: "
#define LOG_GDT "%13ggdt%g: "
#define LOG_IDT "%13gidt%g: "
#define LOG_TIMER "%13gtimer%g: "
#define LOG_SCHED "%13gsched%g: "


typedef uint64_t u64;
typedef uint32_t u32;
typedef uint16_t u16;
typedef uint8_t u8;

typedef int64_t i64;
typedef int32_t i32;
typedef int16_t i16;
typedef int8_t i8;

typedef unsigned char uc;




#endif
