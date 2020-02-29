#ifndef __CACHE_H__
#define __CACHE_H__

#include "nemu.h"

typedef struct {
    int valid;         // one-bit valid bit
    uint32_t mark;     // nineteen bits' mark bit
    uint8_t data[64];
} Cache;

void init_cache();
uint32_t cache_read(paddr_t paddr, size_t len);
void cache_write(paddr_t paddr, size_t len, uint32_t data);

#endif
