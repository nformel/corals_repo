//This code plays the loaded wav files on a schedule AND
//will tell the TPL510 that it's done when Sound Off is trigered
#define SOFTTWARE_VERSION "v2.2"

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
int startH = 16;
int startM = 0;
int startS = 0;

// Play Time (first alarm) [18:0:0 for real]
int playH = 15;
int playM = 0;
int playS = 0;

// Sleep Time
int stopH = 6;
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
  delay(1000); // wait one second between clock display
}
