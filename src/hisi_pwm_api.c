#include "hisi_pwm_api.h"
#include <asm/io.h>




#define HIPWM_WRITE(addr, value)   ((*(volatile u32*)(addr)) = (value))
#define HIPWM_READ(addr)           (*((volatile u32*)(addr)))



static void *pwm_reg_base = NULL;
static void *pwm_reg_iocfg1_base = NULL;
static void *reg_peri_crg111 = NULL;






u32 hipwm_read_reg(u32 ch, u32 off)
{
    return HIPWM_READ(pwm_reg_base + (0x20 * ch) + off);
}

void hipwm_write_reg(u32 ch, u32 off, u32 value)
{
    HIPWM_WRITE(pwm_reg_base + (0x20 * ch) + off, value);
}

void hipwm_setbits(u32 ch, u32 off, u32 mask, u32 value)
{
    u32 read = 0;

    read = hipwm_read_reg(ch, off);

    read &= ~mask;
    read |= value & mask;
    hipwm_write_reg(ch, off, read);
}

static int cfg_io(u32 ch)
{
    if (0 == ch)
        HIPWM_WRITE(pwm_reg_iocfg1_base + 0x24, 0x411);
    else if (1 == ch)
        HIPWM_WRITE(pwm_reg_iocfg1_base + 0x28, 0x411);
    else
        return -1;
    return 0;
}


/* 使能通道 */
void hipwm_enable(u32 ch, int enable)
{
    /* 关闭pwm的时候也要配置io，保证上电后的初次关闭操作让引脚处于pwm模式，即关闭时的电平高低受inverse选项控制 */
    cfg_io(ch);

    if (enable)
        hipwm_setbits(ch, PWM_REG_CTRL, PWM_ENABLE, PWM_ENABLE);
    else
        hipwm_setbits(ch, PWM_REG_CTRL, PWM_ENABLE, 0);
}

/* 设置频率和占空比，单位Hz，范围1~100000。单位1/255，范围[0-255] */
int hipwm_set_time(u32 ch, u32 f, u32 d, u8 inverse)
{
    if (1 < ch)
        return -1;
    if ((0 == f) || (100000 < f))
        return -1;
    if (255 < d)
        return -1;

    hipwm_write_reg(ch, PWM_REG_CFG0, (u32)(0x16e3600 / f));  /* 24M */
    hipwm_write_reg(ch, PWM_REG_CFG1, (u32)(((0x16e3600 / f) * d) / 255));
    if (inverse)
        hipwm_setbits(ch, PWM_REG_CTRL, PWM_INV, PWM_INV);
    else
        hipwm_setbits(ch, PWM_REG_CTRL, PWM_INV, 0);

    /* 关闭时要先打开一次，为了保证上电后首次关闭操作能让引脚电平根据inverse选项来处于相应的关闭状态 */
    hipwm_enable(ch, 1);
    if (0 == d)
        hipwm_enable(ch, 0);

    return 0;
}

static void set_clock(void)
{
    u32 read;

    read = HIPWM_READ(reg_peri_crg111);
    read |= 0x1 << 9;  /* 24MHz */
    HIPWM_WRITE(reg_peri_crg111, read);
}

int hipwm_init(void)
{
    pwm_reg_base = ioremap(PWM0_REG_BASE, 0x40);
    if (NULL == pwm_reg_base)
        return -1;

    pwm_reg_iocfg1_base = ioremap(0x111f0000, 0x40);
    if (NULL == pwm_reg_iocfg1_base)
        return -1;

    reg_peri_crg111 = ioremap(0x120101bc, 0x4);
    if (NULL == reg_peri_crg111)
        return -1;

    set_clock();

    /* 默认配置 */
    hipwm_write_reg(0, PWM_REG_CTRL, PWM_KEEP); /* 一直输出方波 */
    hipwm_write_reg(1, PWM_REG_CTRL, PWM_KEEP); /* 一直输出方波 */

    return 0;
}

void hipwm_deinit(void)
{
    iounmap(pwm_reg_base);
    iounmap(pwm_reg_iocfg1_base);
    iounmap(reg_peri_crg111);
    pwm_reg_base = NULL;
    pwm_reg_iocfg1_base = NULL;
    reg_peri_crg111 = NULL;
}
