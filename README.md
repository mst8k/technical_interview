# technical_interview

Please look at the the 2 files and the context and comments that show the task. Create necessary files as answers and commit and push a branch.

## Technical Challenge Solutions

# M2PP2 Sample Lysis Mixing

Non-blocking mixing for a stepper motor using AccelStepper library. 
- Simple state machine for forward/backward mixing cycles.
- Error handling for invalid inputs.
- Toggleable logging with `M2PP2_LOG_ENABLED`.

## Usage
Call `M2PP2_Setup()` in `setup()`, `M2PP2_Start(cycles, steps)` to start, and `M2PP2_Sample_Lysis_Mixing_NonBlocking()` in `loop()`. Settle times (50 ms, 200 ms) need hardware tuning. Add quiet window check to `M2PP2_LOG` if relays used.


## M2A1_Pres_Homing
Three non-blocking solutions to mitigate relay noise interference with serial communication:
- **Option 1**: Simple quiet window check, skips messages during noise.
- **Option 2**: Queues messages for later sending, prevents loss.
- **Option 3**: Full non-blocking homing with queued messages, distinguishes critical data.
Define OPTION1, OPTION2, or OPTION3 to test. Noise settle times (20–150 ms) should be tuned based on hardware.