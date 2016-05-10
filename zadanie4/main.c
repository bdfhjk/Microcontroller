#include <delay.h>
#include <gpio.h>
#include <stm32.h>
#include <string.h>
#include <fonts.h>
#include <lcd.h>

#define USART_Mode_Rx_Tx (USART_CR1_RE | USART_CR1_TE)
#define USART_Enable      USART_CR1_UE

#define USART_WordLength_8b 0x0000
#define USART_WordLength_9b USART_CR1_M

#define USART_Parity_No 0x0000
#define USART_Parity_Even USART_CR1_PCE
#define USART_Parity_Odd (USART_CR1_PCE | USART_CR1_PS)

#define USART_StopBits_1    0x0000
#define USART_StopBits_0_5  0x1000
#define USART_StopBits_2    0x2000
#define USART_StopBits_1_5  0x3000

#define USART_FlowControl_None 0x0000
#define USART_FlowControl_RTS USART_CR3_RTSE
#define USART_FlowControl_CTS USART_CR3_CTSE

#define HSI_VALUE   16000000U
#define PCLK1_VALUE 50000000U

// 0 - inicjalizacja
// 1 - wczytywanie pierwszej liczby
// 2 - wczytywanie drugiej liczby
// 3 - wypisywanie wyniku
// 4 - wyswietlanie wyniku

char state;
int queue_read_n;
int queue_read_a;
int queue_print_n;
int queue_print_a;
long long number_1;
long long number_2;

char operations_read_queue[1000];
char operations_print_queue[1000];
char operation;

void init_logic(){
  queue_read_a = 0;
  queue_read_n = 0;

  queue_print_a = 0;
  queue_print_n = 0;

  number_1 = 0;
  number_2 = 0;

  state = 0;
}

void print_error(){
  operations_print_queue[0] = 'E';
  operations_print_queue[1] = 'R';
  operations_print_queue[2] = 'R';
  operations_print_queue[3] = 'O';
  operations_print_queue[4] = 'R';
  queue_print_n = 5;
  state = 3;
}

void print(){
  long long number_ans;
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
    operations_print_queue[0] = '0';
    return;
  }
  while (number_ans){
      int number = number_ans % 10;
      operations_print_queue[queue_print_n++] = '0' + number;
      number_ans /= 10;
  }
}

void press(char a){
  if (state == 4)
    init_logic();
  if (state == 3 || state == 0)
    return;
  if (state == 2){
    if ('0' <= a <= '9'){
      number_2 *= 10;
      number_2 += a - '0';
      operations_print_queue[queue_print_n++] = a - '0';
    }
  }
  if (state == 1){
    if ('0' <= a <= '9'){
      number_1 *= 10;
      number_1 += a - '0';
      operations_print_queue[queue_print_n++] = a - '0';
    }
  }
  if (a == '=' && state == 2) print();
  if (state == 1 && isBoolean(a))

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
