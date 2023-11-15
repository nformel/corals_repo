//This code plays the loaded wav files on a schedule AND
//will tell the TPL510 that it's done when Sound Off is trigered
//final version as of 10/18/22 4:47pm

//LIBRARIES
#include <TimeLib.h>
#include <TimeAlarms.h>
//Wav Player Setup
#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>
#include <Entropy.h>
AudioPlaySdWav           playWav1;
AudioOutputI2S           audioOutput;
AudioConnection          patchCord1(playWav1, 0, audioOutput, 0);
AudioConnection          patchCord2(playWav1, 1, audioOutput, 1);
AudioControlSGTL5000     sgtl5000_1;
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

// USER DEFINED GLOBAL VARIABLES
// Wake Time
int startH = 17;
int startM = 50;
int startS = 0;

// Play Time (first alarm) [18:0:0 for real]
int playH = 18;
int playM = 0;
int playS = 0;

// Sleep Time
int stopH = 6;
int stopM = 0;
int stopS = 0;

// digital pin declarations
int done_pin = 17;
int mos_pwr = 3;
int mos_audio = 2;

// declare the sample number as global variable
int sampleNumber;

// SD Logging file instantiation
File myFile;

void setup()  {
  // set the Time library to use Teensy 3.0's RTC to keep time
  setSyncProvider(getTeensy3Time);
  
  // Set up serial for debugging
  Serial.begin(115200);

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
    if (sampleNumber == 1 ){
      printAndLog("sampleNumber = 1");
      }
    else if (sampleNumber == 2){
      printAndLog("sampleNumber = 2");
      }
    else if (sampleNumber == 3){
      printAndLog("sampleNumber = 3");
      }
    else if (sampleNumber == 4){
      printAndLog("sampleNumber = 4");
      }
    else {printAndLog("sampleNumber is out of range.");
    }

    //Set up alarms
    // First alarm for play
    Alarm.alarmRepeat(playH, playM, playS, startPlayingAlarm1); //start playing first file
    // All other alarms
    Alarm.alarmRepeat(19, 0, 0, startPlayingAlarm2); 
    Alarm.alarmRepeat(20, 0, 0, startPlayingAlarm3); 
    Alarm.alarmRepeat(21, 0, 0, startPlayingAlarm4); //start playing second file
    Alarm.alarmRepeat(22, 0, 0, startPlayingAlarm5); //start playing second file
    Alarm.alarmRepeat(23, 0, 0, startPlayingAlarm6); //start playing second file
    Alarm.alarmRepeat(0, 0, 0, startPlayingAlarm7); //start playing second file
    Alarm.alarmRepeat(1, 0, 0, startPlayingAlarm8); //start playing second file
    Alarm.alarmRepeat(2, 0, 0, startPlayingAlarm9); //start playing second file
    Alarm.alarmRepeat(3, 0, 0, startPlayingAlarm10); //start playing second file
    Alarm.alarmRepeat(4, 0, 0, startPlayingAlarm11); //start playing second file
    Alarm.alarmRepeat(5, 0, 0, startPlayingAlarm12); //start playing second file
  }
}

void loop() {
  digitalClockDisplay();
  fault_check(); //If wavfile isn't playing, force on based on time
  Alarm.delay(1000); // wait one second between clock display
}

/////////////
//FUNCTIONS//
/////////////


// ALARM FUNCTIONS
// Audio file 1
void startPlayingAlarm1() {
  printAndLog("Alarm1");
  if (sampleNumber == 1){
    playFile("18TKP1.WAV");  // filenames are always uppercase 8.3 format
    delay(250);
    }
  else if (sampleNumber == 2){
    playFile("18TKP2.WAV");
    delay(250);
    } 
  else if (sampleNumber == 3){
    playFile("18TKP3.WAV");
    delay(250);
    }
  else if (sampleNumber == 4){
    playFile("18TKP4.WAV");
    delay(250);
    }
  else {
    printAndLog("sampleNumber did not register. Default = 1");
    playFile("18TKP1.WAV");
    delay(250);
  }
}
// Audio file 2
void startPlayingAlarm2() {
  printAndLog("Alarm2");
  if (sampleNumber == 1){
    playFile("19TKP1.WAV");
    delay(250);
    }
  else if (sampleNumber == 2){
    playFile("19TKP2.WAV");
    delay(250);
    } 
  else if (sampleNumber == 3){
    playFile("19TKP3.WAV");
    delay(250);
    }
  else if (sampleNumber == 4){
    playFile("19TKP4.WAV");
    delay(250);
    }
  else {
    printAndLog("sampleNumber did not register. Default = 1");
    playFile("19TKP1.WAV");
    delay(250);
  }
}
// Audio file 3
void startPlayingAlarm3() {
  printAndLog("Alarm3");
  if (sampleNumber == 1){
    playFile("20TKP1.WAV");
    delay(250);
    }
  else if (sampleNumber == 2){
    playFile("20TKP2.WAV");
    delay(250);
    } 
  else if (sampleNumber == 3){
    playFile("20TKP3.WAV");
    delay(250);
    }
  else if (sampleNumber == 4){
    playFile("20TKP4.WAV");
    delay(250);
    }
  else {
    printAndLog("sampleNumber did not register. Default = 1");
    playFile("20TKP1.WAV");
    delay(250);
  }
}
// Audio file 4
void startPlayingAlarm4() {
  printAndLog("Alarm4");
  if (sampleNumber == 1){
    playFile("21TKP1.WAV");
    delay(250);
    }
  else if (sampleNumber == 2){
    playFile("21TKP2.WAV");
    delay(250);
    } 
  else if (sampleNumber == 3){
    playFile("21TKP3.WAV");
    delay(250);
    }
  else if (sampleNumber == 4){
    playFile("21TKP4.WAV");
    delay(250);
    }
  else {
    printAndLog("sampleNumber did not register. Default = 1");
    playFile("20TKP1.WAV");
    delay(250);
  }
}
// Audio file 5
void startPlayingAlarm5() {
  printAndLog("Alarm5");
  if (sampleNumber == 1){
    playFile("22TKP1.WAV");
    delay(250);
    }
  else if (sampleNumber == 2){
    playFile("22TKP2.WAV");
    delay(250);
    } 
  else if (sampleNumber == 3){
    playFile("22TKP3.WAV");
    delay(250);
    }
  else if (sampleNumber == 4){
    playFile("22TKP4.WAV");
    delay(250);
    }
  else {
    printAndLog("sampleNumber did not register. Default = 1");
    playFile("22TKP1.WAV");
    delay(250);
  }
}
// Audio file 6
void startPlayingAlarm6() {
  printAndLog("Alarm6");
  if (sampleNumber == 1){
    playFile("23TKP1.WAV");
    delay(250);
    }
  else if (sampleNumber == 2){
    playFile("23TKP2.WAV");
    delay(250);
    } 
  else if (sampleNumber == 3){
    playFile("23TKP3.WAV");
    delay(250);
    }
  else if (sampleNumber == 4){
    playFile("23TKP4.WAV");
    delay(250);
    }
  else {
    printAndLog("sampleNumber did not register. Default = 1");
    playFile("23TKP1.WAV");
    delay(250);
  }
}
// Audio file 7
void startPlayingAlarm7() {
  printAndLog("Alarm7");
  if (sampleNumber == 1){
    playFile("00TKP1.WAV");
    delay(250);
    }
  else if (sampleNumber == 2){
    playFile("00TKP2.WAV");
    delay(250);
    } 
  else if (sampleNumber == 3){
    playFile("00TKP3.WAV");
    delay(250);
    }
  else if (sampleNumber == 4){
    playFile("00TKP4.WAV");
    delay(250);
    }
  else {
    printAndLog("sampleNumber did not register. Default = 1");
    playFile("00TKP1.WAV");
    delay(250);
  }
}
// Audio file 8
void startPlayingAlarm8() {
  printAndLog("Alarm8");
  if (sampleNumber == 1){
    playFile("01TKP1.WAV");
    delay(250);
    }
  else if (sampleNumber == 2){
    playFile("01TKP2.WAV");
    delay(250);
    } 
  else if (sampleNumber == 3){
    playFile("01TKP3.WAV");
    delay(250);
    }
  else if (sampleNumber == 4){
    playFile("01TKP4.WAV");
    delay(250);
    }
  else {
    printAndLog("sampleNumber did not register. Default = 1");
    playFile("01TKP1.WAV");
    delay(250);
  }
}
// Audio file 9
void startPlayingAlarm9() {
  printAndLog("Alarm9");
  if (sampleNumber == 1){
    playFile("02TKP1.WAV");
    delay(250);
    }
  else if (sampleNumber == 2){
    playFile("02TKP2.WAV");
    delay(250);
    } 
  else if (sampleNumber == 3){
    playFile("02TKP3.WAV");
    delay(250);
    }
  else if (sampleNumber == 4){
    playFile("02TKP4.WAV");
    delay(250);
    }
  else {
    printAndLog("sampleNumber did not register. Default = 1");
    playFile("02TKP1.WAV");
    delay(250);
  }
}
// Audio file 10
void startPlayingAlarm10() {
  printAndLog("Alarm10");
  if (sampleNumber == 1){
    playFile("03TKP1.WAV");
    delay(250);
    }
  else if (sampleNumber == 2){
    playFile("03TKP2.WAV");
    delay(250);
    } 
  else if (sampleNumber == 3){
    playFile("03TKP3.WAV");
    delay(250);
    }
  else if (sampleNumber == 4){
    playFile("03TKP4.WAV");
    delay(250);
    }
  else {
    printAndLog("sampleNumber did not register. Default = 1");
    playFile("03TKP1.WAV");
    delay(250);
  }
}
// Audio file 11
void startPlayingAlarm11() {
  printAndLog("Alarm11");
  if (sampleNumber == 1){
    playFile("04TKP1.WAV");
    delay(250);
    }
  else if (sampleNumber == 2){
    playFile("04TKP2.WAV");
    delay(250);
    } 
  else if (sampleNumber == 3){
    playFile("04TKP3.WAV");
    delay(250);
    }
  else if (sampleNumber == 4){
    playFile("04TKP4.WAV");
    delay(250);
    }
  else {
    printAndLog("sampleNumber did not register. Default = 1");
    playFile("04TKP1.WAV");
    delay(250);
  }
}
// Audio file 12
void startPlayingAlarm12() {
  printAndLog("Alarm12");
  if (sampleNumber == 1){
    playFile("05TKP1.WAV");
    delay(250);
    }
  else if (sampleNumber == 2){
    playFile("05TKP2.WAV");
    delay(250);
    } 
  else if (sampleNumber == 3){
    playFile("05TKP3.WAV");
    delay(250);
    }
  else if (sampleNumber == 4){
    playFile("05TKP4.WAV");
    delay(250);
    }
  else {
    printAndLog("sampleNumber did not register. Default = 1");
    playFile("05TKP1.WAV");
    delay(250);
  }
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

// WAV FILE PLAYER AND TPL5110 HELPER FUNCTIONS
// playFile function from WAV file player
void playFile(const char *filename) {
  printAndLog("Playing file:");
  printAndLog(filename);
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

  /*
  Serial.print("Start Seconds: ");
  Serial.println(startSeconds);
  Serial.print("Now Seconds: ");
  Serial.println(nowSeconds);
  Serial.print("Stop Seconds: ");
  Serial.println(stopSeconds);
  */
  
  if (startSeconds < stopSeconds) {
    //Serial.println("Play interval does not include midnight");
    if (0 <= nowSeconds && nowSeconds < startSeconds) {
      //Serial.println("Case 1");
      return 0;
    }
    else if (startSeconds <= nowSeconds && nowSeconds < stopSeconds) {
      //Serial.println("Case 2");
      return 1;
    }
    else if (stopSeconds <= nowSeconds && nowSeconds < 86399) {
      //Serial.println("Case 3");
      return 0;
    }
  }
  else if (startSeconds > stopSeconds) {
    //Serial.println("Play interval includes midnight");
    if (0 <= nowSeconds && nowSeconds < stopSeconds) {
      //Serial.println("Case 1");
      return 1;
    }
    else if (stopSeconds <= nowSeconds && nowSeconds < startSeconds) {
      //Serial.println("Case 2");
      return 0;
    }
    else if (startSeconds <= nowSeconds && nowSeconds <= 86400) {
      //Serial.println("Case 3");
      return 1;
    }
  }
}

//function to determine whether system should be on or off upon wakeup
bool mode_play() {
  
  // convert wake time and stop time, now() into seconds after midnight
  int nowSeconds = time2sec(hour(), minute(), second());
  int startSeconds = time2sec(playH, playM, playS);
  int stopSeconds = time2sec(stopH, stopM, stopS);
  
  if (startSeconds < stopSeconds) {
    //Serial.println("Play interval does not include midnight");
    if (0 <= nowSeconds && nowSeconds < startSeconds) {
      //Serial.println("Case 1");
      return 0;
    }
    else if (startSeconds <= nowSeconds && nowSeconds < stopSeconds) {
      //Serial.println("Case 2");
      return 1;
    }
    else if (stopSeconds <= nowSeconds && nowSeconds < 86399) {
      //Serial.println("Case 3");
      return 0;
    }
  }
  else if (startSeconds > stopSeconds) {
    //Serial.println("Play interval includes midnight");
    if (0 <= nowSeconds && nowSeconds < stopSeconds) {
      //Serial.println("Case 1");
      return 1;
    }
    else if (stopSeconds <= nowSeconds && nowSeconds < startSeconds) {
      //Serial.println("Case 2");
      return 0;
    }
    else if (startSeconds <= nowSeconds && nowSeconds <= 86400) {
      //Serial.println("Case 3");
      return 1;
    }
  }
}
//Digital Clock Code
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
void printDigits(int digits) {
  // utility function for digital clock display on Serial monitor: prints preceding colon and leading 0
  Serial.print(":");
  if (digits < 10)
    Serial.print('0');
  Serial.print(digits);
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
void logSD(char string[]) {
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
void printAndLog(char string[]){
  Serial.println(string);
  logSD(string);
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
  
    if (hour() == 18){
      playFile("18TKP1.WAV");  // filenames are always uppercase 8.3 format 
      delay(250); // wait for library to parse WAV info
    }
    else if (hour() == 19){
      playFile("19TKP1.WAV");
      delay(250);
    }
    else if (hour() == 20){
      playFile("20TKP1.WAV");
      delay(250);
    }
    else if (hour() == 21){
      playFile("21TKP1.WAV");
      delay(250);
    }
    else if (hour() == 22){
      playFile("22TKP1.WAV");
      delay(250);
    }
    else if (hour() == 23){
      playFile("23TKP1.WAV");
      delay(250);
    }
    else if (hour() == 0){
      playFile("00TKP1.WAV");
      delay(250);
    }
    else if (hour() == 1){
      playFile("01TKP1.WAV");
      delay(250);
    }
    else if (hour() == 2){
      playFile("02TKP1.WAV");
      delay(250);
    }
    else if (hour() == 3){
      playFile("03TKP1.WAV");
      delay(250);
    }
    else if (hour() == 4){
      playFile("04TKP1.WAV");
      delay(250);
    }
    else if (hour() == 5){
      playFile("05TKP1.WAV");
      delay(250);
    }
    else{ //Serial.println("Im stuck here");
    }
  }
  else { //Serial.println("No, I'm stuck here"); // Do nothing, system is on but waiting to play. No issue.  
  }
  //Serial.println("I promise, it's here that I am stuck");
}
