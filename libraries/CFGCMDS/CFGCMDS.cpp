#include <CFGCMDS.h>
#include <EEPROMex.h>
#include <RFM69.h>




CFGCMDS::CFGCMDS () {

  
  EEPROM.readBlock(0,_config);
}

bool CFGCMDS::getisvalid () {

  if (_config.frequency!=RF69_433MHZ && _config.frequency!=RF69_868MHZ && _config.frequency!=RF69_915MHZ)
	return false;
  else 
	return true;
  
}

byte CFGCMDS::getfrequency () {

  return(_config.frequency);
  
}
