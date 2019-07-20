/* A quick test to read the ssb core registers from ecos */
#include <stdio.h>


static inline int readl(int *addr)
{
	return *addr;
}

char __ramfs_runtime_data[1024];
char flash_block_array[1024];
int main(void)
{
  printf("Hello, eCos world!\n");
  
  printf("chip id: 0x%x\n"        , readl(0xa0000000 + 0x18000000 + 0x0000)&0x0000FFFF);
  printf("chip revision: 0x%x\n"  , readl(0xa0000000 + 0x18000000 + 0x0000)&0x000F0000);
  printf("package Options: 0x%x\n", readl(0xa0000000 + 0x18000000 + 0x0000)&0x00F00000);
  printf("number of cores 0x%x\n" , readl(0xa0000000 + 0x18000000 + 0x0000)&0x0F000000);
  
  printf("0:\n");
  printf("core id low: 0x%x\n", readl(0xa0000000 + 0x18000000 + 0x0FF8));
  printf("core id high: 0x%x\n", readl(0xa0000000 + 0x18000000 + 0x0FFC));	

  printf("2:\n");
  printf("core id low: 0x%x\n", readl(0xa0000000 + 0x18002000 + 0x0FF8));
  printf("core id high: 0x%x\n", readl(0xa0000000 + 0x18002000 + 0x0FFC));
  
  printf("1:\n");
  printf("core id low: 0x%x\n", readl(0xa0000000 + 0x18001000 + 0x0FF8));
  printf("core id high: 0x%x\n", readl(0xa0000000 + 0x18001000 + 0x0FFC));	

  printf("2:\n");
  printf("core id low: 0x%x\n", readl(0xa0000000 + 0x18002000 + 0x0FF8));
  printf("core id high: 0x%x\n", readl(0xa0000000 + 0x18002000 + 0x0FFC));
  	
  printf("3:\n");
  printf("core id low: 0x%x\n", readl(0xa0000000 + 0x18003000 + 0x0FF8));
  printf("core id high: 0x%x\n", readl(0xa0000000 + 0x18003000 + 0x0FFC));
  	
  printf("4:\n");
  printf("core id low: 0x%x\n", readl(0xa0000000 + 0x18004000 + 0x0FF8));
  printf("core id high: 0x%x\n", readl(0xa0000000 + 0x18004000 + 0x0FFC));	
  return 0;
}
