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

BldcConfig  bldc;

/* The PWM Counter Reset will put the PWM system in "ACTIVE" state, which
 * is defined as the state when the channel is active and a compare event
 * has not yet taken place.
 */
static void cdPwmCounterReset(PWMDriver *pwmp) {
  (void) pwmp;

  bldc.state = bldc.nextState;
  chSysLockFromIsr();
  palWriteGroup (PWM_OUT_PORT, PWM_OUT_PORT_MASK, PWM_OUT_OFFSET, (*bldc.scheme)[bldc.state][0]);
  chSysUnlockFromIsr();


  // Calculate and initiate the state change
  if (!halIsCounterWithin(bldc.prevStateChange, bldc.nextStateChange)) {

    // Prepare next state
    if (bldc.directionFwd) {
      bldc.nextState++;
    }
    else {
      bldc.nextState--;
    }

    // Wrap the state counter
    bldc.nextState = (bldc.nextState+bldc.stateCount)%bldc.stateCount;

    // Prepare the next state change.
    bldc.prevStateChange = bldc.nextStateChange;
    bldc.nextStateChange += bldc.stateChangeInterval;
  }
}


/* The PWM Channel compare will put the PWM system in "IDLE" state, which
 * is defined as the state when the channel is active and a compare event
 * has taken place.
 */
static void cbPwmCh0Compare(PWMDriver *pwmp) {
  (void) pwmp;

  chSysLockFromIsr();
  palWriteGroup (PWM_OUT_PORT, PWM_OUT_PORT_MASK, PWM_OUT_OFFSET,  (*bldc.scheme)[bldc.state][1]);
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


/* This function will start the PWM generator.
 */
extern void startBldc(void) {
  bldc.scheme = &pwmScheme;

  bldc.state = 0;          //Default to first state
  bldc.directionFwd = TRUE;

  bldc.stateChangeInterval = US2RTT(160);
  bldc.prevStateChange = halGetCounterValue();
  bldc.nextStateChange = bldc.prevStateChange + bldc.stateChangeInterval;

  bldc.stateCount = sizeof(pwmScheme)/2;

  palWriteGroup (PWM_OUT_PORT, PWM_OUT_PORT_MASK, PWM_OUT_OFFSET,  PWM_OFF);
  palSetGroupMode (PWM_OUT_PORT, PWM_OUT_PORT_MASK, PWM_OUT_OFFSET, PAL_MODE_OUTPUT_PUSHPULL);

  pwmStart(&PWMD1, &pwmcfg);
  pwmEnableChannel (&PWMD1, PWM_PULSE0_CH, PWM_PERCENTAGE_TO_WIDTH(&PWMD1, 1000));  //Default to 10% duty cycle
}

extern void stopBldc(void) {
  palWriteGroup (PWM_OUT_PORT, PWM_OUT_PORT_MASK, PWM_OUT_OFFSET,  PWM_OFF);
  pwmStop(&PWMD1);
}

extern void bldcSetDutyCycle(uint32_t dutyCycle) {
  uint32_t pulseTime;

  pulseTime = PWM_PERCENTAGE_TO_WIDTH(&PWMD1, dutyCycle);
  pwmEnableChannel (&PWMD1, PWM_PULSE0_CH, pulseTime);
}
