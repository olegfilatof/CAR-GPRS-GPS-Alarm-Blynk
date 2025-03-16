#include "Arduino.h"
////// Срабатывания датчика присутствия при осутствии сна ////////////////
  humansensorI.loop();
  humanSt = humansensorI.getState();
  if (humansensorI.isPressed())  { Serial.println("human ЗАМКНУТ НА 0"); }
  if (humansensorI.isReleased()) { alarmHF = true; Serial.println("human ЗАМКНУТ НА 3.3V!"); Timer1minH.setTimeout(120000);} 

  ////// Срабатывания датчика вибрации и удара при осутсвии сна //////////////
  vibrosensorI.loop();
  vibroSt = vibrosensorI.getState();
  if (vibrosensorI.isPressed()) {Serial.println("vibro ЗАМКНУТ НА 0");}
  if (vibrosensorI.isReleased()) {alarmVF = true; Serial.println("vibro ЗАМКНУТ НА 3.3V!"); Timer1minV.setTimeout(120000);}


//////срабатывание relay2 (EM. Stop Motor) и выключение через 30 сек //////////////////////
  relay1I.loop();
  if (relay1I.isPressed())  {  relay1F = 0;} //Serial.println("relay1  OFF");
  if (relay1I.isReleased()) {  relay1F = 1; digitalWrite(relay1, 1); Timer30sec1.setTimeout(30000);} 
  if (Timer30sec1.isReady()) {digitalWrite(relay1, 0); }
//////срабатывание relay2 (клаксон) и выключение через 30 сек //////////////////////
  relay2I.loop();
  if (relay2I.isPressed())  {  relay2F = 0;} //Serial.println("relay2  OFF");
  if (relay2I.isReleased()) {  relay2F = 1; digitalWrite(relay2, 1); Timer30sec2.setTimeout(30000);} 
  if (Timer30sec2.isReady()) {digitalWrite(relay2, 0); }
////////////////////////////////////////////////////////////////////////////////////
if (Timer1minH.isReady()) { alarmHF = false; } // флаги срабатывания сенсоров
if (Timer1minV.isReady()) { alarmVF = false; }
////////// ALARM сработал //////////////////////////////////////////////////////////
if (alarmVF or alarmHF) {alarmON = true;} //общий флаг срабатывания сенсоров
else  {alarmON = false;}

if (loopData) { // если в первый раз запуск- шлём в вэбморду данные. нафиг они там- тот ещё вопрос... :)
  static unsigned long timerTemp;
  if (millis() - timerTemp >= 5000) {
    timerTemp = millis();
    Voltages();
                                      
//////////  GPS  /////////////////////////////
 gps.read(); //  Читаем данные (чтение может занимать больше 1 секунды). Функции можно указать массив для получения данных о спутниках.
     if(!gps.errPos){   
      String(gps.latitude,5);
      String(gps.longitude,5);
     }
}}

