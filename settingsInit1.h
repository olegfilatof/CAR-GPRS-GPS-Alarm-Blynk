// При замене аккумулятора, либо при reboot - появляется wifi, пропадает через 2 минуты, ибо не нужен после настройки
// подключение делается при старте
#include <Arduino.h>
#include <GyverDBFile.h>
#include <LittleFS.h>
GyverDBFile db(&LittleFS, "/data.db");

#ifdef ESP8266
#include <ESP8266WiFi.h>
#else
#include <WiFi.h>
#endif


// указывается заголовок меню, подключается база данных
#include <SettingsESP.h>
SettingsESP sett("MY AVTO SYSTEM CONFIG", &db);

bool relay1F;
bool relay2F;
int batteryF;
//int engineF;
int startGPSF;
//float voltage = 12.5;
//int humanSt = 1;
//int vibroSt = 0;
//int relay1F = 0;
//int relay2F = 0;
//int temp1 = 20;
//int temp2 = 18;
 
/////////////////////////////////////////

// ключи для хранения в базе данных
DB_KEYS( 
  dataBase,
  wifi_ssid,
  wifi_pass,
  sim_APN,
  GPS_interval_send,
  localwifi_pass,
  saveBattery,
  engineStop,
  beepStart,
  bat,
  hum,
  vib,
  rel1,
  rel2,
  tem1,
  tem2,
  startGPS,
  apply );

void build(sets::Builder& b) {
    {  sets::Group g(b, "PARAMETERS 🚗");
        
    b.HTML("GPS COORDINATES LINK 🌏 ", R"(<a href="https://www.google.com/maps">M A P S & G P S</a>)");
    
    b.beginRow();   
    b.Label(dataBase::bat, "BATTERY VOLTAGE", ""); 
    b.Number("Volt: ", &voltage);
    b.endRow();

    b.beginRow();   
    b.Label(dataBase::hum, "HUMAN SENSOR", ""); 
    b.Number("ON/OFF:", &alarmHF);
    b.endRow();

    b.beginRow();   
    b.Label(dataBase::vib, "VIBRATION SENSOR", ""); 
    b.Number("ON/OFF:", &alarmVF);
    b.endRow();

    b.beginRow();   
    b.Label(dataBase::tem1, "MOTOR TEMPERATURE", ""); 
    b.Number("C:", &Temp1);
    b.endRow();

    b.beginRow();   
    b.Label(dataBase::tem2, "INLET TEMPERATURE", ""); 
    b.Number("C:", &Temp2);
    b.endRow();

    b.beginRow();   
    b.Label(dataBase::rel1, "STOP ENGINE", ""); 
    b.Number("ON/OFF:", &relay1F);
    b.endRow();

    b.beginRow();   
    b.Label(dataBase::rel2, "START ALARM", ""); 
    b.Number("ON/OFF:", &relay2F);
    b.endRow();
    }

            // а это кнопка на вложенное меню. 
        if (b.beginMenu("SETTINGS ⚙ "))
         {
           if (b.beginGroup("")) {

               b.Input(dataBase::wifi_ssid, "My Router SSID");
    b.Pass(dataBase::wifi_pass, "My Router Password");
    b.Pass(dataBase::localwifi_pass, "Local AP password");
    b.Input(dataBase::sim_APN, "SIM card APN");
   // b.Input(dataBase::LYNK_TEMPLATE_ID, "BLYNK_TEMPLATE_ID");
   // b.Input(dataBase::LYNK_TEMPLATE_NAME, "BLYNK_TEMPLATE_NAME");
   // b.Input(dataBase::LYNK_AUTH_TOKEN, "BLYNK_AUTH_TOKEN");
   if(b.Input(dataBase::GPS_interval_send, "GPS interval send time (in minutes, 3 ~ 1440!)"))
    {if(db[dataBase::GPS_interval_send] >= 1441 or (db[dataBase::GPS_interval_send] <= 2.9 )) {db[dataBase::GPS_interval_send] = 3;}}
     
     if (b.Switch(dataBase::engineStop, "EMERGENCY STOP The ENGINE! ")) {
     relay1F = db[dataBase::engineStop];  digitalWrite(relay1, relay1F); db.update();}

     if (b.Switch(dataBase::beepStart, "Start BEEP-BEEP! ")) {
     relay2F = db[dataBase::beepStart];  digitalWrite(relay2, relay2F); db.update();}
    
  /*
    // b.Select(dataBase::saveBattery, "", "1;0");
    if (b.Switch(dataBase::saveBattery, "Save BATTERY! ON- stop wifi till system reboot")) {
      batteryF = db[dataBase::saveBattery];}
     
*/
    // b.Select(dataBase::saveBattery, "", "1;0");
    if (b.Switch(dataBase::startGPS, "START GPS in work, Stop wifi")) {
      startGPSF = db[dataBase::startGPS]; db.update();}

   
//if (db[dataBase::BLYNK_TEMPLATE_NAME].length()) Serial.println(db[dataBase::BLYNK_TEMPLATE_NAME]);

    if (b.Button(dataBase::apply, "Save & Restart")) {
      db.update();  // сохраняем всё вручную
      ESP.restart();
    }

        b.endGroup();
        b.endMenu();
    }
    
    }}