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
static uint8_t pwmScheme[6][3] = {{PWM_UP|PWM_VN, PWM_VN, !PWM_EXPECT_ZERO},    //UP PWM, VN ON
                                  {PWM_UP|PWM_WN, PWM_WN, PWM_EXPECT_ZERO},     //UP PWM, WN ON
                                  {PWM_VP|PWM_WN, PWM_WN, !PWM_EXPECT_ZERO},    //VP PWM, WN ON
                                  {PWM_VP|PWM_UN, PWM_UN, PWM_EXPECT_ZERO},     //VP PWM, UN ON
                                  {PWM_WP|PWM_UN, PWM_UN, !PWM_EXPECT_ZERO},    //WP PWM, UN ON
                                  {PWM_WP|PWM_VN, PWM_VN, PWM_EXPECT_ZERO}      //WP PWM, VN ON
};

BldcConfig  bldc;

/* The PWM Counter Reset will put the PWM system in "ACTIVE" state, which
 * is defined as the state when the channel is active and a compare event
 * has not yet taken place.
 */
static void cdPwmCounterReset(PWMDriver *pwmp) {
  (void) pwmp;

  chSysLockFromIsr();
  palWriteGroup (PWM_OUT_PORT, PWM_OUT_PORT_MASK, PWM_OUT_OFFSET,  bldc.pwmOutT0);
  chSysUnlockFromIsr();


  // Calculate and initiate the state change
  // Consider moving this to a thread to further slim down the ISR callback
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
  palWriteGroup (PWM_OUT_PORT, PWM_OUT_PORT_MASK, PWM_OUT_OFFSET,  bldc.pwmOutT1);
  chSysUnlockFromIsr();

  // Do the state change before the next cycle.
  // Consider moving this to a thread to further slim down the ISR callback
  bldc.state = bldc.nextState;
  bldc.pwmOutT0 = (*bldc.scheme)[bldc.state][0];
  bldc.pwmOutT1 = (*bldc.scheme)[bldc.state][1];
}


static void pcbPwmAdcTrigger(PWMDriver *pwmp) {
  (void) pwmp;

}

static PWMConfig pwmcfg = {
                           PWM_CLOCK_FREQ,
                           PWM_PERIOD,
                           &cdPwmCounterReset,
                           {
                            {PWM_OUTPUT_ACTIVE_HIGH, &cbPwmCh0Compare},
                            {PWM_OUTPUT_DISABLED, NULL},
                            {PWM_OUTPUT_DISABLED, NULL},
                            {PWM_OUTPUT_ACTIVE_HIGH, &pcbPwmAdcTrigger}
                           },
                           0,
};


/* This function will start the PWM generator.
 */
extern void startBldc(void) {
  bldc.scheme = &pwmScheme;
  bldc.state = 0;          //Default to first state
  bldc.nextState = 0;
  bldc.directionFwd = TRUE;
  bldc.stateChangeInterval = US2RTT(160);
  bldc.prevStateChange = halGetCounterValue();
  bldc.nextStateChange = bldc.prevStateChange + bldc.stateChangeInterval;
  bldc.pwmOutT0 = 0;
  bldc.pwmOutT1 = 0;
  bldc.stateCount = sizeof(pwmScheme)/3;

  palWriteGroup (PWM_OUT_PORT, PWM_OUT_PORT_MASK, PWM_OUT_OFFSET,  PWM_OFF);
  palSetGroupMode (PWM_OUT_PORT, PWM_OUT_PORT_MASK, PWM_OUT_OFFSET, PAL_MODE_OUTPUT_PUSHPULL);

  pwmStart(&PWMD1, &pwmcfg);

  // The PWM output generator channel. Defaults to 10% at startup
  pwmEnableChannel (&PWMD1, PWM_PULSE0_CH, PWM_PERCENTAGE_TO_WIDTH(&PWMD1, 1000));

  // ADC trigger channel. This will trigger the ADC read at 95% of the cycle,
  // when all the PWM outputs are set to 0
  pwmEnableChannel (&PWMD1, PWM_ADCTRIG_CH, PWM_PERCENTAGE_TO_WIDTH(&PWMD1, PWM_MAX_DUTY_CYCLE));
}

extern void stopBldc(void) {
  palWriteGroup (PWM_OUT_PORT, PWM_OUT_PORT_MASK, PWM_OUT_OFFSET,  PWM_OFF);
  pwmStop(&PWMD1);
}


// This function should probably be replaced by a Mailbox and a thread.
extern void bldcSetDutyCycle(uint32_t dutyCycle) {
  uint32_t pulseTime;

  if (dutyCycle > PWM_MAX_DUTY_CYCLE) {
    dutyCycle = PWM_MAX_DUTY_CYCLE;
  }

  pulseTime = PWM_PERCENTAGE_TO_WIDTH(&PWMD1, dutyCycle);
  pwmEnableChannel (&PWMD1, PWM_PULSE0_CH, pulseTime);
}
