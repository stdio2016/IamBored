#include <stdio.h>

#if __STDC_VERSION__
#include <stdbool.h>
typedef bool Bool;
#else
typedef int Bool;
#endif

#define TLB_SIZE 16
#define PAGE_TABLE_SIZE 256
#define FRAME_SIZE 256
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

int memused = 0;
int getRealMemory() {
  return memused++;
}
int fifo = 0;

int main(void)
{
  FILE *fAddr = fopen("addresses.txt", "r");
  FILE *fStore = fopen("BACKING_STORE.bin", "rb");
  if (fAddr == NULL || fStore == NULL) {
    puts("Open file error");
    return 1;
  }

  FILE *gAddr = fopen("0416024_address.txt", "w");
  FILE *gValue = fopen("0416024_value.txt", "w");
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

  int tlbhit = 0, pagefault = 0, count = 0;
  int address;
  while (fscanf(fAddr, "%d", &address) == 1) {
    count++;
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
    int realN, realAddr;
    if (j < TLB_SIZE) { // tlb hit
      realN = tlb[j].frame;
      tlbhit++;
    }
    else { // tlb miss
      if (pageTable[pageN].valid) {
        realN = pageTable[pageN].frame;
      }
      else { // page fault
        realN = getRealMemory();
        realAddr = realN << 8 | offset;
        pageTable[pageN].valid = 1;
        pageTable[pageN].frame = realN;
        // read from backstore
        fseek(fStore, pageN<<8, SEEK_SET);
        fread(&mem[realN<<8], FRAME_SIZE, 1, fStore);
        pagefault++;
      }
      // put into tlb
      tlb[fifo].valid = 1;
      tlb[fifo].page = pageN;
      tlb[fifo].frame = realN;
      fifo++;
      if (fifo >= TLB_SIZE) fifo = 0;
    }
    realAddr = realN << 8 | offset;
    fprintf(gAddr, "%d\n", realAddr);
    fprintf(gValue, "%d\n", mem[realAddr]);
  }

  printf("Page Fault = %d\n", pagefault);
  printf("Page Fault Rate = %f\n", (float)pagefault / count);
  printf("TLB Hit = %d\n", tlbhit);
  printf("TLB Hit Rate = %f\n", (float)tlbhit / count);
  fclose(fAddr);
  fclose(fStore);
  fclose(gAddr);
  fclose(gValue);
  return 0;
}
