#ifndef __CRC32_H
#define __CRC32_H
#include <stdint.h>
#include <stddef.h>

typedef struct {
    uint32_t crc;
} crc32_ctx;

void crc32_init(crc32_ctx *ctx);
void crc32_update(crc32_ctx *ctx, const uint8_t *data, size_t len);
void crc32_final(crc32_ctx *ctx, uint32_t *crc);

#endif
