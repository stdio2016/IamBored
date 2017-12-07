#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <time.h>
#include <stdarg.h>
#include <semaphore.h>

struct Trainer {
  char *name;
  int trainTime; // in seconds
  int battleTime; // in seconds
  int remainBattles; // battles needed to win a badge
  sem_t readySem; // inform trainer that he can battle with The Gym Leader
  sem_t battleSem; // to wait for battle to finish
};

struct Gym {
  int seatCount;
  int firstSeat;
  int occupied;
  struct Trainer **seats; // seats as a circular queue
  pthread_mutex_t seatMutex; // to ensure mutual exclusion when the queue is updated
  sem_t wakeSem; // wake up the gym leader
  int sleeping; // is Gym Leader sleeping?
};

struct Gym *NctuGym;

// used to stop the Gym Leader
struct Trainer CloseGym;

// some useful functions
void waitSecs(int seconds) {
  struct timespec req;
  req.tv_sec = seconds;
  req.tv_nsec = 0;
  nanosleep(&req, NULL);
}

pthread_mutex_t printMutex;
time_t startTime;
int showMsg(char *fmt, ...) {
  va_list args;
  int ret;
  unsigned int tm;

  pthread_mutex_lock(&printMutex);
  tm = time(NULL) - startTime;
  printf("%u: ", tm);
  va_start(args, fmt);
  ret = vprintf(fmt, args);
  va_end(args);
  pthread_mutex_unlock(&printMutex);
  return ret;
}

// actions of a trainer
void Train(struct Trainer *me) {
  waitSecs(me->trainTime);
}

void WakeUpGymLeader(struct Gym *gym) {
  sem_post(&gym->wakeSem);
}

// return n-th in a queue, or 0 if no seats
int FindSeatAndSit(struct Trainer *me, struct Gym *gym) {
  int seatCount = gym->seatCount;
  pthread_mutex_lock(&gym->seatMutex);
  int occupied = gym->occupied;
  int i = gym->firstSeat + occupied;
  if (occupied < seatCount) {
    if (i >= seatCount) i -= seatCount;
    gym->seats[i] = me;
    gym->occupied = occupied + 1;
    if (gym->sleeping) { // wake up Gym Leader if he is sleeping
      gym->sleeping = 0;
      WakeUpGymLeader(gym);
    }
  }
  pthread_mutex_unlock(&gym->seatMutex);
  if (occupied < seatCount) return occupied+1;
  return 0;
}

void WaitGymLeader(struct Trainer *me) {
  sem_wait(&me->readySem); // wait until gym leader is free
}

void BattleGymLeader(struct Trainer *me, struct Gym *gym) {
  waitSecs(me->battleTime); // battle for some time
  sem_post(&me->battleSem); // tell the gym leader to stop the battle
}

void *RunTrainer(void *who) {
  struct Trainer *me = who;
  while (me->remainBattles > 0) {
    Train(me);
    int success = FindSeatAndSit(me, NctuGym);
    if (success) {
      if (success == 1) {
        // first come, no need to wait
      }
      else {
        showMsg("The Trainer %s enters the Gym, waiting for The Gym Leader.\n", me->name);
      }
      WaitGymLeader(me);
      BattleGymLeader(me, NctuGym);
      me->remainBattles--;
    }
    else {
      showMsg("The Trainer %s cannot fine a seat in Gym.\n", me->name);
    }
  }
  showMsg("The Trainer %s wins a badge.\n", me->name);
  return me;
}

// actions of a gym leader
void GymLeaderSleep(struct Gym *leader) {
  showMsg("The Gym Leader takes a nap.\n");
  sem_wait(&leader->wakeSem);
}

struct Trainer *LookForChallenger(struct Gym *gym) {
  pthread_mutex_lock(&gym->seatMutex);
  int first = gym->firstSeat;
  struct Trainer *p = gym->seats[first];
  if (p == NULL) {
    gym->sleeping = 1; // if no one is in the gym, the leader will take a nap
  }
  pthread_mutex_unlock(&gym->seatMutex);
  return p;
}

void ChallengerLeave(struct Gym *gym) {
  pthread_mutex_lock(&gym->seatMutex);
  int first = gym->firstSeat;
  gym->seats[first] = NULL;
  first++;
  if (first >= gym->seatCount) {
    first = 0;
  }
  gym->firstSeat = first;
  gym->occupied--;
  pthread_mutex_unlock(&gym->seatMutex);
}

void BattleChallenger(struct Gym *leader, struct Trainer *challenger) {
  sem_post(&challenger->readySem); // tell the trainer the gym leader is ready to battle
  sem_wait(&challenger->battleSem); // battle until the trainer says stop
  ChallengerLeave(leader); // remove the challenger from the seats (queue)
}

void *RunGym(void *where) {
  struct Gym *me = where;
  while (1) {
    GymLeaderSleep(me);
    struct Trainer *p = LookForChallenger(me);
    if (p == &CloseGym) return NULL;
    showMsg("The Gym Leader is waken up by a Trainer.\n");
    while (p != NULL) {
      showMsg("The Gym Leader is battling with Trainer %s now.\n", p->name);
      BattleChallenger(me, p);
      showMsg("The battle with Trainer %s is over.\n", p->name);
      p = LookForChallenger(me);
      if (p == &CloseGym) return NULL;
    }
  }
}

// create and destroy Trainer
struct Trainer *createTrainer(char *name, int trainTime, int battleTime, int battleCount) {
  struct Trainer *one = malloc(sizeof(struct Trainer));
  one->name = name;
  one->trainTime = trainTime;
  one->battleTime = battleTime;
  one->remainBattles = battleCount;
  sem_init(&one->readySem, 0, 0);
  sem_init(&one->battleSem, 0, 0);
  return one;
}

void destroyTrainer(struct Trainer *who) {
  sem_destroy(&who->readySem);
  sem_destroy(&who->battleSem);
  free(who);
}

struct Gym *createGym(int seatCount) {
  struct Gym *gym = malloc(sizeof(struct Gym));
  gym->seatCount = seatCount;
  gym->firstSeat = 0;
  gym->occupied = 0;
  gym->seats = malloc(sizeof(struct Trainer *) * seatCount);
  pthread_mutex_init(&gym->seatMutex, NULL);
  sem_init(&gym->wakeSem, 0, 0);
  gym->sleeping = 1;
  return gym;
}

void destroyGym(struct Gym *gym) {
  free(gym->seats);
  pthread_mutex_destroy(&gym->seatMutex);
  sem_destroy(&gym->wakeSem);
  free(gym);
}

int main(int argc, char *argv[])
{
  // initialize
  pthread_attr_t attr;
  pthread_attr_init(&attr);

  pthread_mutex_init(&printMutex, NULL);
  time(&startTime);

  pthread_t pTrainers[3];
  pthread_t pGym;

  // create trainer and gym
  struct Trainer *trainer[3];
  trainer[0] = createTrainer("Ash", 2, 2, 3);
  trainer[1] = createTrainer("Misty", 3, 3, 2);
  trainer[2] = createTrainer("Brock", 5, 5, 1);
  NctuGym = createGym(2);

  // start thread
  int i;
  for (i = 0; i < 3; i++)
    pthread_create(&pTrainers[i], &attr, RunTrainer, trainer[i]);
  pthread_create(&pGym, &attr, RunGym, NctuGym);

  // stop thread
  for (i = 0; i < 3; i++)
    pthread_join(pTrainers[i], NULL);
  FindSeatAndSit(&CloseGym, NctuGym);
  WakeUpGymLeader(NctuGym);
  pthread_join(pGym, NULL);

  // destroy trainer and gym
  for (i = 0; i < 3; i++)
    destroyTrainer(trainer[i]);
  destroyGym(NctuGym);

  pthread_mutex_destroy(&printMutex);
  pthread_attr_destroy(&attr);
  return 0;
}
