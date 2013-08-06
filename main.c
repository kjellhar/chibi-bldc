

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
