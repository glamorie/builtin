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
