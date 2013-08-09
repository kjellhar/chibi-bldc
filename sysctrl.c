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

    switch (cmdBufp->cmd) {
    case CMD_BLDC1_START:
      startBldc();
      break;
    case CMD_BLDC1_STOP:
      stopBldc();
      break;
    case CMD_BLDC1_DIRECTION:
      break;
    case CMD_BLDC1_PWM:
      break;
    case CMD_BLDC1_RPM:
      break;
    case CMD_LED1_ON:
      palSetPad(GPIOD, GPIOD_LED3);
      break;
    case CMD_LED1_OFF:
      palClearPad(GPIOD, GPIOD_LED3);
      break;
    case CMD_LED2_ON:
      palSetPad(GPIOD, GPIOD_LED4);
      break;
    case CMD_LED2_OFF:
      palClearPad(GPIOD, GPIOD_LED4);
      break;
    case CMD_LED3_ON:
      palSetPad(GPIOD, GPIOD_LED5);
      break;
    case CMD_LED3_OFF:
      palClearPad(GPIOD, GPIOD_LED5);
      break;
    case CMD_LED4_ON:
      palSetPad(GPIOD, GPIOD_LED6);
      break;
    case CMD_LED4_OFF:
      palClearPad(GPIOD, GPIOD_LED6);
      break;
    default:
      break;
    }

    chPoolFree (&cmdMemPool, cmdBufp);

    cmdBufp = chPoolAlloc (&cmdMemPool);
    cmdBufp->cmd = CMD_ACK;
    cmdBufp->pkgSize = 2;

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



