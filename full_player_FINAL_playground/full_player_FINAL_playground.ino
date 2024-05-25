//Tryng to combine Playing_WAV_Pause and bluart_mvp

// Simple WAV file player example
// Play entire file or pause after some number of seconds

//060722 adding some lines to test done pin functionality

#include <Audio.h>
#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <SerialFlash.h>


AudioPlaySdWav           playWav1;
AudioOutputI2S           audioOutput;
AudioConnection          patchCord1(playWav1, 0, audioOutput, 0);
AudioConnection          patchCord2(playWav1, 1, audioOutput, 1);
AudioControlSGTL5000     sgtl5000_1;

// Use these with the Teensy Audio Shield
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14

//TPL5110 Pins and MOSFETS
int done_pin = 17;
int mos_pwr = 3;
int mos_audio = 2;

//Bluefruit Setup
#include <Arduino.h>
#include "Adafruit_BLE.h"
#include "Adafruit_BluefruitLE_UART.h"

#define FACTORYRESET_ENABLE         1
#define MINIMUM_FIRMWARE_VERSION    "0.6.6"
#define MODE_LED_BEHAVIOUR          "MODE"
#define BLUEFRUIT_UART_MODE_PIN 5

//Hardware Serial Object
Adafruit_BluefruitLE_UART ble(Serial3, BLUEFRUIT_UART_MODE_PIN);

// A small helper
void error(const __FlashStringHelper*err) {
  Serial.println(err);
  while (1);
}

void setup() {
  Serial.begin(9600);
  
  pinMode(done_pin, OUTPUT);
  pinMode(mos_audio, OUTPUT);
  pinMode(mos_pwr, OUTPUT);
  pinMode(13, OUTPUT); // LED on pin 13

  // Audio connections require memory to work.  For more
  // detailed information, see the MemoryAndCpuUsage example
  AudioMemory(8);

  sgtl5000_1.enable();
  sgtl5000_1.volume(0.75);

  SPI.setMOSI(SDCARD_MOSI_PIN);
  SPI.setSCK(SDCARD_SCK_PIN);
  if (!(SD.begin(SDCARD_CS_PIN))) {
    // stop here, but print a message repetitively
    while (1) {
      Serial.println("Unable to access the SD card");
      delay(500);
    }
  }

  // Bluefruit
  // Initialise Bluetooth
  Serial.println(F("Initialising Bluefruit LE module"));
  if ( !ble.begin() ){
    Serial.println("Couldn't find Bluefruit, make sure it's in CoMmanD mode & check wiring?");
  }
  Serial.println( F("OK!") );
/*
  if ( ! ble.factoryReset() ){ // Factory reset
    error(F("Couldn't factory reset"));
  }
  ble.echo(false);   //Disable command echo from Bluefruit
  ble.verbose(false);  //debug info is a little annoying after this point!
  while (! ble.isConnected()) { //Wait for connection 
      delay(500);
  }
  ble.sendCommandCheckOK("AT+HWModeLED=" MODE_LED_BEHAVIOUR); // LED Activity command
  ble.setMode(BLUEFRUIT_MODE_DATA); // Set module to DATA mode 
  //if I end the service before audio stuff happens, will it be okay
  
  */
}

void loop() {
  digitalWrite(mos_audio, HIGH);
  digitalWrite(mos_pwr, HIGH);
  playFile("TKAF1.wav");  // filenames are always uppercase 8.3 format
  //ble.print("AT+BLEUARTTX=");
  //ble.println("Play TKP2.WAV");
}

//Play file functions definition
void playFile(const char *filename)
{
  Serial.print("Playing file: ");
  Serial.println(filename);

  // Start playing the file.  This sketch continues to
  // run while the file plays.
  playWav1.play(filename);

  // blink while playing
  if (playWav1.isPlaying() == false) {
    digitalWrite(13, HIGH);
    delay(250);
    digitalWrite(13, LOW);
    delay(250);
    }
  
  //Play entire file
  while (playWav1.isPlaying()) {
    }
}

