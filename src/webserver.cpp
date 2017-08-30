// **********************************************************************************
// Remora WEB Server, route web function
// **********************************************************************************
// Creative Commons Attrib Share-Alike License
// You are free to use/extend this library but please abide with the CC-BY-SA license:
// Attribution-NonCommercial-ShareAlike 4.0 International License
// http://creativecommons.org/licenses/by-nc-sa/4.0/
//
// This program works with the Remora board
// see schematic here https://github.com/thibdct/programmateur-fil-pilote-wifi
//
// Written by Charles-Henri Hallard (http://hallard.me)
//
// History 2015-06-14 - First release
//         2015-11-31 - Added Remora API
//         2016-01-04 - Added Web Interface part
//
// All text above must be included in any redistribution.
//
// **********************************************************************************

// Include header
#include "webserver.h"

#ifdef ESP8266

/* ======================================================================
Function: getContentType
Purpose : return correct mime content type depending on file extension
Input   : -
Output  : Mime content type
Comments: -
====================================================================== */
String getContentType(String filename) {
  if(filename.endsWith(".htm")) return F("text/html");
  else if(filename.endsWith(".html")) return F("text/html");
  else if(filename.endsWith(".css")) return F("text/css");
  else if(filename.endsWith(".json")) return F("application/json");
  else if(filename.endsWith(".js")) return F("application/javascript");
  else if(filename.endsWith(".png")) return F("image/png");
  else if(filename.endsWith(".gif")) return F("image/gif");
  else if(filename.endsWith(".jpg")) return F("image/jpeg");
  else if(filename.endsWith(".ico")) return F("image/x-icon");
  else if(filename.endsWith(".xml")) return F("text/xml");
  else if(filename.endsWith(".pdf")) return F("application/x-pdf");
  else if(filename.endsWith(".zip")) return F("application/x-zip");
  else if(filename.endsWith(".gz")) return F("application/x-gzip");
  else if(filename.endsWith(".otf")) return F("application/x-font-opentype");
  else if(filename.endsWith(".eot")) return F("application/vnd.ms-fontobject");
  else if(filename.endsWith(".svg")) return F("image/svg+xml");
  else if(filename.endsWith(".woff")) return F("application/x-font-woff");
  else if(filename.endsWith(".woff2")) return F("application/x-font-woff2");
  else if(filename.endsWith(".ttf")) return F("application/x-font-ttf");
  return "text/plain";
}

/* ======================================================================
Function: formatSize
Purpose : format a asize to human readable format
Input   : size
Output  : formated string
Comments: -
====================================================================== */
String formatSize(size_t bytes)
{
  if (bytes < 1024){
    return String(bytes) + F(" Byte");
  } else if(bytes < (1024 * 1024)){
    return String(bytes/1024.0) + F(" KB");
  } else if(bytes < (1024 * 1024 * 1024)){
    return String(bytes/1024.0/1024.0) + F(" MB");
  } else {
    return String(bytes/1024.0/1024.0/1024.0) + F(" GB");
  }
}

/* ======================================================================
Function: handleFileRead
Purpose : return content of a file stored on SPIFFS file system
Input   : file path
Output  : true if file found and sent
Comments: -
====================================================================== */
bool handleFileRead(String path, AsyncWebServerRequest *request) {
  if ( path.endsWith("/") )
    path += "index.htm";

  String contentType = getContentType(path);
  String pathWithGz = path + ".gz";
  bool gzip = false;

  DebugF("handleFileRead ");
  Debug(path);

  if (SPIFFS.exists(pathWithGz) || SPIFFS.exists(path)) {
    if (SPIFFS.exists(pathWithGz)){
      path += ".gz";
      DebugF(".gz");
      gzip = true;
    }

    DebuglnF(" found on FS");
    DebugF("ContentType: "); Debugln(contentType);

    AsyncWebServerResponse *response = request->beginResponse(SPIFFS, path, contentType);
    if (gzip) {
      DebuglnF("Add header content-encoding: gzip");
      response->addHeader("Content-Encoding", "gzip");
    }
    request->send(response);
    return true;
  }

  Debugln("");

  request->send(404, "text/plain", "File Not Found");
  return false;
}

/* ======================================================================
Function: formatNumberJSON
Purpose : check if data value is full number and send correct JSON format
Input   : String where to add response
          char * value to check
Output  : -
Comments: 00150 => 150
          ADCO  => "ADCO"
          1     => 1
====================================================================== */
void formatNumberJSON( String &response, char * value)
{
  // we have at least something ?
  if (value && strlen(value))
  {
    boolean isNumber = true;
    uint8_t c;
    char * p = value;

    // just to be sure
    if (strlen(p)<=16) {
      // check if value is number
      while (*p && isNumber) {
        if ( *p < '0' || *p > '9' )
          isNumber = false;
        p++;
      }

      // this will add "" on not number values
      if (!isNumber) {
        response += '\"' ;
        response += value ;
        response += '\"' ;
      } else {
        // this will remove leading zero on numbers
        p = value;
        while (*p=='0' && *(p+1) )
          p++;
        response += p ;
      }
    } else {
      response += "\"Error Value too long\"" ;
      DebuglnF("formatNumberJSON Value too long!");
    }
  } else {
    response += "\"Error Bad Value\"" ;
    DebuglnF("formatNumberJSON Bad Value!");
  }
}

/* ======================================================================
Function: tinfoJSONTable
Purpose : dump all teleinfo values in JSON table format for browser
Input   : linked list pointer on the concerned data
          true to dump all values, false for only modified ones
Output  : -
Comments: -
====================================================================== */
void tinfoJSONTable(AsyncWebServerRequest *request)
{

  // Just to debug where we are
  DebugF("Serving /tinfo page...\r\n");

  #ifdef MOD_TELEINFO

  ValueList * me = tinfo.getList();
  AsyncJsonResponse * response = new AsyncJsonResponse(true);
  JsonArray& arr = response->getRoot();

  // Got at least one ?
  if (me) {
    uint8_t index=0;

    // Loop thru the node
    while (me->next) {

      // we're there
      ESP.wdtFeed();

      // go to next node
      me = me->next;

      { JsonObject& item = arr.createNestedObject();
        item[FPSTR(FP_NA)] = me->name;
        item[FPSTR(FP_VA)] = me->value;
        item[FPSTR(FP_CK)] = (char) me->checksum;
        item[FPSTR(FP_FL)] = me->flags; }
    }

    DebugF("sending...");
    response->setLength();
    request->send(response);
    DebuglnF("OK!");
  } else {
    DebuglnF("sending 404...");
    request->send(404, "text/plain", "No data");
  }

  #else
    DebuglnF("sending 404...");
    request->send(404, "text/plain", "Teleinfo non activée");
  #endif // MOD_TELEINFO

}

/* ======================================================================
Function: sysJSONTable
Purpose : dump all sysinfo values in JSON table format for browser
Input   : request pointer if comming from web request
Output  : JsonStr filled if request is null
Comments: -
====================================================================== */
String sysJSONTable(AsyncWebServerRequest *request)
{
  String JsonStr="";

  // If Web request or just string asking, we'll do JSon stuff
  // in async response,
  AsyncJsonResponse * response = new AsyncJsonResponse(true);
  response->setContentType("application/json");
  JsonArray& arr = response->getRoot();

  // Web request ?
  if (request) {
    DebuglnF("Serving /system page...");
  } else {
    DebuglnF("Getting system JSON table...");
  }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Uptime";
    item[FPSTR(FP_VA)] = uptime; }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Version Logiciel";
    item[FPSTR(FP_VA)] = REMORA_VERSION; }

  String compiled =  __DATE__ " " __TIME__;
  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Compilé le";
    item[FPSTR(FP_VA)] = compiled; }

  char versHard[20];
  #if defined (REMORA_BOARD_V10)
    sprintf_P(versHard, "1.0");
  #elif defined (REMORA_BOARD_V11)
    sprintf_P(versHard, "1.1");
  #elif defined (REMORA_BOARD_V12)
    sprintf_P(versHard, "1.2 avec MCP23017");
  #elif defined (REMORA_BOARD_V13)
    sprintf_P(versHard, "1.3 avec MCP23017");
  #else
    sprintf_P(versHard, "Non définie");
  #endif
  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Version Matériel";
    item[FPSTR(FP_VA)] = versHard; }

  String modules = "";
  #ifdef MOD_OLED
    modules += F("OLED ");
  #endif
  #ifdef MOD_TELEINFO
    modules += F("TELEINFO ");
  #endif
  #ifdef MOD_RF69
    modules += F("RFM69 ");
  #endif
  #ifdef MOD_ADPS
    modules += F("ADPS");
  #endif
  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Modules activés";
    item[FPSTR(FP_VA)] = modules; }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "SDK Version";
    item[FPSTR(FP_VA)] = system_get_sdk_version(); }

  char chipid[9];
  sprintf_P(chipid, PSTR("0x%06X"), system_get_chip_id() );
  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Chip ID";
    item[FPSTR(FP_VA)] = chipid; }

  char boot_version[7];
  sprintf_P(boot_version, PSTR("0x%0X"), system_get_boot_version() );
  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Boot Version";
    item[FPSTR(FP_VA)] = boot_version; }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Flash Real Size";
    item[FPSTR(FP_VA)] = formatSize(ESP.getFlashChipRealSize()); }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Firmware Size";
    item[FPSTR(FP_VA)] = formatSize(ESP.getSketchSize()); }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Free Size";
    item[FPSTR(FP_VA)] = formatSize(ESP.getFreeSketchSpace()); }

  char analog[8];
  sprintf_P( analog, PSTR("%d mV"), ((1000*analogRead(A0))/1024) );
  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Analog";
    item[FPSTR(FP_VA)] = analog; }

  // WiFi Informations
  // =================
  WiFiMode_t wifiMode = WiFi.getMode();
  String ip;
  const char* modes[] = { "NULL", "STA", "AP", "STA+AP" };
  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Wifi Mode";
    item[FPSTR(FP_VA)] = modes[wifiMode]; }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Wifi Channel";
    item[FPSTR(FP_VA)] = wifi_get_channel(); }

  if (wifiMode == WIFI_AP || wifiMode == WIFI_AP_STA) {

    String mac = WiFi.softAPmacAddress();
    { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Wifi MAC";
    item[FPSTR(FP_VA)] = mac; }

    ip = WiFi.softAPIP().toString().c_str();
    { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Wifi IP";
    item[FPSTR(FP_VA)] = ip; }

    //char ipClients[128];
    //getClientAddresses(ipClients);
    { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Wifi clients";
    item[FPSTR(FP_VA)] = (int) WiFi.softAPgetStationNum(); }
    //sprintf_P( buffer, PSTR("%d (%s)"), WiFi.softAPgetStationNum(), ipClients);
  } else if (wifiMode == WIFI_STA) {

    String mac = WiFi.macAddress();
    { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Wifi MAC";
    item[FPSTR(FP_VA)] = mac; }

    ip = WiFi.localIP().toString().c_str();
    { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Wifi IP";
    item[FPSTR(FP_VA)] = ip; }
  }

  // SPIFFS Informations
  // ===================
  FSInfo info;
  SPIFFS.info(info);

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "SPIFFS Total";
    item[FPSTR(FP_VA)] =  formatSize(info.totalBytes); }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "SPIFFS Used";
    item[FPSTR(FP_VA)] = formatSize(info.totalBytes); }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "SPIFFS Occupation (%)";
    item[FPSTR(FP_VA)] = 100*info.usedBytes/info.totalBytes; }

  // regarder l'état de tous les fils Pilotes
  char fp;
  String labFp;
  String valFp;
  for (uint8_t i=1; i<=NB_FILS_PILOTES; i++)
  {
    fp = etatFP[i-1];
    labFp = PSTR("Fil Pilote #") + (String) i;
    if      (fp=='E') valFp = PSTR("Eco");
    else if (fp=='A') valFp = PSTR("Arrêt");
    else if (fp=='H') valFp = PSTR("Hors Gel");
    else if (fp=='1') valFp = PSTR("Conf - 1");
    else if (fp=='2') valFp = PSTR("Conf - 2");
    else if (fp=='C') valFp = PSTR("Confort");

    { JsonObject& item = arr.createNestedObject();
      item[FPSTR(FP_NA)] = labFp;
      item[FPSTR(FP_VA)] = valFp; }
  }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Etat Relais";
    item[FPSTR(FP_VA)] = etatrelais ? "Fermé":"Ouvert"; }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Fnct Relais";
    item[FPSTR(FP_VA)] = (fnctRelais == FNCT_RELAIS_AUTO) ? "Auto" : (fnctRelais == FNCT_RELAIS_FORCE) ? "Force" : "Stop"; }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Delestage";
    item[FPSTR(FP_VA)] = myDelestLimit; }

  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Relestage";
    item[FPSTR(FP_VA)] = myRelestLimit; }

  char stateDel[20];
  #ifdef MOD_ADPS
    sprintf_P(stateDel, PSTR("Niveau %d Zone %d"), nivDelest, plusAncienneZoneDelestee);
  #else
    sprintf_P(stateDel, PSTR("désactivé"));
  #endif
  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Etat Delestage";
    item[FPSTR(FP_VA)] = stateDel; }

  // Free mem should be last one but not really readable on bottom table
  { JsonObject& item = arr.createNestedObject();
    item[FPSTR(FP_NA)] = "Free Ram";
    item[FPSTR(FP_VA)] = formatSize(system_get_free_heap_size());  }


  // Web request send response to client
  // size_t jsonlen ;
  if (request) {
    response->setLength();
    request->send(response);
  } else {
    // Send JSon to our string
    arr.printTo(JsonStr);
    // arr.measureLength();
    // Since it's nor a WEB request, we need to manually delete
    // response object so ArduinJSon object is freed
    delete response;
  }
  //Debugf("Json size %lu bytes\r\n", jsonlen);

  // Will be empty for web request
  return JsonStr;
}

/* ======================================================================
Function: confJSONTable
Purpose : dump all config values in JSON table format for browser
Input   : request pointer if comming from web request
Output  : JsonStr filled if request is null
Comments: -
====================================================================== */
String confJSONTable(AsyncWebServerRequest *request)
{
  String JsonStr;

  AsyncJsonResponse * response = new AsyncJsonResponse(false);
  JsonObject& root = response->getRoot();

  DebuglnF("Serving /config page...");

  root[FPSTR(CFG_SSID)]       = config.ssid;
  root[FPSTR(CFG_PSK)]        = config.psk;
  root[FPSTR(CFG_HOST)]       = config.host;
  root[FPSTR(CFG_AP_PSK)]     = config.ap_psk;

  root[FPSTR(CFG_EMON_HOST)]  = config.emoncms.host;
  root[FPSTR(CFG_EMON_PORT)]  = (String) config.emoncms.port;
  root[FPSTR(CFG_EMON_URL)]   = config.emoncms.url;
  root[FPSTR(CFG_EMON_KEY)]   = config.emoncms.apikey;
  root[FPSTR(CFG_EMON_NODE)]  = (String) config.emoncms.node;
  root[FPSTR(CFG_EMON_FREQ)]  = (String) config.emoncms.freq;

  root[FPSTR(CFG_OTA_AUTH)]   = config.ota_auth;
  root[FPSTR(CFG_OTA_PORT)]   = config.ota_port;

  root[FPSTR(CFG_JDOM_HOST)]  = config.jeedom.host;
  root[FPSTR(CFG_JDOM_PORT)]  = (String) config.jeedom.port;
  root[FPSTR(CFG_JDOM_URL)]   = config.jeedom.url;
  root[FPSTR(CFG_JDOM_KEY)]   = config.jeedom.apikey;
  root[FPSTR(CFG_JDOM_ADCO)]  = config.jeedom.adco;
  root[FPSTR(CFG_JDOM_FREQ)]  = (String) config.jeedom.freq;

  // Web request send response to client
  // size_t jsonlen;
  if (request) {
    response->setLength();
    request->send(response);
  } else {
    // Send JSon to our string
    root.printTo(JsonStr);
    // jsonlen =  root.measureLength();
    // Since it's nor a WEB request, we need to manually delete
    // response object so ArduinJSon object is freed
    delete response;
  }
  //Debugf("Json size %lu bytes\r\n", jsonlen);

  // Will be empty for web request
  return JsonStr;
}

/* ======================================================================
Function: getSpiffsJSONData
Purpose : Return JSON string containing list of SPIFFS files
Input   : request pointer
Output  : -
Comments: -
====================================================================== */
void spiffsJSONTable(AsyncWebServerRequest *request)
{
  AsyncJsonResponse * response = new AsyncJsonResponse(false);
  JsonObject& root = response->getRoot();

  DebugF("Serving /spiffs page...");

  // Loop trough all files
  JsonArray& a_files = root.createNestedArray("files");
  Dir dir = SPIFFS.openDir("/");
  while (dir.next()) {
    JsonObject& o_item = a_files.createNestedObject();
    o_item[FPSTR(FP_NA)] = dir.fileName();
    o_item[FPSTR(FP_VA)] = dir.fileSize();
  }

  // Get SPIFFS File system informations
  FSInfo info;
  SPIFFS.info(info);
  JsonArray& a_spiffs = root.createNestedArray("spiffs");
  JsonObject& o_item = a_spiffs.createNestedObject();
  o_item["Total"] = info.totalBytes;
  o_item["Used"]  = info.usedBytes ;
  o_item["ram"]   = system_get_free_heap_size();

  // size_t jsonlen;
  response->setLength();
  request->send(response);
}

/* ======================================================================
Function: wifiScanJSON
Purpose : scan Wifi Access Point and return JSON code
Input   : request pointer
Output  : -
Comments: -
====================================================================== */
void wifiScanJSON(AsyncWebServerRequest *request)
{
  WiFi.scanNetworksAsync([request](int numNetworks) {
    String buff;
    AsyncJsonResponse * response = new AsyncJsonResponse(false);
    JsonObject& root = response->getRoot();
    JsonArray& arr = root.createNestedArray("result");

    // Just to debug where we are
    DebugF("Serving /wifiscan page...");

    for (uint8_t i = 0; i < numNetworks; ++i) {

      switch(WiFi.encryptionType(i)) {
        case ENC_TYPE_NONE: buff = "Open";  break;
        case ENC_TYPE_WEP:  buff = "WEP";   break;
        case ENC_TYPE_TKIP: buff = "WPA";   break;
        case ENC_TYPE_CCMP: buff = "WPA2";  break;
        case ENC_TYPE_AUTO: buff = "Auto";  break;
        default:            buff = "????";  break;
      }

      Debugf("[%d] '%s' Encryption=%s Channel=%d\r\n", i, WiFi.SSID(i).c_str(), buff.c_str(), WiFi.channel(i));

      JsonObject& item = arr.createNestedObject();
      item[FPSTR(FP_SSID)]       = WiFi.SSID(i);
      item[FPSTR(FP_RSSI)]       = WiFi.RSSI(i);
      item[FPSTR(FP_ENCRYPTION)] = buff;
      item[FPSTR(FP_CHANNEL)]    = WiFi.channel(i);
    }
    root[FPSTR(FP_STATUS)] = FPSTR(FP_OK);

    DebugF("sending...");
    response->setLength();
    request->send(response);
    DebuglnF("Ok!");
  }, false);
}

/* ======================================================================
Function: tinfoJSON
Purpose : dump all teleinfo values in JSON
Input   : -
Output  : -
Comments: -
====================================================================== */
void tinfoJSON(AsyncWebServerRequest *request)
{
  #ifdef MOD_TELEINFO
    AsyncJsonResponse * response = new AsyncJsonResponse(false);
    JsonObject& root = response->getRoot();

    ValueList * me = tinfo.getList();
    // String response = "";

    // Got at least one ?
    if (me) {
      char * p;
      long value;

      root[PSTR("_UPTIME")] = uptime;

      // Loop thru the node
      while (me->next) {
        // go to next node
        me = me->next;

        if (tinfo.calcChecksum(me->name,me->value) == me->checksum) {
          // Check if value is a number
          value = strtol(me->value, &p, 10);

          // conversion failed, add "value"
          if (*p) {
            root[me->name] = me->value;
          // number, add "value"
          } else {
            root[me->name] = value;
          }
          //formatNumberJSON(response, me->value);
        } else {
          String errTinfo = "";
          errTinfo += me->name;
          errTinfo += "=";
          errTinfo += me->value;
          errTinfo += F(" CHK=");
          errTinfo += (char) me->checksum;
          root[PSTR("_Error")] = errTinfo;
        }
      }
      response->setLength();
      request->send(response);
    } else {
      request->send(404, "text/plain", "No data");
    }
  #else
    request->send(404, "text/plain", "teleinfo not enabled");
  #endif
}


/* ======================================================================
Function: fpJSON
Purpose : return fp values in JSON
Input   : string
          fp number 1 to NB_FILS_PILOTE (0=ALL)
Output  : -
Comments: -
====================================================================== */
void fpJSON(String & response, uint8_t fp)
{
  AsyncJsonResponse * respJson = new AsyncJsonResponse(false);
  JsonObject& root = respJson->getRoot();

  DebugF("fpJSON fp: "); Debugln(fp);

  // regarder l'état de tous les fils Pilotes
  if (fp>=0 && fp<=NB_FILS_PILOTES) {
    String labFp; // fp key
    for (uint8_t i=1; i<=NB_FILS_PILOTES; i++)
    {
      // Tout les fils pilote ou juste celui demandé
      if (fp==0 || fp==i) {
        labFp = PSTR("fp") + (String) i;
        root[labFp] = (String) etatFP[i-1];
      }
    }
  }
  root.printTo(response);
}


/* ======================================================================
Function: relaisJSON
Purpose : return relais value in JSON
Input   : -
Output  : -
Comments: -
====================================================================== */
void relaisJSON(String & response)
{
  response = FPSTR(FP_JSON_START);
  response+= "\"relais\": ";
  response+= String(etatrelais);
  response+= ",\r\n\"fnct_relais\": ";
  response+= String(fnctRelais);
  response+= FPSTR(FP_JSON_END);
}

/* ======================================================================
Function: delestageJSON
Purpose : return delestage values in JSON
Input   : -
Output  : -
Comments: -
====================================================================== */
void delestageJSON(String & response)
{
    response = FPSTR(FP_JSON_START);
    #ifdef MOD_ADPS
      response += FPSTR("\"niveau\": ");
      response += String(nivDelest);
      response += FPSTR(", \"zone\": ");
      response += String(plusAncienneZoneDelestee);
    #else
      response += FPSTR("\"etat\": \"désactivé\"");
    #endif
    response += FPSTR(FP_JSON_END);
}


/* ======================================================================
Function: handleFactoryReset
Purpose : reset the module to factory settingd
Input   : -
Output  : -
Comments: -
====================================================================== */
void handleFactoryReset(AsyncWebServerRequest *request)
{
  // Just to debug where we are
  DebugF("Serving /factory_reset page...");
  resetConfig();
  ESP.eraseConfig();
  request->send(200, "text/plain", FPSTR(FP_RESTART));
  delay(1000);
  ESP.restart();
  while (true)
    delay(1);
}

/* ======================================================================
Function: handleReset
Purpose : reset the module
Input   : -
Output  : -
Comments: -
====================================================================== */
void handleReset(AsyncWebServerRequest *request)
{
  // Just to debug where we are
  DebugF("Serving /reset page...");
  request->send(200, "text/plain", FPSTR(FP_RESTART));
  delay(1000);
  ESP.restart();
  // This will fire watchdog
  while (true)
    delay(1);
}

/* ======================================================================
Function: handleFormConfig
Purpose : handle main configuration page
Input   : -
Output  : -
Comments: -
====================================================================== */
void handleFormConfig(AsyncWebServerRequest *request)
{
  String response="";
  int ret ;
  boolean showconfig = false;
  long val;

  // We validated config ?
  if (request->hasParam("save", true)) {
    DebuglnF("===== Posted configuration");

    // WifInfo
    strncpy(config.ssid ,   request->getParam("ssid", true)->value().c_str(),     CFG_SSID_SIZE );
    strncpy(config.psk ,    request->getParam("psk", true)->value().c_str(),      CFG_PSK_SIZE );
    strncpy(config.host ,   request->getParam("host", true)->value().c_str(),     CFG_HOSTNAME_SIZE );
    strncpy(config.ap_psk , request->getParam("ap_psk", true)->value().c_str(),   CFG_PSK_SIZE );
    if (
      strlen(config.ota_auth) != request->getParam("ota_auth", true)->value().length()
      && config.ota_auth != request->getParam("ota_auth", true)->value().c_str())
    {
      strncpy(config.ota_auth, request->getParam("ota_auth", true)->value().c_str(), CFG_PSK_SIZE );
      reboot = true;
    }
    val = request->getParam("ota_port", true)->value().toInt();
    config.ota_port = (val>=0 && val<=65535) ? val : DEFAULT_OTA_PORT ;

    // Emoncms
    strncpy(config.emoncms.host,   request->getParam("emon_host", true)->value().c_str(),  CFG_EMON_HOST_SIZE );
    strncpy(config.emoncms.url,    request->getParam("emon_url", true)->value().c_str(),   CFG_EMON_URL_SIZE );
    strncpy(config.emoncms.apikey, request->getParam("emon_apikey", true)->value().c_str(),CFG_EMON_APIKEY_SIZE );
    val = request->getParam("emon_node", true)->value().toInt();
    config.emoncms.node = (val>=0 && val<=255) ? (uint16_t) val : 0;
    val = request->getParam("emon_port", true)->value().toInt();
    config.emoncms.port = (val>=0 && val<=65535) ? (uint16_t) val : CFG_EMON_DEFAULT_PORT;
    val = request->getParam("emon_freq", true)->value().toInt();
    if (val>0 && val<=86400){
      // Relaunch task send tinfo to Emoncms
      Tick_emoncms.detach();
      Tick_emoncms.attach(val, Task_emoncms);
    } else {
      Tick_emoncms.detach();
      val = 0 ;
    }
    config.emoncms.freq = (uint32_t) val;

    // jeedom
    strncpy(config.jeedom.host,   request->getParam("jdom_host", true)->value().c_str(),  CFG_JDOM_HOST_SIZE );
    strncpy(config.jeedom.url,    request->getParam("jdom_url", true)->value().c_str(),   CFG_JDOM_URL_SIZE );
    strncpy(config.jeedom.apikey, request->getParam("jdom_apikey", true)->value().c_str(),CFG_JDOM_APIKEY_SIZE );
    strncpy(config.jeedom.adco,   request->getParam("jdom_adco", true)->value().c_str(),  CFG_JDOM_ADCO_SIZE );
    // Check if has param jdom_port
    if (request->hasParam("jdom_port", true)) {
      val = request->getParam("jdom_port", true)->value().toInt();
      config.jeedom.port = (val>=0 && val<=65535) ? (uint16_t) val : CFG_JDOM_DEFAULT_PORT;
    }
    val = request->getParam("jdom_freq", true)->value().toInt();
    if (val>0 && val<=86400){
      // Relaunch task send tinfo to Jeedom
      Tick_jeedom.detach();
      Tick_jeedom.attach(val, Task_jeedom);
    } else {
      Tick_jeedom.detach();
      val = 0 ;
    }
    config.jeedom.freq = (uint32_t) val;

    if ( saveConfig() ) {
      ret = 200;
      response = "OK";
    } else {
      ret = 412;
      response = "Unable to save configuration";
    }

    showconfig = true;
  }
  else
  {
    ret = 400;
    response = "Missing Form Field";
  }

  DebugF("Sending response ");
  Debug(ret);
  Debug(":");
  Debugln(response);
  request->send (ret, "text/plain", response);

  // This is slow, do it after response sent
  if (showconfig)
    showConfig();
}

void handle_fw_upload(AsyncWebServerRequest *request, String filename, size_t index, uint8_t *data, size_t len, bool final) {
  if (!index) {
    Update.runAsync(true);
    WiFiUDP::stopAll();
    DebugF("* Upload Started: "); Debugln(filename.c_str());
    LedRGBON(COLOR_MAGENTA);
    ota_blink = true;
    int command = U_FLASH;
    //Debugf("Magic Byte: %02X\n", data[0]);
    if (data[0] != 0xE9) {
      command = U_SPIFFS;
      SPIFFS.end();
      //DebuglnF("Command U_SPIFFS");
    }
    if (!Update.begin((ESP.getFreeSketchSpace() - 0x1000) & 0xFFFFF000, command)) {
      Update.printError(DEBUG_SERIAL);
    }
  }

  if (!Update.hasError()) {
    if (Update.write(data, len) != len) {
      DebugF("*** UPDATE ERROR: ");
      Update.printError(DEBUG_SERIAL);
      if (ota_blink) {
        LedRGBON(COLOR_RED);
      } else {
        LedRGBOFF();
      }
      ota_blink = !ota_blink;
    } else {
      if (ota_blink) {
        LedRGBON(COLOR_MAGENTA);
      } else {
        LedRGBOFF();
      }
      ota_blink = !ota_blink;
      Debug(".");
    }
  }

  if (final) {
    DebuglnF("* Upload Finished.");
    if (Update.end(true)) {
      Debugf("Update Success: %uB\n", index+len);
    } else {
      Update.printError(DEBUG_SERIAL);
    }
    LedRGBOFF();
  }
}

/* ======================================================================
Function: handleNotFound
Purpose : default WEB routing when URI is not found
Input   : -
Output  : -
Comments: We search is we have a name that match to this URI, if one we
          return it's pair name/value in json
====================================================================== */
void handleNotFound(AsyncWebServerRequest *request)
{
  String response = "";

  String sUri = request->url();
  const char * uri;
  bool found = false;
  bool first_elem = true;

  // convert uri to char * for compare
  uri = sUri.c_str();

  Debug("URI[");
  Debug(strlen(uri));
  Debug("]='");
  Debug(uri);
  Debugln("'");

  // Got consistent URI, skip fisrt / ?
  // Attention si ? dans l'URL çà ne fait pas partie de l'URI
  // mais de hasArg traité plus bas
  if (uri && *uri=='/' && *++uri ) {
    uint8_t len = strlen(uri);

    #ifdef MOD_TELEINFO
      // We check for an known label
      ValueList * me = tinfo.getList();

      // Got at least one ?
      if (me) {

        // Loop thru the linked list of values
        while (me->next && !found) {
          // go to next node
          me = me->next;

          //Debugf("compare to '%s' ", me->name);
          // Do we have this one ?
          if (!strcasecmp(me->name, uri)) {
            // no need to continue
            found = true;

            // Add to respone
            response += FPSTR("{\r\n\"") ;
            response += me->name ;
            response += F("\":") ;
            formatNumberJSON(response, me->value);
            response += FPSTR(FP_JSON_END);
          }
        }
      }
    #endif

    // Requêtes d'interrogation
    // ========================

    // http://ip_remora/relais
    if (!strcasecmp("relais", uri)) {
      relaisJSON(response);
      found = true;
    // http://ip_remora/delestage
    } else if (!strcasecmp("delestage", uri)) {
      delestageJSON(response);
      found = true;
    // http://ip_remora/fp ou http://ip_remora/fpx
    } else if ( (len==2 || len==3) && (uri[0]=='f'||uri[0]=='F') && (uri[1]=='p'||uri[1]=='P') ) {
      int8_t fp = -1;

      // http://ip_remora/fp
      if (len==2) {
        fp=0;

      // http://ip_remora/fpx
      } else if ( len==3 ) {
        fp = uri[2];
        if ( fp>='1' && fp<=('0'+NB_FILS_PILOTES) )
         fp -= '0';
      }

      if (fp>=0 && fp<=NB_FILS_PILOTES) {
        fpJSON(response, fp);
        found = true;
      }
    }
  } // valide URI


  // Requêtes modifiantes (cumulable)
  // ================================
  if (  request->hasParam("fp") ||
        request->hasParam("setfp") ||
        request->hasParam("relais") ||
        request->hasParam("frelais")) {

    int error = 0;
    response = FPSTR(FP_JSON_START);

    // http://ip_remora/?setfp=CMD
    if ( request->hasParam("setfp") ) {
      String value = request->getParam("setfp")->value();
      error += setfp(value);
    }
    // http://ip_remora/?fp=CMD
    if ( request->hasParam("fp") ) {
      String value = request->getParam("fp")->value();
      error += fp(value);
    }

    // http://ip_remora/?relais=n
    if ( request->hasParam("relais") ) {
      String value = request->getParam("relais")->value();
      // La nouvelle valeur n'est pas celle qu'on vient de positionner ?
      if ( relais(value) != request->getParam("relais")->value().toInt() )
        error--;
    }

    // http://ip_remora/?frelais=n (n: 0 | 1 | 2)
    if ( request->hasParam("frelais") ) {
      String value = request->getParam("frelais")->value();
      // La nouvelle valeur n'est pas celle qu'on vient de positionner ?
      if ( fnct_relais(value) != request->getParam("frelais")->value().toInt() )
        error--;
    }

    response += F("\"response\": ") ;
    response += String(error) ;

    response += FPSTR(FP_JSON_END);
    found = true;
  }

  // Got it, send json
  if (found) {
    request->send(200, "application/json", response);
  } else {
    // le fichier demandé existe sur le système SPIFFS ?
    found = handleFileRead(request->url(), request);
  }

  // send error message in plain text
  if (!found) {
    String message = F("File Not Found\n\n");
    message += "URI: ";
    message += request->url();
    message += "\nMethod: ";
    message += ( request->method() == HTTP_GET ) ? "GET" : "POST";
    message += "\nArguments: ";
    message += request->params();
    message += "\n";

    uint8_t params = request->params();

    for ( uint8_t i = 0; i < params; i++ ) {
      AsyncWebParameter* p = request->getParam(i);
      message += " " + p->name() + ": " + p->value() + "\n";
    }
    request->send(404, "text/plain", message);
  }

}

#endif
