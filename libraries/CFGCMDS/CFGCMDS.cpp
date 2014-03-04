#include <CFGCMDS.h>
#include <EEPROMex.h>
#include <RFM69.h>



// -------------------------------------constructor uses outside global variable _config ------
CFGCMDS::CFGCMDS () {

  
  EEPROM.readBlock(0,_config);
}

// -------------------------------------------------get functions ------------

bool  CFGCMDS::readconfig(void) {
	EEPROM.readBlock(0, _config);
	return false;
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

byte CFGCMDS::getnodeID () {

  return(_config.nodeID);
  
}

// -------------------------------------------------SET functions ------

bool  CFGCMDS::writeconfig(void) {
	EEPROM.writeBlock(0, _config);
	return false;
}

void  CFGCMDS::setnodeID (byte nodeID) {

  _config.nodeID=nodeID;
  writeconfig();
  
  
}


