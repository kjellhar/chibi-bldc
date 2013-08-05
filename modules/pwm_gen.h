/*
 * pwm_gen.h
 *
 *  Created on: Jul 22, 2013
 *      Author: kjell
 */

#ifndef PWM_GEN_H_
#define PWM_GEN_H_

#define PWM_STACK_SIZE      1024

#define PWM_CLOCK_FREQ        10000000
#define PWM_PERIOD            10000


#define PWM_OUT_PORT_MASK   0x3F
#define PWM_OUT_PORT        GPIOF
#define PWM_OUT_UP          GPIOF_PIN0
#define PWM_OUT_UN          GPIOF_PIN1
#define PWM_OUT_VP          GPIOF_PIN2
#define PWM_OUT_VN          GPIOF_PIN3
#define PWM_OUT_WP          GPIOF_PIN4
#define PWM_OUT_WN          GPIOF_PIN5



#define PWM_PULSE1_CH       0
#define PWM_PULSE2_CH       1
#define PWM_PREP_NEXT_CH    3
#define PWM_ZSENSE_CH       4

#define TIME1_LIMIT         100


/*
 * Defines the states for different signals in the commutation scheme
 * The OFF state indicates that both H and L output is off the entire
 * PWM period.
 * H_ON and L_ON turns on the H or L output the entire period, and
 * implicit states that the other output must be off for the entire period.
 * The H_PULSE and L_PULSE turn on its respective signal for as long
 * as the pulse width is set. The other signal must be off for the entire
 * period.
 */
#define PWM_COMM_OFF            0
#define PWM_COMM_H_ON           10
#define PWM_COMM_L_ON           11
#define PWM_COMM_H1_PULSE       20
#define PWM_COMM_H2_PULSE       21
#define PWM_COMM_L1_PULSE       30
#define PWM_COMM_L2_PULSE       31



extern Semaphore semPwmCounterReset;
extern Semaphore semPwmChannelCompare;


extern void startPwmGen(uint32_t freq, uint32_t per);

#endif /* PWM_GEN_H_ */
