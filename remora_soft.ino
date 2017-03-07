// **********************************************************************************
// Programmateur Fil Pilote et Suivi Conso
// **********************************************************************************
// Copyright (C) 2014 Thibault Ducret
// Licence MIT
//
// History : 15/01/2015 Charles-Henri Hallard (http://hallard.me)
//                      Intégration de version 1.2 de la carte electronique
//           13/04/2015 Theju
//                      Modification des variables cloud teleinfo
//                      (passage en 1 seul appel) et liberation de variables
//           15/09/2015 Charles-Henri Hallard : Ajout compatibilité ESP8266
//           02/12/2015 Charles-Henri Hallard : Ajout API WEB ESP8266 et Remora V1.3
//           04/01/2016 Charles-Henri Hallard : Ajout Interface WEB GUIT
//           04/03/2017 Manuel Hervo          : Ajout des connexions TCP Asynchrones
//
// **********************************************************************************

// Tout est inclus dans le fichier remora.h
// Pour activer des modules spécifiques ou
// changer différentes configurations il
// faut le faire dans le fichier remora.h
#include "remora.h"

#ifdef SPARK
  #include "LibMCP23017.h"
  #include "LibSSD1306.h"
  #include "LibGFX.h"
  #include "LibULPNode_RF_Protocol.h"
  #include "LibLibTeleinfo.h"
  //#include "WebServer.h"
  #include "display.h"
  #include "i2c.h"
  #include "pilotes.h"
  #include "rfm.h"
  #include "tinfo.h"
  #include "linked_list.h"
  #include "LibRadioHead.h"
  #include "LibRH_RF69.h"
  #include "LibRHDatagram.h"
  #include "LibRHReliableDatagram.h"
#endif

// Arduino IDE need include in main INO file
#ifdef ESP8266
  #include <EEPROM.h>
  #include <FS.h>
  #include <ESP8266WiFi.h>
  #include <ESP8266HTTPClient.h>
  // #include <ESP8266WebServer.h>
  // #include <ESP8266mDNS.h>
  #include <ESPAsyncTCP.h>
  #include <ESPAsyncWebServer.h>
  #include <WiFiUdp.h>
  #include <ArduinoOTA.h>
  #include <Wire.h>
  #include <SPI.h>
  #include <Ticker.h>
  #include <NeoPixelBus.h>
  #include <BlynkSimpleEsp8266.h>
#ifdef MOD_TIME
  #include <RTClib.h>
  #include <stddef.h>
#endif
  #include "./LibMCP23017.h"
  #include "./LibSSD1306.h"
  #include "./LibGFX.h"
  #include "./LibULPNode_RF_Protocol.h"
  #include "./LibLibTeleinfo.h"
  #include "./LibRadioHead.h"
  #include "./LibRHReliableDatagram.h"
#endif


// Variables globales
// ==================
// status global de l'application
uint16_t status = 0;
unsigned long uptime = 0;
bool first_setup;

// Nombre de deconexion cloud detectée
int my_cloud_disconnect = 0;

#ifdef SPARK
  // Particle WebServer
  //WebServer server("", 80);
#endif

#ifdef ESP8266
  // ESP8266 WebServer
  AsyncWebServer server(80);
  // Use WiFiClient class to create a connection to WEB server
  WiFiClient client;
  // RGB LED (1 LED)
  MyPixelBus rgb_led(1, RGB_LED_PIN);

  // define whole brigtness level for RGBLED
  uint8_t rgb_brightness = 127;

  Ticker Tick_emoncms;
  Ticker Tick_jeedom;

  volatile boolean task_emoncms = false;
  volatile boolean task_jeedom = false;

  bool ota_blink;
  bool reboot = false;

  /******************************************************************************************************/
  /*                                          TIME                                                      */
  /******************************************************************************************************/
  #ifdef MOD_TIME
    unsigned int localPort = 2390;            // local port to listen for UDP packets
    //IPAddress timeServer(129, 6, 15, 28);     // time.nist.gov NTP server
    IPAddress timeServerIP;
    const char* timeServer = "0.fr.pool.ntp.org";
    const int NTP_PACKET_SIZE = 48;           // NTP time stamp is in the first 48 bytes of the message
    byte packetBuffer[ NTP_PACKET_SIZE];      // buffer to hold incoming and outgoing packets

    WiFiUDP udp;                              // A UDP instance to let us send and receive packets over UDP
    boolean doNTP=false;

    // RTC handler
    RTC_Millis rtc;                           // RTC (soft)
    DateTime now;                             // current time
    int ch,cm,cs,os,cdy,cmo,cyr,cdw;          // current time & date variables
    int nh,nm,ns,ndy,nmo,nyr,ndw;             // NTP-based time & date variables

    long backFromHolidays;                    // Date en seconds, depuis 2000, avant le retour de vacances

    #define min(a,b) ((a)<(b)?(a):(b))        // recreate the min function
  #endif
#endif

/* ======================================================================
Function: spark_expose_cloud
Purpose : declare et expose les variables et fonctions cloud
Input   :
Output  : -
Comments: -
====================================================================== */
#ifdef SPARK
void spark_expose_cloud(void)
{
  Debugln("spark_expose_cloud()");

  #ifdef MOD_TELEINFO
    // Déclaration des variables "cloud" pour la téléinfo (10 variables au maximum)
    // je ne sais pas si les fonction cloud sont persistentes
    // c'est à dire en cas de deconnexion/reconnexion du wifi
    // si elles sont perdues ou pas, à tester
    // -> Theju: Chez moi elles persistes, led passe verte mais OK
    //Spark.variable("papp", &mypApp, INT);
    //Spark.variable("iinst", &myiInst, INT);
    //Spark.variable("isousc", &myisousc, INT);
    //Spark.variable("indexhc", &myindexHC, INT);
    //Spark.variable("indexhp", &myindexHP, INT);
    //Spark.variable("periode", &myPeriode, STRING); // Période tarifaire en cours (string)
    //Spark.variable("iperiode", (ptec_e *)&ptec, INT); // Période tarifaire en cours (numerique)

    // Récupération des valeurs d'étiquettes :
    Particle.variable("tinfo", mytinfo, STRING);

  #endif

  // Déclaration des fonction "cloud" (4 fonctions au maximum)
  Particle.function("fp",    fp);
  Particle.function("setfp", setfp);

  // Déclaration des variables "cloud"
  Particle.variable("nivdelest", &nivDelest, INT); // Niveau de délestage (nombre de zones délestées)
  //Spark.variable("disconnect", &cloud_disconnect, INT);
  Particle.variable("etatfp", etatFP, STRING); // Etat actuel des fils pilotes
  Particle.variable("memfp", memFP, STRING); // Etat mémorisé des fils pilotes (utile en cas de délestage)

  // relais pas disponible sur les carte 1.0
  #ifndef REMORA_BOARD_V10
    Particle.function("relais", relais);
    Particle.variable("etatrelais", &etatrelais, INT);
  #endif
}
#endif


// ====================================================
// Following are dedicated to ESP8266 Platform
// Wifi management and OTA updates
// ====================================================
#ifdef ESP8266

/* ======================================================================
Function: Task_emoncms
Purpose : callback of emoncms ticker
Input   :
Output  : -
Comments: Like an Interrupt, need to be short, we set flag for main loop
====================================================================== */
void Task_emoncms()
{
  task_emoncms = true;
}

/* ======================================================================
Function: Task_jeedom
Purpose : callback of jeedom ticker
Input   :
Output  : -
Comments: Like an Interrupt, need to be short, we set flag for main loop
====================================================================== */
void Task_jeedom()
{
  task_jeedom = true;
}

/* ======================================================================
Function: WifiHandleConn
Purpose : Handle Wifi connection / reconnection and OTA updates
Input   : setup true if we're called 1st Time from setup
Output  : state of the wifi status
Comments: -
====================================================================== */
int WifiHandleConn(boolean setup = false)
{
  int ret = WiFi.status();
  uint8_t timeout ;

  if (setup) {
    // Feed the dog
    _wdt_feed();

    DebugF("========== SDK Saved parameters Start");
    WiFi.printDiag(DEBUG_SERIAL);
    DebuglnF("========== SDK Saved parameters End");

    #if defined (DEFAULT_WIFI_SSID) && defined (DEFAULT_WIFI_PASS)
      DebugF("Connection au Wifi : ");
      Debug(DEFAULT_WIFI_SSID);
      DebugF(" avec la clé '");
      Debug(DEFAULT_WIFI_PASS);
      DebugF("'...");
      Debugflush();
      WiFi.begin(DEFAULT_WIFI_SSID, DEFAULT_WIFI_PASS);
    #else
      if (*config.ssid) {
        DebugF("Connection à: ");
        Debug(config.ssid);
        Debugflush();

        // Do wa have a PSK ?
        if (*config.psk) {
          // protected network
          Debug(F(" avec la clé '"));
          Debug(config.psk);
          Debug(F("'..."));
          Debugflush();
          WiFi.begin(config.ssid, config.psk);
        } else {
          // Open network
          Debug(F("AP Ouvert"));
          Debugflush();
          WiFi.begin(config.ssid);
        }
      }
    #endif

    timeout = 25; // 25 * 200 ms = 5 sec time out

    // 200 ms loop
    while ( ((ret = WiFi.status()) != WL_CONNECTED) && timeout )
    {
      // Orange LED
      LedRGBON(COLOR_ORANGE);
      delay(50);
      LedRGBOFF();
      delay(150);
      DebugF(".");
      --timeout;
    }

    // connected ? disable AP, client mode only
    if (ret == WL_CONNECTED)
    {
      DebuglnF("connecte!");
      WiFi.mode(WIFI_STA);

      DebugF("IP address   : "); Debugln(WiFi.localIP());
      DebugF("MAC address  : "); Debugln(WiFi.macAddress());
      #ifdef MOD_TIME
        // Feed the dog
        _wdt_feed();
        // Time NTP
        udp.begin(localPort);
        DebugF("Starting UDP with Local Port "); Debugln(udp.localPort());
        doNTP=true; // get NTP timestamp immediately
        getTimeNTP();
      #endif

    // not connected ? start AP
    } else {
      char ap_ssid[32];
      DebugF("Erreur, passage en point d'acces ");
      Debugln(DEFAULT_HOSTNAME);

      // protected network
      DebugF(" avec la clé '");
      if (*config.ap_psk) {
        Debug(config.ap_psk);
      } else {
        Debug(DEFAULT_WIFI_AP_PASS);
      }
      Debugln("'");
      Debugflush();

      if (*config.ap_psk) {
        WiFi.softAP(DEFAULT_HOSTNAME, config.ap_psk);
      } else {
        WiFi.softAP(DEFAULT_HOSTNAME, DEFAULT_WIFI_AP_PASS);
      }
      WiFi.mode(WIFI_AP_STA);

      DebugF("IP address   : "); Debugln(WiFi.softAPIP());
      DebugF("MAC address  : "); Debugln(WiFi.softAPmacAddress());
    }

    // Feed the dog
    _wdt_feed();

    // Set OTA parameters
     ArduinoOTA.setPort(DEFAULT_OTA_PORT);
     ArduinoOTA.setHostname(DEFAULT_HOSTNAME);
     if (*config.ota_auth) {
       ArduinoOTA.setPassword(config.ota_auth);
     }/* else {
       ArduinoOTA.setPassword(DEFAULT_OTA_PASS);
     }*/
     ArduinoOTA.begin();

    // just in case your sketch sucks, keep update OTA Available
    // Trust me, when coding and testing it happens, this could save
    // the need to connect FTDI to reflash
    // Usefull just after 1st connexion when called from setup() before
    // launching potentially buggy main()
    for (uint8_t i=0; i<= 10; i++) {
      LedRGBON(COLOR_MAGENTA);
      delay(100);
      LedRGBOFF();
      delay(200);
      ArduinoOTA.handle();
    }

  } // if setup

  return WiFi.status();
}

#endif

/* ======================================================================
Function: timeAgo
Purpose : format total seconds to human readable text
Input   : second
Output  : pointer to string
Comments: -
====================================================================== */
char * timeAgo(unsigned long sec)
{
  static char buff[16];

  // Clear buffer
  buff[0] = '\0';

  if (sec < 2) {
    sprintf_P(buff,PSTR("just now"));
  } else if (sec < 60) {
    sprintf_P(buff,PSTR("%d seconds ago"), sec);
  } else if (sec < 120) {
    sprintf_P(buff,PSTR("1 minute ago"));
  } else if (sec < 3600) {
    sprintf_P(buff,PSTR("%d minutes ago"), sec/60);
  } else if (sec < 7200) {
    sprintf_P(buff,PSTR("1 hour ago"));
  } else if (sec < 86400) {
    sprintf_P(buff,PSTR("%d hours ago"), sec/3660);
  } else if (sec < 172800) {
    sprintf_P(buff,PSTR("yesterday"));
  } else if (sec < 604800) {
    sprintf_P(buff,PSTR("%d days ago"), sec/86400);
  }
  return buff;
}


/* ======================================================================
Function: sendNTPpacket
Purpose : send an NTP request to the time server at the given address
Input   : address of time server
Output  : -
Comments: -
====================================================================== */
void sendNTPpacket(IPAddress& address) {
  // Feed the dog
  _wdt_feed();
  DebuglnF("SendNTPpacket");
  // set all bytes in the buffer to 0
  memset(packetBuffer, 0, NTP_PACKET_SIZE);
  // Initialize values needed to form NTP request
  // (see URL above for details on the packets)
  packetBuffer[0] = 0b11100011;   // LI, Version, Mode
  packetBuffer[1] = 0;     // Stratum, or type of clock
  packetBuffer[2] = 6;     // Polling Interval
  packetBuffer[3] = 0xEC;  // Peer Clock Precision
  // 8 bytes of zero for Root Delay & Root Dispersion
  packetBuffer[12]  = 49;
  packetBuffer[13]  = 0x4E;
  packetBuffer[14]  = 49;
  packetBuffer[15]  = 52;

  // all NTP fields have been given values, now
  // you can send a packet requesting a timestamp:
  udp.beginPacket(address, 123); //NTP requests are to port 123
  udp.write(packetBuffer, NTP_PACKET_SIZE);
  udp.endPacket();
}


/* ======================================================================
Function: getTimeNTP
Purpose : Parse result of NTP request to update RTC soft
Input   : -
Output  : -
Comments: -
====================================================================== */
void getTimeNTP() {
  //get a random server from the pool
  WiFi.hostByName(timeServer, timeServerIP);
  sendNTPpacket(timeServerIP);            // send an NTP packet to a time server
  // Feed the dog
  _wdt_feed();
  delay(1500);                          // wait to see if a reply is available

  int cb = udp.parsePacket();           // get packet (if available)
  if (!cb) {
    DebuglnF("... no NTP packet yet");
    return;
  }
  DebugF("... NTP packet received with "); Debug(cb); DebuglnF(" bytes");     // We've received a packet, read the data from it
  udp.read(packetBuffer, NTP_PACKET_SIZE);                            // read the packet into the buffer

  unsigned long highWord = word(packetBuffer[40], packetBuffer[41]);  // timestamp starts at byte 40 of packet. It is 2 words (4 bytes) long
  unsigned long lowWord = word(packetBuffer[42], packetBuffer[43]);   // Extract each word and...
  unsigned long secsSince1900 = highWord << 16 | lowWord;             // ... combine into long: NTP time (seconds since Jan 1 1900):

  const unsigned long seventyYears = 2208988800UL;                    // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
  unsigned long epoch = secsSince1900 - seventyYears;                 // subtract seventy years to get to 1 Jan. 1900:

  int tz = -1;                                            // adjust for EST time zone
  DateTime gt(epoch - (tz*60*60));                       // obtain date & time based on NTP-derived epoch...
  tz = IsDST(gt.month(), gt.day(), gt.dayOfTheWeek())?-2:-1;  // if in DST correct for GMT+2 hours else GMT+1
  DateTime ntime(epoch - (tz*60*60));                    // if in DST correct for GMT+2 hours else GMT+1
  rtc.adjust(ntime);                                     // and set RTC to correct local time
}

/* ======================================================================
Function: getTime
Purpose : Update RTC time
Input   : -
Output  : -
Comments: -
====================================================================== */
void getTime() {
  now = rtc.now();
  ch  = min(24,now.hour()); if(ch == 0) ch=24; // hours 1-24
  cm  = min(59,now.minute());
  cs  = min(59,now.second());
  cdy = min(31,now.day());
  cmo = min(now.month(),12);
  cyr = min(99,now.year()-2000);
  cdw = now.dayOfTheWeek();
  Debugf("DateTime: %02d/%02d/%d %d:%02d:%02d\n", cdy, cmo, cyr, ch, cm, cs);
}


/* ======================================================================
Function: IsDST
Purpose : returns true if during DST, false otherwise
Input   : month, day, dayOfWeek
Output  : -
Comments: -
====================================================================== */
boolean IsDST(int mo, int dy, int dw) {
  //DebuglnF("IsDST");
  if (mo < 3 || mo > 10) { return false; }                // January, February, and December are out.
  if (mo > 3 && mo < 10) { return true;  }                // April to October are in
  int previousSunday = dy - dw;
  DebugF("Previous Sunday: "); Debugln(previousSunday);
  if (mo == 3) { return previousSunday >= 24; }            // In March, we are DST if our previous Sunday was on or after the 24th.
  return previousSunday < 24;                             // In October we must be after the last Sunday to be DST. That means the previous Sunday was on or after the 24th.
}


/* ======================================================================
Function: setup
Purpose : prepare and init stuff, configuration, ..
Input   : -
Output  : -
Comments: -
====================================================================== */
void setup()
{
  uint8_t rf_version = 0;

  #ifdef SPARK
    Serial.begin(115200); // Port série USB

    waitUntil(Particle.connected);

  #elif defined(DEBUG)
    Serial1.begin(115200);
  #endif
  #if defined DEBUG_INIT || !defined MOD_TELEINFO
    DEBUG_SERIAL.begin(115200);
  #endif

  // says main loop to do setup
  first_setup = true;

  Debugln("Starting main setup");
  Debugflush();
}



/* ======================================================================
Function: mysetup
Purpose : prepare and init stuff, configuration, ..
Input   : -
Output  : -
Comments: -
====================================================================== */
void mysetup()
{
  uint8_t rf_version = 0;

  #ifdef SPARK
    bool start = false;
    long started ;

    // On prend le controle de la LED RGB pour faire
    // un heartbeat si Teleinfo ou OLED ou RFM69
    #if defined (MOD_TELEINFO) || defined (MOD_OLED) || defined (MOD_RF69)
    RGB.control(true);
    RGB.brightness(128);
    // En jaune nous ne sommes pas encore prêt
    LedRGBON(COLOR_YELLOW);
    #endif

    // nous sommes en GMT+1
    Time.zone(+1);

    // Rendre à dispo nos API, çà doit être fait
    // très rapidement depuis le dernier firmware
    spark_expose_cloud();

    // C'est parti
    started = millis();

    // Attendre que le core soit bien connecté à la serial
    // car en cas d'update le core perd l'USB Serial a
    // son reboot et sous windows faut reconnecter quand
    // on veut débugguer, et si on est pas synchro on rate
    // le debut du programme, donc petite pause le temps de
    // reconnecter le terminal série sous windows
    // Une fois en prod c'est plus necessaire, c'est vraiment
    // pour le développement (time out à 1s)
    while(!start)
    {
      // Il suffit du time out ou un caractère reçu
      // sur la liaison série USB pour démarrer
      if (Serial.available() || millis()-started >= 1000)
        start = true;

      // On clignote en jaune pour indiquer l'attente
      LedRGBON(COLOR_YELLOW);
      delay(50);

      // On clignote en rouge pour indiquer l'attente
      LedRGBOFF();
      delay(100);
    }

    // Et on affiche nos paramètres
    Debugln("Core Network settings");
    Debug("IP   : "); Debugln(WiFi.localIP());
    Debug("Mask : "); Debugln(WiFi.subnetMask());
    Debug("GW   : "); Debugln(WiFi.gatewayIP());
    Debug("SSDI : "); Debugln(WiFi.SSID());
    Debug("RSSI : "); Debug(WiFi.RSSI());Debugln("dB");

    //  WebServer / Command
    //server.setDefaultCommand(&handleRoot);
    //webserver.addCommand("json", &sendJSON);
    //webserver.addCommand("tinfojsontbl", &tinfoJSONTable);
    //webserver.setFailureCommand(&handleNotFound);

    // start the webserver
    //server.begin();

  #elif defined (ESP8266)

    #ifdef MOD_TELEINFO
      // Init de la téléinformation
      Serial.begin(1200, SERIAL_7E1);
    #endif

    // Clear our global flags
    config.config = 0;

    #ifdef MOD_TIME
      rtc.begin(DateTime(F(__DATE__), F(__TIME__)));
    #endif

    // Our configuration is stored into EEPROM
    //EEPROM.begin(sizeof(_Config));
    EEPROM.begin(1024);

    DebugF("Config size="); Debug(sizeof(_Config));
    DebugF(" (emoncms=");   Debug(sizeof(_emoncms));
    DebugF("  jeedom=");   Debug(sizeof(_jeedom));
    Debugln(')');
    Debugflush();

    // Check File system init
    if (!SPIFFS.begin())
    {
      // Serious problem
      DebuglnF("SPIFFS Mount failed");
    } else {

      DebuglnF("SPIFFS Mount succesfull");

      Dir dir = SPIFFS.openDir("/");
      while (dir.next()) {
        String fileName = dir.fileName();
        size_t fileSize = dir.fileSize();
        Debugf("FS File: %s, size: %d\n", fileName.c_str(), fileSize);
        _wdt_feed();
      }
      DebuglnF("");
    }

    // Set OTA parameters
    ArduinoOTA.setPort(DEFAULT_OTA_PORT);
    ArduinoOTA.setHostname(DEFAULT_HOSTNAME);
    ArduinoOTA.setPassword(DEFAULT_OTA_PASS);

    // Read Configuration from EEP
    if (readConfig()) {
        DebuglnF("Good CRC, not set!");
        if (strlen(config.jeedom.host) > 0 && strlen(config.jeedom.url) > 0
          && strlen(config.jeedom.apikey) > 0 && (config.jeedom.freq > 0 && config.jeedom.freq < 86400)) {
          // Emoncms Update if needed
          Tick_jeedom.detach();
          Tick_jeedom.attach(config.jeedom.freq, Task_jeedom);
        }
        if (strlen(config.ota_auth) > 0) {
          ArduinoOTA.setPassword(config.ota_auth);
        }
        if (strlen(config.host) > 0) {
          ArduinoOTA.setHostname(config.host);
        }
        if (config.ota_port > 0) {
          ArduinoOTA.setPort(config.ota_port);
        }
    } else {
      // Reset Configuration
      resetConfig();

      // save back
      saveConfig();

      // Indicate the error in global flags
      config.config |= CFG_BAD_CRC;

      DebuglnF("Reset to default");
    }

    // Connection au Wifi ou Vérification
    WifiHandleConn(true);

    ArduinoOTA.begin();

    // just in case your sketch sucks, keep update OTA Available
    // Trust me, when coding and testing it happens, this could save
    // the need to connect FTDI to reflash
    // Usefull just after 1st connexion when called from setup() before
    // launching potentially buggy main()
    for (uint8_t i=0; i<= 10; i++) {
      LedRGBON(COLOR_MAGENTA);
      delay(100);
      LedRGBOFF();
      delay(200);
      ArduinoOTA.handle();
    }
    // OTA callbacks
    ArduinoOTA.onStart([]() {
      if (ArduinoOTA.getCommand() == U_SPIFFS) {
        SPIFFS.end();
      }
      LedRGBON(COLOR_MAGENTA);
      DebugF("\r\nUpdate Started..");
      ota_blink = true;
    });

    ArduinoOTA.onEnd([]() {
      LedRGBOFF();
      DebuglnF("Update finished restarting");
    });

    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
      if (ota_blink) {
        LedRGBON(COLOR_MAGENTA);
      } else {
        LedRGBOFF();
      }
      ota_blink = !ota_blink;
      //Debugf("Progress: %u%%\n", (progress / (total / 100)));
    });

    ArduinoOTA.onError([](ota_error_t error) {
      LedRGBON(COLOR_RED);
      DEBUG_SERIAL.printf("Update Error[%u]: ", error);
      if (error == OTA_AUTH_ERROR) DebuglnF("Auth Failed");
      else if (error == OTA_BEGIN_ERROR) DebuglnF("Begin Failed");
      else if (error == OTA_CONNECT_ERROR) DebuglnF("Connect Failed");
      else if (error == OTA_RECEIVE_ERROR) DebuglnF("Receive Failed");
      else if (error == OTA_END_ERROR) DebuglnF("End Failed");
      //reboot = true;
    });

    // handler for uptime
    server.on("/uptime", HTTP_GET, [](AsyncWebServerRequest *request) {
      String response = "";
      response += FPSTR("{\r\n");
      response += F("\"uptime\":");
      response += uptime;
      response += FPSTR("\r\n}\r\n") ;
      request->send(200, "text/json", response);
    });

    server.on("/config_form.json", HTTP_POST, handleFormConfig);
    server.on("/factory_reset",handleFactoryReset );
    server.on("/reset", handleReset);
    server.on("/tinfo", tinfoJSON);
    server.on("/relais", relaisJSON);
    server.on("/delestage", delestageJSON);
    server.on("/tinfo.json", tinfoJSONTable);
    server.on("/system.json", sysJSONTable);
    server.on("/config.json", confJSONTable);
    server.on("/spiffs.json", spiffsJSONTable);
    server.on("/wifiscan.json", wifiScanJSON);
    server.on("/holidays", [&]() {
      Debugln("holidays request");
      #ifdef MOD_TIME
        if (!server.hasArg("seconds")) {
          String response = FPSTR("{\r\n");
          response += F("\"response\":");
          response += backFromHolidays;
          response += FPSTR("\r\n}\r\n") ;
          server.send(200, "text/json", response);
        } else {
          DebugF("Seconds: "); Debugln(server.arg("seconds"));
          backFromHolidays = DateTime(server.arg("seconds").toInt()).secondstime();
          char cmd[NB_FILS_PILOTES+1] = "";
          uint8_t i = 0;
          // Si la durée avant le retour est supérieur à 2 jours, on coupe tout
          if (backFromHolidays - now.secondstime() > (2*SECONDS_PER_DAY)) {
            DebuglnF("Time of holidays is superior at 2 days, stop relais and radia HG");
            if (fnctRelais == FNCT_RELAIS_AUTO) {
              saveRelais = fnctRelais;
              fnct_relais((String)FNCT_RELAIS_ARRET);
            }
            // On sauvegarde l'état des fils pilotes
            for (i = 0; i < NB_FILS_PILOTES; i++) {
              saveFP[i] = etatFP[i];
              //DebugF("saveFP[i]: "); Debug(saveFP[i]); DebugF(" - etatFP[i]: "); Debugln(etatFP[i]);
              cmd[i] = 'H';
            }
          } else {
            DebuglnF("Time of holidays is inferior at 2 days, radia ECO");
            // On sauvegarde l'état des fils pilotes
            for (i = 0; i < NB_FILS_PILOTES; i++) {
              saveFP[i] = etatFP[i];
              //DebugF("saveFP[i]: "); Debug(saveFP[i]); DebugF(" - etatFP[i]: "); Debugln(etatFP[i]);
              cmd[i] = 'E';
            }
          }
          //DebugF("cmd: "); Debugln(cmd);
          int ret = fp(cmd);
          //DebugF("Retour fp: "); Debugln(String(ret));
          //String response = FPSTR("{") + F("\"response\":") + String(ret) + FPSTR("}") ;
          String response = "";
          response += FPSTR("{\r\n");
          response += F("\"response\":");
          response += ret;
          response += FPSTR("\r\n}\r\n") ;
          server.send(200, "text/json", response);
        }
      #else
        server.sendHeader("Connection", "close");
        server.sendHeader("Access-Control-Allow-Origin", "*");
        server.send(404, "text/plain", "MOD_TIME unactive");
      #endif
    });

    // handler for the hearbeat
    server.on("/hb.htm", HTTP_GET, [](AsyncWebServerRequest *request) {
      AsyncWebServerResponse *response = request->beginResponse(200, "text/html", R"(OK)");
      response->addHeader("Connection", "close");
      response->addHeader("Access-Control-Allow-Origin", "*");
      request->send(response);
    });

    // handler for the /update form POST (once file upload finishes)
    server.on("/update", HTTP_POST, [](AsyncWebServerRequest *request) {
      reboot = !Update.hasError();
      AsyncWebServerResponse *response = request->beginResponse(200, "text/plain", reboot ? "OK" : "FAIL");
      response->addHeader("Connection", "close");
      request->send(response);
    }, handle_fw_upload);

    server.onNotFound(handleNotFound);

    // serves all SPIFFS Web file with 24hr max-age control
    // to avoid multiple requests to ESP
    server.serveStatic("/font", SPIFFS, "/font","max-age=86400");
    server.serveStatic("/js",   SPIFFS, "/js"  ,"max-age=86400");
    server.serveStatic("/css",  SPIFFS, "/css" ,"max-age=86400");
    server.begin();
    DebuglnF("HTTP server started");

    #ifdef BLYNK_AUTH
      Blynk.config(BLYNK_AUTH);
    #endif

  #endif

  // Init bus I2C
  i2c_init();

  Debug("Remora Version ");
  Debugln(REMORA_VERSION);
  Debug("Compile avec les fonctions : ");

  #if defined (REMORA_BOARD_V10)
    Debug("BOARD V1.0 ");
  #elif defined (REMORA_BOARD_V11)
    Debug("BOARD V1.1 ");
  #elif defined (REMORA_BOARD_V12)
    Debug("BOARD V1.2 MCP23017 ");
  #elif defined (REMORA_BOARD_V13)
    Debug("BOARD V1.3 MCP23017 ");
  #else
    Debug("BOARD Inconnue");
  #endif

  #ifdef MOD_OLED
    Debug("OLED ");
  #endif
  #ifdef MOD_TELEINFO
    Debug("TELEINFO ");
  #endif
  #ifdef MOD_RF69
    Debug("RFM69 ");
  #endif
  #ifdef BLYNK_AUTH
    Debug("BLYNK ");
  #endif
  #ifdef MOD_ADPS
    Debug("ADPS ");
  #endif
  #ifdef MOD_TIME
    Debug("TIME ");
  #endif

  Debugln();

  // Init des fils pilotes
  if (pilotes_setup())
    status |= STATUS_MCP ;

  #ifdef MOD_OLED
    // Initialisation de l'afficheur
    if (display_setup())
    {
      status |= STATUS_OLED ;
      // Splash screen
      display_splash();
    }
  #endif

  #ifdef MOD_RF69
    // Initialisation RFM69 Module
    if ( rfm_setup())
      status |= STATUS_RFM ;
  #endif

  // Feed the dog
  _wdt_feed();

  #ifdef MOD_TELEINFO
    // Initialiser la téléinfo et attente d'une trame valide
    // Le status est mis à jour dans les callback de la teleinfo
    tinfo_setup(true);
  #endif

  // Led verte durant le test
  LedRGBON(COLOR_GREEN);

  // Enclencher le relais 1 seconde
  // si dispo sur la carte
  #ifndef REMORA_BOARD_V10
    Debug("Relais=ON   ");
    Debugflush();
    relais("1");
    for (uint8_t i=0; i<20; i++)
    {
      delay(10);
     // Feed the dog
     _wdt_feed();

      // Ne pas bloquer la reception et
      // la gestion de la téléinfo
      #ifdef MOD_TELEINFO
        tinfo_loop();
      #endif
    }
    Debugln("Relais=OFF");
    Debugflush();
    relais("0");
  #endif

  // nous avons fini, led Jaune
  LedRGBON(COLOR_YELLOW);

  // Hors gel, désactivation des fils pilotes
  initFP();

  // On etteint la LED embarqué du core
  LedRGBOFF();

  Debugln("Starting main loop");
  Debugflush();
}

/* ======================================================================
Function: rtc
Purpose : compte le temps qui passe
Input   : -
Output  : -
Comments: -
====================================================================== */
/*void getTimeRTC() {
  timeNow = millis() / 1000; // the number of milliseconds that have passed since boot
  seconds = timeNow - timeLast;
  // the number of seconds that have passed since the last time 60 seconds was reached.
  if (seconds == 60) {
    timeLast = timeNow;
    minutes = minutes + 1;
  }
  // if one minute has passed, start counting milliseconds from zero again
  // and add one minute to the clock.
  if (minutes == 60) {
    minutes = 0;
    hours = hours + 1;
  }
  // if one hour has passed, start counting minutes from zero and add one hour to the clock
  if (hours == 24) {
    hours = 0;
    days = days + 1;
  }
  //if 24 hours have passed, add one day
  if (hours == (24 - startingHour) && correctedToday == 0) {
    delay(dailyErrorFast * 1000);
    seconds = seconds + dailyErrorBehind;
    correctedToday = 1;
  }
  // every time 24 hours have passed since the initial starting time and it has
  // not been reset this day before, add milliseconds or delay the program with
  // some milliseconds.

  // Change these varialbes according to the error of your board.

  // The only way to find out how far off your boards internal clock is,
  // is by uploading this sketch at exactly the same time as the real time,
  // letting it run for a few days

  // and then determining how many seconds slow/fast your boards internal clock
  // is on a daily average. (24 hours).
  if (hours == 24 - startingHour + 2) {
    correctedToday = 0;
  }
  // let the sketch know that a new day has started for what concerns correction,
  // if this line was not here the arduiono would continue to correct for an entire hour
  // that is 24 - startingHour.
}*/


/* ======================================================================
Function: loop
Purpose : boucle principale du programme
Input   : -
Output  : -
Comments: -
====================================================================== */
void loop()
{
  static bool refreshDisplay = false;
  static bool lastcloudstate;
  static unsigned long previousMillis = 0;  // last time update
  unsigned long currentMillis = millis();
  bool currentcloudstate ;

  // our own setup
  if (first_setup) {
    mysetup();
    first_setup = false;
  }

  #ifdef ESP8266
  /* Reboot handler */
  if (reboot) {
    delay(REBOOT_DELAY);
    ESP.restart();
  }
  #endif

  // Gérer notre compteur de secondes
  if (millis() - previousMillis > 1000) {
    // Ceci arrive toute les secondes écoulées
    previousMillis = currentMillis;
    uptime++;
    refreshDisplay = true;

    #ifdef ESP8266
      #ifdef MOD_TIME
        getTime(); // Gestion du temps
        //Debug("doNTP: "); Debugln(doNTP ? "true" : "false");
        if (doNTP) {
          // updated journaly
          if (ch%24 == 0 && cm%60 == 0 && cs%60 == 0) {
            DebuglnF("It's time to get time");
            getTimeNTP();
          }
        }
        // Si la variable des vacances est définie
        if (backFromHolidays > 0) {
          // Si la date de retour est atteinte
          DebugF("backFromHolidays: "); Debugln(backFromHolidays);
          DebugF("now: "); Debugln(now.secondstime());
          // Le mode du relais du ballon doit être enclenché 24h avant le retour,
          // si il était en mode automatique avant les vacances
          if (saveRelais == FNCT_RELAIS_AUTO && backFromHolidays - SECONDS_PER_DAY < now.secondstime()) {
            // On remet le mode auto du relais
            if (fnctRelais != FNCT_RELAIS_AUTO) {
              DebuglnF("Change mode relay in auto");
              fnct_relais((String)FNCT_RELAIS_AUTO);
              saveRelais = 0;
            }
          }
          if (backFromHolidays - SECONDS_PER_DAY < now.secondstime()) {
            // On remet les radiateurs en mode eco
            // TODO: Ajouter un flag pour eviter de boucler
            char cmd[NB_FILS_PILOTES+1] = "";
            for (uint8_t i = 0; i < NB_FILS_PILOTES; i++) {
              cmd[i] = 'E';
              //saveFP[i] = '';
            }
            int ret = fp(cmd);
            DebugF("Retour fp: "); Debugln(String(ret));
          }
          // Le chauffage doit être enclenché 4h avant le retour
          // TODO: calculer le temps de chauffe avec les sondes
          if (backFromHolidays - (4 * 3600) < now.secondstime()) {
            DebuglnF("Holidays are finish, sorry :-(");
            // On réinitialise la variable
            backFromHolidays = 0;

            // On remet le chauffage en route
            char cmd[NB_FILS_PILOTES+1] = "";
            for (uint8_t i = 0; i < NB_FILS_PILOTES; i++) {
              cmd[i] = saveFP[i];
              //saveFP[i] = '';
            }
            int ret = fp(cmd);
            DebugF("Retour fp: "); Debugln(String(ret));
          } else if (backFromHolidays > 0) {
            //Debugf("Holidays in progress");
          }
        }
      #endif
    #endif

    #ifdef BLYNK_AUTH
      if ( Blynk.connected() ) {
        String up    = String(uptime) + "s";
        String papp  = String(mypApp) + "W";
        String iinst = String(myiInst)+ "A";
        Blynk.virtualWrite(V0, up, papp, iinst, mypApp);
        _yield();
      }
    #endif
  } else {
    #ifdef BLYNK_AUTH
      Blynk.run(); // Initiates Blynk
    #endif
  }

  #ifdef MOD_TELEINFO
    // Vérification de la reception d'une 1ere trame téléinfo
    tinfo_loop();
    _yield();
  #endif

  #ifdef MOD_RF69
    // Vérification de la reception d'une trame RF
    if (status & STATUS_RFM)
      rfm_loop();
      _yield();
  #endif

  #ifdef MOD_OLED
    // pour le moment on se contente d'afficher la téléinfo
    screen_state = screen_teleinfo;

    // Modification d'affichage et afficheur présent ?
    if (refreshDisplay && (status & STATUS_OLED))
      display_loop();
      _yield();
  #endif

  // çà c'est fait
  refreshDisplay = false;

  #if defined (SPARK)
    // recupération de l'état de connexion au cloud SPARK
    currentcloudstate = Spark.connected();
  #elif defined (ESP8266)
    // recupération de l'état de connexion au Wifi
    currentcloudstate = WiFi.status()==WL_CONNECTED ? true : false;
  #endif

  // La connexion cloud vient de chager d'état ?
  if (lastcloudstate != currentcloudstate)
  {
    // Mise à jour de l'état
    lastcloudstate=currentcloudstate;

    // on vient de se reconnecter ?
    if (currentcloudstate)
    {
      // on pubie à nouveau nos affaires
      // Plus necessaire
      #ifdef SPARK
      // spark_expose_cloud();
      #endif

      // led verte
      LedRGBON(COLOR_GREEN);
    }
    else
    {
      // on compte la deconnexion led rouge
      my_cloud_disconnect++;
      Debug("Perte de conexion au cloud #");
      Debugln(my_cloud_disconnect);
      LedRGBON(COLOR_RED);
    }
  }

  //#ifdef SPARK
  //char buff[64];
  //int len = 64;

  // process incoming connections one at a time forever
  //server.processConnection(buff, &len);
  //#endif


  // Connection au Wifi ou Vérification
  #ifdef ESP8266
    // Webserver
    //server.handleClient();
    ArduinoOTA.handle();

    if (task_emoncms) {
      emoncmsPost();
      task_emoncms=false;
    } else if (task_jeedom) {
      jeedomPost();
      task_jeedom=false;
    }
  #endif
}
