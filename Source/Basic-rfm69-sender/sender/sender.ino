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




#define GATEWAYID 1
#define ACK_TIME 30 // max # of ms to wait for an ack
#define LED 9 // Moteinos have LEDs on D9
#define SERIAL_BAUD 115200

int TRANSMITPERIOD = 900; //transmit a packet to gateway so often (in ms)
char payload[] = "123 ABCDEFGHIJKLMNOPQRSTUVWXYZ";
char buff[20];
byte sendSize=0;
byte recvSENDERID=0;
byte recvDATALEN =0 ;
byte recvRSSI = 0 ;
boolean requestACK = false;
SPIFlash flash(8, 0xEF30); //EF40 for 16mbit windbond chip
RFM69 radio;
REQUEST recvRequest;


void setup() {
 	delay(200);delay(200);delay(200);delay(200);delay(200);delay(200);delay(200);delay(200);delay(200);delay(200);
	Serial.begin(SERIAL_BAUD);
	delay(10);
    
		if ( ! cfgcmds.getisvalid() ) // virgin CONFIG, expected [4,8,9]
	{
		while (1 ) {
			Serial.println("No valid config found in EEPROM, NOT writing defaults");
			delay(1500);
		}
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
		Serial.print("listen100ms: ");
		Serial.println(cfgcmds.getlisten100ms());
		Serial.print("tempcalib: ");
		Serial.println(cfgcmds.gettempcalibration(),DEC);
		//cfgcmds.setradiopower(5);
		Serial.print("radiopower: ");
		Serial.println(cfgcmds.getradiopower(),DEC);
	}
	radio.initialize(cfgcmds.getfrequency(),cfgcmds.getnodeID(),cfgcmds.getnetworkID());
	if ( cfgcmds.getisHW() ) {
		radio.setHighPower(); //uncomment only for RFM69HW!
	}	
	radio.encrypt(cfgcmds.getencryptionKey());
	
	radio.setPowerLevel(cfgcmds.getradiopower());
	if (flash.initialize()) {
		Serial.println("SPI Flash Init OK!");
	}
	else {
		Serial.println("SPI Flash Init fail - no chip?");
	}
	Serial.print("Starting:"); Serial.println(cfgcmds.getdescription());
}

long lastPeriod = -1;

void loop() {
  //process any Serial input
  if (Serial.available() > 0)
  {
    char input = Serial.read();
    
    if (input == 'r') //d=dump register values
      radio.readAllRegs();


    if (input == 't')
    {      
	  Serial.print ("Node ");
	  Serial.print(cfgcmds.getnodeID());
	  Serial.print( " temp:") ; 
	  Serial.println ( radio.readTemperature(cfgcmds.gettempcalibration()));
    }
	
  }

  //check for any received packets S:1,126,0 S:2,18,0 
  if (radio.receiveDone())
  { 
	if ( radio.DATALEN > 0 ) { 
		// if we have data copy it first
		memcpy(&recvRequest,(void*) &radio.DATA,sizeof(recvRequest)); 
		recvSENDERID = radio.SENDERID ;
		recvDATALEN = radio.DATALEN ;
	}
	// then send ack if requested to free up sender
	if (radio.ACK_REQUESTED)
    {
      radio.sendACK();
      Serial.print(" - ACK sent");
      delay(1);
    }
	radio.sleep();// can we sleep after sending ack
    Serial.print('[');Serial.print(recvSENDERID, DEC);Serial.print("] ");
							//for (byte i = 0; i < radio.DATALEN; i++)
							//  Serial.print(radio.DATA[i]);
	
    if ( recvDATALEN > 0 ) { //check for datalen cause it maybe just an ACK request etc and radio.data maybe stale
        memset( &recvRequest,0,sizeof(recvRequest)); // can we zero the request like this
        if ( recvRequest.nodeID == cfgcmds.getnodeID()  ) {
            Serial.print("command is : "); Serial.println(recvRequest.command);
            Serial.print("parameter is :"); Serial.println(recvRequest.parameter);
        } else {
			Serial.print("To nodeid: "); Serial.println (recvRequest.nodeID);
        }
    }
    Serial.print(" [RX_RSSI:");Serial.print(recvRSSI);Serial.print("]");
	
	
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

    if (radio.sendWithRetry(GATEWAYID, payload, sendSize,1,30))
    //radio.send(GATEWAYID, payload, sendSize,0);
       Serial.print(" ok!");
    else 
       Serial.print(" nothing...");

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
