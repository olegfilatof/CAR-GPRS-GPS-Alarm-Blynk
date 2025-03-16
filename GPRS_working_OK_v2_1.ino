//OK for ESP32 Dev Module
/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial
//-----------------------------------------------------------------------
#define BLYNK_TEMPLATE_ID           "TMPL4fa-kn3Dz"
#define BLYNK_TEMPLATE_NAME         "CAR GPS"
#define BLYNK_AUTH_TOKEN            "HDl45JpsYRZgO3nOyNjqZLBxPqDeuFlx"
// You should get Auth Token in the Blynk App.
// Go to the Project Settings (nut icon).
//char auth[] = BLYNK_AUTH_TOKEN;
//============================================

bool modemStartingF = false;
int sleepTime = 3; // для задачи периода сна, в минутах!!!
bool firstTime = true; // флаг для однократного запуска wifi в 1-й раз

//============================================

#include "initiation.h"
#include "settingsInit1.h"
bool sequrityStart = true; // по умолчанию охрана включена
static bool flag = false;

void SendData() {
  if (alarmON and sequrityStart) {
  //если выходит на связь по программе- отправлять 1 раз координаты, voltage, temperatures. Если сработал alarm- 
  // каждые 30 сек высылается состояние датчиков в течении 3 мин 
    Blynk.virtualWrite(V20,alarmHF);
    Blynk.virtualWrite(V21,alarmVF);
    Blynk.virtualWrite(V13,alarmON); //звуковой сигнал
    //Blynk.virtualWrite(V13,alarmON); //звуковой сигнал
     //  coordinates = "https://www.google.com/maps/place/"+String(latitude,8)+","+String(longitude,8)+"?hl=en";
 // url = "https://www.google.com/maps/place/"+String(latitude,6)+","+String(longitude,6)+"?hl=en"; //ОК
         // real GPS////////////
      gps.read();
  url = "https://www.google.com/maps/place/"+String(gps.latitude,5)+","+String(gps.longitude,5)+"?hl=en"; //ОК 
//////////////////////

  Blynk.virtualWrite(V7, url); // Отправляем динамический URL на V7
  Blynk.virtualWrite(V27, flag); // Отправляем динамический URL на V27
  Blynk.setProperty(V27, "url", url);
  Serial.print("Отправил данные алармов из SendData, alarmHF=   "); Serial.println(alarmHF); 
  Serial.print("Отправил данные алармов из SendData, alarmVF=   "); Serial.println(alarmVF); 
  }}
  

void setup()
{
  // Debug console
  Serial.begin(115200);

  // используем метод ext1 для пробуждения по уровню на портах, понадобится вот эта строчка: 
  esp_sleep_enable_ext1_wakeup(pinBitMask,ESP_EXT1_WAKEUP_ANY_HIGH);
//  ESP_EXT1_WAKEUP_ALL_LOW: wake up when all GPIOs go low; - если ВСЕ порты... 
//  ESP_EXT1_WAKEUP_ANY_HIGH: wake up when any of the GPIOs go high. -любой порт из указанных
//If you’re using an ESP32-S2, ESP32-S3, ESP32-C6 or ESP32-H2, these are the available modes:
//ESP_EXT1_WAKEUP_ANY_LOW: wake up when any of the selected GPIOs is low
//ESP_EXT1_WAKEUP_ANY_HIGH: wake up when any of the selected GPIOs is high

  pinMode(relay1,OUTPUT);
  pinMode(relay2,OUTPUT);
  pinMode(modemSwitch,OUTPUT);
  pinMode(gpsSwitch,OUTPUT);
  pinMode(humansensor,INPUT_PULLUP); humansensorI.setDebounceTime(50);
  pinMode(vibrosensor,INPUT_PULLUP); vibrosensorI.setDebounceTime(50); 
  //pinMode(bsygnal,OUTPUT);  bsygnalI.setDebounceTime(50); 
//после выхода из сна ножка (ножки) IO, которая используется для пробуждения, может быть сконфигурирована как RTC IO. 
//Перед использованием этой ножки в качестве digital GPIO переконфигурируйте её с помощью функции rtc_gpio_deinit.
  
  digitalWrite(relay1,0);
  digitalWrite(relay2,0);
  digitalWrite(modemSwitch,0);
  digitalWrite(gpsSwitch,0);
  //digitalWrite(2,0); 
  pinMode(intLed,OUTPUT);
  digitalWrite(intLed,0); // onboard LED stop

  delay(10);

#include "setup1.h"
#include "settingsSetup2.h"
////////////////////////////////////////////////////////////////////////////////////////////////////////////


startGPSF = db[dataBase::startGPS]; // флаг включения GPRS
   // узнаём, в каком состоянии переменные в базе данных:
   // если строка с APN заполнена - читаем оттуда apn, иначе - смотри строку 37 initiation.h, там якобы универсальный APN 
if (db[dataBase::sim_APN].length()) { apn = db[dataBase::sim_APN].toString(); } // кому не нравятся тут {} - может удалить самостоятельно, я не возражаю :)


//if (db[dataBase::GPS_interval_send].length()) {sleepTime = db[dataBase::GPS_interval_send].toInt64(); } // интервал в микросекундах, загрёбся я переводить эту хрень в нужный формат... 
// else {sleepTime = 30;} // 30 минут - интервал выхода на связь и посылка данных в blynk, если режим экономии батареи
sleepTime = db[dataBase::GPS_interval_send].toInt64(); 
relay1F = db[dataBase::engineStop];
relay2F = db[dataBase::beepStart];
batteryF = db[dataBase::saveBattery];

  
/////////////////////////////////////////////////////////////// 

  // вызываем таймер отправки в blynk по времени if (wifiStart1time) { wifiStart1time = false; Wificonnection(); } 
  timer.setInterval(INTERVAL, SendData);
  //-----------------------------------------------------------  
///////////////////////////////////////////////////////////////
 /*
  wifiStart1time = true;
  if (wifiStart1time) // исли 1-й раз стартуем - старт wifi,  wifiStart1time == true
  { if (startGPSF) {TimerModemStart.start();} // если включён GPRS в настройках- старт модем через 2 мин после старта wifi
  
  // если нет - wifi продолжает работать всё время и аккамулятору капут через неделю
  
  { wifiStart1time = false;  Wificonnection();  } 
  }
  // если это уже второй запуск, после сна, то wifi не включается, а сразу модем, без паузы на wifi:
  else { WifiStop(); ModemStart(); modemStartingF = true; TimerModemStop.setTimeout(120000);}
  
  // если не нажато "Старт GPS, стоп wifi, экономим заряд батареи", то wifi не отключает соединение, модем не стартует, в сон не впадаем
//даём wifi подключиться, затем таймер сработает- и пипец тому wifi, GPRS рулит необычвйно

 //если включён старт gprs в settings морде- стартуем модем через 120 сек и стоп wifi если 1-й запуск,
 //если запуск после сна - стразу старт модем. 
*/

if (startGPSF and wifiStart1time) { TimerModemStart.setTimeout(120000); } // в первый раз - пауза на работу wifi 2 min, затем- модем, если включён в настройках

if (startGPSF and wifiStart1time == false) { ModemStart(); modemStartingF = true; TimerModemStop.setTimeout(120000);} //Если не 1-й раз - модем запускаем сразу 

if (wifiStart1time) { Wificonnection(); loopData = true; } // запускаем в первый раз, потом включается wifiStart1time = false; в функции
// если заснёт -  усё, потом нет wifi-я, зая!. Если перезагрузить - ок, wifi первые 2 минуты + web морда вайфайная, либо wifi всегда, если не запущен GPS в вэбморде

// включаем через 2 минуты модем, если включён в web морде startGPS, выключается автоматически  wifi.
// иначе - пипец заряду аккумулятора через 10 дней, привет тебе, погромист,
// с проектом "GSM автосигнализация" https://www.drive2.ru/c/593248096910065300/ с двумя(!) контроллерами :) и потреблением 150mA постоянно...

//-----------------------------------------------------------
 // Serial.print("startGPSF = " ); Serial.println(startGPSF);
 // Serial.print("apn = " ); Serial.println(apn);
 // Serial.print("sleepTime = " ); Serial.println(sleepTime);
 // Serial.print("relay1F = " ); Serial.println(relay1F);
 // Serial.print("relay2F = " ); Serial.println(relay2F);
 // Serial.print("batteryF = " ); Serial.println(batteryF);
 // Serial.print("wifiStart1time = " ); Serial.println(wifiStart1time);
 //Serial.print("sequrityStart = " ); Serial.println(sequrityStart);
}
 
/**********************************************************************************
 *  loop() function
 **********************************************************************************/
void loop()
{
  
  if (TimerModemStart.isReady()) { WifiStop(); loopData = false; ModemStart(); modemStartingF = true; TimerModemStop.setTimeout(120000);}  

  if (modemStartingF) { Blynk.run();  timer.run();  }
     
  if (TimerModemStop.isReady()) { modemStartingF = false; ModemStopping();  } //Serial.println("сработал таймер сна ");
  
  sett.tick(); //3 settings in loop
  
  #include "loop3.h"

/*
    static unsigned long timerGPS;
  if (millis() - timerGPS >= 2000) {
    timerGPS = millis();
       gps.read();
  url = "https://www.google.com/maps/place/"+String(gps.latitude,5)+","+String(gps.longitude,5)+"?hl=en"; //ОК 
  Serial.println(url);
  }
*/
}
//////////// Лупец finish ///////////////////////////////////////////////
 
 BLYNK_WRITE(V1) // From Blynk to the ESP32, to control a Relay1
  {
  int pinValue1 = param.asInt();
  digitalWrite(relay1 , pinValue1);
  
  }
     BLYNK_WRITE(V2) // From Blynk to the ESP32, to control a Relay2
  {
  int pinValue2 = param.asInt();
  digitalWrite(relay2 , pinValue2); 
  }

       BLYNK_WRITE(V3) // From Blynk to the ESP32, to control a Relay2
  {
  int sequrityStart = param.asInt(); // assigning incoming value from pin V1 to a variable
  
  }

  BLYNK_WRITE(V28)
  {
  int sleepTime = param.asInt(); // assigning incoming value from pin V1 to a variable
  // You can also use: String i = param.asStr();  double d = param.asDouble();
  //Serial.print("V28 Slider value is: ");
  //Serial.println(sleepTime);
  db[dataBase::GPS_interval_send] = sleepTime; db.update(); // ХЗ, с этими update, никогда не удавалось с ходу понять логику Gyver... :)
  }
