
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
CFGCMDS config; //instantiate the class


void setup()
{
  Serial.begin(115200);
  EEPROM.readBlock(0, CONFIG);
  Serial.println("Reading from class:");
  byte f = config.getfrequency();
    
  
  if (CONFIG.frequency!=RF69_433MHZ && CONFIG.frequency!=RF69_868MHZ && CONFIG.frequency!=RF69_915MHZ) // virgin CONFIG, expected [4,8,9]
  {
    Serial.println("No valid config found in EEPROM, writing defaults");
    CONFIG.separator1=CONFIG.separator2=CONFIG.seperator3=0;
    CONFIG.frequency=RF69_433MHZ;
    CONFIG.isHW=1;
    CONFIG.nodeID=36;
    CONFIG.networkID=10;
    strcpy (CONFIG.encryptionKey,"sampleEncryptKey");
    strcpy(CONFIG.description, "general node");
  CONFIG.reserved[0]=0;
  CONFIG.xmitmin = 5; //xmit minimally atleast this many minutes default 5 mins
  CONFIG.xmitchange = 1; //if 1 xmit when changes 0 wait for sleep timer for xmit use each bit for each piece of data
  CONFIG.sleepseconds=10; //go back to sleep for this many seconds - node wakes up default every seconds. default 10 seconds  
  CONFIG.radiopower=3;  //set default radio power to 3
  CONFIG.listen100ms=5; // listen for how many 100 ms  after xmit
  CONFIG.tempcalibration=-1;  //use this for radio temp sensor calib default -1
    
  }
  
  EEPROM.readBlock<byte>(SYNC_EEPROM_ADDR, SYNC_TO, SYNC_MAX_COUNT);
  EEPROM.readBlock<byte>(SYNC_EEPROM_ADDR+SYNC_MAX_COUNT, (byte *)SYNC_INFO, SYNC_MAX_COUNT*2); //int=2bytes so need to cast to byte array
  
  displayMainMenu();
}

void displayMainMenu()
{
  Serial.println();
  Serial.println("-----------------------------------------------");
  Serial.println(" Moteino-RFM69 configuration setup utility");
  Serial.println("-----------------------------------------------");
  Serial.print(" f - set frequency (current: ");Serial.print(CONFIG.frequency==RF69_433MHZ?"433":CONFIG.frequency==RF69_868MHZ?"868":"915");Serial.println("mhz)");
  Serial.print(" i - set node ID (current: ");Serial.print(CONFIG.nodeID);Serial.println(")");
  Serial.print(" n - set network ID (current: ");Serial.print(CONFIG.networkID);Serial.println(")");
  Serial.print(" w - set RFM69 type (current: ");Serial.print(CONFIG.isHW?"HW":"W/CW");Serial.println(")");
  Serial.print(" e - set encryption key (current: ");Serial.print(CONFIG.encryptionKey);Serial.println(")");
  Serial.print(" d - set description (current: ");Serial.print(CONFIG.description);Serial.println(")");
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
void handleMenuInput(char c)
{
  switch(menu)
  {
    case 0:
      switch(c)
      {
        case 'f': Serial.print("\r\nEnter frequency (4 = 433mhz, 8=868mhz, 9=915mhz): "); menu=c; break;
        case 'i': Serial.print("\r\nEnter node ID (1-255 + <ENTER>): "); CONFIG.nodeID=0;menu=c; break;
        case 'n': Serial.print("\r\nEnter network ID (0-255 + <ENTER>): "); CONFIG.networkID=0; menu=c; break;
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
        if (CONFIG.nodeID * 10 + c - 48 <= 255)
        {
          CONFIG.nodeID = CONFIG.nodeID * 10 + c - 48;
          Serial.print(c);
        }
        else
        {
          Serial.print(" - Set to ");Serial.println(CONFIG.nodeID);
          menu=0;
        }
      }
      else if (c == 13 || c == 27)
      {
        Serial.print(" - Set to ");Serial.println(CONFIG.nodeID);
        displayMainMenu();
        menu=0;
      }
      break;

    case 'n':
      if (c >= '0' && c <= '9')
      {
        if (CONFIG.networkID * 10 + c - 48 <= 255)
        {
          CONFIG.networkID = CONFIG.networkID * 10 + c - 48;
          Serial.print(c);
        }
        else
        {
          Serial.print(" - Set to ");Serial.println(CONFIG.networkID);
          menu=0;
        }
      }
      if (c == 13 || c == 27)
      {
        Serial.print(" - Set to ");Serial.println(CONFIG.networkID);
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
        case '0': Serial.println("Set to RFM69W\\CW"); CONFIG.isHW = 0; menu=0; break;
        case '1': Serial.println("Set to RFM69HW"); CONFIG.isHW = 1; menu=0; break;
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
