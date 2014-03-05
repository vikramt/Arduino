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

void  CFGCMDS::setdefaultconfig(byte nodeID) {
	
	_config.frequency=RF69_433MHZ;
	_config.isHW=1;
	_config.nodeID=nodeID;
	_config.networkID=10;
	strcpy(_config.description,"general node");
	_config.xmitmin=5;
	_config.xmitchange=0;
	_config.sleepseconds=10;
	_config.radiopower =3;
	_config.listen100ms=2;
	_config.tempcalibration =-1;
	EEPROM.writeBlock(0, _config);
	
}

void  CFGCMDS::setnodeID (byte nodeID) {

  _config.nodeID=nodeID;
  writeconfig();
}

void  CFGCMDS::setfrequency (byte frequency) {

  _config.frequency=frequency;
  writeconfig();
}

void  CFGCMDS::setisHW (byte isHW) {

  _config.isHW=isHW;
  writeconfig();
}

void  CFGCMDS::setnetworkID (byte networkID) {

  _config.networkID=networkID;
  writeconfig();
}

void  CFGCMDS::setdescription (char* description) {

  strcpy(_config.description,description);
  writeconfig();
}

void  CFGCMDS::setxmitmin (byte xmitmin) {

  _config.xmitmin=xmitmin;
  writeconfig();
}

void  CFGCMDS::setxmitchange (byte xmitchange) {

  _config.xmitchange=xmitchange;
  writeconfig();
}

void  CFGCMDS::setsleepseconds (byte sleepseconds) {

  _config.sleepseconds=sleepseconds;
  writeconfig();
}

void  CFGCMDS::setradiopower (byte radiopower) {

  _config.radiopower=radiopower;
  writeconfig();
}

void  CFGCMDS::setlisten100ms (byte listen100ms) {

  _config.listen100ms=listen100ms;
  writeconfig();
}

void  CFGCMDS::settempcalibration (byte tempcalibration) {

  _config.tempcalibration=tempcalibration;
  writeconfig();
}