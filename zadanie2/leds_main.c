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

#define HSI_VALUE   16000000U
#define PCLK1_VALUE 50000000U

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

void action(const char* act){
    if (message_size != 0)
        return;
    strcpy(message_buffor, act);
    message_index = 0;
    message_size = strlen(message_buffor);    
    Delay(1000000);
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

    __disable_irq();
    
    char c = message_buffor[message_index++];
    USART2->DR = c;

    if (message_index == message_size){
        message_index = 0;
        message_size = 0;
    }
    __enable_irq();
}
/*
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
*/

void EXTI0_IRQHandler(void) {
        action("MODE\r\n");
        EXTI->PR = EXTI_PR_PR0;
}

void EXTI3_IRQHandler(void) {
        action("LEFT\r\n");
        EXTI->PR = EXTI_PR_PR3;
}

void EXTI4_IRQHandler(void) {
        action("MODE\r\n");
        EXTI->PR = EXTI_PR_PR4;
}

void EXTI9_5_IRQHandler(void) {
    if (EXTI->PR | EXTI_PR_PR5){
        action("UP\r\n");
        EXTI->PR = EXTI_PR_PR5;
    }
    if (EXTI->PR | EXTI_PR_PR6){
        action("DOWN\r\n");
        EXTI->PR = EXTI_PR_PR6;
    }
}

void EXTI15_10_IRQHandler(void) {
    if (EXTI->PR | EXTI_PR_PR10){
        action("FIRE\r\n");
        EXTI->PR = EXTI_PR_PR10;
    }
    if (EXTI->PR | EXTI_PR_PR13){
        action("USER\r\n");
        EXTI->PR = EXTI_PR_PR13;
    }
}

void led_init(){
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN | RCC_AHB1ENR_GPIOBEN | RCC_AHB1ENR_GPIOCEN;
    __NOP();

    RCC->APB2ENR |= RCC_APB2ENR_SYSCFGEN;
    //SYSCFG->EXTICR[3] |= SYSCFG_EXTICR4_EXTI13_PC;

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

    //For interrupts
    GPIOinConfigure(GPIOC, 13, GPIO_PuPd_UP,
    EXTI_Mode_Interrupt, EXTI_Trigger_Falling);

    GPIOinConfigure(GPIOB, 3, GPIO_PuPd_UP,
    EXTI_Mode_Interrupt, EXTI_Trigger_Falling);
    GPIOinConfigure(GPIOB, 4, GPIO_PuPd_UP,
    EXTI_Mode_Interrupt, EXTI_Trigger_Falling);
    GPIOinConfigure(GPIOB, 5, GPIO_PuPd_UP,
    EXTI_Mode_Interrupt, EXTI_Trigger_Falling);
    GPIOinConfigure(GPIOB, 6, GPIO_PuPd_UP,
    EXTI_Mode_Interrupt, EXTI_Trigger_Falling);
    GPIOinConfigure(GPIOB, 10, GPIO_PuPd_UP,
    EXTI_Mode_Interrupt, EXTI_Trigger_Falling);

    GPIOinConfigure(GPIOA, 0, GPIO_PuPd_DOWN,
    EXTI_Mode_Interrupt, EXTI_Trigger_Falling);

    NVIC_EnableIRQ(EXTI0_IRQn);
    NVIC_EnableIRQ(EXTI3_IRQn);
    NVIC_EnableIRQ(EXTI4_IRQn);
    NVIC_EnableIRQ(EXTI9_5_IRQn);
    NVIC_EnableIRQ(EXTI15_10_IRQn);
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
    USART2->BRR = (PCLK1_VALUE + (baudrate / 2U)) /
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

void clock_100_configure(){
    int N = 100;
    int M = 4;
    int P = 2;
    int Q = 4;

    RCC->CR &= ~(RCC_CR_PLLI2SON | RCC_CR_PLLON | RCC_CR_HSEBYP | RCC_CR_HSEON);
    RCC->CR |= RCC_CR_HSEBYP | RCC_CR_HSEON;
    
    while(!(RCC->CR & RCC_CR_HSERDY));
    
    /*
    int counter = 0;
    while(counter < 1000000){
        if (RCC->CR & RCC_CR_HSERDY) break;
        counter++;
    }
    if (counter == 1000000){
        action("ERROR WITH HSE CLOCK");
        return;
    }
    */
    
    RCC->PLLCFGR = RCC_PLLCFGR_PLLSRC_HSE | M | N << 6 | ((P >> 1) - 1) << 16 | Q << 24;
    RCC->CR |= RCC_CR_PLLON;

    while (!(RCC->CR & RCC_CR_PLLRDY));

    /*
    counter = 0;
    while(counter < 1000000){
        if (RCC->CR & RCC_CR_PLLRDY) break;
        counter++;
    }
    if (counter == 1000000){
        action("ERROR WITH PLL");
        return;
    }
    */

    int latency = 100000000 / 30000000;
   
    FLASH->ACR = FLASH_ACR_DCEN |
                 FLASH_ACR_ICEN |
                 FLASH_ACR_PRFTEN |
                 latency;

    uint32_t reg;

    reg = RCC->CFGR;
    reg &= ~RCC_CFGR_HPRE;
    reg |= RCC_CFGR_HPRE_DIV1;
    RCC->CFGR = reg;

    reg = RCC->CFGR;
    reg &= ~RCC_CFGR_PPRE1;
    reg |= RCC_CFGR_PPRE1_DIV2;
    RCC->CFGR = reg;

    reg = RCC->CFGR;
    reg &= ~RCC_CFGR_PPRE2;
    reg |= RCC_CFGR_PPRE2_DIV1;
    RCC->CFGR = reg;
 
    reg = RCC->CFGR;
    reg &= ~RCC_CFGR_SW;
    reg |= RCC_CFGR_SW_PLL;
    RCC->CFGR = reg;

    while(!((RCC->CFGR & RCC_CFGR_SWS) == RCC_CFGR_SWS_PLL));
    
    /*
    counter = 0;
    while(counter <1000000){
        if ((RCC->CFGR & RCC_CFGR_SWS) == RCC_CFGR_SWS_PLL)
            break;
    }
    
    if (counter == 1000000){
        action("ERROR WITH SYSCLK");
    }
    */
}

int main() {
    clock_100_configure();
    led_init();
    usart_init();
    action("HELLO\r\n");
    
    led_on(1);

    while(1){
        //check_usart_read();
        check_usart_write();
        //Delay(50);
        //led_on(2);
        //Delay(50);
        //led_off(2); 
    }

    return 0;
}

