#if !defined(BUILTIN_H)
#define BUILTIN_H
#include <stdint.h>

#if defined(__clang__)
  #define COMPILER_CLANG 1
  /* Platform */
  #if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS 1
  #elif defined(__APPLE__) && defined(__MACH__)
    #define PLATFORM_MACOS 1
  #elif defined(__linux__)
    #define PLATFORM_LINUX 1
  #else
    #define PLATFORM_UNKNOWN 1
  #endif
  /* Architecture */
  #if defined(__x86_64__) || defined(_M_X64)
    #define ARCH_X64 1
  #elif defined(__i386__) || defined(_M_IX86)
    #define ARCH_X86 1
  #elif defined(__aarch64__) || defined(_M_ARM64)
    #define ARCH_ARM64 1
  #elif defined(__arm__) || defined(_M_ARM)
    #define ARCH_ARM32 1
  #else
    #define ARCH_UNKNOWN 1
  #endif

#elif defined(_MSC_VER)
  #define COMPILER_MSVC 1
  /* Platform */
  #if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS 1
  #else
    #define PLATFORM_UNKNOWN 1
  #endif
  /* Architecture */
  #if defined(_M_X64)
    #define ARCH_X64 1
  #elif defined(_M_IX86)
    #define ARCH_X86 1
  #elif defined(_M_ARM64)
    #define ARCH_ARM64 1
  #elif defined(_M_ARM)
    #define ARCH_ARM32 1
  #else
    #define ARCH_UNKNOWN 1
  #endif
#elif defined(__GNUC__) || defined(__GNUG__)
  #define COMPILER_GCC 1
  /* Platform */
  #if defined(_WIN32) || defined(_WIN64)
    #define PLATFORM_WINDOWS 1
  #elif defined(__APPLE__) && defined(__MACH__)
    #define PLATFORM_MACOS 1
  #elif defined(__linux__)
    #define PLATFORM_LINUX 1
  #else
    #define PLATFORM_UNKNOWN 1
  #endif
  /* Architecture */
  #if defined(__x86_64__)
    #define ARCH_X64 1
  #elif defined(__i386__)
    #define ARCH_X86 1
  #elif defined(__aarch64__)
    #define ARCH_ARM64 1
  #elif defined(__arm__)
    #define ARCH_ARM32 1
  #else
    #define ARCH_UNKNOWN 1
  #endif
#else
  #define COMPILER_UNKNOWN 1
  #define PLATFORM_UNKNOWN 1
  #define ARCH_UNKNOWN 1
#endif

#if !defined(PLATFORM_WINDOWS)
  #define PLATFORM_WINDOWS 0
#endif

#if !defined(PLATFORM_LINUX)
  #define PLATFORM_LINUX 0
#endif

#if !defined(PLATFORM_MACOS)
  #define PLATFORM_MACOS 0
#endif

#if !defined(PLATFORM_UNKNOWN)
  #define PLATFORM_UNKNOWN 0
#endif

#if !defined(ARCH_X64)
  #define ARCH_X64 0
#endif

#if !defined(ARCH_X86)
  #define ARCH_X86 0
#endif

#if !defined(ARCH_ARM32)
  #define ARCH_ARM32 0
#endif

#if !defined(ARCH_ARM64)
  #define ARCH_ARM64 0
#endif

#if ARCH_X86 || ARCH_ARM32
  #define ARCH_32BIT 1
  #define ARCH_64BIT 0
  #else
  #define ARCH_32BIT 0
  #define ARCH_64BIT 1
#endif

#if defined(__cplusplus)
  #warning This library is meant to be used with c, but if you insist!
  #define LANG_CPP 1
  #define LANG_C 0
#else
  #define LANG_CPP 1
  #define LANG_C 0
#endif 

#ifndef internal
  #define internal static
#endif

#ifndef global
  #define global static
#endif 

#if COMPILER_MSVC || (COMPILER_CLANG && PLATFORM_WINDOWS)
  #pragma section(".rodata", read)
  #define readonly __declspec(allocate(".rodata"))
#elif COMPILER_CLANG && PLATFORM_LINUX
  #define readonly __attribute__((section(".rodata")))
#else 
  #define readonly static 
#endif 

#if COMPILER_MSVC
  #define forceinline __forceinline
#elif COMPILER_CLANG || COMPILER_GCC
  #define forceinline __attribute__((always_inline))
#else 
  #define forceinline inline
#endif

#if PLATFORM_WINDOWS
  #define dllmethod __declspec(dllexport)
#else
  #define dllmethod
#endif 

#if COMPILER_MSVC
  #define packed_struct(name) __pragma(pack(push,1)) name __pragma(pack(pop))
#else 
  #define packed_struct(name)  struct __attribute__((packed)) name
#endif 

#if defined(__clang__)
  #define expect(expr, val) __builtin_expect((expr), (val))
#else 
  #define expect(expr) (expr)
#endif 

#if LANG_C
  #if COMPILER_MSVC
    #define alignof(t) __alignof(t)
    #define alignas(x) __declspec(align(x))
  #elif COMPILER_CLANG
    #define alignof(t) __alignof(t)
    #define alignas(x) __attribute__((aligned(x)))
  #elif COMPILER_GCC
    #define alignof(t) __alignof__(t)
    #define alignas(x) __attribute__((aligned(x)))
  #elif
    #define alignof(t) (sizeof(void*))
    #define alignas(x)
  #endif 
#endif

#if ARCH_ARM32 || ARCH_ARM64 || ARCH_X64 || ARCH_X86
  #define ARCH_LE 1
  #define ARCH_BE 0
#else 
  #define ARCH_LE 0
  #define ARCH_BE 0
#endif

typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;
typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef size_t usize;

#if ARCH_32BIT
typedef i32 isize;
#else 
typedef i64 isize;
#endif

#if !defined(offsetof)
  #define offsetof(t, m) ((usize)&((t*)0)->m)
#endif 

#if !defined(True)
  #define True 1
#endif 

#if !defined(False)
  #define False (!True)
#endif

#if !defined(Or)
  #define Or(a, b) (!!(a) ? (a) : (b))
#endif

#if !defined(ArrayLen)
  #define ArrayLen(x) ((usize)(sizeof(x) / sizeof((x)[0])))
#endif

#define Swap(T, a, b) do { T __t = a; a = b; b = __t; } while (0)

u64
MinU(u64 a, u64 b);

u64
MaxU(u64 a, u64 b);

u64
ClampU(u64 v, u64 a, u64 b);

i64
MinI(i64 a, i64 b);

i64
MaxI(i64 a, i64 b);

i64
ClampI(i64 v, i64 a, i64 b);

float
MinF(float a, float b);

float
MaxF(float a, float b);

float
ClampF(float v, float a, float b);

#endif /* BUILTIN_H*/