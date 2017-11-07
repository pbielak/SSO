#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>
#include <assert.h>
#include <ctype.h>

#define NB_ACCOUNTS 2

struct withdraw_args {
  int thread_id;
  int account_number;
  int cash_amount;
  int steps;
};

int sem[NB_ACCOUNTS] = {1, 1};
int bank_accounts_balance[NB_ACCOUNTS] = {0, 0};
pthread_mutex_t mtx_lock;
pthread_cond_t operation_cond, not_enough[NB_ACCOUNTS];

void* run_withdraw(void* args);

int main(int argc, char* argv[]) {
  int i;
  char c;

  pthread_t* threads;
  int nb_threads;
  struct withdraw_args* w_args;

  if (argc < 5) {
    fprintf(stderr, "%s -N nb_threads [-W account_number~cash_amount~nb_steps]\n", argv[0]);
    exit(-1);
  }

  // Init mutex and cond_var
  pthread_mutex_init(&mtx_lock, NULL);
  pthread_cond_init(&operation_cond, NULL);
  pthread_cond_init(&not_enough[0], NULL);
  pthread_cond_init(&not_enough[1], NULL);

  opterr = 0;
  i = 0;

  while((c = getopt(argc, argv, "N:W:")) != -1) {
   switch(c) {
    case 'N':
      nb_threads = atoi(optarg);
      threads = (pthread_t*) malloc(sizeof(pthread_t*) * nb_threads);
      printf("Got %d threads\n", nb_threads);
      break;

    case 'W':
      if (threads == NULL) {
        fprintf(stderr, "First parameter must be -N nb_threads");
        exit(-1);
      }

      w_args = (struct withdraw_args*) malloc(sizeof(struct withdraw_args));
      w_args->thread_id = i;
      sscanf(optarg, "%d~%d~%d", &(w_args->account_number), &(w_args->cash_amount), &(w_args->steps));
      printf("[%d/%d] Starting withdraw on account %d: Cash: %d Steps: %d\n", i + 1, nb_threads, w_args->account_number, w_args->cash_amount, w_args->steps);
      pthread_create(&threads[i], NULL, run_withdraw, (void*) w_args);
      i++;

      break;
   } 
  }

  for(i = 0; i < nb_threads; i++) {
    pthread_join(threads[i], NULL);
  }

  // Print end balance
  printf("Final account balance: [%d][%d]\n", bank_accounts_balance[0], bank_accounts_balance[1]);

  // Clean up
  pthread_mutex_destroy(&mtx_lock);
  pthread_cond_destroy(&operation_cond);
  pthread_cond_destroy(&not_enough[0]);
  pthread_cond_destroy(&not_enough[1]);

  return 0;
}


void* run_withdraw(void* args) {
  struct withdraw_args* w_args = (struct withdraw_args*) args;
  int i, prev_balance;

  for(i = 0; i < w_args->steps; i++) {
    // Semafor wait
    pthread_mutex_lock(&mtx_lock);
      while(bank_accounts_balance[w_args->account_number] + w_args->cash_amount < 0) {
        pthread_cond_wait(&not_enough[w_args->account_number], &mtx_lock);
      }
      while(sem[w_args->account_number] <= 0) {
        pthread_cond_wait(&operation_cond, &mtx_lock);
      }
      sem[w_args->account_number] --;
    pthread_mutex_unlock(&mtx_lock);
    
    // Critcal section
    prev_balance = bank_accounts_balance[w_args->account_number];
    sleep(1);
    bank_accounts_balance[w_args->account_number] += w_args->cash_amount;
    printf("[Thread %d][%d/%d] WITHDRAW account %d: %d --> %d\n", w_args->thread_id, i + 1, w_args->steps, w_args->account_number, prev_balance, bank_accounts_balance[w_args->account_number]);

    assert(prev_balance + w_args->cash_amount == bank_accounts_balance[w_args->account_number]);
    
    // Semafor post
    pthread_mutex_lock(&mtx_lock);
      sem[w_args->account_number] ++;
      pthread_cond_signal(&operation_cond);
      pthread_cond_signal(&not_enough[w_args->account_number]);
    pthread_mutex_unlock(&mtx_lock);
    sleep(1);
  }

  free(w_args);
  return NULL;
}
