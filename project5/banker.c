#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <stdarg.h>

#define NUMBER_OF_CUSTOMERS 5
#define NUMBER_OF_RESOURCES 3

int available[NUMBER_OF_RESOURCES];

int maximum[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

int allocation[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

int need[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

int finishCount = 0;

// #include <pthread.h>
// pthread_create
// pthread_join

// pthread_mutex_t someMutex;
// pthread_mutex_init(&someMutex);
// pthread_mutex_lock(&someMutex);
// pthread_mutex_unlock(&someMutex);
// pthread_mutex_destroy(&someMutex);

int isSafeState() {
  int finish[NUMBER_OF_CUSTOMERS] = {0};
  int work[NUMBER_OF_RESOURCES];
  int i, finCount = 0, flag;
  for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
    work[i] = available[i];
  }
  do {
    flag = 0;
    // find a customer i
    for (i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
      int j;
      if (finish[i]) continue;
      for (j = 0; j < NUMBER_OF_RESOURCES; j++) {
        if (work[j] < need[i][j])
          break;
      }
      if (j == NUMBER_OF_RESOURCES) { // can allocate
        for (j = 0; j < NUMBER_OF_RESOURCES; j++) {
          work[j] += allocation[i][j];
        }
        finish[i] = 1;
        flag = 1;
        finCount++;
      }
    }
  } while (flag) ;
  if (finCount == NUMBER_OF_CUSTOMERS) {
    return 1;
  }
  return 0;
}

/*
return code
3 succeeds & all customer finish
2 succeeds & current customer finishes
1 succeeds
0 no request
-1 fails since request exceeds need
-2 fails since request exceeds available
-3 fails since the state is unsafe
*/
int requestResources(int customerNum, int request[]) {
  int i;
  for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
    if (request[i] != 0)
      break;
  }
  if (i == NUMBER_OF_RESOURCES) return 0;

  for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
    if (request[i] > need[customerNum][i])
      return -1;
  }
  for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
    if (request[i] > available[i])
      return -2;
  }
  // pretend to allocate resources
  for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
    available[i] -= request[i];
    allocation[customerNum][i] += request[i];
    need[customerNum][i] -= request[i];
  }
  if (isSafeState()) {
    for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
      if (need[customerNum][i] > 0) break;
    }
    if (i == NUMBER_OF_RESOURCES) { // customer finishes
      finishCount++;
      // release resource
      for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
        available[i] += allocation[customerNum][i];
        allocation[customerNum][i] = 0;
        need[customerNum][i] = 0;
      }
      if (finishCount == NUMBER_OF_CUSTOMERS) {
        return 3;
      }
      return 2;
    }
    return 1;
  }
  else {
    // revert
    for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
      available[i] += request[i];
      allocation[customerNum][i] -= request[i];
      need[customerNum][i] += request[i];
    }
    return -3;
  }
}

/*
return code
1 succeeds
0 no release
-1 fails since release exceeds allocation
*/
int releaseResources(int customerNum, int release[]) {
  int i;
  for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
    if (release[i] != 0)
      break;
  }
  if (i == NUMBER_OF_RESOURCES) return 0;

  for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
    if (release[i] > allocation[customerNum][i])
      return -1;
  }
  for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
    available[i] += release[i];
    allocation[customerNum][i] -= release[i];
    need[customerNum][i] += release[i];
  }
  return 1;
}

void printState() {
  puts("current state\n");
  puts("available");
  printf("resources    %3d %3d %3d\n\n", available[0], available[1], available[2]);
  int i, j;
  puts("            maximum     allocation  need");
  for (i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
    printf("customer %d ", i);
    for (j = 0; j < NUMBER_OF_RESOURCES; j++)
      printf(" %3d", maximum[i][j]);
    for (j = 0; j < NUMBER_OF_RESOURCES; j++)
      printf(" %3d", allocation[i][j]);
    for (j = 0; j < NUMBER_OF_RESOURCES; j++)
      printf(" %3d", need[i][j]);
    puts("");
  }
}

int main(int argc, char *argv[])
{
  // initialize
  pthread_attr_t attr;
  pthread_attr_init(&attr);



  pthread_attr_destroy(&attr);
  return 0;
}
