/*Below is the working code for the FONA Shield with the Hyperduino
 * I scrapped different parts of the code from many different projects
 * This means if there are any comments that 
 * Most of the code and actions can be seen in the Serial Monitor
 * BEFORE YOU DO ANYTHING AND UPLOAD YOU MUST SCROLL TO THE BOTTOM AND PUT YOUR PHONE NUMBER IN THE DESIGNATED SPOT IN ORDER TO GET IT WORKING
 * I plan on in the future adding a few lines of code that will enable it to send a text if it is unable to send the GPS location so stay tuned for that.
*/

#include "Adafruit_FONA.h"
#define FONA_RX 2
#define FONA_TX 3
#define FONA_RST 4

// this is a large buffer for replies
char replybuffer[255];
#include <SoftwareSerial.h>
SoftwareSerial fonaSS = SoftwareSerial(FONA_TX, FONA_RX);
SoftwareSerial *fonaSerial = &fonaSS;

Adafruit_FONA fona = Adafruit_FONA(FONA_RST);

uint8_t readline(char *buff, uint8_t maxbuff, uint16_t timeout = 0);

uint8_t type;

void setup() {
  while (!Serial);

  Serial.begin(115200);
  Serial.println(F("FONA basic test"));
  Serial.println(F("Initializing....(May take 3 seconds)")); 

  fonaSerial->begin(4800);
  if (! fona.begin(*fonaSerial)) {
    Serial.println(F("Couldn't find FONA")); //This should only display if the FONA has bad soldering. 
    while (1);
  }
  type = fona.type();
  Serial.println(F("FONA is OK")); 
  Serial.print(F("Found "));
  switch (type) {
    case FONA800L:
      Serial.println(F("FONA 800L")); break;
    case FONA800H:
      Serial.println(F("FONA 800H")); break;
    case FONA808_V1:
      Serial.println(F("FONA 808 (v1)")); break;
    case FONA808_V2:
      Serial.println(F("FONA 808 (v2)")); break; //This is the one I am using. 
    case FONA3G_A:
      Serial.println(F("FONA 3G (American)")); break;
    case FONA3G_E:
      Serial.println(F("FONA 3G (European)")); break;
    default: 
      Serial.println(F("???")); break; //If this prints you will have a problem with everything else working. 
  }
  
  // Print module IMEI number. This is not important I just like to keep it there. 
  char imei[16] = {0}; // MUST use a 16 character buffer for IMEI!
  uint8_t imeiLen = fona.getIMEI(imei);
  if (imeiLen > 0) {
    Serial.print("Module IMEI: "); Serial.println(imei);
  }
  Serial.println(F("Enabling GPS..."));
  fona.enableGPS(true);
  delay(4000);
}
  char fonaNotificationBuffer[64];          //for notifications from the FONA
  char smsBuffer[250];
void loop() {
  char* bufPtr = fonaNotificationBuffer;

    if (fona.available())      //any data available from the FONA?
  {
    int slot = 0;            //this will be the slot number of the SMS 
    int charCount = 0;
    //Read the notification into fonaInBuffer
    do  {
      *bufPtr = fona.read();
      Serial.write(*bufPtr);
      delay(1);
    } while ((*bufPtr++ != '\n') && (fona.available()) && (++charCount < (sizeof(fonaNotificationBuffer)-1)));

    *bufPtr = 0;
       if (1 == sscanf(fonaNotificationBuffer, "+CMTI: " FONA_PREF_SMS_STORAGE ",%d", &slot)) {
      Serial.print("slot: "); Serial.println(slot);
      
      char callerIDbuffer[32];  //we'll store the SMS sender number in here
      
      // Retrieve SMS sender address/phone number.
      if (! fona.getSMSSender(slot, callerIDbuffer, 31)) {
        Serial.println("Didn't find SMS message in slot!");
      }
      Serial.print(F("FROM: ")); Serial.println(callerIDbuffer);

        // Retrieve SMS value.
        uint16_t smslen;
        if (fona.readSMS(slot, smsBuffer, 250, &smslen)) { // pass in buffer and max len!
          Serial.println(smsBuffer);
        }
  float latitude, longitude;//variables to hold initial GPS readings
  boolean gps_success = fona.getGPS(&latitude, &longitude);
  if (gps_success) {
    Serial.print("GPS lat:");
    Serial.println(latitude, 6);
    Serial.print("GPS long:");
    Serial.println(longitude, 6); 
          //send sms
      char message[141];  
      char LAT1[10];//string of lat and long unparsed & overflowing bound
      char LAT[10];
      char LONG[10];
      dtostrf(latitude, 5, 4, LAT1); //gathering GPS data in a format that can be sent
      dtostrf(longitude, 5, 4, LONG);
      
      //initialize desired array from unparsed array
      for(int i = 0; i < 9; i++) {
      LAT[i] = LAT1[i];
      }
      LAT[9] = '\0';//truncate array at last desired value
      sprintf(message, "Sending GPS Cords Now https://www.google.com/maps?q=%s,%s", LAT, LONG); //The final message that will be sent by the FONA
      
      Serial.println(LAT);Serial.println(LAT1);Serial.println(LONG);
      Serial.println(message) ;    //prints the message in the serial monitor before sending
 
      char sendto[13] = "1112223333"; //put the desired destination phone number for sms here.
      
      fona.sendSMS(sendto, message) ; //This will finally send the message. 
  }
       }
  }