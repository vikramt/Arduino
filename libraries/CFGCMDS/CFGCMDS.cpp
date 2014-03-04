#include <CFGCMDS.h>
#include <EEPROMex.h>

CFGCMDS::CFGCMDS () {

  Configuration CONFIG1;
  EEPROM.readBlock(0,CONFIG1);
}
