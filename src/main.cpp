//This code plays the loaded wav files on a schedule AND
//will tell the TPL510 that it's done when Sound Off is trigered
//#define SOFTTWARE_VERSION "v2.3" //This includes BLUEFRUIT print statements and renaming, volume update, fix to config file reference
#define SOFTWARE_VERSION "v3.1" //This includes RTC setting via bluetooth and software version STATUS check, Update made 04/09/2026
//This includes bluetooth interaction via the Bluefruit Connect app. You can send "STATUS" for a quick 
//report on the RTC date and time as well as volume and wake/sleep times. You can send "ON" to enable (temporarily) ALWAYS_ON so that 
//the system turns on as it cycles. Once on you can send more complex commands like "HELLO" or "SET______" insert variable of choice, eg.
//"SET VOLUME 13" or "SET WAKE_TIME 09:00:00". The system will reboot when a SET command is sent. 
//This system also has a failsafe for the time loss issue. If RTC is lost, the time will be set to five minutes past the last logged time
//and an error will log to the ERROR.txt file showing that the time was lost
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
//Wav file playback
AudioPlaySdWav        playWav1;
AudioOutputI2S        audioOutput;
AudioMixer4           mix1;   // fine volume control
AudioControlSGTL5000  sgtl5000_1;
// Audio routing
AudioConnection patchCord1(playWav1, 0, mix1, 0);     // Left WAV → Mixer input 0
AudioConnection patchCord2(mix1, 0, audioOutput, 0);  // Mixer out → Left lineout
AudioConnection patchCord3(mix1, 0, audioOutput, 1);  // Mixer out → Right lineout

//Bluefruit setup
#define FACTORYRESET_ENABLE         0
#define MINIMUM_FIRMWARE_VERSION    "0.8.1" //Other RAPS are on 0.6.6
#define MODE_LED_BEHAVIOUR          "MODE"
#define BLUEFRUIT_UART_MODE_PIN     -1 //the following sets the optional Mode pin, its recommended but not required
#define VERBOSE_MODE                false
Adafruit_BluefruitLE_UART ble(Serial3, BLUEFRUIT_UART_MODE_PIN);
volatile bool configUpdatePending = false;
String pendingVarName = "";
String pendingValue = "";
volatile bool sdOperationPending = false;
volatile int pendingSDOperation = 0; // 0=none, 1=list files, 2=write config
String lastPlayingFile = "";
bool wasPlaying = false;
bool lastConnected = false;  // GLOBAL variable, keeps state between loops


//STATIC DEFINITIONS
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14
#define WAIT_AFTER_PLAY_MS 250
#define SEC_PRE_MIDNIGHT 86399
#define SEC_PER_DAY 86400 //Just for reference
int done_pin = 17;
int mos_pwr = 3;
int mos_audio = 2;

//Settings from Config file
// Alarm times
char *ALARM_1=0;
char *ALARM_2=0;
char *ALARM_3=0;
char *ALARM_4=0;
char *ALARM_5=0;
char *ALARM_6=0;
char *ALARM_7=0;
char *ALARM_8=0;
char *ALARM_9=0;
char *ALARM_10=0;
char *ALARM_11=0;
char *ALARM_12=0;
char *ALARM_13=0;
char *ALARM_14=0;
char *ALARM_15=0;
char *ALARM_16=0;
char *ALARM_17=0;
char *ALARM_18=0;
char *ALARM_19=0;
char *ALARM_20=0;
char *ALARM_21=0;
char *ALARM_22=0;
char *ALARM_23=0;
char *ALARM_24=0;

//SOUND FILE BASE NAMES
char *ALARM_1_FILE_BASE=0; 
char *ALARM_2_FILE_BASE=0; 
char *ALARM_3_FILE_BASE=0; 
char *ALARM_4_FILE_BASE=0; 
char *ALARM_5_FILE_BASE=0; 
char *ALARM_6_FILE_BASE=0; 
char *ALARM_7_FILE_BASE=0; 
char *ALARM_8_FILE_BASE=0; 
char *ALARM_9_FILE_BASE=0; 
char *ALARM_10_FILE_BASE=0; 
char *ALARM_11_FILE_BASE=0; 
char *ALARM_12_FILE_BASE=0; 
char *ALARM_13_FILE_BASE=0;
char *ALARM_14_FILE_BASE=0;
char *ALARM_15_FILE_BASE=0;
char *ALARM_16_FILE_BASE=0;
char *ALARM_17_FILE_BASE=0;
char *ALARM_18_FILE_BASE=0;
char *ALARM_19_FILE_BASE=0; 
char *ALARM_20_FILE_BASE=0; 
char *ALARM_21_FILE_BASE=0; 
char *ALARM_22_FILE_BASE=0; 
char *ALARM_23_FILE_BASE=0; 
char *ALARM_24_FILE_BASE=0; 

// // Arrays to make config writing easier - add after the individual alarm variables
// char* ALARM_TIMES[24] = {ALARM_1, ALARM_2, ALARM_3, ALARM_4, ALARM_5, ALARM_6, 
//                          ALARM_7, ALARM_8, ALARM_9, ALARM_10, ALARM_11, ALARM_12,
//                          ALARM_13, ALARM_14, ALARM_15, ALARM_16, ALARM_17, ALARM_18,
//                          ALARM_19, ALARM_20, ALARM_21, ALARM_22, ALARM_23, ALARM_24};

// char* FILE_BASES[24] = {ALARM_1_FILE_BASE, ALARM_2_FILE_BASE, ALARM_3_FILE_BASE, 
//                         ALARM_4_FILE_BASE, ALARM_5_FILE_BASE, ALARM_6_FILE_BASE,
//                         ALARM_7_FILE_BASE, ALARM_8_FILE_BASE, ALARM_9_FILE_BASE,
//                         ALARM_10_FILE_BASE, ALARM_11_FILE_BASE, ALARM_12_FILE_BASE,
//                         ALARM_13_FILE_BASE, ALARM_14_FILE_BASE, ALARM_15_FILE_BASE,
//                         ALARM_16_FILE_BASE, ALARM_17_FILE_BASE, ALARM_18_FILE_BASE,
//                         ALARM_19_FILE_BASE, ALARM_20_FILE_BASE, ALARM_21_FILE_BASE,
//                         ALARM_22_FILE_BASE, ALARM_23_FILE_BASE, ALARM_24_FILE_BASE};
// AlarmId alarmIDs[24];   // stores the 24 alarm IDs

// Wake Time
char * WAKE_TIME=0;

// Sleep Time
char * SLEEP_TIME=0; //std::string sleep_time = "14:58:00"=0;

// RAPS ID for Bluefruit naming
char * RAPS_ID=0;

//Other
int BAUD_RATE=0;
int NUM_SAMP=0; 
bool USE_SAMP=0;
bool ALWAYS_ON=0;
int VOLUME=0; //Values can be 0 (silent) to 100 (loudest), in air about 11dB swing https://www.pjrc.com/teensy/gui/?info=AudioControlSGTL5000#
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
bool writeConfigFileSafe();

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

// Log errors to a separate SD file
void logError(std::string string) {
  File errorFile = SD.open("ERRORS.txt", FILE_WRITE);
  if (errorFile) {
    errorFile.print(present());   // timestamp
    errorFile.print("  ");
    errorFile.println(string.c_str());
    errorFile.close();
  } else {
    Serial.println("ERROR: could not open ERRORS.txt");
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
// void playFile(std::string filename) { //const char string[]
//   printAndLog("Playing file:");
//   printAndLog(filename);
//   playWav1.play(filename.c_str());
//   active_file = filename.c_str();
//   delay(10);
// }
void playFile(std::string filename) {
    // printAndLog("Playing file:");
    // printAndLog(filename);
    playWav1.play(filename.c_str());
    active_file = String(filename.c_str());  // Convert to Arduino String, not char*
    delay(10);
}

// Turn off sound
void stopFile() {
  playWav1.stop();
  delay(250);
  printAndLog("Audio Stopped"); //ths goes at end of stopFile block. putting here for testing.
}

void doneSignal() {
  // Mute outputs immediately
  sgtl5000_1.muteLineout();
  sgtl5000_1.volume(0.0);
  mix1.gain(0, 0.0); // Mute mixer channel(s)
  Serial.print("Trying to silence.");
  delay(100);
  if (playWav1.isPlaying()) {
    playWav1.stop();
    delay(100);
  }
  sgtl5000_1.disable();
  delay(50);
  digitalWrite(mos_audio, LOW);
  // REMOVE FLAG - Clean shutdown
  if (SD.exists("RUNNING.flg")) {
    SD.remove("RUNNING.flg");
  }
  printAndLog("Clean shutdown - going to sleep");
  digitalWrite(mos_pwr, LOW);
  delay(100);
  digitalWrite(done_pin, HIGH);
  delay(100);
  digitalWrite(done_pin, LOW);
}

//Function to extract integers from strings of form "hh:mm:ss"
std::array<int,3> timeConstruct(std::string timeString){
  std::array<int,3> timeInts;
  std::string hrString = timeString.substr(0,2);
  std::string minString = timeString.substr(3,2);
  std::string secString = timeString.substr(6,2);
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

    // BAUD Rate
    else if (cfg.nameIs("BAUD_RATE")) { 
      BAUD_RATE = cfg.getIntValue();
      Serial.print("BAUD_RATE: ");
      Serial.println(BAUD_RATE);
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
      VOLUME = cfg.getIntValue();  // read as integer directly
      // Clamp to valid range just in case:
      if (VOLUME < 0) VOLUME = 0;
      else if (VOLUME > 100) VOLUME = 100;
      Serial.print("VOLUME: ");
      Serial.println(VOLUME);
    }

    //Might need to add this as a function for powered but no playback muteLineout() to Silence the line level outputs.

    else if (cfg.nameIs("RAPS_ID")) { 
      RAPS_ID = cfg.copyValue();
      Serial.print("RAPS_ID: ");
      Serial.println(RAPS_ID);
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
  // Break up times into H, M, and S
  int inputH = hour();
  int inputM = minute();
  int inputS = second();
  int startH = timeConstruct(startTime)[0];
  int startM = timeConstruct(startTime)[1];
  int startS = timeConstruct(startTime)[2];
  int stopH = timeConstruct(stopTime)[0];
  int stopM = timeConstruct(stopTime)[1];
  int stopS = timeConstruct(stopTime)[2];
  // Convert times to seconds after midnight
  int inputSeconds = time2sec(inputH, inputM, inputS);
  int startSeconds = time2sec(startH, startM, startS);
  int stopSeconds = time2sec(stopH, stopM, stopS);
  bool rtrn = false;
  if (startSeconds < stopSeconds) {
    // Interval does NOT include midnight
    if (inputSeconds < startSeconds) {
      rtrn = false;
    }
    else if (inputSeconds >= startSeconds && inputSeconds < stopSeconds) {
      rtrn = true;
    }
    else { // inputSeconds >= stopSeconds
      rtrn = false;
    }
  }
  else if (startSeconds > stopSeconds) {
    // Interval includes midnight
    if (inputSeconds < stopSeconds) {
      rtrn = true;
    }
    else if (inputSeconds >= stopSeconds && inputSeconds < startSeconds) {
      rtrn = false;
    }
    else if (inputSeconds >= startSeconds && inputSeconds <= SEC_PRE_MIDNIGHT) {
      rtrn = true;
    }
  }
  else {
    // startSeconds == stopSeconds means interval is zero length - treat as false
    rtrn = false;
  }
  return rtrn;
}

// FAULT CHECK
void fault_check(){
    // Add debug info
  Serial.print("Sleep/Wake check: ");
  Serial.println(time_between(SLEEP_TIME, WAKE_TIME) ? "SLEEP" : "AWAKE");
  //check if system is being used in 24 hour mode. If not and system should be asleep, go to sleep.
  if (time_between(SLEEP_TIME, WAKE_TIME) && ALWAYS_ON == false){
    Serial.println("Fault Check: go to sleep");
    //printAndLog("Go to sleep.");
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

// DEBUG: Time recovery function with extensive logging
bool recoverTimeFromLog() {
  Serial.println("DEBUG: >>> Entering recoverTimeFromLog()");
  
  File logFile = SD.open("LOG.txt", FILE_READ);
  if (!logFile) {
    Serial.println("DEBUG: Could not open LOG.txt for time recovery");
    return false;
  }
  Serial.println("DEBUG: LOG.txt opened successfully");
  
  String lastLine = "";
  String currentLine = "";
  int lineCount = 0;
  
  Serial.println("DEBUG: Starting to read log file...");
  while (logFile.available()) {
    char c = logFile.read();
    if (c == '\n') {
      if (currentLine.length() > 0) {
        lastLine = currentLine;
        currentLine = "";
        lineCount++;
      }
    } else {
      currentLine += c;
    }
  }
  Serial.print("DEBUG: Read ");
  Serial.print(lineCount);
  Serial.println(" lines from log");
  
  if (currentLine.length() > 0) {
    lastLine = currentLine;
  }
  
  logFile.close();
  Serial.println("DEBUG: Log file closed");
  
  if (lastLine.length() == 0) {
    Serial.println("DEBUG: No timestamp found in LOG.txt (empty file)");
    return false;
  }
  
  Serial.print("DEBUG: Last log entry: ");
  Serial.println(lastLine);
  
  int firstSlash = lastLine.indexOf('/');
  int secondSlash = lastLine.indexOf('/', firstSlash + 1);
  int firstSpace = lastLine.indexOf(' ', secondSlash);
  int firstColon = lastLine.indexOf(':', firstSpace);
  int secondColon = lastLine.indexOf(':', firstColon + 1);
  int secondSpace = lastLine.indexOf(' ', secondColon);
  
  Serial.print("DEBUG: Parse indices - firstSlash:");
  Serial.print(firstSlash);
  Serial.print(" secondSlash:");
  Serial.print(secondSlash);
  Serial.print(" firstSpace:");
  Serial.print(firstSpace);
  Serial.print(" firstColon:");
  Serial.print(firstColon);
  Serial.print(" secondColon:");
  Serial.print(secondColon);
  Serial.print(" secondSpace:");
  Serial.println(secondSpace);
  
  if (firstSlash == -1 || secondSlash == -1 || firstSpace == -1 || 
      firstColon == -1 || secondColon == -1) {
    Serial.println("DEBUG: Could not parse timestamp format");
    return false;
  }
  
  int logMonth = lastLine.substring(0, firstSlash).toInt();
  int logDay = lastLine.substring(firstSlash + 1, secondSlash).toInt();
  int logYear = lastLine.substring(secondSlash + 1, firstSpace).toInt();
  int logHour = lastLine.substring(firstSpace + 1, firstColon).toInt();
  int logMinute = lastLine.substring(firstColon + 1, secondColon).toInt();
  int logSecond = lastLine.substring(secondColon + 1, secondSpace).toInt();
  
  Serial.print("DEBUG: Parsed - Month:");
  Serial.print(logMonth);
  Serial.print(" Day:");
  Serial.print(logDay);
  Serial.print(" Year:");
  Serial.print(logYear);
  Serial.print(" Hour:");
  Serial.print(logHour);
  Serial.print(" Min:");
  Serial.print(logMinute);
  Serial.print(" Sec:");
  Serial.println(logSecond);
  
  tmElements_t tm;
  tm.Year = logYear - 1970;
  tm.Month = logMonth;
  tm.Day = logDay;
  tm.Hour = logHour;
  tm.Minute = logMinute;
  tm.Second = logSecond;
  
  Serial.println("DEBUG: Creating time_t from parsed values...");
  time_t logTime = makeTime(tm);
  time_t recoveredTime = logTime + 300;//Add 5 minutes to account for low power timer cycling
  
  Serial.println("DEBUG: Setting RTC to recovered time...");
  Teensy3Clock.set(recoveredTime);
  setTime(recoveredTime);
  
  Serial.print("DEBUG: Time recovered and set to: ");
  digitalClockDisplay();
  Serial.println(" (5 minutes after last log entry)");
  
  Serial.println("DEBUG: Logging time recovery event...");
  myFile = SD.open("LOG.txt", FILE_WRITE);
  if (myFile) {
    myFile.print(present());
    myFile.print("  ");
    myFile.print("TIME RECOVERED: RTC was invalid, set to log timestamp + 5 min");
    myFile.println();
    myFile.close();
    Serial.println("DEBUG: Time recovery logged to LOG.txt");
  } else {
    Serial.println("DEBUG: Warning - Could not log time recovery event");
  }
  
  Serial.println("DEBUG: <<< Exiting recoverTimeFromLog() successfully");
  return true;
}

bool isTimeValid() {
  return (year() >= 2025);
}

// Safe SD operation wrapper - call this before any SD file operations
bool beginSDOperation() {
    Serial.println("DEBUG: Beginning SD operation...");
    
    // Check if audio is playing
    wasPlaying = playWav1.isPlaying();
    if (wasPlaying) {
        lastPlayingFile = active_file;
        Serial.print("DEBUG: Pausing audio: ");
        Serial.println(lastPlayingFile);
        playWav1.stop();
        delay(100); // Give time for audio to fully stop
    }
    
    return true;
}

// Resume audio after SD operation
void endSDOperation() {
    Serial.println("DEBUG: Ending SD operation...");
    
    // Resume audio if it was playing
    if (wasPlaying && lastPlayingFile.length() > 0) {
        Serial.print("DEBUG: Resuming audio: ");
        Serial.println(lastPlayingFile);
        playWav1.play(lastPlayingFile.c_str());
        delay(10);
    }
    
    wasPlaying = false;
    Serial.println("DEBUG: SD operation complete");
}

// List files on SD card - safe version
void listSDFiles() {
    Serial.println("DEBUG: Listing SD card files...");
    ble.print("AT+BLEUARTTX=");
    ble.println("=== SD CARD FILES ===");
    
    File root = SD.open("/");
    if (!root) {
        Serial.println("ERROR: Failed to open root directory");
        ble.print("AT+BLEUARTTX=");
        ble.println("ERROR: Can't open SD");
        return;
    }
    
    int fileCount = 0;
    while (true) {
        File entry = root.openNextFile();
        if (!entry) {
            break; // No more files
        }
        
        String fileName = entry.name();
        long fileSize = entry.size();
        
        Serial.print("  ");
        Serial.print(fileName);
        Serial.print(" (");
        Serial.print(fileSize);
        Serial.println(" bytes)");
        
        // Send to BLE in chunks to avoid buffer overflow
        ble.print("AT+BLEUARTTX=");
        ble.print(fileName);
        ble.print(" ");
        ble.print(fileSize);
        ble.println("b");
        
        entry.close();
        fileCount++;
        
        // Small delay between BLE sends
        delay(50);
    }
    
    root.close();
    
    Serial.print("DEBUG: Found ");
    Serial.print(fileCount);
    Serial.println(" files");
    
    ble.print("AT+BLEUARTTX=");
    ble.print("Total: ");
    ble.print(fileCount);
    ble.println(" files");
}

// Modified writeConfigFile with SD operation safety
bool writeConfigFileSafe() {
    Serial.println("DEBUG: Writing config file (SAFE)...");
    
    // Remove old temp file
    if (SD.exists("config.tmp")) {
        SD.remove("config.tmp");
    }
    
    File configFile = SD.open("config.tmp", FILE_WRITE);
    if (!configFile) {
        Serial.println("ERROR: Failed to create temp config file");
        return false;
    }

    configFile.println("# File format: <field>=<value> NO SPACES");
    
    // Write alarm times FIRST
    configFile.println("# ALARM TIMES");
    if (ALARM_1) { configFile.print("ALARM_1="); configFile.println(ALARM_1); }
    if (ALARM_2) { configFile.print("ALARM_2="); configFile.println(ALARM_2); }
    if (ALARM_3) { configFile.print("ALARM_3="); configFile.println(ALARM_3); }
    if (ALARM_4) { configFile.print("ALARM_4="); configFile.println(ALARM_4); }
    if (ALARM_5) { configFile.print("ALARM_5="); configFile.println(ALARM_5); }
    if (ALARM_6) { configFile.print("ALARM_6="); configFile.println(ALARM_6); }
    if (ALARM_7) { configFile.print("ALARM_7="); configFile.println(ALARM_7); }
    if (ALARM_8) { configFile.print("ALARM_8="); configFile.println(ALARM_8); }
    if (ALARM_9) { configFile.print("ALARM_9="); configFile.println(ALARM_9); }
    if (ALARM_10) { configFile.print("ALARM_10="); configFile.println(ALARM_10); }
    if (ALARM_11) { configFile.print("ALARM_11="); configFile.println(ALARM_11); }
    if (ALARM_12) { configFile.print("ALARM_12="); configFile.println(ALARM_12); }
    if (ALARM_13) { configFile.print("ALARM_13="); configFile.println(ALARM_13); }
    if (ALARM_14) { configFile.print("ALARM_14="); configFile.println(ALARM_14); }
    if (ALARM_15) { configFile.print("ALARM_15="); configFile.println(ALARM_15); }
    if (ALARM_16) { configFile.print("ALARM_16="); configFile.println(ALARM_16); }
    if (ALARM_17) { configFile.print("ALARM_17="); configFile.println(ALARM_17); }
    if (ALARM_18) { configFile.print("ALARM_18="); configFile.println(ALARM_18); }
    if (ALARM_19) { configFile.print("ALARM_19="); configFile.println(ALARM_19); }
    if (ALARM_20) { configFile.print("ALARM_20="); configFile.println(ALARM_20); }
    if (ALARM_21) { configFile.print("ALARM_21="); configFile.println(ALARM_21); }
    if (ALARM_22) { configFile.print("ALARM_22="); configFile.println(ALARM_22); }
    if (ALARM_23) { configFile.print("ALARM_23="); configFile.println(ALARM_23); }
    if (ALARM_24) { configFile.print("ALARM_24="); configFile.println(ALARM_24); }

    // Write file base names
    configFile.println("# FILE BASES");
    if (ALARM_1_FILE_BASE) { configFile.print("ALARM_1_FILE_BASE="); configFile.println(ALARM_1_FILE_BASE); }
    if (ALARM_2_FILE_BASE) { configFile.print("ALARM_2_FILE_BASE="); configFile.println(ALARM_2_FILE_BASE); }
    if (ALARM_3_FILE_BASE) { configFile.print("ALARM_3_FILE_BASE="); configFile.println(ALARM_3_FILE_BASE); }
    if (ALARM_4_FILE_BASE) { configFile.print("ALARM_4_FILE_BASE="); configFile.println(ALARM_4_FILE_BASE); }
    if (ALARM_5_FILE_BASE) { configFile.print("ALARM_5_FILE_BASE="); configFile.println(ALARM_5_FILE_BASE); }
    if (ALARM_6_FILE_BASE) { configFile.print("ALARM_6_FILE_BASE="); configFile.println(ALARM_6_FILE_BASE); }
    if (ALARM_7_FILE_BASE) { configFile.print("ALARM_7_FILE_BASE="); configFile.println(ALARM_7_FILE_BASE); }
    if (ALARM_8_FILE_BASE) { configFile.print("ALARM_8_FILE_BASE="); configFile.println(ALARM_8_FILE_BASE); }
    if (ALARM_9_FILE_BASE) { configFile.print("ALARM_9_FILE_BASE="); configFile.println(ALARM_9_FILE_BASE); }
    if (ALARM_10_FILE_BASE) { configFile.print("ALARM_10_FILE_BASE="); configFile.println(ALARM_10_FILE_BASE); }
    if (ALARM_11_FILE_BASE) { configFile.print("ALARM_11_FILE_BASE="); configFile.println(ALARM_11_FILE_BASE); }
    if (ALARM_12_FILE_BASE) { configFile.print("ALARM_12_FILE_BASE="); configFile.println(ALARM_12_FILE_BASE); }
    if (ALARM_13_FILE_BASE) { configFile.print("ALARM_13_FILE_BASE="); configFile.println(ALARM_13_FILE_BASE); }
    if (ALARM_14_FILE_BASE) { configFile.print("ALARM_14_FILE_BASE="); configFile.println(ALARM_14_FILE_BASE); }
    if (ALARM_15_FILE_BASE) { configFile.print("ALARM_15_FILE_BASE="); configFile.println(ALARM_15_FILE_BASE); }
    if (ALARM_16_FILE_BASE) { configFile.print("ALARM_16_FILE_BASE="); configFile.println(ALARM_16_FILE_BASE); }
    if (ALARM_17_FILE_BASE) { configFile.print("ALARM_17_FILE_BASE="); configFile.println(ALARM_17_FILE_BASE); }
    if (ALARM_18_FILE_BASE) { configFile.print("ALARM_18_FILE_BASE="); configFile.println(ALARM_18_FILE_BASE); }
    if (ALARM_19_FILE_BASE) { configFile.print("ALARM_19_FILE_BASE="); configFile.println(ALARM_19_FILE_BASE); }
    if (ALARM_20_FILE_BASE) { configFile.print("ALARM_20_FILE_BASE="); configFile.println(ALARM_20_FILE_BASE); }
    if (ALARM_21_FILE_BASE) { configFile.print("ALARM_21_FILE_BASE="); configFile.println(ALARM_21_FILE_BASE); }
    if (ALARM_22_FILE_BASE) { configFile.print("ALARM_22_FILE_BASE="); configFile.println(ALARM_22_FILE_BASE); }
    if (ALARM_23_FILE_BASE) { configFile.print("ALARM_23_FILE_BASE="); configFile.println(ALARM_23_FILE_BASE); }
    if (ALARM_24_FILE_BASE) { configFile.print("ALARM_24_FILE_BASE="); configFile.println(ALARM_24_FILE_BASE); }

    // Write wake/sleep times
    configFile.println("# Wake Time");
    if (WAKE_TIME) {
        configFile.print("WAKE_TIME=");
        configFile.println(WAKE_TIME);
    }
    
    configFile.println("# Sleep Time");
    if (SLEEP_TIME) {
        configFile.print("SLEEP_TIME=");
        configFile.println(SLEEP_TIME);
    }

    // Write other settings
    configFile.println("# OTHER");
    configFile.print("BAUD_RATE=");
    configFile.println(BAUD_RATE);
    
    configFile.print("NUM_SAMP=");
    configFile.println(NUM_SAMP);
    
    configFile.print("USE_SAMP=");
    configFile.println(USE_SAMP ? "true" : "false");
    
    configFile.print("ALWAYS_ON=");
    configFile.println(ALWAYS_ON ? "true" : "false");
    
    configFile.print("SERIAL_WAIT=");
    configFile.println(SERIAL_WAIT ? "true" : "false");
    
    configFile.print("VOLUME=");
    configFile.println(VOLUME);
    
    if (RAPS_ID) {
        configFile.print("RAPS_ID=");
        configFile.println(RAPS_ID);
    }

    configFile.close();
    Serial.println("DEBUG: Config written to config.tmp");

    // Replace old config file
    if (SD.exists("config.cfg")) {
        SD.remove("config.cfg");
    }
    if (!SD.rename("config.tmp", "config.cfg")) {
        Serial.println("ERROR: Failed to rename config.tmp to config.cfg");
        return false;
    }

    Serial.println("DEBUG: Config update complete!");
    return true;
}

// Call this in setup AFTER ble.begin()
void waitForBLECommands(unsigned long timeoutMs = 10000) {
  if (!ble.isConnected()) return;  // Skip if no central connected
  Serial.println("DEBUG: Central connected, waiting for commands...");
  unsigned long start = millis();
  while (millis() - start < timeoutMs) {
      ble.println("AT+BLEUARTRX");
      ble.readline();
      if (strcmp(ble.buffer, "OK") != 0 && strcmp(ble.buffer, "ERROR") != 0) {
        String command = String(ble.buffer);
        command.trim();
        if (command.equalsIgnoreCase("STATUS")) {
          ble.print("AT+BLEUARTTX=Time: "); 
          ble.print(hour()); ble.print(":");
          if (minute() < 10) ble.print('0'); ble.print(minute()); ble.print(":");
          if (second() < 10) ble.print('0'); 
          ble.println(second());
          delay(20);

          ble.print("AT+BLEUARTTX=Date: "); 
          ble.print(month()); ble.print("/"); 
          ble.print(day()); ble.print("/"); ble.println(year());
          delay(20);

          ble.print("AT+BLEUARTTX=Volume: "); 
          ble.println(VOLUME);
          delay(20);

          ble.print("AT+BLEUARTTX=WAKE_TIME: "); 
          ble.println(WAKE_TIME ? WAKE_TIME : "NULL");
          delay(20);

          ble.print("AT+BLEUARTTX=SLEEP_TIME: "); 
          ble.println(SLEEP_TIME ? SLEEP_TIME : "NULL");
          delay(20);

          ble.print("AT+BLEUARTTX=Version: ");
          ble.println(SOFTWARE_VERSION);
          delay(20);

          Serial.println("DEBUG: Sent STATUS info");
        }

        else if (command.equalsIgnoreCase("ON")) {  
          ALWAYS_ON = true;
          Serial.println("DEBUG: RAPS set to ALWAYS_ON");
          return;  // exit immediately once ON is received
        }
        else {
          ble.print("AT+BLEUARTTX=INVALID FORMAT");
          delay(20);
        }
      }
        delay(20);
      }
    Serial.println("DEBUG: BLE command wait complete, continuing setup...");
}

bool parseDateTimeString(String dateTimeStr, tmElements_t& tm) {
    dateTimeStr.trim();   // remove leading/trailing whitespace

    // Allow either "YYYY-MM-DD HH:MM:SS" or "YYYY-MM-DDTHH:MM:SS"
    dateTimeStr.replace("T", " ");

    // Collapse accidental double spaces between date and time
    while (dateTimeStr.indexOf("  ") != -1) {
        dateTimeStr.replace("  ", " ");
    }

    Serial.print("DEBUG RTC raw string: '");
    Serial.print(dateTimeStr);
    Serial.print("' len=");
    Serial.println(dateTimeStr.length());

    // Must still be at least the expected length
    if (dateTimeStr.length() < 19) {
        return false;
    }

    // Only use the first 19 chars in case BLE app adds extra junk
    dateTimeStr = dateTimeStr.substring(0, 19);

    if (dateTimeStr.charAt(4) != '-' || dateTimeStr.charAt(7) != '-' ||
        dateTimeStr.charAt(10) != ' ' || dateTimeStr.charAt(13) != ':' ||
        dateTimeStr.charAt(16) != ':') {
        return false;
    }

    int yr  = dateTimeStr.substring(0, 4).toInt();
    int mon = dateTimeStr.substring(5, 7).toInt();
    int day = dateTimeStr.substring(8, 10).toInt();
    int hr  = dateTimeStr.substring(11, 13).toInt();
    int min = dateTimeStr.substring(14, 16).toInt();
    int sec = dateTimeStr.substring(17, 19).toInt();

    if (yr < 2020 || yr > 2099) return false;
    if (mon < 1 || mon > 12) return false;
    if (day < 1 || day > 31) return false;
    if (hr < 0 || hr > 23) return false;
    if (min < 0 || min > 59) return false;
    if (sec < 0 || sec > 59) return false;

    tm.Year = yr - 1970;
    tm.Month = mon;
    tm.Day = day;
    tm.Hour = hr;
    tm.Minute = min;
    tm.Second = sec;

    return true;
}

bool setRTCFromBLE(const String& dateTimeStr) {
    tmElements_t tm;

    if (!parseDateTimeString(dateTimeStr, tm)) {
        Serial.println("RTC SET FAILED: invalid datetime format");
        return false;
    }
    time_t newTime = makeTime(tm);
    // Update both TimeLib and Teensy RTC
    Teensy3Clock.set(newTime);
    setTime(newTime);
    Serial.print("RTC updated to: ");
    Serial.print(year());
    Serial.print("-");
    if (month() < 10) Serial.print('0');
    Serial.print(month());
    Serial.print("-");
    if (day() < 10) Serial.print('0');
    Serial.print(day());
    Serial.print(" ");
    if (hour() < 10) Serial.print('0');
    Serial.print(hour());
    Serial.print(":");
    if (minute() < 10) Serial.print('0');
    Serial.print(minute());
    Serial.print(":");
    if (second() < 10) Serial.print('0');
    Serial.println(second());
    logSD("RTC manually set via BLE to " + std::to_string(year()) + "-" +
          std::to_string(month()) + "-" + std::to_string(day()) + " " +
          std::to_string(hour()) + ":" + std::to_string(minute()) + ":" +
          std::to_string(second()));
    return true;
}



void setup()  {
    Serial.begin(9600);  // Start serial FIRST for early debug
    delay(1000);
    Serial.println("DEBUG: ====== SETUP START ======");
  
    Serial.println("DEBUG: Setting sync provider...");
    setSyncProvider(getTeensy3Time);

    // EARLY RTC CHECK - Before SD card is even initialized
    Serial.println("DEBUG: Checking RTC time validity...");
    Serial.print("DEBUG: Current RTC year: ");
    Serial.println(year());
    
    bool needsTimeRecovery = false;
    if (year() < 2025) {
        Serial.println("DEBUG: RTC time is INVALID (year < 2025)");
        needsTimeRecovery = true;
    } else {
        Serial.println("DEBUG: RTC time appears valid");
        Serial.print("DEBUG: Current time: ");
        digitalClockDisplay();
        Serial.println();
    }
    
    Serial.println("DEBUG: Configuring SD card pins...");
    pinMode(SDCARD_CS_PIN, OUTPUT);
    didReadConfig = false;
    SPI.setMOSI(SDCARD_MOSI_PIN);
    SPI.setSCK(SDCARD_SCK_PIN);
    
    Serial.println("DEBUG: Initializing SD card...");
    if (!SD.begin(SDCARD_CS_PIN)) {
        Serial.println("DEBUG: SD card init FAILED!");
        Serial.println("Unable to access the SD card, go to sleep.");
        delay(1000);
        doneSignal();    
    }
    Serial.println("DEBUG: SD card initialized successfully");

    // CHECK FOR UNEXPECTED RESET
    if (SD.exists("RUNNING.flg")) {
    Serial.println("WARNING: Unexpected reset detected!");
    logError("UNEXPECTED RESET DETECTED - System did not shut down cleanly");
    SD.remove("RUNNING.flg");
  }
    
    // CREATE NEW FLAG
    File flagFile = SD.open("RUNNING.flg", FILE_WRITE);
    if (flagFile) {
        flagFile.println("System started");
        flagFile.close();
    }
    
    // NOW ATTEMPT TIME RECOVERY IF NEEDED
    if (needsTimeRecovery) {
        Serial.println("DEBUG: Attempting time recovery from log file...");
        if (recoverTimeFromLog()) {
        Serial.println("DEBUG: Time recovery SUCCESSFUL!");
        Serial.print("DEBUG: New time: ");
        digitalClockDisplay();
        Serial.println();
        } else {
        Serial.println("DEBUG: Time recovery FAILED - continuing with invalid time");
        }
    };
    
    Serial.println("DEBUG: Reading configuration...");
    didReadConfig = readConfiguration();
    if (didReadConfig) {
        Serial.println("DEBUG: Configuration read successfully");
    } else {
        Serial.println("DEBUG: Configuration read FAILED");
    }
    
    Serial.println("DEBUG: Restarting serial with BAUD_RATE...");
    Serial.end();
    Serial.begin(BAUD_RATE);
    delay(500);
    
    if (SERIAL_WAIT == 1){
        while(Serial.available()==0) {
        Serial.println("DEBUG: Waiting for serial input...");
        delay(1000);
        }
    }
    
    Serial.println("");
    Serial.print("Software Version: ");
    Serial.println(SOFTWARE_VERSION);
    
    Serial.println("DEBUG: Configuring pins...");
    pinMode(done_pin, OUTPUT);
    pinMode(mos_pwr, OUTPUT);
    pinMode(mos_audio, OUTPUT);
    
    Serial.println("DEBUG: Initializing Bluefruit...");
    if (!ble.begin(VERBOSE_MODE)) {
        Serial.println("DEBUG: Bluefruit init FAILED!");
    } else {
        Serial.println("DEBUG: Bluefruit initialized successfully");
        ble.echo(false);

        // Block here to allow BLE connection + command for up to 10 seconds
        Serial.println("Always on: ");
        Serial.println(ALWAYS_ON);
        waitForBLECommands(10000);
        Serial.println("Always on: ");
        Serial.println(ALWAYS_ON);    }

    Serial.println("DEBUG: Current time: ");
    digitalClockDisplay();
    Serial.println();
    
    Serial.println("DEBUG: Current time: ");
    digitalClockDisplay();
    Serial.println();
    // Check if we should be asleep BEFORE initializing audio
    if (time_between(SLEEP_TIME, WAKE_TIME) && ALWAYS_ON == false){
        Serial.println("DEBUG: Currently in sleep window - going to sleep without audio init");
        printAndLog("Wake during sleep time - going back to sleep");
        delay(500);
        // Simple sleep without audio shutdown since audio was never started
        digitalWrite(mos_pwr, LOW);
        digitalWrite(mos_audio, LOW);
        delay(1000);
        printAndLog("Send done signal to TPL5110");
        digitalWrite(done_pin, HIGH);
        delay(1000);
        digitalWrite(done_pin, LOW);
        while(1); // Halt here waiting for power cycle
    }
    Serial.println("DEBUG: Turning on system (MOSFETs)...");
    printAndLog("Wake up");
    digitalWrite(mos_pwr, HIGH);
    digitalWrite(mos_audio, HIGH);
    Serial.println("DEBUG: MOSFETs powered on");
    Serial.println("DEBUG: Initializing audio system...");
    AudioMemory(8);
    sgtl5000_1.enable();
    sgtl5000_1.volume(1.0);         // Ensure volume is unmuted (0.0 to 1.0 scale)
    if (VOLUME == 0) {
        sgtl5000_1.muteLineout();   // completely mute
        mix1.gain(0, 0.0);
    } else {
        sgtl5000_1.unmuteLineout(); // ensure line out is active
        float mixerGain = constrain(VOLUME, 0, 100) / 100.0;  // map 0–100 → 0.0–1.0
        mix1.gain(0, mixerGain);     // input 0 of mixer
    }
    Serial.println("Audio ready.");
    
    Serial.println("DEBUG: Initializing entropy...");
    Entropy.Initialize();
    Serial.println("DEBUG: Entropy initialized");
    
    Serial.println("DEBUG: Setting up alarms...");
    Alarm.alarmRepeat(timeConstruct(ALARM_1)[0],  timeConstruct(ALARM_1)[1],  timeConstruct(ALARM_1)[2],  startPlayingAlarm1);
    Alarm.alarmRepeat(timeConstruct(ALARM_2)[0],  timeConstruct(ALARM_2)[1],  timeConstruct(ALARM_2)[2],  startPlayingAlarm2);
    Alarm.alarmRepeat(timeConstruct(ALARM_3)[0],  timeConstruct(ALARM_3)[1],  timeConstruct(ALARM_3)[2],  startPlayingAlarm3);
    Alarm.alarmRepeat(timeConstruct(ALARM_4)[0],  timeConstruct(ALARM_4)[1],  timeConstruct(ALARM_4)[2],  startPlayingAlarm4);
    Alarm.alarmRepeat(timeConstruct(ALARM_5)[0],  timeConstruct(ALARM_5)[1],  timeConstruct(ALARM_5)[2],  startPlayingAlarm5);
    Alarm.alarmRepeat(timeConstruct(ALARM_6)[0],  timeConstruct(ALARM_6)[1],  timeConstruct(ALARM_6)[2],  startPlayingAlarm6);
    Alarm.alarmRepeat(timeConstruct(ALARM_7)[0],  timeConstruct(ALARM_7)[1],  timeConstruct(ALARM_7)[2],  startPlayingAlarm7);
    Alarm.alarmRepeat(timeConstruct(ALARM_8)[0],  timeConstruct(ALARM_8)[1],  timeConstruct(ALARM_8)[2],  startPlayingAlarm8);
    Alarm.alarmRepeat(timeConstruct(ALARM_9)[0],  timeConstruct(ALARM_9)[1],  timeConstruct(ALARM_9)[2],  startPlayingAlarm9);
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
    Serial.println("DEBUG: Alarms configured successfully");
    
    Serial.println("DEBUG: Configuring BLE device name...");
    ble.println("AT+GAPDEVNAME");
    delay(200);
    String response = "";
    while (ble.available()) {
        response += (char)ble.read();
    }
    response.trim();
    int newlineIndex = response.indexOf('\n');
    String currentName = (newlineIndex != -1) ? response.substring(0, newlineIndex) : response;
    currentName.trim();
    Serial.print("DEBUG: Current BLE device name: ");
    Serial.println(currentName);
    
    if (currentName != String(RAPS_ID)) {
        Serial.print("DEBUG: Setting new BLE device name to: ");
        Serial.println(RAPS_ID);
        String cmd = String("AT+GAPDEVNAME=") + RAPS_ID;
        ble.println(cmd);
        delay(200);
        if (ble.waitForOK()) {
        Serial.println("DEBUG: BLE device name set successfully");
        ble.println("ATZ");
        delay(1000);
        } else {
        Serial.println("DEBUG: Failed to set BLE device name");
        }
    } else {
        Serial.println("DEBUG: BLE device name already correct");
    }
    
    Serial.println("DEBUG: ====== SETUP COMPLETE ======");
    }

void loop() {
  static unsigned long lastLoopTime = 0;
  static unsigned long lastStatusUpdate = 0;
  static unsigned long lastBLECheck = 0;  
  // Service audio library frequently - don't block
  if (millis() - lastLoopTime < 1000) {
      delay(10);  // Short delay to service audio/alarms
      Alarm.delay(0);  // Service alarms without blocking
      return;
  }
  lastLoopTime = millis();
    
  // ------------------------------------------------------------
  // 1. PRIORITY: Handle pending SD operations
  // ------------------------------------------------------------
  if (sdOperationPending) {
      Serial.println("DEBUG: Processing pending SD operation...");
      // Begin safe SD access
      beginSDOperation();
      // Execute the pending operation
      switch (pendingSDOperation) {
          case 1: // List files
              listSDFiles();
              break;
          case 2: // Write config
              if (writeConfigFileSafe()) {
                  ble.print("AT+BLEUARTTX=");
                  ble.println("CONFIG UPDATED");
                  Serial.println("Config updated successfully");
                  // Always reboot after config write
                  Serial.println("DEBUG: Rebooting system for config changes...");
                  ble.print("AT+BLEUARTTX=");
                  ble.println("REBOOTING...");
                  delay(500);  // Give BLE time to send
                  // Teensy software reset
                  SCB_AIRCR = 0x05FA0004;  // Reset via ARM's System Control Block
              } else {
                  ble.print("AT+BLEUARTTX=");
                  ble.println("WRITE FAILED");
                  Serial.println("Config write failed!");
              }
              break;
      }
      // End safe SD access (resumes audio if needed)
      endSDOperation();
      // Clear the pending flag
      sdOperationPending = false;
      pendingSDOperation = 0;
      Serial.println("DEBUG: SD operation complete");
      return; // Exit loop to let audio resume properly
  }
  // ------------------------------------------------------------
  // 2. Handle config update (set flag but don't execute yet)
  // ------------------------------------------------------------
  if (configUpdatePending) {
      Serial.println("DEBUG: Config update requested...");
      bool validVar = false;

      if (pendingVarName == "SLEEP_TIME") {
          if (SLEEP_TIME) free(SLEEP_TIME);
          SLEEP_TIME = strdup(pendingValue.c_str());
          validVar = true;
      }
      else if (pendingVarName == "WAKE_TIME") {
          if (WAKE_TIME) free(WAKE_TIME);
          WAKE_TIME = strdup(pendingValue.c_str());
          validVar = true;
      }
      else if (pendingVarName == "VOLUME") {
          VOLUME = pendingValue.toInt();  // Read the new volume value (0–100)
          VOLUME = constrain(VOLUME, 0, 100);
          if (VOLUME == 0) {
              sgtl5000_1.muteLineout();    // completely mute
              mix1.gain(0, 0.0);           // also set mixer to zero just in case
          } else {
              sgtl5000_1.unmuteLineout();  // ensure line out is active
              float mixerGain = VOLUME / 100.0;
              mix1.gain(0, mixerGain);
          }
          validVar = true;
      }
      else if (pendingVarName == "ALWAYS_ON") {
          ALWAYS_ON = (pendingValue == "true" || pendingValue == "1");
          validVar = true;
      }
      else if (pendingVarName == "RAPS_ID") {
          if (RAPS_ID) free(RAPS_ID);
          RAPS_ID = strdup(pendingValue.c_str());
          validVar = true;
      }
      else if (pendingVarName.startsWith("ALARM_")) {
          int alarmNum = pendingVarName.substring(6).toInt();
          if (alarmNum >= 1 && alarmNum <= 24) {
              char** alarmPtr = nullptr;
              switch(alarmNum) {
                  case 1: alarmPtr = &ALARM_1; break;
                  case 2: alarmPtr = &ALARM_2; break;
                  case 3: alarmPtr = &ALARM_3; break;
                  case 4: alarmPtr = &ALARM_4; break;
                  case 5: alarmPtr = &ALARM_5; break;
                  case 6: alarmPtr = &ALARM_6; break;
                  case 7: alarmPtr = &ALARM_7; break;
                  case 8: alarmPtr = &ALARM_8; break;
                  case 9: alarmPtr = &ALARM_9; break;
                  case 10: alarmPtr = &ALARM_10; break;
                  case 11: alarmPtr = &ALARM_11; break;
                  case 12: alarmPtr = &ALARM_12; break;
                  case 13: alarmPtr = &ALARM_13; break;
                  case 14: alarmPtr = &ALARM_14; break;
                  case 15: alarmPtr = &ALARM_15; break;
                  case 16: alarmPtr = &ALARM_16; break;
                  case 17: alarmPtr = &ALARM_17; break;
                  case 18: alarmPtr = &ALARM_18; break;
                  case 19: alarmPtr = &ALARM_19; break;
                  case 20: alarmPtr = &ALARM_20; break;
                  case 21: alarmPtr = &ALARM_21; break;
                  case 22: alarmPtr = &ALARM_22; break;
                  case 23: alarmPtr = &ALARM_23; break;
                  case 24: alarmPtr = &ALARM_24; break;
              }
              if (alarmPtr) {
                  if (*alarmPtr) free(*alarmPtr);
                  *alarmPtr = strdup(pendingValue.c_str());
                  validVar = true;
              }
          }
      }
      if (validVar) {
          sdOperationPending = true;
          pendingSDOperation = 2;
      } else {
          ble.print("AT+BLEUARTTX=");
          ble.println("UNKNOWN VARIABLE");
          Serial.println("Unknown variable, nothing written");
      }
      configUpdatePending = false;
      return;
  }
  // ------------------------------------------------------------
  // 3. Normal operation: time display and fault check
  // ------------------------------------------------------------
  digitalClockDisplay();
  Serial.println();
  fault_check();
  
  // ------------------------------------------------------------
  // 4. Check BLE UART for commands
  // ------------------------------------------------------------
  static bool bleEnabled = true;  // Set to false to disable BLE polling
  if (bleEnabled && millis() - lastBLECheck > 500) {
    lastBLECheck = millis();
    // Request BLE UART data
    ble.println("AT+BLEUARTRX");
    // Wait until data is available or a timeout occurs
    unsigned long start = millis();
    while (!ble.available() && millis() - start < 50) {
              // optional: yield(); 

    }
    if (ble.available()) {
        ble.readline();                     // Read BLE response once
        String command = String(ble.buffer); // store as String
        command.trim();
        // Serial debugging (optional)
        Serial.print("RAW RX: '");
        Serial.print(ble.buffer);
        Serial.println("'");
        
        if (strcmp(ble.buffer, "OK") != 0 && strcmp(ble.buffer, "ERROR") != 0) {
          // HELLO command
          if (command == "HELLO") {
              ble.print("AT+BLEUARTTX=");
              ble.println("HI FRIEND");
              Serial.println("Sent: HI FRIEND");
          }
          // STATUS command
          else if (command == "STATUS") {
              ble.print("AT+BLEUARTTX=");
              ble.print("Version: ");
              ble.println(SOFTWARE_VERSION);
              ble.print("AT+BLEUARTTX=");
              ble.print("Playing: ");
              ble.println(playWav1.isPlaying() ? "YES" : "NO");
              ble.print("AT+BLEUARTTX=");
              ble.print("File: ");
              ble.println(active_file);
              ble.print("AT+BLEUARTTX=");
              ble.print("Volume: ");
              ble.println(VOLUME);
              ble.print("AT+BLEUARTTX=");
              ble.print("Sample: ");
              ble.println(sampleNumber);
              Serial.println("Sent: STATUS info");
          }
          // GET command
          else if (command.startsWith("GET ")) {
              String varName = command.substring(4);
              varName.trim();
              ble.print("AT+BLEUARTTX=");

              if (varName == "VOLUME") {
                  ble.print("VOLUME=");
                  ble.println(VOLUME);
              }
              else if (varName == "WAKE_TIME") {
                  ble.print("WAKE_TIME=");
                  ble.println(WAKE_TIME ? WAKE_TIME : "NULL");
              }
              else if (varName == "SLEEP_TIME") {
                  ble.print("SLEEP_TIME=");
                  ble.println(SLEEP_TIME ? SLEEP_TIME : "NULL");
              }
              else if (varName == "ALWAYS_ON") {
                  ble.print("ALWAYS_ON=");
                  ble.println(ALWAYS_ON ? "1" : "0");
              }
              else if (varName == "RAPS_ID") {
                  ble.print("RAPS_ID=");
                  ble.println(RAPS_ID ? RAPS_ID : "NULL");
              }
              else if (varName.startsWith("ALARM_")) {
                  int alarmNum = varName.substring(6).toInt();
                  char* alarmTime = nullptr;
                  switch(alarmNum) {
                      case 1: alarmTime = ALARM_1; break;
                      case 2: alarmTime = ALARM_2; break;
                      case 3: alarmTime = ALARM_3; break;
                      case 4: alarmTime = ALARM_4; break;
                      case 5: alarmTime = ALARM_5; break;
                      case 6: alarmTime = ALARM_6; break;
                      case 7: alarmTime = ALARM_7; break;
                      case 8: alarmTime = ALARM_8; break;
                      case 9: alarmTime = ALARM_9; break;
                      case 10: alarmTime = ALARM_10; break;
                      case 11: alarmTime = ALARM_11; break;
                      case 12: alarmTime = ALARM_12; break;
                      case 13: alarmTime = ALARM_13; break;
                      case 14: alarmTime = ALARM_14; break;
                      case 15: alarmTime = ALARM_15; break;
                      case 16: alarmTime = ALARM_16; break;
                      case 17: alarmTime = ALARM_17; break;
                      case 18: alarmTime = ALARM_18; break;
                      case 19: alarmTime = ALARM_19; break;
                      case 20: alarmTime = ALARM_20; break;
                      case 21: alarmTime = ALARM_21; break;
                      case 22: alarmTime = ALARM_22; break;
                      case 23: alarmTime = ALARM_23; break;
                      case 24: alarmTime = ALARM_24; break;
                  }
                  if (alarmTime) {
                      ble.print(varName);
                      ble.print("=");
                      ble.println(alarmTime);
                  } else {
                      ble.println("NULL");
                  }
              }
              else {
                  ble.println("UNKNOWN VAR");
              }
              Serial.print("Sent: GET ");
              Serial.println(varName);
          }
          // SET command
          else if (command.startsWith("SET ")) {
              int firstSpace = command.indexOf(' ');
              int secondSpace = command.indexOf(' ', firstSpace + 1);
              if (secondSpace > 0) {
                  String varName = command.substring(firstSpace + 1, secondSpace);
                  String value = command.substring(secondSpace + 1);
                  varName.trim();
                  value.trim();
                  Serial.print("Setting ");
                  Serial.print(varName);
                  Serial.print(" to ");
                  Serial.println(value);
                  // Special case: live RTC update, no config write, no reboot
                  if (varName == "RTC") {
                      Serial.print("DEBUG RTC value received: '");
                      Serial.print(value);
                      Serial.print("' len=");
                      Serial.println(value.length());
                      if (setRTCFromBLE(value)) {
                          ble.print("AT+BLEUARTTX=");
                          ble.println("RTC UPDATED");
                          ble.print("AT+BLEUARTTX=");
                          ble.print("NEW TIME: ");
                          ble.print(year());
                          ble.print("-");
                          if (month() < 10) ble.print('0');
                          ble.print(month());
                          ble.print("-");
                          if (day() < 10) ble.print('0');
                          ble.print(day());
                          ble.print(" ");
                          if (hour() < 10) ble.print('0');
                          ble.print(hour());
                          ble.print(":");
                          if (minute() < 10) ble.print('0');
                          ble.print(minute());
                          ble.print(":");
                          if (second() < 10) ble.print('0');
                          ble.println(second());
                      } else {
                          ble.print("AT+BLEUARTTX=");
                          ble.println("RTC SET FAILED");
                          ble.print("AT+BLEUARTTX=");
                          ble.println("FORMAT: SET RTC YYYY-MM-DD HH:MM:SS");
                      }
                  } else {
                      pendingVarName = varName;
                      pendingValue = value;
                      configUpdatePending = true;
                      return;
                  }
              }
              else {
                  ble.print("AT+BLEUARTTX=");
                  ble.println("INVALID FORMAT");
              }
          }
          // HELP command
          else if (command == "HELP") {
              ble.print("AT+BLEUARTTX=");
              ble.println("=== COMMANDS ===");
              delay(50);
              ble.print("AT+BLEUARTTX=");
              ble.println("HELLO - test connection");
              delay(50);
              ble.print("AT+BLEUARTTX=");
              ble.println("STATUS - system info");
              delay(50);
              ble.print("AT+BLEUARTTX=");
              ble.println("LIST - show SD files");
              delay(50);
              ble.print("AT+BLEUARTTX=");
              ble.println("GET <var> - read config");
              delay(50);
              ble.print("AT+BLEUARTTX=");
              ble.println("SET <var> <val> - update");
              delay(50);
              ble.print("AT+BLEUARTTX=");
              ble.println("SET RTC YYYY-MM-DD HH:MM:SS");
              delay(50);
              ble.print("AT+BLEUARTTX=");
              ble.println("PLAY <file> - play file");
              delay(50);
              ble.print("AT+BLEUARTTX=");
              ble.println("STOP - stop audio");
              delay(50);
              ble.print("AT+BLEUARTTX=");
              ble.println("REBOOT - restart system");
              Serial.println("Sent: HELP");
          }
          // REBOOT command
          else if (command == "REBOOT") {
              Serial.println("Manual reboot requested");
              ble.print("AT+BLEUARTTX=");
              ble.println("REBOOTING NOW...");
              delay(500);
              SCB_AIRCR = 0x05FA0004;
          }
      }
      Alarm.delay(1000);  // KEEP THE OLD BLOCKING DELAY
  }
}

  
  // ------------------------------------------------------------
  // 5. Status message every 30 seconds (reduced from 5)
  // ------------------------------------------------------------
  if (bleEnabled && millis() - lastStatusUpdate > 30000) {
      lastStatusUpdate = millis();
      ble.print("AT+BLEUARTTX=");
      ble.print("Time: ");
      ble.print(hour());
      ble.print(":");
      if (minute() < 10) ble.print('0');
      ble.print(minute());
      ble.print(":");
      if (second() < 10) ble.print('0');
      ble.print(second());
      ble.print(" | File: ");
      ble.println(active_file);
  }
}