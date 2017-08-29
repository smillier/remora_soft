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

#ifndef FLASH_STR_H_
#define FLASH_STR_H_

// Include main project include file
#include "./remora.h"

extern const char FP_NA[];
extern const char FP_VA[];
extern const char FP_RESTART[];

extern const char CFG_SSID[];
extern const char CFG_PSK[];
extern const char CFG_HOST[];
extern const char CFG_AP_PSK[];
extern const char CFG_AP_SSID[];
extern const char CFG_OTA_AUTH[];
extern const char CFG_OTA_PORT[];

extern const char CFG_EMON_HOST[];
extern const char CFG_EMON_PORT[];
extern const char CFG_EMON_URL[];
extern const char CFG_EMON_KEY[];
extern const char CFG_EMON_NODE[];
extern const char CFG_EMON_FREQ[];

extern const char CFG_JDOM_HOST[];
extern const char CFG_JDOM_PORT[];
extern const char CFG_JDOM_URL[];
extern const char CFG_JDOM_KEY[];
extern const char CFG_JDOM_ADCO[];
extern const char CFG_JDOM_FREQ[];

#endif // FLASH_STR_H_
