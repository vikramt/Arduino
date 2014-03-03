
#include <RFM69.h>
#include <SPI.h>
#include <SPIFlash.h>
#include <EEPROMex.h>

#define NODEID 1 //unique for each node on same network
#define NETWORKID 10 //the same on all nodes that talk to each other
//Match frequency to the hardware version of the radio on your Moteino (uncomment one):
#define FREQUENCY RF69_433MHZ
//#define FREQUENCY RF69_868MHZ
//#define FREQUENCY RF69_915MHZ
#define ENCRYPTKEY "sampleEncryptKey" //exactly the same 16 characters/bytes on all nodes!
#define IS_RFM69HW 
#define ACK_TIME 30 // max # of ms to wait for an ack
#define LED 9 // Moteinos have LEDs on D9
#define SERIAL_BAUD 115200

static char payload[32]=             { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};
static char serialInputBuffer[32] =  { 0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0};


RFM69 radio;
SPIFlash flash(8, 0xEF30); //EF40 for 16mbit windbond chip
bool promiscuousMode = false; //set to 'true' to sniff all packets on the same network

void setup() {
  delay(200);delay(200);delay(200);delay(200);delay(200);delay(200);delay(200);delay(200);delay(200);delay(200);
  Serial.begin(SERIAL_BAUD);
  delay(10);
  radio.initialize(FREQUENCY,NODEID,NETWORKID);
#ifdef IS_RFM69HW
  radio.setHighPower(); //uncomment only for RFM69HW!
#endif
  radio.encrypt(ENCRYPTKEY);
  radio.promiscuous(promiscuousMode);
  char buff[50];
  sprintf(buff, "\nListening at %d Mhz...", FREQUENCY==RF69_433MHZ ? 433 : FREQUENCY==RF69_868MHZ ? 868 : 915);
  Serial.println(buff);
  if (flash.initialize())
    Serial.println("SPI Flash Init OK!");
  else
    Serial.println("SPI Flash Init fail - no chip?");
}

byte ackCount=0;
byte goodcommand = 0 ;


void loop() {
  //process any serial input
  char i  = 0;
  if ( Serial.available()) {
    while ( Serial.available() && i <31 ) {
      delay(1);
      serialInputBuffer[i++]=Serial.read();
    } 
    serialInputBuffer[i]= 0;
  }
  //if ( i > 0 ) {
  //  Serial.println((char*)serialInputBuffer);
  //  serialInputBuffer[0]=0;
  //}
    if (serialInputBuffer[0] == 'p')
    {
      promiscuousMode = !promiscuousMode;
      radio.promiscuous(promiscuousMode);
      Serial.print("Promiscuous mode ");Serial.println(promiscuousMode ? "on" : "off");
      serialInputBuffer[0]=0; //command completed
    }
 
   if (serialInputBuffer[0] == 's')
    {
      
      serialInputBuffer[0]=0; //command completed
    }

    if (serialInputBuffer[0] == 't')
    {
      byte temperature = radio.readTemperature(-1); // -1 = user cal factor, adjust for correct ambient
      byte fTemp = 1.8 * temperature + 32; // 9/5=1.8
      Serial.print( "Radio Temp is ");
      Serial.print(temperature);
      Serial.print("C, ");
      Serial.print(fTemp); //converting to F loses some resolution, obvious when C is on edge between 2 values (ie 26C=78F, 27C=80F)
      Serial.println('F');
      serialInputBuffer[0]=0; //command completed
    }
  

  if (radio.receiveDone())
  {
    Serial.print('[');Serial.print(radio.SENDERID, DEC);Serial.print("] ");
    if (promiscuousMode)
    {
      Serial.print("to [");Serial.print(radio.TARGETID, DEC);Serial.print("] ");
    }
    for (byte i = 0; i < radio.DATALEN; i++)
      Serial.print((char)radio.DATA[i]);
    Serial.print(" [RX_RSSI:");Serial.print(radio.RSSI);Serial.print("]");
    
    if (radio.ACK_REQUESTED)
    {
      byte theNodeID = radio.SENDERID;
      radio.sendACK();
      Serial.print(" - ACK sent."); 
    }
    Serial.println();
    
    if ( goodcommand == 1 ) {
       goodcommand=0;
       payload[0]=0;
       sprintf(payload,"%c:%c:%c", 't','r','s');
       Serial.print(" Sending to ");
       //Serial.print(payload);
    }
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
