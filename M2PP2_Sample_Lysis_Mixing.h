#ifndef M2PP2_SAMPLE_LYSIS_MIXING_H
#define M2PP2_SAMPLE_LYSIS_MIXING_H

#include <Arduino.h>
#include <AccelStepper.h>


#define STEPS_PER_REV   4096
#define MAX_SPEED        8400    
#define ACCELERATION     500     

#define PIN1     8   
#define PIN2     9   
#define PIN3     10  
#define PIN4     11  
#define M2PP2_ENABLE_PIN 12

enum MixingStatus {
    MIXING_IN_PROGRESS,
    MIXING_COMPLETE,
    MIXING_ERROR
};

// States
enum MixingState {
    MIXER_STATE_IDLE,
    MIXER_STATE_INIT,
    MIXER_STATE_FORWARD,
    MIXER_STATE_FORWARD_WAIT,
    MIXER_STATE_BACKWARD,
    MIXER_STATE_BACKWARD_WAIT,
    MIXER_STATE_COMPLETE,
    MIXER_STATE_ERROR
};

// Mixing parameters structure
struct Mixing_Params {
    int totalCycles;
    int currentCycle;
    int mixingSteps;
    unsigned long lastStateTime;
    bool isRunning;
    MixingState state;
};

// Function declarations
bool M2PP2_Init_Mixing(int cycles, int steps);
MixingStatus M2PP2_Sample_Lysis_Mixing_NonBlocking();
void M2PP2_Setup();
void M2PP2_Reset();
bool M2PP2_Start(int cycles, int steps);

// External declarations
extern AccelStepper M2PP2;
extern struct Mixing_Params mixer;

#endif // M2PP2_SAMPLE_LYSIS_MIXING_H