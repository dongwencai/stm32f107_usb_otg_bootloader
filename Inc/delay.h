#ifndef __DELAY_H__
#define __DELAY_H__
static __ASM void __INLINE delay_us(uint32_t volatile number_of_us)
{
loop
		SUBS	R0, R0, #1
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		NOP
		BNE    loop
		BX	   LR
}

static void __INLINE delay_ms(uint32_t ms)
{
	int i;
	for(i = 0 ; i < ms; i ++)
		delay_us(1000);
}

#endif
