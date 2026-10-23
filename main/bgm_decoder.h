#pragma once
#include <stdbool.h>
#include <stddef.h>
#include <stdint.h>
// Stateless bounded reads: WAV IMA mono, 256-byte blocks, 505 samples/block.
bool bgm_decode(const uint8_t *data, size_t bytes, uint32_t samples,
                uint32_t cursor, int16_t *output, size_t count);
