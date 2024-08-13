// works for 3 motors. input the degrees 
#include <Arduino.h>
#include <RMCS2303drive.h>
#include <SoftwareSerial.h>

RMCS2303 rmcs;  // Object for class RMCS2303

// Parameter Settings
byte slave_id1 = 6;  // Choose the slave id of connected drive.

int INP_CONTROL_MODE = 513;  // Refer datasheet and set value according to application
int PP_gain = 32;
int PI_gain = 16;
int VF_gain = 32;
int LPR = 13;
int acceleration = 1000;
int speed = 5000;

const long COUNTS_PER_ROTATION1 = 9048;
const float COUNTS_PER_DEGREE1 = (float)COUNTS_PER_ROTATION1 / 360.0;
const float DEGREES_PER_COUNT1 = 360.0 / (float)COUNTS_PER_ROTATION1;

int gear_ratio = 1;

// Function declarations
void getDegreesFromUser(int inputs[3]);
long int* getcurrentPosition();
void STOP_motors();
void moveMotor(int degrees[3]);

void setup()
{
  rmcs.Serial_selection(0);
  rmcs.Serial0(9600);  // Set baudrate for USB serial to monitor data on serial monitor
  Serial.println("RMCS-2303 Position control mode demo\r\n\r\n");

  rmcs.begin(&Serial1,9600); // serial communication with the hardware serial port
  rmcs.WRITE_PARAMETER(slave_id1, INP_CONTROL_MODE, PP_gain, PI_gain, VF_gain, LPR, acceleration, speed);  // Uncomment to write parameters to drive. Comment to ignore.
  rmcs.READ_PARAMETER(slave_id1);
  rmcs.Restart(slave_id1);

  delay(1000);
}

void loop()
{
  int inputs[3];
  long int* current_pos = getcurrentPosition();
  while(true)
  {
  getDegreesFromUser(inputs);
  moveMotor(inputs);
  }

}

void getDegreesFromUser(int inputs[3]) {
  Serial.println("Please enter three integers:");

  for (int i = 0; i < 3; i++) 
  {
    Serial.print("Enter ");
    Serial.print(i + 1);
    Serial.print(" input: ");
    while (Serial.available() == 0) 
    {
      // Wait for input
    }
    inputs[i] = Serial.parseInt();
    Serial.print("Degree ");
    Serial.print(i + 1);
    Serial.print(" is: ");
    Serial.println(inputs[i]);
  }
}

long int* getcurrentPosition() {
  static long int positions[3];

  positions[0] = rmcs.Position_Feedback(slave_id1);
  Serial.print("The current position is: ");
  Serial.println(positions[0]);


  return positions;
}

void STOP_motors()
{
  rmcs.STOP(slave_id1);

}

void moveMotor(int degrees[3])
{
  long int* current_pos = getcurrentPosition(); // finding the current positions of all 3 motors

  // Updating the target position based on degrees inputed
  long targetPosition1 = current_pos[0] + (long)(degrees[0] * COUNTS_PER_DEGREE1 * gear_ratio);

  Serial.print("Motor 1 Moving to position: ");
  Serial.println(targetPosition1*DEGREES_PER_COUNT1);

  rmcs.Absolute_position(slave_id1, targetPosition1); 


  while (true) 
  {  // Keep reading positions. Exit when reached.
    long positionFeedback1 = rmcs.Position_Feedback(slave_id1);  // Read current encoder position 
    long speedFeedback1 = rmcs.Speed_Feedback(slave_id1);  // Read current speed

    Serial.print("Position Feedback 1 : ");
    Serial.print(positionFeedback1);
    Serial.print("\tSpeed Feedback 1 : ");
    Serial.println(speedFeedback1);


    if ((abs(positionFeedback1 - targetPosition1) < 100))   // Adjust the tolerance as needed
    {  
      long positionFeedback1 = rmcs.Position_Feedback(slave_id1);  // Read current encoder position 
      long speedFeedback1 = rmcs.Speed_Feedback(slave_id1);
      Serial.print("Position Feedback where motor stopped : ");
      Serial.print(positionFeedback1);
      Serial.print("\tSpeed Feedback when motor stopped : ");
      Serial.println(speedFeedback1);
      Serial.println("Positions reached.");
      STOP_motors();
      break;
    }
    delay(100);
  }
}