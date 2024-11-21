#include "SdFat.h"

//SD LOGGING
SdFs sd;
FsFile file;
const uint8_t SD_CS_PIN = SS; // Set chip select for built in SD Card
#define SPI_CLOCK SD_SCK_MHZ(50) // Try max SPI clock for an SD. Reduce SPI_CLOCK if errors occur.
#define SD_CONFIG SdioConfig(FIFO_SDIO) // SD Card configuration

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600); 
  if (!sd.begin(SD_CONFIG)) {
    sd.initErrorHalt(&Serial);
  }
}

void loop() {
  // put your main code here, to run repeatedly:
    //Logging setup
    //Open file and append
    if (!file.open("Log_sd.txt", FILE_WRITE)) { //FILE_WRITE
    Serial.println(F("file.open failed"));
    return;
    }

    ///////////
    //Logging//
    ///////////
    Serial.print("log: ");
    Serial.println("Hello World!");
    file.println("Hello world!");
    file.close();
    delay(1000);

}