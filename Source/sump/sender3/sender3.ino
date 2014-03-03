#include <RFM69.h>
#include <RFM69registers.h>
#include <LowPower.h>
#include <RFM69.h>
#include <SPI.h>
#include <SPIFlash.h>

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
#define ECHO 4
#define TRIG 5
#define SAMPLESCOUNT 6



char payload[61] ;
char distancestring[30];
char temperaturestring[30];
int distancearray[20];
int temperaturearray[20];
char buff[20];
byte sendSize=0;
boolean requestACK = false;
SPIFlash flash(8, 0xEF30); //EF40 for 16mbit windbond chip
RFM69 radio;

void setup() {
  delay(5000);
  Serial1.begin(SERIAL_BAUD);
  pinMode(TRIG, OUTPUT);
  pinMode(ECHO, INPUT);
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
#ifdef IS_RFM69HW
  radio.setHighPower(); //uncomment only for RFM69HW!
#endif
  radio.setHighPower();
  radio.setPowerLevel(10);
  radio.encrypt(ENCRYPTKEY);
  char buff[50];
  sprintf(buff, "\nTransmitting at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial1.println(buff);
  
  
}

byte cycle = 0;
byte i;
void loop() {
  
  
        int temperature = radio.readTemperature(5);
        int distance = getDistance();
        distancearray[cycle]=distance;
        temperaturearray[cycle]=temperature;
        cycle++;
      if (distance >= 400 || distance <= 0){
        Serial1.println("Out of range:");
        Serial1.print(distance);
        //sprintf(payload, "SUMP:ooRange TEMP:%ld", temperature);
      }
      else {
        Serial1.print(distance);
        Serial1.print(" cm");
        Serial1.print(" Temp ");
        Serial1.print(temperature);
        //sprintf(payload, "SUMP:%ld TEMP:%ld", distance,temperature);  
      }
      //Blink(LED, 50);
      
  

       if ( cycle ==  SAMPLESCOUNT  ) {
          cycle=0;
         distancestring[0]=0;
         temperaturestring[0]=0;
         for (byte i = 0 ; i < SAMPLESCOUNT -1 ; i++ ) {
               payload[0]=0;
               sprintf(payload, "%d,", distancearray[i]);
               strcat(distancestring,payload);
               payload[0]=0;
               sprintf(payload, "%d,", temperaturearray[i]);
               strcat(temperaturestring,payload);
         }
         payload[0]=0;
         strcat(payload,"SMP:");
         strcat(payload,distancestring);
         strcat (payload, " TMP:");
         strcat(payload,temperaturestring);     
         sendSize=strlen(payload);
        if (radio.sendWithRetry(GATEWAYID, payload, sendSize))
         Serial1.println(" ok ");
        else Serial1.println(" noackrcvd");
       }
       if (radio.ACK_REQUESTED)
        {
          radio.sendACK();
          Serial.print(" - ACK sent");
          delay(10);
        }
  
  radio.sleep();
  LowPower.powerDown(SLEEP_4S, ADC_OFF, BOD_OFF);
}


void Blink(byte PIN, int DELAY_MS)
{
  pinMode(PIN, OUTPUT);
  digitalWrite(PIN,HIGH);
  delay(DELAY_MS);
  digitalWrite(PIN,LOW);
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
