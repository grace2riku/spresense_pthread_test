#include <nuttx/config.h>
#include <stdio.h>
#include <stdlib.h> // exit

#include <arch/board/board.h>
#include <arch/chip/pin.h>

#define THREAD_NUM (2)  // スレッドの数

// APS学習ボードピンアサイン
#define SWITCH_1    (39)  // 拡張ボード JP13-1 SPI3_CS1_X Spresense SDKピン番号39
#define SWITCH_2    (29)  // 拡張ボード JP13-4 SPI2_MOSI Spresense SDKピン番号29
#define USER_LED_1  (46)  // 拡張ボード JP13-2 PIN_PWM0 Spresense SDKピン番号46
#define USER_LED_2  (47)  // 拡張ボード JP13-3 PIN_PWM1 Spresense SDKピン番号47

pthread_mutex_t using_blink_led;

const int aps_board_led_pin_2[2] = {
  USER_LED_1,
  USER_LED_2
};

const int aps_board_switch_pin_2[2] = {
  SWITCH_1,
  SWITCH_2
};

void* blink_led_mutex(void* arg) {
  uint32_t led_index = (uint32_t)arg;
  uint32_t switch_index = (uint32_t)arg;
  unsigned int sleep_sec = 1 + (uint32_t)arg; // sleepする秒数
  int led_value = 1;

  volatile int switch_status = board_gpio_read(aps_board_switch_pin_2[switch_index]);

  printf("start blink_led[%ld] thread ID = %d switch_status = %d.\n", led_index, pthread_self(), switch_status);

  while (switch_status == 1) {  // スイッチ押下されたらループから抜ける
    pthread_mutex_lock(&using_blink_led);

    board_gpio_write(aps_board_led_pin_2[led_index], led_value);
    sleep(sleep_sec);
    led_value ^= 1;
    switch_status = board_gpio_read(aps_board_switch_pin_2[switch_index]);
    printf("blink_led[%ld] thread ID = %d led_value = %d switch_status = %d.\n", led_index, pthread_self(), led_value, switch_status);

    pthread_mutex_unlock(&using_blink_led);
  }

  printf("end blink_led[%ld] thread ID = %d.\n", led_index, pthread_self());

  return (void*)led_index;
}


int main(int argc, FAR char *argv[])
{
  pthread_t thread[THREAD_NUM];
  int ret[THREAD_NUM];
  int i;

  if (pthread_mutex_init(&using_blink_led, NULL) != 0) {
      printf("Error; pthread_mutex_init.\n");
      exit(1);
  }

  for(i = 0; i < 2; i++){
    board_gpio_config(aps_board_switch_pin_2[i], 0, true, true, PIN_PULLUP); // GPIOの設定 入力ポート有効
  }

  for (i = 0; i < THREAD_NUM; i++) {
    if(pthread_create(&thread[i], NULL, blink_led_mutex, (void*)i) != 0) {
      printf("Error; pthread_create[%d].\n", i);
      exit(1);
    }
  }  

  for (i = 0; i < THREAD_NUM; i++){
    if (pthread_join(thread[i], (void**)&ret[i]) != 0) {
      printf("Error; pthread_join[%d].\n", i);
      exit(1);
    }
    printf("exit thread[%d] thread ID = %d return value = %d\n", i, pthread_self(), ret[i]);
  }

  if (pthread_mutex_destroy(&using_blink_led) != 0) {
      printf("Error; pthread_mutex_destroy.\n");
      exit(1);
  }

  printf("Bye.\n");

  return 0;
}
