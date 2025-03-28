//USED libraries:
//1) Blynk         from Volodymyr Shymansky, UA. 
//2) GyverTimer
//3) TinyGSM       from Volodymyr Shymansky, UA. 
//4) OneWire
//5) DallasTemperature
//6) ezButton
//7) iarduino_GPS_NMEA
//8) Settings
//9) iarduino_GPS_ATGM336


//1. ESP8266 - только некоторые функции: напряжение батареи, сенсоры замыкают GND и RST для пробуждения и аларма,
//управление только одним реле. Реально же, использовать устаревшую ESP8266 - это чистая дурь из-за особенностей засыпания и скудного
// количества нормальных портов в ней, костыль в виде Software Serial и пр.  
// мне просто захотелось сделать "универсальную прошивку", что удалось с некоторыми оговорками. Затем всё внимание было уделено на esp32. 
// ESP32C3 - получение координат GPS, напряжение батареи, двв реле управления, два сенсора, температуры. Недостаточно портов. 
// ESP32 Wroom-  можно и нужно использовать, необходимо указать по другому Hardware SERIAL, см. примеры в инете. 

//кнопка WEB страницы - V27, без параметров. 
//кнопка V7 - COORDINATES
// Будильник - V25
// NON- stop GPRS - V11. 
// ПОСТАВИТЬ НА ОХРАНУ - V3 сигнал с сервера о постановке на охрану, pin V3 на сервере, либо физически замкнута кнопка T1 в машине

// +3.3V и порты сенсоров vibro и human, параллельно можно навесить что угодно: герконы на двери, датчики Холла, датчики наклона и т.д
//ESP8266, справа от антенны, вниз
// D0, GPIO16 Wake  HIGH at boot -- 1. used as wake up from deep sleep, соединить напрямую с RST. При прошивке- убрать это соединение!!! Затем восстановить
// D1, GPIO 5                    -- 2. used as output relay1 
// D2, GPIO 4                     -- 3. used as Input Human sensor
// D3, GPIO 0 Flash               -- vibrosensor // можно использовать датчик Холла параллельно, будет 3 и более сенсора
// D4, GPIO 2 TXD1                -- 4. used as Input 2 Temperature sensors Dallas.  boot fails if pulled LOW
// 3V
// GND
// D5, GPIO 14                    -- 5. used as Tx for modem sim800
// D6, GPIO 12                    -- 6. used as Rx for modem sim800
// D7, GPIO 13  RXD2                 7. used as modem Switch, start/stop
// D8, GPIO 15  TXD2                 8. used as relay2
// RX, GPIO0 3  RXD0  HIGH at boot --9. 
// TX, GPIO  1  TXD0  HIGH at boot,--10.    // boot fails if pulled LOW 
// GND
// 3.3V

//ESP8266, слева от антенны, вниз
// A0 - analog sensor  --           11. used as voltage measuring sensor
//(S3) GPIO10 can be used as input only.  
//#define A0 //Для esp32 указать!!!

// Подключение модема SIM800C: 5V, GND- общий с esp. Tx- Rx, Rx -Tx, без согласования сигналов, уже есть на плате!
// TTL - к 3.3V  -  для согласования уровней. У меня заработал без него.
// PWR-GND соединено для запуска при подаче питания, можно через транзистор или оптрон подавать на PRW 1 сек при старте
// Обязательно соединить проводником для старта модема.
//BLYNK: выключатели сделаны на виртуальных пинах
//V1- Emergency Motor Stop
//V2- Beep-Beep start
// V10 - напряжение аккумулятора, 
// V20 - human (сигнал с датчика присутствия), V21 - датчик наклона, вибрации
//V5, V6 - датчики температур двигателя/салона
// V7, V8 - координаты GPS. 
// Будильник и звук - V25, для аларма в режиме реального времени. 
//V28 slider для задачи периода времени выходв устройства на связь (первично задаётся в web морде)

// esp8266: 
// PWR-GND соединено для запуска при подаче питания, можно через транзистор подавать на PWR, 2 сек при старте
//D0 - RST - соединить для возможности перехода в сон ESP8266. Обязательно сделать это ПОСЛЕ прошивки!!!
// соединия в схеме:
//1. D0 - RST - соединить для возможности перехода в сон. Обязательно  после прошивки!!!
//2. Модем: соединить rx c tx  и tx c rx контроллера
//3. Dallas: параллельно, до 5 штук. Идентификация - нагреть рукой, смотреть изменения на экране
//4. Реле: 2 шт, страбатывание - HIGH уровень
//5. Датчик присутствия - обычный инфракрасный датчик с линзой Френеля, срабатывает - размыкает контакты GND- Sygnal на 10 сек. 
// для нормального фунционирования логики схемы сигналы от датчиков инвертируются при помощи любых маломощных NPN транзисторов. 
//6. Датчик вибрации - обычный в сборе юнит для ардуино. 
// Все соединения можно увидеть на монтажной схеме. 
// Для экономии батареи установлены выключатели питания для GPS трекера и Modem на MOSFET транзисторах по традиционной схеме.
// Транзисторы настоятельно рекомендуется использовать с параметром "напряжение начала открытия затвора"- не выше 2.5В. 

//*********************************************************************************************
//++++++++++++++++++++ для ESP32 ++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
// Справа, начиная сверху, антенна слева вверху
/*
*/
//GND
//23  // вызов домофона
// I2C_SCL 22   
//PZEM_RX_PIN 1 
//TX 3     
// I2C_SDA 21  
//pin GND - этот не юзать!
// 19  
//18  // 
//5   // 
//17  
//16 
// 4 
//GPIO0 - 
//#define   2  FREE/ реле не подключать, не запустится перепрошивка!!!
//15 
//Остальные 3 - не юзать!

//слева вниз, антенна вверху справа
//3.3 VDC
//PU-EN
// 36       
// 39      
//34
//35     
//32  
//33      
//25     
//26    
//27   
//14 //SD
//12 //SD
//#define    FREE 12 // реле не юзать, не запустится перепрошивка! 
//GND for 3.3V 
// 13 //SD 
// здесь 3 порта, их не юзать!!!
// 5V+

// Finish GPIO for ESP32++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
//Only the following pins can be used as inputs on the ESP32:

//0-19, 21-23, 25-27, 32-39

//Микросхема ESP32 как использовать пины при загрузке, иначе не стартанёт загружаться или старт
//GPIO 0 -- free
//GPIO 2 на этот пин реле цеплять нелья!!!!!! Точно нельзя
//GPIO 4 // подключены термометры Dallas
//GPIO 5 (во время загрузки должен быть ВЫСОКИЙ) подключил реле, ОК
//GPIO 12 (во время загрузки должно быть НИЗКОЕ) pzem не канает
//GPIO 15 (во время загрузки должен быть ВЫСОКИЙ) подключил реле, OK
///////////////////////////////////////////////////////////////////////////////////////
//Для 40 -пинового OLIMEX ESP32-S3-DEV, антенна вверху, слева вниз


/*
//1   3.3V   
//2   3.3V
//3   RST
//4   GPIO 4  
//5   GPIO 5
//6   GPIO 6
//7   GPIO 7
//8   GPIO 15
//9   GPIO 16
//10  GPIO 17
//11  GPIO 18
//12  GPIO 8
//13  GPIO 3
//14  GPIO 46 ??
//15  GPIO 09
//16  GPIO 10
//17  GPIO 11
//18  GPIO 12
//19  GPIO 13
//20  GPIO 14
// 5V
// GND

//////Справа: 
//44  GND
//43  GPIO 43  UOTX
//42  GPIO 44  UORX
//41  GPIO 01
//40  GPIO 2
//39  GPIO 42
//38  GPIO 41
//37  GPIO 40
//36  GPIO 39
//35  GPIO 38
//34  GPIO 37
//33  GPIO 36
//32  GPIO 35
//31  GPIO 00
//30  GPIO 45
//29  GPIO 48
//28  GPIO 47
//27  GPIO 21
//26  GPIO 20 USB_D+
//25  GPIO 19 USB_D-
//24  GND
//23  GND

//========================================================================================
  Board selected: ESP32S2 Dev Module
USB CDC On Boot: "Enabled" //Option: Disabled
Port: "COM6"
Upload Mode: "UART0" //option: Internal USB
//========================================================================================
//    Antenna               LED OUTPUT -15, for download - push Reset and Boot
39    40     1     EN
37    38     2     3
35    36     4     5
33    34     5     7
18    21     8     9
16    17     10    11
GND  GND     13    12
VBUS  15     14    3.3V DC
              //reset 
*/  
/*
//========================================================================================
//  Board selected: ESP32C3 Dev Module
      //USB CDC On Boot: "Enabled" 
      //Flash mode DIO, FTDI adapter
      // антенна вверху, слева внизу порты:
  //5V
  //GND
  //3.3V
 //SCK A GPIO4    
 3  
 2       
 1 
 0       //A GPIO0

// антенна вверху, справа вниз порты:
//A MISO GPIO5
// MOSI GPIO6
// SS   GPIO7   
//GPIO 21 SDA 
// SCL GPIO9
/ GPIO10
// RX GPIO20 
// TX GPIO21
//========================================================================================
*/

//ESP32-WROOM-DA module, ESP32-DOWD-V3 8M
//Антенна вверху, вид сверху, справа:
//GPIO21  // TXD0
//RXD0    //22
//19      // 23
//18      // 5
//17      // 16
//4       // GND
//3.3V    // 3.3V
//5V      // 5V
//VB      //VB
//GND     //GND
//Антенна вверху, вид сверху, слева:
//36    //37
//38    //39
//En    //34
//35    //32
//33    //25
//26    //27
//14    //12
//13    //15
//2     //0
//GND   //GND
/*
 #define         lock 36       // датчик Холла, положение ригеля замка, открыто либо закрыто+10KOm резистор между +5V и пином
 #define      window1 37      // датчик положения окна1 (открыто либо закрыто)+10KOm резистор между +5V и пином
 #define      window2 38     // датчик положения окна2 (открыто либо закрыто)+10KOm резистор между +5V и пином
 #define      window3 39    // датчик положения окна3 (открыто либо закрыто)+10KOm резистор между +5V и пином
 #define        human 34   // датчик присутствия человека
 #define         door 35      //датчик положения входной двери+10KOm резистор между +5V и пином
 #define         rain 32     // Датчик дождя 
 #define        water 33    // датчик протечки воды, + 10КОм резистор между GND и пином
 #define        relay 25   //   реле вспомогательное
 //#define          26   //- connected to pzem
 #define   ventilator 27
#define domofonCall 14  // вызов домофона
#define I2C_SCL 22  //GPIO 22 SDC // Это занято под датчик I2C  
#define I2C_SDA 21   //GPIO 21 SDA // Это занято под датчик I2C

*/
// 12 
//13  
//15  
//2 
//GPIO0
//GND
//--------------------------
//21
//22 
//19
//23
//18
//5
//17
//16
//4 
