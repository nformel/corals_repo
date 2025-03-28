//This code plays the loaded wav files on a schedule AND
//will tell the TPL510 that it's done when Sound Off is trigered
#define SOFTTWARE_VERSION "v2.1"

//LIBRARIES
#include <Arduino.h>
#include <TimeLib.h>
#include <string>
#include <array>
using namespace std;
#define SEC_PRE_MIDNIGHT 86399
#define MIDNIGHT_IN_SEC 86400
#define BAUDE_RATE = 115200

// Wake Time
int startH = 8;
int startM = 0;
int startS = 0;

// Play Time (first alarm) [18:0:0 for real]
int playH = 8;
int playM = 30;
int playS = 0;

// Sleep Time
int stopH = 22;
int stopM = 0;
int stopS = 0;

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


// construct time string of from "hh:mm:ss" from integer for hours, minutes and seconds (invers of timeConstruct)
const char *  make_time(int H, int M, int S){
  Serial.println("attempting to make a time");
  Serial.print("H = ");
  Serial.println(H);
  Serial.print("M = ");
  Serial.println(M);
  Serial.print("S = ");
  Serial.println(S);

  std::string hrString = "null";
  std::string minString = "null";
  std::string secString = "null";

  // need to ad zero padding for single digit integers
  //Serial.println("evaluating H");
  if(H<10){
    hrString = "0" + std::to_string(H);
  } else{hrString = std::to_string(H);}
  //Serial.println("evaluating M");
  if(M<10){
    minString = "0" + std::to_string(M);
  } else{minString = std::to_string(M);}
  //Serial.println("evaluating S");
  if(S<10){
    secString = "0" + std::to_string(S);
  } else{secString = std::to_string(S);}

  Serial.print("hrString = ");
  Serial.println(hrString.c_str());
  Serial.print("minString = ");
  Serial.println(minString.c_str());
  Serial.print("S = ");
  Serial.println(secString.c_str());

  Serial.println("Concatenate");
  std::string concat = hrString + ":" + minString + ":" + secString;
  const char * result = concat.c_str();
  Serial.println("print concat.c_str()");
  Serial.println(concat.c_str());
  Serial.println("make_time result: ");
  Serial.println(result);
  return result;
}


//Convert hours minutes and seconds to seconds after midnight
int time2sec (int h, int m, int s) {
  int timeSec = s + m * 60 + h * 3600;
  return timeSec;
}

//Function to determine if the present time is between two values
bool time_between(std::string startTime, std::string stopTime) {

  Serial.println("running time_between function");
  delay(100);

  Serial.print("startTime: ");
  String startTimeString = startTime.c_str();
  Serial.println(startTimeString);
  delay(1000);

  String stopTimeString = stopTime.c_str();
  Serial.print("stopTime: ");
  Serial.println(stopTimeString);
  delay(1000);

  return 1;
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

  Serial.println("");
  Serial.print("Software Version: ");
  Serial.println(SOFTTWARE_VERSION);

  //check digital clock once in setup
  Serial.println("Current time: ");
  digitalClockDisplay();
  Serial.println();

  Serial.println("load wake time");
  const char * wake_time = "08:00:00"; //make_time(startH, startM, startS);
  Serial.println("wake_time is: ");
  Serial.println(wake_time);

  Serial.println("load sleep time");
  const char * sleep_time = "23:00:00"; //make_time(stopH, stopM, stopS);
  Serial.println("sleep_time is: ");
  Serial.println(sleep_time);

  Serial.println("double check that wake time hasn't been overwritten: ");
  Serial.println("wake_time is: ");
  Serial.println(wake_time);

  Serial.println("mode_on result is: ");
  bool mode_on = time_between(wake_time, sleep_time);
  Serial.println(mode_on);

}

void loop() {
  digitalClockDisplay(); //serial print the time according to RTC
  Serial.println();
  delay(1000); // wait one second between clock display
}
