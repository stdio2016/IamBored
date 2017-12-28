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
}

/*
return code
1 succeeds
0 no release
-1 fails since release exceeds allocation
*/
int releaseResources(int customerNum, int release[]) {
  
}

int main(int argc, char *argv[])
{
  // initialize
  pthread_attr_t attr;
  pthread_attr_init(&attr);



  pthread_attr_destroy(&attr);
  return 0;
}
