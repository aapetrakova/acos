#include <stdint.h>
#include <string.h>

#include "ieee754_clf.h"

float_class_t classify(double x) {
  uint64_t bits;
  memcpy(&bits, &x, sizeof(double));

  uint64_t sign = (bits >> 63) & 0x01;
  uint64_t exp = (bits >> 52) & 0x7FF;
  uint64_t mantissa = bits & 0xFFFFFFFFFFFFF;

  if (exp == 0x7FF && mantissa != 0) { return NaN; }
  else if (exp == 0x7FF && mantissa == 0) { return (sign ? MinusInf : Inf); }
  else if (exp == 0 && mantissa == 0) { return (sign ? MinusZero : Zero); }
  else if (exp == 0) { return (sign ? MinusDenormal : Denormal); }
  else { return sign ? MinusRegular : Regular; }
}

