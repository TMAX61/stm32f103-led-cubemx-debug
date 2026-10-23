#include <stm32f10x.h>

void LED_Init(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE );
	
	GPIO_InitTypeDef GPIO_Initstructure;
	GPIO_Initstructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Initstructure.GPIO_Pin = GPIO_Pin_1|GPIO_Pin_2;
	GPIO_Initstructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init (GPIOA,&GPIO_Initstructure );
}

void LED_1ON(void)
{
	GPIO_ResetBits (GPIOA ,GPIO_Pin_1 );
}

void LED_2ON(void)
{
	GPIO_ResetBits (GPIOA ,GPIO_Pin_2 );
}

void LED_1OFF(void)
{
    GPIO_SetBits (GPIOA,GPIO_Pin_1 );
}

void LED_2OFF(void)
{
    GPIO_SetBits (GPIOA,GPIO_Pin_2 );
}

void LED_1Turn(void)
{
	if(GPIO_ReadOutputDataBit(GPIOA ,GPIO_Pin_1 ) == 0) 
	{
		GPIO_SetBits (GPIOA ,GPIO_Pin_1 );
	}
	else {
		GPIO_ResetBits (GPIOA ,GPIO_Pin_1 );
	
	}
}
