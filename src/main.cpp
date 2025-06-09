//This code plays the loaded wav files on a schedule AND
//will tell the TPL510 that it's done when Sound Off is trigered
#define SOFTTWARE_VERSION "v2.1"
#define USE_MTP 0

//LIBRARIES
#include <Arduino.h>
#include <TimeLib.h>
#include <TimeAlarms.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_UART.h"
#include <SDConfig.h>
#include <SD.h> //for MTP
//#include <MTP_Teensy.h> //for MTP

//Wav Player Setup
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Entropy.h>
#include <string>
#include <array>
using namespace std;
AudioPlaySdWav           playWav1;
AudioOutputI2S           audioOutput;
AudioConnection          patchCord1(playWav1, 0, audioOutput, 0);
AudioConnection          patchCord2(playWav1, 1, audioOutput, 1);
AudioControlSGTL5000     sgtl5000_1;

//Bluefruit setup
#define FACTORYRESET_ENABLE         0
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"
#define BLUEFRUIT_UART_MODE_PIN     -1 //the following sets the optional Mode pin, its recommended but not required
#define VERBOSE_MODE                false
Adafruit_BluefruitLE_UART ble(Serial3, BLUEFRUIT_UART_MODE_PIN);

//STATIC DEFINITIONS
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14
#define WAIT_AFTER_PLAY_MS 250
#define SEC_PRE_MIDNIGHT 86399
#define MIDNIGHT_IN_SEC 86400
int done_pin = 17;
int mos_pwr = 3;
int mos_audio = 2;

//Settings from Config file
// Alarm times
char *ALARM_1;
char *ALARM_2;
char *ALARM_3;
char *ALARM_4;
char *ALARM_5;
char *ALARM_6;
char *ALARM_7;
char *ALARM_8;
char *ALARM_9;
char *ALARM_10;
char *ALARM_11;
char *ALARM_12;
char *ALARM_13;
char *ALARM_14;
char *ALARM_15;
char *ALARM_16;
char *ALARM_17;
char *ALARM_18;
char *ALARM_19;
char *ALARM_20;
char *ALARM_21;
char *ALARM_22;
char *ALARM_23;
char *ALARM_24;

//SOUND FILE BASE NAMES
char *ALARM_1_FILE_BASE; 
char *ALARM_2_FILE_BASE; 
char *ALARM_3_FILE_BASE; 
char *ALARM_4_FILE_BASE; 
char *ALARM_5_FILE_BASE; 
char *ALARM_6_FILE_BASE; 
char *ALARM_7_FILE_BASE; 
char *ALARM_8_FILE_BASE; 
char *ALARM_9_FILE_BASE; 
char *ALARM_10_FILE_BASE; 
char *ALARM_11_FILE_BASE; 
char *ALARM_12_FILE_BASE; 
char *ALARM_13_FILE_BASE;
char *ALARM_14_FILE_BASE;
char *ALARM_15_FILE_BASE;
char *ALARM_16_FILE_BASE;
char *ALARM_17_FILE_BASE;
char *ALARM_18_FILE_BASE;
char *ALARM_19_FILE_BASE; 
char *ALARM_20_FILE_BASE; 
char *ALARM_21_FILE_BASE; 
char *ALARM_22_FILE_BASE; 
char *ALARM_23_FILE_BASE; 
char *ALARM_24_FILE_BASE; 

// Wake Time
char * WAKE_TIME;

// Sleep Time
char * SLEEP_TIME; //std::string sleep_time = "14:58:00";

//Other
int BAUDE_RATE;
int NUM_SAMP; 
bool USE_SAMP;
bool ALWAYS_ON;
float VOLUME;
boolean didReadConfig;
boolean SERIAL_WAIT;

// initialize the sample number as global variable
int sampleNumber = 0;

// SD Logging file instantiation
File myFile;
String active_file = "no file";

// SD Config instatiations 
const char CONFIG_FILE[] = "config.cfg";
boolean readConfiguration();

/////////////
//FUNCTIONS//
/////////////

//Digital Clock Code
void printDigits(int digits) {
  // utility function for digital clock display on Serial monitor: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
}
void digitalClockDisplay() {
  // digital clock display of the time
  Serial.print(hour());
  printDigits(minute());
  printDigits(second());
}
time_t getTeensy3Time() {
  return Teensy3Clock.get();
}

// Create a string of the date and time for logging
String present(){
  String s = "/";
  String c = ":";
  String space = " ";

  String date = month()+s+day()+s+year();
  String thyme = hour()+c+minute()+c+second();
  String thisMagicMoment = date + space + thyme;
  
  return thisMagicMoment;
}

// LOGGING AND PRINTING
//Log input to SD Card
void logSD(std::string string) {
  myFile = SD.open("LOG.txt", FILE_WRITE);
  // if the file opened okay, write to it:
  if (myFile) {
    myFile.print(present());
    myFile.print("  ");
    myFile.println(string.c_str());
    myFile.close();
    // if the file didn't open, print an error:
  } else {
    Serial.println("error opening file");
  }
}
// Wrap logging and serial print into a single function (now takes a std string and converts to char *)
void printAndLog(std::string string){
  Serial.println(string.c_str());
  logSD(string);
  }

// Add an integer to a string
const char * customAdd(std::string string, int b){
    std::string concat = string + std::to_string(b);
    const char * result = concat.c_str(); //convert string to pointer
    return result;  
}

// construct playback file name from an hour + sample number (e.g. 18TKP1.wav) as a string
std::string makeFileNameString(std::string file_base, int samp, bool use_samp){
  if(use_samp == true){
    std::string result = file_base + std::to_string(samp) + ".wav";  
    return result;    
  }
  else{
    std::string result = file_base + ".wav";  
    return result;       
  }
}

// WAV FILE PLAYER AND TPL5110 HELPER FUNCTIONS
// playFile function from WAV file player
void playFile(std::string filename) { //const char string[]
  printAndLog("Playing file:");
  printAndLog(filename);
  playWav1.play(filename.c_str());
  active_file = filename.c_str();
  delay(10);
}
// Turn off sound
void stopFile() {
  printAndLog("Stopping audio"); //ths goes at end of stopFile block. putting here for testing.
  playWav1.stop();
  delay(250);
}
//TPL5110 done
void doneSignal() {
  //turn off power MOSFETS
  digitalWrite(mos_audio, LOW);
  digitalWrite(mos_pwr, LOW );
  delay(1000);
  printAndLog("Send done signal to TPL5110");
  digitalWrite(done_pin, HIGH);
  delay(1000);
  digitalWrite(done_pin, LOW);
}

//Function to extract integers from strings of form "hh:mm:ss"
std::array<int,3> timeConstruct(std::string timeString){
  std::array<int,3> timeInts;
  std::string hrString = timeString.substr(0,2);
  std::string minString = timeString.substr(3,4);
  std::string secString = timeString.substr(6,7);
  timeInts[0] = stoi(hrString);
  timeInts[1] = stoi(minString);
  timeInts[2] = stoi(secString);

  return timeInts;
}

// Read Config function
// Read our settings from our SD configuration file.
//Returns true if successful, false if it failed.
boolean readConfiguration() {
  const uint8_t CONFIG_LINE_LENGTH = 127;
  
  // The open configuration file.
  SDConfig cfg;
  
  // Open the configuration file.
  if (!cfg.begin(CONFIG_FILE, CONFIG_LINE_LENGTH)) {
    Serial.print("Failed to open configuration file: ");
    Serial.println(CONFIG_FILE);
    return false;
  }
  
  // Read each setting from the file.
  while (cfg.readNextSetting()) {
    
    // Put a nameIs() block here for each setting you have.
    // Alarm times
    if (cfg.nameIs("ALARM_1")) {     
      ALARM_1 = cfg.copyValue();
      Serial.print("ALARM_1: ");
      Serial.println(ALARM_1);      
    }
    
    else if (cfg.nameIs("ALARM_2")) {     
      ALARM_2 = cfg.copyValue();
      Serial.print("ALARM_2: ");
      Serial.println(ALARM_2);
    }    

    else if (cfg.nameIs("ALARM_3")) {     
      ALARM_3 = cfg.copyValue();
      Serial.print("ALARM_3: ");
      Serial.println(ALARM_3);      
    }

    else if (cfg.nameIs("ALARM_4")) {     
      ALARM_4 = cfg.copyValue();
      Serial.print("ALARM_4: ");
      Serial.println(ALARM_4);
    }    

    else if (cfg.nameIs("ALARM_5")) {     
      ALARM_5 = cfg.copyValue();
      Serial.print("ALARM_5: ");
      Serial.println(ALARM_5);
    }
  
      else if (cfg.nameIs("ALARM_6")) {     
      ALARM_6 = cfg.copyValue();
      Serial.print("ALARM_6: ");
      Serial.println(ALARM_6);
    }    

    else if (cfg.nameIs("ALARM_7")) {     
      ALARM_7 = cfg.copyValue();
      Serial.print("ALARM_7: ");
      Serial.println(ALARM_7);
    }

    else if (cfg.nameIs("ALARM_8")) {     
      ALARM_8 = cfg.copyValue();
      Serial.print("ALARM_8: ");
      Serial.println(ALARM_8);
    }    

    else if (cfg.nameIs("ALARM_9")) {     
      ALARM_9 = cfg.copyValue();
      Serial.print("ALARM_9: ");
      Serial.println(ALARM_9);
    }

    else if (cfg.nameIs("ALARM_10")) {     
      ALARM_10 = cfg.copyValue();
      Serial.print("ALARM_10: ");
      Serial.println(ALARM_10);
    }

    else if (cfg.nameIs("ALARM_11")) {     
      ALARM_11 = cfg.copyValue();
      Serial.print("ALARM_11: ");
      Serial.println(ALARM_11);
    }    

    else if (cfg.nameIs("ALARM_12")) {     
      ALARM_12 = cfg.copyValue();
      Serial.print("ALARM_12: ");
      Serial.println(ALARM_12);
    }

    else if (cfg.nameIs("ALARM_13")) {     
      ALARM_13 = cfg.copyValue();
      Serial.print("ALARM_13: ");
      Serial.println(ALARM_13);      
    }
    
    else if (cfg.nameIs("ALARM_14")) {     
      ALARM_14 = cfg.copyValue();
      Serial.print("ALARM_14: ");
      Serial.println(ALARM_14);
    }    

    else if (cfg.nameIs("ALARM_15")) {     
      ALARM_15 = cfg.copyValue();
      Serial.print("ALARM_15: ");
      Serial.println(ALARM_15);      
    }

    else if (cfg.nameIs("ALARM_16")) {     
      ALARM_16 = cfg.copyValue();
      Serial.print("ALARM_16: ");
      Serial.println(ALARM_16);
    }    

    else if (cfg.nameIs("ALARM_17")) {     
      ALARM_17 = cfg.copyValue();
      Serial.print("ALARM_17: ");
      Serial.println(ALARM_17);
    }
  
      else if (cfg.nameIs("ALARM_18")) {     
      ALARM_18 = cfg.copyValue();
      Serial.print("ALARM_18: ");
      Serial.println(ALARM_18);
    }    

    else if (cfg.nameIs("ALARM_19")) {     
      ALARM_19 = cfg.copyValue();
      Serial.print("ALARM_19: ");
      Serial.println(ALARM_19);
    }

    else if (cfg.nameIs("ALARM_20")) {     
      ALARM_20 = cfg.copyValue();
      Serial.print("ALARM_20: ");
      Serial.println(ALARM_20);
    }    

    else if (cfg.nameIs("ALARM_21")) {     
      ALARM_21 = cfg.copyValue();
      Serial.print("ALARM_21: ");
      Serial.println(ALARM_21);
    }

    else if (cfg.nameIs("ALARM_22")) {     
      ALARM_22 = cfg.copyValue();
      Serial.print("ALARM_22: ");
      Serial.println(ALARM_22);
    }

    else if (cfg.nameIs("ALARM_23")) {     
      ALARM_23 = cfg.copyValue();
      Serial.print("ALARM_23: ");
      Serial.println(ALARM_23);
    }    

    else if (cfg.nameIs("ALARM_24")) {     
      ALARM_24 = cfg.copyValue();
      Serial.print("ALARM_24: ");
      Serial.println(ALARM_24);

    }
    
    else if (cfg.nameIs("ALARM_1_FILE_BASE")) {     
      ALARM_1_FILE_BASE = cfg.copyValue();
      Serial.print("ALARM_1_FILE_BASE: ");
      Serial.println(ALARM_1_FILE_BASE);
    }  

    else if (cfg.nameIs("ALARM_2_FILE_BASE")) {     
      ALARM_2_FILE_BASE = cfg.copyValue();
      Serial.print("ALARM_2_FILE_BASE: ");
      Serial.println(ALARM_2_FILE_BASE);
    }    

    else if (cfg.nameIs("ALARM_3_FILE_BASE")) {     
      ALARM_3_FILE_BASE = cfg.copyValue();
      Serial.print("ALARM_3_FILE_BASE: ");
      Serial.println(ALARM_3_FILE_BASE);      
    }

    else if (cfg.nameIs("ALARM_4_FILE_BASE")) {     
      ALARM_4_FILE_BASE = cfg.copyValue();
      Serial.print("ALARM_4_FILE_BASE: ");
      Serial.println(ALARM_4_FILE_BASE);
    }    

    else if (cfg.nameIs("ALARM_5_FILE_BASE")) {     
      ALARM_5_FILE_BASE = cfg.copyValue();
      Serial.print("ALARM_5_FILE_BASE: ");
      Serial.println(ALARM_5_FILE_BASE);
    }
  
      else if (cfg.nameIs("ALARM_6_FILE_BASE")) {     
      ALARM_6_FILE_BASE = cfg.copyValue();
      Serial.print("ALARM_6_FILE_BASE: ");
      Serial.println(ALARM_6_FILE_BASE);
    }    

    else if (cfg.nameIs("ALARM_7_FILE_BASE")) {     
      ALARM_7_FILE_BASE = cfg.copyValue();
      Serial.print("ALARM_7_FILE_BASE: ");
      Serial.println(ALARM_7_FILE_BASE);
    }

    else if (cfg.nameIs("ALARM_8_FILE_BASE")) {     
      ALARM_8_FILE_BASE = cfg.copyValue();
      Serial.print("ALARM_8_FILE_BASE: ");
      Serial.println(ALARM_8_FILE_BASE);
    }    

    else if (cfg.nameIs("ALARM_9_FILE_BASE")) {     
      ALARM_9_FILE_BASE = cfg.copyValue();
      Serial.print("ALARM_9_FILE_BASE: ");
      Serial.println(ALARM_9_FILE_BASE);
    }

    else if (cfg.nameIs("ALARM_10_FILE_BASE")) {     
      ALARM_10_FILE_BASE = cfg.copyValue();
      Serial.print("ALARM_10_FILE_BASE: ");
      Serial.println(ALARM_10_FILE_BASE);
    }

    else if (cfg.nameIs("ALARM_11_FILE_BASE")) {     
      ALARM_11_FILE_BASE = cfg.copyValue();
      Serial.print("ALARM_11_FILE_BASE: ");
      Serial.println(ALARM_11_FILE_BASE);
    }    

    else if (cfg.nameIs("ALARM_12_FILE_BASE")) {     
      ALARM_12_FILE_BASE = cfg.copyValue();
      Serial.print("ALARM_12_FILE_BASE: ");
      Serial.println(ALARM_12_FILE_BASE);
    }

    else if (cfg.nameIs("ALARM_13_FILE_BASE")) {     
      ALARM_13_FILE_BASE = cfg.copyValue();
      Serial.print("ALARM_13_FILE_BASE: ");
      Serial.println(ALARM_13_FILE_BASE);      
    }
    
    else if (cfg.nameIs("ALARM_14_FILE_BASE")) {     
      ALARM_14_FILE_BASE = cfg.copyValue();
      Serial.print("ALARM_14_FILE_BASE: ");
      Serial.println(ALARM_14_FILE_BASE);
    }    

    else if (cfg.nameIs("ALARM_15_FILE_BASE")) {     
      ALARM_15_FILE_BASE = cfg.copyValue();
      Serial.print("ALARM_15_FILE_BASE: ");
      Serial.println(ALARM_15_FILE_BASE);      
    }

    else if (cfg.nameIs("ALARM_16_FILE_BASE")) {     
      ALARM_16_FILE_BASE = cfg.copyValue();
      Serial.print("ALARM_16_FILE_BASE: ");
      Serial.println(ALARM_16_FILE_BASE);
    }    

    else if (cfg.nameIs("ALARM_17_FILE_BASE")) {     
      ALARM_17_FILE_BASE = cfg.copyValue();
      Serial.print("ALARM_17_FILE_BASE: ");
      Serial.println(ALARM_17_FILE_BASE);
    }
  
      else if (cfg.nameIs("ALARM_18_FILE_BASE")) {     
      ALARM_18_FILE_BASE = cfg.copyValue();
      Serial.print("ALARM_18_FILE_BASE: ");
      Serial.println(ALARM_18_FILE_BASE);
    }    

    else if (cfg.nameIs("ALARM_19_FILE_BASE")) {     
      ALARM_19_FILE_BASE = cfg.copyValue();
      Serial.print("ALARM_19_FILE_BASE: ");
      Serial.println(ALARM_19_FILE_BASE);
    }

    else if (cfg.nameIs("ALARM_20_FILE_BASE")) {     
      ALARM_20_FILE_BASE = cfg.copyValue();
      Serial.print("ALARM_20_FILE_BASE: ");
      Serial.println(ALARM_20_FILE_BASE);
    }    

    else if (cfg.nameIs("ALARM_21_FILE_BASE")) {     
      ALARM_21_FILE_BASE = cfg.copyValue();
      Serial.print("ALARM_21_FILE_BASE: ");
      Serial.println(ALARM_21_FILE_BASE);
    }

    else if (cfg.nameIs("ALARM_22_FILE_BASE")) {     
      ALARM_22_FILE_BASE = cfg.copyValue();
      Serial.print("ALARM_22_FILE_BASE: ");
      Serial.println(ALARM_22_FILE_BASE);
    }

    else if (cfg.nameIs("ALARM_23_FILE_BASE")) {     
      ALARM_23_FILE_BASE = cfg.copyValue();
      Serial.print("ALARM_23_FILE_BASE: ");
      Serial.println(ALARM_23_FILE_BASE);
    }    

    else if (cfg.nameIs("ALARM_24_FILE_BASE")) {     
      ALARM_24_FILE_BASE = cfg.copyValue();
      Serial.print("ALARM_24_FILE_BASE: ");
      Serial.println(ALARM_24_FILE_BASE);
    }

    // Baude Rate
    else if (cfg.nameIs("BAUDE_RATE")) { 
      BAUDE_RATE = cfg.getIntValue();
      Serial.print("BAUDE_RATE: ");
      Serial.println(BAUDE_RATE);
    }
    
    // Wake time
    else if (cfg.nameIs("WAKE_TIME")) { 
      WAKE_TIME = cfg.copyValue();
      Serial.print("WAKE_TIME: ");
      Serial.println(WAKE_TIME);
    }

    else if (cfg.nameIs("SLEEP_TIME")) { 
      SLEEP_TIME = cfg.copyValue();
      Serial.print("SLEEP_TIME: ");
      Serial.println(SLEEP_TIME);
    }
    
    else if (cfg.nameIs("NUM_SAMP")) { 
      NUM_SAMP = cfg.getIntValue();
      Serial.print("NUM_SAMP: ");
      Serial.println(NUM_SAMP);
    }

    else if (cfg.nameIs("USE_SAMP")) { 
      USE_SAMP = cfg.getBooleanValue();
      Serial.print("USE_SAMP: ");
      Serial.println(USE_SAMP);
    }

    else if (cfg.nameIs("ALWAYS_ON")) { 
      ALWAYS_ON = cfg.getBooleanValue();
      Serial.print("ALWAYS_ON: ");
      Serial.println(ALWAYS_ON);
    }

    else if (cfg.nameIs("SERIAL_WAIT")) { 
      SERIAL_WAIT = cfg.getBooleanValue();
      Serial.print("SERIAL_WAIT: ");
      Serial.println(SERIAL_WAIT);
    }

    else if (cfg.nameIs("VOLUME")) { 
      VOLUME = atof(cfg.getValue());
      Serial.print("VOLUME: ");
      Serial.println(VOLUME);
    }

    else {
      // report unrecognized names.
      Serial.print("Unknown name in config: ");
      Serial.println(cfg.getName());
    }
  }
  
  // clean up
  cfg.end();
  
  return true;
}

// ALARM FUNCTIONS
// Audio file 1
void startPlayingAlarm1() {
  stopFile();
  printAndLog("Alarm1");
  ble.print("Alarm1");  
  sampleNumber = Entropy.random(1, NUM_SAMP+1);  
  playFile(makeFileNameString(ALARM_1_FILE_BASE, sampleNumber, USE_SAMP)); //make file name from alarm hour and sample number.
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 2
void startPlayingAlarm2() {
  stopFile();
  printAndLog("Alarm2");  
  ble.print("Alarm2");
  sampleNumber = Entropy.random(1, NUM_SAMP+1);
  playFile(makeFileNameString(ALARM_2_FILE_BASE, sampleNumber, USE_SAMP));
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 3
void startPlayingAlarm3() {
  stopFile();
  printAndLog("Alarm3");  
  sampleNumber = Entropy.random(1, NUM_SAMP+1);
  playFile(makeFileNameString(ALARM_3_FILE_BASE, sampleNumber, USE_SAMP));
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 4
void startPlayingAlarm4() {
  stopFile();
  printAndLog("Alarm4");  
  sampleNumber = Entropy.random(1, NUM_SAMP+1);
  playFile(makeFileNameString(ALARM_4_FILE_BASE, sampleNumber, USE_SAMP));
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 5
void startPlayingAlarm5() {
  stopFile();
  printAndLog("Alarm5");  
  sampleNumber = Entropy.random(1, NUM_SAMP+1); 
  playFile(makeFileNameString(ALARM_5_FILE_BASE, sampleNumber, USE_SAMP));
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 6
void startPlayingAlarm6() {
  stopFile();
  printAndLog("Alarm6");  
  sampleNumber = Entropy.random(1, NUM_SAMP+1);
  playFile(makeFileNameString(ALARM_6_FILE_BASE, sampleNumber, USE_SAMP));
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 7
void startPlayingAlarm7() {
  stopFile();
  printAndLog("Alarm7");  
  sampleNumber = Entropy.random(1, NUM_SAMP+1);
  playFile(makeFileNameString(ALARM_7_FILE_BASE, sampleNumber, USE_SAMP));
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 8
void startPlayingAlarm8() {
  stopFile();
  printAndLog("Alarm8");
  sampleNumber = Entropy.random(1, NUM_SAMP+1);
  playFile(makeFileNameString(ALARM_8_FILE_BASE, sampleNumber, USE_SAMP));
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 9
void startPlayingAlarm9() {
  stopFile();
  printAndLog("Alarm9");
  sampleNumber = Entropy.random(1, NUM_SAMP+1);
  playFile(makeFileNameString(ALARM_9_FILE_BASE, sampleNumber, USE_SAMP));
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 10
void startPlayingAlarm10() {
  stopFile();  
  printAndLog("Alarm10");
  sampleNumber = Entropy.random(1, NUM_SAMP+1);
  playFile(makeFileNameString(ALARM_10_FILE_BASE, sampleNumber, USE_SAMP));
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 11
void startPlayingAlarm11() {
  stopFile();
  printAndLog("Alarm11");
  sampleNumber = Entropy.random(1, NUM_SAMP+1); 
  playFile(makeFileNameString(ALARM_11_FILE_BASE, sampleNumber, USE_SAMP));
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 12
void startPlayingAlarm12() {
  stopFile();
  sampleNumber = Entropy.random(1, NUM_SAMP+1);  
  printAndLog("Alarm12");
  playFile(makeFileNameString(ALARM_12_FILE_BASE, sampleNumber, USE_SAMP));
  delay(WAIT_AFTER_PLAY_MS);
}

// Audio file 13
void startPlayingAlarm13() {
  stopFile();
  printAndLog("Alarm13");
  sampleNumber = Entropy.random(1, NUM_SAMP+1);
  playFile(makeFileNameString(ALARM_13_FILE_BASE, sampleNumber, USE_SAMP));
  delay(WAIT_AFTER_PLAY_MS);
}

// Audio file 14
void startPlayingAlarm14() {
  stopFile();
  printAndLog("Alarm14");
  sampleNumber = Entropy.random(1, NUM_SAMP+1);
  playFile(makeFileNameString(ALARM_14_FILE_BASE, sampleNumber, USE_SAMP));
  delay(WAIT_AFTER_PLAY_MS);
}

// Audio file 15
void startPlayingAlarm15() {
  stopFile();
  printAndLog("Alarm15");
  sampleNumber = Entropy.random(1, NUM_SAMP+1); 
  playFile(makeFileNameString(ALARM_15_FILE_BASE, sampleNumber, USE_SAMP));
  delay(WAIT_AFTER_PLAY_MS);
}

// Audio file 16
void startPlayingAlarm16() {
  stopFile();
  printAndLog("Alarm16");
  sampleNumber = Entropy.random(1, NUM_SAMP+1);
  playFile(makeFileNameString(ALARM_16_FILE_BASE, sampleNumber, USE_SAMP));
  delay(WAIT_AFTER_PLAY_MS);
}

// Audio file 17
void startPlayingAlarm17() {
  stopFile();
  printAndLog("Alarm17");
  sampleNumber = Entropy.random(1, NUM_SAMP+1);
  playFile(makeFileNameString(ALARM_17_FILE_BASE, sampleNumber, USE_SAMP));
  delay(WAIT_AFTER_PLAY_MS);
}

// Audio file 18
void startPlayingAlarm18() {
  stopFile();
  printAndLog("Alarm18");
  sampleNumber = Entropy.random(1, NUM_SAMP+1);
  playFile(makeFileNameString(ALARM_18_FILE_BASE, sampleNumber, USE_SAMP));
  delay(WAIT_AFTER_PLAY_MS);
}

// Audio file 19
void startPlayingAlarm19() {
  stopFile();
  printAndLog("Alarm19");
  sampleNumber = Entropy.random(1, NUM_SAMP+1);
  playFile(makeFileNameString(ALARM_19_FILE_BASE, sampleNumber, USE_SAMP));
  delay(WAIT_AFTER_PLAY_MS);
}

// Audio file 20
void startPlayingAlarm20() {
  stopFile();
  printAndLog("Alarm20");
  sampleNumber = Entropy.random(1, NUM_SAMP+1);
  playFile(makeFileNameString(ALARM_20_FILE_BASE, sampleNumber, USE_SAMP));
  delay(WAIT_AFTER_PLAY_MS);
}

// Audio file 21
void startPlayingAlarm21() {
  stopFile();
  printAndLog("Alarm21");
  sampleNumber = Entropy.random(1, NUM_SAMP+1);
  playFile(makeFileNameString(ALARM_21_FILE_BASE, sampleNumber, USE_SAMP));
  delay(WAIT_AFTER_PLAY_MS);
}

// Audio file 22
void startPlayingAlarm22() {
  stopFile();
  printAndLog("Alarm22");
  sampleNumber = Entropy.random(1, NUM_SAMP+1); 
  playFile(makeFileNameString(ALARM_22_FILE_BASE, sampleNumber, USE_SAMP));
  delay(WAIT_AFTER_PLAY_MS);
}

// Audio file 23
void startPlayingAlarm23() {
  stopFile();
  printAndLog("Alarm23");
  sampleNumber = Entropy.random(1, NUM_SAMP+1);  
  playFile(makeFileNameString(ALARM_23_FILE_BASE, sampleNumber, USE_SAMP));
  delay(WAIT_AFTER_PLAY_MS);
}

// Audio file 24
void startPlayingAlarm24() {
  stopFile();
  printAndLog("Alarm24");
  sampleNumber = Entropy.random(1, NUM_SAMP+1);
  playFile(makeFileNameString(ALARM_24_FILE_BASE, sampleNumber, USE_SAMP));
  delay(WAIT_AFTER_PLAY_MS);
}

//Convert hours minutes and seconds to seconds after midnight
int time2sec (int h, int m, int s) {
  int timeSec = s + m * 60 + h * 3600;
  return timeSec;
}

//Function to determine if the present time is between two values
bool time_between(std::string startTime, std::string stopTime) {

  // Break up times into H, M and S
  int inputH = hour();
  int inputM = minute();
  int inputS = second();

  int startH = timeConstruct(startTime)[0];
  int startM = timeConstruct(startTime)[1];
  int startS = timeConstruct(startTime)[2];

  int stopH = timeConstruct(stopTime)[0];
  int stopM = timeConstruct(stopTime)[1];
  int stopS = timeConstruct(stopTime)[2];

  // convert times to seconds after midnight
  int inputSeconds = time2sec(inputH, inputM, inputS);
  int startSeconds = time2sec(startH, startM, startS);
  int stopSeconds = time2sec(stopH, stopM, stopS);
  bool rtrn = 0;
  
  if (startSeconds < stopSeconds) {
    //Serial.println("Play interval does not include midnight");
    if (0 <= inputSeconds && inputSeconds < startSeconds) {
      //Serial.println("Case 1");
      rtrn = 0;
    }
    else if (startSeconds <= inputSeconds && inputSeconds < stopSeconds) {
      //Serial.println("Case 2");
      rtrn = 1;
    }
    else if (stopSeconds <= inputSeconds && inputSeconds < SEC_PRE_MIDNIGHT) {
      //Serial.println("Case 3");
      rtrn = 0;
    }
  }
  else if (startSeconds > stopSeconds) {
    //Serial.println("Play interval includes midnight");
    if (0 <= inputSeconds && inputSeconds < stopSeconds) {
      //Serial.println("Case 1");
      rtrn = 1;
    }
    else if (stopSeconds <= inputSeconds && inputSeconds < startSeconds) {
      //Serial.println("Case 2");
      rtrn = 0;
    }
    else if (startSeconds <= inputSeconds && inputSeconds <= MIDNIGHT_IN_SEC) {
      //Serial.println("Case 3");
      rtrn = 1;
    }
  }

  return rtrn;
}


// FAULT CHECK
void fault_check(){
  //check if system is being used in 24 hour mode. If not and system should be asleep, go to sleep.
  if (time_between(SLEEP_TIME, WAKE_TIME) && ALWAYS_ON == false){
    Serial.println("Fault Check: go to sleep");
    printAndLog("Go to sleep.");
    delay(1000);
    doneSignal();

  } else {
      // if no audio is playing, start the appropriate default track
    if (playWav1.isPlaying() == false){ 
      printAndLog("Fault Check: System was not playing.");
      
        // Check if sampleNumber is set to non-zero (e.g. system has stayed awake since alarm tripped)
        // If zero, then system went through a power cycle. Re-randomize.
        if (sampleNumber == 0){
          sampleNumber = Entropy.random(1, NUM_SAMP+1);
        }
      
        //Changed this to go for 24 hrs
        if (time_between(ALARM_1, ALARM_2)){
          playFile(makeFileNameString(ALARM_1_FILE_BASE, sampleNumber, USE_SAMP));
          delay(250);
        }
        else if (time_between(ALARM_2, ALARM_3)){
          playFile(makeFileNameString(ALARM_2_FILE_BASE, sampleNumber, USE_SAMP));
          delay(250);
        }
        else if (time_between(ALARM_3, ALARM_4)){
          playFile(makeFileNameString(ALARM_3_FILE_BASE, sampleNumber, USE_SAMP));
          delay(250);
        }
        else if (time_between(ALARM_4, ALARM_5)){
          playFile(makeFileNameString(ALARM_4_FILE_BASE, sampleNumber, USE_SAMP));
          delay(250);
        }
        else if (time_between(ALARM_5, ALARM_6)){
          playFile(makeFileNameString(ALARM_5_FILE_BASE, sampleNumber, USE_SAMP));
          delay(250);
        }
        else if (time_between(ALARM_6, ALARM_7)){
          playFile(makeFileNameString(ALARM_6_FILE_BASE, sampleNumber, USE_SAMP));
          delay(250);
        }
        else if (time_between(ALARM_7, ALARM_8)){
          playFile(makeFileNameString(ALARM_7_FILE_BASE, sampleNumber, USE_SAMP));
          delay(250);
        }
        else if (time_between(ALARM_8, ALARM_9)){
          playFile(makeFileNameString(ALARM_8_FILE_BASE, sampleNumber, USE_SAMP));
          delay(250);
        }
        else if (time_between(ALARM_9, ALARM_10)){
          playFile(makeFileNameString(ALARM_9_FILE_BASE, sampleNumber, USE_SAMP));
          delay(250);
        }
        else if (time_between(ALARM_10, ALARM_11)){
          playFile(makeFileNameString(ALARM_10_FILE_BASE, sampleNumber, USE_SAMP));
          delay(250);
        }
        else if (time_between(ALARM_11, ALARM_12)){
          playFile(makeFileNameString(ALARM_11_FILE_BASE, sampleNumber, USE_SAMP));
          delay(250);
        }
        else if (time_between(ALARM_12, ALARM_13)){
          playFile(makeFileNameString(ALARM_12_FILE_BASE, sampleNumber, USE_SAMP));
          delay(250);
        }
        else if (time_between(ALARM_13, ALARM_14)){
          playFile(makeFileNameString(ALARM_13_FILE_BASE, sampleNumber, USE_SAMP));
          delay(250);
        }
        else if (time_between(ALARM_14, ALARM_15)){
          playFile(makeFileNameString(ALARM_14_FILE_BASE, sampleNumber, USE_SAMP));
          delay(250);
        }
        else if (time_between(ALARM_15, ALARM_16)){
          playFile(makeFileNameString(ALARM_15_FILE_BASE, sampleNumber, USE_SAMP));
          delay(250);
        }
        else if (time_between(ALARM_16, ALARM_17)){
          playFile(makeFileNameString(ALARM_16_FILE_BASE, sampleNumber, USE_SAMP));
          delay(250);
        }
        else if (time_between(ALARM_17, ALARM_18)){
          playFile(makeFileNameString(ALARM_17_FILE_BASE, sampleNumber, USE_SAMP));
          delay(250);
        }
        else if (time_between(ALARM_18, ALARM_19)){
          playFile(makeFileNameString(ALARM_18_FILE_BASE, sampleNumber, USE_SAMP));
          delay(250);
        }
        else if (time_between(ALARM_19, ALARM_20)){
          playFile(makeFileNameString(ALARM_19_FILE_BASE, sampleNumber, USE_SAMP));
          delay(250);
        }
        else if (time_between(ALARM_20, ALARM_21)){
          playFile(makeFileNameString(ALARM_20_FILE_BASE, sampleNumber, USE_SAMP));
          delay(250);
        }
        else if (time_between(ALARM_21, ALARM_22)){
          playFile(makeFileNameString(ALARM_21_FILE_BASE, sampleNumber, USE_SAMP));
          delay(250);
        }
        else if (time_between(ALARM_22, ALARM_23)){
          playFile(makeFileNameString(ALARM_22_FILE_BASE, sampleNumber, USE_SAMP));
          delay(250);
        }     
        else if (time_between(ALARM_23, ALARM_24)){
          playFile(makeFileNameString(ALARM_23_FILE_BASE, sampleNumber, USE_SAMP));
          delay(250);
        }
        else if (time_between(ALARM_24, ALARM_1)){
          playFile(makeFileNameString(ALARM_24_FILE_BASE, sampleNumber, USE_SAMP));
          delay(250);
        }
      } else {
        //Serial.println("I am beyond time. Current time is not during the 24 hr day");
        }
  }
  //Serial.println("I promise, it's here that I am stuck");
}

void setup()  {
  // set the Time library to use Teensy 3.0's RTC to keep time
  setSyncProvider(getTeensy3Time);

  // Config settings  
  pinMode(SDCARD_CS_PIN, OUTPUT);
  didReadConfig = false; // might be able to get rid of all these instantiations
  
  // mandatory to begin the MTP session.
  //MTP.begin();

  // Setup the SD card
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);   
  Serial.println("Checking for SD Card...");
  if (!SD.begin(SDCARD_CS_PIN)) {
    Serial.println("SD.begin() failed. Check: ");
    Serial.println("  card insertion,");
    Serial.println("  SD shield I/O pins and chip select,");
    Serial.println("  card formatting.");
    Serial.println("Unable to access the SD card, go to sleep.");
    delay(1000);
    doneSignal();    
  //  return; dont think I need this if Im sending a done signal
  }
  
  //MTP.addFilesystem(SD, "SD Card");
  //Serial.print("success!");
  //Serial.println("");

  // Read and print configuration from the SD card file.
  Serial.println("################################");
  Serial.println("CONFIG VALUES FROM SD");
  didReadConfig = readConfiguration();
  Serial.println("END OF CONFIGS");  
  Serial.println("################################");
  Serial.println("");

  // Set up serial for debugging
  Serial.begin(BAUDE_RATE);

  //If configuraiton SERIAL_WAIT = TRUE, block the script from running until a character is sent
  if (SERIAL_WAIT == 1){
    while(Serial.available()==0) {
        Serial.println("Send any charcter to continue"); //only here for testing. REMOVE for deployment
        delay(1000);
    }
  }

  Serial.println("");
  Serial.print("Software Version: ");
  Serial.println(SOFTTWARE_VERSION);

  //Digital pin configurations
  pinMode(done_pin, OUTPUT);
  pinMode(mos_pwr, OUTPUT);
  pinMode(mos_audio, OUTPUT);

  //Bluetooth Setup
  Serial.println(F("Initialising Bluefruit LE module..."));

  if ( !ble.begin(VERBOSE_MODE) )
  {
    Serial.println("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?");
  }
  Serial.println( F("success!") );

  if ( FACTORYRESET_ENABLE )
  {
    /* Perform a factory reset to make sure everything is in a known state */
    Serial.println(F("Performing a factory reset: "));
    if ( ! ble.factoryReset() ){
      Serial.println("Couldn't factory reset");
    }
  }

  /* Disable command echo from Bluefruit */
  ble.echo(false);

  //check digital clock once in setup
  Serial.println("Current time: ");
  digitalClockDisplay();
  Serial.println();

  // Turn on System
  printAndLog("Wake up");
  digitalWrite(mos_pwr, HIGH);
  digitalWrite(mos_audio, HIGH);

  // WAV Player Setup
  AudioMemory(8);
  sgtl5000_1.enable();
  sgtl5000_1.volume(VOLUME);

  //Initialize the entropy funcition
  Entropy.Initialize();   

  //Set up alarms
  // using timeConstruct to insert hr, min and sec into Alarm definitions
  Serial.print("Set alarms...");
  Alarm.alarmRepeat(timeConstruct(ALARM_1)[0], timeConstruct(ALARM_1)[1], timeConstruct(ALARM_1)[2], startPlayingAlarm2); 
  Alarm.alarmRepeat(timeConstruct(ALARM_2)[0], timeConstruct(ALARM_2)[1], timeConstruct(ALARM_2)[2], startPlayingAlarm2); 
  Alarm.alarmRepeat(timeConstruct(ALARM_3)[0], timeConstruct(ALARM_3)[1], timeConstruct(ALARM_3)[2], startPlayingAlarm3); 
  Alarm.alarmRepeat(timeConstruct(ALARM_4)[0], timeConstruct(ALARM_4)[1], timeConstruct(ALARM_4)[2], startPlayingAlarm4);
  Alarm.alarmRepeat(timeConstruct(ALARM_5)[0], timeConstruct(ALARM_5)[1], timeConstruct(ALARM_5)[2], startPlayingAlarm5);
  Alarm.alarmRepeat(timeConstruct(ALARM_6)[0], timeConstruct(ALARM_6)[1], timeConstruct(ALARM_6)[2], startPlayingAlarm6);
  Alarm.alarmRepeat(timeConstruct(ALARM_7)[0], timeConstruct(ALARM_7)[1], timeConstruct(ALARM_7)[2], startPlayingAlarm7);
  Alarm.alarmRepeat(timeConstruct(ALARM_8)[0], timeConstruct(ALARM_8)[1], timeConstruct(ALARM_8)[2], startPlayingAlarm8);
  Alarm.alarmRepeat(timeConstruct(ALARM_9)[0], timeConstruct(ALARM_9)[1], timeConstruct(ALARM_9)[2], startPlayingAlarm9);
  Alarm.alarmRepeat(timeConstruct(ALARM_10)[0], timeConstruct(ALARM_10)[1], timeConstruct(ALARM_10)[2], startPlayingAlarm10);
  Alarm.alarmRepeat(timeConstruct(ALARM_11)[0], timeConstruct(ALARM_11)[1], timeConstruct(ALARM_11)[2], startPlayingAlarm11);
  Alarm.alarmRepeat(timeConstruct(ALARM_12)[0], timeConstruct(ALARM_12)[1], timeConstruct(ALARM_12)[2], startPlayingAlarm12);
  Alarm.alarmRepeat(timeConstruct(ALARM_13)[0], timeConstruct(ALARM_13)[1], timeConstruct(ALARM_13)[2], startPlayingAlarm13);
  Alarm.alarmRepeat(timeConstruct(ALARM_14)[0], timeConstruct(ALARM_14)[1], timeConstruct(ALARM_14)[2], startPlayingAlarm14);
  Alarm.alarmRepeat(timeConstruct(ALARM_15)[0], timeConstruct(ALARM_15)[1], timeConstruct(ALARM_15)[2], startPlayingAlarm15);
  Alarm.alarmRepeat(timeConstruct(ALARM_16)[0], timeConstruct(ALARM_16)[1], timeConstruct(ALARM_16)[2], startPlayingAlarm16);
  Alarm.alarmRepeat(timeConstruct(ALARM_17)[0], timeConstruct(ALARM_17)[1], timeConstruct(ALARM_17)[2], startPlayingAlarm17);
  Alarm.alarmRepeat(timeConstruct(ALARM_18)[0], timeConstruct(ALARM_18)[1], timeConstruct(ALARM_18)[2], startPlayingAlarm18);
  Alarm.alarmRepeat(timeConstruct(ALARM_19)[0], timeConstruct(ALARM_19)[1], timeConstruct(ALARM_19)[2], startPlayingAlarm19);
  Alarm.alarmRepeat(timeConstruct(ALARM_20)[0], timeConstruct(ALARM_20)[1], timeConstruct(ALARM_20)[2], startPlayingAlarm20);
  Alarm.alarmRepeat(timeConstruct(ALARM_21)[0], timeConstruct(ALARM_21)[1], timeConstruct(ALARM_21)[2], startPlayingAlarm21);
  Alarm.alarmRepeat(timeConstruct(ALARM_22)[0], timeConstruct(ALARM_22)[1], timeConstruct(ALARM_22)[2], startPlayingAlarm22);
  Alarm.alarmRepeat(timeConstruct(ALARM_23)[0], timeConstruct(ALARM_23)[1], timeConstruct(ALARM_23)[2], startPlayingAlarm23);
  Alarm.alarmRepeat(timeConstruct(ALARM_24)[0], timeConstruct(ALARM_24)[1], timeConstruct(ALARM_24)[2], startPlayingAlarm24);

  Serial.println("...success!");
}

void loop() {
  digitalClockDisplay(); //serial print the time according to RTC
  Serial.println();
  fault_check(); //If wavfile isn't playing, force on based on time
  //ble.print("AT+BLEUARTTX=");
  //ble.println(active_file);
  //MTP.loop();  //This is mandatory to be placed in the loop code.
  //Alarm.delay(10); //this has to be super fast for MP to work
  Alarm.delay(1000);
}