#include <LovyanGFX.hpp>

/*
#include <LovyanGFX.hpp>  

*/
//#include <SPIFFS.h>
//#include <FS.h>
#include <EEPROM.h>
#define VERSION "Ver1.42"
#define MODEXDEFAULT 0
// 1 ... 空間除菌有効
#include "commercial.h"
struct LGFX_Config
{
  static constexpr spi_host_device_t spi_host = VSPI_HOST;
  static constexpr int dma_channel = 1;
  static constexpr int spi_sclk = 18;
  static constexpr int spi_mosi = 23;
  static constexpr int spi_miso = 19;
  static constexpr int spi_dlen = 8;
};
static lgfx::LGFX_SPI<LGFX_Config> tft;
static lgfx::Panel_ILI9341 panel;
static LGFX_Sprite sprite(&tft);
#include <RotaryEncoder.h>


// #define DEBUG
#define OZONELIFELIMIT 600 * 3600 // 600時間
#define BEEP


#define TFT_RST 16
#define TFT_DC 17
#define TFT_CS 5
#define PUMP_PIN 27

#define FAN_PIN 22
#define BEEP_PIN 2

// ozone チューブコントロールGPIO番号
int  OZONE_PIN[3] = { 26, 32, 33 };
// ozone チューブ利用優先順位 (利用時間短いものから優先使用)
int OZONEorder[3] = {0, 1, 2};


//  Switch DEBUG
/*
  #define ENCODER1_A_PIN 25   // ロータリーエンコーダーをスイッチ代わり
  #define ENCODER1_B_PIN 35
  #define ENCODER1_SWITCH_PIN 34
  #define ENCODER2_A_PIN 12
  #define ENCODER2_B_PIN 13
  #define ENCODER2_SWITCH_PIN 14
*/
// Not Switch DEBUG (normal)

#define ENCODER1_A_PIN 34
#define ENCODER1_B_PIN 35
#define ENCODER1_SWITCH_PIN 25
#define ENCODER2_A_PIN 12
#define ENCODER2_B_PIN 13
#define ENCODER2_SWITCH_PIN 14


#define TFT_BACKLIGHT_PIN 4
#define FRAMERATEDELAY 60

#define WAITTIMEMAX 82
#define TIMEMAX 64

#define OZONEMAX 5
#define Ozone_WARN_LIMIT 3

const char* recordfile = "/timerecord.txt";
RTC_DATA_ATTR int bootCount = 0;


RotaryEncoder encoder1(ENCODER1_A_PIN, ENCODER1_B_PIN);
RotaryEncoder encoder2(ENCODER2_A_PIN, ENCODER2_B_PIN);



LABEL maintenanceLabels1[6] = { md120, md121, md122, md123 , md124 , md130 };
LABEL maintenanceLabels2[8] = { md210, md211, md212, md213, md214, md215, m216, m217 };
// LABEL setLabels[2]={ setCONNECT,setRETURN};
// LABEL scanLabels[4]={ scannode1,scannode2,scannode3,scannode4};


int ozonePercent[OZONEMAX + 1] = { 7, 15, 20, 50, 70, 100 };
int timeSeconds[WAITTIMEMAX + 1] = {
#ifdef DEBUG
  1,    2,  3, 4,  5,  6,  7,  8,  9,  10,  11,  12,  13,  14,  15,  510,  540,  570,  600,  630,
#else
  60,    90,  120, 150,  180,  210,  240,  270,  300,  330,  360,  390,  420,  450,  480,  510,  540,  570,  600,  630,
#endif
  660,  690,  720, 750,  780,  810,  840,  870,  900,  930,  960,  990, 1020, 1050, 1080, 1110, 1140, 1170, 1200, 1230,
  1260, 1290, 1320, 1350, 1380, 1410, 1440, 1470, 1500, 1530, 1560, 1590, 1620, 1650, 1680, 1710, 1740, 1770, 1800, 2700,
  3600, 5400, 7200, 9000, 10800, 12600, 14400, 16200, 18000, 19800, 21600, 23400, 25200, 27000, 28800, 30600, 32400, 34200, 36000, 37800,
  39600, 41400, 43200
};
// sprite design
  int16_t spriteoffsetx=40, spriteoffsety=100;
  int16_t spritesizex=240, spritesizey=64;
  
// EEPROM paramater
int maintenance_Count = 1;
int oneshot_Ozonelevel = 2;
int oneshot_endtime = 1;
int program_Ozonelevel = 1;
int program_starttime = 61;
int program_endtime = 61;
int log_pump = 0;
int log_ozone = 0;
int log_ozone10; // log_ozoneの10倍
int log_fan = 0;
int log_oncount = 0;
int log_OZONE10[3] = {0, 0, 0}; // 0.1秒単位の稼働時間
int mode_X = MODEXDEFAULT;

// run time using
String mode = "set"; // set,run,resetting
String motionPUMP = "OFF"; // ON/OFF
String motionFAN = "OFF"; // ON/OFF
String motionOZONE = "OFF"; // ON/OFF

int runRemain;      // remain countdown second
int settingRemain;   // setting second
int autoExitCounter; // countdown second
int timercounter = 0;
int Ozonelevel = 0; // 0 to 6 この変数を変更するとオゾン発生に即反映される
int Ozonedisplay = 3;  // 設定中の値 現在は Ozonelevel = Ozonedisplay


hw_timer_t * timer = NULL;

void IRAM_ATTR onTimer() {  // each 20msec
  //  encoder2.tick(); // just call tick() to check the state.
  //  encoder1.tick(); // just call tick() to check the state.
  timercounter++;
}

void IRAM_ATTR ISR() {
  encoder2.tick(); // just call tick() to check the state.
  encoder1.tick(); // just call tick() to check the state.

}



void IRAM_ATTR RELEASE1() {
  static unsigned long nowtime;
  static unsigned long oldtime = 0;
  nowtime = millis();
  if ( 150 < ( nowtime - oldtime )) {
  }
  oldtime = nowtime;
}
void IRAM_ATTR RELEASE2() {
  static unsigned long nowtime;
  static unsigned long oldtime = 0;
  nowtime = millis();
  if ( 150 < ( nowtime - oldtime )) {
  }
  oldtime = nowtime;
}


int verifySPIFFSData( int data1, int data2 , int number ) {
  if ( data1 != data2 )
  {
    Serial.printf("SPIFFS data diff%d= %d %d \n", number, data1, data2);
    return (1);
  } else {
    Serial.printf("SPIFFS data ok= %d %d , %d\n", number, data1, data2);
    return (0);
  }
}


void eeprom_write() {
  // EEPROM操作のためにタイマーを停止
/*  timerAlarmDisable(timer);    // stop alarm
  timerDetachInterrupt(timer);  // detach interrupt
  timerEnd(timer);      // end timer*/
  Serial.println("timerstop done");
  // EEPROM操作のために割り込みを停止
  detachInterrupt(ENCODER1_A_PIN);
  detachInterrupt(ENCODER1_B_PIN);
  detachInterrupt(ENCODER1_SWITCH_PIN);
  detachInterrupt(ENCODER2_A_PIN);
  detachInterrupt(ENCODER2_B_PIN);   // rotary_encoder
  detachInterrupt(ENCODER2_SWITCH_PIN);  // rotary_encoder push switch
  Serial.println("detachinterrupt done");

  int data[14];
  log_ozone =  log_ozone10 / 10;
  data[0]  = maintenance_Count;
  data[1] = oneshot_Ozonelevel;
  data[2] = oneshot_endtime;
  data[3] = program_Ozonelevel;
  data[4] = program_starttime;
  data[5] = program_endtime;
  data[6] = log_pump;
  data[7] = log_ozone;
  data[8] = log_fan;
  data[9] = log_oncount;
  data[11] = log_OZONE10[0];
  data[12] = log_OZONE10[1];
  data[13] = log_OZONE10[2];
  data[10] = mode_X;
  int n = 0;
  Serial.println("EEPROM WRITE");
  for (int i = 0; i < 14; i++) {
    EEPROM.put(n, data[i]);
    Serial.print(i);
    Serial.print(":");
    Serial.println(data[i]);
    n += 4; // 4バイト毎
  }
  delay(100);
  Serial.println("EEPROM.put done");
  EEPROM.commit(); // EEPROMに書き込み確定
  Serial.println("EEPROM.commit done");
 // delay(2000);
  // 割り込み再設定
  attachInterrupt(ENCODER1_A_PIN, ISR, CHANGE);   // rotary_encoder
  attachInterrupt(ENCODER1_B_PIN, ISR, CHANGE);   // rotary_encoder
  attachInterrupt(ENCODER1_SWITCH_PIN, RELEASE1, RISING);  // rotary_encoder push switch
  attachInterrupt(ENCODER2_A_PIN, ISR, CHANGE);   // rotary_encoder
  attachInterrupt(ENCODER2_B_PIN, ISR, CHANGE);   // rotary_encoder
  attachInterrupt(ENCODER2_SWITCH_PIN, RELEASE2, RISING);  // rotary_encoder push switch
  // タイマー再稼働
  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 20000, true);  // 20m sec
  timerAlarmEnable(timer);
  Serial.println("interrupt recover done");

}
int eeprom_verify() {
  int chkEEPROMerrorcount = 0;
  int n = 0;
  int data[14];
  for (int i = 0; i < 14; i++) {
    EEPROM.get(n, data[i]); // EEPROMより読み込み
    n += 4; // 4バイト毎
  }
  if ( maintenance_Count != data[0]) {
    Serial.printf("error 0 as %d\n", data[0]);
    chkEEPROMerrorcount++;
  }
  if ( oneshot_Ozonelevel != data[1]) {
    Serial.printf("error 1 as %d\n", data[1]);
    chkEEPROMerrorcount++;
  }
  if ( oneshot_endtime != data[2]) {
    Serial.printf("error 2 as %d\n", data[2]);
    chkEEPROMerrorcount++;
  }
  if ( program_Ozonelevel != data[3]) {
    Serial.printf("error 3 as %d\n", data[3]);
    chkEEPROMerrorcount++;
  }
  if ( program_starttime != data[4]) {
    Serial.printf("error 4 as %d\n", data[4]);
    chkEEPROMerrorcount++;
  }
  if ( program_endtime != data[5]) {
    Serial.printf("error 5 as %d\n", data[5]);
    chkEEPROMerrorcount++;
  }
  if ( log_pump != data[6]) {
    Serial.printf("error 6 as %d\n", data[6]);
    chkEEPROMerrorcount++;
  }
  if ( log_ozone != data[7]) {
    Serial.printf("error 7 as %d\n", data[7]);
    chkEEPROMerrorcount++;
  }
  if ( log_fan != data[8]) {
    Serial.printf("error 8 as %d\n", data[8]);
    chkEEPROMerrorcount++;
  }
  if ( log_oncount != data[9]) {
    Serial.printf("error 9 as %d\n", data[9]);
    chkEEPROMerrorcount++;
  }
  if ( log_OZONE10[0] != data[11]) {
    Serial.printf("error 11 as %d\n", data[11]);
    chkEEPROMerrorcount++;
  }
  if ( log_OZONE10[1] != data[12]) {
    Serial.printf("error 12 as %d\n", data[12]);
    chkEEPROMerrorcount++;
  }
  if ( log_OZONE10[2] != data[13]) {
    Serial.printf("error 13 as %d\n", data[13]);
    chkEEPROMerrorcount++;
  }
  if ( mode_X != data[10]) {
    Serial.printf("error 10 as %d\n", data[10]);
    chkEEPROMerrorcount++;
  }
  Serial.printf("EEPROM 13 as %d\n", data[13]);
  return ( chkEEPROMerrorcount++);
}

void eeprom_read() {
  int n = 0;
  int data[14];
  Serial.println("eeprom_read");
  for (int i = 0; i < 14; i++) {
    EEPROM.get(n, data[i]); // EEPROMより読み込み
    n += 4; // 4バイト毎
    Serial.print(i);
    Serial.print(":");
    Serial.println(data[i]);
  }
  if ( -1 != data[0] ) {
    if ( 0 == data[0] ) {  // eeprom data version up
      log_OZONE10[0] = data[7] / 30;
      log_OZONE10[1] = data[7] / 30;
      log_OZONE10[2] = data[7] / 30;
    } else {
      maintenance_Count = data[0];
      if ( -1 != data[11] ) log_OZONE10[0] = data[11];
      if ( -1 != data[12] ) log_OZONE10[1] = data[12];
      if ( -1 != data[13] ) log_OZONE10[2] = data[13];
    }
  }

  // ozone管利用優先順位
  if ( log_OZONE10[0] <= log_OZONE10[1] && log_OZONE10[0] <= log_OZONE10[2]  ) {
    OZONEorder[0] = 0;
    if ( log_OZONE10[1] <= log_OZONE10[2] ) {
      OZONEorder[1] = 1;
      OZONEorder[2] = 2;
    } else {
      OZONEorder[1] = 2;
      OZONEorder[2] = 1;
    }
  } else {
    if ( log_OZONE10[1] <= log_OZONE10[2] ) {
      OZONEorder[0] = 1;
      if ( log_OZONE10[0] <= log_OZONE10[2] ) {
        OZONEorder[1] = 0;
        OZONEorder[2] = 2;
      } else {
        OZONEorder[1] = 2;
        OZONEorder[2] = 0;
      }
    } else {
      OZONEorder[0] = 2;
      if ( log_OZONE10[0] <= log_OZONE10[1] ) {
        OZONEorder[1] = 0;
        OZONEorder[2] = 1;
      } else {
        OZONEorder[1] = 1;
        OZONEorder[2] = 0;
      }
    }
  }

  if (
    0 > data[1] || 0 > data[2] || 0 > data[3] || 0 > data[4] || 0 > data[5] ||
    0 > data[6] || 0 > data[7] || 0 > data[8] || 0 > data[9] || 0 > data[10] ||
    WAITTIMEMAX < data[2] || WAITTIMEMAX < data[4] || WAITTIMEMAX < data[5] )
 {     
  Serial.println("EEPROM seems first use!");
  log_ozone10 = log_ozone * 10;
 } else {
   oneshot_Ozonelevel = data[1];
   oneshot_endtime = data[2];
   program_Ozonelevel = data[3];
   program_starttime = data[4];
   program_endtime = data[5];
   log_pump = data[6];
   log_ozone = data[7];
   log_fan = data[8];
   log_oncount = data[9];
   mode_X = data[10];
   }
}


//------------------------------------------------------------------------------
void setup() {

  Serial.begin(115200);
  Serial.println("OZONCONTROLLER START");  
  Serial.println("sprite initialized");
  pinMode(ENCODER1_SWITCH_PIN, INPUT_PULLUP);
  pinMode(ENCODER2_SWITCH_PIN, INPUT_PULLUP);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(OZONE_PIN[0], OUTPUT);
  pinMode(OZONE_PIN[1], OUTPUT);
  pinMode(OZONE_PIN[2], OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(BEEP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, 0);
  digitalWrite(OZONE_PIN[0], 0);
  digitalWrite(OZONE_PIN[1], 0);
  digitalWrite(OZONE_PIN[2], 0);
  digitalWrite(FAN_PIN, 0);
  digitalWrite(BEEP_PIN, 0);

  attachInterrupt(ENCODER1_A_PIN, ISR, CHANGE);   // rotary_encoder
  attachInterrupt(ENCODER1_B_PIN, ISR, CHANGE);   // rotary_encoder
  attachInterrupt(ENCODER1_SWITCH_PIN, RELEASE1, RISING);  // rotary_encoder push switch
  attachInterrupt(ENCODER2_A_PIN, ISR, CHANGE);   // rotary_encoder
  attachInterrupt(ENCODER2_B_PIN, ISR, CHANGE);   // rotary_encoder
  attachInterrupt(ENCODER2_SWITCH_PIN, RELEASE2, RISING);  // rotary_encoder push switch


  Serial.println("interrupt set");

  /*
    //   init filesystem

    bool res = SPIFFS.begin(true); // FORMAT_SPIFFS_IF_FAILED
    Serial.print("SPIFFS.begining...");
    if (!res) {
    Serial.println("fail");
    handleFatalError("ファイルシステムを開始できません");
    }
    Serial.println("OK");

    // set file read
    File fp = SPIFFS.open(recordfile, "r");
    if (!fp) {
    labelText(notifyInitializing );
    SPIFFS.format();
    Serial.println("Spiffs formatted");
    File fp = SPIFFS.open(recordfile, "w");
    if (!fp) {
      Serial.println("Write Opening recordfile failed");
      handleFatalError("ファイルに記録できません");
    } else {
      //  fp.println(12345);
      fp.println(oneshot_Ozonelevel);
      fp.println(oneshot_endtime);
      fp.println(program_Ozonelevel);
      fp.println(program_starttime);
      fp.println(program_endtime);
      fp.println(log_pump);
      fp.println(log_ozone);
      fp.println(log_fan);
      fp.println(log_oncount);
      fp.close();
      File fp = SPIFFS.open(recordfile, "r");
    }
    }
    if (!fp) {
    Serial.println("Opening recordfile failed");
    handleFatalError("ファイルに記録できません");
    } else {
    int temp;
    temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) oneshot_Ozonelevel = temp; // dummy
    temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) oneshot_Ozonelevel = temp; // dummy
    temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) oneshot_Ozonelevel = temp; // dummy
    temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) oneshot_Ozonelevel = temp; // dummy
    temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) oneshot_Ozonelevel = temp; // dummy
    temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) oneshot_Ozonelevel = temp; // dummy
    temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) oneshot_Ozonelevel = temp; // dummy
    temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) oneshot_Ozonelevel = temp; // dummy
    temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) oneshot_Ozonelevel = temp; // dummy
    temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) oneshot_Ozonelevel = temp;
    temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) oneshot_endtime = temp;
    temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) program_Ozonelevel = temp;
    temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) program_starttime = temp;
    temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) program_endtime = temp;
    temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) log_pump = temp;
    temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) log_ozone = temp;
    temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) log_fan = temp;
    temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) log_oncount = temp;
    Serial.print("oneshot_Ozonelevel:");
    Serial.println(oneshot_Ozonelevel);
    Serial.print("oneshot_endtime:");
    Serial.println(oneshot_endtime);
    Serial.print("program_Ozonelevel:");
    Serial.println(program_Ozonelevel);
    Serial.print("program_starttime:");
    Serial.println(program_starttime);
    Serial.print("program_endtime:");
    Serial.println(program_endtime);
    Serial.print("log_pump:");
    Serial.println(log_pump);
    Serial.print("log_ozone:");
    Serial.println(log_ozone);
    Serial.print("log_fan:");
    Serial.println(log_fan);
    Serial.print("log_oncount:");
    Serial.println(log_oncount);
    log_ozone10 = log_ozone * 10;
    fp.close();
    }
    SPIFFS.end();

  */
  EEPROM.begin(64); // int 4 byte x 14 + 8
  delay(200);
  eeprom_read();

  Serial.print("mainetance_Count:");
  Serial.println(maintenance_Count);
  Serial.print("oneshot_Ozonelevel:");
  Serial.println(oneshot_Ozonelevel);
  Serial.print("oneshot_endtime:");
  Serial.println(oneshot_endtime);
  Serial.print("program_Ozonelevel:");
  Serial.println(program_Ozonelevel);
  Serial.print("program_starttime:");
  Serial.println(program_starttime);
  Serial.print("program_endtime:");
  Serial.println(program_endtime);
  Serial.print("log_pump:");
  Serial.println(log_pump);
  Serial.print("log_ozone:");
  Serial.println(log_ozone);
  Serial.print("log_fan:");
  Serial.println(log_fan);
  Serial.print("log_oncount:");
  Serial.println(log_oncount);
  Serial.print("log_OZONE10_0:");
  Serial.println(log_OZONE10[0] / 10);
  Serial.print("log_OZONE10_1:");
  Serial.println(log_OZONE10[1] / 10);
  Serial.print("log_OZONE10_2:");
  Serial.println(log_OZONE10[2] / 10);
  Serial.print("mode_X:");
  Serial.println(mode_X);
  //timerAlarmDisable(timer);    // stop alarm
  //timerDetachInterrupt(timer);  // detach interrupt
  //timerEnd(timer);      // end timer

 log_fan=10;

 eeprom_write();
 Serial.println("eepron write done.");
}

void loop()
{


}
