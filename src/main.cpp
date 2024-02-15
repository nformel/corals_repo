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