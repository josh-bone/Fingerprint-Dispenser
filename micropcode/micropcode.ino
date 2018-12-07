/*************************************************** 
  Master code for feminine hygiene dispenser
  Boston University EK210 J1, Group A
  Written by Joshua Bone, 2018
  Code derived from github examples by Limor Fried/Ladyada at Adafruit Industries. 
 ****************************************************/

#include <Adafruit_Fingerprint.h> //handle fingerprint sensor
#include <RTClib.h> //handle real time clock
// pin #2 is IN from sensor (GREEN wire)
// pin #3 is OUT from arduino  (WHITE wire)
#include <SoftwareSerial.h>
SoftwareSerial mySerial(2, 3);

Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial); //using sensor without password

uint8_t id = 1; //note, uint8_t maxes out at 255, so only 255 fingerprints can be stored

RTC_DS1307 rtc;
DateTime last;

void setup()  
{
  Serial.begin(9600);
  while (!Serial); 
  delay(100);
  Serial.println("\n\nAdafruit finger detect test");

  // set the data rate for the sensor serial port
  finger.begin(57600);
  if (! rtc.begin()) {
    Serial.println("Couldn't find RTC");
    while (1);
  }
  
  finger.getTemplateCount();
  Serial.print("Sensor contains "); Serial.print(finger.templateCount); Serial.println(" templates");
  Serial.println("Waiting for valid finger...");

  pinMode(6, OUTPUT); //turns motor
  digitalWrite(6, HIGH);

  //set starting id
  if(finger.templateCount > 250){ //empty if database is near full (255)
      finger.emptyDatabase();
      last = rtc.now();
  }
  else
    id = finger.templateCount + 1; //avoid overwriting current fingerprints
}

void loop()                     // run over and over again
{ 
  uint8_t s = getFingerprintID(); 
  delay(50);            //don't need to run this at full speed.
}

uint8_t getFingerprintID() { 
  uint8_t p = finger.getImage();

  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      //Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      //Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      //Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!
  
  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      //Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      //Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      //Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      //Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      //Serial.println("Could not find fingerprint features");
      return p;
    default:
      //Serial.println("Unknown error");
      return p;
  }
  
  // OK converted!
  p = finger.fingerFastSearch();
  if (p == FINGERPRINT_OK) {
    //Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    //Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    //enrolling
    //check if a day has passed since last library clear
    DateTime now = rtc.now();
    Serial.println("Checking time...");
    if(now.unixtime() > last.unixtime() + 86400 ){ //if a day has passed since we recorded last
      finger.emptyDatabase();
      last = rtc.now();
    }
  
    id++;
    Serial.println("enrolling...");
    Serial.print("Creating model for #");  Serial.println(id);
    
    p = finger.createModel();
    p = finger.storeModel(id);
    if (p == FINGERPRINT_OK) {
      //Stored
      //dispense
        Serial.println("dispensing...");
        digitalWrite(6, LOW); //start motor
        delay(4000); //modify this time after testing
        digitalWrite(6, HIGH); //stop motor
    } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
      //Serial.println("Communication error");
      return p;
    } else if (p == FINGERPRINT_BADLOCATION) {
      //Serial.println("Could not store in that location");
      return p;
    } else if (p == FINGERPRINT_FLASHERR) {
      //Serial.println("Error writing to flash");
      return p;
    } else {
      //Serial.println("Unknown error");
      return p;
    }   
    return p; //flag for dispensing
  } else {
    //Serial.println("Unknown error");
    return p;
  }   
  
  // found a match!
  //Serial.print("Found ID #"); Serial.print(finger.fingerID); 
  //Serial.print(" with confidence of "); Serial.println(finger.confidence); 

  return finger.fingerID;
}
