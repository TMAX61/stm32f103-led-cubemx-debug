#include <stm32f10x.h>
#include <Delay.h>
int main(void)
{
	RCC_APB2PeriphClockCmd(RCC_APB2Periph_GPIOA, ENABLE );\
	
	GPIO_InitTypeDef GPIO_Initstructure;
	GPIO_Initstructure.GPIO_Mode = GPIO_Mode_Out_PP;
	GPIO_Initstructure.GPIO_Pin = GPIO_Pin_0;
	GPIO_Initstructure.GPIO_Speed = GPIO_Speed_50MHz;
	GPIO_Init (GPIOA,&GPIO_Initstructure );
 	
	while (1)
	{
		GPIO_WriteBit(GPIOA,GPIO_Pin_0, (BitAction)1);
		Delay_ms(200);
		GPIO_WriteBit(GPIOA,GPIO_Pin_0, (BitAction)0);
		Delay_ms(800);
	}
}
