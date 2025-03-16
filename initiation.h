#include "GyverTimer.h"              // библиотека таймера
//Timer Timer1month(MS, 2592000000); // (86400000×30 суток)ms для однократного срабатывания
GTimer TimerModemStart(MS); // 
GTimer TimerModemStop(MS); // 

GTimer Timer2minWIFI(MS); // cтоп wifi после 2 мин. работы
GTimer Timer3minModemON(MS); //время работы модема

GTimer Timer1minH(MS); // сработал датчик human - 1 мин аларм
GTimer Timer1minV(MS); // сработал датчик vibro - 1 мин аларм

GTimer Timer30sec1(MS); // сработала кнопка stop motor - 30 сек аларм
GTimer Timer30sec2(MS); // сработала кнопка старт бибикалка - 30 сек аларм

//GTimer Timer1sec(MS);

//TimerModemStart.setTimeout(120000);   // настроить таймаут 120 сек и запустить (перезапустить) таймер
//if (TimerModemStart.isReady()) Serial.println("Timeout 120сек сработал!");
//TimerModemStart.stop();   // "остановить" таймер
//myTimer.setTimeout(3000);   // настроить таймаут 3 сек, затем - стоп без условий
//----------------------------------------------------------------------
#define VREF 1.0328 // VREF = 1.1V в ESP32, но тут надо с вольтметром в руках прикидывать поправку, от множества факторов зависят измерения, 
// причём, слухи о "лютой нелинейности АЦП esp" - неуместны. :).
// 1.04 - 12.00; 1.034 - 11.94; 1.032 - 11.90

// Измерение Напряжения
// GND -- [ R2 ] -- A0 -- [ R1 ] -- VIN  + 0.1 mkF конденсатор на GND и порт A0
#define DIV_R1 1000000  // точное значение 1000 кОм резистора (1 MOM)
#define DIV_R2 220000   // точное значение 220 кОм резистора

#define A0 34 // Это порт аналогового сигнала от делителя напряжения аккумулятора
//----------------------------------------------------------------------
// Select your modem:
#define TINY_GSM_MODEM_SIM800
//#define TINY_GSM_MODEM_SIM900

//-----------------------------------------------------------------------
// Default heartbeat interval for GSM is 60
// If you want override this value, uncomment and set this option:
#define BLYNK_HEARTBEAT 185 // больше времени соединения, чтоб не пересоединялся каждые 30 сек
//-----------------------------------------------------------------------
#include <TinyGsmClient.h>
#include <BlynkSimpleTinyGSM.h>
//-----------------------------------------------------------------------
// Your GPRS credentials
// Leave empty, if missing user or pass
String (apn) = "internet";
char user[] = "";
char pass[] = "";
//-----------------------------------------------------------------------
#include <iarduino_GPS_NMEA.h>                     //  Подключаем библиотеку для расшифровки строк протокола NMEA получаемых по UART.
#include <iarduino_GPS_ATGM336.h>                  //  Подключаем библиотеку для настройки параметров работы GPS модуля ATGM336.
                                                   //
iarduino_GPS_NMEA    gps;                          //  Объявляем объект gps         для работы с функциями и методами библиотеки iarduino_GPS_NMEA.
iarduino_GPS_ATGM336 SettingsGPS;                  //  Объявляем объект SettingsGPS для работы с функциями и методами библиотеки iarduino_GPS_ATGM336.
/////////////////////////////////////////////////////////////////////////////////////////////////////////
//Modem:
#define RXmodem 18 // 
#define TXmodem 19 // 

//GPS
#define RXgps 17 //
#define TXgps 16 //   
#define GPS_BAUD 9600  // обязательно.  на скорости 115200 - не работает!

HardwareSerial gpsSerial(1);
HardwareSerial SerialAT(2);
#define gpsSerial Serial1
#define SerialAT Serial2


TinyGsm modem(SerialAT);
//TEMPERATURE SENSORS-----------------------------------------------------
#include <OneWire.h>
#include <DallasTemperature.h>
// Setup a oneWire instance to communicate with any OneWire devices
#define ONE_WIRE_BUS 4
OneWire oneWire(ONE_WIRE_BUS);
// Pass our oneWire reference to Dallas Temperature sensor 
DallasTemperature sensors(&oneWire);
//________________________________________________________________________
#define intLed 2 // светодиод blue onboard, надо погасить, шоб не ел ток аккумулятора
#define relay1  12
#define relay2  13
#define modemSwitch 23 // выключатель модема
#define humansensor 33 // объединить с 35
#define vibrosensor 25 // объединить с 32 
//#define bsygnal 15// сигнал с сервера о постановке на охрану, pin V3 на сервере, либо физически замкнута кнопка T1
#define gpsSwitch 22 // выключатель GPS
//----------
float temp1; // pin 4
float temp2; // pin 4
//float longitude;
//float latitude; 
String coordinates;
int humanSt = 0;
int vibroSt = 0;
bool alarmHF; 
bool alarmVF;
//int bsygnalSt = 0;
bool alarmON = false;
String Temp1;
String Temp2;
float voltage;
bool sequrityStartF;
String url;
bool loopData;
// Save reading number on RTC memory
RTC_DATA_ATTR bool wifiStart1time = true;  // смотри функцию Wificonnection() { if (wifiStart1time) {wifiStart1time = false;} 
//-----------------------------------------------------------------------
BlynkTimer timer;
#define INTERVAL 30000L // интервал отправки данных при подключенном модеме
//-----------------------------------------------------------------------

//#define humanSensor  39 // датчик присутствия // уже указан в маске 35
//#define vibroSensor  34 // датчик вибрации   // уже указан в маске  32

//////////////////////////
//Режим глубокого сна и внешнее пробуждение
//В качестве инициатора внешнего пробуждения могут использоваться только контакты типа RTC_GPIO. Вот они: 
//ESP32: 0, 2, 4, 12-15, 25-27, 32-39 //ESP32-S2: 0-21 //ESP32-S3: 0-21 //ESP32-C6: 0-7 //ESP32-H2: 7-14

// Пробуждение, если любая из выбранных ножек находится в лог. 1 (ESP_EXT1_WAKEUP_ANY_HIGH).
// Пробуждение, если любая из выбранных ножек находится в лог. 0 (ESP_EXT1_WAKEUP_ALL_LOW).
//сделал битовый флаг не как у индуса, с маятой и мутью, а грамотно: 
int WakeUppin, wakeFlag = 0;
constexpr uint8_t humanSensor = 35;
constexpr uint8_t vibroSensor = 32;
constexpr uint64_t humanSensorBitFlag = (static_cast<uint64_t>(1) << humanSensor);
constexpr uint64_t vibroSensorBitFlag = (static_cast<uint64_t>(1) << vibroSensor);
constexpr uint64_t pinBitMask = (humanSensorBitFlag | vibroSensorBitFlag);
void print_GPIO_wake_up(){
   WakeUppin = log(esp_sleep_get_ext1_wakeup_status())/log(2);
  Serial.print("WakeUppin = "); Serial.println(WakeUppin); 
  if (WakeUppin == humanSensor) {  wakeFlag = 1; Serial.println("Сработал humanSensor");}
  if (WakeUppin == vibroSensor) {  wakeFlag = 2; Serial.println("Сработал vibroSensor");}
 // if (WakeUppin == newSensor) {  wakeFlag = 3; Serial.println("Сработал newSensor");}
}
////////////////////////////////////////////////////////////////////////////////////////////
 #include <ezButton.h>
ezButton humansensorI(humansensor); 
ezButton vibrosensorI(vibrosensor);
ezButton relay1I(relay1);
ezButton relay2I(relay2);
////////////////////////////////////////////////////////////////////////////////////////////
