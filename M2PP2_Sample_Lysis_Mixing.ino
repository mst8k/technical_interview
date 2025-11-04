void M2PP2_Sample_Lysis_Mixing()  // To shake Sample + Lysis  
{ 
  N_Mixings = Par1; 
  M2PP2_Mixing_Steps = Par2; 
 
  Serial.println("     Moving Sample from M2 MIXER into the silica started..."); 
  digitalWrite(M2PP2_ENABLE_PIN, LOW);    // LOW = Enabled 
  M2PP2.setAcceleration(8400); 
  M2PP2.setSpeed(8400); 
  M2PP2.setMaxSpeed(8400); 
 
  for (int i = 0 ; i <= N_Mixings; i++) {     
      M2PP2.runToNewPosition(M2PP2_Mixing_Steps);  // Forward 
      M2PP2.run();  
      M2PP2.setCurrentPosition(0); 
      delay(5); 
      M2PP2.runToNewPosition(-M2PP2_Mixing_Steps);  // Backward 
      M2PP2.run();  
      M2PP2.setCurrentPosition(0); 
      delay(5); 
  }   
 
  delay(200); 
  digitalWrite(M2PP2_ENABLE_PIN, HIGH); 
  delay(1000); 
 
  Serial.println("     Moving Sample from M2 MIXER into the silica Completed."); 
  Serial.println(""); 
} 


 
// The function M2PP2_Sample_Lysis_Mixing() shown above performs a mixing operation
// by moving a stepper motor forward and backwards several times. However, the current 
// implementation uses blocking delays and synchronous movement 
// commands that halt the entire system until each motion completes.

// Task 

// Identify the main issues in this implementation that would prevent it from running reliably in a real-time or multitasking environment. 

// Rewrite how you would redesign this function using a non-blocking state machine approach so that the system can remain responsive while the mixing sequence executes. 
 