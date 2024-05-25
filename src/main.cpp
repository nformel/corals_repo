//This code plays the loaded wav files on a schedule AND
//will tell the TPL510 that it's done when Sound Off is trigered

//LIBRARIES
#include <Arduino.h>
#include <TimeLib.h>
#include <TimeAlarms.h>

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
bool USE_SAMP = true; //set to false if not using sample number
#define NUM_SAMP 3 //number of samples for each FILE_BASE
#define BAUDE_RATE 115200

//ALARM TIMES
#define ALARM_1 "00:30:00"
#define ALARM_2 "01:30:00"
#define ALARM_3 "02:30:00"
#define ALARM_4 "03:30:00"
#define ALARM_5 "04:30:00"
#define ALARM_6 "05:30:00"
#define ALARM_7 "06:30:00"
#define ALARM_8 "07:30:00"
#define ALARM_9 "08:30:00"
#define ALARM_10 "09:30:00"
#define ALARM_11 "10:30:00"
#define ALARM_12 "11:30:00"
#define ALARM_13 "12:30:00"
#define ALARM_14 "13:30:00"
#define ALARM_15 "14:30:00"
#define ALARM_16 "15:30:00"
#define ALARM_17 "16:30:00"
#define ALARM_18 "17:30:00"
#define ALARM_19 "18:30:00"
#define ALARM_20 "19:30:00"
#define ALARM_21 "20:30:00"
#define ALARM_22 "21:30:00"
#define ALARM_23 "22:30:00"
#define ALARM_24 "23:30:00"

//SOUND FILE BASE NAMES
#define ALARM_1_FILE_BASE "TKLN"
#define ALARM_2_FILE_BASE "TKLN"
#define ALARM_3_FILE_BASE "TKLN"
#define ALARM_4_FILE_BASE "TKLN"
#define ALARM_5_FILE_BASE "TKDN"
#define ALARM_6_FILE_BASE "TKDN"
#define ALARM_7_FILE_BASE "TKMN"
#define ALARM_8_FILE_BASE "TKMN"
#define ALARM_9_FILE_BASE "TKMN"
#define ALARM_10_FILE_BASE "TKMN"
#define ALARM_11_FILE_BASE "TKMN"
#define ALARM_12_FILE_BASE "TKMN"
#define ALARM_13_FILE_BASE "TKAF"
#define ALARM_14_FILE_BASE "TKAF"
#define ALARM_15_FILE_BASE "TKAF"
#define ALARM_16_FILE_BASE "TKAF"
#define ALARM_17_FILE_BASE "TKAF"
#define ALARM_18_FILE_BASE "TKAF"
#define ALARM_19_FILE_BASE "TKDK"
#define ALARM_20_FILE_BASE "TKDK"
#define ALARM_21_FILE_BASE "TKEV"
#define ALARM_22_FILE_BASE "TKEV"
#define ALARM_23_FILE_BASE "TKEV"
#define ALARM_24_FILE_BASE "TKEV"

// Wake Time
int startH = 8;
int startM = 0;
int startS = 0;
// Play Time (first alarm) [18:0:0 for real]
int playH = 8;
int playM = 5;
int playS = 0;
// Sleep Time
int stopH = 16;
int stopM = 59;
int stopS = 0;

// initialize the sample number as global variable
int sampleNumber = 0;

// SD Logging file instantiation
File myFile;

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
  Serial.println();
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

// ALARM FUNCTIONS
// Audio file 1
void startPlayingAlarm1() {
  stopFile();
  printAndLog("Alarm1");  
  sampleNumber = Entropy.random(1, NUM_SAMP+1);  
  playFile(makeFileNameString(ALARM_1_FILE_BASE, sampleNumber, USE_SAMP)); //make file name from alarm hour and sample number.
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 2
void startPlayingAlarm2() {
  stopFile();
  printAndLog("Alarm2");  
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
  // if no audio is playing, start the appropriate default track
  if (playWav1.isPlaying() == false){ 
  printAndLog("Fault Check: System not playing.");
  
    // Check if sampleNumber is set to non-zero (e.g. system has stayed awake since alarm tripped)
    // If zero, then system went through a power cycle. Re-randomize.
    if (sampleNumber == 0){
      sampleNumber = Entropy.random(1, NUM_SAMP+1);
    }
  
    //Changed this to go for 24 hrs
    if (time_between(ALARM_1, ALARM_2)){
      playFile(makeFileNameString(ALARM_1_FILE_BASE, sampleNumber, true));
      delay(250);
    }
    else if (time_between(ALARM_2, ALARM_3)){
      playFile(makeFileNameString(ALARM_2_FILE_BASE, sampleNumber, true));
      delay(250);
    }
    else if (time_between(ALARM_3, ALARM_4)){
      playFile(makeFileNameString(ALARM_3_FILE_BASE, sampleNumber, true));
      delay(250);
    }
    else if (time_between(ALARM_4, ALARM_5)){
      playFile(makeFileNameString(ALARM_4_FILE_BASE, sampleNumber, true));
      delay(250);
    }
    else if (time_between(ALARM_5, ALARM_6)){
      playFile(makeFileNameString(ALARM_5_FILE_BASE, sampleNumber, true));
      delay(250);
    }
    else if (time_between(ALARM_6, ALARM_7)){
      playFile(makeFileNameString(ALARM_6_FILE_BASE, sampleNumber, true));
      delay(250);
    }
    else if (time_between(ALARM_7, ALARM_8)){
      playFile(makeFileNameString(ALARM_7_FILE_BASE, sampleNumber, true));
      delay(250);
    }
    else if (time_between(ALARM_8, ALARM_9)){
      playFile(makeFileNameString(ALARM_8_FILE_BASE, sampleNumber, true));
      delay(250);
    }
    else if (time_between(ALARM_9, ALARM_10)){
      playFile(makeFileNameString(ALARM_9_FILE_BASE, sampleNumber, true));
      delay(250);
    }
    else if (time_between(ALARM_10, ALARM_11)){
      playFile(makeFileNameString(ALARM_10_FILE_BASE, sampleNumber, true));
      delay(250);
    }
    else if (time_between(ALARM_11, ALARM_12)){
      playFile(makeFileNameString(ALARM_11_FILE_BASE, sampleNumber, true));
      delay(250);
    }
    else if (time_between(ALARM_12, ALARM_13)){
      playFile(makeFileNameString(ALARM_12_FILE_BASE, sampleNumber, true));
      delay(250);
    }
    else if (time_between(ALARM_13, ALARM_14)){
      playFile(makeFileNameString(ALARM_13_FILE_BASE, sampleNumber, true));
      delay(250);
    }
    else if (time_between(ALARM_14, ALARM_15)){
      playFile(makeFileNameString(ALARM_14_FILE_BASE, sampleNumber, true));
      delay(250);
    }
    else if (time_between(ALARM_15, ALARM_16)){
      playFile(makeFileNameString(ALARM_15_FILE_BASE, sampleNumber, true));
      delay(250);
    }
    else if (time_between(ALARM_16, ALARM_17)){
      playFile(makeFileNameString(ALARM_16_FILE_BASE, sampleNumber, true));
      delay(250);
    }
    else if (time_between(ALARM_17, ALARM_18)){
      playFile(makeFileNameString(ALARM_17_FILE_BASE, sampleNumber, true));
      delay(250);
    }
    else if (time_between(ALARM_18, ALARM_19)){
      playFile(makeFileNameString(ALARM_18_FILE_BASE, sampleNumber, true));
      delay(250);
    }
    else if (time_between(ALARM_19, ALARM_20)){
      playFile(makeFileNameString(ALARM_19_FILE_BASE, sampleNumber, true));
      delay(250);
    }
    else if (time_between(ALARM_20, ALARM_21)){
      playFile(makeFileNameString(ALARM_20_FILE_BASE, sampleNumber, true));
      delay(250);
    }
    else if (time_between(ALARM_21, ALARM_22)){
      playFile(makeFileNameString(ALARM_21_FILE_BASE, sampleNumber, true));
      delay(250);
    }
    else if (time_between(ALARM_22, ALARM_23)){
      playFile(makeFileNameString(ALARM_22_FILE_BASE, sampleNumber, true));
      delay(250);
    }     
    else if (time_between(ALARM_23, ALARM_24)){
      playFile(makeFileNameString(ALARM_23_FILE_BASE, sampleNumber, true));
      delay(250);
    }
    else if (time_between(ALARM_24, ALARM_1)){
      playFile(makeFileNameString(ALARM_24_FILE_BASE, sampleNumber, true));
      delay(250);
    }
  }

  else { //Serial.println("No, I'm stuck here"); // Do nothing, system is on and playing. No issue.  
  }
  //Serial.println("I promise, it's here that I am stuck");
}

void setup()  {
  // set the Time library to use Teensy 3.0's RTC to keep time
  setSyncProvider(getTeensy3Time);
  
  // Set up serial for debugging
  Serial.begin(BAUDE_RATE);

  //adding delay for tesing when it goes through set up so I can catch the terminal traff
  Serial.println("5s Set up delay");
  delay(5000);

  // This is where I would want to read in all of my configurations

  //Digital pin configurations
  pinMode(done_pin, OUTPUT);
  pinMode(mos_pwr, OUTPUT);
  pinMode(mos_audio, OUTPUT);

  //SD card Setup
  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here, but print a message repetitively
    Serial.println("Unable to access the SD card, go to sleep.");
    delay(1000);
    doneSignal();
  }

  //check digital clock once in setup
  digitalClockDisplay();

  // Turn on System
  printAndLog("Wake up system");
  digitalWrite(mos_pwr, HIGH);
  digitalWrite(mos_audio, HIGH);

  // WAV Player Setup
  AudioMemory(8);
  sgtl5000_1.enable();
  sgtl5000_1.volume(1);

  //Initialize the entropy funcition
  Entropy.Initialize();   

  //Set up alarms
  // using timeConstruct to insert hr, min and sec into Alarm definitions
  Alarm.alarmRepeat(timeConstruct(ALARM_1)[0], timeConstruct(ALARM_1)[1], timeConstruct(ALARM_1)[2], startPlayingAlarm1);
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

}

void loop() {
  digitalClockDisplay(); //serial print the time according to RTC
  fault_check(); //If wavfile isn't playing, force on based on time
  Alarm.delay(1000); // wait one second between clock display
}
