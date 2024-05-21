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

//CONFIGURABLE DEFINITIONS (see config file)
#define ALARM_1 "16:38:00"
#define ALARM_2 "16:38:05"
#define ALARM_3 "16:38:10"
#define ALARM_4 "16:38:15"
#define ALARM_5 "16:38:20"
#define ALARM_6 "16:38:25"
#define ALARM_7 "16:38:30"
#define ALARM_8 "16:38:35"
#define ALARM_9 "16:38:40"
#define ALARM_10 "16:38:45"
#define ALARM_11 "16:38:50"
#define ALARM_12 "11:35:30"
#define ALARM_13 "11:35:35"
#define ALARM_14 "11:35:40"

#define FILE_BASE_1 "18TKP"
#define FILE_BASE_2 "19TKP"
#define FILE_BASE_3 "20TKP"
#define FILE_BASE_4 "21TKP"
#define FILE_BASE_5 "22TKP"
#define FILE_BASE_6 "23TKP"
#define FILE_BASE_7 "00TKP"
#define FILE_BASE_8 "01TKP"
#define FILE_BASE_9 "02TKP"
#define FILE_BASE_10 "03TKP"
#define FILE_BASE_11 "04TKP"
#define FILE_BASE_12 "05TKP"
#define FILE_BASE_13 "05TKP"
#define FILE_BASE_14 "05TKP"

#define SAMPLE_LOCATION "TKP"
#define BAUDE_RATE 115200
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
std::string makeFileNameString(std::string file_base, int samp){
    std::string concat = file_base + std::to_string(samp) + ".wav";
    //const char * result = concat.c_str(); //convert string to pointer
    return concat;  
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
  playFile(makeFileNameString(FILE_BASE_1, sampleNumber)); //make file name from alarm hour and sample number.
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 2
void startPlayingAlarm2() {
  stopFile();
  printAndLog("Alarm2");
  playFile(makeFileNameString(FILE_BASE_2, sampleNumber));
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 3
void startPlayingAlarm3() {
  stopFile();
  printAndLog("Alarm3");
  playFile(makeFileNameString(FILE_BASE_3, sampleNumber));
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 4
void startPlayingAlarm4() {
  stopFile();
  printAndLog("Alarm4");
  playFile(makeFileNameString(FILE_BASE_4, sampleNumber));
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 5
void startPlayingAlarm5() {
  stopFile();
  printAndLog("Alarm5");
  playFile(makeFileNameString(FILE_BASE_5, sampleNumber));
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 6
void startPlayingAlarm6() {
  stopFile();
  printAndLog("Alarm6");
  playFile(makeFileNameString(FILE_BASE_6, sampleNumber));
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 7
void startPlayingAlarm7() {
  stopFile();
  printAndLog("Alarm7");
  playFile(makeFileNameString(FILE_BASE_7, sampleNumber));
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 8
void startPlayingAlarm8() {
  stopFile();
  printAndLog("Alarm8");
  playFile(makeFileNameString(FILE_BASE_8, sampleNumber));
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 9
void startPlayingAlarm9() {
  stopFile();
  printAndLog("Alarm9");
  playFile(makeFileNameString(FILE_BASE_9, sampleNumber));
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 10
void startPlayingAlarm10() {
  stopFile();  
  printAndLog("Alarm10");
  playFile(makeFileNameString(FILE_BASE_10, sampleNumber));
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 11
void startPlayingAlarm11() {
  stopFile();
  printAndLog("Alarm11");
  playFile(makeFileNameString(FILE_BASE_11, sampleNumber));
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 12
void startPlayingAlarm12() {
  stopFile();
  printAndLog("Alarm12");
  playFile(makeFileNameString(FILE_BASE_12, sampleNumber));
  delay(WAIT_AFTER_PLAY_MS);
}

// Audio file 13
void startPlayingAlarm13() {
  stopFile();
  printAndLog("Alarm13");
  playFile(makeFileNameString(FILE_BASE_13, sampleNumber));
  delay(WAIT_AFTER_PLAY_MS);
}

// Audio file 14
void startPlayingAlarm14() {
  stopFile();
  printAndLog("Alarm14");
  playFile(makeFileNameString(FILE_BASE_14, sampleNumber));
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

//function to determine whether system should be playing or not upon wakeup
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
  
  //Changed this to go for 24 hrs
    if (hour() == 0){
      playFile(makeFileNameString(FILE_BASE_7, 1));
      delay(250);
    }
    else if (hour() == 1){
      playFile(makeFileNameString(FILE_BASE_8, 1));
      delay(250);
    }
    else if (hour() == 2){
      playFile(makeFileNameString(FILE_BASE_9, 1));
      delay(250);
    }
    else if (hour() == 3){
      playFile(makeFileNameString(FILE_BASE_10, 1));
      delay(250);
    }
    else if (hour() == 4){
      playFile(makeFileNameString(FILE_BASE_11, 1));
      delay(250);
    }
    else if (hour() == 5){
      playFile(makeFileNameString(FILE_BASE_12, 1));
      delay(250);
    }
    else if (hour() == 6){
      playFile(makeFileNameString(FILE_BASE_6, 1));
      delay(250);
    }
    else if (hour() == 7){
      playFile(makeFileNameString(FILE_BASE_6, 1));
      delay(250);
    }
    else if (hour() == 8){
      playFile(makeFileNameString(FILE_BASE_6, 1));
      delay(250);
    }
    else if (hour() == 9){
      playFile(makeFileNameString(FILE_BASE_6, 1));
      delay(250);
    }
    else if (hour() == 10){
      playFile(makeFileNameString(FILE_BASE_6, 1));
      delay(250);
    }
    else if (hour() == 11){
      playFile(makeFileNameString(FILE_BASE_6, 1));
      delay(250);
    }
    else if (hour() == 12){
      playFile(makeFileNameString(FILE_BASE_6, 1));
      delay(250);
    }
    else if (hour() == 13){
      playFile(makeFileNameString(FILE_BASE_6, 1));
      delay(250);
    }
    else if (hour() == 14){
      playFile(makeFileNameString(FILE_BASE_6, 1));
      delay(250);
    }
    else if (hour() == 15){
      playFile(makeFileNameString(FILE_BASE_6, 1));
      delay(250);
    }
    else if (hour() == 16){
      playFile(makeFileNameString(FILE_BASE_6, 1));
      delay(250);
    }
    else if (hour() == 17){
      playFile(makeFileNameString(FILE_BASE_6, 1));
      delay(250);
    }
    else if (hour() == 18){
      playFile(makeFileNameString(FILE_BASE_1, 1));
      delay(250);
    }
    else if (hour() == 19){
      playFile(makeFileNameString(FILE_BASE_2, 1));
      delay(250);
    }
    else if (hour() == 20){
      playFile(makeFileNameString(FILE_BASE_3, 1));
      delay(250);
    }
    else if (hour() == 21){
      playFile(makeFileNameString(FILE_BASE_4, 1));
      delay(250);
    }     
    else if (hour() == 22){
      playFile(makeFileNameString(FILE_BASE_5, 1));
      delay(250);
    }
    else if (hour() == 23){
      playFile(makeFileNameString(FILE_BASE_6, 1));
      delay(250);
    }
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
  
  //adding delay for tesing when it goes through set up so I can catch the terminal traff
  Serial.println("5s Set up delay");
  delay(5000);

  //check digital clock once in setup
  digitalClockDisplay();
  
  // Determine if teensy should be on
  bool state = mode_on();

  // Testing 24Hourbranch

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
    printAndLog("hard coding sample number to be 1");
    sampleNumber = 1;

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
    Alarm.alarmRepeat(timeConstruct(ALARM_13)[0], timeConstruct(ALARM_13)[1], timeConstruct(ALARM_13)[2], startPlayingAlarm13);
    Alarm.alarmRepeat(timeConstruct(ALARM_14)[0], timeConstruct(ALARM_14)[1], timeConstruct(ALARM_14)[2], startPlayingAlarm14);


  }
}

void loop() {
  digitalClockDisplay(); //serial print the time according to RTC
  fault_check(); //If wavfile isn't playing, force on based on time
  Alarm.delay(1000); // wait one second between clock display
}
