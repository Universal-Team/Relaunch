#include "systemdetails.h"

SystemDetails::SystemDetails()
{

    _isRegularDS = true;

	fifoWaitValue32(FIFO_USER_06);
    if (fifoGetValue32(FIFO_USER_03) == 0)
        _arm7SCFGLocked = true; // If Relaunch is being run from flashcard, then arm7 SCFG is locked.
    
    u16 arm7_SNDEXCNT = fifoGetValue32(FIFO_USER_07);
    if (arm7_SNDEXCNT != 0)
    {
        _isRegularDS = false; // If sound frequency setting is found, then the console is not a DS Phat/Lite
    }
    
    // Restore value.
    fifoSendValue32(FIFO_USER_07, arm7_SNDEXCNT);
}
