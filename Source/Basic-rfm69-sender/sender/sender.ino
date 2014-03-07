#include <EEPROMex.h> //get it here: http://playground.arduino.cc/Code/EEPROMex
#include <SPI.h>
#include <RFM69.h> //get it here: http://github.com/lowpowerlab/rfm69
#include <SPIFlash.h> //get it here: http://github.com/lowpowerlab/spiflash
#include <avr/wdt.h> //comes with Arduino
#include <CFGCMDS.h>

// Sample RFM69 sender/node sketch, with ACK and optional encryption
// Sends periodic messages of increasing length to gateway (id=1)
// It also looks for an onboard FLASH chip, if present
// Library and code by Felix Rusu - felix@lowpowerlab.com
// Get the RFM69 and SPIFlash library at: https://github.com/LowPowerLab/


CFGCMDS cfgcmds; //instantiate the class



#define NODEID 2 //unique for each node on same network
#define NETWORKID 10 //the same on all nodes that talk to each other
#define GATEWAYID 1
//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
#define FREQUENCY RF69_433MHZ
//#define FREQUENCY RF69_868MHZ
//#define FREQUENCY RF69_915MHZ
#define ENCRYPTKEY "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
//#define IS_RFM69HW //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define ACK_TIME 30 // max # of ms to wait for an ack
#define LED 9 // Moteinos have LEDs on D9
#define SERIAL_BAUD 115200

int TRANSMITPERIOD = 1900; //transmit a packet to gateway so often (in ms)
char payload[] = "123 ABCDEFGHIJKLMNOPQRSTUVWXYZ";
char buff[20];
byte sendSize=0;
boolean requestACK = false;
SPIFlash flash(8, 0xEF30); //EF40 for 16mbit windbond chip
RFM69 radio;

void setup() {
  Serial.begin(SERIAL_BAUD);
  
  delay(255);delay(255);delay(255);delay(255);delay(255);delay(255);delay(255);delay(255);

    if ( cfgcmds.getisvalid() )     {
            
            radio.initialize(cfgcmds.getfrequency(),cfgcmds.getnodeID(), cfgcmds.getnetworkID()); 
            radio.encrypt(cfgcmds.getencryptionKey())    ;                           
            if (cfgcmds.getisHW() )   { radio.setHighPower(cfgcmds.getradiopower()); } 
            
    } else {
    
            Serial.println("No valid config found in EEPROM, not writing defaults");
            delay(5000);
    }
    
 
}

long lastPeriod = -1;
void loop() {
  //process any Serial input
  if (Serial.available() > 0)
  {
    char input = Serial.read();
    if (input >= 48 && input <= 57) //[0,9]
    {
      TRANSMITPERIOD = 100 * (input-48);
      if (TRANSMITPERIOD == 0) TRANSMITPERIOD = 1000;
      Serial.print("\nChanging delay to ");
      Serial.print(TRANSMITPERIOD);
      Serial.println("ms\n");
    }
    
    if (input == 'r') //d=dump register values
      radio.readAllRegs();
    //if (input == 'E') //E=enable encryption
    // radio.encrypt(KEY);
    //if (input == 'e') //e=disable encryption
    // radio.encrypt(null);
    
    if (input == 'd') //d=dump flash area
    {
      Serial.println("Flash content:");
      int counter = 0;

      while(counter<=256){
        Serial.print(flash.readByte(counter++), HEX);
        Serial.print('.');
      }
      while(flash.busy());
      Serial.println();
    }
    if (input == 'e')
    {
      Serial.print("Erasing Flash chip ... ");
      flash.chipErase();
      while(flash.busy());
      Serial.println("DONE");
    }
    if (input == 'i')
    {
      Serial.print("DeviceID: ");
      word jedecid = flash.readDeviceId();
      Serial.println(jedecid, HEX);
    }
  }

  //check for any received packets
  if (radio.receiveDone())
  {
    Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
    for (byte i = 0; i < radio.DATALEN; i++)
      Serial.print((char)radio.DATA[i]);
    Serial.print(" [RX_RSSI:");Serial.print(radio.RSSI);Serial.print("]");

    if (radio.ACK_REQUESTED)
    {
      radio.sendACK();
      Serial.print(" - ACK sent");
      delay(10);
    }
    Blink(LED,5);
    Serial.println();
  }
  
  //send FLASH id
  if(sendSize==0)
  {
    sprintf(buff, "FLASH_MEM_ID:0x%X", flash.readDeviceId());
    byte buffLen=strlen(buff);
    radio.sendWithRetry(GATEWAYID, buff, buffLen);
    delay(TRANSMITPERIOD);
  }
  
  int currPeriod = millis()/TRANSMITPERIOD;
  if (currPeriod != lastPeriod)
  {
    lastPeriod=currPeriod;
    Serial.print("Sending[");
    Serial.print(sendSize);
    Serial.print("]: ");
    for(byte i = 0; i < sendSize; i++)
      Serial.print((char)payload[i]);

    if (radio.sendWithRetry(GATEWAYID, payload, sendSize))
     Serial.print(" ok!");
    else Serial.print(" nothing...");

    sendSize = (sendSize + 1) % 31;
    Serial.println();
    Blink(LED,3);
  }
}

void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}

// reads a line feed (\n) terminated line from the serial stream
// returns # of bytes read, up to 255
// timeout in ms, will timeout and return after so long
byte readSerialLine(char* input, char endOfLineChar, byte maxLength, uint16_t timeout)
{
  byte inputLen = 0;
  Serial.setTimeout(timeout);
  inputLen = Serial.readBytesUntil(endOfLineChar, input, maxLength);
  input[inputLen]=0;//null-terminate it
  Serial.setTimeout(0);
  //Serial.println();
  return inputLen;
}
