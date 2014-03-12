#ifndef _CFGCMDS_H_
#define _CFGCMDS_H_
#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <wiring.h>
#include "pins_arduino.h"
#endif
#include <avr/wdt.h>

// bit position for bitchanged configuration
#define bit_temperature 0
#define bit_voltage 1
#define bit_data0 2
#define bit_data1 3
#define bit_data2 4
#define bit_data3 5
#define bit_data4 6
#define bit_data5 7


//Commands magic numbers are below

#define WDTREBOOT			126
#define READtemp			18
#define READvoltage 		19
#define	READdata0			20
#define READdata1			21
#define	READdata2			22
#define READdata3			23
#define READdata4			24
#define READdata5			25
#define READdata6			26
#define READreserve			27

#define READconfigreq		28   // can only request the full configs for reading
#define READconfigresp		254	//slave sends readconfigresponse as teh first byte so gateway knows its config response and not normal data


#define	WRITEdata0			30
#define WRITEdata1			31
#define	WRITEdata2			32
#define WRITEdata3			33
#define WRITEdata4			34
#define WRITEdata5			35
#define WRITEdata6			36


#define WRITEnodeidreq			40
#define WRITExmitminreq			41
#define WRITExmitchangereq		42
#define WRITEsleepsecondsreq	43
#define WRITEradiopowerreq		44  //S:1,44,20
#define WRITElisten100msreq		45
#define WRITEtempcalibrationreq	46





#define SYNC_MAX_COUNT 10 //max number of other nodes to SYNC with, keep the same with same setting in SwitchMote sketch!
#define SYNC_EEPROM_ADDR 512 //SYNC_TO and SYNC_INFO data starts at this EEPROM address

void WDTReboot(void) ;




 typedef struct  {
  byte nodeID;
  byte command;
  byte parameter ;

} REQUEST;

 typedef struct  {
  byte frequency;
  byte isHW;
  byte nodeID;
  byte networkID;
  char encryptionKey[16];
  byte separator1; //separators needed to keep strings from overlapping
  char description[10];
  byte separator2;
  char reserved[10];
  byte seperator3;
  byte xmitmin; //xmit minimally atleast this many minutes default 5 mins
  byte xmitchange; //if 1 xmit when changes 0 wait for sleep timer for xmit use each bit for each piece of data
  byte sleepseconds; //go back to sleep for this many seconds - node wakes up default every second. default 10 seconds
  byte radiopower;  //set default radio power to 3
  byte listen100ms; // listen for how many 100 ms  after xmit default to 5
  char tempcalibration;  //use this for radio temp sensor calib default -1
  
} Configuration;

class CFGCMDS {
	private:
	Configuration _config;
	bool writeconfig(void);
	
	
	public:
	CFGCMDS(void);
	bool readconfig(void);
	void setinvalid ( void );
	void setdefaultconfig(byte nodeid);
	void setfrequency(byte frequency);
	void setisHW(byte isHW);
	void setnodeID(byte nodeid);
	void setnetworkID(byte networkID);
	void setencryptionKey (char* description);
	void setdescription (char* description);
	void setxmitmin (byte xmitmin);
	void setxmitchange(byte xmitchange);
	void setsleepseconds(byte sleepseconds);
	void setradiopower ( byte radiopower);
	void setlisten100ms( byte listen100ms);
	void settempcalibration ( char tempcalibration );
	
    bool getisvalid ( void );
	byte getfrequency(void);
	byte getisHW(void);
	byte getnodeID(void);
	byte getnetworkID(void);
	char* getencryptionKey (void);
	char* getdescription (void);
	byte getxmitmin (void);
	byte getxmitchange(void);
	byte getsleepseconds(void);
	byte getradiopower ( void);
	byte getlisten100ms( void);
	char gettempcalibration ( void );
	
};


#endif
