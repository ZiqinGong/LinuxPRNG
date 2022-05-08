//
//  main.c
//  LinuxPRNG
//
//  Created by Ziqin Gong on 2022/5/7.
//

#include <stdio.h>

#include "random.h"

int main(int argc, const char* argv[]) {
  uint8_t input[10], out[100];
  for (int i = 0; i < 10; ++i) input[i] = i * i;

  for (int i = 0; i < 1000; ++i) mix(128, 10, input);
  output(128, 100, out);

  for (int i = 0; i < 100; ++i) printf("%02x\n", out[i]);

  return 0;
}
