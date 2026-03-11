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