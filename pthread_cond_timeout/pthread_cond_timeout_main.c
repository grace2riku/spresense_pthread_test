#include <nuttx/config.h>
#include <stdio.h>
#include <stdlib.h> // exit

#include <arch/board/board.h>
#include <arch/chip/pin.h>
#include <nuttx/clock.h>

#define THREAD_NUM (2)  // スレッドの数

// APS学習ボードピンアサイン
#define SWITCH_1    (39)  // 拡張ボード JP13-1 SPI3_CS1_X Spresense SDKピン番号39
#define SWITCH_2    (29)  // 拡張ボード JP13-4 SPI2_MOSI Spresense SDKピン番号29
#define USER_LED_1  (46)  // 拡張ボード JP13-2 PIN_PWM0 Spresense SDKピン番号46
#define USER_LED_2  (47)  // 拡張ボード JP13-3 PIN_PWM1 Spresense SDKピン番号47

pthread_mutex_t switch_mutex_timeout[THREAD_NUM];
pthread_cond_t switch_cond_timeout[THREAD_NUM];

const int aps_board_led_pin_4[2] = {
  USER_LED_1,
  USER_LED_2
};

const int aps_board_switch_pin_4[2] = {
  SWITCH_1,
  SWITCH_2
};

/*
* ミリ秒単位で条件待ちをする
*/
int pthread_cond_timedwait_msec(pthread_cond_t* cond,
    pthread_mutex_t* mutex, long msec) {
    int ret;
    struct timespec ts;
    ret = clock_systime_timespec(&ts);
    if(ret != 0) {
      printf("Error; clock_systime_timespec[%d].\n", ret);
      exit(1);
    }
    ts.tv_sec += msec / 1000;
    ts.tv_nsec += (msec % 1000) * 1000000;
    if (ts.tv_nsec >= 1000000000) {
        ts.tv_sec++;
        ts.tv_nsec -= 1000000000;
    }
    return pthread_cond_timedwait(cond, mutex, &ts);
}


void* thread_cond_signal_func2(void* arg) {
  int switch_index = (uint32_t)arg;

  volatile int switch_status = board_gpio_read(aps_board_switch_pin_4[switch_index]);

  while (switch_status == 1) {  // スイッチ押下されたらループから抜ける
    switch_status = board_gpio_read(aps_board_switch_pin_4[switch_index]);
    sleep(1);
  }

  pthread_cond_signal(&switch_cond_timeout[switch_index]);

  printf("thread_cond_signal[%d]:switch[%d] pushed!!!cond_signal[%d] set.\n", switch_index, switch_index + 1, switch_index);

  return (void*)switch_index;
}

void* thread_cond_timedwait_func(void* arg) {
  int switch_index = (uint32_t)arg;
  volatile int stopRequest = 0;
  int ret;

  printf("thread_cond_wait[%d]:cond_wait[%d]...\n", switch_index, switch_index);

  // タイムアウト条件待ち
  pthread_mutex_lock(&switch_mutex_timeout[switch_index]);
  while(!stopRequest) {
    ret = pthread_cond_timedwait_msec(&switch_cond_timeout[switch_index], &switch_mutex_timeout[switch_index], 10000);
    switch (ret)
    {
    case 0: /* got signal */
        stopRequest = 1;
        printf("thread_cond_wait[%d]:got signal.\n", switch_index);
        break;
    case ETIMEDOUT: /* timeout */
        printf("thread_cond_wait[%d]:timeout.\n", switch_index);
        break;
    default:        /* fatal error */
        printf("thread_cond_wait[%d]:Fatal error(%d) on pthread_cond_timedwait.\n", switch_index, ret);
        exit(1);
    }
  }
  pthread_mutex_unlock(&switch_mutex_timeout[switch_index]);

  return (void*)switch_index;
}


int main(int argc, FAR char *argv[])
{
  pthread_t thread_cond_signal[THREAD_NUM];
  pthread_t thread_cond_wait[THREAD_NUM];
  int ret[THREAD_NUM];
  int i;

  for(i = 0; i < THREAD_NUM; i++){
    if (pthread_mutex_init(&switch_mutex_timeout[i], NULL) != 0) {
        printf("Error; pthread_mutex_init-%d.\n", i);
        exit(1);
    }
  }

  for(i = 0; i < THREAD_NUM; i++){
    if (pthread_cond_init(&switch_cond_timeout[i], NULL) != 0) {
        printf("Error; pthread_cond_init-%d.\n", i);
        exit(1);
    }
  }

  for(i = 0; i < 2; i++){
    board_gpio_config(aps_board_switch_pin_4[i], 0, true, true, PIN_PULLUP); // GPIOの設定 入力ポート有効
  }

  for (i = 0; i < THREAD_NUM; i++) {
    if(pthread_create(&thread_cond_signal[i], NULL, thread_cond_signal_func2, (void*)i) != 0) {
      printf("Error; pthread_create cond_signal[%d].\n", i);
      exit(1);
    }
    if(pthread_create(&thread_cond_wait[i], NULL, thread_cond_timedwait_func, (void*)i) != 0) {
      printf("Error; pthread_create cond_wait[%d].\n", i);
      exit(1);
    }
  }

  for (i = 0; i < THREAD_NUM; i++){
    if (pthread_join(thread_cond_signal[i], (void**)&ret[i]) != 0) {
      printf("Error; thread_cond_signal[%d].\n", i);
      exit(1);
    }
    printf("main:thread_cond_signal[%d] exit.return value = %d\n", i, ret[i]);
  }

  for (i = 0; i < THREAD_NUM; i++){
    if (pthread_join(thread_cond_wait[i], (void**)&ret[i]) != 0) {
      printf("Error; thread_cond_wait[%d].\n", i);
      exit(1);
    }
    printf("main:thread_cond_wait[%d] exit.return value = %d\n", i, ret[i]);
  }

  for (i = 0; i < THREAD_NUM; i++) {
    if (pthread_mutex_destroy(&switch_mutex_timeout[i]) != 0) {
        printf("Error; pthread_mutex_destroy-%d.\n", i);
        exit(1);
    }
  }

  for (i = 0; i < THREAD_NUM; i++) {
    if (pthread_cond_destroy(&switch_cond_timeout[i]) != 0) {
        printf("Error; pthread_cond_destroy-%d.\n", i);
        exit(1);
    }
  }

  printf("Bye.\n");

  return 0;
}
