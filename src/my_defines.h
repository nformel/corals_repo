// # defines
#define SDCARD_CS_PIN    10
#define SDCARD_MOSI_PIN  7
#define SDCARD_SCK_PIN   14
#define WAIT_AFTER_PLAY_MS 250
#define ALARM_1 "18:00:00"
#define ALARM_2 "19:00:00"
#define ALARM_3 "20:00:00"
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
#define SEC_PRE_MIDNIGHT 86399
#define MIDNIGHT_IN_SEC 86400
#define BAUDE_RATE 115200

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