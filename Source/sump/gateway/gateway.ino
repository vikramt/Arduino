#include <EEPROMex.h> //get it here: http://playground.arduino.cc/Code/EEPROMex
#include <SPI.h>
#include <RFM69.h> //get it here: http://github.com/lowpowerlab/rfm69
#include <SPIFlash.h> //get it here: http://github.com/lowpowerlab/spiflash
#include <avr/wdt.h> //comes with Arduino
#include <CFGCMDS.h>
/********************        GATEWAY **********************/
/********************        GATEWAY **********************/
CFGCMDS cfgcmds; //instantiate the class
REQUEST sendRequest ;
//#define NODEID 1 //unique for each node on same network
//#define NETWORKID 10 //the same on all nodes that talk to each other
////Match frequency to the hardware version of the radio on your Moteino (uncomment one):
//#define FREQUENCY RF69_433MHZ
////#define FREQUENCY RF69_868MHZ
////#define FREQUENCY RF69_915MHZ
//#define ENCRYPTKEY "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
//#define IS_RFM69HW 
#define ACK_TIME 50 // max # of ms to wait for an ack
#define LED 9 // Moteinos have LEDs on D9
#define SERIAL_BAUD 115200
static char payload[32]=             	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static char radioBuffer[32]= 			{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static char serialInputBuffer[32] =  	{ 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
RFM69 radio;
SPIFlash flash(8, 0xEF30); //EF40 for 16mbit windbond chip
bool promiscuousMode = false; //set to 'true' to sniff all packets on the same network

void setup() {
	delay(200);delay(200);delay(200);delay(200);delay(200);delay(200);delay(200);delay(200);delay(200);delay(200);
	Serial.begin(SERIAL_BAUD);
	delay(10);
	if ( ! cfgcmds.getisvalid() ) // virgin CONFIG, expected [4,8,9]
	{
		while (1 ) {
			Serial.println("No valid config found in EEPROM, NOT writing defaults");
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
		Serial.print("listen: ");
		Serial.println(cfgcmds.getlisten100ms());
		Serial.print("tempcalib: ");
		Serial.println(cfgcmds.gettempcalibration(),DEC);
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


void loop() {
	//get any serial input
	//readSerialLine(char* input, char endOfLineChar=10, byte maxLength=32, uint16_t timeout=1000);
	byte inputLen = readSerialLine(serialInputBuffer, ';', 32, 10); 
    if (radio.receiveDone())
    {
      if (radio.ACK_REQUESTED)
      {
        byte theNodeID = radio.SENDERID;
        radio.sendACK();
        Serial.print(" - ACK sent."); 
      }
      Serial.println();
	  Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
      if (promiscuousMode)
      {
        Serial.print("to [");Serial.print(radio.TARGETID, DEC);Serial.print("] ");
      }
	  //copy the radio data serial port 
	  strcpy(radioBuffer,(char*)radio.DATA);
	  Serial.print(radioBuffer);
      Serial.print(" [RX_RSSI:");Serial.print(radio.RSSI);Serial.print("]");
      if (radio.ACK_REQUESTED)
      {
        byte theNodeID = radio.SENDERID;
        radio.sendACK();
        Serial.print(" - ACK sent."); 
      }
      Serial.println();
      // Blink(LED,3);
    }
	// after receive done check serial buffer and transmit if command ready   or serial
	// do a bitmap of which nodes have command or just loop ? loop will imply priority
	// transmitimmediate via serial command or wait after node checks in logic should be in pc
	// gateway will just transmit when its sent serial commands.
	if (serialInputBuffer[0] == 'p')    {
      promiscuousMode = !promiscuousMode;
      radio.promiscuous(promiscuousMode);
      Serial.print("Promiscuous mode ");Serial.println(promiscuousMode ? "on" : "off");
      serialInputBuffer[0]=0; //command completed
    }
    if (serialInputBuffer[0] == 't')     {
      byte temperature = radio.readTemperature(-1); // -1 = user cal factor, adjust for correct ambient
      byte fTemp = 1.8 * temperature + 32; // 9/5=1.8
      Serial.print( "Radio Temp is ");
      Serial.print(temperature);
      Serial.print("C, ");
      Serial.print(fTemp); //converting to F loses some resolution, obvious when C is on edge between 2 values (ie 26C=78F, 27C=80F)
      Serial.println('F');
      serialInputBuffer[0]=0; //command completed
    }
	if (serialInputBuffer[0] == 'S')
    {
	  byte  requestgood ;
	  char * p;  
	  char * last ;
	  payload[0]==0;
	  // next char is a :
	  if ( serialInputBuffer[1]==':' ) {
		  requestgood = 0;
		  p = strtok_r(serialInputBuffer,":", &last);
		  
		  p = strtok_r(NULL,",", &last );
		  if ( p != NULL ) {
			sendRequest.nodeID = atoi(p); 
			requestgood++;
		  }		  
		  else { 
			sendRequest.nodeID = 0;
		  }
		  
		  p = strtok_r(NULL,",", &last );
		  if ( p != NULL ) {
			sendRequest.command = atoi(p);
			if ( sendRequest.command > 0 && sendRequest.command <255  ) //command can not be zero
			    requestgood++;
			else 
				requestgood--;
		  }
		  else { 
			sendRequest.command = 0;
		  }
		  
		  p = strtok_r(NULL,",", &last );
		  if (  p != NULL )  {
		    sendRequest.parameter = atoi(p);			
			requestgood++;
		  }
		  else { 
			sendRequest.parameter = 0;
		  }
		  if ( requestgood > 1 ) {
			requestgood=0;
		    Serial.print("Sending to node : "); Serial.println(sendRequest.nodeID);
		    Serial.print ("Command is  "); Serial.println(sendRequest.command);
			Serial.print ("Parameter is  "); Serial.println(sendRequest.parameter);
			//transmit the packet
			if (radio.sendWithRetry(sendRequest.nodeID, &sendRequest, sizeof(sendRequest),2,30 ) )
				Serial.print("Sent and acked  ok!");
			else 
				Serial.print(" Unable to  transmit ...");
			
		  } else {

			Serial.print("Not Sending Request received: "); Serial.println(serialInputBuffer);
		  }
	  }
    }
}  //*************** loop end ********************/



void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}
byte readSerialLine(char* input, char endOfLineChar, byte maxLength, uint16_t timeout)
{	
// reads a line feed (\n) terminated line from the serial stream
// returns # of bytes read, up to 255
// timeout in ms, will timeout and return after so long
  byte inputLen = 0;
  Serial.setTimeout(timeout);
  inputLen = Serial.readBytesUntil(endOfLineChar, input, maxLength);
  input[inputLen]=0;//null-terminate it
  Serial.setTimeout(0);
  //Serial.println();
  return inputLen;
}


