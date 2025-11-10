#include "M2PP2_Sample_Lysis_Mixing.h"

#define M2PP2_LOG_ENABLED 1

#if M2PP2_LOG_ENABLED
  #define M2PP2_LOG(fmt, ...)  do { Serial.println("[M2PP2] " fmt, ##__VA_ARGS__); } while(0)
#else
  #define M2PP2_LOG(fmt, ...)  do {} while(0)
#endif

#define MOVEMENT_SETTLE_TIME 50   // ms
#define COMPLETE_SETTLE_TIME 200  // ms

AccelStepper M2PP2(AccelStepper::HALF4WIRE, PIN1, PIN3, PIN2, PIN4);

struct Mixing_Params mixer = {0};

void M2PP2_Setup() {
    M2PP2.setMaxSpeed(MAX_SPEED);
    M2PP2.setAcceleration(ACCELERATION);
}

bool M2PP2_Init_Mixing(int cycles, int steps) {
    if(cycles <= 0 || steps <= 0) {
        M2PP2_LOG("      M2PP2 Init Error: Invalid parameters!");
        mixer.state = MIXER_STATE_ERROR;
        return false;
    }
    mixer.totalCycles = cycles;
    mixer.currentCycle = 0;
    mixer.mixingSteps = steps;
    mixer.lastStateTime = 0;
    mixer.isRunning = true;
    mixer.state = MIXER_STATE_INIT;
    return true;
}

void M2PP2_Reset() {
    M2PP2.disableOutputs();
    digitalWrite(M2PP2_ENABLE_PIN, HIGH);
    mixer.state = MIXER_STATE_IDLE;
    mixer.isRunning = false;
    mixer.currentCycle = 0;
    mixer.lastStateTime = 0;
}

bool M2PP2_Start(int cycles, int steps) {
    if (mixer.isRunning) {
        M2PP2_LOG("      Cannot start: Mixing already in progress!");
        return false;
    }
    return M2PP2_Init_Mixing(cycles, steps);
}

MixingStatus M2PP2_Sample_Lysis_Mixing_NonBlocking() {
    unsigned long currentTime = millis();
    bool isActive = false;

    switch (mixer.state) {
        case MIXER_STATE_IDLE:
            mixer.isRunning = false;
            return MIXING_COMPLETE;

        case MIXER_STATE_INIT:
            digitalWrite(M2PP2_ENABLE_PIN, LOW); // enable driver before starting to mix 
            M2PP2_LOG("     MIXER started...");
            M2PP2.setMaxSpeed(MAX_SPEED);
            M2PP2.setSpeed(MAX_SPEED);
            mixer.state = MIXER_STATE_FORWARD;
            isActive = true;
            break;

        case MIXER_STATE_FORWARD:
            if (!M2PP2.isRunning()) {
                M2PP2.moveTo(mixer.mixingSteps);
                mixer.state = MIXER_STATE_FORWARD_WAIT;
            }
            isActive = true;
            break;

        case MIXER_STATE_FORWARD_WAIT:
            if (!M2PP2.isRunning()) {
                M2PP2.setCurrentPosition(0);
                mixer.lastStateTime = currentTime;
                mixer.state = MIXER_STATE_BACKWARD;
            }
            isActive = true;
            break;

        case MIXER_STATE_BACKWARD:
            if (currentTime - mixer.lastStateTime >= MOVEMENT_SETTLE_TIME) {
                if (!M2PP2.isRunning()) {
                    M2PP2.moveTo(-mixer.mixingSteps);
                    mixer.state = MIXER_STATE_BACKWARD_WAIT;
                }
            }
            isActive = true;
            break;

        case MIXER_STATE_BACKWARD_WAIT:
            if (!M2PP2.isRunning()) {
                M2PP2.setCurrentPosition(0);
                mixer.currentCycle++;

                if (mixer.currentCycle > mixer.totalCycles) {
                    mixer.lastStateTime = currentTime;
                    mixer.state = MIXER_STATE_COMPLETE;
                } else {
                    mixer.lastStateTime = currentTime;
                    mixer.state = MIXER_STATE_FORWARD;
                }
            }
            isActive = true;
            break;

        case MIXER_STATE_COMPLETE:
            if (currentTime - mixer.lastStateTime >= COMPLETE_SETTLE_TIME) {
                M2PP2.disableOutputs();
                digitalWrite(M2PP2_ENABLE_PIN, HIGH); // Disable stepper motor when complete
                M2PP2_LOG("     State Completed.");
                mixer.state = MIXER_STATE_IDLE;
            }
            isActive = true;
            break;

        case MIXER_STATE_ERROR:
            M2PP2.disableOutputs();
            digitalWrite(M2PP2_ENABLE_PIN, HIGH); // Disabling the stteper motor driver
            M2PP2_LOG("     Mixing Error occurred!");
            mixer.state = MIXER_STATE_IDLE;
            break;
    }

    if (isActive) {
        M2PP2.run();
    }
    
    if (mixer.state == MIXER_STATE_ERROR) {
        return MIXING_ERROR;
    }
    return MIXING_IN_PROGRESS;
}