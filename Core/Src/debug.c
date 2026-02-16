#include "debug.h"
#include "gpio.h"
#include "stm32f446xx.h"

void Debug_Init(void) {
    RCC->APB1ENR |= RCC_APB1ENR_USART2EN;
    RCC->AHB1ENR |= RCC_AHB1ENR_GPIOAEN;

    GPIO_SetMode(GPIOA, 2, GPIO_MODE_AF);
    GPIO_SetAF(GPIOA, 2, 7);

        // Make sure these are clear (no parity, 1 stop bit)
    USART2->CR1 &= ~(USART_CR1_PCE | USART_CR1_M);  // No parity, 8 bits
    USART2->CR2 &= ~USART_CR2_STOP;  // 1 stop bit


    USART2->BRR = (8 << 4) | 11; 
    USART2->CR1 |= USART_CR1_UE | USART_CR1_TE;
}

int __io_putchar(int ch) {
    while ( !(USART2->SR & USART_SR_TXE) );
    USART2->DR = ch;
    while (!(USART2->SR & USART_SR_TC)); 
    return ch;
}