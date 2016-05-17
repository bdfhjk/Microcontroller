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

char ready_to_process;

void init_logic(){
  queue_read_a = 0;
  queue_read_n = 0;

  queue_print_a = 0;
  queue_print_n = 0;

  number_1 = 0;
  number_2 = 0;

  operation = 0;

  state = 0;

  ready_to_process = 0;
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

  RCC->APB1ENR |= RCC_APB1ENR_TIM3EN;
  __NOP();

  //Counting up mode.
  TIM3->CR1 = 0;

  //Prescaler (Fck_cnt = Fck_tim / (PSC+1))
  //10ms = 100hz
  //Base clock: 50MHz
  //50Mhz / 100hz = 500'000
  TIM3->PSC = 65000;//65000;//500000;
  TIM3->ARR = 100;

  TIM3->EGR = TIM_EGR_UG;

  //Enable interrupts
  TIM3->SR = ~(TIM_SR_UIF);
  TIM3->DIER = TIM_DIER_UIE;
  NVIC_EnableIRQ(TIM3_IRQn);
}

void TIM3_IRQHandler(void) {
  if (
      (GPIOC->IDR & (1 << 6)) &&
      (GPIOC->IDR & (1 << 7)) &&
      (GPIOC->IDR & (1 << 8)) &&
      (GPIOC->IDR & (1 << 9))
    )

    //scan_keyboard();

    if (ready_to_process != 0)
      operations_print_queue[queue_print_n++] = ready_to_process;

    TIM3->CR1 &= 0x1110;
    TIM3->DIER = TIM_DIER_UIE;
    TIM3->SR = ~TIM_SR_UIF;
    TIM3->SR;

    GPIOC->BSRRH = 1 << 0;
    GPIOC->BSRRH = 1 << 1;
    GPIOC->BSRRH = 1 << 2;
    GPIOC->BSRRH = 1 << 3;
    NVIC_EnableIRQ(EXTI9_5_IRQn);

/*
  if (
      (GPIOC->IDR & (1 << 6)) &&
      (GPIOC->IDR & (1 << 7)) &&
      (GPIOC->IDR & (1 << 8)) &&
      (GPIOC->IDR & (1 << 9))
    )
      operations_print_queue[queue_print_n++] = '0' + 6;
  else
      operations_print_queue[queue_print_n++] = '0' + 8;

  //TIM3->CR1 &= 0x1110;

  /*
  uint32_t it_status = TIM3->SR & TIM3->DIER;

  //Update event
  if (it_status & TIM_SR_UIF) {
    TIM3->SR = ~TIM_SR_UIF;
  }

  //A button is still pressed
  if (
      (GPIOC->IDR & (1 << 6)) &&
      (GPIOC->IDR & (1 << 7)) &&
      (GPIOC->IDR & (1 << 8)) &&
      (GPIOC->IDR & (1 << 9))
    ) {
      //operations_print_queue[queue_print_n++] = '0' + 7;

      //Disable timTIM3->EGR = TIM_EGR_UG;print_queue[queue_print_n++] = '0' + 5;

        //Low state on collumns
        GPIOC->BSRRH = 1 << 0;
        GPIOC->BSRRH = 1 << 1;
        GPIOC->BSRRH = 1 << 2;
        GPIOC->BSRRH = 1 << 3;
        NVIC_EnableIRQ(EXTI9_5_IRQn);
        //Disable timer
        TIM3->CR1 &= 0x1110;

        //Clear interrupt bits and enable interrupts
        EXTI->PR = 255;

        //NVIC_EnableIRQ(EXTI9_5_IRQn);

    }
        //scan_keyboard(); */


}

void scan_keyboard(){
  ready_to_process = 0;
  int i;
  //Set only 1 column active
  GPIOC->BSRRH = 1 << 0;
  GPIOC->BSRRL = 1 << 1;
  GPIOC->BSRRL = 1 << 2;
  GPIOC->BSRRL = 1 << 3;

  for (i = 0; i < 1000; i++)
  __NOP();

  if (!(GPIOC->IDR & (1 << 6))) ready_to_process = '0' + 1;
  if (!(GPIOC->IDR & (1 << 7))) ready_to_process = '0' + 4;
  if (!(GPIOC->IDR & (1 << 8))) ready_to_process = '0' + 7;
  if (!(GPIOC->IDR & (1 << 9))) ready_to_process = 'C';


  //Set only 1 column active
  GPIOC->BSRRL = 1 << 0;
  GPIOC->BSRRH = 1 << 1;
  GPIOC->BSRRL = 1 << 2;
  GPIOC->BSRRL = 1 << 3;

  for (i = 0; i < 1000; i++)
  __NOP();

  if (!(GPIOC->IDR & (1 << 6))) ready_to_process = '0' + 2;
  if (!(GPIOC->IDR & (1 << 7))) ready_to_process = '0' + 5;
  if (!(GPIOC->IDR & (1 << 8))) ready_to_process = '0' + 8;
  if (!(GPIOC->IDR & (1 << 9))) ready_to_process = '0';

  //Set only 1 column active
  GPIOC->BSRRL = 1 << 0;
  GPIOC->BSRRL = 1 << 1;
  GPIOC->BSRRH = 1 << 2;
  GPIOC->BSRRL = 1 << 3;

  for (i = 0; i < 1000; i++)
  __NOP();

  if (!(GPIOC->IDR & (1 << 6))) ready_to_process = '0' + 3;
  if (!(GPIOC->IDR & (1 << 7))) ready_to_process = '0' + 6;
  if (!(GPIOC->IDR & (1 << 8))) ready_to_process = '0' + 9;
  if (!(GPIOC->IDR & (1 << 9))) ready_to_process = '=';

  //Set only 1 column active
  GPIOC->BSRRL = 1 << 0;
  GPIOC->BSRRL = 1 << 1;
  GPIOC->BSRRL = 1 << 2;
  GPIOC->BSRRH = 1 << 3;

  for (i = 0; i < 1000; i++)
  __NOP();

  if (!(GPIOC->IDR & (1 << 6))) ready_to_process = '+';
  if (!(GPIOC->IDR & (1 << 7))) ready_to_process = '-';
  if (!(GPIOC->IDR & (1 << 8))) ready_to_process = '*';
  if (!(GPIOC->IDR & (1 << 9))) ready_to_process = '/';

}

void configure_keyboard() {
  RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
  RCC->AHB1ENR |= RCC_AHB1ENR_GPIOCEN;
  __NOP();
  __NOP();
  //Configure columns
  GPIOoutConfigure(GPIOC, 0, GPIO_OType_PP, GPIO_Low_Speed, GPIO_PuPd_NOPULL);
  GPIOoutConfigure(GPIOC, 1, GPIO_OType_PP, GPIO_Low_Speed, GPIO_PuPd_NOPULL);
  GPIOoutConfigure(GPIOC, 2, GPIO_OType_PP, GPIO_Low_Speed, GPIO_PuPd_NOPULL);
  GPIOoutConfigure(GPIOC, 3, GPIO_OType_PP, GPIO_Low_Speed, GPIO_PuPd_NOPULL);

  GPIOC->BSRRH = 1 << 0;
  GPIOC->BSRRH = 1 << 1;
  GPIOC->BSRRH = 1 << 2;
  GPIOC->BSRRH = 1 << 3;

  //Configure rows
  GPIOinConfigure(GPIOC, 6, GPIO_PuPd_UP, EXTI_Mode_Interrupt, EXTI_Trigger_Falling);
  GPIOinConfigure(GPIOC, 7, GPIO_PuPd_UP, EXTI_Mode_Interrupt, EXTI_Trigger_Falling);
  GPIOinConfigure(GPIOC, 8, GPIO_PuPd_UP, EXTI_Mode_Interrupt, EXTI_Trigger_Falling);
  GPIOinConfigure(GPIOC, 9, GPIO_PuPd_UP, EXTI_Mode_Interrupt, EXTI_Trigger_Falling);

  //Clear interrupt bits and enable interrupts
  EXTI->PR = 255;
  EXTI->PR;

  NVIC_EnableIRQ(EXTI9_5_IRQn);
}

void EXTI9_5_IRQHandler(void) {
    //Temporary disable interrupts
    NVIC_DisableIRQ(EXTI9_5_IRQn);

    scan_keyboard();
    //Set high state in collumn lines
    GPIOC->BSRRL = 1 << 0;
    GPIOC->BSRRL = 1 << 1;
    GPIOC->BSRRL = 1 << 2;
    GPIOC->BSRRL = 1 << 3;

    //Reset timer CNT
    TIM3->CNT = 0;



    if (EXTI->PR & EXTI_PR_PR6){
        EXTI->PR |= EXTI_PR_PR6;
    }
    if (EXTI->PR & EXTI_PR_PR7){
        EXTI->PR |= EXTI_PR_PR7;
    }
    if (EXTI->PR & EXTI_PR_PR8){
        EXTI->PR |= EXTI_PR_PR8;
    }
    if (EXTI->PR & EXTI_PR_PR9){
        EXTI->PR |= EXTI_PR_PR9;
    }

    //operations_print_queue[queue_print_n++] = '0' + 4;
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
  init_logic();
  configure_timer();
  configure_keyboard();
  //operations_print_queue[queue_print_n++] = '0' + 1;
  //operations_print_queue[queue_print_n++] = '0' + 2;
  //operations_print_queue[queue_print_n++] = '0' + 3;

  //press(5);
  //press(1);

  while(1){
    process_read();
    process_write();
  }
  return 0;
}
