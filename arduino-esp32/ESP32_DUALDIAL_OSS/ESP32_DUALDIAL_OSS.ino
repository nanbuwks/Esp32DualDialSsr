#include <LovyanGFX.hpp>
//#include <SPIFFS.h>
//#include <FS.h>
#include <EEPROM.h>
#define VERSION "Ver1.1"
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


#include <RotaryEncoder.h>


// #define DEBUG
#define OZONELIFELIMIT 600 * 3600 // 600時間
#define BEEP


#define TFT_RST 16
#define TFT_DC 17
#define TFT_CS 5
#define PUMP_PIN 27
#define OZONE0_PIN 26
#define OZONE1_PIN 32
#define OZONE2_PIN 33
#define FAN_PIN 22
#define BEEP_PIN 2

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



LABEL maintenanceLabels1[5] = { md120, md121, md122, md123 , md124};
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
int log_OZONE0 = 0;
int log_OZONE1 = 0;
int log_OZONE2 = 0;


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

static int encoder1Pushed = 0; // you must turn 0 after button check
static int encoder1Pushing = 0;
static int encoder2Pushed = 0; // you must turn 0 after button check
static int encoder2Pushing = 0;

// colors
int OzoneBgColor = TFT_BLACK;
int OzoneFgColor = TFT_BLACK;
int OzonePnColor = TFT_LIGHTGRAY;
int OzoneTitleColor = TFT_BLUE;


hw_timer_t * timer = NULL;

void IRAM_ATTR onTimer() {  // each 20msec
  //  encoder2.tick(); // just call tick() to check the state.
  //  encoder1.tick(); // just call tick() to check the state.
  timercounter++;
  if ( motionOZONE == "ON" ) {

    if ( 2 <  Ozonelevel ){
       runFAN();
    } else {
       stopFAN();
    }
    if ( ozonePercent[Ozonelevel] == timercounter % 100 )  // 100 means 20msec * 100 = 2 sec.
      digitalWrite(OZONE0_PIN, LOW);
    if ( 0                        == timercounter % 100 )
      digitalWrite(OZONE0_PIN, HIGH);
    if ( ozonePercent[Ozonelevel] == ( timercounter + 33 ) % 100 )
      digitalWrite(OZONE1_PIN, LOW);
    if ( 0                        == ( timercounter + 33 ) % 100 )
      digitalWrite(OZONE1_PIN, HIGH);
    if ( ozonePercent[Ozonelevel] == ( timercounter + 66 ) % 100 )
      digitalWrite(OZONE2_PIN, LOW);
    if ( 0                        == ( timercounter + 66 ) % 100 )
      digitalWrite(OZONE2_PIN, HIGH);
    if ( 0 == timercounter % 500  ) // each 10 sec.
      log_ozone10 = log_ozone10 + ozonePercent[Ozonelevel];
  }
  if ( 0 == timercounter % 5 )
    countdown();
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
    /*

        Serial.print("released!!!!!!!!!!");
        Serial.print(oldtime);
        Serial.print("  ");
        Serial.println(nowtime);
        Serial.print("1release");
    */
    encoder1Pushed = 1;

    encoder1Pushing = 0;
  }
  /*
      Core  1 panic'ed (Interrupt wdt timeout on CPU1) 対策でコメントアウト

    Serial.print(nowtime);
    Serial.print("-------------------------------------------------------------------------------------------------------------------------------------------------------------------------- ");
    Serial.print(encoder1Pushed);
    Serial.print(" ");
    Serial.print(encoder1Pushing);
    Serial.print(" ");
    Serial.print(encoder2Pushed);
    Serial.print(" ");
    Serial.print(encoder2Pushing);
    Serial.println(" ");
  */
  oldtime = nowtime;
}
void IRAM_ATTR RELEASE2() {
  static unsigned long nowtime;
  static unsigned long oldtime = 0;
  nowtime = millis();
  if ( 150 < ( nowtime - oldtime )) {
    encoder2Pushed = 1;
    encoder2Pushing = 0;
  }
  /*
      Core  1 panic'ed (Interrupt wdt timeout on CPU1) 対策でコメントアウト
       Serial.print(" ");
    Serial.print(encoder1Pushed);
    Serial.print(" ");
    Serial.print(encoder1Pushing);
    Serial.print(" ");
    Serial.print(encoder2Pushed);
    Serial.print(" ");
    Serial.print(encoder2Pushing);
    Serial.println(" ");
  */
  oldtime = nowtime;
}

void countdown() { // Timer割り込みから呼び出される
  // each 100 msec.
  static int countdowncounter = 0;
  countdowncounter++;
  //Serial.print(countdowncounter);

  if ( mode == "notify" ) {
    if ( 0 == countdowncounter % 10 )
    {
      // Serial.printf("runRemain:%d", runRemain);
      runRemain--;
      // Serial.printf("runRemain:%d", runRemain);
      if ( 0 > runRemain )
      {
        runRemain = 0;
      }
    }
  }
  if ( mode == "run" ) { // timeStateだけ0.5秒間隔で点滅
    if ( 0 == countdowncounter % 10 )
    {
      timeState.fgcolor = TFT_GREEN;
    }
    if ( 5 == countdowncounter % 10 )
    {
      ozoneChangeColor(encoder2.getPosition());
    }

    if ( 0 == countdowncounter % 10 )
    {
      runRemain--;
      // Serial.printf("runRemain:%d", runRemain);
      if ( 0 > runRemain )
      {
        runRemain = 0;
      }
      // 表示用ラベルを更新
      char buf[20];
      int seconds = runRemain;
      if ( seconds >= 3600 ) {
        sprintf(buf, "%2d", seconds / 3600);
        timeHour.text = String(buf);
        sprintf(buf, "%02d", (seconds / 60) % 60);
        timeMinute.text = String(buf);
        sprintf(buf, "%02d", seconds % 60);
        timeSecond.text = String(buf);
      } else if ( seconds < 3600 && seconds >= 60 ) {
        sprintf(buf, "  ");
        timeHour.text = String(buf);
        sprintf(buf, "%2d", (seconds / 60) % 60);
        timeMinute.text = String(buf);
        sprintf(buf, "%02d", seconds % 60);
        timeSecond.text = String(buf);
      } else if ( seconds < 60 ) {
        sprintf(buf, "  ");
        timeHour.text = String(buf);
        sprintf(buf, "  ");
        timeMinute.text = String(buf);
        sprintf(buf, "%2d", seconds % 60);
        timeSecond.text = String(buf);
      }
    }
  }

  if ( mode == "programmode1" || mode == "programmode2" ) {
    //     Serial.println(countdowncounter);


    if ( 0 == countdowncounter % 10 )// timeStateだけ0.5秒間隔で点滅
    {
      timeState.fgcolor = TFT_GREEN;
    }
    if ( 5 == countdowncounter % 10 )
    {
      ozoneChangeColor(encoder2.getPosition());
    }



    if ( 0 == countdowncounter % 10 )
    {
      runRemain--;
      if ( 0 > runRemain )
        runRemain = 0;
      // 表示用ラベルを更新
      char buf[20];
      int seconds = runRemain;
      if ( seconds >= 3600 ) {
        sprintf(buf, "%2d", seconds / 3600);
        timeHour.text = String(buf);
        sprintf(buf, "%02d", (seconds / 60) % 60);
        timeMinute.text = String(buf);
        sprintf(buf, "%02d", seconds % 60);
        timeSecond.text = String(buf);
      } else if ( seconds < 3600 && seconds >= 60 ) {
        sprintf(buf, "  ");
        timeHour.text = String(buf);
        sprintf(buf, "%2d", (seconds / 60) % 60);
        timeMinute.text = String(buf);
        sprintf(buf, "%02d", seconds % 60);
        timeSecond.text = String(buf);
      } else if ( seconds < 60 ) {
        sprintf(buf, "  ");
        timeHour.text = String(buf);
        sprintf(buf, "  ");
        timeMinute.text = String(buf);
        sprintf(buf, "%2d", seconds % 60);
        timeSecond.text = String(buf);
      }
    }
  }

  if ( mode == "resetting" ) {


    if ( 0 == countdowncounter % 5 )
    {
      timeState.fgcolor = TFT_GREEN;
      timeHour.fgcolor = TFT_GREEN;
      timeMinute.fgcolor = TFT_GREEN;
      timeSecond.fgcolor = TFT_GREEN;
      timeZikan.fgcolor = TFT_GREEN;
      timeFan.fgcolor = TFT_GREEN;
      timeByou.fgcolor = TFT_GREEN;
    }
    if ( 2 == countdowncounter % 5 )
    {
      ozoneChangeColor(encoder2.getPosition());
    }
    if ( 0 == countdowncounter % 10 )
    {
      runRemain--;
      if ( 0 > runRemain )
        runRemain = 0;
      autoExitCounter--;
      if ( 0 > autoExitCounter )
        autoExitCounter = 0;
    }
  }
  if ( mode == "programmode2resetting" ) {
    if ( 0 == countdowncounter % 5 )
    {
      timeState.fgcolor = TFT_GREEN;
      timeHour.fgcolor = TFT_GREEN;
      timeMinute.fgcolor = TFT_GREEN;
      timeSecond.fgcolor = TFT_GREEN;
      timeZikan.fgcolor = TFT_GREEN;
      timeFan.fgcolor = TFT_GREEN;
      timeByou.fgcolor = TFT_GREEN;

    }
    if ( 2 == countdowncounter % 5 )
    {
      ozoneChangeColor(encoder2.getPosition());
    }
    if ( 0 == countdowncounter % 10 )
    {
      runRemain--;
      if ( 0 > runRemain )
        runRemain = 0;
      autoExitCounter--;
      if ( 0 > autoExitCounter )
        autoExitCounter = 0;
    }

  }
  if ( mode == "programmode1resetting" ) {

    if ( 0 == countdowncounter % 5 )
    {
      timeState.fgcolor = TFT_GREEN;
      timeHour.fgcolor = TFT_GREEN;
      timeMinute.fgcolor = TFT_GREEN;
      timeSecond.fgcolor = TFT_GREEN;
      timeZikan.fgcolor = TFT_GREEN;
      timeFan.fgcolor = TFT_GREEN;
      timeByou.fgcolor = TFT_GREEN;
    }
    if ( 2 == countdowncounter % 5 )
    {
      ozoneChangeColor(encoder2.getPosition());
    }
    if ( 0 == countdowncounter % 10 )
    {
      runRemain--;
      if ( 0 > runRemain )
        runRemain = 0;
      autoExitCounter--;
      if ( 0 > autoExitCounter )
        autoExitCounter = 0;
    }
  }
  if ( 0 == countdowncounter % 10 )
  {
    if ( motionPUMP == "ON" )
      log_pump++;
    if ( motionFAN == "ON" )
      log_fan++;
  }
}
int maintenanceSelect1() {
  static int timeminutes = 30;
  encoder1.setPosition(0);
  int startpos = encoder1.getPosition();
  int pos1 = 100;
  int newPos1 = encoder1.getPosition();
  int exitflag = 0;
  int selected;
  //  while ( 0 == exitflag )
  while (1)
  {
    newPos1 = encoder1.getPosition();
    Serial.println(newPos1);

    if (pos1 != newPos1) {
      Serial.print("pos1=");
      Serial.print(newPos1);
      Serial.println();
      pos1 = newPos1;
      selected = (( pos1 % 5) + 5 ) % 5; // negative modulo
      for ( int i = 0; i < 5; i++ ) {
        if ( i == selected) {
          selectLabelText(maintenanceLabels1[i]);
        } else {
          labelText(maintenanceLabels1[i]);
        }
      }
    }

    if ( 1 == encoder1Pushed ) {
      encoder1Pushed = 0;
      switch (selected) {
        case 0:
          // pump
          if (  maintenanceLabels1[selected].text == "OFF" ) {
            maintenanceLabels1[selected].text = "ON";
            selectLabelText(maintenanceLabels1[selected]);
            digitalWrite(PUMP_PIN, HIGH);
          } else {
            maintenanceLabels1[selected].text = "OFF";
            selectLabelText(maintenanceLabels1[selected]);
            digitalWrite(PUMP_PIN, LOW);
          }
          break;
        case 1:
          // ozone0
          if (  maintenanceLabels1[selected].text == "OFF" ) {
            maintenanceLabels1[selected].text = "ON";
            selectLabelText(maintenanceLabels1[selected]);
            digitalWrite(OZONE0_PIN, HIGH);
          } else {
            maintenanceLabels1[selected].text = "OFF";
            selectLabelText(maintenanceLabels1[selected]);
            digitalWrite(OZONE0_PIN, LOW);
          }
          break;
        case 2:
          // ozone1
          if (  maintenanceLabels1[selected].text == "OFF" ) {
            maintenanceLabels1[selected].text = "ON";
            selectLabelText(maintenanceLabels1[selected]);
            digitalWrite(OZONE1_PIN, HIGH);
          } else {
            maintenanceLabels1[selected].text = "OFF";
            selectLabelText(maintenanceLabels1[selected]);
            digitalWrite(OZONE1_PIN, LOW);
          }
          break;

        case 3:
          // ozone2
          if (  maintenanceLabels1[selected].text == "OFF" ) {
            maintenanceLabels1[selected].text = "ON";
            selectLabelText(maintenanceLabels1[selected]);
            digitalWrite(OZONE2_PIN, HIGH);
          } else {
            maintenanceLabels1[selected].text = "OFF";
            selectLabelText(maintenanceLabels1[selected]);
            digitalWrite(OZONE2_PIN, LOW);
          }
          break;

        case 4:
          // ozone
          log_ozone10 = 0;
          Serial.printf("log_ozone:1:%d", log_ozone);
          eeprom_write();
          Serial.printf("log_ozone:2:%d", log_ozone);
          md111.text = mkTimeString(log_ozone);
          labelText(md111);
          break;

      }
    }

    if ( 1 == encoder2Pushed ) {
      encoder2Pushed = 0;
      //    Serial.println("leave maintenanceselect1");
      return (2);
    }
    delay(100);

  }

}
int maintenanceSelect2() {
  digitalWrite(PUMP_PIN, LOW);
  digitalWrite(OZONE0_PIN, LOW);
  digitalWrite(OZONE1_PIN, LOW);
  digitalWrite(OZONE2_PIN, LOW);
  static int timeminutes = 30;
  encoder1.setPosition(0);
  encoder2.setPosition(0);
  int startpos = encoder1.getPosition();
  int pos1 = 0;
  int pos2 = 0;
  int push1 = 0;
  int push2 = 0;

  int newPos1 = encoder1.getPosition();
  int exitflag = 0;
  int selected = 0;
  int buzzerflag = 0;
  char buf[30];
  //  tft.fillScreen(TFT_WHITE);
  //  selectLabelText(timetitle);
  //  labelText(timemin);
  selectLabelText(maintenanceLabels2[0]);
  encoder1Pushed = 1;
  selected = -1;

  while ( 0 == exitflag )  // 現在exit条件はない
  {

    Serial.println(newPos1);
    if (1 == encoder1Pushed) {
      encoder1Pushed = 0;
      selected++;


      switch (selected) {
        case 0:
          // pump
          maintenanceLabels2[selected].text = "ON";
          selectLabelText(maintenanceLabels2[selected]);
          digitalWrite(PUMP_PIN, HIGH);
          break;
        case 1:
          // ozone0 & pump
          maintenanceLabels2[selected].text = "ON";
          selectLabelText(maintenanceLabels2[selected]);
          digitalWrite(OZONE0_PIN, HIGH);
          break;
        case 2:
          // ozone1 & pump
          maintenanceLabels2[selected].text = "ON";
          maintenanceLabels2[1].text = "OFF";
          digitalWrite(OZONE1_PIN, HIGH);
          digitalWrite(OZONE0_PIN, LOW);
          break;
        case 3:
          // ozone2 & pump
          maintenanceLabels2[selected].text = "ON";
          maintenanceLabels2[2].text = "OFF";
          digitalWrite(OZONE2_PIN, HIGH);
          digitalWrite(OZONE1_PIN, LOW);
          break;
        case 4:
          // timer
          maintenanceLabels2[selected].text = "ON";
          maintenanceLabels2[0].text = "OFF";
          maintenanceLabels2[3].text = "OFF";
          digitalWrite(PUMP_PIN, LOW);
          digitalWrite(OZONE2_PIN, LOW);
          if ( 0 == buzzerflag ) {
            mode = "run";
#ifdef DEBUG
            runRemain = 6;
#else.
            runRemain = 60;
#endif
            buzzerflag = 1;
          }
          break;
        case 5:
          // buzzer
          maintenanceLabels2[selected].text = "ON";
          maintenanceLabels2[4].text = "OFF";
          digitalWrite(BEEP_PIN, HIGH);
          break;
        case 6:
          // lotation

          maintenanceLabels2[5].text = "OFF";
          digitalWrite(BEEP_PIN, LOW);

          break;
        case 7:
          // button


          break;
      }
      for ( int i = 0; i < 8; i++ ) {
        if ( i == selected) {
          selectLabelText(maintenanceLabels2[i]);
        } else {
          labelText(maintenanceLabels2[i]);
        }
      }
    }
    if ( 6 == selected ) {
      pos1 = encoder1.getPosition();
      pos2 = encoder2.getPosition();
      sprintf(buf, "  L%4d     R%4d", pos2, pos1);
      md217.text = String(buf);
      labelText(md217);
    }
    if ( 7 == selected) {
      encoder1Pushed = 0;
      encoder2Pushed = 0;

      // 無限ループ
      while (1) {
        if ( 1 == encoder1Pushed || 1 == encoder2Pushed) {
          if ( 1 == encoder1Pushed) {
            encoder1Pushed = 0;
            push1++;
          }
          if ( 1 == encoder2Pushed) {
            encoder2Pushed = 0;
            push2++;
          }
          sprintf(buf, "  L %3d    R %3d",  push2,  push1);
          md217.text = String(buf);
          labelText(md217);
        }
      }
    }
    // Serial.printf(" buzzerflag=%d,runRemain=%d mode=%s",buzzerflag,runRemain,mode);
    if ( 1 == buzzerflag && 0 == runRemain )
    {
      buzzerflag = 2;
      beep(5000);
    }
    delay(FRAMERATEDELAY);
    //  touchvalue = touchscan();
    //  if (touchvalue == 1 || touchvalue == 4 )
    //    exitflag = 1;
  }
  Serial.print("exit! ");
  //Serial.println(touchvalue);
}
void selectLabelText(LABEL label) {
  unsigned long start = micros();
  tft.fillRect(label.x1, label.y1, label.xlength, label.ylength, label.fgcolor);
  int16_t xstart, ystart;
  uint16_t w, h;
  tft.setFont( &label.font);
  tft.setTextSize(label.scale);
  //  tft.getTextBounds( label.text,0,0,&xstart,&ystart,&w,&h);
  w = tft.textWidth(label.text);
  h = tft.fontHeight();
  int xtextbase, ytextbase;
  //xtextbase = label.x1 + label.xlength / 2 - w / 2;
  //ytextbase = label.y1 + label.ylength / 2 + h / 2;
  tft.setCursor(label.x1, label.y1);
  tft.setTextColor(label.bgcolor);
  tft.println(label.text);
}

void labelText(LABEL label) {
  unsigned long start = micros();
  tft.fillRect(label.x1, label.y1, label.xlength, label.ylength, label.bgcolor);
  int16_t xstart, ystart;
  uint16_t w, h;
  tft.setFont( &label.font);
  tft.setTextSize(label.scale);
  // tft.getTextBounds( label.text,0,0,&xstart,&ystart,&w,&h);
  w = tft.textWidth(label.text);
  h = tft.fontHeight();
  int xtextbase, ytextbase;
  // xtextbase = label.x1+label.xlength/2 - w/2;
  // ytextbase = label.y1+label.ylength/2 + h/2;
  // tft.setCursor(xtextbase, ytextbase);
  tft.setCursor(label.x1, label.y1);
  tft.setTextColor(label.fgcolor);
  tft.println(label.text);
}

void frameText(LABEL label) {
  unsigned long start = micros();
  tft.drawRect(label.x1, label.y1, label.xlength, label.ylength, label.bgcolor);
  int16_t xstart, ystart;
  uint16_t w, h;
  tft.setFont( &label.font);
  tft.setTextSize(label.scale);
  // tft.getTextBounds( label.text,0,0,&xstart,&ystart,&w,&h);
  w = tft.textWidth(label.text);
  h = tft.fontHeight();
  int xtextbase, ytextbase;
  xtextbase = label.x1 + label.xlength / 2 - w / 2;
  ytextbase = label.y1 + label.ylength / 2 + h / 2;
  tft.setCursor(xtextbase, ytextbase);
  tft.setTextColor(label.fgcolor);
  tft.println(label.text);
}


//----------------------------------------------------------------------------------
// サブルーチン
int handleFatalError(String msg) {

  tft.fillScreen(TFT_WHITE);
  tft.drawRect(12, 14, 296, 160, TFT_RED);
  tft.drawRect(13, 15, 294, 158, TFT_RED);
  tft.fillRect(25, 26, 271, 45, TFT_RED);
  tft.setTextColor(TFT_WHITE);
  tft.setFont(&myFont40);
  tft.setCursor(114, 30);
  tft.println("注 意");
  tft.setTextColor(TFT_BLACK);
  tft.setFont(&lgfxJapanGothicP_16);
  tft.setCursor(23, 91);
  tft.println("内部エラーが発生しました。");
  tft.setCursor(23, 111);
  tft.setFont(&lgfxJapanGothicP_12);
  tft.println("(" + msg + ")");
  tft.setFont(&lgfxJapanGothicP_16);
  tft.setCursor(23, 131);
  tft.println("メンテナンスを依頼してください。");
  tft.setFont(&lgfxJapanGothicP_16);
  tft.setCursor(68, 185);
  tft.println("【お問合せ先】 株式会社レボル");
  tft.setCursor(184, 208);
  tft.println("048-254-0070");
  while (1) {
    if ( 1 == encoder1Pushed || 1 == encoder2Pushed )
    { // exit
      if (1 == encoder1Pushed )
      {
        encoder1Pushed = 0;
        pi();
        return (1); // set done
      }
      if (1 == encoder2Pushed )
      {
        encoder2Pushed = 0;
        return (2); // cancel
      }
    }
    delay(FRAMERATEDELAY);
  }
}

int ozoneRun(int newPos2, int pos2) {  // オゾン濃度反映、オゾン濃度表示、オゾン警告画面切り替え
  autoExitCounter = 5;
  Serial.printf("in ozoneRun - newPos2:%d,pos2:%d\n", newPos2, pos2);
  if ( OZONEMAX < newPos2 ) {
    newPos2 = OZONEMAX;
    encoder2.setPosition(OZONEMAX);
  }
  if ( 0 >  newPos2 ) {
    newPos2 = 0;
    encoder2.setPosition(0);
  }

  if (
    ( pos2 >= Ozone_WARN_LIMIT && newPos2 <  Ozone_WARN_LIMIT )
    ||
    ( pos2 < Ozone_WARN_LIMIT && newPos2 >=  Ozone_WARN_LIMIT )
  )
  { // ゲージ4
    ozoneChangeColor(newPos2);
    drawSetDisp();
  }

  Ozonedisplay = newPos2;
  Ozonelevel = Ozonedisplay; // 現在は即反映
  ozoneDraw(Ozonedisplay);
  return (newPos2);
}
String mkTimeString(int seconds) { // 10:29:30 形式の文字列作成
  char buf[20];

  if ( seconds >= 3600 ) {
    sprintf(buf, "%2d:%02d:%02d", seconds / 3600, (seconds / 60) % 60, seconds % 60);
    return ( String(buf));

  } else if ( seconds < 3600 && seconds >= 60 ) {
    sprintf(buf, "   %2d:%02d", (seconds / 60) % 60, seconds % 60);
    return ( String(buf));
  } else if ( seconds < 60 ) {
    sprintf(buf, "      %2d", seconds % 60);
    return ( String(buf));
  }
}

void beep(int millis) {
  Serial.printf("beep:%d", millis);
#ifdef BEEP
  digitalWrite(BEEP_PIN, HIGH);
#endif
  delay(millis);
  digitalWrite(BEEP_PIN, LOW);
}
void pi() {
  beep(200);
}
void pipi() {
  beep(200);
  delay(50);
  beep(100);

}
void pipipipi() {
  beep(200);
  delay(50);
  beep(100);
  delay(100);
  beep(100);
  delay(100);
  beep(100);

}
void peee() {
  beep(1000);
}

//----------------------------------------------------------------------------------
// 画面描画関係
int notify() {            // スプラッシュ
  mode = "notify";
  tft.fillScreen(TFT_BLACK);
  tft.drawBitmap(160 - 30, 120 - 30, myBitmap, 60, 60, 65535); //make sure the width and height is exactly the dimension of the image, drawXBitmap does NOT work
  runRemain = 2;
  tft.setTextColor(TFT_WHITE);
  tft.setFont(&lgfxJapanGothicP_8);
  tft.setCursor(170, 150);
  tft.println(VERSION);
  int commandCounter = 0;
  while (1) {
    if ( 1 == encoder2Pushed && 0 == commandCounter % 2 ) {
      encoder2Pushed = 0;
      commandCounter++;
      Serial.printf("in notify:commandCounter:%d\n", commandCounter);
    }
    if ( 1 == encoder1Pushed && 1 == commandCounter % 2 ) {
      encoder1Pushed = 0;
      commandCounter++;
      Serial.printf("in notify:commandCounter:%d\n", commandCounter);
    }
    if ( 0 == runRemain )
    {
      //   Serial.printf("in notify:runRemain is zero\n");
      encoder1Pushed = 0;
      encoder2Pushed = 0;
      // stop();
      if ( 8 == commandCounter ) {
        return (1);
      } else {
        return (0);
      }
    }
    delay(FRAMERATEDELAY);
  }
}
int caution() {            // 警告
  tft.fillScreen(TFT_WHITE);
  tft.drawRect(12, 14, 296, 160, TFT_RED);
  tft.drawRect(13, 15, 294, 158, TFT_RED);
  tft.fillRect(25, 26, 271, 45, TFT_RED);
  tft.setTextColor(TFT_WHITE);
  tft.setFont(&myFont40);
  tft.setCursor(114, 30);
  tft.println(cautiontxt1);
  tft.setTextColor(TFT_BLACK);
  tft.setFont(&lgfxJapanGothicP_16);
  tft.setCursor(23, 91);
  tft.println(cautiontxt2);
  tft.setCursor(23, 111);
  tft.println(cautiontxt3);
  tft.setCursor(23, 131);
  tft.println(cautiontxt4);
  tft.setFont(&lgfxJapanGothicP_16);
  tft.setCursor(68, 185);
  tft.println(cautiontxt5);
  tft.setCursor(184, 208);
  tft.println(cautiontxt6);
  
  while (1) {
    if ( 1 == encoder1Pushed || 1 == encoder2Pushed )
    { // exit
      if (1 == encoder1Pushed )
      {
        encoder1Pushed = 0;
        pi();
        return (1); // set done
      }
      if (1 == encoder2Pushed )
      {
        encoder2Pushed = 0;
        return (2); // cancel
      }
    }
    delay(FRAMERATEDELAY);
  }
}
void setTimeColor(int ozonelevel) {  // 時間表示を戻す
  ozoneChangeColor(ozonelevel);
}
void ozoneChangeColor( int ozonelevel ) { // オゾン濃度によって色指定を変更する
  if ( Ozone_WARN_LIMIT > ozonelevel ) {  // ゲージ4
    OzoneBgColor = TFT_BLACK;
    OzoneFgColor = TFT_BLACK;
    OzonePnColor = TFT_LIGHTGRAY;
    OzoneTitleColor = TFT_BLUE;
    timeOzone.bgcolor = TFT_BLACK;
    timeOzoneMin.bgcolor = TFT_BLACK;
    timeOzoneMax.bgcolor = TFT_BLACK;
    timeOzoneBar.bgcolor = TFT_BLACK;
    timeState.bgcolor = TFT_LIGHTGRAY;
    // Serial.printf("timeState.bgcolor=%d\n", timeState.bgcolor);
    timeState.fgcolor = TFT_BLACK;
    timetitle.bgcolor = TFT_BLUE;
    timetitle.fgcolor = TFT_WHITE;
    timeHour.bgcolor = TFT_LIGHTGRAY;
    timeMinute.bgcolor = TFT_LIGHTGRAY;
    timeSecond.bgcolor = TFT_LIGHTGRAY;
    timeZikan.bgcolor = TFT_LIGHTGRAY;
    timeFan.bgcolor = TFT_LIGHTGRAY;
    timeByou.bgcolor = TFT_LIGHTGRAY;
    timeHour.fgcolor = TFT_BLACK;
    timeMinute.fgcolor = TFT_BLACK;
    timeSecond.fgcolor = TFT_BLACK;
    timeZikan.fgcolor = TFT_BLACK;
    timeFan.fgcolor = TFT_BLACK;
    timeByou.fgcolor = TFT_BLACK;
  } else {
    OzoneBgColor = TFT_RED;
    OzoneFgColor = TFT_WHITE;
    OzonePnColor = TFT_BLACK;
    OzoneTitleColor = TFT_RED;
    timeOzone.bgcolor = TFT_RED;
    timeOzoneMin.bgcolor = TFT_RED;
    timeOzoneMax.bgcolor = TFT_RED;
    timeOzoneBar.bgcolor = TFT_RED;
    timeState.bgcolor = TFT_BLACK;
    //  Serial.printf("timeState.bgcolor=%d\n", timeState.bgcolor);
    timeState.fgcolor = TFT_WHITE;
    timetitle.bgcolor = TFT_RED;
    timetitle.fgcolor = TFT_WHITE;
    timeHour.bgcolor = TFT_BLACK;
    timeMinute.bgcolor = TFT_BLACK;
    timeSecond.bgcolor = TFT_BLACK;
    timeZikan.bgcolor = TFT_BLACK;
    timeFan.bgcolor = TFT_BLACK;
    timeByou.bgcolor = TFT_BLACK;
    timeHour.fgcolor = TFT_WHITE;
    timeMinute.fgcolor = TFT_WHITE;
    timeSecond.fgcolor = TFT_WHITE;
    timeZikan.fgcolor = TFT_WHITE;
    timeFan.fgcolor = TFT_WHITE;
    timeByou.fgcolor = TFT_WHITE;
  }
}
void ozoneDraw(int ozonelevel) { // オゾンバー描画
  timeOzoneBar.text = "■□□□□□";
  if ( 1 == ozonelevel )
    timeOzoneBar.text = "■■□□□□";
  if ( 2 == ozonelevel )
    timeOzoneBar.text = "■■■□□□";
  if ( 3 == ozonelevel )
    timeOzoneBar.text = "■■■■□□";
  if ( 4 == ozonelevel )
    timeOzoneBar.text = "■■■■■□";
  if ( 5 == ozonelevel )
    timeOzoneBar.text = "■■■■■■";
  labelText(timeOzoneMin);
  labelText(timeOzoneMax);
  labelText(timeOzoneBar);
}

void drawTimeAs(int level) { // 指定時間の描画
  char buf[20];
  int seconds = timeSeconds[level];
  if ( seconds >= 3600 ) {
    sprintf(buf, "%2d", seconds / 3600);
    timeHour.text = String(buf);
    sprintf(buf, "%02d", (seconds / 60) % 60);
    timeMinute.text = String(buf);
    sprintf(buf, "%02d", seconds % 60);
    timeSecond.text = String(buf);
  } else if ( seconds < 3600 && seconds >= 60 ) {
    sprintf(buf, "  ");
    timeHour.text = String(buf);
    sprintf(buf, "%2d", (seconds / 60) % 60);
    timeMinute.text = String(buf);
    sprintf(buf, "%02d", seconds % 60);
    timeSecond.text = String(buf);
  } else if ( seconds < 60 ) {
    sprintf(buf, "  ");
    timeHour.text = String(buf);
    sprintf(buf, "  ");
    timeMinute.text = String(buf);
    sprintf(buf, "%2d", seconds % 60);
    timeSecond.text = String(buf);
  }
  drawTime();
}

void drawTime() { // 時間描画
  /*  Serial.print(mode);
    Serial.println(":h,m,s=");
    Serial.println(timeHour.text);
    Serial.println(timeMinute.text);
    Serial.println(timeSecond.text);
    Serial.println(timeHour.text.compareTo("  "));
  */
  if ( 0 != timeHour.text.compareTo("  "))
  {
    labelText(timeHour);      // 3600秒以上の場合
    labelText(timeZikan);
    Serial.println("zikan drawing...");
  } else {                    // 3600秒未満の場合
    tft.fillRect(timeHour.x1, timeHour.y1, timeHour.xlength, timeHour.ylength, timeHour.bgcolor);
    tft.fillRect(timeZikan.x1, timeZikan.y1, timeZikan.xlength, timeZikan.ylength, timeZikan.bgcolor);
  }

  if ( 0 != timeMinute.text.compareTo("  ")) {
    labelText(timeMinute);
    labelText(timeFan);       // 60秒以上の場合
  } else {
    labelText(timeMinute);    // 60秒未満の場合
    tft.fillRect(timeFan.x1, timeFan.y1, timeFan.xlength, timeFan.ylength, timeFan.bgcolor);
  }
  labelText(timeSecond);
  labelText(timeByou);
}
void drawSetDisp() {         // 描画一式
  tft.fillScreen(OzoneBgColor);
  tft.fillRect(22, 13, 277, 154, OzonePnColor);
  tft.fillRect(35, 25, 211, 30, OzoneTitleColor);
  labelText(timetitle);
  labelText(timeState);
  drawTime();
  labelText(timeOzone);
  labelText(timeOzoneMin);
  labelText(timeOzoneMax);
  labelText(timeOzoneBar);
}
//----------------------------------------------------------------------------------
// 動作モード関係
void start(int ozonelevel ) {
  Serial.printf("Start with Ozonelevel:%d ", ozonelevel);
  Ozonelevel = ozonelevel;
  runOZONE();
  runPUMP();
  log_oncount++;
}

void runPUMP() {
  digitalWrite(PUMP_PIN, HIGH);
  motionPUMP = "ON";
}
void stopPUMP() {
  digitalWrite(PUMP_PIN, LOW);
  motionPUMP = "OFF";
}
void runFAN() {
  digitalWrite(FAN_PIN, HIGH);
  motionFAN = "ON";
}
void stopFAN() {
  digitalWrite(FAN_PIN, LOW);
  motionFAN = "OFF";
}
void runOZONE() {
  motionOZONE = "ON";
}
void stopOZONE() {
  motionOZONE = "OFF";
  digitalWrite(OZONE0_PIN, LOW);
  digitalWrite(OZONE1_PIN, LOW);
  digitalWrite(OZONE2_PIN, LOW);
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

void stop() {
  stopOZONE();

  Ozonelevel = 0;
  mode = "stop";
  int pos2 = encoder2.getPosition();
  tft.fillRect(timeHour.x1, timeHour.y1, timeHour.xlength, timeHour.ylength, timeHour.bgcolor);
  tft.fillRect(timeZikan.x1, timeZikan.y1, timeZikan.xlength, timeZikan.ylength, timeZikan.bgcolor);
  tft.fillRect(timeFan.x1, timeFan.y1, timeFan.xlength, timeFan.ylength, timeFan.bgcolor);
  timeHour.text = "  ";
  timeMinute.text = "  ";
  timeSecond.text = " 0";
  labelText(timeHour);
  labelText(timeMinute);
  labelText(timeSecond);
  peee();
    
  int chkOneshot_Ozonelevel[10];
  int chkOneshot_endtime;
  int chkProgram_Ozonelevel;
  int chkProgram_starttime;
  int chkProgram_endtime;
  int chkLog_pump;
  int chkLog_ozone;
  int chkLog_fan;
  int chkLog_oncount;
  int chkSPIFFSreadcount = 0;
  int chkSPIFFSwritecount = 0;
  int chkSPIFFSexitflag = 0 ; // 1 then exit
  //    File fp ;
  //      labelText(timeZikan);
  //      labelText(timeFan);
  //      labelText(timeByou);
  for ( int i = 0; i < 3; i++)  // ポンプを3秒程度動作させる
  {
    timeHour.fgcolor = TFT_GREEN;
    timeMinute.fgcolor = TFT_GREEN;
    timeSecond.fgcolor = TFT_GREEN;
    timeZikan.fgcolor = TFT_GREEN;
    timeFan.fgcolor = TFT_GREEN;
    timeByou.fgcolor = TFT_GREEN;

    labelText(timeHour);
    labelText(timeMinute);
    labelText(timeSecond);
    //      labelText(timeZikan);
    //      labelText(timeFan);
    labelText(timeByou);
    delay(500);
    ozoneChangeColor( pos2 );
    labelText(timeHour);
    labelText(timeMinute);
    labelText(timeSecond);
    //      labelText(timeZikan);
    //      labelText(timeFan);
    labelText(timeByou);
    delay(500);
  }

  /*


    // SPIFFS操作のためにタイマーを停止
    timerAlarmDisable(timer);    // stop alarm
    timerDetachInterrupt(timer);  // detach interrupt
    timerEnd(timer);      // end timer
    Serial.println("tiner stop");
    // SPIFFS操作のためにSerialを停止
    Serial.end();

    while (  0 == chkSPIFFSexitflag)
    {

    bool res = SPIFFS.begin(true); // FORMAT_SPIFFS_IF_FAILED
    Serial.print("SPIFFS.begining...");
    if (!res) {
    // Serial.println("fail");
    handleFatalError("ファイルシステムを開始できません");
    }
    //Serial.println("OK");


    fp = SPIFFS.open(recordfile, "w");
    if (!fp) {
    //  Serial.println("Write Opening recordfile failed");
      handleFatalError("ファイルに記録できません");
    } else {
      delay(10);
      log_ozone = log_ozone10 / 10;
      //  fp.println(12345);
      fp.println(oneshot_Ozonelevel); // dummy
      fp.println(oneshot_Ozonelevel); // dummy
      fp.println(oneshot_Ozonelevel); // dummy
      fp.println(oneshot_Ozonelevel); // dummy
      fp.println(oneshot_Ozonelevel); // dummy
      fp.println(oneshot_Ozonelevel); // dummy
      fp.println(oneshot_Ozonelevel); // dummy
      fp.println(oneshot_Ozonelevel); // dummy
      fp.println(oneshot_Ozonelevel); // dummy
      delay(10);
      fp.println(oneshot_Ozonelevel);
      delay(10);
      fp.println(oneshot_endtime);
      delay(10);
      fp.println(program_Ozonelevel);
      delay(10);
      fp.println(program_starttime);
      delay(10);
      fp.println(program_endtime);
      delay(10);
      fp.println(log_pump);
      delay(10);
      fp.println(log_ozone);
      delay(10);
      fp.println(log_fan);
      delay(10);
      fp.println(log_oncount);
      fp.close();
    }
    fp = SPIFFS.open(recordfile, "r");
    if (!fp) {
      SPIFFS.end();
      // Serial起動
      Serial.begin(115200);
      delay(100);
      Serial.println("Opening recordfile failed");
      handleFatalError("ファイルに記録できません");
    } else {
      int temp;
      temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) chkOneshot_Ozonelevel[0] = temp; // dummy
      temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) chkOneshot_Ozonelevel[1] = temp; // dummy
      temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) chkOneshot_Ozonelevel[2] = temp; // dummy
      temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) chkOneshot_Ozonelevel[3] = temp; // dummy
      temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) chkOneshot_Ozonelevel[4] = temp; // dummy
      temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) chkOneshot_Ozonelevel[5] = temp; // dummy
      temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) chkOneshot_Ozonelevel[6] = temp; // dummy
      temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) chkOneshot_Ozonelevel[7] = temp; // dummy
      temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) chkOneshot_Ozonelevel[8] = temp; // dummy
      temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) chkOneshot_Ozonelevel[9] = temp;
      temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) chkOneshot_endtime = temp;
      temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) chkProgram_Ozonelevel = temp;
      temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) chkProgram_starttime = temp;
      temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) chkProgram_endtime = temp;
      temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) chkLog_pump = temp;
      temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) chkLog_ozone = temp;
      temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) chkLog_fan = temp;
      temp = fp.readStringUntil('\n').toInt(); if ( 0 != temp ) chkLog_oncount = temp;
      fp.close();
      SPIFFS.end();
      // Serial起動
      Serial.begin(115200);
      delay(100);
      verifySPIFFSData( oneshot_Ozonelevel, chkOneshot_Ozonelevel[0]  ,-8);
      verifySPIFFSData( oneshot_Ozonelevel, chkOneshot_Ozonelevel[1]  ,-7);
      verifySPIFFSData( oneshot_Ozonelevel, chkOneshot_Ozonelevel[2]  ,-6);
      verifySPIFFSData( oneshot_Ozonelevel, chkOneshot_Ozonelevel[3]  ,-5);
      verifySPIFFSData( oneshot_Ozonelevel, chkOneshot_Ozonelevel[4]  ,-4);
      verifySPIFFSData( oneshot_Ozonelevel, chkOneshot_Ozonelevel[5]  ,-3);
      verifySPIFFSData( oneshot_Ozonelevel, chkOneshot_Ozonelevel[6]  ,-2);
      verifySPIFFSData( oneshot_Ozonelevel, chkOneshot_Ozonelevel[7]  ,-1);
      verifySPIFFSData( oneshot_Ozonelevel, chkOneshot_Ozonelevel[8]  ,0);
      Serial.println();
      chkSPIFFSreadcount += verifySPIFFSData( oneshot_Ozonelevel, chkOneshot_Ozonelevel[9]  ,1);
      chkSPIFFSreadcount += verifySPIFFSData( oneshot_endtime, chkOneshot_endtime  , 2);
      chkSPIFFSreadcount += verifySPIFFSData( program_Ozonelevel, chkProgram_Ozonelevel  ,3);
      chkSPIFFSreadcount += verifySPIFFSData( program_starttime, chkProgram_starttime  ,4);
      chkSPIFFSreadcount += verifySPIFFSData( program_endtime, chkProgram_endtime  , 5);
      chkSPIFFSreadcount += verifySPIFFSData( log_pump, chkLog_pump  , 6);
      chkSPIFFSreadcount += verifySPIFFSData( log_ozone, chkLog_ozone  , 7);
      chkSPIFFSreadcount += verifySPIFFSData( log_fan, chkLog_fan  , 8);
      chkSPIFFSreadcount += verifySPIFFSData( log_oncount, chkLog_oncount  , 9);

    }
    if ( 0 == chkSPIFFSreadcount  ) {
      chkSPIFFSexitflag = 1; // exit
    } else {
      Serial.printf("SPIFFS verify error count %d\n",++chkSPIFFSwritecount);
      if ( 8 < chkSPIFFSwritecount ){
            handleFatalError("ファイルに保存できません");
        }
      delay(1000);
    }
    }
    Serial.printf("saved to flash: %d %d %d %d %d %d %d %d %d\n", oneshot_Ozonelevel, oneshot_endtime, program_Ozonelevel, program_starttime, program_endtime, log_pump, log_ozone, log_fan, log_oncount);
    // タイマー再稼働
    timer = timerBegin(0, 80, true);
    timerAttachInterrupt(timer, &onTimer, true);
    timerAlarmWrite(timer, 20000, true);  // 20m sec
    timerAlarmEnable(timer);
    Serial.println("tiner setted again");
  */
  eeprom_write();
  if ( 0 != eeprom_verify())
    Serial.printf("error EEPROM\n");

  stopPUMP();
  stopFAN();
}

void eeprom_write() {
  // EEPROM操作のためにタイマーを停止
  timerAlarmDisable(timer);    // stop alarm
  timerDetachInterrupt(timer);  // detach interrupt
  timerEnd(timer);      // end timer
  // EEPROM操作のために割り込みを停止
  detachInterrupt(ENCODER1_A_PIN);
  detachInterrupt(ENCODER1_B_PIN);
  detachInterrupt(ENCODER1_SWITCH_PIN);
  detachInterrupt(ENCODER2_A_PIN);
  detachInterrupt(ENCODER2_B_PIN);   // rotary_encoder
  detachInterrupt(ENCODER2_SWITCH_PIN);  // rotary_encoder push switch

  int data[10];
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
  data[10] = log_OZONE0;
  data[11] = log_OZONE1;
  data[12] = log_OZONE2;
  int n = 0;
  for (int i = 0; i < 10; i++) {
    EEPROM.put(n, data[i]);
    n += 4; // 4バイト毎
  }
  delay(100);
  EEPROM.commit(); // EEPROMに書き込み確定

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

}
int eeprom_verify() {
  int chkEEPROMerrorcount = 0;
  int n = 0;
  int data[10];
  for (int i = 0; i < 10; i++) {
    EEPROM.get(n, data[i]); // EEPROMより読み込み
    n += 4; // 4バイト毎
  }
  if ( maintenance_Count != data[0]) {
    Serial.printf("error 1 as %d\n", data[0]);
    chkEEPROMerrorcount++;
  }
  if ( oneshot_Ozonelevel != data[1]) {
    Serial.printf("error 1 as %d\n", data[1]);
    chkEEPROMerrorcount++;
  }
  if ( oneshot_endtime != data[2]) {
    Serial.printf("error 1 as %d\n", data[2]);
    chkEEPROMerrorcount++;
  }
  if ( program_Ozonelevel != data[3]) {
    Serial.printf("error 1 as %d\n", data[3]);
    chkEEPROMerrorcount++;
  }
  if ( program_starttime != data[4]) {
    Serial.printf("error 1 as %d\n", data[4]);
    chkEEPROMerrorcount++;
  }
  if ( program_endtime != data[5]) {
    Serial.printf("error 1 as %d\n", data[5]);
    chkEEPROMerrorcount++;
  }
  if ( log_pump != data[6]) {
    Serial.printf("error 1 as %d\n", data[6]);
    chkEEPROMerrorcount++;
  }
  if ( log_ozone != data[7]) {
    Serial.printf("error 1 as %d\n", data[7]);
    chkEEPROMerrorcount++;
  }
  if ( log_fan != data[8]) {
    Serial.printf("error 1 as %d\n", data[8]);
    chkEEPROMerrorcount++;
  }
  if ( log_oncount != data[9]) {
    Serial.printf("error 1 as %d\n", data[9]);
    chkEEPROMerrorcount++;
  }
  if ( log_OZONE0 != data[10]) {
    Serial.printf("error 1 as %d\n", data[10]);
    chkEEPROMerrorcount++;
  }
  if ( log_OZONE1 != data[11]) {
    Serial.printf("error 1 as %d\n", data[11]);
    chkEEPROMerrorcount++;
  }
  if ( log_OZONE2 != data[12]) {
    Serial.printf("error 1 as %d\n", data[12]);
    chkEEPROMerrorcount++;
  }
  return ( chkEEPROMerrorcount++);
}
void eeprom_read() {
  int n = 0;
  int data[10];
  for (int i = 0; i < 10; i++) {
    EEPROM.get(n, data[i]); // EEPROMより読み込み
    n += 4; // 4バイト毎
  }
  if ( -1 != data[0] ) {
    if ( 0 == data[0] ) {  // eeprom data version up
      log_OZONE0 = data[7] / 3;
      log_OZONE1 = data[7] / 3;
      log_OZONE2 = data[7] / 3;
    } else {
      maintenance_Count = data[0];
      if ( -1 != data[10] ) log_OZONE0 = data[10];
      if ( -1 != data[11] ) log_OZONE1 = data[11];
      if ( -1 != data[12] ) log_OZONE2 = data[12];

    }
  }
  if ( -1 != data[1] ) oneshot_Ozonelevel = data[1];
  if ( -1 != data[2] ) oneshot_endtime = data[2];
  if ( -1 != data[3] ) program_Ozonelevel = data[3];
  if ( -1 != data[4] ) program_starttime = data[4];
  if ( -1 != data[5] ) program_endtime = data[5];
  if ( -1 != data[6] ) log_pump = data[6];
  if ( -1 != data[7] ) log_ozone = data[7];
  if ( -1 != data[8] ) log_fan = data[8];
  if ( -1 != data[9] ) log_oncount = data[9];
  if ( -1 == data[0] )  Serial.println("EEPROM seems first use!");
  log_ozone10 = log_ozone * 10;
}

int getTwiceTimeLevel(int seconds) {  // timeLevel の
  for ( int i = 0; i < WAITTIMEMAX + 1 ; i++ ) {
    if ( seconds == timeSeconds[i] )
    {
      return (i * 2); // 配列値と同じ場合は偶数が返る
    }
    if ( seconds < timeSeconds[i] )
    {
      return (i * 2 - 1); // 配列の間に入っている場合は奇数が返る
    }
  }
}

//----------------------------------------------------------------------------------


// 作動までのカウントダウン
int programmode1(int waittimelevel, int endtimelevel, int ozonelevel) {
  int startpos1 = waittimelevel;
  int pos1 = waittimelevel;
  int newPos1 = waittimelevel;
  int startpos2 = endtimelevel;
  int pos2 = ozonelevel;
  int newPos2 = ozonelevel;
  int exitflag = 0;
  int selected;

  timeState.text = "作動まで";
  drawSetDisp();
  runRemain = timeSeconds[waittimelevel];
  mode = "programmode1";
  encoder1.setPosition(newPos1);
  encoder2.setPosition(newPos2);

  // start(newPos2);
  while (1) {
    newPos1 = encoder1.getPosition();
    newPos2 = encoder2.getPosition();


    if ( pos2 != newPos2 ) {
      newPos2 = ozoneRun(newPos2, pos2);
      // newPos2 = encoder2.getPosition();
      pos2 = newPos2;
    }

    if (  pos1 != newPos1) {  // ダイヤル操作

      mode = "programmode1resetting";
      int returncode = resetting(runRemain, newPos2, "作動まで", WAITTIMEMAX); //  待ち時間を再設定
      if ( 1 == returncode )   // 時間などが再設定された場合
      {
        pi();
        Serial.print("mode=programmode1resetting exit 1 : endtimeseconds=");       // time modifiedの場合
        int temppos1 = encoder1.getPosition();   // 設定確定まで暫定設定値を保存
        pos2 = program_Ozonelevel;
        Serial.println(timeSeconds[endtimelevel]);
        returncode = resetting(timeSeconds[endtimelevel], pos2, "稼働時間", TIMEMAX); //  稼働時間を再設定
        if ( 1 == returncode )   // 時間などが再設定された場合
        {
          peee();
          pos2 = Ozonelevel;
          endtimelevel = encoder1.getPosition();
          runRemain = timeSeconds[temppos1]; // 半端値は失われる
        }
      } else {
        Serial.println("mode=programmode1resetting exit without 1 ");  // timeoutの場合
      }
      mode = "programmode1";
      // 描画を元に戻す
      timeState.text = "作動まで";
      newPos1 = pos1;
      newPos2 = pos2;
      encoder1.setPosition(pos1);
      encoder2.setPosition(pos2);
      ozoneChangeColor( pos2 );
      drawSetDisp();
      setTimeColor(pos2);
      ozoneDraw(pos2);
    }
    if ( 1 == encoder1Pushed || 1 == encoder2Pushed ) // プッシュ動作
    {
      if (1 == encoder2Pushed )
      {
        encoder1Pushed = 0;
        encoder2Pushed = 0;
        //            oneshot_Ozonelevel = pos2;
        mode = "";
        ozoneChangeColor( pos2 );
        labelText(timeState);  // 点滅を元に戻す

        stop();
        return (2);
      }
    }

    if ( 0 == runRemain )
    {
      encoder1Pushed = 0;
      // stop();
      peee();
      return (0);
    }
    int oldTimeStateColor;
    if ( oldTimeStateColor != timeState.fgcolor )
    {
      drawTime();   // 点滅
      labelText(timeState);  // 点滅
    }
    oldTimeStateColor = timeState.fgcolor;
    delay(FRAMERATEDELAY);
  }
}
// プログラム動作中
int programmode2(int endtimelevel) {
  int startpos1 = endtimelevel;
  int pos1 = startpos1;
  int newPos1 = endtimelevel;
  int startpos2 = encoder2.getPosition();
  int pos2 = startpos2;
  int newPos2 = encoder2.getPosition();
  timeState.text = "稼働時間";
  drawSetDisp();
  runRemain = timeSeconds[endtimelevel];
  mode = "programmode2";
  Serial.println("entered programmode2------------");
  encoder1.setPosition(newPos1);
  encoder2.setPosition(newPos2);
  // delay(800);
  start(newPos2);  // start with Ozone percent
  while (1) {
    newPos1 = encoder1.getPosition();
    newPos2 = encoder2.getPosition();
    if ( pos2 != newPos2 ) {
      newPos2 = ozoneRun(newPos2, pos2);
      //  newPos2 = encoder2.getPosition();
      pos2 = newPos2;
    }

    if (  pos1 != newPos1  ) {  // ダイヤル操作

      mode = "programmode2resetting";

      pos2 = program_Ozonelevel;
      Serial.println(timeSeconds[endtimelevel]);
      pi();
      int returncode = resetting(timeSeconds[endtimelevel], pos2, "稼働時間", TIMEMAX); //  稼働時間を再設定
      if ( 1 == returncode )   // 時間などが再設定された場合
      {
        peee();
        pos2 = Ozonelevel;
        endtimelevel = encoder1.getPosition();
        runRemain = timeSeconds[endtimelevel]; // 半端値は失われる
      }
      mode = "programmode2";
      // 描画を元に戻す
      timeState.text = "稼働時間";
      newPos1 = pos1;
      newPos2 = pos2;
      encoder1.setPosition(pos1);
      encoder2.setPosition(pos2);
      ozoneChangeColor( pos2 );
      drawSetDisp();
      setTimeColor(pos2);
      ozoneDraw(pos2);
    }
    if ( 1 == encoder1Pushed || 1 == encoder2Pushed )
    { // exit
      //    if (1 == encoder1Pushed )
      //    {
      //       encoder1Pushed=0;
      //       stop();
      //       return (1);
      //    }
      if (1 == encoder2Pushed )
      {
        encoder1Pushed = 0;
        encoder2Pushed = 0;
        //                oneshot_Ozonelevel = pos2;
        mode = "";
        ozoneChangeColor( pos2 );
        labelText(timeState);  // 点滅を元に戻す

        stop();
        return (2);
      }
    }

    if ( 0 == runRemain )
    {
      encoder1Pushed = 0;
      //              oneshot_Ozonelevel = pos2;
      mode = "";
      ozoneChangeColor( pos2 );
      labelText(timeState);  // 点滅を元に戻す
      stop();
      return (0);
    }
    int oldTimeStateColor;
    if ( oldTimeStateColor != timeState.fgcolor )
    {
      drawTime();   // 点滅
      labelText(timeState);  // 点滅
    }
    oldTimeStateColor = timeState.fgcolor;
    delay(FRAMERATEDELAY);
  }
}


//-----------------------------------------------------------------------------
// 予約動作設定モード

int setonstart() {
  mode = "onstart";
  int startpos1 = program_starttime;
  int pos1 = startpos1 - 1 ; // 異なる値を与えることで強制的に画面描画する
  int newPos1 = startpos1;
  int startpos2 = program_Ozonelevel;
  int pos2 = startpos2 - 1; // 異なる値を与えることで強制的に画面描画する
  int newPos2 = startpos2;
  encoder1.setPosition(startpos1);
  encoder2.setPosition(startpos2);

  timetitle.text = "ONタイマーモード";
  timeState.text = "作動まで";
  ozoneChangeColor( pos2 );
  drawSetDisp();
  Serial.println("mode=ontimer");
  while ( 1 )
  {
    newPos1 = encoder1.getPosition();
    newPos2 = encoder2.getPosition();
    if (pos2 != newPos2) {
      if ( OZONEMAX < newPos2 ) {
        newPos2 = OZONEMAX;
        encoder2.setPosition(OZONEMAX);
      }
      if ( 0 >  newPos2 ) {
        newPos2 = 0;
        encoder2.setPosition(0);
      }
      if (
        ( pos2 >= Ozone_WARN_LIMIT && newPos2 <  Ozone_WARN_LIMIT )
        ||
        ( pos2 < Ozone_WARN_LIMIT && newPos2 >=  Ozone_WARN_LIMIT )
      )
      { // ゲージ4
        ozoneChangeColor(newPos2);
        drawSetDisp();
      }
      pos2 = newPos2;
      ozoneDraw(newPos2);      // オゾン濃度表示部分
    }
    // time setting
    if (pos1 != newPos1) {
      Serial.print("pos1,newPos1,newPos1:  ");
      Serial.print(pos1);
      Serial.print("  ");
      Serial.print(newPos1);
      if ( 0 >  newPos1 ) {
        newPos1 = 0;
        encoder1.setPosition(0);
      }
      if ( WAITTIMEMAX < newPos1 ) {
        newPos1 = WAITTIMEMAX;
        encoder1.setPosition(newPos1);
      }
      Serial.print("  ");
      Serial.println(newPos1);
      char buf[20];
      drawTimeAs(newPos1);  // 時間表示部分
      pos1 = newPos1;
      program_starttime = pos1;
    }
    if ( 1 == encoder1Pushed || 1 == encoder2Pushed )
    { // exit

      if (1 == encoder1Pushed && 0 == encoder1Pushing )
      {
        encoder1Pushed = 0;
        pi();
        setonend(newPos1, newPos2); // newPos1(待ち時間)は変更しないがprogrammode1のために渡す, Ozonelevelはsetonendでも変更
        // 表示を元に戻す
        timeState.text = "作動まで";
        Serial.printf("setonstart-newPos1:%d\n", newPos1);
        encoder1.setPosition(newPos1);
        drawTimeAs(newPos1);  // 時間表示部分
        ozoneChangeColor(newPos2);
        drawSetDisp();
        pos2 = newPos2;
        ozoneDraw(newPos2);
      }
      if (1 == encoder2Pushed && 0 == encoder2Pushing)
      {
        encoder2Pushed = 0;
        pipi();
        return (0);
      }
    }
    delay(FRAMERATEDELAY);
  }
}

int setonend(int waittime, int ozonelevel) { // waittime ... programmode1のために待ち時間をtimelevelで渡される
  mode = "setonend";
  int startpos1 = program_endtime;
  int pos1 = startpos1 - 1 ; // 異なる値を与えることで強制的に画面描画する
  int newPos1 = startpos1;
  int startpos2 = ozonelevel;
  int pos2 = startpos2 - 1 ; // 異なる値を与えることで強制的に画面描画する
  int newPos2 = startpos2;

  encoder1.setPosition(startpos1);
  encoder2.setPosition(startpos2);

  Serial.println("mode=ontimer");
  //timetitle.fgcolor=TFT_BLACK;
  //tft.fillScreen(TFT_WHITE);
  timeState.text = "稼働時間";
  drawSetDisp();
  while ( 1 )
  {

    newPos1 = encoder1.getPosition();
    newPos2 = encoder2.getPosition();
    if (pos2 != newPos2) {
      if ( OZONEMAX < newPos2 ) {
        newPos2 = OZONEMAX;
        encoder2.setPosition(OZONEMAX);
      }
      if ( 0 >  newPos2 ) {
        newPos2 = 0;
        encoder2.setPosition(0);
      }
      if (
        ( pos2 >= Ozone_WARN_LIMIT && newPos2 <  Ozone_WARN_LIMIT )
        ||
        ( pos2 < Ozone_WARN_LIMIT && newPos2 >=  Ozone_WARN_LIMIT )
      )
      { // ゲージ4
        ozoneChangeColor(newPos2);
        drawSetDisp();
      }
      pos2 = newPos2;
      ozoneDraw(newPos2);
      delay(FRAMERATEDELAY);
    }
    // time setting
    if (pos1 != newPos1) {
      Serial.print(pos1);
      Serial.print("  ");
      Serial.print(newPos1);
      if ( 0 >  newPos1 ) {
        newPos1 = 0;
        encoder1.setPosition(0);
      }
      if ( TIMEMAX < newPos1 ) {
        newPos1 = TIMEMAX;
        encoder1.setPosition(newPos1);
      }
      Serial.print("  ");
      Serial.println(newPos1);

      drawTimeAs(newPos1);
      pos1 = newPos1;
    }

    if ( 1 == encoder1Pushed || 1 == encoder2Pushed )
    { // exit

      if (1 == encoder1Pushed && 0 == encoder1Pushing )
      {
        Serial.print("trigger on");
        encoder1Pushed = 0;
        Serial.print(" waittime:");
        Serial.print(waittime);
        Serial.print(" newPos1:");
        Serial.print(newPos1);
        Serial.print(" newPos2:");
        Serial.println(newPos2);
        // EEPROM保存用変数をセット
        program_Ozonelevel = newPos2;
        program_starttime = waittime;
        program_endtime = newPos1;
        peee();
        if ( 0 == programmode1(waittime, newPos1, newPos2)) // 待ち時間(単位:timelevel), 稼働時間(単位:timelevel), Ozonelevel
        { // もし待ち時間がゼロになっていれば
          //   peee();
          programmode2(newPos1);
          return (1);
        } else {
          //  peee();
          return (1);
        }
        encoder1.setPosition(newPos1);
        timeState.text = "稼働時間";
        mode = "setonend";
        drawSetDisp();
      }
      if (1 == encoder2Pushed && 0 == encoder2Pushing)
      {
        encoder2Pushed = 0;
        pi();
        return (2);   // cancel
      }
    }

    delay(FRAMERATEDELAY);
  }
}


//-------------------------------------------------------------------------
// 通常動作(オンデマンド動作)モード


int set() {
  mode = "set";
  static int timemin = 30;
  int pos1 = oneshot_endtime + 1; // 初期値と異なる値を与えることで強制的に画面描画する
  int pos2 = oneshot_Ozonelevel + 1; // 初期値と異なる値を与えることで強制的に画面描画する
  encoder1.setPosition(oneshot_endtime);
  encoder2.setPosition(oneshot_Ozonelevel);
  int newPos1 = oneshot_endtime;
  int newPos2 = oneshot_Ozonelevel;
  int exitflag = 0;
  int selected;
  timetitle.text = "通常モード";
  timeState.text = "稼働時間";
  ozoneChangeColor(newPos2);
  tft.fillScreen(OzonePnColor); // 画面のちらつきを防止するために先に時間だけ書いておく
  drawTimeAs(newPos1);  // 時間表示部分 時間の文字列を作成 副作用で画面に出力される
  drawSetDisp();        // 画面表示完成
  // Serial.println("enter set");
  //  Serial.printf("pos1 %d newPos1 %d pos2 %d newPos2 %d\n", pos1, newPos1, pos2, newPos2);
  // delay(5000);
  Serial.println("mode=set");
  while ( 1 )
  {
    //   Serial.println("enter set while1");
    //   Serial.printf("pos1 %d newPos1 %d pos2 %d newPos2 %d\n", pos1, newPos1, pos2, newPos2);
    newPos1 = encoder1.getPosition();
    newPos2 = encoder2.getPosition();
    //   Serial.println("enter set while2");
    //   Serial.printf("pos1 %d newPos1 %d pos2 %d newPos2 %d\n", pos1, newPos1, pos2, newPos2);
    // time setting
    if (pos1 != newPos1) {
      //     Serial.print(pos1);
      //     Serial.print("  ");
      //     Serial.print(newPos1);
      if ( 0 >  newPos1 ) {
        newPos1 = 0;
        encoder1.setPosition(0);
      }
      if ( TIMEMAX < newPos1 ) {
        newPos1 = TIMEMAX;
        encoder1.setPosition(newPos1);
      }
      drawTimeAs(newPos1);  // 時間表示部分
      pos1 = newPos1;
    }
    if (pos2 != newPos2) {
      if ( OZONEMAX < newPos2 ) {
        newPos2 = OZONEMAX;
        encoder2.setPosition(OZONEMAX);
      }
      if ( 0 >  newPos2 ) {
        newPos2 = 0;
        encoder2.setPosition(0);
      }
      if (
        ( pos2 >= Ozone_WARN_LIMIT && newPos2 <   Ozone_WARN_LIMIT )
        ||
        ( pos2 <  Ozone_WARN_LIMIT && newPos2 >=  Ozone_WARN_LIMIT )
      )
      { // ゲージ4
        ozoneChangeColor(newPos2);
        drawSetDisp();
      }
      pos2 = newPos2;
      oneshot_Ozonelevel = pos2;
      ozoneDraw(newPos2);      // オゾン濃度表示部分
    }

    if ( 1 == encoder1Pushed || 1 == encoder2Pushed )
    { // exit

      if (1 == encoder1Pushed && 0 == encoder1Pushing )
      { // 確定
        delay(FRAMERATEDELAY); // suppless chattling
        oneshot_endtime = newPos1;
        oneshot_Ozonelevel = newPos2;
        encoder1Pushed = 0;
        Serial.printf("exit from set with newPos2:%d ", newPos2);
        pipi();
        return (1);
      }
      if (1 == encoder2Pushed && 0 == encoder2Pushing)
      { // キャンセル
        //        delay(100); // suppless chattling
        pipi();
        encoder2Pushed = 0;
        return (2);
      }
    }
    delay(FRAMERATEDELAY);
  }
}

int run() {
  int pos1  = encoder1.getPosition();
  int newPos1;
  int pos2 = encoder2.getPosition();
  int newPos2;
  int exitflag = 0;
  int selected;
  int timelevel;
  timeState.text = "残り時間";
  // Ozonelevel = oneshot_Ozonelevel;
  drawSetDisp();
  runRemain = timeSeconds[encoder1.getPosition()];
  mode = "run";
  start(pos2);  // start with Ozone percent
  while (1) {
    newPos1 = encoder1.getPosition();
    newPos2 = encoder2.getPosition();
    if ( pos2 != newPos2 ) {
      newPos2 = ozoneRun(newPos2, pos2);
      // newPos2 = encoder2.getPosition();
      pos2 = newPos2;
    }
    if ( pos1 != newPos1 ) {
      mode = "resetting";
      pi();
      int returncode = resetting(runRemain, newPos2, "稼働時間", TIMEMAX); //  時間再設定 runRemain は秒指定
      mode = "run";
      timeState.text = "残り時間";               //  元に戻す
      if ( 1 == returncode )                    //  時間が再設定された場合
      {
        peee();
        Serial.println("mode=resetting exit 1");
        pos2 = Ozonelevel;
        //  pos1 = runRemain;
      } else {                                  // 時間が再設定されずキャンセルになった場合
        Serial.println("mode=resetting exit without 1 ");
      }
      newPos1 = pos1;
      //  newPos2 = pos2;
      encoder1.setPosition(pos1);
      //  encoder2.setPosition(pos2);
      ozoneChangeColor( Ozonelevel );
      drawSetDisp();
      //  ozoneDraw(pos2);
      pos2 = Ozonelevel;
    }
    if ( 1 == encoder1Pushed || 1 == encoder2Pushed )
    {
      if (1 == encoder2Pushed )
      {
        encoder1Pushed = 0;
        encoder2Pushed = 0;
        oneshot_Ozonelevel = pos2;
        mode = "";
        ozoneChangeColor( pos2 );
        labelText(timeState);  // 点滅を元に戻す
        stop();
        return (2);
      }
    }

    if ( 0 == runRemain )
    {
      encoder1Pushed = 0;
      oneshot_Ozonelevel = pos2;
      mode = "";
      ozoneChangeColor( pos2 );
      labelText(timeState);  // 点滅を元に戻す
      stop();
      return (0);
    }
    int oldTimeStateColor;
    if ( oldTimeStateColor != timeState.fgcolor );
    {
      drawTime();   // 点滅
      labelText(timeState);  // 点滅
    }
    oldTimeStateColor = timeState.fgcolor;
    delay(FRAMERATEDELAY);
  }
}
int resetting(int timesecond, int ozonelevel, char *statetext, int maxTimeLevel ) {
  // 返り値 0..timeout 1..更新 2..キャンセル
  // 設定値は timelevel および ozonelevel , それぞれ encoder1.getPosition() encoder2.getPosition() に入る
  unsigned long start = micros();
  static int timemin = 30;
  int pos1 = 100;
  int newPos1;
  int pos2 = ozonelevel + 1;
  int newPos2;
  int exitflag = 0;
  int selected;
  Ozonedisplay = ozonelevel;
  Ozonelevel = ozonelevel;
  timeState.text = statetext;
  ozoneChangeColor( Ozonedisplay );
  drawSetDisp();
  //  settingRemain = runRemain;
  autoExitCounter = 5;
  Serial.print("in the resetting: timesecond:");
  Serial.print(timesecond);
  Serial.print(" altertimelevel:");
  int altertimelevel = getTwiceTimeLevel(timesecond);  // レベル*2 ないし レベル*2+1 が返る
  Serial.print(altertimelevel);
  encoder1.setPosition(100 + 1); // 初期値はレンジ外に置いて、timeSeconds配列以外の値を表す
  // newPos1とは異なる値を与えて最初に強制的に描画させる
  newPos1 = 100;
  Serial.print(" ozonelevel:");
  Serial.print(ozonelevel);
  Serial.print(" runRemain:");
  Serial.print(runRemain);
  Serial.print(" altertimelevel:");
  Serial.println(altertimelevel);
  settingRemain = timesecond;

  pos1 = 100;
  while ( 1 )
  {
    newPos1 = encoder1.getPosition();
    newPos2 = encoder2.getPosition();
    // Ozone percent setting

    if ( pos2 != newPos2 ) {
      newPos2 = ozoneRun(newPos2, pos2);
      // newPos2 = encoder2.getPosition();
      pos2 = newPos2;
    }

    // time setting
    if (pos1 != newPos1) {
      if ( 100 == pos1 ) { // 1回目はtimeSeconds配列の間にあるための処理
        Serial.print(" first altertimelevel : ");
        Serial.print(altertimelevel);
        Serial.print(" pos1 ");
        Serial.print(pos1);
        Serial.print(" newPos1 ");
        Serial.print(newPos1);
        Serial.print(" -> ");
        newPos1 = ( altertimelevel + ( newPos1 - pos1 )) / 2;
        Serial.println(newPos1);
        encoder1.setPosition(newPos1);
      }
      autoExitCounter = 5;
      Serial.print(" pos1  ");
      Serial.print(pos1);
      Serial.print("  ");
      Serial.print(newPos1);
      if ( 0 >  newPos1 ) {
        newPos1 = 0;
        encoder1.setPosition(0);
      }
      if ( maxTimeLevel < newPos1 ) {
        newPos1 = maxTimeLevel;
        encoder1.setPosition(newPos1);
      }
      Serial.print("  ");
      Serial.println(newPos1);
      char buf[20];
      drawTimeAs(newPos1);
      pos1 = newPos1;
      Serial.print("  ");
      Serial.println(newPos1);
      settingRemain = timeSeconds[newPos1];
    }
    if ( 1 == encoder1Pushed || 1 == encoder2Pushed )
    { // exit
      if (1 == encoder1Pushed )
      {
        //     Ozonelevel = Ozonedisplay;
        runRemain = settingRemain;
        encoder1Pushed = 0;
        return (1); // set done
      }
      if (1 == encoder2Pushed )
      {
        pi();
        encoder2Pushed = 0;
        return (2); // cancel
      }
    }
    if ( 0 == runRemain )
    {
      pi();
      return (0); // auto cancel
    }
    if ( 0 == autoExitCounter)
    {
      pi();
      return (0); // auto cancel
    }
    int oldTimeStateColor;
    if ( oldTimeStateColor != timeState.fgcolor )
    {
      drawTimeAs(newPos1);   // 点滅
      labelText(timeState);  // 点滅
    }
    oldTimeStateColor = timeState.fgcolor;
    delay(FRAMERATEDELAY);
  }
}
//-----------------------------------------------------------------------
// メンテナンス動作モード
void maintenancemode2() {
  Serial.println("enter maintenancemode2");
  delay(1000);
  Serial.println("enter maintenancemode2-2");

  tft.fillScreen(TFT_BLACK);
  labelText(m2title1);
  labelText(m2version);
  labelText(m210);
  labelText(m211);
  labelText(m212);
  labelText(m213);
  labelText(m214);
  labelText(m215);
  labelText(m216);
  labelText(m217);
  labelText(md210);
  labelText(md211);
  labelText(md212);
  labelText(md213);
  labelText(md214);
  labelText(md215);
  // labelText(md216);
  labelText(md217);
  int pos1 = 100;
  encoder1.setPosition(0);
  maintenanceSelect2();

}

void maintenancemode1() {

  tft.fillScreen(TFT_BLACK);
  labelText(m1title1);
  labelText(m1title2);
  labelText(m1version);
  labelText(m110);
  labelText(m111);
  labelText(m112);
  labelText(m113);
  md110.text = mkTimeString(log_pump);
  md111.text = String(mkTimeString(log_OZONE0)+mkTimeString(log_OZONE1)+mkTimeString(log_OZONE2));
  md112.text = mkTimeString(log_fan);
  char buf[20];
  sprintf(buf, "%d", log_oncount);
  md113.text = String(buf);
  labelText(m120);
  labelText(m121);
  labelText(m122);
  labelText(m123);
  labelText(m124);
  labelText(md110);
  labelText(md111);
  labelText(md112);
  labelText(md113);
  labelText(md120);
  labelText(md121);
  labelText(md122);
  labelText(md123);
  labelText(md124);

  int pos1 = 100;
  int newPos1;
  int newPos2;
  encoder1.setPosition(0);
  while (1) {
    newPos1 = encoder1.getPosition();
    newPos2 = encoder2.getPosition();

    if (pos1 != newPos1) {
      int i = newPos1 % 5;
      md120.fgcolor = TFT_WHITE;
      md121.fgcolor = TFT_WHITE;
      md122.fgcolor = TFT_WHITE;
      md123.fgcolor = TFT_WHITE;
      md124.fgcolor = TFT_WHITE;
      md120.bgcolor = TFT_BLACK;
      md121.bgcolor = TFT_BLACK;
      md122.bgcolor = TFT_BLACK;
      md123.bgcolor = TFT_BLACK;
      md124.bgcolor = TFT_BLACK;
      if ( 0 == i ) {
        md120.fgcolor = TFT_BLACK;
        md120.bgcolor = TFT_WHITE;
      }
      if ( 1 == i ) {
        md120.fgcolor = TFT_BLACK;
        md120.bgcolor = TFT_WHITE;
      }
      if ( 2 == i ) {
        md120.fgcolor = TFT_BLACK;
        md120.bgcolor = TFT_WHITE;
      }
      if ( 3 == i ) {
        md120.fgcolor = TFT_BLACK;
        md120.bgcolor = TFT_WHITE;
      }
      if (1 == encoder1Pushed )
      {
        encoder1Pushed = 0;
      }

      pos1 = newPos1;
    }


    delay(FRAMERATEDELAY);
    if (2 == maintenanceSelect1() )
    {

      Serial.println("return to maintenancemode1");
      encoder2Pushed = 0;
      delay(FRAMERATEDELAY);
      Serial.println("enter to maintenancemode2");
      delay(FRAMERATEDELAY);
      maintenancemode2();
      delay(FRAMERATEDELAY);
    }

  }
}
//------------------------------------------------------------------------------
void setup() {

  Serial.begin(115200);
  Serial.println("OZONCONTROLLER START");
  panel.freq_write = 20000000;
  panel.freq_fill  = 27000000;
  panel.freq_read  = 16000000;
  panel.spi_mode = 0;
  panel.spi_mode_read = 0;
  panel.len_dummy_read_pixel = 8;
  panel.spi_read = true;
  panel.spi_3wire = false;
  panel.spi_cs = 5;
  panel.spi_dc = 17;
  panel.gpio_rst = 16;
  panel.gpio_bl  = 4;
  panel.pwm_ch_bl = 7;
  panel.backlight_level = true;
  panel.reverse_invert = false;
  panel.rgb_order = false;
  panel.memory_width  = 240;
  panel.memory_height = 320;
  panel.panel_width  = 240;
  panel.panel_height = 320;
  panel.offset_x = 0;
  panel.offset_y = 0;
  panel.rotation = 0;
  panel.offset_rotation = 0;
  tft.setPanel(&panel);
  tft.init();
  tft.setRotation(3);
  Serial.println("TFT initialized");
  labelText(notifyInitializing);
  pinMode(ENCODER1_SWITCH_PIN, INPUT_PULLUP);
  pinMode(ENCODER2_SWITCH_PIN, INPUT_PULLUP);
  pinMode(PUMP_PIN, OUTPUT);
  pinMode(OZONE0_PIN, OUTPUT);
  pinMode(OZONE1_PIN, OUTPUT);
  pinMode(OZONE2_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  pinMode(BEEP_PIN, OUTPUT);
  digitalWrite(PUMP_PIN, 0);
  digitalWrite(OZONE0_PIN, 0);
  digitalWrite(OZONE1_PIN, 0);
  digitalWrite(OZONE2_PIN, 0);
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
  EEPROM.begin(52); // int 4 byte x 13
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
  Serial.print("log_OZONE0:");
  Serial.println(log_OZONE0);
  Serial.print("log_OZONE1:");
  Serial.println(log_OZONE1);
  Serial.print("log_OZONE2:");
  Serial.println(log_OZONE2);

  //timerAlarmDisable(timer);    // stop alarm
  //timerDetachInterrupt(timer);  // detach interrupt
  //timerEnd(timer);      // end timer


  timer = timerBegin(0, 80, true);
  timerAttachInterrupt(timer, &onTimer, true);
  timerAlarmWrite(timer, 20000, true);  // 20m sec
  timerAlarmEnable(timer);
  Serial.println("tiner set");

  //  pinMode(TFT_BACKLIGHT_PIN,OUTPUT);


  // digitalWrite(TFT_BACKLIGHT_PIN,HIGH);
  runFAN();
  Serial.println("FAN start");

  //tft.begin();

  //  Serial.print(F("Rectangles (filled)      "));
  //  Serial.println(testFilledRects(TFT_WHITE, TFT_BLUE));
  //  delay(500);


  if (1 ==  notify())  // 隠しコマンドが入力された場合
  {
    encoder1Pushed = 0;
    encoder2Pushed = 0;
    maintenancemode1();
  }

  if ( OZONELIFELIMIT < log_ozone ) {
    caution();
  }
  stopFAN();
}

void loop()
{
  static int powerstate = 0;
  static int pos1 = 0;
  static int pos2 = 0;

  Serial.println("mode=set enter");
  int kekka = set();
  Serial.println("mode=set exited");
  if ( 1 == kekka ) {
    Serial.println("mode=run enter");
    run();
    Serial.println("mode=run exited");
  }
  if ( 2 == kekka ) {
    setonstart();
  }


}
