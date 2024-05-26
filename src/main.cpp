//Powers amp and does nothing

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
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14


//STATIC DEFINITIONS
#define WAIT_AFTER_PLAY_MS 250
int done_pin = 17;
int mos_pwr = 3;
int mos_audio = 2;
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
  printAndLog("Alarm1");  
  printAndLog("Power amp, play nothing.");
  digitalWrite(mos_pwr, HIGH);
  digitalWrite(mos_audio, HIGH);
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 2
void startPlayingAlarm2() {
  printAndLog("Alarm2");  
  printAndLog("Power amp, play nothing.");
  digitalWrite(mos_pwr, HIGH);
  digitalWrite(mos_audio, HIGH);
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 3
void startPlayingAlarm3() {
  printAndLog("Alarm3");  
  printAndLog("Power amp, play nothing.");
  digitalWrite(mos_pwr, HIGH);
  digitalWrite(mos_audio, HIGH);
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 4
void startPlayingAlarm4() {
  printAndLog("Alarm4");  
  printAndLog("Power amp, play nothing.");
  digitalWrite(mos_pwr, HIGH);
  digitalWrite(mos_audio, HIGH);
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 5
void startPlayingAlarm5() {
  printAndLog("Alarm5");  
  printAndLog("Power amp, play nothing.");
  digitalWrite(mos_pwr, HIGH);
  digitalWrite(mos_audio, HIGH);
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 6
void startPlayingAlarm6() {
  printAndLog("Alarm6");  
  printAndLog("Power amp, play nothing.");
  digitalWrite(mos_pwr, HIGH);
  digitalWrite(mos_audio, HIGH);
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 7
void startPlayingAlarm7() {
  printAndLog("Alarm7");  
  printAndLog("Power amp, play nothing.");
  digitalWrite(mos_pwr, HIGH);
  digitalWrite(mos_audio, HIGH);
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 8
void startPlayingAlarm8() {
  printAndLog("Alarm8");  
  printAndLog("Power amp, play nothing.");
  digitalWrite(mos_pwr, HIGH);
  digitalWrite(mos_audio, HIGH);
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 9
void startPlayingAlarm9() {
  printAndLog("Alarm9");  
  printAndLog("Power amp, play nothing.");
  digitalWrite(mos_pwr, HIGH);
  digitalWrite(mos_audio, HIGH);
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 10
void startPlayingAlarm10() {
  printAndLog("Alarm10");  
  printAndLog("Power amp, play nothing.");
  digitalWrite(mos_pwr, HIGH);
  digitalWrite(mos_audio, HIGH);
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 11
void startPlayingAlarm11() {
  printAndLog("Alarm11");  
  printAndLog("Power amp, play nothing.");
  digitalWrite(mos_pwr, HIGH);
  digitalWrite(mos_audio, HIGH);
  delay(WAIT_AFTER_PLAY_MS);
}
// Audio file 12
void startPlayingAlarm12() {
  printAndLog("Alarm12");  
  printAndLog("Power amp, play nothing.");
  digitalWrite(mos_pwr, HIGH);
  digitalWrite(mos_audio, HIGH);
  delay(WAIT_AFTER_PLAY_MS);
}

// Audio file 13
void startPlayingAlarm13() {
  printAndLog("Alarm13");  
  printAndLog("Power amp, play nothing.");
  digitalWrite(mos_pwr, HIGH);
  digitalWrite(mos_audio, HIGH);
  delay(WAIT_AFTER_PLAY_MS);
}

// Audio file 14
void startPlayingAlarm14() {
  printAndLog("Alarm14");  
  printAndLog("Power amp, play nothing.");
  digitalWrite(mos_pwr, HIGH);
  digitalWrite(mos_audio, HIGH);
  delay(WAIT_AFTER_PLAY_MS);
}

// Audio file 15
void startPlayingAlarm15() {
  printAndLog("Alarm15");  
  printAndLog("Power amp, play nothing.");
  digitalWrite(mos_pwr, HIGH);
  digitalWrite(mos_audio, HIGH);
  delay(WAIT_AFTER_PLAY_MS);
}

// Audio file 16
void startPlayingAlarm16() {
  printAndLog("Alarm16");  
  printAndLog("Power amp, play nothing.");
  digitalWrite(mos_pwr, HIGH);
  digitalWrite(mos_audio, HIGH);
  delay(WAIT_AFTER_PLAY_MS);
}

// Audio file 17
void startPlayingAlarm17() {
  printAndLog("Alarm17");  
  printAndLog("Power amp, play nothing.");
  digitalWrite(mos_pwr, HIGH);
  digitalWrite(mos_audio, HIGH);
  delay(WAIT_AFTER_PLAY_MS);
}

// Audio file 18
void startPlayingAlarm18() {
  printAndLog("Alarm18");  
  printAndLog("Power amp, play nothing.");
  digitalWrite(mos_pwr, HIGH);
  digitalWrite(mos_audio, HIGH);
  delay(WAIT_AFTER_PLAY_MS);
}

// Audio file 19
void startPlayingAlarm19() {
  printAndLog("Alarm19");  
  printAndLog("Power amp, play nothing.");
  digitalWrite(mos_pwr, HIGH);
  digitalWrite(mos_audio, HIGH);
  delay(WAIT_AFTER_PLAY_MS);
}

// Audio file 20
void startPlayingAlarm20() {
  printAndLog("Alarm20");  
  printAndLog("Power amp, play nothing.");
  digitalWrite(mos_pwr, HIGH);
  digitalWrite(mos_audio, HIGH);
  delay(WAIT_AFTER_PLAY_MS);
}

// Audio file 21
void startPlayingAlarm21() {
  printAndLog("Alarm21");  
  printAndLog("Power amp, play nothing.");
  digitalWrite(mos_pwr, HIGH);
  digitalWrite(mos_audio, HIGH);
  delay(WAIT_AFTER_PLAY_MS);
}

// Audio file 22
void startPlayingAlarm22() {
  printAndLog("Alarm22");  
  printAndLog("Power amp, play nothing.");
  digitalWrite(mos_pwr, HIGH);
  digitalWrite(mos_audio, HIGH);
  delay(WAIT_AFTER_PLAY_MS);
}

// Audio file 23
void startPlayingAlarm23() {
  printAndLog("Alarm23");  
  printAndLog("Power amp, play nothing.");
  digitalWrite(mos_pwr, HIGH);
  digitalWrite(mos_audio, HIGH);
  delay(WAIT_AFTER_PLAY_MS);
}

// Audio file 24
void startPlayingAlarm24() {
  printAndLog("Alarm24");  
  printAndLog("Power amp, play nothing.");
  digitalWrite(mos_pwr, HIGH);
  digitalWrite(mos_audio, HIGH);
  delay(WAIT_AFTER_PLAY_MS);
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
  printAndLog("Power amp, play nothing.");
  digitalWrite(mos_pwr, HIGH);
  digitalWrite(mos_audio, HIGH);
 
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
  Alarm.delay(1000); // wait one second between clock display
}
