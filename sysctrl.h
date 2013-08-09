/*
 * sysctrl.h
 *
 *  Created on: Jul 11, 2013
 *      Author: kjell
 */

#ifndef SYSCTRL_H_
#define SYSCTRL_H_

#define SYS_CTRL_STACK_SIZE     1024

#define CMD_MAILBOX_SIZE	4
#define CMD_MEM_POOL_SIZE   10

typedef struct {
	uint8_t		cmd;
	uint8_t		pkgSize;
	uint8_t		payload [62];
} cmdPkg;


extern MemoryPool cmdMemPool;
extern Mailbox cmdInMailbox;
extern Mailbox cmdOutMailbox;

extern void startSysCtrl(void);


#endif /* SYSCTRL_H_ */
