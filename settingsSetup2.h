 // STA
    WiFi.mode(WIFI_AP);
    sett.begin();
    sett.onBuild(build);
     // базу данных запускаем до подключения к точке
#ifdef ESP32
    LittleFS.begin(true);
#else
    LittleFS.begin();
#endif

    // ======== SETTINGS ========

    // настройки вебморды
    // sett.config.requestTout = 3000;
    // sett.config.sliderTout = 500;
    // sett.config.updateTout = 1000;
     sett.config.theme = sets::Colors::Orange;

    // ======== DATABASE ========    
  db.begin();
  db.init(dataBase::wifi_ssid, "");
  db.init(dataBase::wifi_pass, "");
  db.init(dataBase::sim_APN, "");
  db.init(dataBase::GPS_interval_send, "");
  db.init(dataBase::localwifi_pass, "");
  db.init(dataBase::engineStop, "");
  db.init(dataBase::beepStart, "");
  db.init(dataBase::startGPS, "");
  db.init(dataBase::saveBattery, ""); //хранить данные в памяти!

  // ======== WIFI ============ 
//см setup
  //-----------------------------------------------------------


 // db[dataBase::BLYNK_TEMPLATE_ID] = 100;  - так присваивать значение переменной в базе данных
 //  // отправить апдейт прямо сейчас
  //      sett.updater().update(something), random(100));
    //если  сделать изменение в БД - обновление будет отправлено само чуть позже
