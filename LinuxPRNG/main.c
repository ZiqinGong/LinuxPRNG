//
//  main.c
//  LinuxPRNG
//
//  Created by Ziqin Gong on 2022/5/7.
//

#include <stdio.h>

#include "random.h"

int main(int argc, const char* argv[]) {
  uint8_t input[10], out[10];
  for (int i = 0; i < 10; ++i) input[i] = i * i + 1;

  mix(128, 10, input);
  output(128, 10, out);

  for (int i = 0; i < 10; ++i) printf("%x\n", out[i]);

  return 0;
}
