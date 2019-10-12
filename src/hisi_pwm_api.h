/*
*  HI3516DV300 PWM REG
*/

#ifndef __HISI_PWM_API_H__
#define __HISI_PWM_API_H__


#include <linux/types.h>




extern void hipwm_deinit(void);
extern int  hipwm_init(void);
extern int  hipwm_set_time(u32 ch, u32 f, u32 d, u8 inverse);
extern void hipwm_enable(u32 ch, int enable);




#define PWM0_REG_BASE             0x12070000
#define PWM1_REG_BASE             0x12070020

#define PWM_REG_CFG0                  0x0000  /* 周期 */
#define PWM_REG_CFG1                  0x0004  /* 高电平时长，配置为0无法关闭输出，会有一个小脉冲 */
#define PWM_REG_CFG2                  0x0008
#define PWM_REG_CTRL                  0x000c
#define PWM_REG_STATE0                0x0010
#define PWM_REG_STATE1                0x0014
#define PWM_REG_STATE2                0x0018



//PWM_REG_CFG2
#define PWM_NUM_SHIFT                  0
#define LSADC_DATA_DELTA_MASK          0x3ff         /* 输出方波个数 */


//PWM_REG_CTRL
#define PWM_KEEP                      (0x1 << 2)  /* PWM输出模式 */
#define PWM_INV                       (0x1 << 1)  /* PWM输出反相 */
#define PWM_ENABLE                    (0x1 << 0)  /* PWM使能控制 */




#endif
