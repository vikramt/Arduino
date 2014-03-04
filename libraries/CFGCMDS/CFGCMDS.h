#ifndef _CFGCMDS_H_
#define _CFGCMDS_H_
#if ARDUINO >= 100
#include <Arduino.h>
#else
#include <wiring.h>
#include "pins_arduino.h"
#endif

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
#define ConfigRead 100
#define ConfigWrite 101
#define DataRead	110
#define DataWrite	111
#define Reboot		126
#define TEMP	51
#define VOLTAGE 52
#define	DATA0	53
#define DATA1	54
#define	DATA2	55
#define DATA3	56
#define DATA4	57
#define DATA5	58
#define DATA6	59
#define DATA7	60
#define DATA8	61

#define SYNC_MAX_COUNT 10 //max number of other nodes to SYNC with, keep the same with same setting in SwitchMote sketch!
#define SYNC_EEPROM_ADDR 512 //SYNC_TO and SYNC_INFO data starts at this EEPROM address



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
  byte tempcalibration;  //use this for radio temp sensor calib default -1
  
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
	void setdescription (char* description);
	void setxmitmin (byte xmitmin);
	void setxmitchange(byte xmitchange);
	void setsleepseconds(byte sleepseconds);
	void setradiopower ( byte radiopower);
	void setlisten100ms( byte listen100ms);
	void settempcalibration ( byte tempcalibration );
	
    bool getisvalid ( void );
	byte getfrequency(void);
	byte getisHW(void);
	byte getnodeID(void);
	byte getnetworkID(void);
	char* getdescription (void);
	byte getxmitmin (void);
	byte getxmitchange(void);
	byte getsleepseconds(void);
	byte getradiopower ( void);
	byte getlisten100ms( void);
	byte gettempcalibration ( void );
	
};


#endif
