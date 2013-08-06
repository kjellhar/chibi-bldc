/*
 * pwm_gen.c
 *
 *  Created on: Jul 22, 2013
 *      Author: kjell
 */


#include "ch.h"
#include "hal.h"
#include "pwm.h"

#include "bldc.h"

Semaphore semPwmZeroSense;
Semaphore semPwmCounterReset;

/*
 * PWM Scheme:  High PWM, Lo ON
 * Time0 Time1
 */
static uint8_t pwmScheme[6][2] = {{PWM_UP|PWM_VN, PWM_VN},  //UP PWM, VN ON
                                  {PWM_UP|PWM_WN, PWM_WN},  //UP PWM, WN ON
                                  {PWM_VP|PWM_WN, PWM_WN},  //VP PWM, WN ON
                                  {PWM_VP|PWM_UN, PWM_UN},  //VP PWM, UN ON
                                  {PWM_WP|PWM_UN, PWM_UN},  //WP PWM, UN ON
                                  {PWM_WP|PWM_VN, PWM_VN}   //WP PWM, VN ON
};


uint32_t stateCount;
uint32_t dutyCycle;
uint32_t outputState;
uint32_t time1;

//uint8_t* pwmScheme[][2];

/* The PWM Channel compare will put the PWM system in "IDLE" state, which
 * is defined as the state when the channel is active and a compare event
 * has taken place.
 */
static void cbPwmPulse1(PWMDriver *pwmp) {
  (void) pwmp;
  chSysLockFromIsr();
  palWriteGroup (PWM_OUT_PORT, PWM_OUT_PORT_MASK, 0,  pwmScheme[outputState][1]);
  chSysUnlockFromIsr();
}

static void cbPwmZeroSense(PWMDriver *pwmp) {
  (void) pwmp;
  /* Wakes up all the threads waiting on the semaphore.*/
  chSysLockFromIsr();
  chSemResetI(&semPwmZeroSense, 0);
  chSysUnlockFromIsr();
}

/* The PWM Counter Reset will put the PWM system in "ACTIVE" state, which
 * is defined as the state when the channel is active and a compare event
 * has not yet taken place.
 */
static void cdPwmCounterReset(PWMDriver *pwmp) {
  (void) pwmp;

  chSysLockFromIsr();
  palWriteGroup (PWM_OUT_PORT, PWM_OUT_PORT_MASK, 0, pwmScheme[outputState][0]);
  chSemResetI(&semPwmCounterReset, 0);
  chSysUnlockFromIsr();
}


static PWMConfig pwmcfg = {
                           PWM_CLOCK_FREQ,
                           PWM_PERIOD,
                           &cdPwmCounterReset,
                           {
                            {PWM_OUTPUT_ACTIVE_HIGH, &cbPwmPulse1},
                            {PWM_OUTPUT_DISABLED, NULL},
                            {PWM_OUTPUT_DISABLED, NULL},
                            {PWM_OUTPUT_ACTIVE_HIGH, &cbPwmZeroSense}
                           },
                           0,
};



static WORKING_AREA(waPwmPrepareNext, PWM_STACK_SIZE);
static msg_t tPwmPrepareNext(void *arg) {
  (void)arg;
  chRegSetThreadName("PwmPrepareNext");


  while (TRUE) {
    time1 = PWM_PERCENTAGE_TO_WIDTH(&PWMD1, dutyCycle);
    pwmEnableChannel (&PWMD1, PWM_PULSE1_CH, time1);

  }
  return 0;
}


/* This function will start the PWM generator.
 */
extern void startPwmGen(uint32_t freq, uint32_t per) {
  chSemInit (&semPwmZeroSense, 0);
  chSemInit (&semPwmCounterReset, 0);

  pwmcfg.frequency = freq;
  pwmcfg.period = per;

  dutyCycle = 1000;         //Default to 10% duty cycle
  outputState = 0;          //Default to first state

  stateCount = sizeof(pwmScheme)/2;
  // pwmScheme = pwmSchemeHPLO;

  chThdCreateStatic(waPwmPrepareNext,
                    sizeof(waPwmPrepareNext),
                    NORMALPRIO,
                    tPwmPrepareNext,
                    NULL);

  pwmStart(&PWMD1, &pwmcfg);
  //pwmEnableChannel (&PWMD1, PWM_ZSENSE_CH, PWM_PERCENTAGE_TO_WIDTH(&PWMD1, 9000));


}

extern void pwmSetDutyCycle(uint32_t dc) {
  dutyCycle = dc;
}
