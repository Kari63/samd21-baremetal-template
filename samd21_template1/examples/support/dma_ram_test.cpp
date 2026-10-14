/*
 * dma_ram_test.cpp
 *
 * Created: 21.9.2026 20.57.32
 *  Author: Kari63
 */ 

#include "examples/support/dma_ram_test.h"

#define src_canary_pre  (src_buf.canary_pre)
#define src_block       (src_buf.block)
#define src_canary_post (src_buf.canary_post)
#define dst_canary_pre  (dst_buf.canary_pre)
#define dst_block       (dst_buf.block)
#define dst_canary_post (dst_buf.canary_post)

volatile guarded_buf_t src_buf;
volatile guarded_buf_t dst_buf;


void fill_test_pattern(void)
{
	for (uint32_t i = 0; i < BLOCK_SIZE; i++) {
		src_buf.block[i] = 0x40;
		dst_buf.block[i] = 0x55;
	}

	src_buf.canary_pre = 0xCCCCCCCC;
	src_buf.canary_post = 0xCCCCCCCC;
	dst_buf.canary_pre = 0xCCCCCCCC;
	dst_buf.canary_post = 0xCCCCCCCC;
}

