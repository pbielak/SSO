#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <assert.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>
#include <sys/shm.h>
#include <errno.h>

#define RAND_SLEEP usleep((rand() % (int) 5E5) + 1E5);

#define MAX_ACCOUNTS 5

#define SEM_KEY ((key_t) 6660666L)
#define SEM_PERMISSIONS 0600

#define SHM_KEY ((key_t) 3330333L)
#define SHM_PERMISSIONS 0600
#define SHM_SIZE (MAX_ACCOUNTS * sizeof(int))

#define WITHDRAW_MODE "WITHDRAW"
#define TRANSFER_MODE "TRANSFER"

void may_die(int ret_code, char* cause); 
void sem_wait(int sem_id, int semnum);
void sem_post(int sem_id, int semnum);
void sem_sync(int sem_id);

void run_withdraw(int account_sem_id, int* account_ptr, int account_idx, int cash_amount, int nb_steps);
void run_transfer(int account_sem_id, int* account_ptr, int account_from, int account_to, int cash_amount, int nb_steps);

int main(int argc, char* argv[]) {
  srand(time(NULL)); // Random seed

  int res, i;
  int account_idx, account_from, account_to, cash_amount, nb_steps;
  int account_sem_id, account_shm_id;
  int* account_ptr;
  struct shmid_ds buf;

  account_idx = -1;
  account_from = -1;
  account_to = -1;

  if (strcmp(argv[1], WITHDRAW_MODE) == 0) {
    if (argc < 5) {
      fprintf(stderr, "Usage: %s WITHDRAW account_idx cash_amount nb_steps\n", argv[0]);
      exit(-1);
    }
    
    account_idx = atoi(argv[2]);
    cash_amount = atoi(argv[3]);
    nb_steps = atoi(argv[4]);
  }
  else if (strcmp(argv[1], TRANSFER_MODE) == 0) {
    if (argc < 6) {
      fprintf(stderr, "Usage: %s TRANSFER account_from account_to cash_amount nb_steps\n", argv[0]);
      exit(-1);
    }

    account_from = atoi(argv[2]);
    account_to = atoi(argv[3]);
    cash_amount = atoi(argv[4]);
    nb_steps = atoi(argv[5]);
  }
  else {
    fprintf(stderr, "Unkown mode. Use: WITHDRAW or TRANSFER\n");
    exit(-1);
  }

  if (account_idx >= MAX_ACCOUNTS || account_from >= MAX_ACCOUNTS || account_to >= MAX_ACCOUNTS) {
    fprintf(stderr, "Account index out of bounds\n");
    exit(-1);
  }

  // Create semaphores
  account_sem_id = semget(SEM_KEY, MAX_ACCOUNTS, SEM_PERMISSIONS | IPC_CREAT | IPC_EXCL);
  if (account_sem_id < 0) { // Already exists
    account_sem_id = semget(SEM_KEY, MAX_ACCOUNTS, SEM_PERMISSIONS | IPC_CREAT);
  }
  else { // Newly created -> need to set values
    printf("Created account semaphores\n");
            
    for(i = 0; i < MAX_ACCOUNTS; i++) {
      res = semctl(account_sem_id, i, SETVAL, 1);
      may_die(res, "semctl set value");
    }
  
  }

  // Create shared memory 
  account_shm_id = shmget(SHM_KEY, SHM_SIZE, SHM_PERMISSIONS | IPC_CREAT | IPC_EXCL);
  if (account_shm_id < 0) { // Already exists
    account_shm_id = shmget(SHM_KEY, SHM_SIZE, SHM_PERMISSIONS | IPC_CREAT);
    account_ptr = (int*) shmat(account_shm_id, NULL, 0);

    if (account_ptr == (int*) -1) { perror("shmat"); exit(-1); }
  }
  else { // Newly created -> need to set values
    printf("Created account shared memory\n");

    account_ptr = (int*) shmat(account_shm_id, NULL, 0);
    for(i = 0; i < MAX_ACCOUNTS; i++) {
      account_ptr[i] = 0;
    }
  }

  // Run account operation
  if (strcmp(argv[1], WITHDRAW_MODE) == 0) {
    run_withdraw(account_sem_id, account_ptr, account_idx, cash_amount, nb_steps);
  }
  else {
    run_transfer(account_sem_id, account_ptr, account_from, account_to, cash_amount, nb_steps);
  }

  // Print balances of all accounts at end of program
  for(i = 0; i < MAX_ACCOUNTS; i++) {
    printf("[%d]", account_ptr[i]);
  }
  printf("\n");

  // Shared memory detach
  res = shmdt(account_ptr); 
  may_die(res, "shmdt");

  // Remove semaphores and shared memory
  res = shmctl(account_shm_id, IPC_STAT, &buf);
  may_die(res, "shmctl IPC_STAT");

  if (buf.shm_nattch == 0) { // Last attached one
    res = shmctl(account_shm_id, IPC_RMID, 0);
    may_die(res, "shmctl IPC_RMID");

    res = semctl(account_sem_id, 0, IPC_RMID, 0);
    may_die(res, "semctl RMID");

    printf("Removed shared memory and semaphores");
  } 

  return 0;
}

void may_die(int ret_code, char* cause) {
  if (ret_code < 0) {
    perror(cause);
    exit(-1);
  }
}

void sem_wait(int sem_id, int semnum) {
  int res;
  int value;
  struct sembuf op = { semnum, -1, 0 };

  res = semop(sem_id, &op, 1);
  if (res == EINTR) {
    value = semctl(sem_id, semnum, GETVAL, 0);
    if (value != 0) {
      res = semctl(sem_id, semnum, SETVAL, 0);
      may_die(res, "semctl set val wait");
    }
  }
  else {
    may_die(res, "semop wait");
  }
}

void sem_post(int sem_id, int semnum) {
  int res;
  int value;
  struct sembuf op = { semnum, 1, 0 };

  res = semop(sem_id, &op, 1);
  if (res == EINTR) {
    value = semctl(sem_id, semnum, GETVAL, 0);
    if (value != 1) {
      res = semctl(sem_id, semnum, SETVAL, 1);
      may_die(res, "semctl set val post");
    }
  }
  else {
    may_die(res, "semop post");
  }
}

void run_withdraw(int account_sem_id, int* account_ptr, int account_idx, int cash_amount, int nb_steps) {
  int i, prev_value;

  printf("[WITHDRAW] Account: %d\tCash amount: %d\tSteps: %d\n", account_idx, cash_amount, nb_steps);
  for(i = 0; i < nb_steps; i++) {
    sem_wait(account_sem_id, account_idx);

    prev_value = account_ptr[account_idx];
    RAND_SLEEP
    account_ptr[account_idx] += cash_amount;
    assert(prev_value + cash_amount == account_ptr[account_idx]);
    printf("[%d/%d][Account %d] %d --> %d\n", i + 1, nb_steps, account_idx, prev_value, account_ptr[account_idx]);

    sem_post(account_sem_id, account_idx);

    sleep(1); 
  } 
}


void run_transfer(int account_sem_id, int* account_ptr, int account_from, int account_to, int cash_amount, int nb_steps) {
  int i, prev_value_from, prev_value_to;
  int res, value;
  struct sembuf ops[2] = { { account_from, -1, 0 }, { account_to, -1, 0} };

  printf("[TRANSFER] Accounts: %d --> %d\tCash amount: %d\tSteps: %d\n", account_from, account_to, cash_amount, nb_steps);
  for(i = 0; i < nb_steps; i++) {
    // Sem_wait for both accounts
    ops[0].sem_op = -1;
    ops[1].sem_op = -1;
    res = semop(account_sem_id, ops, 2);
    if(res == EINTR) {
      value = semctl(account_sem_id, account_from, GETVAL, 0);
      if (value != 0) {
        res = semctl(account_sem_id, account_from, SETVAL, 0);
        may_die(res, "semctl set val wait");
        res = semctl(account_sem_id, account_to, SETVAL, 0);
        may_die(res, "semctl set val wait");
      }
    }
    else {
      may_die(res, "semop transfer wait");
    }

    prev_value_from = account_ptr[account_from];
    prev_value_to = account_ptr[account_to];
    RAND_SLEEP

    account_ptr[account_from] -= cash_amount;
    account_ptr[account_to] += cash_amount;

    assert(prev_value_from - cash_amount == account_ptr[account_from]);
    assert(prev_value_to + cash_amount  == account_ptr[account_to]);

    printf("[%d/%d]\n", i + 1, nb_steps);
    printf("Account from %d: %d --> %d\n", account_from, prev_value_from, account_ptr[account_from]);
    printf("Account to %d: %d --> %d\n", account_to, prev_value_to, account_ptr[account_to]);
    
    // Sem_post for both accounts
    ops[0].sem_op = 1;
    ops[1].sem_op = 1;
    res = semop(account_sem_id, ops, 2);
    if (res == EINTR) {
      value = semctl(account_sem_id, account_from, GETVAL, 0);
      if (value != 1) {
        res = semctl(account_sem_id, account_from, SETVAL, 1);
        may_die(res, "semctl set val post");
        res = semctl(account_sem_id, account_to, SETVAL, 1);
        may_die(res, "semctl set val post");
      }
    }
    else {
      may_die(res, "semop transfer post");
    }
    sleep(1);
  }
}
