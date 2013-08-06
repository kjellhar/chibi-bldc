/*
 * commands.h
 *
 *  Created on: Jul 27, 2013
 *      Author: kjell
 */

#ifndef COMMANDS_H_
#define COMMANDS_H_

#define     C1_SYS                  0x01
#define     C2_SYS_LED1_ON          0xF0
#define     C2_SYS_LED1_OFF         0xF1
#define     C2_SYS_LED2_ON          0xF2
#define     C2_SYS_LED2_OFF         0xF3
#define     C2_SYS_LED3_ON          0xF4
#define     C2_SYS_LED3_OFF         0xF5
#define     C2_SYS_LED4_ON          0xF6
#define     C2_SYS_LED4_OFF         0xF7

#define     C1_BLDC             0x40
#define     C2_BLDC_START       0x01
#define     C2_BLDC_SET_F       0x10
#define     C2_BLDC_SET_DC      0x11
#define     C2_BLDC_FWD         0x20
#define     C2_BLDC_REV         0x21

#define     C1_COMM         0x41
#define     C1_PID          0x42
#define     C1_ZSENSE       0x43
#define     C1_CSENSE       0x44




#endif /* COMMANDS_H_ */
