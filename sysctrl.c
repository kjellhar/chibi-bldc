/*
 * sysctrl.c
 *
 *  Created on: Jul 22, 2013
 *      Author: kjell
 */

#include "ch.h"
#include "hal.h"

#include "sysctrl.h"
#include "commands.h"

#include "usbcfg.h"

#include "bldc.h"


cmdPkg cmdMemPoolBuffer[CMD_MEM_POOL_SIZE];
MemoryPool cmdMemPool;

uint8_t* cmdOutMailboxBuffer[CMD_MAILBOX_SIZE];
uint8_t* cmdInMailboxBuffer[CMD_MAILBOX_SIZE];

Mailbox cmdInMailbox;
Mailbox cmdOutMailbox;



static WORKING_AREA(waSysCtrlCmdParser, SYS_CTRL_STACK_SIZE);
static msg_t tSysCtrlCmdParser(void *arg) {
  (void)arg;
  chRegSetThreadName("SysCtrlCmdParser");

  msg_t msg;
  cmdPkg *cmdBufp;

  while (TRUE) {
    chMBFetch (&cmdInMailbox, &msg, TIME_INFINITE);

    cmdBufp=(cmdPkg *)msg;

    switch (cmdBufp->cmd1) {
    case C1_SYS:

      switch (cmdBufp->cmd2) {
      case C2_SYS_LED1_ON:
        palSetPad(GPIOD, GPIOD_LED3);
        break;
      case C2_SYS_LED1_OFF:
        palClearPad(GPIOD, GPIOD_LED3);
        break;
      case C2_SYS_LED2_ON:
        palSetPad(GPIOD, GPIOD_LED4);
        break;
      case C2_SYS_LED2_OFF:
        palClearPad(GPIOD, GPIOD_LED4);
        break;
      case C2_SYS_LED3_ON:
        palSetPad(GPIOD, GPIOD_LED5);
        break;
      case C2_SYS_LED3_OFF:
        palClearPad(GPIOD, GPIOD_LED5);
        break;
      case C2_SYS_LED4_ON:
        palSetPad(GPIOD, GPIOD_LED6);
        break;
      case C2_SYS_LED4_OFF:
        palClearPad(GPIOD, GPIOD_LED6);
        break;
      }

      break;
    case C1_PWM:
      switch (cmdBufp->cmd2) {
      case C2_PWM_START:
        startPwmGen(PWM_CLOCK_FREQ, PWM_PERIOD);
        break;

      default:
        break;
      }

      break;
    default:
      break;
    }

    chPoolFree (&cmdMemPool, cmdBufp);

    cmdBufp = chPoolAlloc (&cmdMemPool);
    cmdBufp->cmd1 = 0xFF;
    cmdBufp->pkgSize = 6;
    cmdBufp->payload[0] = 'O';
    cmdBufp->payload[1] = 'K';

    chMBPost (&cmdOutMailbox, (msg_t)cmdBufp, TIME_INFINITE);

  }
  return 0;
}


void startSysCtrl(void) {
  chPoolInit (&cmdMemPool, sizeof(cmdPkg), NULL);
  chPoolLoadArray(&cmdMemPool, &cmdMemPoolBuffer, CMD_MEM_POOL_SIZE);

  chMBInit (&cmdOutMailbox, (msg_t *)cmdOutMailboxBuffer, CMD_MAILBOX_SIZE);
  chMBInit (&cmdInMailbox, (msg_t *)cmdInMailboxBuffer, CMD_MAILBOX_SIZE);

  chThdCreateStatic(waSysCtrlCmdParser,
                    sizeof(waSysCtrlCmdParser),
                    NORMALPRIO,
                    tSysCtrlCmdParser,
                    NULL);


  /* Start system peripherals such as communication modules and other
   * modules not directly related to the BLDC
   */

  startUsbControl();
}



