
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#include "ch.h"
#include "hal.h"

#include "sysctrl.h"


int main(void) {

  //Start System
  halInit();
  chSysInit();

  startSysCtrl();


  while (TRUE) {
    chThdSleepMilliseconds(1000);
  }
}
