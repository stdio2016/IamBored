#include <stdio.h>

#if __STDC_VERSION__
#include <stdbool.h>
typedef bool Bool;
#else
typedef int Bool;
#endif

#define TLB_SIZE 16
#define PAGE_TABLE_SIZE 256
#define MEM_SIZE 65536

struct TLB {
  Bool valid;
  int page;
  int frame;
} tlb[TLB_SIZE];

struct PageTableEntry {
  Bool valid;
  int frame;
} pageTable[PAGE_TABLE_SIZE];

signed char mem[MEM_SIZE];

int main(void)
{
  FILE *fAddr = fopen("addresses.txt", "r");
  FILE *fStore = fopen("BACKING_STORE.bin", "rb");
  if (fAddr == NULL || fStore == NULL) {
    puts("Open file error");
    return 1;
  }

  FILE *gAddr = fopen("0xxxxxx_address.txt", "w");
  FILE *gValue = fopen("0xxxxxx_value.txt", "w");
  if (gAddr == NULL || gValue == NULL) {
    puts("Write file error");
    return 1;
  }

  int i;
  for (i = 0; i < TLB_SIZE; i++) {
    tlb[i].valid = 0;
  }
  for (i = 0; i < PAGE_TABLE_SIZE; i++) {
    pageTable[i].valid = 0;
  }

  int address;
  while (fscanf(fAddr, "%d", &address) == 1) {
    address &= 0xffff;
    int pageN = address >> 8;
    int offset = address & 0xff;

    // search from tlb
    int j;
    for (j = 0; j < TLB_SIZE; j++) {
      // tlb hit
      if (tlb[j].valid && tlb[j].page == pageN)
        break;
    }
    if (j < TLB_SIZE) { // tlb hit
    }
    else { // tlb miss
    }
  }

  fclose(fAddr);
  fclose(fStore);
  fclose(gAddr);
  fclose(gValue);
  return 0;
}
