#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <stdarg.h>

// #include <pthread.h>
// pthread_create
// pthread_join

// pthread_mutex_t someMutex;
// pthread_mutex_init(&someMutex);
// pthread_mutex_lock(&someMutex);
// pthread_mutex_unlock(&someMutex);
// pthread_mutex_destroy(&someMutex);

int main(int argc, char *argv[])
{
  // initialize
  pthread_attr_t attr;
  pthread_attr_init(&attr);



  pthread_attr_destroy(&attr);
  return 0;
}
