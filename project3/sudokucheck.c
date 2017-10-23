#include <stdio.h>
#include <pthread.h>
char sudokuboard[9][11];
char fail_check[10] = "false";

void *check_row(void *param) {
  int has, r, c;
  for (r = 0; r < 9; r++) {
    for (c = 0, has = 0; c < 9; c++) {
      int n = sudokuboard[r][c];
      if ((has>>n & 1) == 1) {
        return fail_check;
      }
      has |= 1<<n;
    }
  }
  return NULL;
}

void *check_column(void *param) {
  int has, r, c;
  for (c = 0; c < 9; c++) {
    for (r = 0, has = 0; r < 9; r++) {
      int n = sudokuboard[r][c];
      if ((has>>n & 1) == 1) {
        return fail_check;
      }
      has |= 1<<n;
    }
  }
  return NULL;
}

void *check_block(void *param) {
  int decode = *(int *) param;
  int has = 0, r, c, i, j;
  r = decode & 3;
  c = decode >> 2;
  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      int n = sudokuboard[r*3+i][c*3+j];
      if ((has>>n & 1) == 1) {
        return fail_check;
      }
      has |= 1<<n;
    }
  }
  return NULL;
}

int param[9];
int create_thread_to_solve_this() {
  pthread_t tid[11]; // 11 threads
  pthread_attr_t attr;
  pthread_attr_init(&attr);

  pthread_create(&tid[0], &attr, check_row, NULL);
  pthread_create(&tid[1], &attr, check_column, NULL);
  int i, j;
  for (i = 0; i < 3; i++) {
    for (j = 0; j < 3; j++) {
      int idx = i*3 + j;
      param[idx] = i << 2 | j;
      pthread_create(&tid[2 + idx], &attr, check_block, &param[idx]);
    }
  }

  void *result;
  int yes = 1;
  for (i = 0; i < 11; i++) {
    pthread_join(tid[i], &result);
    if (result == fail_check) {
      yes = 0;
    }
  }
  pthread_attr_destroy(&attr);
  return yes;
}

int main(int argc, char *argv[])
{
  if (argc != 2) {
    fprintf(stderr, "usage: sudokucheck <file>\n");
    return 1;
  }
  FILE *f = fopen(argv[1], "r");
  if (NULL == f) {
    fprintf(stderr, "unable to read \"%s\"\n", argv[1]);
    return 1;
  }
  // read sudoku
  int i, j, ch;
  char buf[11];
  int yes = 1;
  for (i = 0; i < 9 && yes; i++) {
    fgets(buf, 10, f);
    for (j = 0; j < 9; j++) {
      sudokuboard[i][j] = buf[j] - '0';
      if (buf[j] > '9' || buf[j] < '1') {
        yes = 0;
        break;
      }
    }
    do {
      ch = fgetc(f);
    } while (ch != EOF && ch != '\n');
    if (ch == EOF && i < 8) {
      yes = 0;
    }
  }
  fclose(f);
  // sudoku format error
  if (!yes) {
    puts("false");
    return 0;
  }
  yes = create_thread_to_solve_this();
  if (yes) {
    puts("true");
  }
  else {
    puts("false");
  }
  return 0;
}
