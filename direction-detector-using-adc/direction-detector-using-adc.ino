/*
 Direction Detector using ADC
 
 Due to the use of ADC as direction sensor (slow measurements),
 this can't detect very fast moving objects.
 
 Created 12 April 2018
 @Gorontalo, Indonesia
 by ZulNs
 */


#define SENSOR_A_PIN        0
#define SENSOR_B_PIN        1
#define SENSOR_THRESHOLD    700
#define SENSOR_STATE_NORMAL 0
#define SENSOR_STATE_A      1
#define SENSOR_STATE_B      2
#define SENSOR_STATE_AB     3

boolean isMovingIn  = false;
boolean isMovingOut = false;
boolean isInvalid   = false;
byte oldSensorState = SENSOR_STATE_NORMAL;
byte personCounter  = 0;

void setup()
{
  Serial.begin(115200);
  Serial.println(F("*** Direction Detector using ADC ***"));
  Serial.println();
  analogRead(0);
}

void loop()
{
  byte curSensorState = getSensorState();
  byte movingStep;
  byte ctr = personCounter;
  
  if (curSensorState == oldSensorState)
  {
    return;
  }
  
  if (curSensorState == SENSOR_STATE_NORMAL)
  {
    if (isMovingIn && oldSensorState == SENSOR_STATE_B)
    {
      personCounter++;
    }
    else if (isMovingOut && oldSensorState == SENSOR_STATE_A && personCounter > 0)
    {
      personCounter--;
    }
    isMovingIn = false;
    isMovingOut = false;
    isInvalid = false;
  }
  
  else if (oldSensorState == SENSOR_STATE_NORMAL)
  {
    if (curSensorState == SENSOR_STATE_A)
    {
      isMovingIn = true;
    }
    else if (curSensorState == SENSOR_STATE_B)
    {
      isMovingOut = true;
    }
    else
    {
      isInvalid = true;
    }
  }
  
  else if (!isInvalid && (oldSensorState ^ 3) == curSensorState)
  {
    isInvalid = true;
    isMovingIn = false;
    isMovingOut = false;
  }
  
  oldSensorState = curSensorState;
  
  if (isMovingIn)
  {
    if (curSensorState == SENSOR_STATE_A)
    {
      movingStep = 1;
    }
    else if (curSensorState == SENSOR_STATE_AB)
    {
      movingStep = 2;
    }
    else if (curSensorState == SENSOR_STATE_B)
    {
      movingStep = 3;
    }
    Serial.print(F("Moving in state: "));
    Serial.println(movingStep);
  }
  
  else if (isMovingOut)
  {
    if (curSensorState == SENSOR_STATE_B)
    {
      movingStep = 1;
    }
    else if (curSensorState == SENSOR_STATE_AB)
    {
      movingStep = 2;
    }
    else if (curSensorState == SENSOR_STATE_A)
    {
      movingStep = 3;
    }
    Serial.print(F("Moving out state: "));
    Serial.println(movingStep);
  }
  
  else if (isInvalid)
  {
    Serial.print(F("Invalid state: "));
    if (curSensorState == SENSOR_STATE_A)
    {
      Serial.println(F("Ax"));
    }
    else if (curSensorState == SENSOR_STATE_B)
    {
      Serial.println(F("xB"));
    }
    else if (curSensorState == SENSOR_STATE_AB)
    {
      Serial.println(F("AB"));
    }
  }
  
  else
  {
    Serial.println(F("Sensor state normal..."));
    if (ctr != personCounter)
    {
      Serial.print(F("Person counts: "));
      Serial.println(personCounter);
    }
    Serial.println();
  }
}

byte getSensorState()
{
  byte res = 0;
  int saAdc = analogRead(SENSOR_A_PIN);
  int sbAdc = analogRead(SENSOR_B_PIN);
  bitWrite(res, 0, (saAdc < SENSOR_THRESHOLD));
  bitWrite(res, 1, (sbAdc < SENSOR_THRESHOLD));
  return res;
}

