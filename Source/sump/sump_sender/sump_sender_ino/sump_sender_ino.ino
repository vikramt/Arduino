#include <RFM69.h>
#include <EEPROMex.h>
#include <RFM69registers.h>
#include <LowPower.h>
#include <RFM69.h>
#include <SPI.h>
#include <SPIFlash.h>
#include <CFGCMDS.h>

#define NODEID 2 //unique for each node on same network
#define NETWORKID 10 //the same on all nodes that talk to each other
#define GATEWAYID 1
//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
#define FREQUENCY RF69_433MHZ
#define FREQUENCY RF69_868MHZ
#define FREQUENCY RF69_915MHZ
#define ENCRYPTKEY "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
//#define IS_RFM69HW //uncomment only for RFM69HW! Leave out if you have RFM69W!
#define ACK_TIME 30 // max # of ms to wait for an ack
#define LED 9 // Moteinos have LEDs on D9
#define SERIAL_BAUD 115200
#define ECHO 4
#define TRIG 5



char payload[32] ="                               ";

char buff[20];
byte sendSize=0;
boolean requestACK = false;
SPIFlash flash(8, 0xEF30); //EF40 for 16mbit windbond chip
RFM69 radio;

byte sleepseconds = 10 ;    //replace this with configuration sleepseconds
byte minxmit = 5    ;      //replace this with configuration minxmit
long cycle = minxmit*60;  //sleepseconds ;
byte xmitchange=0;       // replace this with configuration

byte datachanged=0;       // bitwise flag to find out if any of the 8 possible data changed  bit0 is temp bit1 is voltage
byte doxmit = 1;             // only xmit when set
byte sleepsecondscounter=0 ;
long cyclecounter=0;

long prevtemperature = 0;
long prevboltage = 0;
long prevdistance = 0;
//long prevdata3 = 0;
//long prevdata4 = 0;
//long prevdata5 = 0;
//long prevdata6 = 0;
//long prevdata7 = 0;



void setup() {
  Serial.begin(SERIAL_BAUD);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
#ifdef IS_RFM69HW
  radio.setHighPower(); //uncomment only for RFM69HW!
#endif
  radio.setHighPower();
  radio.setPowerLevel(3);
  radio.encrypt(ENCRYPTKEY);
  char buff[50];
  sprintf(buff, "\nTransmitting at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(buff);
  
  
}







void loop() 
{
  
  if ( doxmit >0 )  // do this if if sleeptimer has countdown or minxmit has countdown 
  {     
        doxmit=0; // reset the flag
        cyclecounter=0 ; //reset the minxmit flag too
        
        long temperature = radio.readTemperature(-1);
        long distance    = getDistance();
        long boltage     = getboltage();
             
        if (distance >= 400 || distance <= 0)
         {
          Serial.println("Out of range:");
          Serial.print(distance);
          distance=999;
         }
        else 
         {
           Serial.print(distance);
           Serial.print(" cm");
           Serial.print(" Temp ");
           Serial.print(temperature);
            
         }
        sprintf(payload, "SUMP:%ld TEMP:%ld", distance,temperature);
        
        
        
    
        if (radio.ACK_REQUESTED)
          {
            radio.sendACK();
            Serial.print(" - ACK sent");
            delay(5);
          }
         sendSize=strlen(payload);
         
         if (prevtemperature != temperature)      { datachanged|(1<<bit_temperature) ; prevtemperature = temperature; }
         if (prevboltage     != boltage    )      { datachanged|(1<<bit_voltage) ;     prevboltage     = boltage;     }
         if (prevdistance    != distance   )      { datachanged|(1<<bit_data0) ;       prevdistance    = distance;    }
         
         
         if (radio.sendWithRetry(GATEWAYID, payload, sendSize))
            Serial.println(" ok ");
         else Serial.println(" noackrcvd");
          
    
             //         We just did transmit 
            //    Add listen for 200ms code here
           //     this will mess up the sleep in seconds for 1 go around ....
          //
//         radio.sleep();
//         LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);

          //  while ( !radio.receiveDone())
          //  {
          //      delay(1); // sleep here for some milliseconds
          //
          //      if (radio.ACK_REQUESTED)
           //     {
           //       radio.sendACK();
           //       Serial.print(" - ACK sent");
           //       delay(10);
            //    }
            //
            //}
         
  }
  else  //came here since doxmit is false
  {
     // we came cause we did not enter previous loop - work on sleepseconds 
     if (sleepsecondscounter++ == sleepseconds) 
     {
       doxmit=1;  //set xmit flag to transmit depending on xmitchange
       sleepsecondscounter=0;
     }
     if ( cyclecounter++ == cycle )
     {
       doxmit=2; //set xmit flag to must transmit - we will reset this counter in the xmit body
       sleepsecondscounter=0; //if we xmit lets go back to sleep for sleepseconds
     }
     
     
  }
  
    //sleep radio and cpu at end of loop for 1s
    radio.sleep();
    LowPower.powerDown(SLEEP_1S, ADC_OFF, BOD_OFF);
  
}


void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
}

long getDistance() {
  long duration, distance;
  digitalWrite(TRIG, LOW);
  delayMicroseconds(2);
  digitalWrite(TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(TRIG, LOW);
  duration = pulseIn(ECHO, HIGH);
  distance = (duration/2) / 29.1;
  return distance;
}

long getboltage() {
  
  return 0;
}
