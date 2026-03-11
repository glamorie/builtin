#include "builtin.h"

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
