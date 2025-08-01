

struct LABEL {
  int x1;
  int y1;
  int xlength;
  int ylength;
  int fgcolor;
  int bgcolor;
  String text;
  int scale;
  lgfx::U8g2font font;
};
extern const lgfx::U8g2font myFont40;

// LABEL notifyTitle = {30, 70, 240, 40, TFT_WHITE , TFT_BLUE, "REVONISER", 1, lgfxJapanGothicP_20};
// LABEL notifyVersion  = {40, 130, 240, 40, TFT_BLACK, TFT_WHITE , "URN-X", 1, lgfxJapanGothicP_20};
LABEL timetitle  = {  53, 28, 120, 20, TFT_WHITE, TFT_BLUE, "オゾン発生", 1, lgfxJapanGothicP_20};
LABEL timeHour = {44, 106, 40, 40, TFT_BLACK, TFT_LIGHTGRAY,    " 1", 1, myFont40};
LABEL timeMinute = {126, 106, 40, 40, TFT_BLACK, TFT_LIGHTGRAY,    "30", 1, myFont40};
LABEL timeSecond = {197, 106, 40, 40, TFT_BLACK, TFT_LIGHTGRAY,    "00", 1, myFont40};
LABEL timeZikan  = {87, 121, 32, 18, TFT_BLACK, TFT_LIGHTGRAY,    "時間", 1, lgfxJapanGothicP_16};
LABEL timeFan    = {173, 118, 20, 20, TFT_BLACK, TFT_LIGHTGRAY,    "分", 1, lgfxJapanGothicP_20};
LABEL timeByou   = {240, 118, 20, 20, TFT_BLACK, TFT_LIGHTGRAY,    "秒", 1, lgfxJapanGothicP_20};
LABEL timeState  = {  53, 73, 100, 20, TFT_BLACK, TFT_LIGHTGRAY, "稼働時間", 1, lgfxJapanGothicP_20};
LABEL timeOzone   = { 155, 177, 100, 20, TFT_WHITE, TFT_BLACK,   "オゾン濃度", 1, lgfxJapanGothicP_20};
LABEL timeOzoneMin = {128, 211, 36, 12, TFT_WHITE, TFT_BLACK, "MIN", 1, lgfxJapanGothicP_12};
LABEL timeOzoneMax = {276, 211, 36, 12, TFT_WHITE, TFT_BLACK, "MAX", 1, lgfxJapanGothicP_12};
LABEL timeOzoneBar = {153, 205, 120, 20, TFT_WHITE, TFT_BLACK, "□□□□□□", 1, lgfxJapanGothicP_20};

LABEL notifyInitializing = {30, 70, 240, 40, TFT_BLACK , (2 * 64 + 4) * 32 + 2 , "起動中...", 1, lgfxJapanGothicP_20};


LABEL m1title1 = {24, 4, 210, 23, TFT_WHITE, TFT_BLUE, " 累計稼働時間", 1, lgfxJapanGothicP_20};
LABEL m1version = {260, 4, 30, 80,  TFT_WHITE, TFT_BLACK, VERSION , 1, lgfxJapanGothicP_12};
LABEL m110 = {30, 28, 220, 18,  TFT_WHITE, TFT_BLACK, "・ポンプ"    , 1, lgfxJapanGothicP_12};
LABEL m111 = {30, 42, 220, 18,  TFT_WHITE, TFT_BLACK, "・オゾンユニット", 1, lgfxJapanGothicP_12};
LABEL m112 = {30, 56, 220, 18,  TFT_WHITE, TFT_BLACK, "・ファン稼働時間", 1, lgfxJapanGothicP_12};
LABEL m113 = {30, 70, 220, 18,  TFT_WHITE, TFT_BLACK, "・累計ON回数"  , 1, lgfxJapanGothicP_12};

LABEL m1title3 = {24, 95, 210, 23, TFT_WHITE, TFT_BLUE, " モード", 1, lgfxJapanGothicP_20};
LABEL m130 = {30, 119, 220, 14,  TFT_WHITE, TFT_BLACK, "・Xモード"    , 1, lgfxJapanGothicP_12};

LABEL m1title2 = {24, 144, 210, 23, TFT_WHITE, TFT_BLUE, " 強制動作", 1, lgfxJapanGothicP_20};
LABEL m120 = {30, 168, 220, 14,  TFT_WHITE, TFT_BLACK, "・ポンプ"    , 1, lgfxJapanGothicP_12};
LABEL m121 = {30, 182, 220, 14,  TFT_WHITE, TFT_BLACK, "・第１オゾンユニット" , 1, lgfxJapanGothicP_12};
LABEL m122 = {30, 196, 220, 14,  TFT_WHITE, TFT_BLACK, "・第２オゾンユニット" , 1, lgfxJapanGothicP_12};
LABEL m123 = {30, 210, 220, 14,  TFT_WHITE, TFT_BLACK, "・第３オゾンユニット" , 1, lgfxJapanGothicP_12};
LABEL m124 = {30, 224, 220, 14,  TFT_WHITE, TFT_BLACK, "・オゾン動作ログ" , 1, lgfxJapanGothicP_12};

LABEL md110 = {180, 28, 150, 18,  TFT_WHITE, TFT_BLACK, "00:00:00" , 1, lgfxJapanGothic_12};
LABEL md111 = {160, 42, 150, 18,  TFT_WHITE, TFT_BLACK, "00:00 00:00 00:00", 1,lgfxJapanGothicP_8};
LABEL md112 = {180, 56, 150, 18,  TFT_WHITE, TFT_BLACK, "00:00:00", 1, lgfxJapanGothic_12};
LABEL md113 = {200, 70, 50, 18,  TFT_WHITE, TFT_BLACK, "       0", 1, lgfxJapanGothicP_12};

LABEL md130 = {200, 119, 35, 14,  TFT_WHITE, TFT_BLACK, "ON ", 1, lgfxJapanGothicP_12};

LABEL md120 = {200, 168, 35, 14,  TFT_WHITE, TFT_BLACK, "OFF", 1, lgfxJapanGothicP_12};
LABEL md121 = {200, 182, 35, 14,  TFT_WHITE, TFT_BLACK, "OFF", 1, lgfxJapanGothicP_12};
LABEL md122 = {200, 196, 35, 14,  TFT_WHITE, TFT_BLACK, "OFF", 1, lgfxJapanGothicP_12};
LABEL md123 = {200, 210, 35, 14,  TFT_WHITE, TFT_BLACK, "OFF", 1, lgfxJapanGothicP_12};
LABEL md124 = {170, 224, 80, 14,  TFT_WHITE, TFT_BLACK, "      RESET  ", 1, lgfxJapanGothicP_12};



LABEL m2title1 = {34, 16, 210, 29, TFT_WHITE, TFT_BLUE, " 検査モード", 1, lgfxJapanGothicP_20};
LABEL m2version = {260, 16, 30, 80,  TFT_WHITE, TFT_BLACK, VERSION , 1, lgfxJapanGothicP_12};
LABEL m210 = {30, 48, 220, 20,  TFT_WHITE, TFT_BLACK, "・ポンプ"                   , 1, lgfxJapanGothicP_12};
LABEL m211 = {30, 68, 220, 20,  TFT_WHITE, TFT_BLACK, "・第１オゾンユニット"         , 1, lgfxJapanGothicP_12};
LABEL m212 = {30, 88, 220, 20,  TFT_WHITE, TFT_BLACK, "・第２オゾンユニット"         , 1, lgfxJapanGothicP_12};
LABEL m213 = {30, 108, 220, 20,  TFT_WHITE, TFT_BLACK, "・第３オゾンユニット"        , 1, lgfxJapanGothicP_12};
LABEL m214 = {30, 128, 250, 20,  TFT_WHITE, TFT_BLACK, "・タイマーチェック"          , 1, lgfxJapanGothicP_12};
LABEL m215 = {30, 148, 220, 20,  TFT_WHITE, TFT_BLACK, "・ブザーチェック"            , 1, lgfxJapanGothicP_12};
LABEL m216 = {30, 168, 220, 20,  TFT_WHITE, TFT_BLACK, "・ロータリーエンコーダー左右テスト", 1, lgfxJapanGothicP_12};
LABEL m217 = {30, 188, 220, 20,  TFT_WHITE, TFT_BLACK, "・左右スイッチチャタリングテスト", 1, lgfxJapanGothicP_12};

LABEL md210 = {240, 48, 40, 16,  TFT_WHITE, TFT_BLACK, "OFF", 1, lgfxJapanGothicP_12};
LABEL md211 = {240, 68, 40, 16,  TFT_WHITE, TFT_BLACK, "OFF", 1, lgfxJapanGothicP_12};
LABEL md212 = {240, 88, 40, 16,  TFT_WHITE, TFT_BLACK, "OFF", 1, lgfxJapanGothicP_12};
LABEL md213 = {240, 108, 40, 16,  TFT_WHITE, TFT_BLACK, "OFF", 1, lgfxJapanGothicP_12};
LABEL md214 = {240, 128, 40, 16,  TFT_WHITE, TFT_BLACK, "OFF", 1, lgfxJapanGothicP_12};
LABEL md215 = {240, 148, 40, 16,  TFT_WHITE, TFT_BLACK, "OFF", 1, lgfxJapanGothicP_12};
LABEL md216 = {240, 168, 40, 16,  TFT_WHITE, TFT_BLACK, "OFF", 1, lgfxJapanGothicP_12};
LABEL md217 = {80, 208, 190, 20,  TFT_WHITE, TFT_BLACK, "  L    0     R    0    ", 1, lgfxJapanGothicP_20};


static const uint8_t  PROGMEM myBitmap[] = {
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0xc, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x1e, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x1e, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x3e, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x3c, 0x0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x7d, 0xc0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0x79, 0xe0, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0xfb, 0xf8, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x0, 0xf3, 0xfc, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x1, 0xf7, 0xff, 0x0, 0x0, 0x0, 0x0,
  0x0, 0x1, 0xe7, 0xff, 0xc0, 0x0, 0x0, 0x0,
  0x0, 0x3, 0xef, 0xff, 0xf8, 0x0, 0x0, 0x0,
  0x0, 0x3, 0xcf, 0xff, 0xff, 0x0, 0x0, 0x0,
  0x0, 0x7, 0xdf, 0xff, 0xff, 0xc0, 0x0, 0x0,
  0x0, 0x7, 0x9f, 0xff, 0xff, 0xf0, 0x0, 0x0,
  0x0, 0xf, 0xbf, 0xff, 0xff, 0xfc, 0x0, 0x0,
  0x0, 0xf, 0xbf, 0xff, 0xff, 0xff, 0x0, 0x0,
  0x0, 0x1f, 0x7f, 0xff, 0xff, 0xff, 0x80, 0x0,
  0x0, 0x1f, 0x7f, 0xff, 0xff, 0xff, 0xc0, 0x0,
  0x0, 0x3e, 0xff, 0xff, 0xff, 0xff, 0xe0, 0x0,
  0x0, 0x3e, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x0,
  0x0, 0x3d, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x0,
  0x0, 0x7d, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x0,
  0x0, 0x7b, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x0,
  0x0, 0xfb, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x0,
  0x0, 0xf7, 0xff, 0xff, 0xff, 0xff, 0xf0, 0x0,
  0x0, 0xf7, 0xfe, 0x3, 0xff, 0xff, 0xe0, 0x0,
  0x1, 0xf7, 0xc0, 0x1c, 0xff, 0xff, 0xc0, 0x0,
  0x1, 0xee, 0x0, 0x3f, 0x7f, 0xff, 0x80, 0x0,
  0x1, 0xe8, 0x0, 0x7f, 0xbf, 0xff, 0x0, 0x0,
  0x3, 0xc0, 0x1, 0xff, 0xbf, 0xfe, 0x0, 0x0,
  0x3, 0xc0, 0x3, 0xff, 0xbf, 0xf8, 0x0, 0x0,
  0x3, 0xc0, 0x7, 0xff, 0x7f, 0xe0, 0x0, 0x0,
  0x7, 0x80, 0x7, 0xfe, 0xff, 0x0, 0x0, 0x0,
  0x7, 0x80, 0xf, 0xfd, 0xf8, 0x0, 0x0, 0x0,
  0x7, 0x0, 0x1f, 0xfb, 0x8c, 0x0, 0x0, 0x0,
  0xf, 0x0, 0x3f, 0xe1, 0xfe, 0x0, 0x0, 0x0,
  0xf, 0x0, 0x3f, 0x9f, 0xff, 0x0, 0x0, 0x0,
  0xe, 0x0, 0x7c, 0xff, 0xff, 0xc0, 0x0, 0x0,
  0xe, 0x0, 0x63, 0xff, 0xff, 0xe0, 0x0, 0x0,
  0x1c, 0x0, 0x9f, 0xff, 0xff, 0xf8, 0x0, 0x0,
  0x1c, 0x0, 0x7f, 0xff, 0xff, 0xfc, 0x0, 0x0,
  0x1c, 0x0, 0xff, 0xff, 0xff, 0xfe, 0x0, 0x0,
  0x38, 0x0, 0xff, 0xff, 0xff, 0xff, 0x80, 0x0,
  0x38, 0x0, 0xff, 0xff, 0xff, 0xff, 0xc0, 0x0,
  0x30, 0x0, 0x7f, 0xff, 0xff, 0xff, 0xe0, 0x0,
  0x30, 0x0, 0x1f, 0xff, 0xff, 0xff, 0xf0, 0x0,
  0x30, 0x0, 0x0, 0x7f, 0xff, 0xff, 0xf8, 0x0,
  0x60, 0x0, 0x0, 0x0, 0x1f, 0xff, 0xfc, 0x0,
  0x60, 0x0, 0x0, 0x0, 0x3, 0xff, 0xfe, 0x0,
  0x40, 0x0, 0x0, 0x0, 0x0, 0x7f, 0xff, 0x0,
  0x40, 0x0, 0x0, 0x0, 0x0, 0xf, 0xff, 0x80,
  0x40, 0x0, 0x0, 0x0, 0x0, 0x3, 0xfb, 0xc0,
  0x80, 0x0, 0x0, 0x0, 0x0, 0x0, 0xfc, 0xe0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x3c, 0x30,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1e, 0x10,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x7, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x1, 0x0,
  0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0, 0x0
};

String  cautiontxt1=("注 意");
String  cautiontxt2=("稼働時間が設計使用時間を超えました。");
String  cautiontxt3=("オゾンユニットが消耗しています。");
String  cautiontxt4=("メンテナンスを依頼してください。");
String  cautiontxt5=("【お問合せ先】 株式会社レボル");
String  cautiontxt6=("048-254-0070");
