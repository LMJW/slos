#include <stdlib.h>
#include <stdint-gcc.h>
#include <timer.h>
#include <gic.h>
#include <xparameters_ps.h>
#include <regops.h>
#include <xil_printf.h>
#include <rbtree.h>
#include <ktimer.h>
#include <defs.h>

/*static uint32_t tc;*/
static uint32_t tc;
static uint32_t ticks_per_sec;
extern struct timer_struct *sched_timer;
extern struct timer_root *ptroot;

static void delay(uint32_t ticks)
{
	volatile uint32_t cnt;
	uint32_t init_cnt;
	uint32_t timeout = 0;

	cnt = timer_get_phy_tick_cnt();
	init_cnt = cnt;

	if (init_cnt < ticks) {
		while (init_cnt >= cnt)
			cnt = timer_get_phy_tick_cnt();
		timeout = ticks - init_cnt;
		init_cnt = cnt;
		while (init_cnt - cnt <= timeout) 
			cnt = timer_get_phy_tick_cnt();
	} else {
		timeout = init_cnt - ticks;
		while (init_cnt - cnt <= timeout) 
			cnt = timer_get_phy_tick_cnt();
	}

}

void mdelay(unsigned msecs)
{
	uint64_t ticks;
	ticks = ((uint64_t)msecs * ticks_per_sec)/1000;
	delay(ticks);
}

void udelay(unsigned usecs)
{
	uint64_t ticks;
	ticks = ((uint64_t)usecs * ticks_per_sec)/1000000;
	delay(ticks);
}

inline uint32_t get_timer_freq()
{
	return (XPAR_CPU_CORTEXA9_0_CPU_CLK_FREQ_HZ / 2);
}

inline uint32_t timer_get_phy_tick_cnt(void)
{
	return (uint32_t)readl(PRIV_TMR_CNTR);
}

void timer_enable()
{
	int ctrl;

	ctrl = readl(PRIV_TMR_CTRL);
	ctrl = ctrl | (PRIV_TMR_EN_MASK 
			| PRIV_TMR_AUTO_RE_MASK
			| PRIV_TMR_IRQ_EN_MASK);
		
	writel(ctrl, PRIV_TMR_CTRL);
}

void timer_disable()
{
	int ctrl;

	ctrl = readl(PRIV_TMR_CTRL);
	ctrl = ctrl & ~PRIV_TMR_EN_MASK;
		
	writel(ctrl, PRIV_TMR_CTRL);
}

int timer_irq (void *arg)
{
	struct timer_struct *pnt, *pct; 
	uint32_t elapsed;

	elapsed = get_elapsedtime();
	pct = container_of(ptroot->rb_leftmost, struct timer_struct, run_node);
	update_timer_tree(elapsed);
	pnt = container_of(ptroot->rb_leftmost, struct timer_struct, run_node);
	tc = pnt->tc;
	/* reprogram next earliest deadline timer intr */
	writel(tc, PRIV_TMR_LD);

	switch(pct->type) {
		case SCHED_TIMER:
			sched_timer->handler(elapsed);
			break;

		case REALTIME_TIMER:
			pct->handler(elapsed);
			break;

		case ONESHOT_TIMER:
			break;

		default:
			break;
	}

	return 0;
}

uint32_t get_ticks_per_sec()
{
	return ticks_per_sec;
}

#define RT_TIMER_NUM		3

void rt_timer1(uint32_t el)
{
	xil_printf("rt timer 1!!\n");
}

void rt_timer2(uint32_t el)
{
	xil_printf("rt timer 2!!\n");
}

void rt_timer3(uint32_t el)
{
	xil_printf("rt timer 3!!\n");
}

timer_handler rt_timer_handler[RT_TIMER_NUM] = {
	rt_timer1,
	rt_timer2,
	rt_timer3,
};

void create_rt_timers()
{
	int i;
	for (i = 0; i < RT_TIMER_NUM; i++) {
		create_rt_timer(rt_timer_handler[i], 200 + 20 * i, i + 1, NULL);
	}
}

void timer_init()
{
	struct timer_struct *pct;
	ticks_per_sec = get_timer_freq();
	timertree_init();
	create_sched_timer(sched_timer_handler, 1000, 0, NULL);
	create_rt_timers();

	/*tc = sched_timer->tc;*/
	pct = container_of(ptroot->rb_leftmost, struct timer_struct, run_node);
	tc = pct->tc;
	writel(tc, PRIV_TMR_LD);
	gic_register_int_handler(PRIV_TMR_INT_VEC, timer_irq, NULL);
	gic_mask_interrupt(PRIV_TMR_INT_VEC);
	timer_enable();
}

