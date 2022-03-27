#include <nuttx/config.h>
#include <stdio.h>
#include <stdlib.h> // exit

#include <arch/board/board.h>
#include <arch/chip/pin.h>

#define THREAD_NUM   (3)  // スレッドの数

// APS学習ボードピンアサイン
#define SWITCH_1    (39)  // 拡張ボード JP13-1 SPI3_CS1_X Spresense SDKピン番号39
#define SWITCH_2    (29)  // 拡張ボード JP13-4 SPI2_MOSI Spresense SDKピン番号29
#define USER_LED_1  (46)  // 拡張ボード JP13-2 PIN_PWM0 Spresense SDKピン番号46
#define USER_LED_2  (47)  // 拡張ボード JP13-3 PIN_PWM1 Spresense SDKピン番号47

// 初期化完了同期用バリア変数
pthread_barrier_t initialization_completion_barrier;


void* thread_func(void* arg) {
  int thread_index = (uint32_t)arg;
  int ret;
#if 1
  unsigned int initialize_time = 3 * (thread_index + 1);
#else
  unsigned int initialize_time;

  if (thread_index == 0) {
    initialize_time = 9;    
  } else if (thread_index == 1) {
    initialize_time = 6;    
  } else {
    initialize_time = 3;
  }
#endif

  printf("thread index[%d] start.initialize_time = %d(sec).\n", thread_index, initialize_time);

  sleep(initialize_time);

  printf("thread index[%d] initialize end.\n", thread_index);

  ret = pthread_barrier_wait(&initialization_completion_barrier);
  if (!(ret == 0 || ret == PTHREAD_BARRIER_SERIAL_THREAD)) {
    printf("Error; thread index[%d] pthread_barrier_wait ret = %d.\n", thread_index, ret);
    exit(1);
  }

  printf("thread index[%d] barrier exit.ret = %d.\n", thread_index, ret);

  return (void*)thread_index;
}


int main(int argc, FAR char *argv[])
{
  pthread_t thread_id[THREAD_NUM];
  int ret[THREAD_NUM];
  int i;


  if (pthread_barrier_init(&initialization_completion_barrier, NULL, THREAD_NUM) != 0) {
      printf("Error; pthread_barrier_init.\n");
      exit(1);
  }


  for (i = 0; i < THREAD_NUM; i++) {
    if(pthread_create(&thread_id[i], NULL, thread_func, (void*)i) != 0) {
      printf("Error; pthread_create [%d].\n", i);
      exit(1);
    }
  }

  for (i = 0; i < THREAD_NUM; i++){
    if (pthread_join(thread_id[i], (void**)&ret[i]) != 0) {
      printf("Error; pthread_join[%d].\n", i);
      exit(1);
    }
    printf("main:thread index[%d] exit.return value = %d\n", i, ret[i]);
  }

  if (pthread_barrier_destroy(&initialization_completion_barrier) != 0) {
      printf("Error; pthread_barrier_destroy.\n");
      exit(1);
  }

  printf("Bye.\n");

  return 0;
}
