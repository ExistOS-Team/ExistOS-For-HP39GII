#pragma once

#include "regs.h"
#include "regsdcp.h"

#ifdef __cplusplus 
extern "C" { 
#endif

typedef struct _dcp_descriptor
{
    reg32_t *next;
    hw_dcp_packet1_t ctrl0;
    hw_dcp_packet2_t ctrl1;
    reg32_t *src,
        *dst,
        buf_size,
        *payload,
        status;
} DCP_DESCRIPTOR;

typedef struct _dcp_context
{
    unsigned cipher_context : 16;
    unsigned hash_context   : 24;
} DCP_CONTEXT;

void dcp_init();
reg32_t dcp_memset(void *dst, int c, reg32_t size);
reg32_t dcp_memcpy(void *dst, void *src, reg32_t size);

#ifdef __cplusplus 
}
#endif