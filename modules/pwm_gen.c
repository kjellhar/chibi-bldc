/*
 * pwm_gen.c
 *
 *  Created on: Jul 22, 2013
 *      Author: kjell
 */


#include "ch.h"
#include "hal.h"
#include "pwm.h"

#include "pwm_gen.h"

Semaphore semPwmZeroSense;
Semaphore semPwmPrepareNext;
Semaphore semPwmCounterReset;


/*
 * PWM Scheme:  High PWM, Lo ON
 * U V L
 */
static uint8_t pwmSchemeHPLO[6][3] = {{PWM_COMM_H2_PULSE, PWM_COMM_L_ON, PWM_COMM_OFF},
                                      {PWM_COMM_H2_PULSE, PWM_COMM_OFF, PWM_COMM_L_ON},
                                      {PWM_COMM_OFF, PWM_COMM_H2_PULSE, PWM_COMM_L_ON},
                                      {PWM_COMM_L_ON, PWM_COMM_H2_PULSE, PWM_COMM_OFF},
                                      {PWM_COMM_L_ON, PWM_COMM_OFF, PWM_COMM_H2_PULSE},
                                      {PWM_COMM_OFF, PWM_COMM_L_ON, PWM_COMM_H2_PULSE}};

static uint8_t pwmSchemeHOLP[6][3] = {{PWM_COMM_H_ON, PWM_COMM_L2_PULSE, PWM_COMM_OFF},
                                      {PWM_COMM_H_ON, PWM_COMM_OFF, PWM_COMM_L2_PULSE},
                                      {PWM_COMM_OFF, PWM_COMM_H_ON, PWM_COMM_L2_PULSE},
                                      {PWM_COMM_L2_PULSE, PWM_COMM_H_ON, PWM_COMM_OFF},
                                      {PWM_COMM_L2_PULSE, PWM_COMM_OFF, PWM_COMM_H_ON},
                                      {PWM_COMM_OFF, PWM_COMM_L2_PULSE, PWM_COMM_H_ON}};


static uint32_t pwmStateCountList[] = {6,
                                       6
};

static uint8_t (*pwmSchemeList[])[][3] = {&pwmSchemeHPLO,
                                          &pwmSchemeHOLP
};


uint8_t (*scheme)[][3];
uint32_t stateCount;
uint32_t dutyCycle;
uint32_t time1Fraction;
uint32_t outputState;

uint8_t outValT0, outValT1, outValT2;
uint8_t outNextT0, outNextT1, outNextT2;

/* The PWM Channel compare will put the PWM system in "IDLE" state, which
 * is defined as the state when the channel is active and a compare event
 * has taken place.
 */
static void cbPwmPulse1(PWMDriver *pwmp) {
  (void) pwmp;
  chSysLockFromIsr();
  palWriteGroup (PWM_OUT_PORT, PWM_OUT_PORT_MASK, 0, outValT1);
  chSysUnlockFromIsr();
}

static void cbPwmPulse2(PWMDriver *pwmp) {
  (void) pwmp;
  chSysLockFromIsr();
  palWriteGroup (PWM_OUT_PORT, PWM_OUT_PORT_MASK, 0, outValT2);
  chSysUnlockFromIsr();
}

static void cbPwmPrepareNext(PWMDriver *pwmp) {
  (void) pwmp;
  chSysLockFromIsr();
  chSemResetI(&semPwmPrepareNext, 0);
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

  outValT0 = outNextT0;
  outValT1 = outNextT1;
  outValT2 = outNextT2;

  palWriteGroup (PWM_OUT_PORT, PWM_OUT_PORT_MASK, 0, outValT0);
  chSemResetI(&semPwmCounterReset, 0);
  chSysUnlockFromIsr();
}





static PWMConfig pwmcfg = {
                           PWM_CLOCK_FREQ,
                           PWM_PERIOD,
                           &cdPwmCounterReset,
                           {
                            {PWM_OUTPUT_ACTIVE_HIGH, &cbPwmPulse1},
                            {PWM_OUTPUT_ACTIVE_HIGH, &cbPwmPulse2},
                            {PWM_OUTPUT_ACTIVE_HIGH, &cbPwmPrepareNext},
                            {PWM_OUTPUT_ACTIVE_HIGH, &cbPwmZeroSense}
                           },
                           0,
};



static WORKING_AREA(waPwmPrepareNext, PWM_STACK_SIZE);
static msg_t tPwmPrepareNext(void *arg) {
  (void)arg;
  chRegSetThreadName("PwmPrepareNext");

  uint32_t time1, time2;

  uint8_t outT0, outT1, outT2;
  uint8_t c;


  while (TRUE) {
    outT0 = 0;
    outT1 = 0;
    outT2 = 0;


    for (c=0; c<3; c++) {
      switch ((*scheme)[outputState][c]) {
      case PWM_COMM_OFF:
        //Defaults to off
        break;
      case PWM_COMM_H_ON:
        outT0 |= 0x1<<(2*c);
        outT1 |= 0x1<<(2*c);
        outT2 |= 0x1<<(2*c);
        break;
      case PWM_COMM_L_ON:
        outT0 |= 0x2<<(2*c);
        outT1 |= 0x2<<(2*c);
        outT2 |= 0x2<<(2*c);
        break;
      case PWM_COMM_H1_PULSE:
        outT1 |= 0x1<<(2*c);
        break;
      case PWM_COMM_H2_PULSE:
        outT0 |= 0x1<<(2*c);
        outT1 |= 0x1<<(2*c);
        break;
      case PWM_COMM_L1_PULSE:
        outT1 |= 0x2<<(2*c);
        break;
      case PWM_COMM_L2_PULSE:
        outT0 |= 0x2<<(2*c);
        outT1 |= 0x2<<(2*c);
        break;
      default:
        // Defaults to off
        break;
      }
    }


    // Calc period for Pulse2, which is always the longest
    time2 = PWM_PERCENTAGE_TO_WIDTH(&PWMD1, dutyCycle);
    // Calc period for Pulse1
    time1 = (time2*time1Fraction)/10000;

    chSysLock();
    outNextT0 = outT0;
    outNextT1 = outT1;
    outNextT2 = outT2;
    chSysUnlock();

    pwmEnableChannel (&PWMD1, PWM_PULSE2_CH, time2);

    if (time1Fraction > TIME1_LIMIT) {
      pwmEnableChannel (&PWMD1, PWM_PULSE1_CH, time1);
    }
    else {
      pwmDisableChannel(&PWMD1, PWM_PULSE1_CH);
    }
    // Wait for mailbox

  }
  return 0;
}






/* This function will start the PWM generator.
 */
extern void startPwmGen(uint32_t freq, uint32_t per) {
  chSemInit (&semPwmPrepareNext, 0);
  chSemInit (&semPwmZeroSense, 0);
  chSemInit (&semPwmCounterReset, 0);

  pwmcfg.frequency = freq;
  pwmcfg.period = per;

  outValT0 = 0;
  outValT1 = 0;
  outValT2 = 0;

  dutyCycle = 1000;         //Default to 10% duty cycle
  time1Fraction = 0;
  outputState = 0;          //Default to first state

  scheme = pwmSchemeList[outputState];
  stateCount = pwmStateCountList[outputState];


  chThdCreateStatic(waPwmPrepareNext,
                    sizeof(waPwmPrepareNext),
                    NORMALPRIO,
                    tPwmPrepareNext,
                    NULL);


  pwmStart(&PWMD1, &pwmcfg);
  pwmEnableChannel (&PWMD1, PWM_PREP_NEXT_CH, PWM_PERCENTAGE_TO_WIDTH(&PWMD1, 9000));
  pwmEnableChannel (&PWMD1, PWM_ZSENSE_CH, PWM_PERCENTAGE_TO_WIDTH(&PWMD1, 9000));


}

extern void pwmSetDutyCycle(uint32_t dc) {
  dutyCycle = dc;
}
