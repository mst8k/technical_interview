#include "M2PP2_Sample_Lysis_Mixing.h"
// A simple Arduino sketch to demonstrate the usage of M2PP2 Sample Lysis Mixing library.
void setup() {
    Serial.begin(9600);
    M2PP2_Setup();
}

void loop() {
    static bool mixingStarted = false;
    if (!mixingStarted) {
        if (M2PP2_Start(2, 2048)) {
            mixingStarted = true;
        } else {
            while(1) {
                delay(1000);
            }
        }
    }
    
    if (mixer.isRunning) {
        MixingStatus status = M2PP2_Sample_Lysis_Mixing_NonBlocking();
        
        switch (status) {
            case MIXING_COMPLETE:
                Serial.println("Mixing completed");
                mixingStarted = false; 
                while(1) {
                    delay(1000);
                }
                break;
                
            case MIXING_ERROR:
                Serial.println("Mixing error!");
                M2PP2_Reset();  
                mixingStarted = false; 
                while(1) {
                    delay(1000);
                }         
                break;
                
            case MIXING_IN_PROGRESS:
                break;
        }
    }
}