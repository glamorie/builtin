#if !defined(BUILTIN_H)
#define BUILTIN_H
#include <stdint.h>
#include <stdio.h>
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
  #define LANG_CPP 0
  #define LANG_C 1
#endif 

#ifndef internal
  #define internal static
#endif

#ifndef global
  #define global static
#endif 

#if COMPILER_MSVC || (COMPILER_CLANG && PLATFORM_WINDOWS)
  #pragma section(".rodata", read)
  #define readonly __declspec(allocate(".rodata")) const
#elif COMPILER_CLANG && PLATFORM_LINUX
  #define readonly __attribute__((section(".rodata"))) const 
#else 
  #define readonly static const
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
    #define threadlocal __declspec(thread)
  #elif COMPILER_CLANG
    #define alignof(t) __alignof(t)
    #define alignas(x) __attribute__((aligned(x)))
    #define threadlocal __thread
  #elif COMPILER_GCC
    #define alignof(t) __alignof__(t)
    #define alignas(x) __attribute__((aligned(x)))
  #elif
    #define alignof(t) (sizeof(void*))
    #define alignas(x)
    #define threadlocal 
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

#if !defined(OffsetOf)
  #define OffsetOf(t, m) ((usize)&((t*)0)->m)
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

void*
MemoryCopy(void* Dest, const void* Src, usize Length);

void*
MemorySet(void* Dest, u8 Value, usize Length);

void*
MemoryZero(void* Dest, usize Length);

usize
MemoryCompare(const void* A, const void* B, usize Length);

u32
MemoryEqual(const void* A, const void* B, usize Length);

void*
MemoryReverse(void* A, usize Length);

void
MemorySwap(void* A, void* B, usize Length);

u64
MemoryHashSdbm(const u8* Value, usize Length);

u64
MemoryHashDjb2(const u8* Value, usize Length);

usize
MemoryHashFnv1a(const u8* Value, usize Length);

// Platform memory allocation

usize
PlatformLargePageSize(void);

usize
PlatformNormalPageSize(void);

void*
PlatformReserve(usize Size);

u32
PlatformCommitLarge(void* Base, usize Size);

u32
PlatformCommitNormal(void* Base, usize Size);

void
PlatformRelease(void* Base);

// arena 

// arena
enum
{
  ArenaFlagLargePages = 1<<1,
  ArenaFlagNoChain = 1<<2,
};

typedef struct arena arena;

arena*
ArenaMake(usize Reserve, usize Commit, usize Flags);

void
ArenaTake(arena* Arena);

void
ArenaPopTo(arena* Arena, usize Position);

usize
ArenaPosition(arena* Arena);

void
ArenaPop(arena* Arena, usize Size);

void*
ArenaPush(arena* Arena, usize Size, usize Align);

void*
ArenaPushN(arena* Arena, usize Size, usize Count, usize Align);

void*
ArenaZPush(arena* Arena, usize Size, usize Align);

void*
ArenaZPushN(arena* Arena, usize Size, usize Count, usize Align);

typedef struct temp temp;
struct temp
{
  arena* Arena;
  usize Position;
};

temp
TempBegin(arena* Arena);

void
TempEnd(temp Temp);

#define TempScope(A) for (temp __x = TempBegin(A); __x.Arena; __x.Arena = 0, (TempEnd(__x)))

arena*
ArenaGetScratch(arena** Conflicts, usize Count);

// Char
usize
CharUtf8Length(u32 Char);

usize
CharUtf8Advance(u8 Start);

usize
CharUtf8Encode_(u32 Char, u8* Parts);

void
CharUtf8Encode(u32 Char, u8* Parts, usize Length);

u32
CharUtf8Decode_(const u8* Parts);

u32
CharUtf8Decode(const u8* Parts, usize Length);

usize
CharUtf16Length(u32 Char);

usize
CharUtf16Advance(u16 Start);

usize
CharUtf16Encode_(u32 Char, u16* Parts);

void
CharUtf16Encode(u32 Char, u16* Parts, usize Length);

u32
CharUtf16Decode_(const u16* Parts);

u32
CharUtf16Decode(const u16* Parts, usize Length);

#if !defined(BUILTIN_CHAR)
#define BUILTIN_CHAR 1
#else 
#define BUILTIN_CHAR 0
#endif

u32
CharIsAlpha(u32 Ch);

u32
CharIsDigit(u32 Ch);

u32
CharIsAlnum(u32 Ch);

u32
CharIsCntrl(u32 Ch);

u32
CharIsPunct(u32 Ch);

u32
CharIsGraph(u32 Ch);

u32
CharIsPrint(u32 Ch);

u32
CharIsLower(u32 Ch);

u32
CharIsUpper(u32 Ch);

u32
CharIsXdigit(u32 Ch);

u32
CharIsBlank(u32 Ch);

u32
CharIsSpace(u32 Ch);

i32
CharToDigit(u32 Ch);

u32
CharToUpper(u32 Ch);

u32
CharToLower(u32 Ch);

u32
CharSwapCase(u32 Ch);

u32
CharCasefold(u32 Ch);

i32
CharToDigit(u32 Ch);

// string construction
typedef struct string string;
struct string
{
  u8* Value;
  usize Length;
};

#define S(s) ((string){(u8*)(s), sizeof(s)-1})

#define StringSentinel ((string){(u8*)(-1), (usize)(-1)})

typedef struct strings strings;
struct strings
{
  string* Items;
  usize Length;
};

u32
StringIsSentinel(string String);

usize
StringCLen(const char* String);

string
StringCAs(const char* Value);

string
StringC(const char* Value, arena* Arena);

string
StringFv(const char* Format, va_list Args, arena* Arena);

string
StringF(arena* Arena, const char* Format, ...);

string
StringClone(string String, arena* Arena);

string
StringJoinFv(arena* Arena, string Sep, va_list Args);

string
StringJoinF_(arena* Arena, string Sep, ...);

#define StringJoinF(Arena, Sep, ...) StringJoinF_(Arena, Sep, __VA_ARGS__, StringSentinel)

string
StringJoinN(string* Strings, usize Count, string Sep, arena* Arena);

string
StringJoins(strings Strings, string Sep, arena* Arena);

string
StringCJoinFv(string Sep, va_list Args, arena* Arena);

string
StringCJoinF_(arena* Arena, string Sep, ...);

#define StringCJoinF(Arena, Sep, ...) StringCJoinF_(Arena, S(Sep), __VA_ARGS__, 0)

string
StringCJoinN(const char** Strings, usize Count, string Sep, arena* Arena);

string
StringRange(string String, usize Start, usize End);

// String transform
string
StringCasefold(string String, arena* Arena);

string
StringToLower(string String, arena* Arena);

string
StringToUpper(string String, arena* Arena);

string
StringSwapcase(string String, arena* Arena);

string
StringCapitalize(string String, arena* Arena);

string
StringTitle(string String, arena* Arena);

string
StringReverse(string String, arena* Arena);

// String formatting and finding

string
StringCenter(string String, usize Width, u32 FillChar, arena* Arena);

string
StringLjust(string String, usize Width, u32 FillChar, arena* Arena);

string
StringRjust(string String, usize Width, u32 FillChar, arena* Arena);

usize
StringCount(string String);

usize
StringCountSub(string String, string Substring);

isize
StringFind(string String, string Substring);

isize
StringRfind(string String, string Substring);

isize
StringIndex(string String, u32 Char);

isize
StringRindex(string String, u32 Char);

// Split
string*
StringSplit_(string String, string Sep, usize* Count, u32 Right, arena* Arena);

strings
StringSplit(string String, string Sep, u32 Right, arena* Arena);

string*
StringSplitSpace_(string String, usize* Count, arena* Arena);

strings
StringSplitSpace(string String, arena* Arena);

string*
StringSplitLines_(string String, usize* Count, u32 KeepEnds, arena* Arena);

strings
StringSplitLines(string String, u32 KeepEnds, arena* Arena);

// Strip
string
StringStrip(string String, arena* Arena);

string
StringLStrip(string String, arena* Arena);

string
StringRStrip(string String, arena* Arena);

string
StringReplace(string String, string Substring, string Substitute, arena* Arena);

string
StringExpandTabs(string String, u32 TabSize, arena* Arena);

string
StringRemovePrefix(string String, string Prefix, arena* Arena);

string
StringRemoveSuffix(string String, string Prefix, arena* Arena);

usize
StringCompare(string String, string B);

u32
StringEqual(string String, string B);

isize
StringCompareFv(string String, va_list Args);

isize
StringCompareF_(string String, ...);

u32
StringEqualF_(string String, ...);

#define StringEqualF(String, ...) (StringEqualF_(String, __VA_ARGS__, StringSentinel) != -1)
#define StringCompareF(String, ...) StringCompareF_(String, __VA_ARGS__, StringSentinel)

u32
StringStartsWith(string String, string Prefix);

u32
StringEndsWith(string String, string Prefix);

u32
StringContains(string String, string Sub);

usize
StringCompareCI(string A, string B);

u32
StringEqualCI(string A, string B);

isize
StringCompareCIFv(string String, va_list Args);

isize
StringCompareCIF_(string String, ...);

u32
StringEqualCIF_(string String, ...);

#define StringEqualCIF(String, ...) (StringEqualCIF_(String, __VA_ARGS__, StringSentinel) != -1)
#define StringCompareCIF(String, ...) StringCompareCIF_(String, __VA_ARGS__, StringSentinel)

// Number parsing
typedef struct _flit _flit;
struct _flit
{ union { u64 Bits; double Value; }; };

#define FLT_LITERAL(v) ((_flit){.Bits = (v)})
#if !defined(INFINITY)
  #define INFINITY FLT_LITERAL(0x7ff0000000000000ULL).Value
#endif

#if !defined(NAN)
  #define NAN FLT_LITERAL(0x7ff8000000000000ULL).Value
#endif 

double
StringToDouble(string String, usize* End);

isize
StringToInt(string String, usize* End);

usize
StringHash(string String);

#define DLLPushFrontEx(List, Node, Head, Tail, Prev, Next) \
do \
{ \
  (Node)->Prev = NULL; \
  (Node)->Next = NULL; \
  if ((List)->Head) (List)->Head->Prev = (Node); \
  else (List)->Tail = (Node); \
  (Node)->Next = (List)->Head; \
  (List)->Head = (Node); \
} while (0)

#define DLLPushBackEx(List, Node, Head, Tail, Prev, Next) \
do \
{ \
  (Node)->Prev = NULL; \
  (Node)->Next = NULL; \
  if ((List)->Tail) (List)->Tail->Next = (Node); \
  else (List)->Head = (Node); \
  (Node)->Prev = (List)->Tail; \
  (List)->Tail = (Node); \
} while (0)

#define DLLInsertBeforeEx(List, At, Node, Head, Tail, Prev, Next) \
do \
{ \
  if ((At)->Prev) (At)->Prev->Next = (Node); \
  else (List)->Head = (Node); \
  (Node)->Prev = (At)->Prev; \
  (Node)->Next = (At); \
} while (0)

#define DLLInsertEx(List, At, Node, Head, Tail, Prev, Next) \
do  \
{ \
  if ((At)->Next) (At)->Next->Next = (Node); \
  else (List)->Tail = (Node); \
  (Node)->Prev = (At); \
  (Node)->Next = (At)->Next; \
} while(0)

#define DLLRemoveEx(List, Node, Head, Tail, Prev, Next) \
do \
{ \
  if ((Node)->Prev) (Node)->Prev->Next = (Node)->Next; \
  else (List)->Head = (Node)->Next; \
  if ((Node)->Next) (Node)->Next->Prev = (Node)->Prev; \
  else (List)->Tail = (Node)->Prev; \
  (Node)->Prev = NULL; \
  (Node)->Next = NULL; \
} while (0)

#define DLLPopFrontEx(List, Result, Head, Tail, Prev, Next) \
do \
{ \
  (Result) = (List)->Head; \
  DLLRemoveEx((List), (Result), Head, Tail, Prev, Next); \
} while (0)

#define DLLPopEx(List, Result, Head, Tail, Prev, Next) \
do \
{ \
  (Result) = (List)->Tail; \
  DLLRemoveEx(List, (Result), Head, Tail, Prev, Next); \
} while (0)

#define DLLMergeEx(A, B, Node, Head, Tail, Prev, Next) \
do  \
{ \
  if ((A)->Tail) (A)->Tail->Next = (B)->Head; \
  else (A)->Head = (B)->Head; \
  if ((B)->Head) (B)->Head->Prev = (A)->Tail; \
  (A)->Tail = (B)->Tail; \
} while (0)

#define DLLPushFront(List, Node) DLLPushFrontEx(List, Node, Head, Tail, Prev, Next)
#define DLLPushBack(List, Node) DLLPushBackEx(List, Node, Head, Tail, Prev, Next)
#define DLLInsertBefore(List, At, Node) DLLInsertBeforeEx(List, At, Node, Head, Tail, Prev, Next)
#define DLLInsert(List, At, Node) DLLInsertEx(List, At, Node, Head, Tail, Prev, Next)
#define DLLRemove(List, Node) DLLRemoveEx(List, Node, Head, Tail, Prev, Next)
#define DLLPopFront(List, Result) DLLPopFrontEx(List, Result, Head, Tail, Prev, Next)
#define DLLPop(List, Result) DLLPopEx(List, Result, Head, Tail, Prev, Next)
#define DLLMerge(A, B) DLLMergeEx(A, B, Node, Head, Tail, Prev, Next)

#define SLLPushFrontEx(List, Node, Head, Tail, Next) \
do \
{ \
  (Node)->Next = (List)->Head; \
  if (!(List)->Head) List->Tail = (Node); \
  (List)->Head = (Node); \
} while (0)

#define SLLPushEx(List, Node, Head, Tail, Next) \
do \
{ \
  if ((List)->Tail) (List)->Tail->Next = (Node); \
  else (List)->Head = (Node); \
  (Node)->Next = (List)->Tail; \
} while (0)

#define SLLPopFrontEx(List, Result, Head, Tail, Next) \
do \
{ \
  (Result) = (List)->Head; \
  (List)->Head = (Result)->Next; \
  if ((Result)) (Result)->Next = NULL; \
  else (List)->Tail = NULL; \
} while (0)

#define SLLPopEx(List, Result, Head, Tail, Next) \
do \
{ \
  if ((List)->Head == (List)->Tail) \
  { \
    (Result) = (List)->Head; \
    (List)->Head = NULL; \
    (List)->Tail = NULL; \
    if ((Result)) (Result)->Next = NULL; \
  } else \
  { \
    (Result) = (List)->Head; \
    while ((Result) && (Result)->Next != (List)->Tail) (Result) = (Result)->Next; \
    (Result)->Next = NULL; \
    void* _xx = (void*)((List)->Tail); \
    (List)->Tail = (Result); \
    (Result) = _xx; \
  }; \
} while (0)

#define SLLPushFront(List, Node) SLLPushFrontEx(List, Node, Head, Tail, Next)
#define SLLPush(List, Node) SLLPushEx(List, Node, Head, Tail, Next)
#define SLLPopFront(List, Result) SLLPopFrontEx(List, Result, Head, Tail, Next)
#define SLLPop(List, Result) SLLPopEx(List, Result, Head, Tail, Next)

#endif /* BUILTIN_H*/