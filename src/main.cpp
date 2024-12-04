#include <SD.h>
#include <SDConfig.h>

/*
 * Example use of the SDConfigFile library.
 * This sketch reads the configuration file from the SD card,
 * then prints the configuration and prints a hello message
 * at a rate given in the configuration file.
 *
 * This example app is placed in the public domain by its author,
 * Bradford Needham (@bneedhamia, https://www.needhamia.com )
 */
 
/*
 * Hardware: An Arduino Uno
 * plus an SD card shield, for example the
 * Sparkfun MicroSD Shield:
 *   https://www.sparkfun.com/products/12761
 *
 * Make sure that pinSelectSD (below) is correct
 * for the SD card you're using.
 */

/*
 * To operate:
 * 1) format your SD card.
 * 2) copy the examples/SDConfigFileExample/example.cfg file
 *    to the SD card.
 * 3) Download and run this Sketch
 * 4) Open the Serial Monitor at 9600 baud.
 * See that the Serial Monitor shows the settings
 * and the greeting, and the hello message is printed
 * with the timing given in the configuration file.
 *
 * Change the example.cfg file contents and see how the
 * hello message and timing change - without downloading
 * a new version of this sketch.
 */

const int pinSelectSD = 10; // SD shield Chip Select pin.

// The filename of the configuration file on the SD card
const char CONFIG_FILE[] = "example.cfg";

/*
 * Settings we read from the configuration file.
 *   didReadConfig = true if the configuration-reading succeeded;
 *     false otherwise.
 *     Used to prevent odd behaviors if the configuration file
 *     is corrupt or missing.
 *   hello = the "hello world" string, allocated via malloc().
 *   doDelay = if true, delay waitMs in loop().
 *     if false, don't delay.
 *   waitMs = time (milliseconds) to wait after printing hello.
 */
boolean didReadConfig;
char *hello;
boolean doDelay;
int waitMs;

// my variables
char *ALARM_1;
char *ALARM_2;
char *ALARM_3;
char *ALARM_4;
char *ALARM_5;
char *ALARM_6;
char *ALARM_7;
char *ALARM_8;
char *ALARM_9;
char *ALARM_10;
char *ALARM_11;
char *ALARM_12;

char *SAMPLE_LOCATION;
int BAUDE_RATE;

// Wake Time
int startH;
int startM;
int startS;

// Play Time (first alarm) [18:0:0 for real]
int playH;
int playM;
int playS;

// Sleep Time
int stopH;
int stopM;
int stopS;

boolean readConfiguration();

void setup() {
  Serial.begin(9600);
  while(Serial.available()==0) {};

  pinMode(pinSelectSD, OUTPUT);

  didReadConfig = false;
  hello = 0;
  doDelay = false;
  waitMs = 0;

  // my variables
  ALARM_1 = 0;
  ALARM_2 = 0;
  ALARM_3 = 0;
  ALARM_4 = 0;
  ALARM_5 = 0;
  ALARM_6 = 0;
  ALARM_7 = 0;
  ALARM_8 = 0;
  ALARM_9 = 0;
  ALARM_10 = 0;
  ALARM_11 = 0;
  ALARM_12 = 0;
  SAMPLE_LOCATION = 0;
  BAUDE_RATE = 0;
  startH = 0;
  startM = 0;
  startS = 0;
  playH = 0;
  playM = 0;
  playS = 0;
  stopH = 0;
  stopM = 0;
  stopS = 0;
  
  
  // Setup the SD card 
  Serial.println("Calling SD.begin()...");
  if (!SD.begin(pinSelectSD)) {
    Serial.println("SD.begin() failed. Check: ");
    Serial.println("  card insertion,");
    Serial.println("  SD shield I/O pins and chip select,");
    Serial.println("  card formatting.");
    return;
  }
  Serial.println("...succeeded.");

  // Read our configuration from the SD card file.
  didReadConfig = readConfiguration();
}

void loop() {

  /*
   * If we didn't read the configuration, do nothing.
   */
   
  if (!didReadConfig) {
    return;
  }

  /*
   * print the hello message,
   * then wait the configured time.
   */
  Serial.print("hello: ");
  Serial.println(hello);

  if (doDelay) {
    delay(waitMs);
  }
}

/*
 * Read our settings from our SD configuration file.
 * Returns true if successful, false if it failed.
 */
boolean readConfiguration() {
  /*
   * Length of the longest line expected in the config file.
   * The larger this number, the more memory is used
   * to read the file.
   * You probably won't need to change this number.
   */
  const uint8_t CONFIG_LINE_LENGTH = 127;
  
  // The open configuration file.
  SDConfig cfg;
  
  // Open the configuration file.
  if (!cfg.begin(CONFIG_FILE, CONFIG_LINE_LENGTH)) {
    Serial.print("Failed to open configuration file: ");
    Serial.println(CONFIG_FILE);
    return false;
  }
  
  // Read each setting from the file.
  while (cfg.readNextSetting()) {
    
    // Put a nameIs() block here for each setting you have.
    
    // OG Settings
    if (cfg.nameIs("doDelay")) {
      doDelay = cfg.getBooleanValue();
    }
    // waitMs integer
    else if (cfg.nameIs("waitMs")) {
      waitMs = cfg.getIntValue();
    }
    // hello string (char *)
    else if (cfg.nameIs("hello")) {
      hello = cfg.copyValue();
    }

    // Alarm times
    else if (cfg.nameIs("ALARM_1")) {     
      ALARM_1 = cfg.copyValue();
      Serial.print("ALARM_1: ");
      Serial.println(ALARM_1);      
    }
    
    else if (cfg.nameIs("ALARM_2")) {     
      ALARM_2 = cfg.copyValue();
      Serial.print("ALARM_2: ");
      Serial.println(ALARM_2);
    }    

    else if (cfg.nameIs("ALARM_3")) {     
      ALARM_3 = cfg.copyValue();
      Serial.print("ALARM_3: ");
      Serial.println(ALARM_3);      
    }

    else if (cfg.nameIs("ALARM_4")) {     
      ALARM_4 = cfg.copyValue();
      Serial.print("ALARM_4: ");
      Serial.println(ALARM_4);
    }    

    else if (cfg.nameIs("ALARM_5")) {     
      ALARM_5 = cfg.copyValue();
      Serial.print("ALARM_5: ");
      Serial.println(ALARM_5);
    }
  
      else if (cfg.nameIs("ALARM_6")) {     
      ALARM_6 = cfg.copyValue();
      Serial.print("ALARM_6: ");
      Serial.println(ALARM_6);
    }    

    else if (cfg.nameIs("ALARM_7")) {     
      ALARM_7 = cfg.copyValue();
      Serial.print("ALARM_7: ");
      Serial.println(ALARM_7);
    }

    else if (cfg.nameIs("ALARM_8")) {     
      ALARM_8 = cfg.copyValue();
      Serial.print("ALARM_8: ");
      Serial.println(ALARM_8);
    }    

    else if (cfg.nameIs("ALARM_9")) {     
      ALARM_9 = cfg.copyValue();
      Serial.print("ALARM_9: ");
      Serial.println(ALARM_9);
    }

    else if (cfg.nameIs("ALARM_10")) {     
      ALARM_10 = cfg.copyValue();
      Serial.print("ALARM_10: ");
      Serial.println(ALARM_10);
    }

    else if (cfg.nameIs("ALARM_11")) {     
      ALARM_11 = cfg.copyValue();
      Serial.print("ALARM_11: ");
      Serial.println(ALARM_11);
    }    

    else if (cfg.nameIs("ALARM_12")) {     
      ALARM_12 = cfg.copyValue();
      Serial.print("ALARM_12: ");
      Serial.println(ALARM_12);
    }

    // Sample location
    else if (cfg.nameIs("SAMPLE_LOCATION")) {     
      SAMPLE_LOCATION = cfg.copyValue();
      Serial.print("SAMPLE_LOCATION: ");
      Serial.println(SAMPLE_LOCATION);
    }

    // Baude Rate
    else if (cfg.nameIs("BAUDE_RATE")) { 
      BAUDE_RATE = cfg.getIntValue();
      Serial.print("BAUDE_RATE: ");
      Serial.println(BAUDE_RATE);
    }
    
    // Wake time
    else if (cfg.nameIs("startH")) { 
      startH = cfg.getIntValue();
      Serial.print("startH: ");
      Serial.println(startH);
    }

    else if (cfg.nameIs("startM")) { 
      startM = cfg.getIntValue();
      Serial.print("startM: ");
      Serial.println(startM);
    }

    else if (cfg.nameIs("startS")) { 
      startS = cfg.getIntValue();
      Serial.print("startS: ");
      Serial.println(startS);
    }    

    // Play time
    else if (cfg.nameIs("playH")) { 
      playH = cfg.getIntValue();
      Serial.print("playH: ");
      Serial.println(playH);
    }

    else if (cfg.nameIs("playM")) { 
      playM = cfg.getIntValue();
      Serial.print("playM: ");
      Serial.println(playM);
    }

    else if (cfg.nameIs("playS")) { 
      playS = cfg.getIntValue();
      Serial.print("playS: ");
      Serial.println(playS);
    }  

    // Stop time
    else if (cfg.nameIs("stopH")) { 
      stopH = cfg.getIntValue();
      Serial.print("stopH: ");
      Serial.println(stopH);
    }

    else if (cfg.nameIs("stopM")) { 
      stopM = cfg.getIntValue();
      Serial.print("stopM: ");
      Serial.println(stopM);
    }

    else if (cfg.nameIs("stopS")) { 
      stopS = cfg.getIntValue();
      Serial.print("stopS: ");
      Serial.println(stopS);
    }

    else {
      // report unrecognized names.
      Serial.print("Unknown name in config: ");
      Serial.println(cfg.getName());
    }
  }
  
  // clean up
  cfg.end();
  
  return true;
}