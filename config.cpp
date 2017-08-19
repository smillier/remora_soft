// **********************************************************************************
// Remora Configuration source file
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
// History 2016-02-04 - First release
//
// All text above must be included in any redistribution.
//
// **********************************************************************************

#include "./config.h"

#ifdef ESP8266

// Configuration structure for whole program
_Config config;

uint16_t crc16Update(uint16_t crc, uint8_t a)
{
  int i;
  crc ^= a;
  for (i = 0; i < 8; ++i)  {
    if (crc & 1)
      crc = (crc >> 1) ^ 0xA001;
    else
      crc = (crc >> 1);
  }
  return crc;
}

/* ======================================================================
Function: eeprom_dump
Purpose : dump eeprom value to serial
Input 	: -
Output	: -
Comments: -
====================================================================== */
void eepromDump(uint8_t bytesPerRow)
{
  uint16_t i,b;
  char buf[10];
  uint16_t j=0 ;

  // default to 16 bytes per row
  if (bytesPerRow==0)
    bytesPerRow=16;

  Debugln();

  // loop thru EEP address
  for (i = 0; i < sizeof(_Config); i++) {
    // First byte of the row ?
    if (j==0) {
			// Display Address
      Debugf("%04X : ", i);
    }

    // write byte in hex form
    Debugf("%02X ", EEPROM.read(i));

		// Last byte of the row ?
    // start a new line
    if (++j >= bytesPerRow) {
			j=0;
      Debugln();
		}
  }
}

/* ======================================================================
Function: readConfig
Purpose : fill config structure with data located into eeprom
Input 	: true if we need to clear actual struc in case of error
Output	: true if config found and crc ok, false otherwise
Comments: -
====================================================================== */
bool readConfig (bool clear_on_error)
{
	uint16_t crc = ~0;
	uint8_t * pconfig = (uint8_t *) &config ;
	uint8_t data ;

	// For whole size of config structure
	for (uint16_t i = 0; i < sizeof(_Config); ++i) {
		// read data
		data = EEPROM.read(i);

		// save into struct
		*pconfig++ = data ;

		// calc CRC
		crc = crc16Update(crc, data);
	}

  #ifdef MOD_RF69
    if (!read_config_rfm69()) {
      DebuglnF("Error read config rfm69");
    }
  #endif

	// CRC Error ?
	if (crc != 0) {
		// Clear config if wanted
    if (clear_on_error)
		  memset(&config, 0, sizeof( _Config ));
		return false;
	}

	return true ;
}

/* ======================================================================
Function: saveConfig
Purpose : save config structure values into eeprom
Input 	: -
Output	: true if saved and readback ok
Comments: once saved, config is read again to check the CRC
====================================================================== */
bool saveConfig (void)
{
  uint8_t * pconfig ;
  bool ret_code;

  //eepromDump(32);

  // Init pointer
  pconfig = (uint8_t *) &config ;

	// Init CRC
  config.crc = ~0;

	// For whole size of config structure, pre-calculate CRC
  for (uint16_t i = 0; i < sizeof (_Config) - 2; ++i)
    config.crc = crc16Update(config.crc, *pconfig++);

	// Re init pointer
  pconfig = (uint8_t *) &config ;

  // For whole size of config structure, write to EEP
  for (uint16_t i = 0; i < sizeof(_Config); ++i)
    EEPROM.write(i, *pconfig++);

  // Physically save
  EEPROM.commit();

  // Read Again to see if saved ok, but do
  // not clear if error this avoid clearing
  // default config and breaks OTA
  ret_code = readConfig(false);

  DebugF("Write config ");

  if (ret_code) {
    Debugln(F("OK!"));
  } else {
    Debugln(F("Error!"));
  }

  //eepromDump(32);

  // return result
  return (ret_code);
}

/* ======================================================================
Function: showConfig
Purpose : display configuration
Input 	: -
Output	: -
Comments: -
====================================================================== */
void showConfig()
{
  DebuglnF("===== Wifi");
  DebugF("ssid     :"); Debugln(config.ssid);
  DebugF("psk      :"); Debugln(config.psk);
  DebugF("host     :"); Debugln(config.host);
  DebugF("ap_psk   :"); Debugln(config.ap_psk);
  DebugF("OTA auth :"); Debugln(config.ota_auth);
  DebugF("OTA port :"); Debugln(config.ota_port);
  DebugF("Config   :");
  if (config.config & CFG_RGB_LED) DebugF(" RGB");
  if (config.config & CFG_DEBUG)   DebugF(" DEBUG");
  if (config.config & CFG_LCD)     DebugF(" LCD");
  _wdt_feed();

  DebuglnF("\r\n===== Emoncms");
  DebugF("host     :"); Debugln(config.emoncms.host);
  DebugF("port     :"); Debugln(config.emoncms.port);
  DebugF("url      :"); Debugln(config.emoncms.url);
  DebugF("key      :"); Debugln(config.emoncms.apikey);
  DebugF("node     :"); Debugln(config.emoncms.node);
  DebugF("freq     :"); Debugln(config.emoncms.freq);
  _wdt_feed();

  DebuglnF("\r\n===== Jeedom");
  DebugF("host     :"); Debugln(config.jeedom.host);
  DebugF("port     :"); Debugln(config.jeedom.port);
  DebugF("url      :"); Debugln(config.jeedom.url);
  DebugF("key      :"); Debugln(config.jeedom.apikey);
  DebugF("compteur :"); Debugln(config.jeedom.adco);
  DebugF("freq     :"); Debugln(config.jeedom.freq);
  _wdt_feed();
}

/* ======================================================================
Function: ResetConfig
Purpose : Set configuration to default values
Input   : -
Output  : -
Comments: -
====================================================================== */
void resetConfig(void)
{
  // enable default configuration
  memset(&config, 0, sizeof(_Config));

  // Set default Hostname
  sprintf_P(config.host, PSTR("Remora_%06X"), ESP.getChipId());
  strcpy_P(config.ota_auth, PSTR(DEFAULT_OTA_AUTH));
  config.ota_port = DEFAULT_OTA_PORT ;

  // Add other init default config here

  // Emoncms
  strcpy_P(config.emoncms.host, CFG_EMON_DEFAULT_HOST);
  config.emoncms.port = CFG_EMON_DEFAULT_PORT;
  strcpy_P(config.emoncms.url, CFG_EMON_DEFAULT_URL);
  config.emoncms.apikey[0] = '\0';
  config.emoncms.node = 0;
  config.emoncms.freq = 0;

  // Jeedom
  strcpy_P(config.jeedom.host, CFG_JDOM_DEFAULT_HOST);
  config.jeedom.port = CFG_JDOM_DEFAULT_PORT;
  strcpy_P(config.jeedom.url, CFG_JDOM_DEFAULT_URL);
  strcpy_P(config.jeedom.adco, CFG_JDOM_DEFAULT_ADCO);
  config.jeedom.apikey[0] = '\0';
  config.jeedom.freq = 0;


  config.config |= CFG_RGB_LED;

  // save back
  saveConfig();
}

/* ======================================================================
Function: check_config_file
Purpose : check json configuration meet requierement
Input   : -
Output  : file size if okay 0 otherwise
Comments: -
====================================================================== */
size_t check_config_file(const char * cfg_file)
{
  File configFile = SPIFFS.open(cfg_file, "r");
  if (!configFile) {
    DebugfF(PSTR("Failed to open %s file\r\n"), cfg_file);
    return 0;
  }

  size_t size = configFile.size();
  if (size > 1024) {
    DebugfF(PSTR("file %s size is too large\r\n"), cfg_file);
    configFile.close();
    return 0;
  }

  configFile.close();
  DebugfF(PSTR("file %s looks good (%d bytes)\r\n"), cfg_file, size);
  return size;
}

/* ======================================================================
Function: write_config_file
Purpose : write json configuration meet requierement
Input   : -
Output  : file size if okay 0 otherwise
Comments: -
====================================================================== */
size_t write_config_file(const char * cfg_file, char * buffer)
{
  File configFile = SPIFFS.open(cfg_file, "w");
  if (!f) {
    DebugfF(PSTR("file %s creation failed\r\n"), cfg_file);
    return 0;
  }

  DebuglnF("====== Writing to SPIFFS file =========");
  configFile.print(buffer);
  size_t size = configFile.size();

  configFile.close();
  DebugfF(PSTR("file %s looks good (%d bytes)\r\n"), cfg_file, size);
  return size;
}

/* ======================================================================
Function: read_config_rfm69
Purpose : read json RFM69 module config file
Input   : -
Output  : true if config ok, false otherwise
Comments: -
====================================================================== */
bool read_config_rfm69(void)
{
  const char cfg_file[] = "/cfg_nwk_rfm69.json";
  bool ret = false;

  size_t size = check_config_file(cfg_file);

  if ( size ) {

    File configFile = SPIFFS.open(cfg_file, "r");

    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);

    // We don't use String here because ArduinoJson library requires the input
    // buffer to be mutable. If you don't use ArduinoJson, you may as well
    // use configFile.readString instead.
    if ( configFile.readBytes(buf.get(), size) == size ) {
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(buf.get());

      if (json.success()) {
        //const char* data ;

        if (json["nodeids"]) {
          JsonArray& array = json["nodeids"];
          uint8_t sizeArr = array.size();
          if (sizeArr > 0) {
            for (uint8_t i = 0; i < sizeArr; i++) {
              _rfm_network_nodeid node;
              JsonObject& temp = array.get<JsonObject&>(i);
              node->chipid = (uint32_t) temp.chipid;
              node->nodeid = (uint8_t) temp.nodeid;
              nodeids[i] = node;
              delete node;
              delete temp;
            }
          }
        }

        /*int8_t cs_pin = json["rfm69"]["cs_pin"];
        if (cs_pin != config.rf69.cs_pin) {
          config.rf69.cs_pin = cs_pin;
          DebugfF(PSTR("CS %d\r\n"), config.rf69.cs_pin);
        }

        int8_t irq_pin = json["rfm69"]["irq_pin"];
        if (irq_pin != config.rf69.irq_pin) {
          config.rf69.irq_pin = irq_pin;
          DebugfF(PSTR("IRQ %d\r\n"), config.rf69.irq_pin);
        }

        uint8_t nodeid = json["rfm69"]["nodeid"];
        if ( nodeid != config.rf69.nodeid) {
          config.rf69.nodeid = nodeid;
          DebugfF(PSTR("Node ID %d\r\n"), config.rf69.nodeid);
        }

        uint8_t groupid = json["rfm69"]["groupid"];
        if ( groupid != config.rf69.groupid) {
          config.rf69.groupid = groupid;
          DebugfF(PSTR("Group ID %d\r\n"), config.rf69.groupid);
        }

        uint8_t modemconfig = json["rfm69"]["modemconfig"];
        if (modemconfig != config.rf69.modemconfig) {
          config.rf69.modemconfig = modemconfig;
          DebugfF(PSTR("ModemCfg %d\r\n"), config.rf69.modemconfig);
        }

        int8_t palevel = json["rfm69"]["palevel"];
        if ( palevel != config.rf69.palevel) {
          config.rf69.palevel = palevel;
          DebugfF(PSTR("Power %d\r\n"), config.rf69.palevel);
        }

        float fvalue = json["rfm69"]["frequency"];
        fvalue *=  100.0f ;
        uint32_t frequency = (uint32_t) (fvalue) ;
        if ( frequency != config.rf69.frequency) {
          config.rf69.frequency = frequency ;
          DebugfF(PSTR("Frequency %ld\r\n"), config.rf69.frequency);
        }*/

        // All is fine
        ret = true;
      } else {
        DebuglnF("Failed to parse file");
      }
    } else {
      DebuglnF("Failed to read file content");
    }

    if (configFile) {
      configFile.close();
    }

  }
  return ret;
}

/* ======================================================================
Function: read_config_rfm69
Purpose : read json RFM69 module config file
Input   : -
Output  : true if config ok, false otherwise
Comments: -
====================================================================== */
bool write_config_rfm69(void)
{
  const char cfg_file[] = "/cfg_nwk_rfm69.json";
  bool ret = false;

  size_t size = check_config_file(cfg_file);

  if ( size ) {

    File configFile = SPIFFS.open(cfg_file, "r");

    // Allocate a buffer to store contents of the file.
    std::unique_ptr<char[]> buf(new char[size]);

    // We don't use String here because ArduinoJson library requires the input
    // buffer to be mutable. If you don't use ArduinoJson, you may as well
    // use configFile.readString instead.
    if ( configFile.readBytes(buf.get(), size) == size ) {
      DynamicJsonBuffer jsonBuffer;
      JsonObject& json = jsonBuffer.parseObject(buf.get());
      configFile.close();

      if (json.success()) {
        JsonArray& array = jsonBuffer.createArray();
        for (uint8_t i = 0; i < CFG_RFM_MAX_NODES; i++) {
          if (nodeids[i] && nodeids[i].chipid) {
            JsonObject& objNode = jsonBuffer.createObject();
            objNode["chipid"] = nodeids[i].chipid;
            objNode["nodeid"] = nodeids[i].nodeid;
            if (!array.add(objNode)) {
              DebuglnF("Error add objNode in array");
            }
          } else {
            break;
          }
        }

        json.set("nodeids", array);

        size_t lenJson = json.measureLength();
        char * buffer[lenJson+1];
        json.printTo(buffer, lenJson+1);
        if (write_config_file(cfg_file, buffer) > 0) {
          DebuglnF("Config file RFM69 writed");
        }

        /*int8_t cs_pin = json["rfm69"]["cs_pin"];
        if (cs_pin != config.rf69.cs_pin) {
          config.rf69.cs_pin = cs_pin;
          DebugfF(PSTR("CS %d\r\n"), config.rf69.cs_pin);
        }

        int8_t irq_pin = json["rfm69"]["irq_pin"];
        if (irq_pin != config.rf69.irq_pin) {
          config.rf69.irq_pin = irq_pin;
          DebugfF(PSTR("IRQ %d\r\n"), config.rf69.irq_pin);
        }

        uint8_t nodeid = json["rfm69"]["nodeid"];
        if ( nodeid != config.rf69.nodeid) {
          config.rf69.nodeid = nodeid;
          DebugfF(PSTR("Node ID %d\r\n"), config.rf69.nodeid);
        }

        uint8_t groupid = json["rfm69"]["groupid"];
        if ( groupid != config.rf69.groupid) {
          config.rf69.groupid = groupid;
          DebugfF(PSTR("Group ID %d\r\n"), config.rf69.groupid);
        }

        uint8_t modemconfig = json["rfm69"]["modemconfig"];
        if (modemconfig != config.rf69.modemconfig) {
          config.rf69.modemconfig = modemconfig;
          DebugfF(PSTR("ModemCfg %d\r\n"), config.rf69.modemconfig);
        }

        int8_t palevel = json["rfm69"]["palevel"];
        if ( palevel != config.rf69.palevel) {
          config.rf69.palevel = palevel;
          DebugfF(PSTR("Power %d\r\n"), config.rf69.palevel);
        }

        float fvalue = json["rfm69"]["frequency"];
        fvalue *=  100.0f ;
        uint32_t frequency = (uint32_t) (fvalue) ;
        if ( frequency != config.rf69.frequency) {
          config.rf69.frequency = frequency ;
          DebugfF(PSTR("Frequency %ld\r\n"), config.rf69.frequency);
        }*/

        // All is fine
        ret = true;
      } else {
        DebuglnF("Failed to parse file");
      }
    } else {
      DebuglnF("Failed to read file content");
    }

    if (configFile) {
      configFile.close();
    }

  }
  return ret;
}

#endif // ESP8266