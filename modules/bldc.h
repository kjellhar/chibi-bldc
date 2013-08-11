/* bldc.h
 *
 *  Created on: Jul 22, 2013
 *      Author: kjell
 */

#ifndef BLDC_H_
#define BLDC_H_

#define BLDC_COMM_STACK_SIZE    1024

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
#define PWM_PERIOD            1000
#define PWM_FREQ              PWM_CLOCK_FREQ/PWM_PERIOD


#define PWM_OUT_PORT_MASK   0x3F
#define PWM_OUT_PORT        GPIOE
#define PWM_OUT_OFFSET      8


#define PWM_PULSE0_CH       0

#define TIME1_LIMIT         100


extern Semaphore semPwmCounterReset;
extern Semaphore semPwmCh0Compare;


extern void startBldc(void);
extern void stopBldc(void);
extern void bldcStateFwd(void);
extern void bldcStateRev(void);
extern void bldcSetDutyCycle(uint32_t dutyCycle);

#endif /* BLDC_H_ */
