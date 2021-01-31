/* 库定义*/
#include <stdio.h>
#include <string.h>
#include <DS1302.h>//控制DS1302
#include <U8glib.h>//控制LCD12864
#include <dht11.h>//控制DHT11
#include <Keypad.h>//控制4*4矩阵键盘
//#include <Adafruit_NeoPixel.h>//控制5050RGBLED

/*全局变量定义*/
const byte ROWS = 4; //键盘行数
const byte COLS = 4; //键盘列数
const uint8_t lcdPageNum = 2; //LCD显示页数
//定义键盘的按键cymbols
//A、B、C、D为模式选择
//*为清零、自动模式，#为确认、手动模式，数字为数字输入
char hexaKeys[ROWS][COLS] =
{
    {'1', '2', '3', 'A'},
    {'4', '5', '6', 'B'},
    {'7', '8', '9', 'C'},
    {'*', '0', '#', 'D'}
};

static uint8_t dhtFlag = 0; //DHT是否读取成功的标志，0成功，非0失败
static uint8_t modeFlag = 1; //工作模式选择标志，0为自动模式，1为手动模式,2为定时模式
static uint8_t lcdPageFlag = 0; //LCD显示页面的选择标志，lcdPageFlag等于几，显示第几页
static uint8_t lcdOnOffFlag = 0; //LCD显示显示开关标志，0开，1关
/*
  //从LCD进行阙值设置的标志，0不进行设置，1进入环境湿度(EH)阙值的设置，2进入温度(Te)阙值的设置
  //3进入土壤湿度(SH)阙值的设置，4进入光照(Li)阙值的设置，5进入阙值设置选择界面
  //11为AHmin,12为AHmax
  //21为Temin,22为Temax
  //31为SHmin,32为SHmax
  //41为Limin,42为Limax
  //60设置成功,61设置失败1[阙值设置失败(越界)显示界面(范围在0-100之间)]
  //62设置失败2[阙值设置失败(非法)显示界面(min必须<=max)]
*/
static uint8_t setThresholdFromLcdFlag = 0;//初值为0，不进入设置，进入到MENU界面
static uint8_t controllerFlag1 = 1; //继电器1开关标志(控制日光灯)，0开，非0关
static uint8_t controllerCountFlag1=0;//继电器1开启时长控制标志(控制日光灯)
static uint8_t controllerFlag2 = 1; //继电器2开关标志(控制水泵)，0开，非0关

static int thresholdTemporary = 0; //临时阙值，用于按键模式模式D(阙值设置)时暂存键盘输入的数据
static int thresholdOfTemp_MAX = 35;
static int thresholdOfTemp_MIN = 15;
static int thresholdOfAirHumi_MAX = 60;
static int thresholdOfAirHumi_MIN = 15;
static int thresholdOfSoilHumi_MAX = 90;
static int thresholdOfSoilHumi_MIN = 20;
static int thresholdOfLigh_MAX = 80;
static int thresholdOfLigh_MIN = 50;


/* 接口定义
    使用Arduino MEGA2560 R3 开发板
    LCD12864(ST7920核，采用SPI工作方式)
  LCD12864 SCK (EN) -> Arduino D18
  LCD12864 CS  (RS) -> Arduino D17
  LCD12864 MOSI(RW) -> Arduino D16
  LCD12864 VCC,BLA  -> Arduino 5v
  LCD12864 PSB,GND,BLK -> Arduino GND
    RTC(DS1302)
  DS1302 RST -> Arduino D5
  DS1302 DAT -> Arduino D6
  DS1302 CLK -> Arduino D7
  DS1302 VCC -> Arduino 5v
  DS1302 GND -> Arduino GND
    HC05模块
    HC05模块使用UART3口，需用对象Serial3来控制。由于Arduino Mega 2560的USB口为UART0，
  这样就可以用对象Serial控制PC串口和Arduino通信，Serial3用于控制Arduino和蓝牙模块通信。
  且两者可以互不影响，各自通信。
  HC05 RXD -> Arduino D14(TX3)
  HC05 TXD -> Arduino D15(RX3)
  HC05 AT(KEY) -> Arduino D2
  HC05 VCC -> Arduino 5V
  HC05 GND -> Arduino GND
    DHT11
  DHT11 VCC -> Arduino 5v
  DHT11 GND -> Arduino GND
  DHT11 DAT -> Arduino D19
    光敏电阻
  LDR(光敏电阻)-> Arduino A0
    土壤湿度传感器(SHS)
  SHS AO-> Arduino A15
  SHS DO用不到，空接
  SHS VCC -> Arduino 5v
  SHS GND -> Arduino GND
    继电器1控制日光灯
  Controller1 IN  -> Arduino D47
  Controller1 GND -> Arduino GND
  Controller1 VCC -> Arduino 5v
    继电器2控制水泵
  Controller2 IN  -> Arduino D29
  Controller2 GND -> Arduino GND
  Controller2 VCC -> Arduino 5v
    4*4键盘
  行接D31,D33,D35,D37;列接D39,D41,D43,D45
    LED状态灯(发光二极管)
  二极管正极接D13口，负极接1K电阻再接GND
  ////    5050RGBLED(驱动器IC为WS2812)
  ////    5050RGB灯带可以通过一个角即可控制多个RGBLED灯，
  ////  如果有条件可以用5050灯带代替RGB灯，但相比传统普通RGB灯，价格贵了好多，
  ////  设计5050RGB灯带的代码，被逼着用////注释(实现和传统RGB的功能),有条件鼓励用5050替代传统RGB
  ////  5050  DI  -> Arduino D6
  ////  5050  VCC -> Arduino 5v
  ////  5050  GND -> Arduino GND
*/


#define RTC_RST_PIN    5
#define RTC_DAT_PIN    6
#define RTC_CLK_PIN    7
#define HC05_AT_PIN    2
#define DHT_DAT_PIN    19
#define LCD_SCK_PIN    18
#define LCD_CS_PIN     17
#define LCD_MOSI_PIN   16
#define LED_PIN        13
#define Controller1IN  47
#define Controller2IN  29
#define LDR_DAT_PIN    A0
#define SHS_DAT_PIN    A15
byte rowPins[ROWS] = {31, 33, 35, 37};
byte colPins[COLS] = {39, 41, 43, 45};
////#define RGBLED_DI_PIN  5

/*静态常量定义*/
//字模定义
/*-- 宋体; 此字体下对应的点阵为：宽x高=14x14 --*/
static unsigned char wenHz[] U8G_PROGMEM =  /*-- 文字:  温--*/
{
    0xE0, 0x07, 0x22, 0x04, 0x24, 0x04, 0xE0, 0x07, 0x21, 0x04, 0x22, 0x04, 0xE0, 0x07, 0x04, 0x00,
    0xF4, 0x0F, 0x52, 0x0A, 0x52, 0x0A, 0x51, 0x0A, 0x51, 0x0A, 0xF8, 0x1F
};
static unsigned char shiHz[] U8G_PROGMEM =  /*-- 文字:  湿--*/
{
    0x00, 0x00, 0xF2, 0x0F, 0x14, 0x08, 0x10, 0x08, 0xF1, 0x0F, 0x12, 0x08, 0x10, 0x08, 0xF4, 0x0F,
    0x44, 0x02, 0x4A, 0x12, 0x52, 0x0A, 0x61, 0x06, 0x41, 0x02, 0xF8, 0x1F
};
static unsigned char duHz[] U8G_PROGMEM =  /*-- 文字:  度--*/
{
    0x40, 0x00, 0x80, 0x00, 0xFC, 0x1F, 0x24, 0x04, 0xFC, 0x1F, 0x24, 0x04, 0xE4, 0x07, 0x04, 0x00,
    0xF4, 0x0F, 0x24, 0x04, 0x44, 0x02, 0x82, 0x01, 0x62, 0x06, 0x19, 0x18
};
static unsigned char guangHz[] U8G_PROGMEM =  /*-- 文字:  光--*/
{
    0x40, 0x00, 0x42, 0x08, 0x44, 0x04, 0x48, 0x02, 0x40, 0x00, 0xFF, 0x1F, 0x10, 0x01, 0x10, 0x01,
    0x10, 0x01, 0x10, 0x01, 0x08, 0x01, 0x08, 0x11, 0x04, 0x11, 0x02, 0x1E
};
static unsigned char zhaoHz[] U8G_PROGMEM =  /*-- 文字:  照--*/
{
    0xDE, 0x0F, 0x92, 0x08, 0x92, 0x08, 0x92, 0x08, 0x5E, 0x06, 0x12, 0x00, 0xD2, 0x0F, 0x52, 0x08,
    0x52, 0x08, 0xDE, 0x0F, 0x00, 0x00, 0x92, 0x08, 0x22, 0x11, 0x21, 0x11
};
static unsigned char tuHz[] U8G_PROGMEM =  /*-- 文字:  土--*/
{
    0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0xFE, 0x0F, 0x40, 0x00, 0x40, 0x00,
    0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0xFF, 0x1F
};
static unsigned char rangHz[] U8G_PROGMEM =  /*-- 文字:  壤--*/
{
    0x04, 0x01, 0xF4, 0x1F, 0x04, 0x00, 0xE4, 0x0E, 0xAF, 0x0A, 0xE4, 0x0E, 0x44, 0x04, 0xF4, 0x1F,
    0x44, 0x04, 0xE4, 0x0F, 0x4C, 0x04, 0xF3, 0x1F, 0x60, 0x0A, 0xD0, 0x1C
};
static unsigned char kongHz[] U8G_PROGMEM =  /*-- 文字:  空--*/
{
    0x20, 0x00, 0x40, 0x00, 0xFF, 0x1F, 0x11, 0x11, 0x09, 0x12, 0x04, 0x04, 0x00, 0x00, 0xFC, 0x07,
    0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0x40, 0x00, 0xFF, 0x1F
};
static unsigned char qiHz[] U8G_PROGMEM =  /*-- 文字:  气--*/
{
    0x04, 0x00, 0x04, 0x00, 0xFC, 0x1F, 0x02, 0x00, 0xF9, 0x07, 0x00, 0x00, 0xFE, 0x07, 0x00, 0x04,
    0x00, 0x04, 0x00, 0x04, 0x00, 0x04, 0x00, 0x14, 0x00, 0x18, 0x00, 0x10
};
static unsigned char queHz[] U8G_PROGMEM =  /*-- 文字:  阙--*/
{
    0x02, 0x00, 0xF4, 0x1F, 0x00, 0x10, 0x8A, 0x12, 0x52, 0x12, 0x02, 0x16, 0xFA, 0x15, 0x22, 0x10,
    0xAA, 0x12, 0xAA, 0x12, 0xFA, 0x12, 0x22, 0x12, 0x12, 0x15, 0x8A, 0x1C
};
static unsigned char zhiHz[] U8G_PROGMEM =  /*-- 文字:  值--*/
{
    0x08, 0x01, 0x08, 0x01, 0xF4, 0x1F, 0x04, 0x01, 0xE6, 0x0F, 0x25, 0x08, 0xE4, 0x0F, 0x24, 0x08,
    0xE4, 0x0F, 0x24, 0x08, 0xE4, 0x0F, 0x24, 0x08, 0x24, 0x08, 0xF4, 0x1F
};

static unsigned char zhuangHz[] U8G_PROGMEM =  /*-- 文字:  状--*/
{
    0x08, 0x01, 0x08, 0x05, 0x09, 0x09, 0x0A, 0x01, 0xEA, 0x1F, 0x08, 0x01, 0x08, 0x01, 0x8C, 0x02,
    0x8A, 0x02, 0x89, 0x02, 0x48, 0x04, 0x48, 0x04, 0x28, 0x08, 0x18, 0x10
};
static unsigned char taiHz[] U8G_PROGMEM =  /*-- 文字:  态--*/
{
    0x40, 0x00, 0x40, 0x00, 0xFF, 0x1F, 0x40, 0x00, 0xA0, 0x00, 0x10, 0x01, 0x2C, 0x06, 0x43, 0x18,
    0x00, 0x00, 0x40, 0x00, 0x88, 0x08, 0x8A, 0x10, 0x0A, 0x12, 0xF1, 0x03
};
static unsigned char dangHz[] U8G_PROGMEM =  /*-- 文字:  当--*/
{
    0x40, 0x00, 0x42, 0x08, 0x44, 0x04, 0x48, 0x02, 0xFF, 0x0F, 0x00, 0x08, 0x00, 0x08, 0x00, 0x08,
    0xFE, 0x0F, 0x00, 0x08, 0x00, 0x08, 0x00, 0x08, 0xFF, 0x0F, 0x00, 0x08
};
static unsigned char qianHz[] U8G_PROGMEM =  /*-- 文字:  前--*/
{
    0x04, 0x04, 0x08, 0x02, 0xFF, 0x1F, 0x00, 0x00, 0x7E, 0x08, 0x42, 0x09, 0x42, 0x09, 0x7E, 0x09,
    0x42, 0x09, 0x42, 0x09, 0x7E, 0x09, 0x42, 0x08, 0x42, 0x08, 0x72, 0x0E
};
static unsigned char sheHz[] U8G_PROGMEM =  /*-- 文字:  设--*/
{
    0x02, 0x00, 0xC4, 0x07, 0x44, 0x04, 0x40, 0x04, 0x40, 0x04, 0x27, 0x18, 0x14, 0x00, 0xE4, 0x0F,
    0x44, 0x08, 0x44, 0x04, 0x94, 0x02, 0x0C, 0x01, 0xC4, 0x06, 0x30, 0x18
};
static unsigned char zhi2Hz[] U8G_PROGMEM =  /*-- 文字:  置--*/
{
    0xFE, 0x0F, 0x12, 0x09, 0x12, 0x09, 0xFE, 0x0F, 0x40, 0x00, 0xFE, 0x0F, 0x40, 0x00, 0xFC, 0x0,
    0x04, 0x04, 0x04, 0x04, 0x44, 0x04, 0x44, 0x04, 0x44, 0x04, 0xFF, 0x1F
};
static unsigned char xuanHz[] U8G_PROGMEM =  /*-- 文字:  选--*/
{
    0x00, 0x01, 0x22, 0x01, 0x24, 0x01, 0xE4, 0x0F, 0x10, 0x01, 0x00, 0x01, 0xF7, 0x1F, 0x84, 0x02,
    0x84, 0x02, 0x44, 0x02, 0x24, 0x12, 0x14, 0x1C, 0x0A, 0x00, 0xF1, 0x1F
};
static unsigned char zeHz[] U8G_PROGMEM =  /*-- 文字:  则--*/
{
    0x04, 0x00, 0xE4, 0x0F, 0x44, 0x04, 0x9F, 0x02, 0x04, 0x01, 0x84, 0x06, 0x64, 0x18, 0x0C, 0x01,
    0xC7, 0x07, 0x04, 0x01, 0x04, 0x01, 0xF4, 0x1F, 0x04, 0x01, 0x07, 0x01
};
static unsigned char chengHz[] U8G_PROGMEM =  /*-- 文字:  成--*/
{
    0x00, 0x05, 0x00, 0x09, 0x00, 0x01, 0xFE, 0x1F, 0x02, 0x01, 0x02, 0x09, 0x3E, 0x09, 0x22, 0x09,
    0x22, 0x05, 0x22, 0x05, 0x22, 0x12, 0x1A, 0x15, 0x82, 0x18, 0x41, 0x10
};
static unsigned char gongHz[] U8G_PROGMEM =  /*-- 文字:  功--*/
{
    0x80, 0x00, 0x80, 0x00, 0x9F, 0x00, 0xC4, 0x0F, 0x84, 0x08, 0x84, 0x08, 0x84, 0x08, 0x84, 0x08,
    0x84, 0x08, 0x9C, 0x08, 0x43, 0x08, 0x40, 0x08, 0x20, 0x08, 0x10, 0x06
};
static unsigned char shi2Hz[] U8G_PROGMEM =  /*-- 文字:  失--*/
{
    0x40, 0x00, 0x48, 0x00, 0x48, 0x00, 0xF8, 0x07, 0x44, 0x00, 0x42, 0x00, 0x40, 0x00, 0xFF, 0x1F,
    0xA0, 0x00, 0xA0, 0x00, 0x10, 0x01, 0x08, 0x02, 0x04, 0x04, 0x03, 0x18
};
static unsigned char baiHz[] U8G_PROGMEM =  /*-- 文字:  败--*/
{
    0x80, 0x00, 0x9F, 0x00, 0x91, 0x00, 0x95, 0x1F, 0x55, 0x08, 0xB5, 0x08, 0x95, 0x08, 0x95, 0x08,
    0x15, 0x05, 0x15, 0x05, 0x04, 0x02, 0x0A, 0x05, 0x92, 0x08, 0x61, 0x10
};
/* 类定义*/
class LDR
{
public:
    LDR(uint8_t pin) : dataPin(pin) {}
    long getLdrData()
    {
        pinMode(dataPin, INPUT);
        return analogRead(dataPin);
    }
    long getLight()
    {
        return map(getLdrData(), 1023, 0, 0, 100);
    }
private:
    const uint8_t dataPin;
};
class SHS//土壤湿度传感器(Soil humidity sensor)
{
public:
    SHS(uint8_t pin) : dataPin(pin) {}
    long getShsData()
    {
        pinMode(dataPin, INPUT);
        return analogRead(dataPin);
    }
    long getSoilHumi()
    {
        return map(getShsData(), 1023, 0, 0, 100);
    }
private:
    const uint8_t dataPin;
};

/* 构造对象*/
// 创建DS1302对象rtc
DS1302 rtc(RTC_RST_PIN, RTC_DAT_PIN, RTC_CLK_PIN);
// 创建LCD12864对象lcd
U8GLIB_ST7920_128X64_4X lcd(LCD_SCK_PIN, LCD_MOSI_PIN, LCD_CS_PIN);
// 创建dht11对象dht
dht11 dht;
// 创建LDR对象LDR
LDR ldr(LDR_DAT_PIN);
// 创建SHS对象shs
SHS shs(SHS_DAT_PIN);
// 创建Keypad对象keypad
Keypad keypad = Keypad( makeKeymap(hexaKeys), rowPins, colPins, ROWS, COLS);
//////创建 RGBLED对象rgbled
/////*构造函数参数说明：
////  Parameter 1 = number of pixels in strip
////  Parameter 2 = pin number (most are valid)
////  Parameter 3 = pixel type flags, add together as needed:
////  NEO_KHZ800  800 KHz bitstream (most NeoPixel products w/WS2812 LEDs)
////  NEO_KHZ400  400 KHz (classic 'v1' (not v2) FLORA pixels, WS2811 drivers)
////  NEO_GRB     Pixels are wired for GRB bitstream (most NeoPixel products)
////  NEO_RGB     Pixels are wired for RGB bitstream (v1 FLORA pixels, not v2)
////*/
////Adafruit_NeoPixel rgbled = Adafruit_NeoPixel(4, RGBLED_DI_PIN, NEO_GRB + NEO_KHZ800);

/* 函数声明*/
//RTC函数
void RTCInit();
void TimePrint();
void TimeSetFromBluetooth(String);
void TimeYMDSetFromBluetooth(String);
void TimeHMSSetFromBluetooth(String);
void TimeYrSetFromBluetooth(String);
void TimeMonSetFromBluetooth(String);
void TimeDateSetFromBluetooth(String);
void TimeHrSetFromBluetooth(String);
void TimeMinSetFromBluetooth(String);
void TimeSecSetFromBluetooth(String);
void TimeDaySetFromBluetooth(String);
void TimePrintToLcd();
//LCD函数
void LCDInit();//LCD初始化
void draw();//LCD控制函数
void draw0();//关闭LCD显示
void draw1();//MENU1显示
void draw2();//MENU2显示
void drawD1();//AH设置选择显示
void drawD11();//AHmin设置界面
void drawD12();//AHmax设置界面
void drawD2();//Te设置选择显示
void drawD21();//Temin设置界面
void drawD22();//Temax设置界面
void drawD3();//SH设置选择显示
void drawD31();//SHmin设置界面
void drawD32();//SHmax设置界面
void drawD4();//Li设置选择显示
void drawD41();//Limin设置界面
void drawD42();//LImax设置界面
void drawD5();//阙值设置选择界面
void drawD60();//阙值设置成功显示界面
void drawD61();//阙值设置失败(越界)显示界面(范围在0-100之间)
void drawD62();//阙值设置失败(非法)显示界面(min必须<=max)

//DHT函数
uint8_t DHTGetData();
void DHTHumiPrint();
void DHTTempPrint();
void DHTPrintToLcd();
//LDR函数
void LDRPrint();
void LDRPrinttoLcd();
//SHS函数
void SHSPrint();
void SHSPrinttoLcd();
//////RGBLED控制
////void RGBControl();
//阙值控制
void SETthresholdOfTemp_MAX(String);
void SETthresholdOfTemp_MIN(String);
void SETthresholdOfAirHumi_MAX(String);
void SETthresholdOfAirHumi_MIN(String);
void SETthresholdOfSoilHumi_MAX(String);
void SETthresholdOfSoilHumi_MIN(String);
void SETthresholdOfLigh_MAX(String);
void SETthresholdOfLigh_MIN(String);
void GETthresholdOfTemp();
void GETthresholdOfEnvirHumi();
void GETthresholdOfSoilHumi();
void GETthresholdOfLigh();
//HC05函数
void BluetoothInit();
//模式获取
void GetmodeFlag();
//按键处理逻辑
void keyContral();
//LED逻辑
void ledControl();

//错误码约定：
//401 DHT11读数据发生校验和错误
//402 DHT11读数据发生超时错误
//403 DHT11读数据发生其他未知错误
//404 获取数据失败
//405 阙值设置时，上阙值小于下阙值
//406 光照和湿度的阙值设置范围必须为0-100之间
/*函数入口*/
void setup()
{
    Serial.begin(9600);
    Serial.println("Start Init");
    RTCInit();
    LCDInit();
    BluetoothInit();
    ////    rgbled.begin();
    ////    rgbled.show(); // 将rgbled所有像素初始化为“关闭”
    // 把这个数字引脚初始化为输出引脚
    pinMode(LED_PIN, OUTPUT);
    pinMode(Controller1IN, OUTPUT); //定义Controller1IN 接口为输出接口(继电器1)
    pinMode(Controller2IN, OUTPUT); //定义Controller2IN 接口为输出接口(继电器2)
    Serial.println("Init  OK");
}
void loop()
{
    //DHT读数据
    dhtFlag = DHTGetData();
    //通信处理
    char BluetoothDataBuf[32] {' '}, commandData[32] {' '}, commandType = ' ';
    for (uint8_t i = 0; Serial3.available() > 0; i++)
    {
        BluetoothDataBuf[i] = char(Serial3.read());
        delay(2);
    }
    //将蓝牙串口数据拆分成指令类型和指令数据
    sscanf(BluetoothDataBuf, "%c:%s", &commandType, commandData);
    switch (commandType)
    {
    //LCD 控制
    case 'a'://关LCD
    {
        lcdOnOffFlag = 1;
        /*void U8GLIB::setColorIndex(uint8_t color_index)
          color_index：①1：表示显示，不透明②0：表示不显示，透明。*/
        Serial.println("LCD12864 OFF");
    }
    break;
    case 'A'://开LCD
    {
        lcdOnOffFlag = 0;
        Serial.println("LCD12864 ON");
    }
    break;
    //RTC时间管理
    case 'b'://获取时间
        TimePrint();
        break;
    case 'B'://设置时间
        TimeSetFromBluetooth(commandData);
        break;
    case 'c'://设置时间-年月日
        TimeYMDSetFromBluetooth(commandData);
        break;
    case 'C'://设置时间-时分秒
        TimeHMSSetFromBluetooth(commandData);
        break;
    case 'd'://设置时间-年
        TimeYrSetFromBluetooth(commandData);
        break;
    case 'D'://设置时间-月
        TimeMonSetFromBluetooth(commandData);
        break;
    case 'e'://设置时间-日
        TimeDateSetFromBluetooth(commandData);
        break;
    case 'E'://设置时间-时
        TimeHrSetFromBluetooth(commandData);
        break;
    case 'f'://设置时间-分
        TimeMinSetFromBluetooth(commandData);
        break;
    case 'F'://设置时间-秒
        TimeSecSetFromBluetooth(commandData);
        break;
    case 'g'://设置时间-星期
        TimeDaySetFromBluetooth(commandData);
        break;
    //传感器数据获取
    case 'G'://获取温度
    {
        if (!dhtFlag)
            DHTTempPrint();
        else
            Serial3.println("404");
    }
    break;
    case 'h'://获取环境湿度
    {
        if (!dhtFlag)
            DHTHumiPrint();
        else
            Serial3.println("404");
    }
    break;
    case 'H'://获取土壤湿度
        SHSPrint();
        break;
    case 'i'://获取光照
        LDRPrint();
        break;
    //阙值控制
    case 'I'://获取thresholdOfTemp
        GETthresholdOfTemp();
        break;
    case 'j'://设置thresholdOfTemp_MIN
        SETthresholdOfTemp_MIN(commandData);
        break;
    case 'J'://设置thresholdOfTemp_MAX
        SETthresholdOfTemp_MAX(commandData);
        break;
    case 'l'://获取thresholdOfEnvirHumi
        GETthresholdOfEnvirHumi();
        break;
    case 'K'://设置thresholdOfAirHumi_MAX
        SETthresholdOfAirHumi_MAX(commandData);
        break;
    case 'k'://设置thresholdOfAirHumi_MIN
        SETthresholdOfAirHumi_MIN(commandData);
        break;
    case 'L'://获取thresholdOfSoilHumi
        GETthresholdOfSoilHumi();
        break;
    case 'N'://设置thresholdOfSoilHumi_MAX
        SETthresholdOfSoilHumi_MAX(commandData);
        break;
    case 'n'://设置thresholdOfSoilHumi_MIN
        SETthresholdOfSoilHumi_MIN(commandData);
        break;
    case 'o'://获取thresholdOfLigh
        GETthresholdOfLigh();
        break;
    case 'P'://设置thresholdOfLigh_MAX
        SETthresholdOfLigh_MAX(commandData);
        break;
    case 'p'://设置thresholdOfLigh_MIN
        SETthresholdOfLigh_MIN(commandData);
        break;
    //工作模式控制
    case 'm'://设置为自动模式
    {
        GetmodeFlag();
        modeFlag = 0;
        GetmodeFlag();

    }
    break;
    case 'M'://设置为手动模式
    {
        GetmodeFlag();
        modeFlag = 1;
        controllerFlag1 = 1;
        controllerFlag2 = 1;
        GetmodeFlag();
    }
    break;
    //日光灯控制(继电器1)
    case 'q'://继电器1打开
        controllerFlag1 = 0;
        break;
    case 'Q'://继电器1关闭
        controllerFlag1 = 1;
        break;
    //水泵模式控制(继电器2)
    case 'r'://继电器2打开
        controllerFlag2 = 0;
        break;
    case 'R'://继电器2关闭
        controllerFlag2 = 1;
        break;
    case 's'://LCD显示页面1
        lcdPageFlag = 0;
        break;
    case 'S'://LCD显示页面2
        lcdPageFlag = 1;
        break;
    case 't'://获得当前mode
        GetmodeFlag();
        break;
    default:
        break;
    }
    //按键逻辑
    //按键处理放置在通信处理之后,优先级高于通信处理
    keyContral();
    //LED逻辑
    ledControl();
    //LCD逻辑
    draw();
    ////RGBControl();
    //如果在自动模式下
    if (!modeFlag)
    {
        if ((ldr.getLight() < thresholdOfLigh_MIN)||(controllerCountFlag1>0))
        {
            if(controllerCountFlag1==0)
            {
                controllerCountFlag1=30;//开灯30*500ms=15s
            }
            controllerFlag1 = 0;
            controllerCountFlag1--;
        }
        else if (ldr.getLight() > (thresholdOfLigh_MAX + thresholdOfLigh_MIN) / 2)
            controllerFlag1 = 1;
        if (shs.getSoilHumi() < thresholdOfSoilHumi_MIN)
            controllerFlag2 = 0;
        else if (shs.getSoilHumi() > (thresholdOfSoilHumi_MAX + thresholdOfSoilHumi_MIN) / 2)
            controllerFlag2 = 1;
    }
    //继电器控制
    if (!controllerFlag1)
        digitalWrite(Controller1IN, HIGH); //驱动继电器1闭合导通
    else
        digitalWrite(Controller1IN, LOW); //驱动继电器1断开
    if (!controllerFlag2)
        digitalWrite(Controller2IN, HIGH); //驱动继电器2闭合导通
    else
        digitalWrite(Controller2IN, LOW); //驱动继电器2断开

    delay(500);
}

/* 函数定义*/
//RTC函数
void RTCInit()
{
    Serial.println("RTC Set Begin");
    rtc.write_protect(false);
    rtc.halt(false);
    Serial.println("RTC Set OK");
}
void TimePrint()
{
    //日期变量缓存
    char rtcTimeBuf[50];
    char rtcDayBuf[10];
    /*从 DS1302 获取当前时间
      Time::Time(uint16_t yr,uint8_t mon,uint8_t date,uint8_t hr,uint8_t min,uint8_t sec,uint8_t day)
                        年         月           日          时         分          秒          星期
      默认初始化为Time(2017, 1, 1, 0, 0, 0, 7)*/
    Time t = rtc.time();
    /* 将星期从数字转换为名称
      void *memset(void *s, int ch, size_t n)函数解释：
      将s中当前位置后面的n个字节(typedef unsigned int size_t)用 ch 替换并返回s。*/
    memset(rtcDayBuf, 0, sizeof(rtcDayBuf));
    switch (t.day)
    {
    /*char *strcpy(char* dest, const char *src)函数解释：
      头文件：#include <string.h> 和 #include <stdio.h>
      功能：把从src地址开始且含有NULL结束符的字符串复制到以dest开始的地址空间
      说明：src和dest所指内存区域不可以重叠且dest必须有足够的空间来容纳src的字符串,返回指向dest的指针。*/
    case 1:
        strcpy(rtcDayBuf, "Monday");
        break;
    case 2:
        strcpy(rtcDayBuf, "Tuesday");
        break;
    case 3:
        strcpy(rtcDayBuf, "Wednesday");
        break;
    case 4:
        strcpy(rtcDayBuf, "Thursday");
        break;
    case 5:
        strcpy(rtcDayBuf, "Friday");
        break;
    case 6:
        strcpy(rtcDayBuf, "Saturday");
        break;
    case 7:
        strcpy(rtcDayBuf, "Sunday");
        break;
    default:
        break;
    }
    /* 将日期代码格式化凑成buf等待输出
      snprintf()为sprintf()的安全版本，在<stdio.h>中声明
      int snprintf(char *str, size_t size, const char *format, ...),
      将可变个参数(...)按照format格式化成字符串，然后将其复制到str中*/
    snprintf(rtcTimeBuf, sizeof(rtcTimeBuf), "%s %04d-%02d-%02d %02d:%02d:%02d", rtcDayBuf, t.yr, t.mon, t.date, t.hr, t.min, t.sec);
    /* 输出日期到蓝牙和PC串口 */
    Serial3.println(rtcTimeBuf);
    Serial.println(rtcTimeBuf);
}
void TimeSetFromBluetooth(String commandData)
{
    int numData[7] = {0}, j = 0;
    /* 以逗号分隔分解commandData的字符串，分解结果变成转换成数字到numData[]数组 */
    for (uint8_t i = 0; i < commandData.length() ; i++)
    {
        //约定用','分隔各参数
        if (commandData[i] == ',')
            j++;
        else
            numData[j] = numData[j] * 10 + (commandData[i] - '0');
    }
    /* 将转换好的numData凑成时间格式，写入DS1302 */
    Time t(numData[0], numData[1], numData[2], numData[3], numData[4], numData[5], numData[6]);
    rtc.time(t);
}
void TimeYMDSetFromBluetooth(String commandData)
{
    int numData[3] = {0}, j = 0;
    for (uint8_t i = 0; i < commandData.length() ; i++)
    {
        if (commandData[i] == ',')
            j++;
        else
            numData[j] = numData[j] * 10 + (commandData[i] - '0');
    }
    rtc.year(numData[0]);
    rtc.month(numData[1]);
    rtc.date(numData[2]);
}
void TimeHMSSetFromBluetooth(String commandData)
{
    int numData[3] = {0}, j = 0;
    for (uint8_t i = 0; i < commandData.length() ; i++)
    {
        if (commandData[i] == ',')
            j++;
        else
            numData[j] = numData[j] * 10 + (commandData[i] - '0');
    }
    rtc.hour(numData[0]);
    rtc.minutes(numData[1]);
    rtc.seconds(numData[2]);
}
void TimeYrSetFromBluetooth(String commandData)
{
    int numData = 0;
    for (uint8_t i = 0; i < commandData.length() ; i++)
    {
        if (commandData[i] == ',')
            break;
        else
            numData = numData * 10 + (commandData[i] - '0');
    }
    rtc.year(numData);
}
void TimeMonSetFromBluetooth(String commandData)
{
    int numData = 0;
    for (uint8_t i = 0; i < commandData.length() ; i++)
    {
        if (commandData[i] == ',')
            break;
        else
            numData = numData * 10 + (commandData[i] - '0');
    }
    rtc.month(numData);
}
void TimeDateSetFromBluetooth(String commandData)
{
    int numData = 0;
    for (uint8_t i = 0; i < commandData.length() ; i++)
    {
        if (commandData[i] == ',')
            break;
        else
            numData = numData * 10 + (commandData[i] - '0');
    }
    rtc.date(numData);
}
void TimeHrSetFromBluetooth(String commandData)
{
    int numData = 0;
    for (uint8_t i = 0; i < commandData.length() ; i++)
    {
        if (commandData[i] == ',')
            break;
        else
            numData = numData * 10 + (commandData[i] - '0');
    }
    rtc.hour(numData);
}
void TimeMinSetFromBluetooth(String commandData)
{
    int numData = 0;
    for (uint8_t i = 0; i < commandData.length() ; i++)
    {
        if (commandData[i] == ',')
            break;
        else
            numData = numData * 10 + (commandData[i] - '0');
    }
    rtc.minutes(numData);
}
void TimeSecSetFromBluetooth(String commandData)
{
    int numData = 0;
    for (uint8_t i = 0; i < commandData.length() ; i++)
    {
        if (commandData[i] == ',')
            break;
        else
            numData = numData * 10 + (commandData[i] - '0');
    }
    rtc.seconds(numData);
}
void TimeDaySetFromBluetooth(String commandData)
{
    int numData = 0;
    for (uint8_t i = 0; i < commandData.length() ; i++)
    {
        if (commandData[i] == ',')
            break;
        else
            numData = numData * 10 + (commandData[i] - '0');
    }
    rtc.day(numData);
}

void TimePrintToLcd()
{
    lcd.setFont(u8g_font_gdr20n);
    if (rtc.time().hr % 100 >= 10)
    {
        lcd.setPrintPos(2, 26);
        lcd.print(rtc.time().hr % 100);
    }
    else
    {
        lcd.drawStr(2, 26, "0");
        lcd.setPrintPos(18, 26);
        lcd.print(rtc.time().hr % 100);
    }
    lcd.drawStr(32, 23, ":");
    if (rtc.time().min % 100 >= 10)
    {
        lcd.setPrintPos(40, 26);
        lcd.print(rtc.time().min % 100);
    }
    else
    {
        lcd.drawStr(40, 26, "0");
        lcd.setPrintPos(56, 26);
        lcd.print(rtc.time().min % 100);
    }
    lcd.setFont(u8g_font_tpss);
    lcd.setPrintPos(74, 26);
    lcd.print(rtc.time().sec);

    if (rtc.time().yr % 100 >= 10)
    {
        lcd.setPrintPos(79, 12);
        lcd.print(rtc.time().yr % 100);
    }
    else
    {
        lcd.drawStr(79, 12, "0");
        lcd.setPrintPos(85, 12);
        lcd.print(rtc.time().yr % 100);
    }
    lcd.drawStr(90, 12, "-");
    if (rtc.time().mon % 100 >= 10)
    {
        lcd.setPrintPos(97, 12);
        lcd.print(rtc.time().mon);
    }
    else
    {
        lcd.drawStr(97, 12, "0");
        lcd.setPrintPos(103, 12);
        lcd.print(rtc.time().mon);
    }
    lcd.drawStr(108, 12, "-");
    if (rtc.time().date % 100 >= 10)
    {
        lcd.setPrintPos(115, 12);
        lcd.print(rtc.time().date);
    }
    else
    {
        lcd.drawStr(115, 12, "0");
        lcd.setPrintPos(121, 12);
        lcd.print(rtc.time().date);
    }

    String tempStr = " ";
    switch (rtc.time().day)
    {
    case 1:
        tempStr = "Mon";
        break;
    case 2:
        tempStr = "Tues";
        break;
    case 3:
        tempStr = "Wed";
        break;
    case 4:
        tempStr = "Thur";
        break;
    case 5:
        tempStr = "Fri";
        break;
    case 6:
        tempStr = "Sat";
        break;
    case 7:
        tempStr = "Sun";
        break;
    default:
        tempStr = ">-<";
        break;
    }
    lcd.setPrintPos(100, 26);
    lcd.print(tempStr);
}
//LCD函数
void LCDInit()
{
    Serial.println("LCD Set Begin");
    rtc.halt(false);//设置rtc工作方式
    lcd.setColorIndex(2);// 亮度调节
    Serial.println("LCD Set OK");
}
void draw()
{
    lcd.firstPage();
    if (lcdOnOffFlag)//关闭LCD
    {
        do
        {
            draw0();
        }
        while ( lcd.nextPage() );
    }
    else//打开LCD并显示
    {
        if (setThresholdFromLcdFlag == 0) //不是按键阙值设置模式
        {
            thresholdTemporary = 0;
            if (lcdPageFlag == 1)
            {
                do
                {
                    draw1();
                }
                while ( lcd.nextPage() );
            }
            else if (lcdPageFlag == 2)
            {
                do
                {
                    draw2();
                }
                while ( lcd.nextPage() );
            }
        }
        else if (setThresholdFromLcdFlag == 5)
        {
            do
            {
                drawD5();
            }
            while ( lcd.nextPage() );
        }
        else if (setThresholdFromLcdFlag == 1)
        {
            do
            {
                drawD1();
            }
            while ( lcd.nextPage() );
        }
        else if (setThresholdFromLcdFlag == 2)
        {
            do
            {
                drawD2();
            }
            while ( lcd.nextPage() );
        }
        else if (setThresholdFromLcdFlag == 3)
        {
            do
            {
                drawD3();
            }
            while ( lcd.nextPage() );
        }
        else if (setThresholdFromLcdFlag == 4)
        {
            do
            {
                drawD4();
            }
            while ( lcd.nextPage() );
        }
        else if (setThresholdFromLcdFlag == 11)
        {
            do
            {
                drawD11();
            }
            while ( lcd.nextPage() );
        }
        else if (setThresholdFromLcdFlag == 12)
        {
            do
            {
                drawD12();
            }
            while ( lcd.nextPage() );
        }
        else if (setThresholdFromLcdFlag == 21)
        {
            do
            {
                drawD21();
            }
            while ( lcd.nextPage() );
        }
        else if (setThresholdFromLcdFlag == 22)
        {
            do
            {
                drawD22();
            }
            while ( lcd.nextPage() );
        }
        else if (setThresholdFromLcdFlag == 31)
        {
            do
            {
                drawD31();
            }
            while ( lcd.nextPage() );
        }
        else if (setThresholdFromLcdFlag == 32)
        {
            do
            {
                drawD32();
            }
            while ( lcd.nextPage() );
        }
        else if (setThresholdFromLcdFlag == 41)
        {
            do
            {
                drawD41();
            }
            while ( lcd.nextPage() );
        }
        else if (setThresholdFromLcdFlag == 42)
        {
            do
            {
                drawD42();
            }
            while ( lcd.nextPage() );
        }
        else if (setThresholdFromLcdFlag == 60)
        {
            do
            {
                drawD60();
            }
            while ( lcd.nextPage() );
        }
        else if (setThresholdFromLcdFlag == 61)
        {
            do
            {
                drawD61();
            }
            while ( lcd.nextPage() );
        }
        else if (setThresholdFromLcdFlag == 62)
        {
            do
            {
                drawD62();
            }
            while ( lcd.nextPage() );
        }
    }
}
void draw0(void) {}
void draw1(void)
{
    lcd.drawFrame(0, 0, 128, 64);
    lcd.drawLine(1, 28, 127, 28);
    TimePrintToLcd();
    DHTPrintToLcd();
    LDRPrinttoLcd();
    SHSPrinttoLcd();
    //Serial.println("Draw Page1");
}
void draw2(void)
{
    lcd.setFont(u8g_font_tpss);
    lcd.drawFrame(0, 0, 128, 64);
    lcd.drawXBMP(2, 1, 14, 14, queHz);
    lcd.drawXBMP(17, 1, 14, 14, zhiHz);
    lcd.drawLine(1, 16, 127, 16);
    if (modeFlag)
        lcd.drawStr(36, 14, "Hand");
    else
        lcd.drawStr(36, 14, "Auto");
    char buf[12];
    sprintf(buf, "AH m:%d M:%d", thresholdOfAirHumi_MIN, thresholdOfAirHumi_MAX);
    lcd.drawStr(2, 26, buf);
    sprintf(buf, "Te m:%d M:%d", thresholdOfTemp_MIN, thresholdOfTemp_MAX);
    lcd.drawStr(2, 38, buf);
    sprintf(buf, "SH m:%d M:%d", thresholdOfSoilHumi_MIN, thresholdOfSoilHumi_MAX);
    lcd.drawStr(2, 50, buf);
    sprintf(buf, "Li  m:%d M:%d", thresholdOfLigh_MIN, thresholdOfLigh_MAX);
    lcd.drawStr(2, 62, buf);
    lcd.drawLine(68, 1, 68, 63);

    int te=dht.temperature;
    int ah=dht.humidity;
    int sh=shs.getSoilHumi();
    int li=ldr.getLight();
    lcd.drawXBMP(69, 1, 14, 14, dangHz);
    lcd.drawXBMP(83, 1, 14, 14, qianHz);
    lcd.setPrintPos(77, 26);
    lcd.print(te);
    lcd.setPrintPos(77, 38);
    lcd.print(ah);
    lcd.setPrintPos(77, 50);
    lcd.print(sh);
    lcd.setPrintPos(77, 62);
    lcd.print(li);
    lcd.drawLine(97, 1, 97, 63);

    lcd.drawXBMP(99, 1, 14, 14, zhuangHz);
    lcd.drawXBMP(113, 1, 14, 14, taiHz);
    if (ah < thresholdOfAirHumi_MIN)
        lcd.drawStr(100, 26, "LOW");
    else if (ah > thresholdOfAirHumi_MAX)
        lcd.drawStr(100, 26, "HIGH");
    else
        lcd.drawStr(100, 26, "NORM");

    if (te < thresholdOfTemp_MIN)
        lcd.drawStr(100, 38, "LOW");
    else if (te > thresholdOfTemp_MAX)
        lcd.drawStr(100, 38, "HIGH");
    else
        lcd.drawStr(100, 38, "NORM");

    if (sh < thresholdOfSoilHumi_MIN)
        lcd.drawStr(100, 50, "LOW");
    else if (sh > thresholdOfSoilHumi_MAX)
        lcd.drawStr(100, 50, "HIGH");
    else
        lcd.drawStr(100, 50, "NORM");

    if (li < thresholdOfLigh_MIN)
        lcd.drawStr(100, 62, "LOW");
    else if (li > thresholdOfLigh_MAX)
        lcd.drawStr(100, 62, "HIGH");
    else
        lcd.drawStr(100, 62, "NORM");
    //Serial.println("Draw Page2");
}
void drawD5()
{
    lcd.setFont(u8g_font_tpss);
    lcd.drawFrame(0, 0, 128, 64);
    lcd.drawXBMP(2, 1, 14, 14, queHz);
    lcd.drawXBMP(17, 1, 14, 14, zhiHz);
    lcd.drawXBMP(32, 1, 14, 14, sheHz);
    lcd.drawXBMP(47, 1, 14, 14, zhi2Hz);
    lcd.drawXBMP(62, 1, 14, 14, xuanHz);
    lcd.drawXBMP(77, 1, 14, 14, zeHz);
    lcd.drawLine(1, 16, 127, 16);
    lcd.drawStr(5, 28, "1->AH");
    lcd.drawStr(64, 28, "2->Te");
    lcd.drawStr(5, 40, "3->SH");
    lcd.drawStr(64, 40, "4->Li");
    lcd.drawStr(5, 52, "D->Exit");
}
void drawD1()
{
    lcd.setFont(u8g_font_tpss);
    lcd.drawFrame(0, 0, 128, 64);
    lcd.drawXBMP(2, 1, 14, 14, kongHz);
    lcd.drawXBMP(17, 1, 14, 14, shiHz);
    lcd.drawXBMP(32, 1, 14, 14, queHz);
    lcd.drawXBMP(47, 1, 14, 14, zhiHz);
    lcd.drawXBMP(62, 1, 14, 14, sheHz);
    lcd.drawXBMP(77, 1, 14, 14, zhi2Hz);
    lcd.drawXBMP(92, 1, 14, 14, xuanHz);
    lcd.drawXBMP(107, 1, 14, 14, zeHz);
    lcd.drawLine(1, 16, 127, 16);
    lcd.drawStr(5, 28, "1->AHmin");
    lcd.drawStr(5, 40, "2->AHmax");
    lcd.drawStr(5, 52, "D->Exit");
}
void drawD11()
{
    lcd.setFont(u8g_font_tpss);
    lcd.drawFrame(0, 0, 128, 64);
    lcd.drawStr(3, 14, "AHmin");
    lcd.drawXBMP(32, 1, 14, 14, queHz);
    lcd.drawXBMP(47, 1, 14, 14, zhiHz);
    lcd.drawXBMP(62, 1, 14, 14, sheHz);
    lcd.drawXBMP(77, 1, 14, 14, zhi2Hz);
    lcd.drawLine(1, 16, 127, 16);
    char buf[16];;
    sprintf(buf, "AHmin=%d  ?", thresholdTemporary);
    lcd.drawStr(5, 28, buf);
    lcd.drawStr(5, 50, "D->Exit  Range:0-100");
    lcd.drawStr(5, 62, "#->confirm *->clear");
}
void drawD12()
{
    lcd.setFont(u8g_font_tpss);
    lcd.drawFrame(0, 0, 128, 64);
    lcd.drawStr(3, 14, "AHmax");
    lcd.drawXBMP(32, 1, 14, 14, queHz);
    lcd.drawXBMP(47, 1, 14, 14, zhiHz);
    lcd.drawXBMP(62, 1, 14, 14, sheHz);
    lcd.drawXBMP(77, 1, 14, 14, zhi2Hz);
    lcd.drawLine(1, 16, 127, 16);
    char buf[16];;
    sprintf(buf, "AHmax=%d  ?", thresholdTemporary);
    lcd.drawStr(5, 28, buf);
    lcd.drawStr(5, 50, "D->Exit  Range:0-100");
    lcd.drawStr(5, 62, "#->confirm *->clear");
}
void drawD2()
{
    lcd.setFont(u8g_font_tpss);
    lcd.drawFrame(0, 0, 128, 64);
    lcd.drawXBMP(2, 1, 14, 14, wenHz);
    lcd.drawXBMP(17, 1, 14, 14, duHz);
    lcd.drawXBMP(32, 1, 14, 14, queHz);
    lcd.drawXBMP(47, 1, 14, 14, zhiHz);
    lcd.drawXBMP(62, 1, 14, 14, sheHz);
    lcd.drawXBMP(77, 1, 14, 14, zhi2Hz);
    lcd.drawXBMP(92, 1, 14, 14, xuanHz);
    lcd.drawXBMP(107, 1, 14, 14, zeHz);
    lcd.drawLine(1, 16, 127, 16);
    lcd.drawStr(5, 28, "1->Temin");
    lcd.drawStr(5, 40, "2->Temax");
    lcd.drawStr(5, 52, "D->Exit");
}
void drawD21()
{
    lcd.setFont(u8g_font_tpss);
    lcd.drawFrame(0, 0, 128, 64);
    lcd.drawStr(3, 14, "Temin");
    lcd.drawXBMP(32, 1, 14, 14, queHz);
    lcd.drawXBMP(47, 1, 14, 14, zhiHz);
    lcd.drawXBMP(62, 1, 14, 14, sheHz);
    lcd.drawXBMP(77, 1, 14, 14, zhi2Hz);
    lcd.drawLine(1, 16, 127, 16);
    char buf[16];;
    sprintf(buf, "Temin=%d  ?", thresholdTemporary);
    lcd.drawStr(5, 28, buf);
    lcd.drawStr(5, 50, "D->Exit  Range:0-100");
    lcd.drawStr(5, 62, "#->confirm *->clear");
}
void drawD22()
{
    lcd.setFont(u8g_font_tpss);
    lcd.drawFrame(0, 0, 128, 64);
    lcd.drawStr(3, 14, "Temax");
    lcd.drawXBMP(32, 1, 14, 14, queHz);
    lcd.drawXBMP(47, 1, 14, 14, zhiHz);
    lcd.drawXBMP(62, 1, 14, 14, sheHz);
    lcd.drawXBMP(77, 1, 14, 14, zhi2Hz);
    lcd.drawLine(1, 16, 127, 16);
    char buf[16];;
    sprintf(buf, "Temax=%d  ?", thresholdTemporary);
    lcd.drawStr(5, 28, buf);
    lcd.drawStr(5, 50, "D->Exit  Range:0-100");
    lcd.drawStr(5, 62, "#->confirm *->clear");
}
void drawD3()
{
    lcd.setFont(u8g_font_tpss);
    lcd.drawFrame(0, 0, 128, 64);
    lcd.drawXBMP(2, 1, 14, 14, tuHz);
    lcd.drawXBMP(17, 1, 14, 14, shiHz);
    lcd.drawXBMP(32, 1, 14, 14, queHz);
    lcd.drawXBMP(47, 1, 14, 14, zhiHz);
    lcd.drawXBMP(62, 1, 14, 14, sheHz);
    lcd.drawXBMP(77, 1, 14, 14, zhi2Hz);
    lcd.drawXBMP(92, 1, 14, 14, xuanHz);
    lcd.drawXBMP(107, 1, 14, 14, zeHz);
    lcd.drawLine(1, 16, 127, 16);
    lcd.drawStr(5, 28, "1->SHmin");
    lcd.drawStr(5, 40, "2->SHmax");
    lcd.drawStr(5, 52, "D->Exit");
}
void drawD31()
{
    lcd.setFont(u8g_font_tpss);
    lcd.drawFrame(0, 0, 128, 64);
    lcd.drawStr(3, 14, "SHmin");
    lcd.drawXBMP(32, 1, 14, 14, queHz);
    lcd.drawXBMP(47, 1, 14, 14, zhiHz);
    lcd.drawXBMP(62, 1, 14, 14, sheHz);
    lcd.drawXBMP(77, 1, 14, 14, zhi2Hz);
    lcd.drawLine(1, 16, 127, 16);
    char buf[16];;
    sprintf(buf, "SHmin=%d  ?", thresholdTemporary);
    lcd.drawStr(5, 28, buf);
    lcd.drawStr(5, 50, "D->Exit  Range:0-100");
    lcd.drawStr(5, 62, "#->confirm *->clear");
}
void drawD32()
{
    lcd.setFont(u8g_font_tpss);
    lcd.drawFrame(0, 0, 128, 64);
    lcd.drawStr(3, 14, "SHmax");
    lcd.drawXBMP(32, 1, 14, 14, queHz);
    lcd.drawXBMP(47, 1, 14, 14, zhiHz);
    lcd.drawXBMP(62, 1, 14, 14, sheHz);
    lcd.drawXBMP(77, 1, 14, 14, zhi2Hz);
    lcd.drawLine(1, 16, 127, 16);
    char buf[16];;
    sprintf(buf, "SHmax=%d  ?", thresholdTemporary);
    lcd.drawStr(5, 28, buf);
    lcd.drawStr(5, 50, "D->Exit  Range:0-100");
    lcd.drawStr(5, 62, "#->confirm *->clear");
}
void drawD4()
{
    lcd.setFont(u8g_font_tpss);
    lcd.drawFrame(0, 0, 128, 64);
    lcd.drawXBMP(2, 1, 14, 14, guangHz);
    lcd.drawXBMP(17, 1, 14, 14, zhaoHz);
    lcd.drawXBMP(32, 1, 14, 14, queHz);
    lcd.drawXBMP(47, 1, 14, 14, zhiHz);
    lcd.drawXBMP(62, 1, 14, 14, sheHz);
    lcd.drawXBMP(77, 1, 14, 14, zhi2Hz);
    lcd.drawXBMP(92, 1, 14, 14, xuanHz);
    lcd.drawXBMP(107, 1, 14, 14, zeHz);
    lcd.drawLine(1, 16, 127, 16);
    lcd.drawStr(5, 28, "1->Limin");
    lcd.drawStr(5, 40, "2->Limax");
    lcd.drawStr(5, 52, "D->Exit");
}
void drawD41()
{
    lcd.setFont(u8g_font_tpss);
    lcd.drawFrame(0, 0, 128, 64);
    lcd.drawStr(3, 14, "Limin");
    lcd.drawXBMP(32, 1, 14, 14, queHz);
    lcd.drawXBMP(47, 1, 14, 14, zhiHz);
    lcd.drawXBMP(62, 1, 14, 14, sheHz);
    lcd.drawXBMP(77, 1, 14, 14, zhi2Hz);
    lcd.drawLine(1, 16, 127, 16);
    char buf[16];;
    sprintf(buf, "Limin=%d  ?", thresholdTemporary);
    lcd.drawStr(5, 28, buf);
    lcd.drawStr(5, 50, "D->Exit  Range:0-100");
    lcd.drawStr(5, 62, "#->confirm *->clear");
}
void drawD42()
{
    lcd.setFont(u8g_font_tpss);
    lcd.drawFrame(0, 0, 128, 64);
    lcd.drawStr(3, 14, "Limax");
    lcd.drawXBMP(32, 1, 14, 14, queHz);
    lcd.drawXBMP(47, 1, 14, 14, zhiHz);
    lcd.drawXBMP(62, 1, 14, 14, sheHz);
    lcd.drawXBMP(77, 1, 14, 14, zhi2Hz);
    lcd.drawLine(1, 16, 127, 16);
    char buf[16];;
    sprintf(buf, "Limax=%d  ?", thresholdTemporary);
    lcd.drawStr(5, 28, buf);
    lcd.drawStr(5, 50, "D->Exit  Range:0-100");
    lcd.drawStr(5, 62, "#->confirm *->clear");
}
void drawD60()
{
    lcd.setFont(u8g_font_tpss);
    lcd.drawFrame(0, 0, 128, 64);
    lcd.drawXBMP(17, 6, 14, 14, queHz);
    lcd.drawXBMP(32, 6, 14, 14, zhiHz);
    lcd.drawXBMP(47, 6, 14, 14, sheHz);
    lcd.drawXBMP(62, 6, 14, 14, zhi2Hz);
    lcd.drawXBMP(77, 6, 14, 14, chengHz);
    lcd.drawXBMP(92, 6, 14, 14, gongHz);
    lcd.drawStr(5, 62, "D->Exit");
}
void drawD61()
{
    lcd.setFont(u8g_font_tpss);
    lcd.drawFrame(0, 0, 128, 64);
    lcd.drawXBMP(17, 6, 14, 14, queHz);
    lcd.drawXBMP(32, 6, 14, 14, zhiHz);
    lcd.drawXBMP(47, 6, 14, 14, sheHz);
    lcd.drawXBMP(62, 6, 14, 14, zhi2Hz);
    lcd.drawXBMP(77, 6, 14, 14, shi2Hz);
    lcd.drawXBMP(92, 6, 14, 14, baiHz);
    lcd.drawStr(5, 34, "Reason:");
    lcd.drawStr(5, 48, "Range in 0-100");
    lcd.drawStr(5, 62, "D->Exit");
}
void drawD62()
{
    lcd.setFont(u8g_font_tpss);
    lcd.drawFrame(0, 0, 128, 64);
    lcd.drawXBMP(17, 6, 14, 14, queHz);
    lcd.drawXBMP(32, 6, 14, 14, zhiHz);
    lcd.drawXBMP(47, 6, 14, 14, sheHz);
    lcd.drawXBMP(62, 6, 14, 14, zhi2Hz);
    lcd.drawXBMP(77, 6, 14, 14, shi2Hz);
    lcd.drawXBMP(92, 6, 14, 14, baiHz);
    lcd.drawStr(5, 34, "Reason:");
    lcd.drawStr(5, 48, "Tmin must <=Tmax");
    lcd.drawStr(5, 62, "D->Exit");
}
//DHT函数
uint8_t DHTGetData()
{
    int chk = dht.read(DHT_DAT_PIN);
    //检测是否正常接受数据,返回0正常接受
    switch (chk)
    {
    case DHTLIB_OK:
        return 0;
    case DHTLIB_ERROR_CHECKSUM:
    {
        Serial3.println("401");
        Serial.println("DHT Data:Checksum error");
    }
    break;
    case DHTLIB_ERROR_TIMEOUT:
    {
        Serial3.println("402");
        Serial.println("DHT Data:Time out error");
    }
    break;
    default:
    {
        Serial3.println("403");
        Serial.println("DHT Data:Unknown error");
    }
    break;
    }
    return 1;
}
void DHTHumiPrint()
{
    //输出环境湿度信息
    Serial3.println(dht.humidity);
    Serial.print("EnvirHumi=");
    Serial.println(dht.humidity);
}
void DHTTempPrint()
{
    //输出温度信息
    Serial3.println(dht.temperature);
    Serial.print("Temp=");
    Serial.println(dht.temperature);
}
void DHTPrintToLcd()
{
    if (!dhtFlag)
    {
        lcd.setFont(u8g_font_tpss);
        lcd.drawXBMP(2, 30, 14, 14, wenHz);
        lcd.drawXBMP(17, 30, 14, 14, duHz);
        lcd.drawStr(32, 42, ":");
        lcd.setPrintPos(35, 44);
        lcd.print(dht.temperature);
        lcd.drawStr(47, 44, "C");
        lcd.drawLine(56, 29, 56, 63);
        lcd.drawXBMP(58, 30, 14, 14, shiHz);
        lcd.drawXBMP(58, 48, 14, 14, duHz);
        lcd.drawLine(72, 29, 72, 63);
        lcd.drawLine(73, 46, 128, 46);
        lcd.drawXBMP(74, 30, 14, 14, kongHz);
        lcd.drawXBMP(88, 30, 14, 14, qiHz);
        lcd.drawStr(102, 42, ":");
        lcd.setPrintPos(105, 44);
        lcd.print(dht.humidity);
        lcd.drawStr(120, 44, "%");
    }
    else
    {
        lcd.setFont(u8g_font_tpss);
        lcd.drawXBMP(2, 30, 14, 14, wenHz);
        lcd.drawXBMP(17, 30, 14, 14, duHz);
        lcd.drawStr(32, 42, ":");
        lcd.drawStr(35, 44, ">-<");
        lcd.drawLine(56, 29, 56, 63);
        lcd.drawXBMP(58, 30, 14, 14, shiHz);
        lcd.drawXBMP(58, 48, 14, 14, duHz);
        lcd.drawLine(72, 29, 72, 63);
        lcd.drawLine(73, 46, 128, 46);
        lcd.drawXBMP(74, 30, 14, 14, kongHz);
        lcd.drawXBMP(88, 30, 14, 14, qiHz);
        lcd.drawStr(102, 42, ":");
        lcd.drawStr(105, 44, ">-<");
    }
}
//LDR函数
void LDRPrint()
{
    Serial3.println(ldr.getLight());
    Serial.print("Light=");
    Serial.println(ldr.getLight());
}
void LDRPrinttoLcd()
{
    lcd.setFont(u8g_font_tpss);
    lcd.drawXBMP(2, 48, 14, 14, guangHz);
    lcd.drawXBMP(17, 48, 14, 14, zhaoHz);
    lcd.drawStr(32, 60, ":");
    lcd.setPrintPos(35, 62);
    lcd.print(ldr.getLight());
    lcd.drawStr(47, 62, "%");
}
//SHS函数
void SHSPrint()
{
    Serial3.println(shs.getSoilHumi());
    Serial.print("SoilHumi=");
    Serial.println(shs.getSoilHumi());
}
void SHSPrinttoLcd()
{
    lcd.drawXBMP(74, 48, 14, 14, tuHz);
    lcd.drawXBMP(88, 48, 14, 14, rangHz);
    lcd.drawStr(102, 60, ":");
    lcd.setPrintPos(105, 62);
    lcd.print(shs.getSoilHumi());
    lcd.drawStr(120, 62, "%");
}
//////RGBLED控制
////void RGBControl()
////{
////    /*蓝色代表高于阙值最大值，黄色代表低于阙值最小值
////      绿色代表在阙值范围内，红色代表传感器数据采集异常
////    */
////    //灯0显示温度状态
////    if (!dhtFlag)
////    {
////        if (dht.temperature > thresholdOfTemp_MAX)
////            rgbled.setPixelColor(0, 0, 0, 255); //第0个灯蓝色
////        else if (dht.temperature < thresholdOfTemp_MIN)
////            rgbled.setPixelColor(0, 255, 255, 0); //第0个灯黄色
////        else
////            rgbled.setPixelColor(0, 0, 255, 0); //第0个灯绿色
////    }
////    else
////        rgbled.setPixelColor(0, 255, 0, 0); //第0个灯红色
////
////    //灯1显示环境湿度状态
////    if (!dhtFlag)
////    {
////        if (dht.temperature > thresholdOfAirHumi_MAX)
////            rgbled.setPixelColor(1, 0, 0, 255); //第1个灯蓝色
////        else if (dht.temperature < thresholdOfAirHumi_MIN)
////            rgbled.setPixelColor(1, 255, 255, 0); //第1个灯黄色
////        else
////            rgbled.setPixelColor(1, 0, 255, 0); //第1个灯绿色
////    }
////    else
////        rgbled.setPixelColor(1, 255, 0, 0); //第1个灯红色
////
////    //灯2显示土壤湿度状态
////
////    if (shs.getSoilHumi() > thresholdOfSoilHumi_MAX)
////        rgbled.setPixelColor(2, 0, 0, 255); //第2个灯蓝色
////    else if (shs.getSoilHumi() < thresholdOfSoilHumi_MIN)
////        rgbled.setPixelColor(2, 255, 255, 0); //第2个灯黄色
////    else
////        rgbled.setPixelColor(2, 0, 255, 0); //第2个灯绿色
////
////    //灯3显示光照状态
////
////    if (ldr.getLight() > thresholdOfLigh_MAX)
////        rgbled.setPixelColor(3, 0, 0, 255); //第3个灯蓝色
////    else if (ldr.getLight() < thresholdOfLigh_MIN)
////        rgbled.setPixelColor(3, 255, 255, 0); //第3个灯黄色
////    else
////        rgbled.setPixelColor(3, 0, 255, 0); //第3个灯绿色
////
////    rgbled.show();
////}

//阙值控制
void SETthresholdOfTemp_MAX(String commandData)
{
    int numData = 0;
    for (uint8_t i = 0; i < commandData.length() ; i++)
    {
        if (commandData[i] == ',' || commandData[i] == ' ' || commandData[i] == 0x11 || commandData[i] == 0x13)
            break;
        else
            numData = numData * 10 + (commandData[i] - '0');
    }
    if (numData >= thresholdOfTemp_MIN)
        thresholdOfTemp_MAX = numData;
    else
    {
        Serial.println("thresholdOfTemp_MAX must larger than thresholdOfTemp_MIN");
        Serial3.println("405");
    }
}
void SETthresholdOfTemp_MIN(String commandData)
{
    int numData = 0;
    for (uint8_t i = 0; i < commandData.length() ; i++)
    {
        if (commandData[i] == ',' || commandData[i] == ' ' || commandData[i] == 0x11 || commandData[i] == 0x13)
            break;
        else
            numData = numData * 10 + (commandData[i] - '0');
    }
    if (numData <= thresholdOfTemp_MAX)
        thresholdOfTemp_MIN = numData;
    else
    {
        Serial.println("thresholdOfTemp_MAX must larger than thresholdOfTemp_MIN");
        Serial3.println("405");
    }
}
void SETthresholdOfAirHumi_MAX(String commandData)
{
    int numData = 0;
    for (uint8_t i = 0; i < commandData.length() ; i++)
    {
        if (commandData[i] == ',' || commandData[i] == ' ' || commandData[i] == 0x11 || commandData[i] == 0x13)
            break;
        else
            numData = numData * 10 + (commandData[i] - '0');
    }
    if (numData >= thresholdOfAirHumi_MIN)
        thresholdOfAirHumi_MAX = numData;
    else
    {
        Serial.println("thresholdOfAirHumi_MAX must larger than thresholdOfAirHumi_MIN");
        Serial3.println("405");
    }
}
void SETthresholdOfAirHumi_MIN(String commandData)
{
    int numData = 0;
    for (uint8_t i = 0; i < commandData.length() ; i++)
    {
        if (commandData[i] == ',' || commandData[i] == ' ' || commandData[i] == 0x11 || commandData[i] == 0x13)
            break;
        else
            numData = numData * 10 + (commandData[i] - '0');
    }
    if (numData <= thresholdOfAirHumi_MAX)
        thresholdOfAirHumi_MIN = numData;
    else
    {
        Serial.println("thresholdOfAirHumi_MAX must larger than thresholdOfAirHumi_MIN");
        Serial3.println("405");
    }
}
void SETthresholdOfSoilHumi_MAX(String commandData)
{
    int numData = 0;
    for (uint8_t i = 0; i < commandData.length() ; i++)
    {
        if (commandData[i] == ',' || commandData[i] == ' ' || commandData[i] == 0x11 || commandData[i] == 0x13)
            break;
        else
            numData = numData * 10 + (commandData[i] - '0');
    }
    if ((numData <= 100) && (numData >= 0))
    {
        if (numData >= thresholdOfSoilHumi_MIN)
            thresholdOfSoilHumi_MAX = numData;
        else
        {
            Serial.println("thresholdOfSoilHumi_MAX must larger than thresholdOfSoilHumi_MIN");
            Serial3.println("405");
        }
    }
    else
    {
        Serial.println("thresholdOfSoilHumi_MAX must larger than 0 and smaller than 100");
        Serial3.println("406");
    }
}
void SETthresholdOfSoilHumi_MIN(String commandData)
{
    int numData = 0;
    for (uint8_t i = 0; i < commandData.length() ; i++)
    {
        if (commandData[i] == ',' || commandData[i] == ' ' || commandData[i] == 0x11 || commandData[i] == 0x13)
            break;
        else
            numData = numData * 10 + (commandData[i] - '0');
    }
    if ((numData <= 100) && (numData >= 0))
    {
        if (numData <= thresholdOfSoilHumi_MAX)
            thresholdOfSoilHumi_MIN = numData;
        else
        {
            Serial.println("thresholdOfSoilHumi_MAX must larger than thresholdOfSoilHumi_MIN");
            Serial3.println("405");
        }
    }
    else
    {
        Serial.println("thresholdOfSoilHumi_MIN must larger than 0 and smaller than 100");
        Serial3.println("406");
    }
}
void SETthresholdOfLigh_MAX(String commandData)
{
    int numData = 0;
    for (uint8_t i = 0; i < commandData.length() ; i++)
    {
        if (commandData[i] == ',' || commandData[i] == ' ' || commandData[i] == 0x11 || commandData[i] == 0x13)
            break;
        else
            numData = numData * 10 + (commandData[i] - '0');
    }
    if ((numData <= 100) && (numData >= 0))
    {
        if (numData >= thresholdOfLigh_MIN)
            thresholdOfLigh_MAX = numData;
        else
        {
            Serial.println("thresholdOfLigh_MAX must larger than thresholdOfLigh_MIN");
            Serial3.println("405");
        }
    }
    else
    {
        Serial.println("thresholdOfLigh_MAX must larger than 0 and smaller than 100");
        Serial3.println("406");
    }
}
void SETthresholdOfLigh_MIN(String commandData)
{
    int numData = 0;
    for (uint8_t i = 0; i < commandData.length() ; i++)
    {
        if (commandData[i] == ',' || commandData[i] == ' ' || commandData[i] == 0x11 || commandData[i] == 0x13)
            break;
        else
            numData = numData * 10 + (commandData[i] - '0');
    }
    if ((numData <= 100) && (numData >= 0))
    {
        if (numData <= thresholdOfLigh_MAX)
            thresholdOfLigh_MIN = numData;
        else
        {
            Serial.println("thresholdOfLigh_MAX must larger than thresholdOfLigh_MIN");
            Serial3.println("405");
        }
    }
    else
    {
        Serial.println("thresholdOfLigh_MIN must larger than 0 and smaller than 100");
        Serial3.println("406");
    }
}
void GETthresholdOfTemp()
{
    Serial3.println(thresholdOfTemp_MIN);
    Serial3.println(thresholdOfTemp_MAX);
    Serial.print("thresholdOfTemp_MIN=");
    Serial.println(thresholdOfTemp_MIN);
    Serial.print("thresholdOfTemp_MAX=");
    Serial.println(thresholdOfTemp_MAX);
}
void GETthresholdOfEnvirHumi()
{
    Serial3.println(thresholdOfAirHumi_MIN);
    Serial3.println(thresholdOfAirHumi_MAX);
    Serial.print("thresholdOfAirHumi_MIN=");
    Serial.println(thresholdOfAirHumi_MIN);
    Serial.print("thresholdOfAirHumi_MAX=");
    Serial.println(thresholdOfAirHumi_MAX);
}
void GETthresholdOfSoilHumi()
{
    Serial3.println(thresholdOfSoilHumi_MIN);
    Serial3.println(thresholdOfSoilHumi_MAX);
    Serial.print("thresholdOfSoilHumi_MIN=");
    Serial.println(thresholdOfSoilHumi_MIN);
    Serial.print("thresholdOfSoilHumi_MAX=");
    Serial.println(thresholdOfSoilHumi_MAX);
}
void GETthresholdOfLigh()
{
    Serial3.println(thresholdOfLigh_MIN);
    Serial3.println(thresholdOfLigh_MAX);
    Serial.print("thresholdOfLigh_MIN=");
    Serial.println(thresholdOfLigh_MIN);
    Serial.print("thresholdOfLigh_MAX=");
    Serial.println(thresholdOfLigh_MAX);
}
//HC05函数
void BluetoothInit()
{
    //蓝牙模块HC-05的初始化
    pinMode(HC05_AT_PIN, OUTPUT);
    digitalWrite(HC05_AT_PIN, HIGH); //将AT位置高，HC05进入AT指令模式
    Serial.println("AT Set Begin");//PC串口打印提示语
    Serial3.begin(38400);//HC05在AT指令模式下的固定通信波特率
    delay(100);
    Serial3.println("AT");
    delay(100);
    Serial3.println("AT+NAME=SmaPot");//命名模块名
    delay(100);
    Serial3.println("AT+ROLE=0");//设置主从模式：0从机，1主机
    delay(100);
    Serial3.println("AT+PSWD=8888");//设置配对密码
    delay(100);
    Serial3.println("AT+UART=9600,0,0");//设置HC05的传输波特率9600，停止位1，校验位无
    delay(100);
    Serial3.println("AT+RMAAD");//清空配对列表
    Serial.println("AT Set OK");//PC串口打印提示语
    Serial3.begin(9600);//将UART3的串口波特率改为9600
}
void GetmodeFlag()
{
    //0为自动模式，1为手动模式
    if (modeFlag == 0)
        Serial.println("Mode=Auto");//PC串口打印提示语
    else if (modeFlag == 1)
        Serial.println("Mode=Hand");//PC串口打印提示语
}
void keyContral()
{
    char keyValue = keypad.getKey();
    if (keyValue)
    {
        Serial.println(keyValue);
        if (setThresholdFromLcdFlag == 0) //不是按键阙值设置模式
        {
            if (keyValue == 'A')//模式A，LCD控制
            {
                lcdPageFlag++;
                if (lcdPageFlag > lcdPageNum)
                    lcdPageFlag = 0;
                if (lcdPageFlag == 0) //第0页为关LCD
                {
                    lcdOnOffFlag = 1;
                    Serial.println("LCD12864 OFF");
                }
                else//其它页为开LCD
                {
                    lcdOnOffFlag = 0;
                    Serial.println("LCD12864 ON");
                }
            }
            else if (keyValue == 'B')//模式B，继电器1控制
            {
                if (controllerFlag1 == 0)
                    controllerFlag1 = 1;
                else if (controllerFlag1 == 1)
                    controllerFlag1 = 0;
            }
            else if (keyValue == 'C')//模式C，继电器2控制
            {
                if (controllerFlag2 == 0)
                    controllerFlag2 = 1;
                else if (controllerFlag2 == 1)
                    controllerFlag2 = 0;
            }
            else if (keyValue == '#')//自动模式
            {
                modeFlag = 0;
            }
            else if (keyValue == '*')//手动模式
            {
                modeFlag = 1;
            }
            else if ((keyValue == 'D') && (lcdOnOffFlag == 0)) //模式D需要LCD打开来支持操作
            {
                setThresholdFromLcdFlag = 5; //下一轮loop进入按键阙值设置模式
            }
        }
        else if (setThresholdFromLcdFlag == 5) //如果是阙值设置选择界面
        {
            if (keyValue == '1')//如果是1,进入AH阙值设置选择界面
                setThresholdFromLcdFlag = 1;
            else if (keyValue == '2') //如果是2,进入Te阙值设置选择界面
                setThresholdFromLcdFlag = 2;
            else if (keyValue == '3') //如果是3,进入SH阙值设置选择界面
                setThresholdFromLcdFlag = 4;
            else if (keyValue == '4') //如果是4,进入Li阙值设置选择界面
                setThresholdFromLcdFlag = 4;
            else if (keyValue == 'D') //如果是D,退出阙值设置选择界面
                setThresholdFromLcdFlag = 0;
        }
        else if (setThresholdFromLcdFlag == 1) //如果是AH阙值设置选择界面
        {
            if (keyValue == '1')//如果是1,进入AHmin阙值设置界面
                setThresholdFromLcdFlag = 11;
            else if (keyValue == '2') //如果是2,进入AHmax阙值设置界面
                setThresholdFromLcdFlag = 12;
            else if (keyValue == 'D') //如果是D,退出阙值设置选择界面
                setThresholdFromLcdFlag = 0;
        }
        else if (setThresholdFromLcdFlag == 2) //如果是Te阙值设置选择界面
        {
            if (keyValue == '1')//如果是1,进入Temin阙值设置界面
                setThresholdFromLcdFlag = 21;
            else if (keyValue == '2') //如果是2,进入Temax阙值设置界面
                setThresholdFromLcdFlag = 22;
            else if (keyValue == 'D') //如果是D,退出阙值设置选择界面
                setThresholdFromLcdFlag = 0;
        }
        else if (setThresholdFromLcdFlag == 3) //如果是SH阙值设置选择界面
        {
            if (keyValue == '1')//如果是1,进入SHmin阙值设置界面
                setThresholdFromLcdFlag = 31;
            else if (keyValue == '2') //如果是2,进入SHmax阙值设置界面
                setThresholdFromLcdFlag = 32;
            else if (keyValue == 'D') //如果是D,退出阙值设置选择界面
                setThresholdFromLcdFlag = 0;
        }
        else if (setThresholdFromLcdFlag == 4) //如果是Li阙值设置选择界面
        {
            if (keyValue == '1')//如果是1,进入Limin阙值设置界面
                setThresholdFromLcdFlag = 41;
            else if (keyValue == '2') //如果是2,进入Limax阙值设置界面
                setThresholdFromLcdFlag = 42;
            else if (keyValue == 'D') //如果是D,退出阙值设置选择界面
                setThresholdFromLcdFlag = 0;
        }
        else if (setThresholdFromLcdFlag == 11)
        {
            if (keyValue == 'D') //如果是D,退出阙值设置选择界面
                setThresholdFromLcdFlag = 0;
            else if (keyValue == '#')
            {
                if (thresholdTemporary > 100 || thresholdTemporary < 0)
                    setThresholdFromLcdFlag = 61;
                else if (thresholdTemporary > thresholdOfAirHumi_MAX)
                    setThresholdFromLcdFlag = 62;
                else
                {
                    thresholdOfAirHumi_MIN = thresholdTemporary;
                    setThresholdFromLcdFlag = 60;
                }
            }

            else if (keyValue == '*')
                thresholdTemporary = 0;
            else if ((keyValue >= '0') && (keyValue <= '9'))
                thresholdTemporary = thresholdTemporary * 10 + (int)(keyValue - '0');
        }
        else if (setThresholdFromLcdFlag == 12)
        {
            if (keyValue == 'D') //如果是D,退出阙值设置选择界面
                setThresholdFromLcdFlag = 0;
            else if (keyValue == '#')
            {
                if (thresholdTemporary > 100 || thresholdTemporary < 0)
                    setThresholdFromLcdFlag = 61;
                else if (thresholdTemporary < thresholdOfAirHumi_MIN)
                    setThresholdFromLcdFlag = 62;
                else
                {
                    thresholdOfAirHumi_MAX = thresholdTemporary;
                    setThresholdFromLcdFlag = 60;
                }
            }
            else if (keyValue == '*')
                thresholdTemporary = 0;
            else if ((keyValue >= '0') && (keyValue <= '9'))
                thresholdTemporary = thresholdTemporary * 10 + (int)(keyValue - '0');
        }
        else if (setThresholdFromLcdFlag == 21)
        {
            if (keyValue == 'D') //如果是D,退出阙值设置选择界面
                setThresholdFromLcdFlag = 0;
            else if (keyValue == '#')
            {
                if (thresholdTemporary > 100 || thresholdTemporary < 0)
                    setThresholdFromLcdFlag = 61;
                else if (thresholdTemporary > thresholdOfTemp_MAX)
                    setThresholdFromLcdFlag = 62;
                else
                {
                    thresholdOfTemp_MIN = thresholdTemporary;
                    setThresholdFromLcdFlag = 60;
                }
            }
            else if (keyValue == '*')
                thresholdTemporary = 0;
            else if ((keyValue >= '0') && (keyValue <= '9'))
                thresholdTemporary = thresholdTemporary * 10 + (int)(keyValue - '0');
        }
        else if (setThresholdFromLcdFlag == 22)
        {
            if (keyValue == 'D') //如果是D,退出阙值设置选择界面
                setThresholdFromLcdFlag = 0;
            else if (keyValue == '#')
            {
                if (thresholdTemporary > 100 || thresholdTemporary < 0)
                    setThresholdFromLcdFlag = 61;
                else if (thresholdTemporary < thresholdOfTemp_MIN)
                    setThresholdFromLcdFlag = 62;
                else
                {
                    thresholdOfTemp_MAX = thresholdTemporary;
                    setThresholdFromLcdFlag = 60;
                }
            }
            else if (keyValue == '*')
                thresholdTemporary = 0;
            else if ((keyValue >= '0') && (keyValue <= '9'))
                thresholdTemporary = thresholdTemporary * 10 + (int)(keyValue - '0');
        }
        else if (setThresholdFromLcdFlag == 31)
        {
            if (keyValue == 'D') //如果是D,退出阙值设置选择界面
                setThresholdFromLcdFlag = 0;
            else if (keyValue == '#')
            {
                if (thresholdTemporary > 100 || thresholdTemporary < 0)
                    setThresholdFromLcdFlag = 61;
                else if (thresholdTemporary > thresholdOfSoilHumi_MAX)
                    setThresholdFromLcdFlag = 62;
                else
                {
                    thresholdOfSoilHumi_MIN = thresholdTemporary;
                    setThresholdFromLcdFlag = 60;
                }
            }
            else if (keyValue == '*')
                thresholdTemporary = 0;
            else if ((keyValue >= '0') && (keyValue <= '9'))
                thresholdTemporary = thresholdTemporary * 10 + (int)(keyValue - '0');
        }
        else if (setThresholdFromLcdFlag == 32)
        {
            if (keyValue == 'D') //如果是D,退出阙值设置选择界面
                setThresholdFromLcdFlag = 0;
            else if (keyValue == '#')
            {
                if (thresholdTemporary > 100 || thresholdTemporary < 0)
                    setThresholdFromLcdFlag = 61;
                else if (thresholdTemporary < thresholdOfSoilHumi_MIN)
                    setThresholdFromLcdFlag = 62;
                else
                {
                    thresholdOfSoilHumi_MAX = thresholdTemporary;
                    setThresholdFromLcdFlag = 60;
                }
            }
            else if (keyValue == '*')
                thresholdTemporary = 0;
            else if ((keyValue >= '0') && (keyValue <= '9'))
                thresholdTemporary = thresholdTemporary * 10 + (int)(keyValue - '0');
        }
        else if (setThresholdFromLcdFlag == 41)
        {
            if (keyValue == 'D') //如果是D,退出阙值设置选择界面
                setThresholdFromLcdFlag = 0;
            else if (keyValue == '#')
            {
                if (thresholdTemporary > 100 || thresholdTemporary < 0)
                    setThresholdFromLcdFlag = 61;
                else if (thresholdTemporary > thresholdOfLigh_MAX)
                    setThresholdFromLcdFlag = 62;
                else
                {
                    thresholdOfLigh_MIN = thresholdTemporary;
                    setThresholdFromLcdFlag = 60;
                }
            }
            else if (keyValue == '*')
                thresholdTemporary = 0;
            else if ((keyValue >= '0') && (keyValue <= '9'))
                thresholdTemporary = thresholdTemporary * 10 + (int)(keyValue - '0');
        }
        else if (setThresholdFromLcdFlag == 42)
        {
            if (keyValue == 'D') //如果是D,退出阙值设置选择界面
                setThresholdFromLcdFlag = 0;
            else if (keyValue == '#')
            {
                if (thresholdTemporary > 100 || thresholdTemporary < 0)
                    setThresholdFromLcdFlag = 61;
                else if (thresholdTemporary < thresholdOfLigh_MIN)
                    setThresholdFromLcdFlag = 62;
                else
                {
                    thresholdOfLigh_MAX = thresholdTemporary;
                    setThresholdFromLcdFlag = 60;
                }
            }
            else if (keyValue == '*')
                thresholdTemporary = 0;
            else if ((keyValue >= '0') && (keyValue <= '9'))
                thresholdTemporary = thresholdTemporary * 10 + (int)(keyValue - '0');
        }
        else if (setThresholdFromLcdFlag == 60)
        {
            if (keyValue == 'D') //如果是D,退出阙值设置选择界面
                setThresholdFromLcdFlag = 0;
        }
        else if (setThresholdFromLcdFlag == 61)
        {
            if (keyValue == 'D') //如果是D,退出阙值设置选择界面
                setThresholdFromLcdFlag = 0;
        }
        else if (setThresholdFromLcdFlag == 62)
        {
            if (keyValue == 'D') //如果是D,退出阙值设置选择界面
                setThresholdFromLcdFlag = 0;
        }
    }
}
void ledControl()
{
    if ((dht.temperature >= thresholdOfTemp_MIN) && (dht.temperature <= thresholdOfTemp_MAX) &&
            (dht.humidity >= thresholdOfAirHumi_MIN) && (dht.humidity <= thresholdOfAirHumi_MAX) &&
            (shs.getSoilHumi() >= thresholdOfSoilHumi_MIN) && (shs.getSoilHumi() <= thresholdOfSoilHumi_MAX) &&
            (ldr.getLight() >= thresholdOfLigh_MIN) && (ldr.getLight() <= thresholdOfLigh_MAX))
        digitalWrite(LED_PIN, LOW);    // 关上LED
    else
        digitalWrite(LED_PIN, HIGH);    // 打开LED

}
