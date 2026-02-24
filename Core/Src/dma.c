#include "dma.h"
#include "debug.h"

void DMA_TIM3_CH1_Init(uint16_t* buffer, uint16_t size) {
    // Enable DMA1 clock
    RCC->AHB1ENR |= RCC_AHB1ENR_DMA1EN;

    // Disable stream before config
    DMA1_Stream4->CR &= ~DMA_SxCR_EN;
    while (DMA1_Stream4->CR & DMA_SxCR_EN);

    // Channel 5
    DMA1_Stream4->CR &= ~DMA_SxCR_CHSEL;
    DMA1_Stream4->CR |= (5 << DMA_SxCR_CHSEL_Pos);

    // 16-bit memory and peripheral
    DMA1_Stream4->CR &= ~DMA_SxCR_MSIZE;
    DMA1_Stream4->CR |= (1 << DMA_SxCR_MSIZE_Pos);
    DMA1_Stream4->CR &= ~DMA_SxCR_PSIZE;
    DMA1_Stream4->CR |= (1 << DMA_SxCR_PSIZE_Pos);

    // Memory increment ON, peipheral increment OFF
    DMA1_Stream4->CR |= DMA_SxCR_MINC;
    DMA1_Stream4->CR &= ~DMA_SxCR_PINC;

    // Peripheral-to-memory
    DMA1_Stream4->CR &= ~DMA_SxCR_DIR; 

    // Transfer complete interrupt
    DMA1_Stream4->CR |= DMA_SxCR_TCIE;

    // Set addresses and count
    DMA1_Stream4->NDTR = size;
    DMA1_Stream4->PAR = (uint32_t)&TIM3->CCR1;
    DMA1_Stream4->M0AR = (uint32_t)buffer;

    // Explicitly disable FIFO (use direct mode)
    DMA1_Stream4->FCR &= ~DMA_SxFCR_DMDIS;

    // Clear ALL flags for stream 4 before enabling
    DMA1->HIFCR = DMA_HIFCR_CTCIF4 | DMA_HIFCR_CHTIF4 | 
                    DMA_HIFCR_CTEIF4 | DMA_HIFCR_CDMEIF4 | DMA_HIFCR_CFEIF4;

    // Enable
    DMA1_Stream4->CR |= DMA_SxCR_EN;

    // Enable interrupt
    NVIC_EnableIRQ(DMA1_Stream4_IRQn);

    // Enable timer DMA request
    TIM3->DIER |= TIM_DIER_CC1DE;
}