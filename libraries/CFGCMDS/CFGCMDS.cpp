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

byte CFGCMDS::getisHW () {
  return(_config.isHW);
}

byte CFGCMDS::getnetworkID () {
  return(_config.networkID);
}

byte CFGCMDS::getradiopower () {
  return(_config.radiopower);
}

byte CFGCMDS::getlisten100ms () {
  return(_config.listen100ms);
}

char CFGCMDS::gettempcalibration () {
  return(_config.tempcalibration);
}

byte CFGCMDS::getxmitmin () {
  return(_config.xmitmin);
}

byte CFGCMDS::getxmitchange () {
  return(_config.xmitchange);
}

char* CFGCMDS::getdescription () {
  return(_config.description);
}

char* CFGCMDS::getencryptionKey () {
  return(_config.encryptionKey);
}

// -------------------------------------------------SET functions ------

bool  CFGCMDS::writeconfig(void) {
	EEPROM.writeBlock(0, _config);
	return false;
}

void  CFGCMDS::setdefaultconfig(byte nodeID) {
	
	_config.frequency=RF69_433MHZ;
	_config.isHW=1;
	//_config.nodeID=nodeID;
	_config.networkID=10;
	strcpy(_config.description,"blank nde");
	strcpy(_config.encryptionKey,"sampleEncryptKey");
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

void  CFGCMDS::setencryptionKey (char* encryptionkey) {

  strcpy(_config.encryptionKey,encryptionkey);
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

void  CFGCMDS::settempcalibration (char tempcalibration) {

  _config.tempcalibration=tempcalibration;
  writeconfig();
}


void WDTReboot()
{
  wdt_enable(WDTO_15MS);
  while(1)
  {
  }
}
