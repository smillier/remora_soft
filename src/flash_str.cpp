// **********************************************************************************
// Remora Strings stored in flash for size Optimization
// **********************************************************************************
// Creative Commons Attrib Share-Alike License
// You are free to use/extend this library but please abide with the CC-BY-SA license:
// Attribution-NonCommercial-ShareAlike 4.0 International License
// http://creativecommons.org/licenses/by-nc-sa/4.0/
//
// For any explanation about teleinfo ou use , see my blog
// http://hallard.me/category/tinfo
//
// This program works with the Wifinfo board
// see schematic here https://github.com/hallard/teleinfo/tree/master/Wifinfo
//
// Written by Charles-Henri Hallard (http://hallard.me)
//
// History : V1.00 2015-06-14 - First release
//
// All text above must be included in any redistribution.
//
// **********************************************************************************

#include "flash_str.h"

// Used in JSON array for bootstrap table
const char FP_NA[]          PROGMEM = "na";
const char FP_VA[]          PROGMEM = "va";
const char FP_RESTART[]     PROGMEM = "OK, Redémarrage en cours\r\n";

const char CFG_SSID[]       PROGMEM = "ssid";
const char CFG_PSK[]        PROGMEM = "psk";
const char CFG_HOST[]       PROGMEM = "host";
const char CFG_AP_PSK[]     PROGMEM = "appsk";
const char CFG_AP_SSID[]    PROGMEM = "apssid";
const char CFG_OTA_AUTH[]   PROGMEM = "ota_auth";
const char CFG_OTA_PORT[]   PROGMEM = "ota_port";

const char CFG_EMON_HOST[]  PROGMEM = "emon_host";
const char CFG_EMON_PORT[]  PROGMEM = "emon_port";
const char CFG_EMON_URL[]   PROGMEM = "emon_url";
const char CFG_EMON_KEY[]   PROGMEM = "emon_apikey";
const char CFG_EMON_NODE[]  PROGMEM = "emon_node";
const char CFG_EMON_FREQ[]  PROGMEM = "emon_freq";

const char CFG_JDOM_HOST[]  PROGMEM = "jdom_host";
const char CFG_JDOM_PORT[]  PROGMEM = "jdom_port";
const char CFG_JDOM_URL[]   PROGMEM = "jdom_url";
const char CFG_JDOM_KEY[]   PROGMEM = "jdom_apikey";
const char CFG_JDOM_ADCO[]  PROGMEM = "jdom_adco";
const char CFG_JDOM_FREQ[]  PROGMEM = "jdom_freq";
