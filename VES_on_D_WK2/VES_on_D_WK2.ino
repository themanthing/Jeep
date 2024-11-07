#include <mcp_can.h>
#include <SPI.h>

// Скетч активирует режим VES в авто JEEP WK2 и разблокирует видео-в-движении
unsigned long rxId;
byte len;
byte rxBuf[8];
MCP_CAN JEEP(9);   // CS от адаптера "в машину"
MCP_CAN MYGIG(10);  // CS от адаптера "в радио"

byte ves1[] = {0x00, 0x04, 0x00, 0x07, 0x00, 0x00, 0x00, 0x07};
byte ves2[] = {0x01, 0x00, 0x50, 0x00, 0x00, 0x00, 0x00, 0x44};

void setup() {
  JEEP.begin(MCP_ANY, CAN_125KBPS, MCP_8MHZ);
  JEEP.setMode(MCP_NORMAL); // Change to normal mode to allow messages to be transmitted
  MYGIG.begin(MCP_ANY, CAN_125KBPS, MCP_8MHZ);
  MYGIG.setMode(MCP_NORMAL); // Change to normal mode to allow messages to be transmitted
}

void loop() {
  if(!digitalRead(3)){          // INT от адаптера со стороны машины. Считываем, если он в LOW.
    JEEP.readMsgBuf(&rxId, &len, rxBuf);  // читаем из шины и посмотрим, пропускать ли дальше в радио
    if (rxId == 0x20E) { // положение КПП
      if ( (rxBuf[2] == 0x84 && rxBuf[4] == 0x44) || (rxBuf[2] == 0x82 && rxBuf[4] == 0x4E) ) { // D или N
        rxBuf[2] = 0x80; // меняем на P
        rxBuf[4] = 0x50;
      }
      MYGIG.sendMsgBuf(rxId, 0, len, rxBuf);
    }    
    else if (rxId == 0x21F) {
      MYGIG.sendMsgBuf(0x3A1, 0, 0x08, ves1); // разблокирует режим VES - после этого видео можно транслировать на ГУ
      MYGIG.sendMsgBuf(0x21F, 0, len, ves2);  // включает VES-меню на ГУ. Питание ГУ нужно передернуть иначе оно помнит что меню отсутствует
    }
    else {
      MYGIG.sendMsgBuf(rxId, 0, len, rxBuf);
    }
  }
  if(!digitalRead(2)){                    // INT от адаптера со стороны радио.
    MYGIG.readMsgBuf(&rxId, &len, rxBuf);
    JEEP.sendMsgBuf(rxId, 0, len, rxBuf);
  }
}
