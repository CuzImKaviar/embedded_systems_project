#include "adc.h"

void ADC_init(void){
    RCC->APB2ENR |= RCC_APB2ENR_ADC1EN; 
    GPIOA->MODER |= GPIO_MODER_MODER0; 
    ADC1->CR |= ADC_CR_ADEN;
    while(!(ADC1->ISR & ADC_ISR_ADRDY));
    ADC1->CHSELR = ADC_CHSELR_CHSEL0;
}

int ADC_read(void){
    ADC1->CHSELR = 0;
    ADC1->CHSELR |= ADC_CHSELR_CHSEL0;
    ADC1->CR |= ADC_CR_ADSTART;
    while(!(ADC1->ISR & ADC_ISR_EOC));
    return ADC1->DR;
}