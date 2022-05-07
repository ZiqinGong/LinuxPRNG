//
//  random.c
//  LinuxPRNG
//
//  Created by Ziqin Gong on 2022/5/7.
//

#include "random.h"

#include <inttypes.h>
#include <stdio.h>

#include "sha1.h"

uint8_t _last_pos_input = 128;
uint8_t _last_pos_output = 32;
uint32_t _input_pool[128];
uint32_t _output_pool[32];

uint32_t _rotate_left(uint32_t data, uint8_t rot) {
  rot %= 32;
  return ((data << rot) | (data >> (32 - rot)));
}

void mix(uint8_t pool, uint8_t size, uint8_t input[]) {
  uint8_t i = pool;
  uint8_t rot = 0;
  uint8_t fb_pos[6];
  uint32_t w = 0;

  if (pool == 128) {
    // Mix into the input pool
    i = _last_pos_input;
    for (int j = 0; j < size; ++j) {
      i = (i - 1) % 128;
      fb_pos[0] = (i + 1) % 128;
      fb_pos[1] = (i + 25) % 128;
      fb_pos[2] = (i + 51) % 128;
      fb_pos[3] = (i + 76) % 128;
      fb_pos[4] = (i + 103) % 128;
      fb_pos[5] = (i + 128) % 128;

      w = (uint32_t)input[j];
      w = _rotate_left(w, rot);

      for (int k = 0; k < 6; ++k) w ^= _input_pool[fb_pos[k]];
      _input_pool[i] = w;

      if (i == 0)
        rot = (rot + 14) % 32;
      else
        rot = (rot + 7) % 32;
    }
    _last_pos_input = i;
  } else {
    // Mix into the output pool
    i = _last_pos_output;
    for (int j = 0; j < size; ++j) {
      i = (i - 1) % 32;
      fb_pos[0] = (i + 1) % 32;
      fb_pos[1] = (i + 6) % 32;
      fb_pos[2] = (i + 13) % 32;
      fb_pos[3] = (i + 19) % 32;
      fb_pos[4] = (i + 25) % 32;
      fb_pos[5] = (i + 31) % 32;

      w = (uint32_t)input[j];
      w = _rotate_left(w, rot);

      for (int k = 0; k < 6; ++k) w ^= _input_pool[fb_pos[k]];
      _input_pool[i] = w;

      if (i == 0)
        rot = (rot + 14) % 32;
      else
        rot = (rot + 7) % 32;
    }
    _last_pos_output = i;
  }
}

void output(uint8_t pool, uint8_t requested_num, uint8_t output_buffer[]) {
  SHA1_CTX ctx;
  unsigned char buffer[64], hash[20];
  uint8_t j = 0, r = 0, t[4], buf[20], fold[10];
  uint32_t tmp = 0, w[16], b[5] = {0};

  while (j < requested_num) {
    if (pool == 128) {
      // Generate data from the input pool
      SHA1Init(&ctx);

      // Phase I: Hash the whole pool and mix back
      for (uint8_t l = 0; l < 8; ++l) {
        for (uint8_t i = 0; i < 16; ++i) {
          tmp = _input_pool[16 * l + i];
          t[0] = tmp >> 24;
          t[1] = (tmp << 8) >> 24;
          t[2] = (tmp << 16) >> 24;
          t[3] = (tmp << 24) >> 24;
          for (uint8_t k = 0; k < 4; ++k)
            buffer[4 * i + k] = (unsigned char)t[k];
        }
        // SHA1Update(&ctx, buffer, 64);
        SHA1Transform(b, buffer);
        SHA1Final(hash, &ctx);
        for (uint8_t k = 0; k < 20; ++k) buf[k] = (uint8_t)hash[k];
        for (uint8_t k = 0; k < 5; ++k)
          b[k] = ((uint32_t)buf[4 * k] << 24) +
                 ((uint32_t)buf[4 * k + 1] << 16) +
                 ((uint32_t)buf[4 * k + 2] << 8) + (uint32_t)buf[4 * k + 3];
      }
      SHA1Final(hash, &ctx);
      for (uint8_t k = 0; k < 20; ++k) buf[k] = (uint8_t)hash[k];
      mix(pool, 20, buf);

      // Phase II: Produce output data
      for (uint8_t l = 0; l < 16; ++l)
        w[l] = _input_pool[(_last_pos_input - l) % 128];
      for (uint8_t i = 0; i < 16; ++i) {
        tmp = w[i];
        t[0] = tmp >> 24;
        t[1] = (tmp << 8) >> 24;
        t[2] = (tmp << 16) >> 24;
        t[3] = (tmp << 24) >> 24;
        for (uint8_t k = 0; k < 4; ++k) buffer[4 * i + k] = (unsigned char)t[k];
      }
      SHA1Update(&ctx, buffer, 64);
      SHA1Final(hash, &ctx);
      for (uint8_t k = 0; k < 20; ++k) buf[k] = (uint8_t)hash[k];
      for (uint8_t k = 0; k < 10; ++k)
        fold[k] = buf[k] ^ buf[19 - k];  // Fold 20 bytes to 10 bytes
      r = (10 < requested_num - j) ? 10 : requested_num - j;
      for (uint8_t k = 0; k < r; ++k) output_buffer[j + k] = fold[k];

      j += 10;
    } else {
      // Generate data from the output pool
      SHA1Init(&ctx);

      // Phase I: Hash the whole pool and mix back
      for (uint8_t l = 0; l < 2; ++l) {
        for (uint8_t i = 0; i < 16; ++i) {
          tmp = _input_pool[16 * l + i];
          t[0] = tmp >> 24;
          t[1] = (tmp << 8) >> 24;
          t[2] = (tmp << 16) >> 24;
          t[3] = (tmp << 24) >> 24;
          for (uint8_t k = 0; k < 4; ++k)
            buffer[4 * i + k] = (unsigned char)t[k];
        }
        SHA1Update(&ctx, buffer, 64);
      }
      SHA1Final(hash, &ctx);
      for (uint8_t k = 0; k < 20; ++k) buf[k] = (uint8_t)hash[k];
      mix(pool, 20, buf);

      // Phase II: Produce output data
      for (uint8_t l = 0; l < 16; ++l)
        w[l] = _output_pool[(_last_pos_output - l) % 32];
      for (uint8_t i = 0; i < 16; ++i) {
        tmp = w[i];
        t[0] = tmp >> 24;
        t[1] = (tmp << 8) >> 24;
        t[2] = (tmp << 16) >> 24;
        t[3] = (tmp << 24) >> 24;
        for (uint8_t k = 0; k < 4; ++k) buffer[4 * i + k] = (unsigned char)t[k];
      }
      SHA1Update(&ctx, buffer, 64);
      SHA1Final(hash, &ctx);
      for (uint8_t k = 0; k < 20; ++k) buf[k] = (uint8_t)hash[k];
      for (uint8_t k = 0; k < 10; ++k)
        fold[k] = buf[k] ^ buf[19 - k];  // Fold 20 bytes to 10 bytes
      r = (10 < requested_num - j) ? 10 : requested_num - j;
      for (uint8_t k = 0; k < r; ++k) output_buffer[j + k] = fold[k];

      j += 10;
    }
  }
}
