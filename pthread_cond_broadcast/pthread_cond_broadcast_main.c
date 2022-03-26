#include <nuttx/config.h>
#include <stdio.h>
#include <stdlib.h> // exit

#include <arch/board/board.h>
#include <arch/chip/pin.h>

#define WAIT_THREAD_NUM   (3)  // スレッドの数

// APS学習ボードピンアサイン
#define SWITCH_1    (39)  // 拡張ボード JP13-1 SPI3_CS1_X Spresense SDKピン番号39
#define SWITCH_2    (29)  // 拡張ボード JP13-4 SPI2_MOSI Spresense SDKピン番号29
#define USER_LED_1  (46)  // 拡張ボード JP13-2 PIN_PWM0 Spresense SDKピン番号46
#define USER_LED_2  (47)  // 拡張ボード JP13-3 PIN_PWM1 Spresense SDKピン番号47

pthread_mutex_t switch_mutex_5;
pthread_cond_t switch_cond_5;

const int aps_board_led_pin_5[2] = {
  USER_LED_1,
  USER_LED_2
};

const int aps_board_switch_pin_5[2] = {
  SWITCH_1,
  SWITCH_2
};

void* thread_cond_broadcast(void* arg) {
  int switch_index = (uint32_t)arg;

  volatile int switch_status = board_gpio_read(aps_board_switch_pin_5[switch_index]);

  while (switch_status == 1) {  // スイッチ押下されたらループから抜ける
    switch_status = board_gpio_read(aps_board_switch_pin_5[switch_index]);
    sleep(1);
  }

  pthread_cond_broadcast(&switch_cond_5);

  printf("thread_cond_broadcast[%d]:switch[%d] pushed!!!cond_signal set.\n", switch_index, switch_index + 1);

  return (void*)switch_index;
}

void* thread_cond_wait_5(void* arg) {
  int switch_index = (uint32_t)arg;

  printf("thread_cond_wait[%d]:cond_wait[%d]...\n", switch_index, switch_index);

  // 条件待ち
  pthread_mutex_lock(&switch_mutex_5);
  if (pthread_cond_wait(&switch_cond_5, &switch_mutex_5) != 0) {
      printf("Fatal error on pthread[%d]_cond_wait.\n", switch_index);
      exit(1);
  }
  pthread_mutex_unlock(&switch_mutex_5);

  printf("thread_cond_wait[%d]:wait end.\n", switch_index);

  return (void*)switch_index;
}


int main(int argc, FAR char *argv[])
{
  pthread_t thread_cond_signal;
  pthread_t thread_cond_wait[WAIT_THREAD_NUM];
  int ret[WAIT_THREAD_NUM];
  int i;

  if (pthread_mutex_init(&switch_mutex_5, NULL) != 0) {
      printf("Error; pthread_mutex_init.\n");
      exit(1);
  }

  if (pthread_cond_init(&switch_cond_5, NULL) != 0) {
      printf("Error; pthread_cond_init.\n");
      exit(1);
  }

  for(i = 0; i < 2; i++){
    board_gpio_config(aps_board_switch_pin_5[i], 0, true, true, PIN_PULLUP); // GPIOの設定 入力ポート有効
  }

  for (i = 0; i < WAIT_THREAD_NUM; i++) {
    if(pthread_create(&thread_cond_wait[i], NULL, thread_cond_wait_5, (void*)i) != 0) {
      printf("Error; pthread_create cond_wait[%d].\n", i);
      exit(1);
    }
  }

  if(pthread_create(&thread_cond_signal, NULL, thread_cond_broadcast, (void*)0) != 0) {
    printf("Error; pthread_create cond_broadcast.\n");
    exit(1);
  }

  if (pthread_join(thread_cond_signal, (void**)&ret[0]) != 0) {
    printf("Error; thread_cond_broadcast.\n");
    exit(1);
  }
  printf("main:thread_cond_broadcast[%d] exit.return value = %d\n", i, ret[0]);

  for (i = 0; i < WAIT_THREAD_NUM; i++){
    if (pthread_join(thread_cond_wait[i], (void**)&ret[i]) != 0) {
      printf("Error; thread_cond_wait[%d].\n", i);
      exit(1);
    }
    printf("main:thread_cond_wait[%d] exit.return value = %d\n", i, ret[i]);
  }

  if (pthread_mutex_destroy(&switch_mutex_5) != 0) {
      printf("Error; pthread_mutex_destroy.\n");
      exit(1);
  }

  if (pthread_cond_destroy(&switch_cond_5) != 0) {
      printf("Error; pthread_cond_destroy.\n");
      exit(1);
  }

  printf("Bye.\n");

  return 0;
}
