#include <Arduino.h>

#define GNSS_RX 43 // UART_TX
#define GNSS_TX 44 // UART_RX

HardwareSerial Serial_GNSS(2);

enum Command {
    SET = 0,
    GET,
    SAVE,
    RESTORE,
    REBOOT,
};

const char* commands[] = {
    "$PSTMSETPAR",
    "$PSTMGETPAR",
    "$PSTMSAVEPAR",
    "$PSTMRESTOREPAR",
    "$PSTMSRR",
};

enum Register {
    NMEA_PORT_MSG_LIST_0_LOW = 201,
    NMEA_PORT_MSG_LIST_0_HIGH = 228,
};

void gnssConf_read_test() {
    int avail;
    Serial_GNSS.printf("%s,3%i\r\n", commands[Command::GET], NMEA_PORT_MSG_LIST_0_LOW);  // Expected: 0x288435F
    Serial.print("Sent: \"");
    Serial.printf("%s,3%i\r\n", commands[Command::GET], NMEA_PORT_MSG_LIST_0_LOW);
    Serial.print("\"\n");
    
    delay(10);
    avail = Serial_GNSS.available();
    if(avail) {
        Serial.printf("Received %i bytes: \"", avail);
        for (uint32_t idx = 0; idx < avail; idx++) {
            const int readByte = Serial_GNSS.read();
            Serial.printf("%c", readByte);
        }
        Serial.print("\"\n");
    }
    

    Serial_GNSS.printf("%s,3%i\r\n", commands[Command::GET], NMEA_PORT_MSG_LIST_0_HIGH); // Expected: 0x2000
    Serial.print("Sent: \"");
    Serial.printf("%s,3%i\r\n", commands[Command::GET], NMEA_PORT_MSG_LIST_0_HIGH);
    Serial.print("\"\n");
    delay(10);
    avail = Serial_GNSS.available();
    if(avail) {
        Serial.printf("Received %i bytes: \"", avail);
        for (uint32_t idx = 0; idx < avail; idx++) {
            const int readByte = Serial_GNSS.read();
            Serial.printf("%c", readByte);
        }
        Serial.print("\"\n");
    }
}

void gnssConf_get() {
    Serial.print(" Get\n");
    Serial_GNSS.flush();
    Serial_GNSS.printf("%s,3%i\r\n",
                       commands[Command::GET],
                       NMEA_PORT_MSG_LIST_0_LOW);
    Serial_GNSS.printf("%s,3%i\r\n",
                       commands[Command::GET],
                       NMEA_PORT_MSG_LIST_0_HIGH);
    const int avail = Serial_GNSS.available();
    
    Serial.print("Read: ");
    for(uint32_t idx = 0; idx < avail; idx++) {
        const int readByte = Serial_GNSS.read();
        Serial.printf("%c", readByte);
    }
}

// Settings are in C:\Users\MenMe\Downloads\um2229-teseoliv3-gnss-module--software-manual-stmicroelectronics.pdf 12.14 CDB-ID 190 - CDB-ID 201 - CDB-ID 228 - NMEA on UART message list parameters
void gnssConf_set() {
    const uint32_t NMEA_PORT_MSG_LIST_0_LOW_VAL = 0x1 | 0x2; //| 0x4 | 0x8 | 0x10; //GPGNS, GPGGA
    const uint32_t NMEA_PORT_MSG_LIST_0_HIGH_VAL = 0;

    Serial.print(" Set\n");
    Serial_GNSS.printf("%s,3%i,0x%x\r\n",
                       commands[Command::SET],
                       NMEA_PORT_MSG_LIST_0_LOW,
                       NMEA_PORT_MSG_LIST_0_LOW_VAL);
    Serial_GNSS.printf("%s,3%i,0x%x\r\n",
                       commands[Command::SET],
                       NMEA_PORT_MSG_LIST_0_HIGH,
                       NMEA_PORT_MSG_LIST_0_HIGH_VAL);

}

// NMEA Port Msg-List 0
// 0x288435F and 0x2000
// CDB-ID 201 and CDB-ID 228

void gnssConf_reset() {
    const uint32_t NMEA_PORT_MSG_LIST_0_LOW_DEFAULT_VAL = 0x288435F;
    const uint32_t NMEA_PORT_MSG_LIST_0_HIGH_DEFAULT_VAL = 0x2000;

    Serial.print(" Reset\n");
    Serial_GNSS.printf("%s,3%i,0x%x\r\n",
                       commands[Command::SET],
                       NMEA_PORT_MSG_LIST_0_LOW,
                       NMEA_PORT_MSG_LIST_0_LOW_DEFAULT_VAL);
    Serial_GNSS.printf("%s,3%i,0x%x\r\n",
                       commands[Command::SET],
                       NMEA_PORT_MSG_LIST_0_HIGH,
                       NMEA_PORT_MSG_LIST_0_HIGH_DEFAULT_VAL);
}

void gnssConf_resetAll() {
    Serial.print(" Reset all\n");
    Serial.print("Are you sure? y/N\n");
    while (!Serial.available()) {
        delay(1);
    }
    
    switch(Serial.read()) {
    case 'y':
        goto gnssConf_resetAll_confirm;
        break;
    default:
        Serial.print(" Quit");
        return;
    }

    gnssConf_resetAll_confirm:
    Serial.print(" Reseting");
    Serial_GNSS.printf("%s\r\n", commands[Command::RESTORE]);
}

void gnssConf_save() {
    Serial.print(" Save\n");
    Serial.print("Are you sure? y/N\n");
    while (!Serial.available()) {
        delay(1);
    }
    
    switch(Serial.read()) {
    case 'y':
        goto gnssConf_save_confirm;
        break;
    default:
        Serial.print(" Quit");
        return;
    }

    gnssConf_save_confirm:
    Serial.print(" Saving");
    Serial_GNSS.printf("%s\r\n", commands[Command::SAVE]);
}

void gnssConf_reboot() {
    Serial.print(" Reboot\n");
    Serial.print("Are you sure? y/N\n");
    while (!Serial.available()) {
        delay(1);
    }
    
    switch(Serial.read()) {
    case 'y':
        goto gnssConf_reboot_confirm;
        break;
    default:
        Serial.print(" Quit");
        return;
    }

    gnssConf_reboot_confirm:
    Serial.print(" Rebooting");
    Serial_GNSS.printf("%s\r\n", commands[Command::REBOOT]);
}

void gnssConf_printBuffer() {
    Serial.print(" Print\n");
    const int avail = Serial_GNSS.available();
    if(avail) {
        Serial.printf("The thing says: \"\n", avail);
        for(uint32_t idx = 0; idx < avail; idx++) {
            const int readByte = Serial_GNSS.read();
            Serial.printf("%c", readByte);
        }
        Serial.print("\"\n");
    }
}


void setup() {
    Serial.begin(115200);
    
    Serial_GNSS.begin(
        9600,                // Baud rate (adjust if needed)
        SERIAL_8N1,
        GNSS_TX,
        GNSS_RX
    );
    
    delay(2000);


    Serial.printf(
   "GNSS thing NMEA messages configuration setter\n"
   "g\tGets current config\n"
   "s\tSets config from a constant\n"
   "r\tResets to factory defaults\n"
   "R\tResets whole config to defaults\n"
   "w\tSaves changes\n"
   "q\tReboots the thing (to apply changes)\n"
   "p\tPrints out buffer that receives stuff from the thing\n"
    );

    Serial.print("> ");
}

void loop() {
    if(Serial.available()) {
        const int newChar = Serial.read();
        switch(newChar) {
        case 'g':
            gnssConf_get();
            break;
        case 's':
            gnssConf_set();
            break;
        case 'r':
            gnssConf_reset();
            break;
        case 'R':
            gnssConf_resetAll();
            break;
        case 'w':
            gnssConf_save();
            break;
        case 'q':
            gnssConf_reboot();
            break;
        case 'p':
            gnssConf_printBuffer();
            break;
        default:
            Serial.printf("Unknown option '%c'\n", newChar);
            break;
        }

        Serial.print("> ");
    }
    delay(100);
}