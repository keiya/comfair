#include <Arduino.h>


#include <DaikinHeatpumpARC480A14IR.h>



#ifndef ESP8266
IRSenderPWM irSender(9);       // IR led on Arduino digital pin 9, using Arduino PWM
//IRSenderBlaster irSender(3); // IR led on Arduino digital pin 3, using IR Blaster (generates the 38 kHz carrier)
#else
IRSenderBitBang irSender(D2);  // IR led on Wemos D1 mini, connect between D2 and G
#endif

// Array with all supported heatpumps
HeatpumpIR *heatpumpIR = new DaikinHeatpumpARC480A14IR();

void setup()
{
  Serial.begin(9600);
  delay(500);

  Serial.println(F("Starting"));
  Serial.println(String(DAIKIN_AIRCON_HDR_MARK));
}

void loop()
{
  if (Serial.available() > 0) {
    // read the incoming string:
    String command = Serial.readStringUntil('\n');

    if (command.startsWith("dry")) {
      if (command == "dry-2") {
        dry(0xDC);
      } else if (command == "dry-1") {
        dry(0xDE);
      } else if (command == "dry0") {
        dry(0xC0);
      } else if (command == "dry+1") {
        dry(0xC2);
      } else if (command == "dry+2") {
        dry(0xC4);
      }
    } else if (command.startsWith("cool")) {
      heatpumpIR->send(irSender, POWER_ON, MODE_COOL, FAN_SILENT, 26, VDIR_UP, HDIR_AUTO);
    } else if (command.startsWith("old")) {
      //old();
    }
  }
  delay(100);
}

//void old() {
//  uint8_t daikinTemplate[19] = {
//  // cold @ 18c fan speed 5       MODE  TEMP         FAN                     \QUIET POWERFUL/     \specials/
//  //0x11  0xDA  0x27  0x00  0x00  0x31  0x24  0x00  0xA0  0x00  0x00  0x00  0x00  0x00  0x00  0xC5  0x02  0x08  0xD6
//    0x11, 0xDA, 0x27, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x60, 0x00, 0x00, 0xC5, 0x00, 0x08, 0x00 };
//   // 0     1     2     3     4     5     6     7     8     9    10     11    12    13    14    15    16    17    18
//   //                                                           | on/off timers  |
//  daikinTemplate[5] = DAIKIN_AIRCON_MODE_COOL;
//  daikinTemplate[6] = 25 << 1;
//  daikinTemplate[8] = DAIKIN_AIRCON_FAN5 + DAIKIN_AIRCON_SWING_OFF;
//  daikinTemplate[13] = DAIKIN_AIRCON_QUIET_OFF + DAIKIN_AIRCON_POWERFUL_OFF;
//  daikinTemplate[16] = DAIKIN_AIRCON_COMFORT_OFF + DAIKIN_AIRCON_ECONO_OFF + DAIKIN_AIRCON_SENSOR_OFF;
//
//  // Checksum calculation
//  // * Checksums at bytes 7 and 15 are calculated the same way
//  uint8_t checksum = 0x00;
//
//  for (int i=0; i<18; i++) {
//    checksum += daikinTemplate[i];
//  }
//
//  daikinTemplate[18] = checksum;
//
//  // 38 kHz PWM frequency
//  irSender.setFrequency(38);
//
//  // Header
//  irSender.mark(DAIKIN_AIRCON_HDR_MARK);
//  irSender.space(DAIKIN_AIRCON_HDR_SPACE);
//
//  // First header
//  for (int i=0; i<19; i++) {
//    irSender.sendIRbyte(daikinTemplate[i], DAIKIN_AIRCON_BIT_MARK, DAIKIN_AIRCON_ZERO_SPACE, DAIKIN_AIRCON_ONE_SPACE);
//  }
//
//  irSender.mark(DAIKIN_AIRCON_BIT_MARK);
//  irSender.space(0);
//}

void dry(uint8_t tempDelta) {
  uint8_t daikinTemplate[39] = {
    // dry @ -2 fan speed auto
    //   0     1     2     3     4     5     6     7     8     9   
    //                                MODE  TEMP         FAN
/* 0 */ 0x11, 0xDA, 0x27, 0x00, 0x02, 0x00, 0x00, 0x00, 0x00, 0x00,
/* 1 */ 0x00, 0x00, 0x10, 0x00, 0x40, 0x00, 0x00, 0x00, 0x00, 0x64, 
    //                                   \DRY TEMP/
/* 2 */ 0x11, 0xDA, 0x27, 0x00, 0x00, 0x29, 0xDC, 0x80, 0xA0, 0x00,
/* 3 */ 0x00, 0x06, 0x60, 0x00, 0x00, 0xC3, 0x00, 0x00, 0x60 };
  daikinTemplate[26] = tempDelta;
  uint8_t checksum1 = 0x00;
  uint8_t checksum2 = 0x00;

  for (int i=0; i<19; i++) {
    checksum1 += daikinTemplate[i];
  }
  for (int i=20; i<38; i++) {
    checksum2 += daikinTemplate[i];
  }

  daikinTemplate[19] = checksum1;
  daikinTemplate[38] = checksum2;

    // 38 kHz PWM frequency
  irSender.setFrequency(38);

  // Header
  irSender.mark(DAIKIN_AIRCON_HDR_MARK);
  irSender.space(DAIKIN_AIRCON_HDR_SPACE);

  // First header
  for (int i=0; i<20; i++) {
    irSender.sendIRbyte(daikinTemplate[i], DAIKIN_AIRCON_BIT_MARK, DAIKIN_AIRCON_ZERO_SPACE, DAIKIN_AIRCON_ONE_SPACE);
  }

  // Pause + new header
  irSender.mark(DAIKIN_AIRCON_BIT_MARK);
  irSender.space(DAIKIN_AIRCON_MSG_SPACE);
  irSender.mark(DAIKIN_AIRCON_HDR_MARK);
  irSender.space(DAIKIN_AIRCON_HDR_SPACE);

    // Second header
  for (int i=20; i<39; i++) {
    irSender.sendIRbyte(daikinTemplate[i], DAIKIN_AIRCON_BIT_MARK, DAIKIN_AIRCON_ZERO_SPACE, DAIKIN_AIRCON_ONE_SPACE);
  }

  irSender.mark(DAIKIN_AIRCON_BIT_MARK);
  irSender.space(0);
}
