#ifndef _SEMACODE_H
#define _SEMACODE_H

#include <strings.h>
#include "iec16022ecc200.h"

typedef struct semacode_t {
  int width;
  int height;
  int raw_encoded_length;
  int symbol_capacity;
  int ecc_bytes;
  char *encoding;
  char *data;
} semacode_t;

#ifndef RB_STRING_VALUE
#define RB_STRING_VALUE(s) (TYPE(s) == T_STRING ? (s) : (*(volatile VALUE *)&(s) = rb_str_to_str(s)))
#endif

#ifndef StringValuePtr
#define StringValuePtr(s) RSTRING_LEN(RB_STRING_VALUE(s))
#endif

#ifndef StringValueLen
#define StringValueLen(s) RSTRING_LEN(RB_STRING_VALUE(s))
#endif

#endif
