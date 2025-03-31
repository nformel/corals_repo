//This code plays the loaded wav files on a schedule AND
//will tell the TPL510 that it's done when Sound Off is trigered
#define SOFTTWARE_VERSION "test v1.2"

//LIBRARIES
#include <Arduino.h>
#include <TimeLib.h>
#include <string>
#include <array>
using namespace std;
#define SEC_PRE_MIDNIGHT 86399
#define MIDNIGHT_IN_SEC 86400
#define BAUDE_RATE = 115200
int fault_cnt = 0;

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
bool USE_SAMP = false; //set to false if not using sample number
int sampleNumber = 0;
#define NUM_SAMP 3 //number of samples for each FILE_BASE

// Wake Time
int startH = 6;
int startM = 0;
int startS = 0;

// Play Time (first alarm) [18:0:0 for real]
int playH = 7;
int playM = 0;
int playS = 0;

// Sleep Time
int stopH = 18;
int stopM = 0;
int stopS = 0;

//Other
// SD Logging file instantiation
File myFile;
String active_file = "no file";

//settings from config file
// Alarm times
//ALARM TIMES
#define ALARM_1 "7:00:00"
#define ALARM_2 "12:51:10"
#define ALARM_3 "12:51:20"
#define ALARM_4 "12:51:30"
#define ALARM_5 "12:51:40"
#define ALARM_6 "12:51:50"
#define ALARM_7 "12:52:00"
#define ALARM_8 "12:52:10"
#define ALARM_9 "12:52:20"
#define ALARM_10 "12:52:30"
#define ALARM_11 "12:52:40"
#define ALARM_12 "12:52:50"
#define ALARM_13 "12:53:00"
#define ALARM_14 "12:53:10"
#define ALARM_15 "12:53:20"
#define ALARM_16 "12:53:30"
#define ALARM_17 "12:53:40"
#define ALARM_18 "12:53:50"
#define ALARM_19 "12:54:00"
#define ALARM_20 "12:54:10"
#define ALARM_21 "12:54:20"
#define ALARM_22 "12:54:30"
#define ALARM_23 "12:54:40"
#define ALARM_24 "14:54:50"

// FILE BASES
#define ALARM_1_FILE_BASE "SDTEST1"
#define ALARM_2_FILE_BASE "SDTEST2"
#define ALARM_3_FILE_BASE "SDTEST3"
#define ALARM_4_FILE_BASE "SDTEST4"
#define ALARM_5_FILE_BASE "SDTEST1"
#define ALARM_6_FILE_BASE "SDTEST2"
#define ALARM_7_FILE_BASE "SDTEST3"
#define ALARM_8_FILE_BASE "SDTEST4"
#define ALARM_9_FILE_BASE "SDTEST1"
#define ALARM_10_FILE_BASE "SDTEST2"
#define ALARM_11_FILE_BASE "SDTEST3"
#define ALARM_12_FILE_BASE "SDTEST4"
#define ALARM_13_FILE_BASE "SDTEST1"
#define ALARM_14_FILE_BASE "SDTEST2"
#define ALARM_15_FILE_BASE "SDTEST3"
#define ALARM_16_FILE_BASE "SDTEST4"
#define ALARM_17_FILE_BASE "SDTEST1"
#define ALARM_18_FILE_BASE "SDTEST2"
#define ALARM_19_FILE_BASE "SDTEST3"
#define ALARM_20_FILE_BASE "SDTEST4"
#define ALARM_21_FILE_BASE "SDTEST1"
#define ALARM_22_FILE_BASE "SDTEST2"
#define ALARM_23_FILE_BASE "SDTEST3"
#define ALARM_24_FILE_BASE "SDTEST4"

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

//Convert hours minutes and seconds to seconds after midnight
int time2sec (int h, int m, int s) {
  int timeSec = s + m * 60 + h * 3600;
  return timeSec;
}

// construct time string of from "hh:mm:ss" from integer for hours, minutes and seconds (invers of timeConstruct)
char * make_time(int H, int M, int S){
  char* time = (char *)malloc(9);
  //add terminating character to string
  strcpy(time, "\0");


  // convert ints to strings
  char HString[2];
  itoa(H, HString, 10);
  char MString[2];
  itoa(M, MString, 10);
  char SString[2];
  itoa(S, SString, 10);

  // add hours
  if(H<10){
    char hrString[3] = "0";
    strcat(hrString, HString);
    strcat(hrString, ":");
    strcat(time, hrString);
  } else{
    strcat(HString, ":");
    strcat(time, HString);
  }

  // add minutes
  if(M<10){
    char minString[3] = "0";
    strcat(minString, MString);
    strcat(minString, ":");
    strcat(time, minString);
  } else{
    strcat(MString, ":");
    strcat(time, MString);
  }

  // add seconds
  if(S<10){
    char secString[3] = "0";
    strcat(secString, SString);
    strcat(time, secString);
  } else{
    strcat(time, SString);
  }
  return time;
}

// playFile function from WAV file player
void playFile(std::string filename) { //const char string[]
  Serial.println("Playing file:");
  Serial.println(filename.c_str());
  playWav1.play(filename.c_str());
  active_file = filename.c_str();
  delay(10);
}

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

//Function to determine if the present time is between two values
bool time_between(std::string startTime, std::string stopTime) {

  Serial.println("running time_between function");
  delay(100);

  Serial.print("startTime: ");
  Serial.println(startTime.c_str());
  delay(1000);
  Serial.print("stopTime: ");
  Serial.println(stopTime.c_str());
  delay(1000);
  
  // Break up times into H, M and S
  int nowH = hour();
  int nowM = minute();
  int nowS = second();

  int startH = timeConstruct(startTime)[0];
  int startM = timeConstruct(startTime)[1];
  int startS = timeConstruct(startTime)[2];

  int stopH = timeConstruct(stopTime)[0];
  int stopM = timeConstruct(stopTime)[1];
  int stopS = timeConstruct(stopTime)[2];

  // convert times to seconds after midnight
  int nowSeconds = time2sec(nowH, nowM, nowS);
  int startSeconds = time2sec(startH, startM, startS);
  int stopSeconds = time2sec(stopH, stopM, stopS);
  bool rtrn = 0;
  
  if (startSeconds < stopSeconds) {
    //Serial.println("Play interval does not include midnight");
    delay(100);
    if (0 <= nowSeconds && nowSeconds < startSeconds) {
      //Serial.println("Case 1");
      rtrn = 0;
    }
    else if (startSeconds <= nowSeconds && nowSeconds < stopSeconds) {
      //Serial.println("Case 2");
      rtrn = 1;
    }
    else if (stopSeconds <= nowSeconds && nowSeconds < SEC_PRE_MIDNIGHT) {
      //Serial.println("Case 3");
      rtrn = 0;
    }
  }
  else if (startSeconds > stopSeconds) {
    //Serial.println("Play interval includes midnight");
    delay(100);
    if (0 <= nowSeconds && nowSeconds < stopSeconds) {
      //Serial.println("Case 1");
      rtrn = 1;
    }
    else if (stopSeconds <= nowSeconds && nowSeconds < startSeconds) {
      //Serial.println("Case 2");
      rtrn = 0;
    }
    else if (startSeconds <= nowSeconds && nowSeconds <= MIDNIGHT_IN_SEC) {
      //Serial.println("Case 3");
      rtrn = 1;
    }
  }
  else{//Serial.println("no case met");
    }

  return rtrn;
}

void fault_check(){
  Serial.println("run fault_check");
  //Decide if system should be on and/or playing OR go to sleep
  //create wake time string from integers
  //Serial.println("load wake time");
  char * wake_time = make_time(startH, startM, startS);
  //Serial.println("wake_time is: ");
  //Serial.println(wake_time);

  //create play time string from integers
  //Serial.println("load play time");
  char * play_time = make_time(playH, playM, playS);
  //Serial.println("play_time is: ");
  //Serial.println(play_time);

  //create sleep time string from integers
  //Serial.println("load sleep time");
  char * sleep_time = make_time(stopH, stopM, stopS);
  //Serial.println("sleep_time is: ");
  //Serial.println(sleep_time);

  //Determine if sysem should be on
  bool mode_on = time_between(wake_time, sleep_time);
  if (mode_on == false) {
    Serial.println("Fault: Sleep.");
    delay(250);
    free(wake_time);
    free(play_time);
    free(sleep_time);
    digitalWrite(mos_pwr, LOW);
    digitalWrite(mos_audio, LOW);
    digitalWrite(done_pin, HIGH);
    delay(1000);
    digitalWrite(done_pin, LOW);
  } else{
  // Keep system on
  Serial.println("Fault: system should be on");
  digitalWrite(mos_pwr, HIGH);
  digitalWrite(mos_audio, HIGH);

  Serial.println("passed point 1");

  free(wake_time);
  free(play_time);
  free(sleep_time);

  Serial.println("passed point 2");
  }

  Serial.println("passed point 3");

  //Determine if sysem should be playing
  bool mode_play = time_between(play_time, sleep_time);

  Serial.println("passed point 4");

  if (mode_play){Serial.println("System should be playing");}
  else {Serial.println("System should be on, but not playing yet");};
  
  Serial.println("passed point 5");

  Serial.println("playWav1.isPlaying() == true: ");
  Serial.println((playWav1.isPlaying() == true));

  Serial.println("I made it past evaluating that statement");

  if (playWav1.isPlaying() == true && mode_play == true){
    Serial.println("System is already playing, all is well");
  } else if (playWav1.isPlaying() == false && mode_play == true){ 
    Serial.println("Fault: System was not playing but it should be.");

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
  }

  else {
    //passed point 
  }
  //Serial.println("I promise, it's here that I am stuck");
}

void setup()  {
  // set the Time library to use Teensy 3.0's RTC to keep time
  setSyncProvider(getTeensy3Time);

    // Set up serial for debugging
  Serial.begin(115200);

  while(Serial.available()==0) {
      Serial.println("Send any charcter to continue"); //only here for testing. REMOVE for deployment
      delay(1000);
  }

  ////Print using foo function
  //Serial.println("print using foo function in main  loop");
  //char * p = foo ();
  //Serial.println("Serial print the char*p which is the return variable of foo");
  //Serial.println (p);
  //free (p);

  //Digital pin configurations
  pinMode(done_pin, OUTPUT);
  pinMode(mos_pwr, OUTPUT);
  pinMode(mos_audio, OUTPUT);

  // Software version
  Serial.println("");
  Serial.print("Software Version: ");
  Serial.println(SOFTTWARE_VERSION);

  //check digital clock once in setup
  Serial.println("now time: ");
  digitalClockDisplay();
  Serial.println();

  //create wake time string from integers
  Serial.println("load wake time");
  char * wake_time = make_time(startH, startM, startS);
  Serial.println("wake_time is: ");
  Serial.println(wake_time);

  //create play time string from integers
  Serial.println("load play time");
  char * play_time = make_time(playH, playM, playS);
  Serial.println("play_time is: ");
  Serial.println(play_time);

  //create sleep time string from integers
  Serial.println("load sleep time");
  char * sleep_time = make_time(stopH, stopM, stopS);
  Serial.println("sleep_time is: ");
  Serial.println(sleep_time);

  //Determine if sysem should be on
  Serial.println("mode_on result is: ");
  bool mode_on = time_between(wake_time, sleep_time);
  if (mode_on){Serial.println("True");}
  else {Serial.println("False");};

  //free dynamically allocated variables
  free(wake_time);
  free(play_time);
  free(sleep_time);

}

void loop() {
  digitalClockDisplay(); //serial print the time according to RTC
  Serial.println();
  Serial.print("Fault count is: ");
  Serial.println(fault_cnt);

  if (fault_cnt == 10){
    fault_check();
    fault_cnt = 0;
  } else{fault_cnt += 1;}


  delay(1000); // wait one second between clock display
}
