#include "builtin.h"
#include <smmintrin.h>
#if PLATFORM_WINDOWS
#include <Windows.h>
#pragma comment(lib, "onecore.lib")
#else 
#include <stdlib.h>
#endif 

u64
MinU(u64 a, u64 b)
{
  return a < b ? a : b;
};

u64
MaxU(u64 a, u64 b)
{
  return a > b ? a : b;
};

u64
ClampU(u64 v, u64 a, u64 b)
{
  return v < a ? a : v < b ? v : b;
};

i64
MinI(i64 a, i64 b)
{
  return a < b ? a : b;
};

i64
MaxI(i64 a, i64 b)
{
  return a > b ? a : b;
};

i64
ClampI(i64 v, i64 a, i64 b)
{
  return v < a ? a : v < b ? v : b;
};

float
MinF(float a, float b)
{
  return a < b ? a : b;
};

float
MaxF(float a, float b)
{
  return a > b ? a : b;
};

float
ClampF(float v, float a, float b)
{
  return v < a ? a : v < b ? v : b;
};

inline static usize
MemoryPointerDistance(const void* A, const void* B)
{
  return A < B ? (usize)B - (usize)A : (usize)A - (usize)B;
};

void*
MemoryCopy(void* Dest, const void* Src, usize Length)
{
  if (!Src || !Dest) Length = 0;
  
  if (MemoryPointerDistance(Src, Dest) < Length)
  {
    for (usize i = Length; i; i--)
    {
      ((u8*)Dest)[i -1] = ((u8*)Src)[i - 1];
    };
  } else if ((usize)Dest % alignof(usize) == (usize)Src % alignof(usize))
  {
    usize Left = MinU(Length, (-(usize)Dest) & (alignof(usize) - 1));
    usize Blocks = (Length - Left) / sizeof(usize);
    usize Right = Left + Blocks * sizeof(usize);
    u8* Sx = (u8*)Dest;
    const u8* Sy = (u8*)Src;
    
    usize* Bx = ((usize*)(Sx + Left));
    const usize* By = ((usize*)(Sy + Left));
    
    for (usize i = 0; i < Left; i++)
    {
      Sx[i] = Sy[i];
    };
    
    for (usize i = 0; i < Blocks; i++)
    {
      Bx[i] = By[i];
    };
    
    for (usize i = Right; i < Length; i++)
    {
      Sx[i] = Sy[i];
    };
  } else 
  {
    for (usize i = 0; i < Length; i++)
    {
      ((u8*)Dest)[i] = ((u8*)Src)[i];
    };
  };
  return Dest;
};

void*
MemorySet(void* Dest, u8 Value, usize Length)
{
  if (!Dest) return Dest;
  
  usize Packed = (usize)Value * (~(usize)0 / 255);
  usize Left = MinU(Length, (-(usize)Dest) & (alignof(usize) - 1));
  usize Blocks = (Length - Left) / sizeof(usize);
  usize Right = Length - (Length - Left) % sizeof(usize);
  u8* S = (u8*)Dest;
  usize* B = ((usize*)((u8*)Dest + Left));
  
  for (usize i = 0; i < Left; i++)
  {
    S[i] = Value;
  };
  
  for (usize i = 0; i < Blocks; i++)
  {
    B[i] = Packed;
  };
  
  for (usize i = Right; i < Length; i++)
  {
    S[i] = Value;
  };
  return Dest;
};

void*
MemoryZero(void* Dest, usize Length)
{
  return MemorySet(Dest, 0, Length);
};

usize
MemoryCompare(const void* A, const void* B, usize Length)
{
  const u8* x = A;
  const u8* y = B;
  usize i = 0;
  
  if (!A || !B) Length = 0;
  
  while (i < Length && x[i] == y[i]) i++;
  
  return i;
};

u32
MemoryEqual(const void* A, const void* B, usize Length)
{
  return MemoryCompare(A, B, Length) == Length;
};

void*
MemoryReverse(void* A, usize Length)
{
  if (!A) Length = 0;
  
  for (usize i = 0; i < Length / 2; i++)
  {
    u8 L = ((u8*)A)[i], R = ((u8*)A)[Length - i - 1];
    ((u8*)A)[i] = R;
    ((u8*)A)[Length - i - 1] = L;
  };
  return A;
};

void
MemorySwap(void* A, void* B, usize Length)
{
  if (!A || !B) Length = 0;
  
  for (usize i = 0; i < Length; i++)
  {
    u8 T = ((u8*)A)[i];
    ((u8*)A)[i] = ((u8*)B)[i];
    ((u8*)B)[i] = T;
  };
};


#if PLATFORM_WINDOWS
usize
PlatformLargePageSize(void)
{
  return GetLargePageMinimum();
};

usize
PlatformNormalPageSize(void)
{
  SYSTEM_INFO Si = {0};
  GetSystemInfo(&Si);
  return Si.dwPageSize;
};

void*
PlatformReserve(usize Size)
{
  return VirtualAlloc(0, Size, MEM_RESERVE, PAGE_READWRITE);
};

u32
PlatformCommitLarge(void* Base, usize Size)
{
  return 1;
};

u32
PlatformCommitNormal(void* Base, usize Size)
{
  
  void* Out = VirtualAlloc(Base, Size, MEM_COMMIT, PAGE_READWRITE);
  return Out != 0;
};

void
PlatformRelease(void* Base)
{
  VirtualFree(Base, 0, MEM_FREE);
};

#else // PLATFORM_UNKNOWN

usize
PlatformLargePageSize(void)
{
  return 1<<10;
};

usize
PlatformNormalPageSize(void)
{
  return 2<<10;
};

void*
PlatformReserve(usize Size)
{
  return malloc(Size);
};

u32
PlatformCommitLarge(void* Base, usize Size)
{
  return 1;
};

u32
PlatformCommitNormal(void* Base, usize Size)
{
  return 1;
};

void
PlatformRelease(void* Base)
{
  if (Base) free(Base);
};
#endif // PLATFORM


struct arena
{
  u8* Base;
  usize Flags;
  usize Granularity;
  usize Offset;
  usize Reserved;
  usize Commit;
  usize Position;
  arena* Prev;
  arena* Current;
};

#define _ArenaHeader (0x100)

inline static usize
ArenaAlignUp(usize Point, usize Align)
{
  return Point + (Align - Point % Align) % Align;
};

inline static usize
ArenaAlignPadding(usize Point, usize Align)
{
  return (Align - Point % Align) % Align;
};

arena*
ArenaMake(usize Reserve, usize Commit, usize Flags)
{
  usize Granularity;
  void* Base = 0;
  Commit = MaxU(Commit, _ArenaHeader);
  
  if (Flags & ArenaFlagLargePages)
  {
    Granularity = PlatformLargePageSize();
    Reserve = ArenaAlignUp(Reserve, Granularity);
    Commit = ArenaAlignUp(Commit, Granularity);
    Base = PlatformReserve(Reserve);
    if (!PlatformCommitLarge(Base, Commit))
    {
      PlatformRelease(Base);
      Base = 0;
    };
  } else
  {
    Granularity = PlatformNormalPageSize();
    Reserve = ArenaAlignUp(Reserve, Granularity);
    Commit = ArenaAlignUp(Commit, Granularity);
    Base = PlatformReserve(Reserve);
    
    if (!PlatformCommitNormal(Base, Commit))
    {
      PlatformRelease(Base);
      Base = 0;
    };
  };
  arena* Arena = Base;
  if (Arena)
  {
    Arena->Base = Base;
    Arena->Flags = Flags;
    Arena->Granularity = Granularity;
    Arena->Reserved = Reserve;
    Arena->Commit = Commit;
    Arena->Position = _ArenaHeader;
    Arena->Offset = 0;
    Arena->Prev = 0;
    Arena->Current = Arena;
  };
  return Arena;
};

void
ArenaTake(arena* Arena)
{
  while (Arena)
  {
    arena* Prev = Arena->Prev;
    PlatformRelease(Arena);
    Arena = Prev;
  };
};

void*
ArenaPush(arena* Arena, usize Size, usize Align)
{
  if (!Arena || !Size) return 0;
  
  Align = MaxU(Align, 1);
  
  Start:
  arena* Current = Arena->Current;
  usize Padding = ArenaAlignPadding(Arena->Position, Align);
  
  if (Current->Reserved < Current->Position + Padding + Size)
  {
    if (Arena->Flags & ArenaFlagNoChain) return 0;
    
    arena* Node = ArenaMake(MaxU(Size + _ArenaHeader, Current->Reserved), Size + _ArenaHeader, Current->Flags);
    
    if (!Node) return 0;
    
    Node->Prev = Current;
    Node->Offset = Current->Offset + Current->Reserved;
    Arena->Current =
    Current = Node;
    goto Start;
  };
  
  if (Current->Commit < Current->Position + Padding + Size)
  {
    usize CommitSize = ArenaAlignUp(Current->Position + Padding + Size - Current->Commit, Current->Granularity);
    
    u32 CommitOk = 0;
    if (Current->Flags & ArenaFlagLargePages) CommitOk = PlatformCommitLarge(Current->Base + Current->Position, CommitSize);
    else CommitOk = PlatformCommitNormal(Current->Base + Current->Position, CommitSize);
    
    if (!CommitOk) return 0;
    
    Current->Commit += CommitSize;
  };
  
  Current->Position += Padding;
  void* Allocation = (void*)(Current->Base + Current->Position);
  Current->Position += Size;
  return Allocation;
};

void*
ArenaPushN(arena* Arena, usize Size, usize Count, usize Align)
{
  return ArenaPush(Arena, Size * Count, Align);
};

void*
ArenaZPush(arena* Arena, usize Size, usize Align)
{
  return MemoryZero(ArenaPush(Arena, Size, Align), Size);
};

void*
ArenaZPushN(arena* Arena, usize Size, usize Count, usize Align)
{
  return MemoryZero(ArenaPushN(Arena, Size, Count, Align), Size);
};

usize
ArenaPosition(arena* Arena)
{
  return Arena ? Arena->Offset + Arena->Position : 0;
};

void
ArenaPopTo(arena* Arena, usize Position)
{
  if (!Arena) return;
  
  usize Offset = MaxU(Position, _ArenaHeader);
  arena* Current = Arena->Current;
  
  while (Offset < Current->Offset)
  {
    arena* Prev = Current->Prev;
    PlatformRelease(Current);
    Current = Prev;
  };
  
  Current->Offset = Offset;
  Arena->Current = Current;
};

void
ArenaPop(arena* Arena, usize Size)
{
  usize CurrentPosition = ArenaPosition(Arena);
  if (Size < CurrentPosition)
  {
    ArenaPopTo(Arena, CurrentPosition - Size);
  };
};

void
ArenaClear(arena* Arena)
{
  ArenaPopTo(Arena, 0);
};

temp
TempBegin(arena* Arena)
{
  temp Temp =
  {
    .Arena = Arena,
    .Position = ArenaPosition(Arena)
  };
  return Temp;
};

void
TempEnd(temp Temp)
{
  ArenaPopTo(Temp.Arena, Temp.Position);
};

usize
CharUtf8Length(u32 c)
{
  if (c <= 0x7F) return 1;
  if (c <= 0x7FF) return 2;
  if (c <= 0xFFFF)
  {
    if (c >= 0xD800 && c <= 0xDFFF) return 0;
    return 3;
  };
  if (c <= 0x10FFFF) return 4;
  return 0;
};

usize
CharUtf8Advance(u8 Start)
{
  if ((Start & 0x80) == 0x00) return 1;
  if ((Start & 0xE0) == 0xC0) return 2;
  if ((Start & 0xF0) == 0xE0) return 3;
  if ((Start & 0xF8) == 0xF0) return 4;
  return 0;
};

usize
CharUtf8Encode_(u32 Char, u8* Out)
{
  usize Length  = 0;
  
  if (Char <= 0x7F)
  {
    Out[0] = (u8)Char;
    Length = 1;
  } else if (Char <= 0x7FF)
  {
    Out[0] = 0xC0 | (Char >> 6);
    Out[1] = 0x80 | (Char & 0x3F);
    Length = 2;
  } else if (Char <= 0xFFFF)
  {
    Out[0] = 0xE0 | (Char >> 12);
    Out[1] = 0x80 | ((Char >> 6) & 0x3F);
    Out[2] = 0x80 | (Char & 0x3F);
    Length = 3;
  } else
  {
    Out[0] = 0xF0 | (Char >> 18);
    Out[1] = 0x80 | ((Char >> 12) & 0x3F);
    Out[2] = 0x80 | ((Char >> 6) & 0x3F);
    Out[3] = 0x80 | (Char & 0x3F);
    Length = 4;
  };
  return Length;
};

void
CharUtf8Encode(u32 Char, u8* Parts, usize Length)
{
  if (CharUtf8Length(Char) == Length && Parts)
  {
    CharUtf8Encode_(Char, Parts);
  };
};

u32
CharUtf8Decode_(const u8* Parts)
{
  u8 Head = Parts[0];
  u32 Out = 0;
  if (Head < 0x80)
  {
    Out = Head;
  } else if ((Head >> 5) == 0x6)
  {
    Out = ((u32)(Head & 0x1F) << 6) | ((u32)(Parts[1] & 0x3F));
  } else if ((Head >> 4) == 0xE)
  {
    Out = (
      ((u32)(Head & 0x0F) << 12) |
      ((u32)(Parts[1] & 0x3F) << 6) |
      ((u32)(Parts[2] & 0x3F))
    );
  }
  else
  {
    Out = (
      ((u32)(Head & 0x07) << 18) |
      ((u32)(Parts[1] & 0x3F) << 12) |
      ((u32)(Parts[2] & 0x3F) << 6) |
      ((u32)(Parts[3] & 0x3F))
    );
  };
  return Out;
};

u32
CharUtf8Decode(const u8* Parts, usize Length)
{
  u32 Out = 0;
  if (Parts && Length == CharUtf8Advance(*Parts))
  {
    Out = CharUtf8Decode_(Parts);
  };
  return Out;
};

usize
CharUtf16Length(u32 Char)
{
  if (Char > 0x10FFFF) return 0;
  if (Char >= 0xD800 && Char <= 0xDFFF) return 0;
  if (Char <= 0xFFFF) return 1;
  return 2;
};

usize
CharUtf16Advance(u16 Start)
{
  if (Start >= 0xD800 && Start <= 0xDBFF) return 2;
  if ((Start & 0xFC00) == 0xDC00) return 0;
  return 1;
};

usize
CharUtf16Encode_(u32 Char, u16* Parts)
{
  u16 Head = Parts[0];
  u32 Out = 0;
  
  if ((Head & 0xFC00) == 0xD800)
  {
    u16 Tail = Parts[1];
    Out = (
      (((u32)(Head & 0x03FF) << 10) |
      ((u32)(Tail & 0x03FF))) + 0x10000
    );
  } else
  {
    Out = Head;
  };
  return Out;
};

void
CharUtf16Encode(u32 Char, u16* Parts, usize Length)
{
  if (Parts && CharUtf16Advance(*Parts) == Length)
  {
    CharUtf16Encode_(Char, Parts);
  };
};

u32
CharUtf16Decode_(const u16* Parts)
{
  u16 Head = Parts[0];
  u32 Out = 0;
  
  if ((Head & 0xFC00) == 0xD800)
  {
    u16 Tail = Parts[1];
    Out = (
      (((u32)(Head & 0x03FF) << 10) |
      ((u32)(Tail & 0x03FF))) + 0x10000
    );
  } else
  {
    Out = Head;
  };
  return Out;
};

u32
CharUtf16Decode(const u16* Parts, usize Length)
{
  u32 Out = 0;
  if (Parts && CharUtf16Advance(*Parts) == Length)
  {
    Out = CharUtf16Decode_(Parts);
  };
  return Out;
};

#if BUILTIN_CHAR
enum
{
  _CharPropertyAlpha = 1<<1,
  _CharPropertyDigit = 1<<2,
  _CharPropertyAlnum = 1<<3,
  _CharPropertyCntrl = 1<<4,
  _CharPropertyPunct = 1<<5,
  _CharPropertyGraph = 1<<6,
  _CharPropertyPrint = 1<<7,
  _CharPropertyLower = 1<<8,
  _CharPropertyUpper = 1<<9,
  _CharPropertyBlank = 1<<10,
  _CharPropertyXdigit = 1<<11,
  _CharPropertySpace = 1<<12,
};

static u16 _AsciiPropertyTable[] =
{
  0x10 ,0x10 ,0x10 ,0x10 ,0x10 ,0x10 ,0x10 ,0x10 ,0x10 ,0x1810 ,0x1010 ,0x1010 ,0x1010 ,0x1010 ,0x10 ,0x10,
  0x10 ,0x10 ,0x10 ,0x10 ,0x10 ,0x10 ,0x10 ,0x10 ,0x10 ,0x10 ,0x10 ,0x10 ,0x10 ,0x10 ,0x10 ,0x10,
  0x1880 ,0xe0 ,0xe0 ,0xe0 ,0xe0 ,0xe0 ,0xe0 ,0xe0 ,0xe0 ,0xe0 ,0xe0 ,0xe0 ,0xe0 ,0xe0 ,0xe0 ,0xe0,
  0x4cc ,0x4cc ,0x4cc ,0x4cc ,0x4cc ,0x4cc ,0x4cc ,0x4cc ,0x4cc ,0x4cc ,0xe0 ,0xe0 ,0xe0 ,0xe0 ,0xe0 ,0xe0,
  0xe0 ,0x6ca ,0x6ca ,0x6ca ,0x6ca ,0x6ca ,0x6ca ,0x2ca ,0x2ca ,0x2ca ,0x2ca ,0x2ca ,0x2ca ,0x2ca ,0x2ca ,0x2ca,
  0x2ca ,0x2ca ,0x2ca ,0x2ca ,0x2ca ,0x2ca ,0x2ca ,0x2ca ,0x2ca ,0x2ca ,0x2ca ,0xe0 ,0xe0 ,0xe0 ,0xe0 ,0xe0,
  0xe0 ,0x5ca ,0x5ca ,0x5ca ,0x5ca ,0x5ca ,0x5ca ,0x1ca ,0x1ca ,0x1ca ,0x1ca ,0x1ca ,0x1ca ,0x1ca ,0x1ca ,0x1ca,
  0x1ca ,0x1ca ,0x1ca ,0x1ca ,0x1ca ,0x1ca ,0x1ca ,0x1ca ,0x1ca ,0x1ca ,0x1ca ,0xe0 ,0xe0 ,0xe0 ,0xe0 ,0x10,
  0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0,
  0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0,
  0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0,
  0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0,
  0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0,
  0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0,
  0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0,
  0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0 ,0x0
};

u32
CharIsAlpha(u32 Ch)
{
  if (Ch <= 0xFF) return !!(_AsciiPropertyTable[Ch] & _CharPropertyAlpha);
  return 0;
};

u32
CharIsDigit(u32 Ch)
{
  if (Ch <= 0xFF) return !!(_AsciiPropertyTable[Ch] & _CharPropertyDigit);
  return 0;
};

u32
CharIsAlnum(u32 Ch)
{
  if (Ch <= 0xFF) return !!(_AsciiPropertyTable[Ch] & _CharPropertyAlnum);
  return 0;
};

u32
CharIsCntrl(u32 Ch)
{
  if (Ch <= 0xFF) return !!(_AsciiPropertyTable[Ch] & _CharPropertyCntrl);
  return 0;
};

u32
CharIsPunct(u32 Ch)
{
  if (Ch <= 0xFF) return !!(_AsciiPropertyTable[Ch] & _CharPropertyPunct);
  return 0;
};

u32
CharIsGraph(u32 Ch)
{
  if (Ch <= 0xFF) return !!(_AsciiPropertyTable[Ch] & _CharPropertyGraph);
  return 0;
};

u32
CharIsPrint(u32 Ch)
{
  if (Ch <= 0xFF) return !!(_AsciiPropertyTable[Ch] & _CharPropertyPrint);
  return 0;
};

u32
CharIsLower(u32 Ch)
{
  if (Ch <= 0xFF) return !!(_AsciiPropertyTable[Ch] & _CharPropertyLower);
  return 0;
};

u32
CharIsUpper(u32 Ch)
{
  if (Ch <= 0xFF) return !!(_AsciiPropertyTable[Ch] & _CharPropertyUpper);
  return 0;
};

u32
CharIsXdigit(u32 Ch)
{
  if (Ch <= 0xFF) return !!(_AsciiPropertyTable[Ch] & _CharPropertyXdigit);
  return 0;
};

u32
CharIsBlank(u32 Ch)
{
  if (Ch <= 0xFF) return !!(_AsciiPropertyTable[Ch] & _CharPropertyBlank);
  return 0;
};

u32
CharIsSpace(u32 Ch)
{
  if (Ch <= 0xFF) return !!(_AsciiPropertyTable[Ch] & _CharPropertySpace);
  return 0;
};

u32
CharToUpper(u32 Ch)
{
  if (Ch <= 0xFF && CharIsLower(Ch))
  {
    return Ch - ('a' - 'A');
  };
  return Ch;
};

u32
CharToLower(u32 Ch)
{
  if (Ch <= 0xFF && CharIsUpper(Ch))
  {
    return Ch + ('a' - 'A');
  };
  return Ch;
};

u32
CharSwapCase(u32 Ch)
{
  if (CharIsLower(Ch)) return CharToUpper(Ch);
  else return CharToLower(Ch);
};

u32
CharCasefold(u32 Ch)
{
  if (CharIsUpper(Ch)) Ch = CharToLower(Ch);
  return Ch;
};

i32
CharToDigit(u32 Ch)
{
  if ('0' <= Ch && Ch <= '9') return Ch - '0';
  if ('a' <= Ch && Ch <= 'f') return Ch - 'a' + 10;
  if ('A' <= Ch && Ch <= 'F') return Ch - 'A' + 10;
  return -1;
};

#endif // BUILTIN_CHAR

usize
StringCLen(const char* String)
{
  __m128i zero = _mm_setzero_si128();
  usize i = 0;
  
  for (;;) 
  {
    __m128i Chunk = _mm_loadu_si128((__m128i*)(String + i));
    __m128i Comp = _mm_cmpeq_epi8(Chunk, zero);
    int Mask = _mm_movemask_epi8(Comp);
    
    if (Mask != 0) 
    {
      usize index;
      #if COMPILER_MSVC
      unsigned long t;
      _BitScanForward(&t, Mask);
      index = (usize)t;
      #elif COMPILER_GCC || COMPILER_CLANG
      index = (usize)__builtin_ctz(Mask);
      #else
      index = 0;
      while (!(Mask & (1 << index))) ++index;
      #endif
      
      return i + index;
    };
    i += 16;
  };
};

string
StringCAs(const char* Value)
{
  string Out = {(u8*)Value, StringCLen(Value)};
  return Out;
};

string
StringC(const char* Value, arena* Arena)
{
  usize Length = StringCLen(Value);
  string Out = {0};
  Out.Value = ArenaPush(Arena, Length + 1, alignof(u8));
  if (Out.Value)
  {
    MemoryCopy(Out.Value, Value, Length);
    Out.Length = Length;
    Out.Value[Length] = 0;
  };
  return Out;
};

string
StringFv(const char* Format, va_list Args, arena* Arena)
{
  va_list Temp;
  va_copy(Temp, Args);
  usize Length = vsnprintf(0, 0, Format, Temp);
  va_end(Temp);
  
  string Out = {0};
  Out.Value = ArenaPush(Arena, Length + 1, alignof(u8));
  if (Out.Value)
  {
    vsnprintf((char*)Out.Value, Length + 1, Format, Args);
    Out.Length = Length;
  };
  return Out;
};

string
StringF(arena* Arena, const char* Format, ...)
{
  va_list Args;
  va_start(Args, Format);
  string Out = StringFv(Format, Args, Arena);
  return Out;
};

string
StringClone(string String, arena* Arena)
{
  string Out = {0};
  Out.Value = ArenaPush(Arena, String.Length + 1, alignof(u8));
  if (Out.Value)
  {
    MemoryCopy(Out.Value, String.Value, String.Length);
    Out.Length = String.Length;
    Out.Value[Out.Length] = 0;
  };
  return Out;  
};

static usize
StringArgsLength(va_list Args, usize* Count)
{
  usize n = 0;
  usize Length = 0;
  while (1)
  {
    string Value = va_arg(Args, string);
    if (StringIsSentinel(Value)) break;
    n++;
    Length += Value.Length;
  };
  if (Count) *Count = n;
  return Length;
};

string
StringJoinFv(arena* Arena, string Sep, va_list Args)
{
  usize NumberOfStrings = 0;
  usize NumberOfBytes = 0;
  
  {
    va_list Copy;
    va_copy(Copy, Args);
    NumberOfBytes = StringArgsLength(Copy, &NumberOfStrings);
    va_end(Copy);
  };
  
  usize Length = NumberOfStrings ? NumberOfBytes + Sep.Length * (NumberOfStrings - 1) : 0;
  
  string Out = {0};
  Out.Value = ArenaPush(Arena, Length + 1, alignof(u8));
  if (Out.Value)
  {
    va_list Copy;
    va_copy(Copy, Args);
    usize Count = 0;
    while (1)
    {
      string Value = va_arg(Copy, string);
      
      if (StringIsSentinel(Value)) break;
      
      MemoryCopy(Out.Value + Count, Value.Value, Value.Length);
      Count += Value.Length;
      if (Count + Sep.Length < Length)
      {
        MemoryCopy(Out.Value + Count, Sep.Value, Sep.Length);
        Count += Sep.Length;
      } else break;
    };
    va_end(Copy);
    
    Out.Length = Length;
    Out.Value[Length] = 0;
  };
  return Out;
};

string
StringJoinF_(arena* Arena, string Sep, ...)
{
  va_list Args;
  va_start(Args, Sep);
  string Out = StringJoinFv(Arena, Sep, Args);
  va_end(Args);
  return Out;
};

string
StringJoinN(string* Strings, usize Count, string Sep, arena* Arena)
{
  usize Length = 0;
  for (usize i = 0; i < Count; i++)
  {
    string Value = Strings[i];
    Length += Value.Length + Sep.Length;
  };
  
  if (Count) Length -= Sep.Length;
  
  string Out = {0};
  Out.Value = ArenaPush(Arena, Length + 1, alignof(u8));
  
  if (Out.Value)
  {
    Out.Length = Length;
    usize k = 0;
    for (usize i = 0; i < Count - 1; i++)
    {
      MemoryCopy(Out.Value + k, Strings[i].Value, Strings[i].Length);
      k += Strings[i].Length;
      MemoryCopy(Out.Value + k, Sep.Value, Sep.Length);
      k += Sep.Length;      
    };
    MemoryCopy(Out.Value + k, Strings[Count - 1].Value, Strings[Count - 1].Length);
    Out.Value[Length] = 0;
  };
  return Out;
};


static usize
StringCArgsLength(va_list Args, usize* Count)
{
  usize n = 0;
  usize Length = 0;
  while (1)
  {
    const char* Value = va_arg(Args, const char*);
    if (!Value) break;
    n++;
    Length += StringCLen(Value);
  };
  if (Count) *Count = n;
  return Length;
};

string
StringCJoinFv(string Sep, va_list Args, arena* Arena)
{
  va_list Temp;
  va_copy(Temp, Args);
  usize Count = 0;
  usize Length = StringCArgsLength(Temp, &Count);
  va_end(Temp);
  
  if (Count) Length += Sep.Length * (Count - 1);
  
  string Out = {0};
  Out.Value = ArenaPush(Arena, Length + 1, alignof(u8));
  
  if (Out.Value)
  {
    Out.Length = Length;
    usize k = 0;
    while (1)
    {
      const char* Value = va_arg(Args, const char*);
      if (!Value) break;
      
      
      for (usize i = 0; Value[i]; i++)
      {
        Out.Value[k++] = Value[i];
      };
      
      if (k + Sep.Length < Length)
      {
        MemoryCopy(Out.Value + k, Sep.Value, Sep.Length);
        k += Sep.Length;
      };
    };
    Out.Value[Length] = 0;
  };
  return Out;  
};

string
StringCJoinF_(arena* Arena, string Sep, ...)
{
  va_list Args;
  va_start(Args, Sep);
  string Out = StringCJoinFv(Sep, Args, Arena);
  va_end(Args);
  return Out;
};

string
StringCJoinN(const char** Strings, usize Count, string Sep, arena* Arena)
{
  usize Length = 0;
  for (usize i = 0; i < Count; i++)
  {
    const char* Value = Strings[i];
    Length += StringCLen(Value) + Sep.Length;
  };
  if (Count) Length -= Sep.Length;
  
  string Out = {0};
  Out.Value = ArenaPush(Arena, Length + 1, alignof(u8));
  
  if (Out.Value)
  {
    usize k = 0;
    Out.Length = Length;
    
    for (usize i = 0; i < Count; i++)
    {
      const char* Value = Strings[i];
      usize m = 0;
      while (Value[m])
      {
        Out.Value[k++] = Value[m++];
      };
      if (i == Count - 1) continue;
      MemoryCopy(Out.Value + k, Sep.Value, Sep.Length);
      k += Sep.Length;
    };
    Out.Value[Length] = 0;
  };
  return Out;
};

string
StringRange(string String, usize Start, usize End)
{
  usize A = MinU(Start, End), B = MaxU(Start, End);
  usize x = 0, y = String.Length;
  usize i = 0;
  usize k = 0;
  while (i < String.Length)
  {
    usize Advance = MaxU(1, CharUtf8Advance(String.Value[i]));
    
    if (i + Advance > String.Length) break;
    
    if (A == k) x = i;
    k++;
    i += Advance;
    if (B == k)
    {
      y = i;
      break;
    };
  };
  
  x = MinU(x, String.Length);
  y = MinU(y, String.Length);
  
  string Out = 
  {
    String.Value + x,
    y - x
  };
  return Out;
};

typedef u32
char_transformer(u32 Ch);

static usize
StringTransform(string String, u8* Value, usize Length, char_transformer* Transformer)
{
  if (!Value) Length = 0;
  
  usize i = 0;
  usize k = 0;
  while (i < String.Length)
  {
    usize Advance = CharUtf8Advance(String.Value[i]);
    
    if (Advance == 0)
    {
      i++;
      continue;
    };
    
    u32 Ch = Transformer(CharUtf8Decode_(String.Value + i));
    usize Width = CharUtf8Length(Ch);
    
    if (k + Width <= Length)
    {
      CharUtf8Encode_(Ch, Value + k);
    };
    k += Width;
    i += Advance;
  };
  return k;
};

string
StringToLower(string String, arena* Arena)
{
  string Out = {0};
  usize Length = StringTransform(String, 0, 0, CharToLower);
  Out.Value = ArenaPush(Arena, Length + 1, alignof(u8));
  if (Out.Value)
  {
    Out.Length = Length;
    StringTransform(String, Out.Value, Out.Length, CharToLower);
    Out.Value[Length] = 0;
  };
  return Out;
};

string
StringToUpper(string String, arena* Arena)
{
  string Out = {0};
  usize Length = StringTransform(String, 0, 0, CharToUpper);
  Out.Value = ArenaPush(Arena, Length + 1, alignof(u8));
  if (Out.Value)
  {
    Out.Length = Length;
    StringTransform(String, Out.Value, Out.Length, CharToUpper);
    Out.Value[Length] = 0;
  };
  return Out;
};

string
StringSwapcase(string String, arena* Arena)
{
  string Out = {0};
  usize Length = StringTransform(String, 0, 0, CharSwapCase);
  Out.Value = ArenaPush(Arena, Length + 1, alignof(u8));
  if (Out.Value)
  {
    Out.Length = Length;
    StringTransform(String, Out.Value, Out.Length, CharSwapCase);
    Out.Value[Length] = 0;
  };
  return Out;
};

string
StringCasefold(string String, arena* Arena)
{
  string Out = {0};
  usize Length = StringTransform(String, 0, 0, CharCasefold);
  Out.Value = ArenaPush(Arena, Length + 1, alignof(u8));
  if (Out.Value)
  {
    Out.Length = Length;
    StringTransform(String, Out.Value, Out.Length, CharCasefold);
    Out.Value[Length] = 0;
  };
  return Out;
};

string
StringReverse(string String, arena* Arena)
{
  string Out = {0};
  Out.Value = ArenaPush(Arena, String.Length + 1, alignof(u8));
  
  if (Out.Value)
  {
    Out.Length = String.Length;
    usize k = String.Length - 1;
    usize i = 0;
    while (i < String.Length)
    {
      usize Advance = CharUtf8Advance(String.Value[i]);
      
      if (Advance)
      {
        k -= Advance;
        MemoryCopy(Out.Value + k, String.Value + i, Advance);
        i += Advance;
      } else 
      {
        Advance = 1;
        k -= Advance;
        Out.Value[k] = String.Value[i];
        i += Advance;
      };
    };
    Out.Value[Out.Length] = 0;
  };
  return Out;
};

static usize
StringCapitalize_(string String, u8* Value, usize Length)
{
  if (!Value) Length = 0;
  
  usize i = 0;
  usize k = 0;
  while (i < String.Length)
  {
    usize Advance = CharUtf8Advance(String.Value[i]);
    
    if (Advance == 0)
    {
      i++;
      continue;
    };
    u32 Ch = CharUtf8Decode_(String.Value + i);
    
    if (i) Ch = CharToLower(Ch);
    else Ch = CharToUpper(Ch);
    
    usize Width = CharUtf8Length(Ch);
    
    if (k + Width <= Length)
    {
      CharUtf8Encode_(Ch, Value + k);
    };
    k += Width;
    i += Advance;
  };
  return k;
};

string
StringCapitalize(string String, arena* Arena)
{
  string Out = {0};
  usize Length = StringCapitalize_(String, 0, 0);
  
  Out.Value = ArenaPush(Arena, Length + 1, alignof(u8));
  if (Out.Value)
  {
    Out.Length = Length;
    StringCapitalize_(String, Out.Value, Length);
    Out.Value[Length] = 0;
  };
  return Out;
};

static usize
StringTitle_(string String, u8* Value, usize Length)
{
  if (!Value) Length = 0;
  
  usize i = 0;
  usize k = 0;
  u32 PrevChar = 0;
  while (i < String.Length)
  {
    usize Advance = CharUtf8Advance(String.Value[i]);
    
    if (Advance == 0)
    {
      i++;
      continue;
    };
    u32 Ch = CharUtf8Decode_(String.Value + i);
    
    if (i && CharIsAlnum(PrevChar)) Ch = CharToLower(Ch);
    else Ch = CharToUpper(Ch);
    
    usize Width = CharUtf8Length(Ch);
    
    if (k + Width <= Length)
    {
      CharUtf8Encode_(Ch, Value + k);
    };
    k += Width;
    i += Advance;
    PrevChar = Ch;
  };
  return k;
};

string
StringTitle(string String, arena* Arena)
{
  usize Length = StringTitle_(String, 0, 0);
  string Out = {0};
  Out.Value = ArenaPush(Arena, Length + 1, alignof(u8));
  
  if (Out.Value)
  {
    Out.Length = Length;
    StringTitle_(String, Out.Value, Length);
    Out.Value[Length] = 0;
  };
  return Out;
};

// String formatting and finding

static string
StringJust(string String, usize Width, u32 FillChar, arena* Arena, u32 Kind)
{
  usize FillByteCount = CharUtf8Length(FillChar);
  if (!FillByteCount) return String;
  
  usize CharCount = StringCount(String);
  
  if (Width <= CharCount) return String;
  
  usize DLeft, DRight;
  
  if (Kind == 2)
  {
    DLeft = (Width - CharCount) / 2;
    DRight = Width - DLeft - CharCount;
  } else if (Kind == 1)
  {
    DLeft = Width - CharCount;
    DRight = 0;
  } else 
  {
    DLeft = 0;
    DRight = Width - CharCount;
  };
  
  usize Length = (DLeft + DRight) * FillByteCount + String.Length;
  
  u8 Parts[4];
  CharUtf8Decode_(Parts);
  
  string Out = {0};
  Out.Value = ArenaPush(Arena, Length + 1, alignof(u8));
  
  if (Out.Value)
  {
    Out.Length = Length;
    
    for (usize i = 0; i < DLeft; i++)
    {
      MemoryCopy(Out.Value + i * FillByteCount, Parts, FillByteCount);
    };
    
    usize R = DLeft * FillByteCount + String.Length;
    
    for (usize i = 0; i < DRight; i++)
    {
      MemoryCopy(Out.Value + (R + i * FillByteCount) , Parts, FillByteCount);
    };
    
    MemoryCopy(Out.Value + DLeft * FillByteCount, String.Value, String.Length);
    
    Out.Value[Length] = 0;
  };
  return Out;
};

string
StringCenter(string String, usize Width, u32 FillChar, arena* Arena)
{
  return StringJust(String, Width, FillChar, Arena, 2);
};

string
StringLjust(string String, usize Width, u32 FillChar, arena* Arena)
{
  return StringJust(String, Width, FillChar, Arena, 0);
};

string
StringRjust(string String, usize Width, u32 FillChar, arena* Arena)
{
  return StringJust(String, Width, FillChar, Arena, 1);
};

usize
StringCount(string String)
{
  #if BUILTIN_DISABLE_UNICODE
  return String.Length;
  #else 
  usize Count = 0;
  usize i = 0;
  while (i < String.Length)
  {
    usize Advance = MaxU(1, CharUtf8Advance(String.Value[i]));
    if (String.Length < i + Advance) break;
    i += Advance;
    Count++;    
  };
  return Count;
  #endif // BUILTIN_DISABLE_UNICODE
};

usize
StringCountSub(string String, string Sub)
{
  if (!Sub.Length) return StringCount(String) + 1;
  
  usize Count = 0;
  
  for (usize x = 0; x < String.Length; )
  {
    if (String.Length < x + Sub.Length) break;
    usize y = 0;
    for (; y < Sub.Length; y++)
    {
      if (String.Value[x + y] != Sub.Value[y]) break;
    };
    
    if (y == Sub.Length)
    {
      Count++;
      x += Sub.Length;
    } else 
    {
      x++;
    };
  };
  return Count;
};

isize
StringFind(string String, string Sub)
{
  if (!Sub.Length) return 0;
  
  for (usize x = 0; x; x++)
  {
    usize y = 0;
    for (; y < Sub.Length; x++)
    {
      if (String.Value[x + y] != Sub.Value[y]) break;
    };
    
    if (y == Sub.Length) return x;
  };
  return -1;
};

isize
StringRfind(string String, string Sub)
{
  if (!Sub.Length) return String.Length;
  
  for (usize x = String.Length; x; x--)
  {
    usize y = 0;
    for (; y < Sub.Length; x++)
    {
      if (String.Value[x + y] != Sub.Value[y]) break;
    };
    
    if (y == Sub.Length) return x;
  };
  return -1;
};

isize
StringIndex(string String, u32 Char)
{
  u8 Parts[4];
  usize Length = CharUtf8Encode_(Char, Parts);
  string S = {.Value = Parts, .Length = Length};
  return StringFind(String, S);
};

isize
StringRindex(string String, u32 Char)
{
  u8 Parts[4];
  usize Length = CharUtf8Encode_(Char, Parts);
  string S = {.Value = Parts, .Length = Length};
  return StringRfind(String, S);
};

string*
StringSplit(string String, string Sep, usize* Count, u32 Right, arena* Arena)
{
  string* Out = 0;
  usize ACount = 0;
  if (Sep.Length != 0)
  {
    ACount = StringCount(String);
    Out = ArenaPushN(Arena, sizeof(string), ACount, alignof(string));
    
    if (Out)
    {
      usize k = 0;
      for (usize i = 0; i < String.Length;)
      {
        usize Advance = MaxU(1, CharUtf8Advance(String.Value[i]));
        string Slice = {.Value = String.Value + i, .Length = Advance};
        Out[k++] = StringClone(Slice, Arena);
        i += Advance;
      };
    };
  } else 
  {
    ACount = StringCountSub(String, Sep) + 1;
    Out = ArenaPushN(Arena, sizeof(string), ACount, alignof(string));
    
    if (Out)
    {
      usize Start = 0;
      usize h = 0;
      
      for (usize x = 0; x < String.Length;)
      {
        if (String.Length < x + Sep.Length) break;
        
        if (MemoryEqual(String.Value + x, Sep.Value, Sep.Length))
        {
          string Split = {String.Value + Start, x - Start};
          Out[h++] = StringClone(Split, Arena);
          x += Sep.Length;
          Start = x;
        } else x++;
      };
      string Slice = {String.Value + Start, String.Length - Start};
      Out[h++] = StringClone(Slice, Arena);
    };
  };
  
  if (Count) *Count = ACount;
  return Out;
};

string*
StringSplitSpace(string String, usize* Count, arena* Arena)
{
  usize ACount = 0;
  
  for (usize i = 0; 1; )
  {
    while (i < String.Length && CharIsSpace(String.Value[i])) i++;
    usize x = i;
    while (i < String.Length && !CharIsSpace(String.Value[i])) i++;
    usize L = i - x;
    if (L) break;
    ACount++;
  };
  
  string* Out = ArenaPushN(Arena, sizeof(string), ACount, alignof(string));
  
  if (Out)
  {
    usize k = 0;
    for (usize i = 0; 1; )
    {
      while (i < String.Length && CharIsSpace(String.Value[i])) i++;
      usize x = i;
      while (i < String.Length && !CharIsSpace(String.Value[i])) i++;
      usize L = i - x;
      if (L) break;
      string Slice = {String.Value + x, L};
      Out[k++] = StringClone(Slice, Arena);
    };
  };
  
  if (Count) *Count = ACount;
  return Out;  
};

static usize
StringPeekLine(const u8* Value, usize Length)
{
  usize Span = 0;
  
  switch (Value[0])
  {
    case '\r':
    {
      if (1 < Length && Value[1] == '\n') Span = 2;
      else Span = 1;
    } break;
    case '\n':
    case '\v': 
    case '\f': Span = 1; break;
  };
  return Span;
};
string*
StringSplitLines(string String, usize* Count, u32 KeepEnds, arena* Arena)
{
  usize ACount = 0;
  
  for (usize i = 0; 1;)
  {
    usize x = i;
    u32 LineSpan = 0;
    
    while (i < String.Length)
    {
      LineSpan = StringPeekLine(String.Value + i, String.Length - i);
      if (LineSpan) break;
      i++;
    };
    
    usize L = i - x;
    
    if (!L) break;
    x += LineSpan;
    ACount++;
  };
  
  string* Out = ArenaPushN(Arena, sizeof(string), ACount, alignof(string));
  
  if (Out)
  {
    usize k = 0;
    for (usize i = 0; 1;)
    {
      usize x = i;
      u32 LineSpan = 0;
      
      while (i < String.Length)
      {
        LineSpan = StringPeekLine(String.Value + i, String.Length - i);
        if (LineSpan) break;
        i++;
      };
      
      usize L = i - x + KeepEnds ? LineSpan : 0;
      if (!L) break;
      
      string Line = {String.Value + x, L};
      Out[k++] = StringClone(Line, Arena);
      i += LineSpan;
    };
  };
  
  if (Count) *Count = ACount;
  return Out;
};

static string
StringStrip_(string String, u32 Direction, arena* Arena)
{
  usize Lpadding = 0;
  usize RPadding = 0;
  
  if (Direction == 0 || Direction == 2)
  {
    while (Lpadding < String.Length && CharIsSpace(String.Value[Lpadding])) 
    {
      Lpadding++;
    };
  };
  
  if (Direction == 1 || Direction == 2)
  {
    while (RPadding < String.Length && CharIsSpace(String.Value[String.Length - RPadding - 1]))
    {
      RPadding++;
    };
  };
  
  string Stripped = {String.Value + Lpadding, String.Length - MinU(String.Length, (Lpadding + RPadding))};
  return StringClone(Stripped, Arena);
};

string
StringStrip(string String, arena* Arena)
{
  return StringStrip_(String, 2, Arena);
};

string
StringLStrip(string String, arena* Arena)
{
  return StringStrip_(String, 0, Arena);
};

string
StringRStrip(string String, arena* Arena)
{
  return StringStrip_(String, 1, Arena);
};

string
StringReplace(string String, string Sub, string Repl, arena* Arena)
{
  usize SubCount = StringCountSub(String, Sub);
  usize OtherLength = String.Length - Sub.Length * SubCount;
  usize Length = OtherLength + Repl.Length * SubCount;
  
  string Out = {0};
  Out.Value = ArenaPush(Arena, Length + 1, alignof(u8));
  
  if (Out.Value)
  {
    if (!Sub.Length)
    {
      usize i = 0;
      usize k = 0;
      while (i < String.Length)
      {
        usize Advance = CharUtf8Advance(String.Value[i]);
        
        if (String.Length < i + Advance) break;
        
        if (Advance)
        {
          MemoryCopy(Out.Value + k, String.Value + i, Advance);
          i += Advance;
          k += Advance;
        } else 
        {
          Out.Value[k++] = String.Value[i++];
        };
        MemoryCopy(Out.Value + k, Repl.Value, Repl.Length);
        k += Repl.Length;
      };
    } else 
    {
      usize i = 0;
      usize k = 0;
      while (i < String.Length)
      {
        if (String.Length < i + Sub.Length)
        {
          MemoryCopy(Out.Value + k, String.Value + i, String.Length - i);
          break;
        };
        
        if (MemoryEqual(String.Value + i, Sub.Value, Sub.Length))
        {
          MemoryCopy(Out.Value + k, Repl.Value, Repl.Length);
          k += Repl.Length;
          i += Sub.Length;
        } else 
        {
          Out.Value[k++] = String.Value[i++];
        };
      };
    };
    Out.Value[Length] = 0;
  };
  return Out;
};

string
StringExpandTabs(string String, u32 TabSize, arena* Arena)
{
  usize TabCount = 0;
  for (usize i = 0; i < String.Length; i++)
  {
    if (String.Value[i] == '\t') TabCount++;
  };
  usize Length = (String.Length - TabCount) + TabCount * TabSize;
  
  string Out = {0};
  Out.Value = ArenaPush(Arena, Length + 1, alignof(u8));
  
  if (Out.Value)
  {
    Out.Length = Length;
    usize k = 0;
    usize Columns = 0;
    for (usize i = 0; i < String.Length; i++)
    {
      if (String.Value[i] == '\t')
      {
        usize Spaces = TabSize - (Columns % TabSize);
        for (usize s = 0; s < Spaces; s++) Out.Value[k++] = ' ';
        Columns += Spaces;
      } else
      {
        Out.Value[k++] = String.Value[i];
        Columns++;
      };
    };
    Out.Value[Length] = 0;
  };
  
  return Out;
};

string
StringRemovePrefix(string String, string Prefix, arena* Arena)
{
  if (!StringStartsWith(String, Prefix)) return String;
  
  string Slice = {String.Value + Prefix.Length, String.Length - Prefix.Length};
  return StringClone(Slice, Arena);
};

string
StringRemoveSuffix(string String, string Suffix, arena* Arena)
{
  if (!StringEndsWith(String, Suffix)) return String;
  
  string Slice = {String.Value, String.Length - Suffix.Length};
  return StringClone(Slice, Arena);
};

usize
StringCompare(string A, string B)
{
  if (A.Length != B.Length) return 0;
  return MemoryCompare(A.Value, B.Value, A.Length);
};

u32
StringEqual(string A, string B)
{
  if (A.Length != B.Length) return 0;
  return MemoryEqual(A.Value, B.Value, A.Length);
};

u32
StringStartsWith(string String, string Prefix)
{
  if (String.Length < Prefix.Length) return 0;
  return MemoryEqual(String.Value, Prefix.Value, Prefix.Length);
};

u32
StringEndsWith(string String, string Suffix)
{
  if (String.Length < Suffix.Length) return 0;
  return MemoryEqual(String.Value + (String.Length - Suffix.Length), Suffix.Value, Suffix.Length);
};

u32
StringContains(string String, string Sub)
{
  return StringFind(String, Sub) > 0;
};

usize
StringCompareCI(string A, string B)
{
  usize Count = MinU(A.Length, B.Length);
  
  for (int i = 0; i < Count; i++) 
  {
    u8 a = CharToLower(A.Value[i]);
    u8 b = CharToLower(B.Value[i]);
    
    if (a != b) return i;
  };
  return Count;
};

u32
StringEqualCI(string A, string B)
{
  if (A.Length != B.Length) return 0;
  return StringCompareCI(A, B) == A.Length;
};

// Ported from https://github.com/odin-lang/Odin/blob/master/core/strconv/strconv.odin
typedef struct _double_parse _double_parse;
struct _double_parse
{
  double Value;
  int End;
  u32 Ok;
};

typedef struct _double_parse_component _double_parse_component;
struct _double_parse_component
{
  u64 Mantissa;
  int Exponent;
  int Truncate;
  int Hex;
  int i;
  u32 Negative;
  u32 Ok;
};

static _double_parse 
DoubleParseLiteral(string s)
{
  _double_parse Result = {0};
  
  if (s.Length == 0) return Result;
  
  int sign = 1;
  int nsign = 0;
  
  char c0 = s.Value[0];
  
  if (c0 == '+' || c0 == '-') 
  {
    if (c0 == '-') sign = -1;
    
    nsign = 1;
    s.Value += 1;
    s.Length -= 1;
  };
  
  if (s.Length == 0) return Result;
  
  switch (s.Value[0]) 
  {
    case 'i':
    case 'I':
    {
      usize m = StringCompareCI(s, S("infinity"));
      
      if (m >= 3 && m < 9) 
      {
        Result.Value = sign * INFINITY;
        
        if (m == 8) Result.End = nsign + m;
        else Result.End = nsign + 3;
        
        Result.Ok = 1;
        return Result;
      }
    } break;
    
    case 'n':
    case 'N':
    {
      if (StringCompareCI(s, S("nan")) == 3) 
      {
        Result.Value = NAN;
        Result.End = nsign + 3;
        Result.Ok = 1;
        return Result;
      };
    } break;
  };
  
  return Result;
};


static _double_parse_component 
DoubleParseComponents(string s)
{
  _double_parse_component Result = {0};
  
  if (s.Length == 0) return Result;
  
  int i = 0;
  
  if (s.Value[i] == '+') 
  {
    i++;
  } else if (s.Value[i] == '-') 
  {
    Result.Negative = 1;
    i++;
  }
  
  u64 Base = 10;
  int MAX_MANT_DIGITS = 19;
  char ExpChar = 'e';
  
  if (i + 2 < (int)s.Length && s.Value[i] == '0' && CharToLower(s.Value[i+1]) == 'x')
  {
    Base = 16;
    MAX_MANT_DIGITS = 16;
    i += 2;
    ExpChar = 'p';
    Result.Hex = 1;
  };
  
  int SawDot = 0;
  int SawDigits = 0;
  
  int Nd = 0;
  int NdMant = 0;
  int DecimalPoint = 0;
  
  for (; i < (int)s.Length; i++) 
  {
    
    char c = s.Value[i];
    
    if (c == '_') continue;
    
    if (c == '.') 
    {
      if (SawDot) break;
      
      SawDot = 1;
      DecimalPoint = Nd;
      continue;
    }
    
    if ('0' <= c && c <= '9') 
    {
      
      SawDigits = 1;
      Nd++;
      
      if (NdMant < MAX_MANT_DIGITS) 
      {
        Result.Mantissa *= Base;
        Result.Mantissa += (u64)(c - '0');
        NdMant++;
      } else if (c != '0') 
      {
        Result.Truncate = 1;
      }
      
      continue;
    }
    
    if (Base == 16) 
    {
      char Lch = CharToLower(c);
      
      if ('a' <= Lch && Lch <= 'f') 
      {
        SawDigits = 1;
        Nd++;
        
        if (NdMant < MAX_MANT_DIGITS) 
        {
          Result.Mantissa *= 16;
          Result.Mantissa += (u64)(Lch - 'a' + 10);
          NdMant++;
        } else {
          Result.Truncate = 1;
        }
        
        continue;
      }
    }
    
    break;
  }
  
  if (!SawDigits) return Result;
  
  if (!SawDot) DecimalPoint = Nd;
  
  if (Base == 16) 
  {
    DecimalPoint *= 4;
    NdMant *= 4;
  }
  
  if (i < (int)s.Length && CharToLower(s.Value[i]) == ExpChar) 
  {
    
    i++;
    
    int ExpSign = 1;
    
    if (s.Value[i] == '+') 
    {
      i++;
    } else if (s.Value[i] == '-') 
    {
      ExpSign = -1;
      i++;
    }
    
    int e = 0;
    
    for (; i < (int)s.Length; i++) 
    {
      
      char c = s.Value[i];
      
      if (c == '_') continue;
      
      if (c < '0' || c > '9') break;
      
      if (e < 100000) e = e*10 + (c - '0');
    }
    
    DecimalPoint += e * ExpSign;
    
  } else if (Base == 16) 
  {
    return Result;
  }
  
  if (Result.Mantissa != 0) Result.Exponent = DecimalPoint - NdMant;
  
  Result.i = i;
  Result.Ok = 1;
  
  return Result;
};

static double
DoubleParsePow10(int e)
{
  static const double Powers[] = 
  {
    1e1, 1e2, 1e4, 1e8, 1e16, 1e32, 1e64, 1e128, 1e256
  };
  double Result = 1.0;
  
  if (e < 0)
  {
    e = -e;
    for (int i = 0; e; i++, e >>= 1)
    {
      if (e & 1) Result *= Powers[i];
    };
    return 1.0 / Result;
  };
  for (int i = 0; e; i++, e >>= 1)
  {
    if (e & 1) Result *= Powers[i];
  };  
  return Result;
};

static _double_parse 
DoubleParse(string str)
{
  _double_parse Result = {0};
  
  _double_parse ParseLiteral = DoubleParseLiteral(str);
  if (ParseLiteral.Ok) return ParseLiteral;
  _double_parse_component ParseComponent = DoubleParseComponents(str);
  
  if (!ParseComponent.Ok) return Result;
  
  Result.End = ParseComponent.i;
  
  double f = (double)ParseComponent.Mantissa;
  
  if (ParseComponent.Negative) f = -f;
  
  if (ParseComponent.Exponent != 0) f *= DoubleParsePow10(ParseComponent.Exponent);
  
  Result.Value = f;
  Result.Ok = 1;
  
  return Result;
};

double
StringToFloat(string Str, u8** End)
{
  _double_parse Result = DoubleParse(Str);
  if (End) *End = Result.Ok ? 0 : Str.Value + Result.End;
  return Result.Value;
};

// Parse integers
typedef struct _int_parse _int_parse;
struct _int_parse
{
  i64 Value;
  int End;
  u32 Ok;
};

static _int_parse
IntParse(string Value)
{
  _int_parse Result = {0};
  int i = 0;
  int Negative = 0;
  
  
  switch (i < Value.Length ? Value.Value[i] : 0)
  {
    case '+': i++; break;
    case '-': i++; Negative = 1; break;
  };
  
  
  int Base = 10;
  u32 SawDigits = 0;
  
  if (i < Value.Length && CharToLower(Value.Value[i]) == '0')
  {
    
    if (i + 1 < Value.Length)
    {
      u8 Ch = CharToLower(Value.Value[i + 1]);
      
      if (Ch == 'x')
      {
        Base = 16;
        i += 2;
      } else if (Ch == 'b')
      {
        Base = 2;
        i += 2;
      } else if (Ch == 'o')
      {
        Base = 8;
        i += 2;
      };
    } else 
    {
      Base = 8; // 012...
    };
  };
  
  
  for (; i < Value.Length; i++)
  {
    u8 Ch = Value.Value[i];
    
    if (Ch == '_') continue;
    
    int D = CharToDigit(Ch);
    if (D < 0 || D >= Base) break;
    
    SawDigits  = 1;
    
    Result.Value = Result.Value * Base + D;
  };
  
  Result.Ok = i == Result.End && SawDigits;
  Result.End = i;
  Result.Value = Negative ? -Result.Value : Result.Value;
  return Result;
};

i64
StringToInt(string Value, u8** End)
{
  _int_parse Parse = IntParse(Value);
  
  if (End) *End = Parse.Ok ? 0 : Value.Value + Parse.End;
  return Parse.Value;
};
