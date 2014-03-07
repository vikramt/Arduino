
#include <EEPROMex.h> //get it here: http://playground.arduino.cc/Code/EEPROMex
#include <SPI.h>
#include <RFM69.h> //get it here: http://github.com/lowpowerlab/rfm69
#include <SPIFlash.h> //get it here: http://github.com/lowpowerlab/spiflash
#include <avr/wdt.h> //comes with Arduino
#include <CFGCMDS.h>

//these settings are for the SwitchMotes, should match the same values exactly from the SwitchMote sketch
#define SYNC_MAX_COUNT 10 //max number of other nodes to SYNC with, keep the same with same setting in SwitchMote sketch!
#define SYNC_EEPROM_ADDR 512 //SYNC_TO and SYNC_INFO data starts at this EEPROM address
byte SYNC_TO[SYNC_MAX_COUNT]; // stores the address of the remote SM(s) that this SM has to notify/send request to
int SYNC_INFO[SYNC_MAX_COUNT]; // stores the buttons and modes of this and the remote SM as last 4 digits:




Configuration   CONFIG; //see commands.h for details
CFGCMDS cfgcmds; //instantiate the class


void setup()
{
  Serial.begin(115200);
  EEPROM.readBlock(0, CONFIG);
  Serial.println("Reading nodeID from class:");
  cfgcmds.readconfig();
  Serial.print(cfgcmds.getnodeID());
  Serial.println("Reading from local:");
  Serial.print(CONFIG.nodeID);
  
  //delay(5000);
  
  // Serial.println("Setting nodeid 22 into class");
  // cfgcmds.setnodeID(22);
  //  Serial.println("Reading from local again:");
  //  EEPROM.readBlock(0, CONFIG);
  //  Serial.print(CONFIG.nodeID);
  //  Serial.println("Reread from class");
  // cfgcmds.readconfig();
  //  Serial.println("Reading nodeID  again from class:");
  //  Serial.print(cfgcmds.getnodeID());
  //  delay(5000);
  
  
  
  
  


    
  
  if ( cfgcmds.getisvalid() ) // virgin CONFIG, expected [4,8,9]
  {
	  Serial.println("No valid config found in EEPROM, writing defaults");
	  cfgcmds.setdefaultconfig(17);  //defaults set in cfgcmds library except nodeid set here    
  }
  else 
  {
	  Serial.print("Frequency: ");
	  Serial.println(cfgcmds.getfrequency());
	  Serial.print("nodeid: ");
	  Serial.println(cfgcmds.getnodeID());
	  Serial.print("networkid: ");
	  Serial.println(cfgcmds.getnetworkID());
	  Serial.print("key: ");
	  Serial.println(cfgcmds.getencryptionKey());
	  Serial.print("desc: ");
	  Serial.println(cfgcmds.getdescription());
	  Serial.print("xmitmin: ");
	  Serial.println(cfgcmds.getxmitmin());
	  Serial.print("xmitchange: ");
	  Serial.println(cfgcmds.getxmitchange());
	  Serial.print("sleepseconds: ");
	  Serial.println(cfgcmds.getradiopower());
	  Serial.print("listen: ");
	  Serial.println(cfgcmds.getlisten100ms());
	  Serial.print("tempcalib: ");
	  Serial.println(cfgcmds.gettempcalibration(),DEC);
  
  
  }
  
  // EEPROM.readBlock<byte>(SYNC_EEPROM_ADDR, SYNC_TO, SYNC_MAX_COUNT);
  //EEPROM.readBlock<byte>(SYNC_EEPROM_ADDR+SYNC_MAX_COUNT, (byte *)SYNC_INFO, SYNC_MAX_COUNT*2); //int=2bytes so need to cast to byte array
  
  displayMainMenu();
}

void displayMainMenu()
{
  Serial.println();
  Serial.println("-----------------------------------------------");
  Serial.println(" Moteino-RFM69 configuration setup utility");
  Serial.println("-----------------------------------------------");
  Serial.print(" f - set frequency (current: ");Serial.print(cfgcmds.getfrequency()==RF69_433MHZ?"433":cfgcmds.getfrequency()==RF69_868MHZ?"868":"915");Serial.println("mhz)");
  Serial.print(" i - set node ID (current: ");Serial.print(cfgcmds.getnodeID());Serial.println(")");
  Serial.print(" n - set network ID (current: ");Serial.print(cfgcmds.getnetworkID());Serial.println(")");
  Serial.print(" w - set RFM69 type (current: ");Serial.print(cfgcmds.getisHW()?"HW":"W/CW");Serial.println(")");
  Serial.print(" e - set encryption key (current: ");Serial.print(cfgcmds.getencryptionKey());Serial.println(")");
  Serial.print(" d - set description (current: ");Serial.print(cfgcmds.getdescription());Serial.println(")");
  Serial.println(" s - save CONFIG to EEPROM");
  Serial.println(" E - erase whole EEPROM - [0..1023]");
  Serial.print(" S - erase SYNC data [");Serial.print(SYNC_EEPROM_ADDR);Serial.print("..");Serial.print(SYNC_EEPROM_ADDR+3*SYNC_MAX_COUNT-1);Serial.print("]:[");
    displaySYNC();
    Serial.println(']');
  Serial.println(" r - reboot");
  Serial.println(" ESC - return to main menu");
}

char menu = 0;
byte charsRead = 0;
byte tempchar = 0;
void handleMenuInput(char c)
{
  switch(menu)
  {
    case 0:
      switch(c)
      {
      case 'f': Serial.print("\r\nEnter frequency (4 = 433mhz, 8=868mhz, 9=915mhz): "); menu=c; break;
      case 'i': Serial.print("\r\nEnter node ID (1-255 + <ENTER>): "); tempchar=0;menu=c; break;
      case 'n': Serial.print("\r\nEnter network ID (0-255 + <ENTER>): "); tempchar=0; menu=c; break;
      case 'e': Serial.print("\r\nEnter encryption key (type 16 characters): "); menu=c; break;
      case 'w': Serial.print("\r\nIs this RFM69W/CW/HW (0=W/CW, 1=HW): "); menu=c; break;
      case 'd': Serial.print("\r\nEnter description (10 chars max + <ENTER>): "); menu=c; break;
      case 's': Serial.print("\r\nCONFIG saved to EEPROM!"); EEPROM.writeBlock(0, CONFIG); break;
      case 'E': Serial.print("\r\nErasing EEPROM ... "); menu=c; break;
      case 'S': Serial.print("\r\nErasing SYNC EEPROM ... "); menu=c; break;
      case 'r': Serial.print("\r\nRebooting"); resetUsingWatchdog(1); break;
      case 27: displayMainMenu();menu=0;break;
      }
      break;
      
    case 'f':
      switch(c)
      {
        case '4': Serial.println("Set to 433Mhz"); CONFIG.frequency = RF69_433MHZ; menu=0; break;
        case '8': Serial.println("Set to 868Mhz"); CONFIG.frequency = RF69_868MHZ; menu=0; break;
        case '9': Serial.println("Set to 915Mhz"); CONFIG.frequency = RF69_915MHZ; menu=0; break;
        case 27: displayMainMenu();menu=0;break;
      }
      break;

    case 'i':
      if (c >= '0' && c <= '9')
      {
        if (tempchar * 10 + c - 48 <= 255)
        {
          cfgcmds.setnodeID(tempchar * 10 + c - 48);
          
          Serial.print(c);
        }
        else
        {
          Serial.print(" - Set to ");Serial.println(cfgcmds.getnodeID());
          menu=0;
        }
      }
      else if (c == 13 || c == 27)
      {
        Serial.print(" - Set to ");Serial.println(cfgcmds.getnodeID());
        displayMainMenu();
        menu=0;
      }
      break;

    case 'n':
      if (c >= '0' && c <= '9')
      {
        if (tempchar * 10 + c - 48 <= 255)
        {
          cfgcmds.setnetworkID(tempchar * 10 + c - 48);
          Serial.print(c);
        }
        else
        {
          Serial.print(" - Set to ");Serial.println(cfgcmds.getnetworkID());
          menu=0;
        }
      }
      if (c == 13 || c == 27)
      {
        Serial.print(" - Set to ");Serial.println(cfgcmds.getnetworkID());
        displayMainMenu();
        menu=0;
      }
      break;

    case 'e':
      if (c >= ' ' && c <= '~') //human readable chars (32 - 126)
        if (++charsRead<=16)
        {
          CONFIG.encryptionKey[charsRead-1] = c;
          CONFIG.encryptionKey[charsRead] = 0;
          Serial.print(c);
        }
      if (charsRead >= 16 || c == 27 || c == 13)
      {
        //Serial.print(" - Set to [");Serial.print(CONFIG.encryptionKey);Serial.println(']');
        Serial.println(" - DONE");
        displayMainMenu();menu=0;charsRead=0;
      }
      break;

    case 'd':
      if (c >= ' ' && c <= '~') //human readable chars (32 - 126)
      {
        if (++charsRead<=10)
        {
          CONFIG.description[charsRead-1] = c;
          CONFIG.description[charsRead] = 0;
          Serial.print(c);
        }
      }
      if (charsRead>=10 || c == 13 || c == 27)
      {
        //Serial.print(" - Set to [");Serial.print(CONFIG.description);Serial.println(']');
        Serial.println(" - DONE");
        displayMainMenu();menu=0;charsRead=0;
      }
      break;

    case 'w':
      switch(c)
      {
        case '0': Serial.println("Set to RFM69W\\CW"); cfgcmds.setisHW(0); menu=0; break;
        case '1': Serial.println("Set to RFM69HW"); cfgcmds.setisHW(1);; menu=0; break;
        case 27: displayMainMenu();menu=0;break;
      }
      break;
    case 'E':
      for (int i=0;i<1024;i++) EEPROM.write(i,255); //eeprom_write_byte((unsigned char *) i, 255);
      Serial.println("DONE");
      //resetUsingWatchdog(1);
      menu=0;
      break;
    case 'S':
      eraseSYNC();
      Serial.println("DONE");
      menu=0;
      break;
  }
}

void loop()
{
  if(Serial.available())
    handleMenuInput(Serial.read());
}

void resetUsingWatchdog(boolean DEBUG)
{
  wdt_enable(WDTO_15MS);
  while(1) if (DEBUG) Serial.print('.');
}

void displaySYNC()
{
  for (byte i=0; i < SYNC_MAX_COUNT; i++)
  {
    Serial.print(SYNC_INFO[i]);
    if (i!=SYNC_MAX_COUNT-1) Serial.print(',');
  }
}

void eraseSYNC()
{
  for(byte i = 0; i<SYNC_MAX_COUNT; i++)
  {
    SYNC_TO[i]=0;
    SYNC_INFO[i]=0;
  }
  EEPROM.writeBlock<byte>(SYNC_EEPROM_ADDR, SYNC_TO, SYNC_MAX_COUNT);
  EEPROM.writeBlock<byte>(SYNC_EEPROM_ADDR+SYNC_MAX_COUNT, (byte*)SYNC_INFO, SYNC_MAX_COUNT*2);
}
