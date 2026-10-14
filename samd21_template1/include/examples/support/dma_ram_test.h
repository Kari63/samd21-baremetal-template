/*
 * dma_ram_test.h
 *
 * Created: 21.9.2026 20.58.14
 *  Author: Kari63
 */ 

#pragma once

#include <stdint.h>
#include <stddef.h>

#define BLOCK_SIZE  64  // 64-byte page/block - divisible by 4



typedef struct {
	volatile uint32_t canary_pre;        // 4 bytes (0xCCCCCCCC)
	volatile uint8_t  block[BLOCK_SIZE]; // 64 bytes (divisible by 4)
	volatile uint32_t canary_post;       // 4 bytes (0xCCCCCCCC)
} __attribute__((aligned(4))) guarded_buf_t;

static_assert((BLOCK_SIZE % sizeof(uint32_t)) == 0, "BLOCK_SIZE must be word-aligned");
static_assert(offsetof(guarded_buf_t, canary_pre) == 0, "canary_pre offset changed");
static_assert(offsetof(guarded_buf_t, block) == sizeof(uint32_t), "block offset changed");
static_assert(offsetof(guarded_buf_t, canary_post) == sizeof(uint32_t) + BLOCK_SIZE,
"canary_post offset changed");
static_assert(sizeof(guarded_buf_t) == (2 * sizeof(uint32_t)) + BLOCK_SIZE,
"unexpected guarded buffer padding");

extern volatile guarded_buf_t src_buf;
extern volatile guarded_buf_t dst_buf;


 void fill_test_pattern(void);