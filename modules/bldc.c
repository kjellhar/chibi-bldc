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

Semaphore semPwmCh0Compare;
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

/* Number of states in PWM scheme*/
uint32_t stateCount;

/* The length of the pulse in timer ticks*/
uint32_t pulseTime;

/* The current and the next state in the PWM scheme*/
uint32_t state, stateNext;

/* Time for last and next state change */
uint32_t lastStateChange, nextStateChange;

/* Electrical revolutions pr second   erps = RPM*Polecount/(2*60) */
uint32_t erps;      // Electrical revolutions pr sec * 1000



/* The PWM Counter Reset will put the PWM system in "ACTIVE" state, which
 * is defined as the state when the channel is active and a compare event
 * has not yet taken place.
 */
static void cdPwmCounterReset(PWMDriver *pwmp) {
  (void) pwmp;

  if (halGetCounterValue()-lastStateChange > nextStateChange-lastStateChange &&
      halGetCounterValue()-lastStateChange < US2RTT(STATE_CHANGE_LIMIT_US)) {
    state = stateNext;
  }

  chSysLockFromIsr();
  palWriteGroup (PWM_OUT_PORT, PWM_OUT_PORT_MASK, PWM_OUT_OFFSET, pwmScheme[state][0]);
  chSemResetI(&semPwmCounterReset, 0);
  chSysUnlockFromIsr();
}


/* The PWM Channel compare will put the PWM system in "IDLE" state, which
 * is defined as the state when the channel is active and a compare event
 * has taken place.
 */
static void cbPwmCh0Compare(PWMDriver *pwmp) {
  (void) pwmp;
  chSysLockFromIsr();
  palWriteGroup (PWM_OUT_PORT, PWM_OUT_PORT_MASK, PWM_OUT_OFFSET,  pwmScheme[state][1]);
  chSemResetI(&semPwmCh0Compare, 0);
  chSysUnlockFromIsr();
}

static PWMConfig pwmcfg = {
                           PWM_CLOCK_FREQ,
                           PWM_PERIOD,
                           &cdPwmCounterReset,
                           {
                            {PWM_OUTPUT_ACTIVE_HIGH, &cbPwmCh0Compare},
                            {PWM_OUTPUT_DISABLED, NULL},
                            {PWM_OUTPUT_DISABLED, NULL},
                            {PWM_OUTPUT_DISABLED, NULL}
                           },
                           0,
};


static WORKING_AREA(waBldcComm, BLDC_COMM_STACK_SIZE);
static msg_t tBldcComm(void *arg) {
  (void)arg;
  chRegSetThreadName("BldcComm");

  uint32_t currentAngle;     // Angle in degrees * 1000
  uint32_t next_comm;   // Angle of next commutation

  currentAngle = 0;          //Start at 0.000 deg

  while (TRUE) {
    chSemWait(&semPwmCounterReset);

  }
  return 0;
}

/* This function will start the PWM generator.
 */
extern void startBldc(void) {
  chSemInit (&semPwmCh0Compare, 0);
  chSemInit (&semPwmCounterReset, 0);

  state = 0;          //Default to first state

  stateCount = sizeof(pwmScheme)/2;

  palWriteGroup (PWM_OUT_PORT, PWM_OUT_PORT_MASK, PWM_OUT_OFFSET,  PWM_OFF);
  palSetGroupMode (PWM_OUT_PORT, PWM_OUT_PORT_MASK, PWM_OUT_OFFSET, PAL_MODE_OUTPUT_PUSHPULL);

  chThdCreateStatic(waBldcComm,
                    sizeof(waBldcComm),
                    NORMALPRIO,
                    tBldcComm,
                    NULL);

  pwmStart(&PWMD1, &pwmcfg);
  pulseTime = PWM_PERCENTAGE_TO_WIDTH(&PWMD1, 1000);   //Default to 10% duty cycle
  pwmEnableChannel (&PWMD1, PWM_PULSE0_CH, pulseTime);
}

extern void stopBldc(void) {
  palWriteGroup (PWM_OUT_PORT, PWM_OUT_PORT_MASK, PWM_OUT_OFFSET,  PWM_OFF);
  pwmStop(&PWMD1);
}

extern void bldcStateFwd(void) {
  if (state == stateCount-1) {
    stateNext = 0;
  }
  else {
    stateNext = state+1;
  }
}

extern void bldcStateRev(void) {
  if (state == 0) {
    stateNext = stateCount-1;
  }
  else {
    stateNext = state-1;
  }
}

extern void bldcSetDutyCycle(uint32_t dutyCycle) {
  pulseTime = PWM_PERCENTAGE_TO_WIDTH(&PWMD1, dutyCycle);
  pwmEnableChannel (&PWMD1, PWM_PULSE0_CH, pulseTime);
}
