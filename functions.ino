 void ModemStart() {
  digitalWrite(gpsSwitch, 1); //GPS power start

        // gps.read();
        ////String(gps.speed);
       ////  String(gps.latitude,5);
      ////  String(gps.longitude,5);
//   url = "https://www.google.com/maps/place/"+String(gps.latitude,5);+","+String(gps.longitude,5);+"?hl=en"; //ОК  
////////////////// if(!gps.errPos) { // если пошли полноценные данные с GPS, то запускаем modem}

// РАСКРОЙ, КОГДА GPS ПРИЕДЕТ OPEN comments OPEN comments OPEN  comments OPEN  comments OPEN comments

if(gps.errPos){                               //  Если данные не прочитаны (gps.errPos=1) или координаты недостоверны (gps.errPos=2), то ...
       Serial.println("Координаты недостоверны");  //  Выводим сообщение об ошибке.
       delay(1000);
    //   Timer1sec.setTimeout(1000);
    //   if (Timer1sec.isReady()) 
       return;
                                      
     }          
  
  TimerModemStop.setTimeout(120000); // старт таймера отсчёта выключения модема
/////////////////////////////////////////////////////////////////////////////////////////////////////////////////
 
 // Serial.println("Initializing modem...");
  digitalWrite(modemSwitch, 1); //Modem power start
 // modem.restart();
  // Restart takes quite some time
  // To skip it, call init() instead of restart()
  modem.init();
  //------------------------------------------------------------
// Blynk.begin(auth, modem, apn, user, pass); // старт модема
Blynk.begin(BLYNK_AUTH_TOKEN, modem, apn.c_str(), user, pass); // все аргументы - const char*, пришлось изгаляться
  //------------------------------------------------------------
  Blynk.syncAll();

  Voltages(); // temp1, temp2 + voltages
  
  Blynk.virtualWrite(V5,temp1);
  Blynk.virtualWrite(V6,temp2);
  Blynk.virtualWrite(V10,voltage);
  Blynk.virtualWrite(V20,alarmHF);
  Blynk.virtualWrite(V21,alarmVF);
  Blynk.virtualWrite(V13,alarmON); //звуковой сигнал

 // url = "https://www.google.com/maps/place/"+String(latitude,6)+","+String(longitude,6)+"?hl=en"; //ОК
  
  //  закомментируй строчку выше и
  // РАСКРОЙ, КОГДА подключишь реальный GPS//  OPEN comments OPEN comments OPEN  comments OPEN  comments OPEN comments
      gps.read();
  url = "https://www.google.com/maps/place/"+String(gps.latitude,5)+","+String(gps.longitude,5)+"?hl=en"; //ОК 
  
  Serial.println(url);
 
  Blynk.virtualWrite(V7, url); // Отправляем динамический URL на V7
// если нет коорд - включить это  Blynk.virtualWrite(V27, flag); // Отправляем динамический URL на V27
  Blynk.setProperty(V27, "url", url);
  

 //Serial.print("Отправил voltage из функции  "); Serial.println(voltage); 
 //Serial.print("alarmON из modem вне alarmON and sequrityStart  "); Serial.println(alarmON);

  // Выделяем буфер для URL
 // String url = String("<a href=\"https://www.google.com/maps/place/") + String(latitude,6) + "," + String(longitude,6) + "\">" + "Открыть карту с координатами" + "</a>";
// }
 }

 ////////////////////////////////////////////////////////////////
void ModemStopping()
{
Blynk.disconnect();
digitalWrite(modemSwitch, 0); //модем стоп
digitalWrite(gpsSwitch, 0); //GPS power start

Serial.println("Иду спать, на срок указанного интервала");
 
sleepTime = db[dataBase::GPS_interval_send].toInt64();
 esp_sleep_enable_timer_wakeup(sleepTime * 60000000); // надо задавать в миллисекундах, sleepTime - в минутах
// esp_sleep_enable_timer_wakeup(2 * 60000000); // в миллисекундах 
 esp_deep_sleep_start();

  }

//////////////////////////////////////////////////////////////////////
void Voltages() {
    sensors.requestTemperatures(); 
  temp1 = sensors.getTempCByIndex(0);
  temp2 = sensors.getTempCByIndex(1)- 0.2; // поправка термометра
 float v1 = (float)analogRead(A0) * VREF * ((DIV_R1 + DIV_R2) / DIV_R2) / 1024; 
 float v2 = (float)analogRead(A0) * VREF * ((DIV_R1 + DIV_R2) / DIV_R2) / 1024; 
 float v3 = (float)analogRead(A0) * VREF * ((DIV_R1 + DIV_R2) / DIV_R2) / 1024;
voltage = ((v1+v2+v3)/3); //среднее по замерам
   Temp1 = String(temp1, 1); 
   Temp2 = String(temp2, 1); 
}

//////////////////////////////////////////////////////////////////////
 void WifiStop() {
           // WiFi.disconnect(true);
        WiFi.persistent(false);
        Serial.println("wifi stoped!");
       // WiFi.setSleepMode(WIFI_LIGHT_SLEEP);
       // WiFi.setOutputPower(18);        // 10dBm == 10mW, 14dBm = 25mW, 17dBm = 50mW, 20dBm = 100mW
  // WiFi.disconnect(false);
        WiFi.mode(WIFI_OFF);
  }

//////////////////////////////////////////////////////////////////////
 // ======== WIFI ============
 
 void Wificonnection() {
     wifiStart1time = false; // для 2-го и следующих запусков- флаг старта wifi - false
  // db[dataBase::BLYNK_TEMPLATE_ID] = 100;  - так присваивать значение переменной в базе данных
 // ======= AP =======
    WiFi.softAP("MY_AVTO-WIFI", db[dataBase::localwifi_pass]);  // 192.168.4.1 - адрес своей ТД
    Serial.print("AP IP: ");
    Serial.println(WiFi.softAPIP());  
    } 
//////////////////////////////////////////////////////////////////////////////////////////////////////



