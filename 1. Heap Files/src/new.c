#include "bf.h"

int main(void){
  BF_Block *block;
  BF_Block_Init(&block);
  printf("Add:-%p\n", (void*)block);
}

