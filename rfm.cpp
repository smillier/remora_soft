// **********************************************************************************
// RFM69 module management source file for remora project
// **********************************************************************************
// Creative Commons Attrib Share-Alike License
// You are free to use/extend but please abide with the CC-BY-SA license:
// http://creativecommons.org/licenses/by-sa/4.0/
//
// Written by Charles-Henri Hallard (http://hallard.me)
//
// History : V1.00 2015-01-22 - First release
//
// All text above must be included in any redistribution.
//
// **********************************************************************************

#include "./linked_list.h"
#include "./LibULPNode_RF_Protocol.h"
#include "./rfm.h"
#include "./LibRH_RF69.h"


unsigned long rf_rgb_led_timer = 0;
unsigned long packet_last_seen=0;// second since last packet received

// data received by RF module
// independent from module received (RF12 or RF69)
// used to display or send to serial
RFData rfData;
_SensorData sensorData; // sensor data received

// Linked list of nodes seens
NodeList nodes_list;

// RadioHead Library need driver and message class
//RH_RF69     rf69_drv(RF69_CS, RF69_IRQ);// instance of the radio driver
//RHDatagram  rf69(rf69_drv);         // Manage message delivery and receipt
//RH_RF69 * prf69_drv = &rf69_drv;
#ifdef ESP8266
  #ifdef RH_RF69_IRQLESS
    RH_RF69 driver(RF69_CS, RF69_IRQ);// instance of the radio driver
  #else
    // instance of the radio driver we can't use GPPIO2
    RH_RF69 driver(RF69_IRQ, RF69_CS);
  #endif
#else
  // instance of the radio driver
  RH_RF69 driver(RF69_CS, RF69_IRQ);
#endif

/* ======================================================================
Function: rfm_receive_data
Purpose : receive data payload on RF and manage ACK
Input   : -
Output  : millis since this node has been seen
Comments: called from loop, this is a non blocking receiver
====================================================================== */
unsigned long rfm_receive_data(void)
{
  unsigned long node_last_seen = 0;

  LedRGBON(COLOR_BLUE);
  rf_rgb_led_timer = millis();

  // Fill known values
  rfData.size    = sizeof(rfData.buffer);
  rfData.type  = RF_MOD_RFM69;
  rfData.groupid = RFM69_NETWORKID;
  rfData.ack = '\0';  // default no ack

  // grab the frame received
  if (driver.recv(rfData.buffer, &rfData.size)) {
    bool  known_node = false;
    int   nb_node;
    int   current;

    // Get header data
    rfData.nodeid = driver.headerFrom();
    rfData.gwid   = driver.headerTo();
    rfData.seqid  = driver.headerId();
    rfData.flags  = driver.headerFlags();
    rfData.rssi   = driver.lastRssi();

    // Are we authorized to send response ?
    // This is in case of multiple gateways with same ID
    //if (!(config.mode & CFG_RF69_NOSEND)) {
    if (true) {
      // Sender explicitly requested a ACK ?
      if (rfData.flags & RF_PAYLOAD_REQ_ACK) {
        //DebugF("Frame from ");
        //Debug(rfData.nodeid);
        //DebugF(" to ");
        //Debug(rfData.gwid);
        //DebugF(" want ACK ");

        // Never ACK on broadcast address or a already ACK response
        if (rfData.gwid != RH_BROADCAST_ADDRESS && !(rfData.flags & RH_FLAGS_ACK)) {
          //DebugF("Me=");
          //Debug(rfData.gwid);
          //DebugF(" Sending ACK to ");
          //Debug(rfData.nodeid);

          // Header is now ACK response (set) and no more ACK Request (clear)
          driver.setHeaderFlags(RH_FLAGS_ACK, RF_PAYLOAD_REQ_ACK);
          driver.setHeaderId(rfData.seqid);
          driver.setHeaderTo(rfData.nodeid);
          driver.setHeaderFrom(rfData.gwid);
          rfData.ack = '!';

          // We have powerfull speed CPU, but Wait slave to setup the receiver for
          // ACK reception
          #if (RF_GATEWAY_PLATFORM==RF_GATEWAY_PLATFORM_PARTICLE) || defined (ESP8266)
            delay(1);
            //delayMicroseconds(100);
          #endif

          // Send the response packet
          driver.send(&rfData.ack, sizeof(rfData.ack));
          driver.waitPacketSent();

          // ACK makes led to green
          #if defined (RGB_LED_PIN)
            LedRGBON(COLOR_GREEN);
          #endif

        }
      }
    }
    // Prepare our last seen value
    node_last_seen = uptime;

    ll_Add(&nodes_list, rfData.groupid, rfData.nodeid, rfData.rssi, &node_last_seen);
    //ll_Dump(&nodes_list, g_second);

  } // revcfrom()

  return node_last_seen;
}


/* ======================================================================
Function: rfm_setup
Purpose : prepare and init stuff, configuration, ..
Input   : -
Output  : true if RF module found, false otherwise
Comments: -
====================================================================== */
bool rfm_setup(void)
{
  bool ret = false;

  // RF Radio initialization
  // =======================
  // Defaults after RadioHead init are
  // 434.0MHz
  // modulation GFSK_Rb250Fd250, +13dbM
  // No encryption
  if (driver.init()) {
    ret = true;
    driver.setFrequency( (float) RFM69_FREQUENCY /100.0f);
    // If you are using a high power RF69, you *must* set a Tx power in the
    // range 14 to 20 like this:
    driver.setTxPower(RFM69_TXPWR);

    // set driver parameters
    driver.setThisAddress(RFM69_NODEID);  // filtering address when receiving
    driver.setHeaderFrom(RFM69_NODEID);   // Transmit From Node

    // Init of our linked list of seen received nodes
    nodes_list.next     = NULL ;
    nodes_list.groupid  = 0 ;
    nodes_list.nodeid   = 0 ;
    nodes_list.rssi     = 0 ;
    nodes_list.lastseen = 0 ;

    sensorData.temp = SENSOR_NOT_A_TEMP;
    sensorData.hum  = SENSOR_NOT_A_HUM;
    sensorData.bat  = SENSOR_NOT_A_BAT;
    sensorData.lux  = SENSOR_NOT_A_LUX;
  }

  return (ret);

}

/* ======================================================================
Function: rfm_loop
Purpose : really need to tell ?
Input   : -
Output  : -
Comments: -
====================================================================== */
void rfm_loop(void)
{
  //static unsigned long packet_last_seen=0;// second since last packet received
  uint8_t packetReceived = 0;
  unsigned long node_last_seen;  // Second since we saw this node
  unsigned long currentMillis = millis();
  RFData data;

  // Data received from driver ?
  if (driver.available()) {
    node_last_seen = rfm_receive_data();
    packet_last_seen = uptime;
    packetReceived = true;
    data = rfData;
    got_first = true;
  }

  // received data ?
  if (packetReceived) {
    // command code
    uint8_t cmd = rfData.buffer[0];
    unsigned long seen = uptime-node_last_seen;

    #define DEBUG_VERBOSE
    #ifdef DEBUG_VERBOSE
      // Dump Raw packet
      DebugF("# (");
      Debug(uptime);
      DebugF(")");

      if (rfData.flags & RF_PAYLOAD_REQ_ACK)
        DebugF(" ACKED");


      DebugF(" <- node:");  DEBUG_SERIAL.print(rfData.nodeid,DEC);
      DebugF(" size:");     Debug(rfData.size);
      DebugF(" type:");     DEBUG_SERIAL.print(decode_frame_type(cmd));
      DebugF(" (0x");       DEBUG_SERIAL.print(cmd,HEX);
      DebugF(") RSSI:");    DEBUG_SERIAL.print(rfData.rssi,DEC);
      DebugF("dB  seen :");
      Debug(timeAgo(seen));

      // Detail to know exactly number of seconds between node send
      if ( seen<=300 && seen>0 ) {
        DebugF(" (");
        Debug(seen);
        DebugF(")");
      }

      DebugF("\r\n# buffer:");

      char buff[4];
      for (uint8_t i=0; i<rfData.size; i++) {
        sprintf_P(buff, PSTR(" %02X"), rfData.buffer[i]);
        Debug(buff);
      }

      DebugF("  ");
      #ifdef SPARK
        Debug(System.freeMemory());
      #else
        Debug(ESP.getFreeHeap());
      #endif
      DebuglnF(" Bytes free ");
    #endif

    // decode format
    // return command code validated by payload type size received
    // so if we had a command and the payload does not match
    // code as been set to 0 by decode_received_data
    cmd = decode_received_data(rfData.nodeid, rfData.rssi, rfData.size, cmd, rfData.buffer);

    //DebugF("2 cmd: "); DEBUG_SERIAL.print(cmd,HEX); DebugF(" - DHCP_REQUEST cmd: "); DEBUG_SERIAL.print(RF_PL_DHCP_REQUEST,HEX); Debugln();
    //Debugflush();
   // special ping packet, we need to answer back
   if ( cmd==RF_PL_PING ) {
     RFPingPayload * ppl = (RFPingPayload *) rfData.buffer;

     // prepare send back response
     ppl->command = RF_PL_PINGBACK;
     ppl->vbat = 0;
     ppl->rssi = rfData.rssi; // RSSI of node
     ppl->status = 0;

     driver.setHeaderId(rfData.seqid);
     driver.setHeaderFlags(RH_FLAGS_NONE);

     // We're on a fast gateway, let node some time
     // To node to set to receive mode before sending response
     delay(2);
     driver.setHeaderTo(rfData.nodeid);
     driver.send((uint8_t *) ppl, (uint8_t) sizeof(RFPingPayload)) ;
     driver.waitPacketSent();

     // Start line with a # (comment)
     // indicate external parser that it's just debug information
     DebugF("\r\n# -> ");
     DEBUG_SERIAL.print(rfData.nodeid,DEC);
     DebugF(" PINGBACK (");
     DEBUG_SERIAL.print(ppl->rssi,DEC);
     DebuglnF("dB)");
   }
  // Respond to DHCP_REQUEST for send new ID at node
  else if (cmd == RF_PL_DHCP_REQUEST) {
    // Dirty global to display
    StaticJsonBuffer<256> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(json_str);

    if (!root.success()) {
      DebuglnF("JSON parseObject() failed");
    } else {
      DebugF("Received...");
      uint16_t nodeid = 0;
      // Save sensor value for display later
      if (root.containsKey("chipid")) {
        Debugf("chipid=%s", root["chipid"].asString());
        uint32_t chipid = root["chipid"].as<unsigned long>();
        nodeid = get_node_id();
      }
      s_dhcp * ppl = (s_dhcp *) rfData.buffer;

      ppl->code = RF_PL_DHCP_OFFER;
      ppl->nodeid = nodeid; // TODO: Ajouter la gestion dynamique d'affectation d'identifiant
      ppl->networkid = RFM69_NETWORKID;

      driver.setHeaderId(rfData.seqid);
      driver.setHeaderFlags(RH_FLAGS_NONE);

      delay(2);
      driver.send((uint8_t *) ppl, (uint8_t) sizeof(s_dhcp));
      DebuglnF("nodeid sent, wait packet sent");
      driver.waitPacketSent();

      // Start line with a # (comment)
      // indicate external parser that it's just debug information
      DebugF("\r\n# -> ");
      DEBUG_SERIAL.print(rfData.nodeid, DEC);
      DebugF(" DHCP OFFER (");
      DEBUG_SERIAL.print(rfData.rssi, DEC);
      DebuglnF("dB)");
    }
  }

   // known Payload ? send frame to serial
   // known Payload ? send frame to serial
   else if (cmd) {
    uint8_t post_err = 0;

    // This is not debug, should be printed, it's the data
    Serial.println(json_str);
    Serial.flush();

    // Dirty global to display
    StaticJsonBuffer<256> jsonBuffer;
    JsonObject& root = jsonBuffer.parseObject(json_str);

    sensorData.temp = SENSOR_NOT_A_TEMP;
    sensorData.hum  = SENSOR_NOT_A_HUM;
    sensorData.bat  = SENSOR_NOT_A_BAT;
    sensorData.lux  = SENSOR_NOT_A_LUX;

    if (!root.success()) {
      DebuglnF("JSON parseObject() failed");
    } else {
      DebugF("Received...");

      // Save sensor value for display later
      if (root.containsKey("temp")) {
        Debugf("temp=%s", root["temp"].asString());
        sensorData.temp = root["temp"].as<double>() * 100;
      }
      if (root.containsKey("hum") ) {
        Debugf(" hum=%s", root["hum"].asString());
        sensorData.hum  = root["hum"].as<double>() * 10;
      }
      if (root.containsKey("bat") ) {
        Debugf(" bat=%s", root["bat"].asString());
        sensorData.bat  = root["bat"].as<double>() * 1000;
      }
      if (root.containsKey("lux") ) {
        Debugf(" lux=%s", root["lux"].asString());
        sensorData.lux  = root["lux"].as<double>() * 10;
      }
      DebuglnF(" ...OK!");
    }

    // ArduinoJson parsing broke our JSON str, redo
    //decode_received_data(0, rfData.rssi, rfData.size, cmd, rfData.buffer);

    Serial.println(json_str);

    #ifdef MOD_OLED
    if (status & STATUS_OLED) {
      // switch to RF Frame immediatly
      ui->switchToFrame(RF_FRAME_INDEX);
      ui->update();
    }
    #endif
  }

   // Start blue led
   LedRGBON(COLOR_BLUE);
   rf_rgb_led_timer=millis();

   // Display Results only if something new received
   // As display on OLED is quite long, this avoid time out
   // on ack of RF module because if we are in display, we can't
   // ACK quickly and may be delayed by 100ms, but if we are
   // there this is because we received a command and treated ACK
   //#ifdef MOD_LCD
  //   screen_state = screen_rf;
  //   doDisplay();
  // #endif

  }

  // Do we have led timer expiration ?
  if (rf_rgb_led_timer && (millis()-rf_rgb_led_timer >= RF_LED_BLINK_MS)) {
   LedRGBOFF();     // Light Off the LED
   rf_rgb_led_timer=0; // Stop virtual timer
  }

}
