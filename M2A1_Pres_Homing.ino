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