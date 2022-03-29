
#include <nuttx/config.h>
#include <stdio.h>
#include <stdlib.h> // exit

void* thread_leader(void* arg);
void* thread_jimon_san(void* arg);
void* thread_ryu_san(void* arg);

#define THREAD_NUM   (3)  // スレッドの数

enum dachoClubMember {
  Leader,
  Jimon_san,
  Ryu_san,
  MemberMax
};

typedef struct{
    char name[16] ;
    void* (*func)(void*) ;
} DachoClub;

DachoClub dachoClubTable[MemberMax] = {
  { "リーダー",   thread_leader },
  { "ジモンさん", thread_jimon_san },
  { "竜さん",     thread_ryu_san }
};

pthread_mutex_t raise_hand_furi_mutex[MemberMax-1];
pthread_cond_t raise_hand_furi_cond[MemberMax-1];
pthread_barrier_t raise_hand_furi_completion_barrier;
pthread_barrier_t raise_hand_barrier;
pthread_barrier_t douzo_wait_barrier;
pthread_barrier_t tukkomi_wait_barrier;


void* thread_leader(void* arg) {
  enum dachoClubMember member_index = (uint32_t)arg;
  int ret;

  printf("[%s]:熱湯風呂入ったらおいしいよね。おれやるよ。\n", dachoClubTable[member_index].name);
  sleep(2);
  pthread_cond_signal(&raise_hand_furi_cond[Jimon_san]);

  // 条件待ち
  pthread_mutex_lock(&raise_hand_furi_mutex[member_index]);
  if (pthread_cond_wait(&raise_hand_furi_cond[member_index], &raise_hand_furi_mutex[member_index]) != 0) {
      printf("Fatal error on pthread[%d]_cond_wait.\n", member_index);
      exit(1);
  }
  pthread_mutex_unlock(&raise_hand_furi_mutex[member_index]);

  printf("[%s]:おれやるよ。\n", dachoClubTable[member_index].name);
  sleep(2);

  ret = pthread_barrier_wait(&raise_hand_furi_completion_barrier);
  if (!(ret == 0 || ret == PTHREAD_BARRIER_SERIAL_THREAD)) {
    printf("Error; thread index[%d] pthread_barrier_wait ret = %d.\n", member_index, ret);
    exit(1);
  }

  ret = pthread_barrier_wait(&raise_hand_barrier);
  if (!(ret == 0 || ret == PTHREAD_BARRIER_SERIAL_THREAD)) {
    printf("Error; thread index[%d] pthread_barrier_wait(raise_hand_barrier) ret = %d.\n", member_index, ret);
    exit(1);
  }

  printf("[%s]:どうぞ、どうぞ\n", dachoClubTable[member_index].name);

  ret = pthread_barrier_wait(&douzo_wait_barrier);
  if (!(ret == 0 || ret == PTHREAD_BARRIER_SERIAL_THREAD)) {
    printf("Error; thread index[%d] pthread_barrier_wait(douzo_wait_barrier) ret = %d.\n", member_index, ret);
    exit(1);
  }

  ret = pthread_barrier_wait(&tukkomi_wait_barrier);
  if (!(ret == 0 || ret == PTHREAD_BARRIER_SERIAL_THREAD)) {
    printf("Error; thread index[%d] pthread_barrier_wait(tukkomi_wait_barrier) ret = %d.\n", member_index, ret);
    exit(1);
  }

  printf("[%s]:やー!!!\n", dachoClubTable[member_index].name);

  return (void*)member_index;
}

void* thread_jimon_san(void* arg) {
  enum dachoClubMember member_index = (uint32_t)arg;
  int ret;

  // 条件待ち
  pthread_mutex_lock(&raise_hand_furi_mutex[member_index]);
  if (pthread_cond_wait(&raise_hand_furi_cond[member_index], &raise_hand_furi_mutex[member_index]) != 0) {
      printf("Fatal error on pthread[%d]_cond_wait.\n", member_index);
      exit(1);
  }
  pthread_mutex_unlock(&raise_hand_furi_mutex[member_index]);

  printf("[%s]:いやいや、俺がやるよ。\n", dachoClubTable[member_index].name);
  sleep(2);
  pthread_cond_signal(&raise_hand_furi_cond[Leader]);

  ret = pthread_barrier_wait(&raise_hand_furi_completion_barrier);
  if (!(ret == 0 || ret == PTHREAD_BARRIER_SERIAL_THREAD)) {
    printf("Error; thread index[%d] pthread_barrier_wait ret = %d.\n", member_index, ret);
    exit(1);
  }

  ret = pthread_barrier_wait(&raise_hand_barrier);
  if (!(ret == 0 || ret == PTHREAD_BARRIER_SERIAL_THREAD)) {
    printf("Error; thread index[%d] pthread_barrier_wait(raise_hand_barrier) ret = %d.\n", member_index, ret);
    exit(1);
  }

  printf("[%s]:どうぞ、どうぞ\n", dachoClubTable[member_index].name);

  ret = pthread_barrier_wait(&douzo_wait_barrier);
  if (!(ret == 0 || ret == PTHREAD_BARRIER_SERIAL_THREAD)) {
    printf("Error; thread index[%d] pthread_barrier_wait(douzo_wait_barrier) ret = %d.\n", member_index, ret);
    exit(1);
  }

  ret = pthread_barrier_wait(&tukkomi_wait_barrier);
  if (!(ret == 0 || ret == PTHREAD_BARRIER_SERIAL_THREAD)) {
    printf("Error; thread index[%d] pthread_barrier_wait(tukkomi_wait_barrier) ret = %d.\n", member_index, ret);
    exit(1);
  }

  printf("[%s]:やー!!!\n", dachoClubTable[member_index].name);

  return (void*)member_index;
}

void* thread_ryu_san(void* arg) {
  enum dachoClubMember member_index = (uint32_t)arg;
  int ret;

  ret = pthread_barrier_wait(&raise_hand_furi_completion_barrier);
  if (!(ret == 0 || ret == PTHREAD_BARRIER_SERIAL_THREAD)) {
    printf("Error; thread index[%d] pthread_barrier_wait ret = %d.\n", member_index, ret);
    exit(1);
  }

  printf("[%s]:じゃあ、俺がやるよ。\n", dachoClubTable[member_index].name);
  sleep(2);
  printf("\n");

  ret = pthread_barrier_wait(&raise_hand_barrier);
  if (!(ret == 0 || ret == PTHREAD_BARRIER_SERIAL_THREAD)) {
    printf("Error; thread index[%d] pthread_barrier_wait(raise_hand_barrier) ret = %d.\n", member_index, ret);
    exit(1);
  }

  ret = pthread_barrier_wait(&douzo_wait_barrier);
  if (!(ret == 0 || ret == PTHREAD_BARRIER_SERIAL_THREAD)) {
    printf("Error; thread index[%d] pthread_barrier_wait(douzo_wait_barrier) ret = %d.\n", member_index, ret);
    exit(1);
  }

  sleep(2);
  printf("\n");
  printf("[%s]:おい!!!\n", dachoClubTable[member_index].name);
  printf("\n");
  sleep(2);

  ret = pthread_barrier_wait(&tukkomi_wait_barrier);
  if (!(ret == 0 || ret == PTHREAD_BARRIER_SERIAL_THREAD)) {
    printf("Error; thread index[%d] pthread_barrier_wait(tukkomi_wait_barrier) ret = %d.\n", member_index, ret);
    exit(1);
  }

  printf("[%s]:やー!!!\n", dachoClubTable[member_index].name);

  return (void*)member_index;
}


int main(int argc, FAR char *argv[])
{
  pthread_t thread_id[MemberMax];
  int ret[MemberMax];
  int i;

  for(i = 0; i < (MemberMax-1); i++){
    if (pthread_mutex_init(&raise_hand_furi_mutex[i], NULL) != 0) {
        printf("Error; pthread_mutex_init-%d.\n", i);
        exit(1);
    }
  }

  for(i = 0; i < (MemberMax-1); i++){
    if (pthread_cond_init(&raise_hand_furi_cond[i], NULL) != 0) {
        printf("Error; pthread_cond_init-%d.\n", i);
        exit(1);
    }
  }

  if (pthread_barrier_init(&raise_hand_furi_completion_barrier, NULL, MemberMax) != 0) {
      printf("Error; pthread_barrier_init(raise_hand_furi_completion_barrier).\n");
      exit(1);
  }

  if (pthread_barrier_init(&raise_hand_barrier, NULL, MemberMax) != 0) {
      printf("Error; pthread_barrier_init(raise_hand_barrier).\n");
      exit(1);
  }

  if (pthread_barrier_init(&douzo_wait_barrier, NULL, MemberMax) != 0) {
      printf("Error; pthread_barrier_init(douzo_wait_barrier).\n");
      exit(1);
  }

  if (pthread_barrier_init(&tukkomi_wait_barrier, NULL, MemberMax) != 0) {
      printf("Error; pthread_barrier_init(douzo_wait_barrier).\n");
      exit(1);
  }


  for (i = 0; i < MemberMax; i++) {
    if(pthread_create(&thread_id[i], NULL, dachoClubTable[i].func, (void*)i) != 0) {
      printf("Error; pthread_create [%d].\n", i);
      exit(1);
    }
  }

  for (i = 0; i < MemberMax; i++){
    if (pthread_join(thread_id[i], (void**)&ret[i]) != 0) {
      printf("Error; pthread_join[%d].\n", i);
      exit(1);
    }
  }

  for (i = 0; i < (MemberMax-1); i++) {
    if (pthread_mutex_destroy(&raise_hand_furi_mutex[i]) != 0) {
        printf("Error; pthread_mutex_destroy-%d.\n", i);
        exit(1);
    }
  }

  for (i = 0; i < (MemberMax-1); i++) {
    if (pthread_cond_destroy(&raise_hand_furi_cond[i]) != 0) {
        printf("Error; pthread_cond_destroy-%d.\n", i);
        exit(1);
    }
  }

  if (pthread_barrier_destroy(&raise_hand_furi_completion_barrier) != 0) {
      printf("Error; pthread_barrier_destroy(raise_hand_furi_completion_barrier).\n");
      exit(1);
  }

  if (pthread_barrier_destroy(&raise_hand_barrier) != 0) {
      printf("Error; pthread_barrier_destroy(raise_hand_barrier).\n");
      exit(1);
  }

  if (pthread_barrier_destroy(&douzo_wait_barrier) != 0) {
      printf("Error; pthread_barrier_destroy(douzo_wait_barrier).\n");
      exit(1);
  }

  if (pthread_barrier_destroy(&tukkomi_wait_barrier) != 0) {
      printf("Error; pthread_barrier_destroy(tukkomi_wait_barrier).\n");
      exit(1);
  }

  printf("Bye.\n");

  return 0;
}
