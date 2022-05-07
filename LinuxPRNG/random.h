//
//  random.h
//  LinuxPRNG
//
//  Created by Ziqin Gong on 2022/5/7.
//

#ifndef random_h
#define random_h

#include <stdio.h>

void mix(uint8_t pool, uint8_t size, uint8_t input[]);
void output(uint8_t pool, uint8_t requested_num, uint8_t output_buffer[]);

#endif /* random_h */
