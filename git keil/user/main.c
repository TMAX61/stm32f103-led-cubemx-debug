#include <stm32f10x.h>
#include <Delay.h>
volatile int a=0;
	volatile int b=1;
	volatile int c=2;
int main(void)
{	
	while(1)
	{
		a=b*c;
		c++;
		Delay_ms (100);
	}
}
