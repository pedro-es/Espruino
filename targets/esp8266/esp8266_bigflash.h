#ifndef __ESP8266_BIGFLASH_H__
#define __ESP8266_BIGFLASH_H__

#include <c_types.h>

extern const unsigned char _bigflash_free_start[];
extern const unsigned char _bigflash_free_end[];

#ifndef alloca
inline void *safe_alloca(size_t size) {
  void *frame = __builtin_alloca(0);
  if((size_t)frame - size < 0x3fffc000) return 0;
  return __builtin_alloca(size);
}
#define alloca safe_alloca
#endif

#endif
