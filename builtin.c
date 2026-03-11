#include "builtin.h"
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