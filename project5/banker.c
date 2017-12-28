#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>

#define NUMBER_OF_CUSTOMERS 5
#define NUMBER_OF_RESOURCES 3

pthread_mutex_t bankMutex;
// access to rand may have data race
pthread_mutex_t randMutex;

pthread_t customers[NUMBER_OF_CUSTOMERS];

int available[NUMBER_OF_RESOURCES], all[NUMBER_OF_RESOURCES];

int maximum[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

int allocation[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

int need[NUMBER_OF_CUSTOMERS][NUMBER_OF_RESOURCES];

int finishCount = 0;

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

void showRequest(int customerNum, int request[], int code) {
  int i;
  printf("Request");
  for (i = 0; i < NUMBER_OF_RESOURCES; i++)
    printf(" %d", request[i]);
  puts("");
  printf("Request Code %d: ", code);
  switch (code) {
    case 3:
      printf("customer %d's request succeeds & all customer finish\n", customerNum); break;
    case 2:
      printf("customer %d's request succeeds & finishes\n", customerNum); break;
    case 1:
      printf("customer %d's request succeeds\n", customerNum); break;
    case 0:
      printf("customer %d doesn't request\n", customerNum); break;
    case -1:
      printf("customer %d's request fails since request exceeds need\n", customerNum); break;
    case -2:
      printf("customer %d's request fails since request exceeds available\n", customerNum); break;
    case -3:
      printf("customer %d's request fails since the state is unsafe\n", customerNum); break;
  }
}

void showRelease(int customerNum, int release[], int code) {
  int i;
  printf("Release");
  for (i = 0; i < NUMBER_OF_RESOURCES; i++)
    printf(" %d", release[i]);
  puts("");
  printf("Release Code %d: ", code);
  switch (code) {
    case 1:
      printf("customer %d's release succeeds\n", customerNum); break;
    case 0:
      printf("customer %d doesn't release any resource\n", customerNum); break;
    case -1:
      printf("customer %d's release fails since release exceeds allocation\n", customerNum); break;
  }
}

void printState() {
  puts("current state\n");
  puts("available");
  printf("resources  %3d %3d %3d\n\n", available[0], available[1], available[2]);
  int i, j;
  puts("             maximum     allocation  need");
  for (i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
    printf("customer %d", i);
    for (j = 0; j < NUMBER_OF_RESOURCES; j++)
      printf(" %3d", maximum[i][j]);
    for (j = 0; j < NUMBER_OF_RESOURCES; j++)
      printf(" %3d", allocation[i][j]);
    for (j = 0; j < NUMBER_OF_RESOURCES; j++)
      printf(" %3d", need[i][j]);
    puts("");
  }
  puts("");
}

void *customerThread(void *param) {
  int *idPtr = param;
  int id = *idPtr;
  int rndarr[NUMBER_OF_RESOURCES];
  while (11) {
    int i, good;
    pthread_mutex_lock(&randMutex);
    for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
      rndarr[i] = rand() % (all[i]+1);
    }
    pthread_mutex_unlock(&randMutex);
    pthread_mutex_lock(&bankMutex);
    good = requestResources(id, rndarr);
    showRequest(id, rndarr, good);
    printState();
    pthread_mutex_unlock(&bankMutex);
    // don't exit before releasing lock
    if (good == 2 || good == 3) break;

    pthread_mutex_lock(&randMutex);
    for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
      rndarr[i] = rand() % (all[i]+1);
    }
    pthread_mutex_unlock(&randMutex);
    pthread_mutex_lock(&bankMutex);
    good = releaseResources(id, rndarr);
    showRelease(id, rndarr, good);
    printState();
    pthread_mutex_unlock(&bankMutex);
  }
  free(param);
  return NULL;
}

int main(int argc, char *argv[])
{
  if (argc < NUMBER_OF_RESOURCES + 1) {
    printf("usage: %s <resource 1> <resource 2> <resource 3>\n", argv[0]);
    return 1;
  }
  // initialize
  pthread_attr_t attr;
  pthread_attr_init(&attr);
  pthread_mutex_init(&bankMutex, NULL);
  pthread_mutex_init(&randMutex, NULL);
  // randomize
  int i, j;
  srand(time(NULL));
  for (i = 0; i < NUMBER_OF_RESOURCES; i++) {
    all[i] = available[i] = strtol(argv[i+1], NULL, 10);
  }
  for (i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
    int zero = NUMBER_OF_RESOURCES;
    do {
      zero = 0;
      for (j = 0; j < NUMBER_OF_RESOURCES; j++) {
        maximum[i][j] = rand() % (available[j]+1);
        if (maximum[i][j] == 0) zero++;
        allocation[i][j] = 0;
        need[i][j] = maximum[i][j];
      }
    } while (zero == NUMBER_OF_RESOURCES) ;
  }
  // create thread
  for (i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
    int *param = malloc(sizeof(int));
    *param = i;
    pthread_create(&customers[i], &attr, customerThread, param);
  }

  // wait
  for (i = 0; i < NUMBER_OF_CUSTOMERS; i++) {
    void *unused;
    pthread_join(customers[i], &unused);
  }
  // finalize
  pthread_mutex_destroy(&randMutex);
  pthread_mutex_destroy(&bankMutex);
  pthread_attr_destroy(&attr);
  return 0;
}
