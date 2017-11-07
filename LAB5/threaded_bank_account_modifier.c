#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <pthread.h>

struct withdraw_args {
  int thread_id;
  int account_number;
  int cash_amount;
  int steps;
};

void* run_withdraw(void* args);

int main(int argc, char* argv[]) {
  int nb_threads_0, nb_threads_1, nb_threads_transfer;
  int nb_steps_0, nb_steps_1, nb_steps_transfer;
  int cash_amount_0, cash_amount_1, cash_amount_transfer;

  int i;

  pthread_t* account_0_threads;
  pthread_t* account_1_threads;
  struct withdraw_args* w_args;

  if (argc < 10) {
   fprintf(stderr, "%s nb_threads_0 cash_amount_acc_0 steps_0 nb_threads_1 cash_amount_acc_1 steps_1 nb_threads_transfer cash_amount_transfer steps_transfer\n", argv[0]);
     exit(-1);
  }

  nb_threads_0 = atoi(argv[1]);
  cash_amount_0 = atoi(argv[2]);
  nb_steps_0 = atoi(argv[3]);

  nb_threads_1 = atoi(argv[4]);
  cash_amount_1 = atoi(argv[5]);
  nb_steps_1 = atoi(argv[6]);

  nb_threads_transfer = atoi(argv[7]);
  cash_amount_transfer = atoi(argv[8]);
  nb_steps_transfer = atoi(argv[9]);

  // Run account 0 threads
  account_0_threads = (pthread_t*) malloc(sizeof(pthread_t*) * nb_threads_0);
  for(i = 0; i < nb_threads_0; i++) {
    w_args = (struct withdraw_args*) malloc(sizeof(struct withdraw_args));
    w_args->thread_id = i;
    w_args->account_number = 0;
    w_args->cash_amount = cash_amount_0;
    w_args->steps = nb_steps_0;

    printf("[%d/%d] Starting withdraw on account 0\n", i + 1, nb_threads_0);

    pthread_create(&account_0_threads[i], NULL, run_withdraw, (void*) w_args);
  }

  // Run account 1 threads 
  account_1_threads = (pthread_t*) malloc(sizeof(pthread_t*) * nb_threads_1);
  for(i = 0; i < nb_threads_1; i++) {
    w_args = (struct withdraw_args*) malloc(sizeof(struct withdraw_args));
    w_args->thread_id = i;
    w_args->account_number = 1;
    w_args->cash_amount = cash_amount_1;
    w_args->steps = nb_steps_1;

    printf("[%d/%d] Starting withdraw on account 1\n", i + 1, nb_threads_1);

    pthread_create(&account_1_threads[i], NULL, run_withdraw, (void*) w_args);
  }


  // Join account 0 threads
  for(i = 0; i < nb_threads_0; i++) {
    pthread_join(account_0_threads[i], NULL);
  }
  // Join account 1 threads
  for(i = 0; i < nb_threads_1; i++) {
    pthread_join(account_1_threads[i], NULL);
  }

  free(account_0_threads);
  free(account_1_threads);

  return 0;
}


void* run_withdraw(void* args) {
  struct withdraw_args* w_args = (struct withdraw_args*) args;
  int i;
  
  for(i = 0; i < w_args->steps; i++) {
    printf("[Thread %d][%d/%d] Withdraw account %d cash %d\n", w_args->thread_id, i + 1, w_args->steps, w_args->account_number, w_args->cash_amount);
    sleep(1);
  }


  free(w_args);
  return NULL;
}
