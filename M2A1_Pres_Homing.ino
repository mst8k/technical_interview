#if 0
void M2A1_Pres_Homing() { 

    errorFlag = NO_ERROR; 
    unsigned long homingStartTime = millis(); 

    Serial.println("     Homing (+) the Power Pump Started..."); 

    ModbusRTU(30, 1); 
    delay(1000); 

    ModbusRTU(21, 1); 
    delay(1000); 

    M2A1.setMaxSpeed(M2A1_Max_Speed);      
    M2A1.setAcceleration(M2A1_Accel);  
    M2A1.setSpeed(M2A1_Speed); 
    digitalWrite(M2A1_ENABLE_PIN, LOW); 

    while (digitalRead(M2LS1_PIN) == LOW) { 
        if (millis() - homingStartTime >= M2A1homingTimeout) { 
            Serial.println("ERROR: Power Pump Homing timeout - Limit switch not reached"); 
            errorFlag = SLAVE_TIMEOUT_ERROR; 
            digitalWrite(M2A1_ENABLE_PIN, HIGH); 
            sendData(); 
            return; 
        } 
        M2A1.runSpeed(); 
    } 

    M2A1.setCurrentPosition(0); 
    M2A1.runToNewPosition(-M2A1_Offset_Homing); 
    M2A1.run(); 

    ModbusRTU(30, 0); 
    delay(1000); 

    ModbusRTU(21, 0); 
    delay(1000); 

    digitalWrite(M2A1_ENABLE_PIN, HIGH); 
    delay(1000); 

    Serial.println("     Homing (+) the Power Pump Completed."); 
    Serial.println(""); 
    delay(1000); 

}

// Due to a PCB design issue in this hardware revision, the RS-485 communication lines used for relay control (ModbusRTU) 
// share a common ground with the TTL serial lines used for master-slave communication (MASTER_SERIAL / Serial). 
// When relays are triggered, ground bounce and electrical noise can corrupt data frames being transmitted or received on the serial communication lines. 

// In the code above: 

// Serial.println() sends status messages to the master controller 
// ModbusRTU() triggers relays via RS485, causing electrical interference 
// sendData() sends critical error/status information to the master 

// The hardware team estimates a 6-month lead time for a PCB redesign. Your task is to propose software-level workarounds
//  to minimise data loss and ensure reliable operation with the current hardware. 
  
// Task

// Propose 2–3 software-based mitigation strategies with code examples. 

#define OPTION1
// #define OPTION2
// #define OPTION3

#if defined(OPTION1)
// checking a quiet window before sending. If anyone tries to send a message before the pre defined quite window 
// the system will not let it do that
unsigned long lastModbusTime = 0;
const unsigned long MODBUS_SETTLE_TIME = 50; // ms

void ModbusRTU_WithTimeCapture(int cmd, int state)
{
    ModbusRTU(cmd, state);
    lastModbusTime = millis();
}

bool canSendSerial(void)
{
    return (millis() - lastModbusTime) > MODBUS_SETTLE_TIME;
}

void safeSerialPrint(const char *msg)
{
    if (canSendSerial())
    {
        Serial.println(msg);
    }
    else
    {

        // Skip or Retry or queue later or make the Serial.printLn a blocking one as follows
        // while (!canSendSerial()) delay(10); // wait until it is safe to transmit
        // Serial.println(msg);
    }
}

void safeSendData(const char *msg)
{
    if (canSendSerial())
    {
        sendData();
    }
    else
    {
        // Skip or Retry or queue later or make the sendData() a blocking one as follows
        // while (!canSendSerial()) delay(10); // wait until it is safe to transmit
        // sendData();
        // Or errorFlag can be used to indicate failure and use it in case of a critical message for retry
    }
}

/* usage example
ModbusRTU_WithTimeCapture(30, 1);
safeSerialPrint("Relay 30 triggered");
*/

#elif defined(OPTION2)

#define SERIAL_BUFFER_SIZE 10
#define MSG_LEN 64
unsigned long lastModbusTime = 0;
const unsigned long MODBUS_SETTLE_TIME = 150; // ms

char serialBuffer[SERIAL_BUFFER_SIZE][MSG_LEN];
int wIndex = 0;
int rIndex = 0;
int bufferCount = 0;

void ModbusRTU_WithTimeCapture(int cmd, int state)
{
    ModbusRTU(cmd, state);
    lastModbusTime = millis();
}

bool canSendSerial(void)
{
    return (millis() - lastModbusTime) > MODBUS_SETTLE_TIME;
}

void queueSerial(const char *msg)
{
    if (bufferCount < SERIAL_BUFFER_SIZE)
    {
        strncpy(serialBuffer[wIndex], msg, MSG_LEN - 1);
        serialBuffer[wIndex][MSG_LEN - 1] = '\0';
        wIndex = (wIndex + 1) % SERIAL_BUFFER_SIZE;
        bufferCount++;
    } else {
        // Buffer full, handle overflow (e.g., discard message or overwrite oldest) 
    }
}

void processSerialQueue(void)
{
    if (bufferCount > 0 && canSendSerial())
    {
        Serial.println(serialBuffer[rIndex]);
        rIndex = (rIndex + 1) % SERIAL_BUFFER_SIZE;
        bufferCount--;
    }
}

/* usage example
queueSerial("Starting homing...");
ModbusRTU_WithTimeCapture(30, 1);
processSerialQueue(); // This should be called everytime in a loop 
*/

#elif defined(OPTION3)

#define NOISE_WINDOW_MS 20
#define MAX_MESSAGES 10

typedef struct {
    char msg[64];
    bool isCritical;
} Msg;

Msg Msgs[MAX_MESSAGES];
int msgCount = 0;
unsigned long lastRelayTime = 0;
bool inNoiseWindow = false;

void addMessage(const char* msg, bool critical) {
    if (msgCount < MAX_MESSAGES) {
        strncpy(Msgs[msgCount].msg, msg, 63);
        Msgs[msgCount].msg[63] = '\0';
        Msgs[msgCount].isCritical = critical;
        msgCount++;
    } else {
        // Buffer full, handle overflow (e.g., discard message or overwrite oldest) 
    }
}

void processMessages() {
    if (inNoiseWindow && millis() - lastRelayTime >= NOISE_WINDOW_MS) {
        inNoiseWindow = false;
    }
    if (!inNoiseWindow && msgCount > 0) {
        if (Msgs[0].isCritical) sendData();
        else Serial.println(Msgs[0].msg);
        // Shift messages
        for (int i = 1; i < msgCount; i++) Msgs[i - 1] = Msgs[i];
        msgCount--;
    }
}

void M2A1_Pres_Homing_NonBlocking() {
    static int stage = 0;
    static unsigned long lastTime = 0, startTime = 0;

    processMessages();

    switch (stage) {
        case 0:
            errorFlag = NO_ERROR;
            startTime = millis();
            addMessage("     Homing (+) the Power Pump Started...", false);
            lastTime = millis();
            stage = 1;
            break;
        case 1:
            if (millis() - lastTime >= NOISE_WINDOW_MS) {
                ModbusRTU(30, 1);
                lastRelayTime = millis();
                inNoiseWindow = true;
                lastTime = millis();
                stage = 2;
            }
            break;
        case 2:
            if (millis() - lastTime >= 1000) {
                ModbusRTU(21, 1);
                lastRelayTime = millis();
                inNoiseWindow = true;
                lastTime = millis();
                stage = 3;
            }
            break;
        case 3:
            if (millis() - lastTime >= NOISE_WINDOW_MS) {
                M2A1.setMaxSpeed(M2A1_Max_Speed);
                M2A1.setAcceleration(M2A1_Accel);
                M2A1.setSpeed(M2A1_Speed);
                digitalWrite(M2A1_ENABLE_PIN, LOW);
                stage = 4;
            }
            break;
        case 4:
            if (digitalRead(M2LS1_PIN) == HIGH) {
                stage = 5;
            } else if (millis() - startTime >= M2A1homingTimeout) {
                addMessage("ERROR: Power Pump Homing timeout - Limit switch not reached", false);
                errorFlag = SLAVE_TIMEOUT_ERROR;
                digitalWrite(M2A1_ENABLE_PIN, HIGH);
                addMessage("", true); // Queue sendData()
                stage = 6;
            } else {
                M2A1.runSpeed();
            }
            break;
        case 5:
            M2A1.setCurrentPosition(0);
            M2A1.runToNewPosition(-M2A1_Offset_Homing);
            M2A1.run();
            lastTime = millis();
            stage = 6;
            break;
        case 6:
            if (millis() - lastTime >= 1000) {
                ModbusRTU(30, 0);
                lastRelayTime = millis();
                inNoiseWindow = true;
                lastTime = millis();
                stage = 7;
            }
            break;
        case 7:
            if (millis() - lastTime >= 1000) {
                ModbusRTU(21, 0);
                lastRelayTime = millis();
                inNoiseWindow = true;
                lastTime = millis();
                stage = 8;
            }
            break;
        case 8:
            if (millis() - lastTime >= NOISE_WINDOW_MS) {
                digitalWrite(M2A1_ENABLE_PIN, HIGH);
                lastTime = millis();
                stage = 9;
            }
            break;
        case 9:
            if (millis() - lastTime >= NOISE_WINDOW_MS) {
                addMessage("     Homing (+) the Power Pump Completed.", false);
                addMessage("", false);
                stage = 0;
            }
            break;
    }
}

/* usage example
In the main loop, call M2A1_Pres_Homing_NonBlocking() repeatedly
*/
#else
    #error "Please define one of the options (OPTION1, OPTION2, or OPTION3)"
#endif
#endif