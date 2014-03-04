#include <COMMANDS.h>
#include <EEPROMex.h>

COMMANDS::COMMANDS (void) {

  Configuration CONFIG1;
  EEPROM.readBlock(0,CONFIG1);
}