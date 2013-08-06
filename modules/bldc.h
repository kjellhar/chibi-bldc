/* bldc.h
 *
 *  Created on: Jul 22, 2013
 *      Author: kjell
 */

#ifndef BLDC_H_
#define BLDC_H_


/*
 * Defines the states for different signals in the commutation scheme
 */
#define PWM_OFF     0b000000
#define PWM_UP      0b000001
#define PWM_UN      0b000010
#define PWM_VP      0b000100
#define PWM_VN      0b001000
#define PWM_WP      0b010000
#define PWM_WN      0b100000



#define PWM_STACK_SIZE      1024

#define PWM_CLOCK_FREQ        10000000
#define PWM_PERIOD            10000


#define PWM_OUT_PORT_MASK   0x3F
#define PWM_OUT_PORT        GPIOF


#define PWM_PULSE1_CH       0
#define PWM_ZSENSE_CH       4

#define TIME1_LIMIT         100


extern Semaphore semPwmCounterReset;
extern Semaphore semPwmZeroSense;


extern void startPwmGen(uint32_t freq, uint32_t per);

#endif /* BLDC_H_ */
