#include <EEPROMex.h> //get it here: http://playground.arduino.cc/Code/EEPROMex
#include <SPI.h>
#include <RFM69.h> //get it here: http://github.com/lowpowerlab/rfm69
#include <SPIFlash.h> //get it here: http://github.com/lowpowerlab/spiflash
#include <LowPower.h>
#include <avr/wdt.h> //comes with Arduino
#include <CFGCMDS.h>

// HC-SR04 pins
#define ECHO 3
#define TRIG 4

// Based on LowPowerLab SumpAlert

#define  curDistance curData0
#define  prevDistance prevData0

int curVoltage,prevVoltage=0;
int curRadiotemp, prevRadiotemp=0;
int curData0, prevData0=0;
int txErrors=0;
int txGood=0;
long curmillis,listenmillis=0;

byte doTransmit=0;

CFGCMDS cfgcmds; //instantiate the class




#define GATEWAYID 1
#define ACK_TIME 30 // max # of ms to wait for an ack
#define LED 9 // Moteinos have LEDs on D9
#define SERIAL_BAUD 115200

int TRANSMITPERIOD = 900; //transmit a packet to gateway so often (in ms)
char payload[60] = {"aBc"};
char buff[20];
byte sendSize=0;
byte recvSENDERID=0;
byte recvDATALEN =0 ;
byte recvRSSI = 0 ;
boolean requestACK = false;
SPIFlash flash(8, 0xEF30); //EF40 for 16mbit windbond chip
RFM69 radio;
REQUEST recvRequest;
int minxmit = 0 ; 


void setup() {
 	delay(200);delay(200);delay(200);delay(200);delay(200);
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
	
	curDistance=getDistance();
	curRadiotemp=radio.readTemperature(cfgcmds.gettempcalibration());
	
   doTransmit=1;    
   //IF xmit change is 1 then hold back transmit if 0 then transmit on loop
	if ( (cfgcmds.getxmitchange() & bitVoltage ) && (curVoltage != prevVoltage)) { 
		doTransmit=0; 
		prevVoltage=curVoltage;
	}
	if ( (cfgcmds.getxmitchange() & bitRadiotemp ) && (curRadiotemp != prevRadiotemp)) { 
		doTransmit=0; 
		prevRadiotemp=curRadiotemp;
	}
	if ( (cfgcmds.getxmitchange() & bitData0 ) && (curDistance != prevDistance)) { 
		doTransmit=0; 
		prevDistance=curDistance;
	}
	
   minxmit--;
	if ( minxmit <=0 ) { 
		minxmit = ( cfgcmds.getxmitmin() * 60 )/( cfgcmds.getsleepseconds() ) ;  //reset minxmit countdown to cyles of sleepseconds
		doTransmit = 1 ;
	}
	
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
  
  if ( doTransmit ) {
	
	
	// fill payload here and send data
   sprintf(payload,"SMP V%d, T%d, D%d", curVoltage, curRadiotemp, curDistance);
	
	if (radio.sendWithRetry(GATEWAYID, payload, sizeof(payload),1,50)) {
    
       Serial.print(" TX ok: ");
	   Serial.println(txGood++);
	}
    else {
       
	   Serial.print(" TX Error: "); 
	   Serial.println(txErrors++);
	}
	   

  }

   // listen for listen100ms durations after the transmit
   curmillis=millis();
   listenmillis=100 * cfgcmds.getlisten100ms() ;

   
   while ( millis() < ( curmillis + listenmillis)   ) 
   {
      //Serial.println("Now listening");
      //check for any received packets S:2,126,0 S:2,20,0 
      if (radio.receiveDone())
      { 
         memset( &recvRequest,0,sizeof(recvRequest)); // can we zero the request like this
         recvSENDERID=0; recvDATALEN=0;
         
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
            //delay(1);
         }
         radio.sleep();// can we sleep after sending ack
         Serial.print('[');Serial.print(recvSENDERID, DEC);Serial.print("] ");

         
         if ( recvDATALEN > 0 ) { //check for datalen cause it maybe just an ACK request etc and radio.data maybe stale
            
            if ( recvRequest.nodeID == cfgcmds.getnodeID()  ) {
                  Serial.print("command is : "); Serial.println(recvRequest.command);
                  Serial.print("parameter is :"); Serial.println(recvRequest.parameter);
            } else {
               Serial.print("To nodeid: "); Serial.println (recvRequest.nodeID);
            }
         }
         Serial.print(" [RX_RSSI:");Serial.print(recvRSSI);Serial.print("]");
         
         if( recvRequest.command > 0 && recvRequest.nodeID == cfgcmds.getnodeID() ) {
            Serial.print("Executing cmmand: "); Serial.println(recvRequest.command); 
            Serial.print("Parameter is: "); Serial.println(recvRequest.parameter);
            
            doCommand(recvRequest.command,recvRequest.parameter);
            recvRequest.command=0;
         }
         
         //Blink(LED,5);
         Serial.println();
         break; // if tx  received break out of listen mode
      }
  
   }
  

    radio.sleep();
    byte s = cfgcmds.getsleepseconds() ; 
    while ( --s ) {
      LowPower.powerDown(SLEEP_500MS, ADC_OFF, BOD_OFF);
      // sleep for sleepseconds durations - seems to need 500MS powerdown instead of 1S to be close????
   }
  

}

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

int getDistance() {
  int duration, distance;
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  duration = pulseIn(ECHO, HIGH);
  distance = (duration/2) / 29.1;
  return distance;
}

void doCommand(byte command, byte parameter )
{
    long localdata ;

    if ( command  ) {
        //command should be greater than zero
        switch(command) {
            case READtemp:
                Serial.print ("Radio Temp:") ;
                Serial.println(curRadiotemp);
                payload[0] = 0;
                sprintf(payload, "T%d", curRadiotemp );
                radio.sendWithRetry(GATEWAYID, payload, sizeof(payload), 1, 50) ;
                break ;

            case READvoltage:
                Serial.print ("BatVoltage") ;
                Serial.println(curVoltage);
                payload[0] = 0;
                sprintf(payload, "V%d", curVoltage );
                radio.sendWithRetry(GATEWAYID, payload, sizeof(payload), 1, 50);
                break ;

            case READdata0:
                Serial.print ("Data0: ") ;
                Serial.println(curDistance);
                payload[0] = 0;
                sprintf(payload, "D%d", curDistance );
                radio.sendWithRetry(GATEWAYID, payload, sizeof(payload), 1, 50)  ;
                break ;

            case WRITEtempcalibrationreq:
                Serial.println("Set tempcalib");
                cfgcmds.settempcalibration(parameter);
                break ;

            case WRITElisten100msreq:
                Serial.println("Set listen100ms");
                cfgcmds.setlisten100ms(parameter);
                break ;

            case WRITEsleepsecondsreq:
                Serial.println("Set sleepseconds");
                cfgcmds.setsleepseconds(parameter);
                break ;

            case WRITEradiopowerreq:
                Serial.println("Set Radiopower");
                cfgcmds.setradiopower(parameter);
                break ;

            case WRITExmitminreq:
                Serial.println("Set xmitmin");
                cfgcmds.setxmitmin(parameter);
                break ;

            case WRITExmitchangereq:
                Serial.println("Set xmitchange");
                cfgcmds.setxmitchange(parameter);
                break ;

            case WDTREBOOT :
                Serial.println("Rebooting node");
                WDTReboot();
                break ;

        }


    } else {

        Serial.print("Not Executing ..  Command: ");
    }


}

