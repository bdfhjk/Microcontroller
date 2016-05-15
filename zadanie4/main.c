#include <delay.h>
#include <gpio.h>
#include <stm32.h>
#include <string.h>
#include <fonts.h>
#include <lcd.h>
#include "constants.h"


enum STATE {
    INIT,     //Initialisation of the program
    READ_1,   //Reading the first number
    READ_2,   //Reading the second number
    WRITE,    //Writting the result to LCD
    DISPLAY   //Displaying the result on the LCD
};

enum STATE state;

long long number_1;
long long number_2;
char operation;

int queue_read_n;
int queue_read_a;
char operations_read_queue[1000];

int queue_print_n;
int queue_print_a;
char operations_print_queue[1000];

void init_logic(){
  queue_read_a = 0;
  queue_read_n = 0;

  queue_print_a = 0;
  queue_print_n = 0;

  number_1 = 0;
  number_2 = 0;

  operation = 0;

  state = 0;
}

void print_error(){
  operations_print_queue[0] = 'E';
  operations_print_queue[1] = 'R';
  operations_print_queue[2] = 'R';
  operations_print_queue[3] = 'O';
  operations_print_queue[4] = 'R';
  queue_print_n = 5;
  queue_print_a = 0;
  state = 3;
}

void print(){
  long long number_ans = 0;
  if (operation == '+') number_ans = number_1 + number_2;
  if (operation == '-') number_ans = number_1 - number_2;
  if (operation == '*') number_ans = number_1 * number_2;
  if (operation == '/') {
    if (number_2 == 0){
      print_error();
      return;
    }
    number_ans = number_1 / number_2;
  }

  if (number_ans == 0){
    queue_print_n = 1;
    queue_print_a = 0;
    operations_print_queue[0] = '0';
    return;
  }
  while (number_ans){
      int number = number_ans % 10;
      operations_print_queue[queue_print_n++] = '0' + number;
      number_ans /= 10;
      queue_print_a = 0;
  }
  state=3;
}

int is_boolean(char a){
    if (a=='+') return 1;
    if (a=='-') return 1;
    if (a=='*') return 1;
    if (a=='/') return 1;
    return 0;
}


void press(char a){
  if (state == 4)
    init_logic();
  if (state == 3 || state == 0)
    return;
  if (state == 2){
    if ('0' <= a && a <= '9'){
      number_2 *= 10;
      number_2 += a - '0';
      operations_print_queue[queue_print_n++] = a - '0';
    }
  }
  if (state == 1){
    if ('0' <= a && a <= '9'){
      number_1 *= 10;
      number_1 += a - '0';
      operations_print_queue[queue_print_n++] = a - '0';
    }
  }
  if (a == '=' && state == 2) print();
  if (state == 1 && (is_boolean(a) == 1)){
      operation = a;
      state++;
  }
}

void configure_timer(){

  //Counting up mode.
  TIM3->CR1 = 0;

  //Prescaler (Fck_cnt = Fck_tim / (PSC+1))
  //10ms = 100hz
  //Base clock: 50MHz
  //50Mhz / 100hz = 500'000
  TIM3->PSC = 500000;
  TIM3->ARR = 1;

  TIM3->SR = ~(TIM_SR_UIF | TIM_SR_CC1IF);
  TIM3->DIER = TIM_DIER_UIE | TIM_DIER_CC1IE;
  NVIC_EnableIRQ(TIM3_IRQn);
}

void TIM3_IRQHandler(void) {
  uint32_t it_status = TIM3->SR & TIM3->DIER;
  if (it_status & TIM_SR_UIF) {
    TIM3->SR = ~TIM_SR_UIF;

  }
  if (it_status & TIM_SR_CC1IF) {
    TIM3->SR = ~TIM_SR_CC1IF;

  }

  if (
      (GPIOC->IDR & (1 << 5)) &&
      (GPIOC->IDR & (1 << 6)) &&
      (GPIOC->IDR & (1 << 7)) &&
      (GPIOC->IDR & (1 << 8))
     )
        scan_keyboard();
}

void scan_keyboard(){

}

void configure_keyboard(){
  //Configure columns
  GPIOoutConfigure(GPIOC, 1, GPIO_OType_PP, GPIO_Low_Speed, GPIO_PuPd_NOPULL);
  GPIOoutConfigure(GPIOC, 2, GPIO_OType_PP, GPIO_Low_Speed, GPIO_PuPd_NOPULL);
  GPIOoutConfigure(GPIOC, 3, GPIO_OType_PP, GPIO_Low_Speed, GPIO_PuPd_NOPULL);
  GPIOoutConfigure(GPIOC, 4, GPIO_OType_PP, GPIO_Low_Speed, GPIO_PuPd_NOPULL);

  //Configure rows
  GPIOinConfigure(GPIOC, 5, GPIO_PuPd_UP, EXTI_Mode_Interrupt, EXTI_Trigger_Falling);
  GPIOinConfigure(GPIOC, 6, GPIO_PuPd_UP, EXTI_Mode_Interrupt, EXTI_Trigger_Falling);
  GPIOinConfigure(GPIOC, 7, GPIO_PuPd_UP, EXTI_Mode_Interrupt, EXTI_Trigger_Falling);
  GPIOinConfigure(GPIOC, 8, GPIO_PuPd_UP, EXTI_Mode_Interrupt, EXTI_Trigger_Falling);

  //Clear interrupt bits and enable interrupts
  EXTI->PR = EXTI_PR_PR5;
  EXTI->PR = EXTI_PR_PR6;
  EXTI->PR = EXTI_PR_PR7;
  EXTI->PR = EXTI_PR_PR8;
  NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void EXTI9_5_IRQHandler(void) {

    //Temporary disable interrupts
    NVIC_DisableIRQ(EXTI9_5_IRQn);

    //Set high state in collumn lines
    GPIOC->BSRRH = 1 << 1;
    GPIOC->BSRRH = 1 << 2;
    GPIOC->BSRRH = 1 << 3;
    GPIOC->BSRRH = 1 << 4;

    //Reset timer CNT
    TIM3->CNT = 0;

    if (EXTI->PR | EXTI_PR_PR5){

        EXTI->PR = EXTI_PR_PR5;
    }
    if (EXTI->PR | EXTI_PR_PR6){

        EXTI->PR = EXTI_PR_PR6;
    }
    if (EXTI->PR | EXTI_PR_PR7){

        EXTI->PR = EXTI_PR_PR7;
    }
    if (EXTI->PR | EXTI_PR_PR8){

        EXTI->PR = EXTI_PR_PR8;
    }

    //Start the timer
    TIM3->CR1 |= TIM_CR1_CEN;
}


void process_read()
{

}

void process_write()
{
  if (queue_print_a != queue_print_n)
    LCDputchar(operations_print_queue[queue_print_a++]);
}

int main() {
  LCDconfigure();
  press(5);
  press(1);

  while(1){
    process_read();
    process_write();
  }
  return 0;
}
