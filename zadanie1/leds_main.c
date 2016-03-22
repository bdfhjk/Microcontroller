#include <delay.h>
#include <gpio.h>
#include <stm32.h>
#include <string.h>

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

#define HSI_VALUE 16000000U


char command_buffor[100];
int command_length;

int state[5];

char message_buffor[100];
int message_index;
int message_size;

void led_on(int number){
    if (number == 1)
        GPIOA->BSRRH = 1 << 6;
    if (number == 2)
        GPIOA->BSRRH = 1 << 7;
    if (number == 3)
        GPIOB->BSRRH = 1 << 0;
    if (number == 4)
        GPIOA->BSRRL = 1 << 5;

    state[number] = 1;
}

void led_off(int number){
    if (number == 1)
        GPIOA->BSRRL = 1 << 6;
    if (number == 2)
        GPIOA->BSRRL = 1 << 7;
    if (number == 3)
        GPIOB->BSRRL = 1 << 0;
    if (number == 4)
        GPIOA->BSRRH = 1 << 5;

    state[number] = 0;
}

void led_toggle(int number){
    if (state[number] == 0)
        led_on(number);
    else
        led_off(number);
}

void check_usart_write(){
    if (!(message_index < message_size))
        return;

    if (!(USART2->SR & USART_SR_TXE))
        return;

    char c = message_buffor[message_index++];
    USART2->DR = c;

    if (message_index == message_size){
        message_size = 0;
        message_index = 0;
    }
     
}

void check_usart_read(){
    if (!(USART2->SR & USART_SR_RXNE))
        return;
    
    char c;
    c = USART2->DR;
    
    command_buffor[command_length] = c;
    command_length++;
    command_buffor[command_length] = 0;

    // LED 1

    if (strcmp(command_buffor, "LED 1 ON")==0){       
        led_on(1);
        command_length = 0;
    }
    if (strcmp(command_buffor, "LED 1 OFF")==0){       
        led_off(1);
        command_length = 0;
    }
    if (strcmp(command_buffor, "LED 1 TOGGLE")==0){       
        led_toggle(1);
        command_length = 0;
    }

    // LED 2

    if (strcmp(command_buffor, "LED 2 ON")==0){       
        led_on(2);
        command_length = 0;
    }
    if (strcmp(command_buffor, "LED 2 OFF")==0){       
        led_off(2);
        command_length = 0;
    }
    if (strcmp(command_buffor, "LED 2 TOGGLE")==0){       
        led_toggle(2);
        command_length = 0;
    }

    // LED 3

    if (strcmp(command_buffor, "LED 3 ON")==0){       
        led_on(3);
        command_length = 0;
    }
    if (strcmp(command_buffor, "LED 3 OFF")==0){       
        led_off(3);
        command_length = 0;
    }
    if (strcmp(command_buffor, "LED 3 TOGGLE")==0){       
        led_toggle(3);
        command_length = 0;
    }


    // LED 4

    if (strcmp(command_buffor, "LED 4 ON")==0){       
        led_on(4);
        command_length = 0;
    }
    if (strcmp(command_buffor, "LED 4 OFF")==0){       
        led_off(4);
        command_length = 0;
    }
    if (strcmp(command_buffor, "LED 4 TOGGLE")==0){       
        led_toggle(4);
        command_length = 0;
    }

}

void led_init(){
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN;
    __NOP();
    GPIOA->BSRRL = 1 << 6 | 1 << 7;
    GPIOB->BSRRL = 1 << 0;
    GPIOA->BSRRH = 1 << 5;
    GPIOoutConfigure(GPIOA, 6, GPIO_OType_PP,
    GPIO_Low_Speed, GPIO_PuPd_NOPULL);
    GPIOoutConfigure(GPIOA, 7, GPIO_OType_PP,
    GPIO_Low_Speed, GPIO_PuPd_NOPULL);
    GPIOoutConfigure(GPIOB, 0, GPIO_OType_PP,
    GPIO_Low_Speed, GPIO_PuPd_NOPULL);
    GPIOoutConfigure(GPIOA, 5, GPIO_OType_PP,
    GPIO_Low_Speed, GPIO_PuPd_NOPULL);

    GPIOinConfigure(GPIOC, 13, GPIO_PuPd_NOPULL,
    EXTI_Mode_Disable, EXTI_Trigger_Irrelevant);
}

void usart_init(){
    //Taktowanie układów zasilania USART
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;

    //Konfituracja linii USART
    GPIOafConfigure(GPIOA,
        2,
        GPIO_OType_PP,
        GPIO_Fast_Speed,
        GPIO_PuPd_NOPULL,
        GPIO_AF_USART2);

    GPIOafConfigure(GPIOA,
            3,
            GPIO_OType_PP,
            GPIO_Fast_Speed,
            GPIO_PuPd_UP,
            GPIO_AF_USART2);

    USART2->CR1 = USART_Mode_Rx_Tx |
        USART_WordLength_8b |
        USART_Parity_No;

    USART2->CR2 = USART_StopBits_1;

    USART2->CR3 = USART_FlowControl_None;

    uint32_t baudrate = 9600U;
    USART2->BRR = (HSI_VALUE + (baudrate / 2U)) /
        baudrate;

    USART2->CR1 |= USART_Enable;
}

void led_test(){
    for (;;) {
        GPIOA->BSRRH = 1 << 6;
        Delay(400000);
        GPIOA->BSRRL = 1 << 6;
        GPIOA->BSRRH = 1 << 7;
        Delay(400000);
        GPIOA->BSRRL = 1 << 7;
        GPIOB->BSRRH = 1 << 0;
        Delay(400000);
        GPIOB->BSRRL = 1 << 0;
        GPIOA->BSRRL = 1 << 5;
        Delay(400000);
        GPIOA->BSRRH = 1 << 5;;
    }
}

void action(const char* act){
    strcpy(message_buffor, act);
    message_index = 0;
    message_size = strlen(message_buffor);    
}

void check_IO(){

    if (message_size != 0)
        return;

    // USER PC13 0
    if ((GPIOC->IDR & (1 << 13)) == 0){
        action("USER\r\n");
        Delay(900000);
    }

    // LEFT PB3 0
    if ((GPIOB->IDR & (1 << 3)) == 0){
        action("LEFT\r\n");
        Delay(900000);
    }
    
    // RIGHT PB4 0
    if ((GPIOB->IDR & (1 << 4)) == 0){
        action("RIGHT\r\n");
        Delay(900000);
    }

    // UP PB5 0
    if ((GPIOB->IDR & (1 << 5)) == 0){
        action("UP\r\n");
        Delay(900000);
    }

    // DOWN PB6 0
    if ((GPIOB->IDR & (1 << 6)) == 0){
        action("DOWN\r\n");
        Delay(900000);
    }

    // FIRE  PB10 0
    if ((GPIOB->IDR & (1 << 10)) == 0){
        action("FIRE\r\n");
        Delay(900000);
    }

    // MODE PA0 1
     if ((GPIOA->IDR & (1 << 0)) == 1){
        action("MODE\r\n");
        Delay(900000);
    }

}

int main() {
    led_init();
    usart_init();
    action("HELLO\r\n");
    for(;;) {
        check_usart_read();
        check_IO();
        check_usart_write();
    }

    return 0;
}

