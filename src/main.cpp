//This code plays the loaded wav files on a schedule AND
//will tell the TPL510 that it's done when Sound Off is trigered
//final version as of 10/18/22 4:47pm

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

//CONFIGURABLE DEFINITIONS (see config file)
#define ALARM_1 "16:42:00"
#define ALARM_2 "16:43:00"
#define ALARM_3 "16:44:00"
#define ALARM_4 "21:00:00"
#define ALARM_5 "22:00:00"
#define ALARM_6 "23:00:00"
#define ALARM_7 "00:00:00"
#define ALARM_8 "01:00:00"
#define ALARM_9 "02:00:00"
#define ALARM_10 "03:00:00"
#define ALARM_11 "04:00:00"
#define ALARM_12 "05:00:00"
#define SAMPLE_LOCATION "TKP"
#define BAUDE_RATE 115200
// Wake Time
int startH = 16;
int startM = 0;
int startS = 0;
// Play Time (first alarm) [18:0:0 for real]
int playH = 16;
int playM = 42;
int playS = 0;
// Sleep Time
int stopH = 23;
int stopM = 0;
int stopS = 0;

// declare the sample number as global variable
int sampleNumber;

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
void logSD(const char string[]) {
  myFile = SD.open("LOG.txt", FILE_WRITE);
  // if the file opened okay, write to it:
  if (myFile) {
    myFile.print(present());
    myFile.print("  ");
    myFile.println(string);
    myFile.close();
    // if the file didn't open, print an error:
  } else {
    Serial.println("error opening file");
  }
}
// Wrap logging and serial print into a single function
void printAndLog(const char string[]){
  Serial.println(string);
  logSD(string);
  }

// Add an integer to a string
const char * customAdd(std::string string, int b){
    std::string concat = string + std::to_string(b);
    const char * result = concat.c_str(); //convert string to pointer
    return result;  
}

// construct playback file name from an hour + sample number (e.g. 18TKP1.WAV)
const char * makeFileName(int hr, int samp){
    std::string hour = "00";
    if (hr < 10){
      hour = "0" + std::to_string(hr);
    }
    else {
      hour = std::to_string(hr);
    }
    
    std::string concat = hour + SAMPLE_LOCATION + std::to_string(samp) + ".WAV";
    const char * result = concat.c_str(); //convert string to pointer
    return result;  
}

// WAV FILE PLAYER AND TPL5110 HELPER FUNCTIONS
// playFile function from WAV file player
void playFile(const char string[]) {
  printAndLog("Playing file:");
  printAndLog(string);
  const char *filename = string;
  playWav1.play(filename);
  delay(10);
}
// Turn off sound
void stopFile() {
  printAndLog("Stopping audio");
  playWav1.stop();
}
//TPL5110 done
void doneSignal() {
  printAndLog("Send done signal to TPL5110");
  digitalWrite(done_pin, HIGH);
  delay(1000);
  digitalWrite(done_pin, LOW);
}

//Function to extract integers for strings of form "hh:mm:ss"
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
  printAndLog("Alarm1");
  playFile(makeFileName(timeConstruct(ALARM_1)[0], sampleNumber)); //make file name from alarm hour and sample number.
  delay(WAIT_AFTER_PLAY_MS);
  /* Got rid of this block, perhaps could put it back in wth a try block. But fault checker should do the same thing..  
  else {
    printAndLog("sampleNumber did not register. Default = 1");
    playFile("18TKP1.WAV");
  }
  */
}
// Audio file 2
void startPlayingAlarm2() {
  printAndLog("Alarm2");
  playFile(makeFileName(timeConstruct(ALARM_2)[0], sampleNumber));
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 3
void startPlayingAlarm3() {
  printAndLog("Alarm3");
  playFile(makeFileName(timeConstruct(ALARM_3)[0], sampleNumber));
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 4
void startPlayingAlarm4() {
  printAndLog("Alarm4");
  playFile(makeFileName(timeConstruct(ALARM_4)[0], sampleNumber));
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 5
void startPlayingAlarm5() {
  printAndLog("Alarm5");
  playFile(makeFileName(timeConstruct(ALARM_5)[0], sampleNumber));
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 6
void startPlayingAlarm6() {
  printAndLog("Alarm6");
  playFile(makeFileName(timeConstruct(ALARM_6)[0], sampleNumber));
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 7
void startPlayingAlarm7() {
  printAndLog("Alarm7");
  playFile(makeFileName(timeConstruct(ALARM_7)[0], sampleNumber));
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 8
void startPlayingAlarm8() {
  printAndLog("Alarm8");
  playFile(makeFileName(timeConstruct(ALARM_8)[0], sampleNumber));
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 9
void startPlayingAlarm9() {
  printAndLog("Alarm9");
  playFile(makeFileName(timeConstruct(ALARM_9)[0], sampleNumber));
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 10
void startPlayingAlarm10() {
  printAndLog("Alarm10");
  playFile(makeFileName(timeConstruct(ALARM_10)[0], sampleNumber));
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 11
void startPlayingAlarm11() {
  printAndLog("Alarm11");
  playFile(makeFileName(timeConstruct(ALARM_11)[0], sampleNumber));
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 12
void startPlayingAlarm12() {
  printAndLog("Alarm12");
  playFile(makeFileName(timeConstruct(ALARM_12)[0], sampleNumber));
  delay(WAIT_AFTER_PLAY_MS);
}

// Turn off system
// due to limit on number of alarms we can make, I am not using this alarm. 
// instead, using the fault checker to do that. 
void stopPlayingAlarm() {
  printAndLog("Alarm: turn off system");
  digitalWrite(mos_pwr, LOW);
  digitalWrite(mos_audio, LOW);
  stopFile();
  doneSignal();
}

//TIME FUNCTIONS AND MODE DETERMINATION
//Convert hours minutes and seconds to seconds after midnight
int time2sec (int h, int m, int s) {
  int timeSec = s + m * 60 + h * 3600;
  return timeSec;
}
//function to determine whether system should be on or off upon wakeup
bool mode_on() {
  
  // convert wake time and stop time, now() into seconds after midnight
  int nowSeconds = time2sec(hour(), minute(), second());
  int startSeconds = time2sec(startH, startM, startS);
  int stopSeconds = time2sec(stopH, stopM, stopS);
  bool rtrn = 0;
  
  if (startSeconds < stopSeconds) {
    //Serial.println("Play interval does not include midnight");
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

  return rtrn;
}

//function to determine whether system should be on or off upon wakeup
bool mode_play() {
  
  // convert wake time and stop time, now() into seconds after midnight
  int nowSeconds = time2sec(hour(), minute(), second());
  int startSeconds = time2sec(timeConstruct(ALARM_1)[0], timeConstruct(ALARM_1)[1], timeConstruct(ALARM_1)[2]);
  int stopSeconds = time2sec(stopH, stopM, stopS);

  bool rtrn = 0;
  
  if (startSeconds < stopSeconds) {
    //Serial.println("Play interval does not include midnight");
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

  return rtrn;
}

// FAULT CHECK
void fault_check(){
  bool mode_on_result = mode_on();  
  if (mode_on_result == 0){
    printAndLog("Fault check: Shutdown system.");
    playWav1.stop();
    digitalWrite(mos_pwr, LOW);
    digitalWrite(mos_audio, LOW);
    digitalWrite(done_pin, HIGH);
    delay(1000);
    digitalWrite(done_pin, LOW);
    }
    
  bool mode_play_result = mode_play();
  if (mode_play_result == 1 && playWav1.isPlaying() == false){ 
  printAndLog("Fault check: play default hour track");
  }
  else { //Serial.println("No, I'm stuck here"); // Do nothing, system is on but waiting to play. No issue.  
  }
  //Serial.println("I promise, it's here that I am stuck");
}

void setup()  {
  // set the Time library to use Teensy 3.0's RTC to keep time
  setSyncProvider(getTeensy3Time);
  
  // Set up serial for debugging
  Serial.begin(BAUDE_RATE);

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
  
  // Determine if teensy should be on
  bool state = mode_on();

  // If state is false, send digital high to done pin (go to sleep)
  if (state == false) {
    printAndLog("Sleep.");
    digitalWrite(mos_pwr, LOW);
    digitalWrite(mos_audio, LOW);
    digitalWrite(done_pin, HIGH);
    delay(1000);
    digitalWrite(done_pin, LOW);
  }

  // If state is true, start playing file and proceed to main loop
  if (state == true) {
    printAndLog("Wake up system");
    digitalWrite(mos_pwr, HIGH);
    digitalWrite(mos_audio, HIGH);

    // WAV Player Setup
    AudioMemory(8);
    sgtl5000_1.enable();
    sgtl5000_1.volume(0.75);

    //update the value of sampleNumber to be a random value between 1 and 4 (inclusive)
    Entropy.Initialize();
    sampleNumber = Entropy.random(1,4);

    //FOR TESTING
    sampleNumber = 3;

    if (sampleNumber <= 4 ){
      printAndLog(customAdd("sampleNumber =", sampleNumber));
      }
    else{
      printAndLog("sampleNumber out of range");
      }    

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
  }
}

void loop() {
  digitalClockDisplay();
  fault_check(); //If wavfile isn't playing, force on based on time
  Alarm.delay(1000); // wait one second between clock display
}
