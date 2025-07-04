#include "delay.h" // Device header

void delay_us(uint16_t us)
{
    /* 定时器重装值 */
    SysTick->LOAD = 72 * us;
    /* 清除当前计数值 */
    SysTick->VAL = 0;
    /*设置内部时钟源（2位->1）,不需要中断（1位->0），并启动定时器(0位->1)*/
    SysTick->CTRL = 0x5;
    /*等待计数到0， 如果计数到0则16位会置为1*/
    while (!(SysTick->CTRL & SysTick_CTRL_COUNTFLAG));
	SysTick->CTRL &= ~SysTick_CTRL_COUNTFLAG;  // 清除标志
    /* 关闭定时器 */
    SysTick->CTRL &= ~SysTick_CTRL_ENABLE; 
}

void delay_ms(uint16_t ms)
{
    while (ms--)
    {
		/* 临界区保护：关闭全局中断 */
		__disable_irq();
        delay_us(1000);
		/* 恢复中断 */
		__enable_irq();
    }
}

void delay_s(uint16_t s)
{
    while (s--)
    {
        delay_ms(1000);
    }
}
