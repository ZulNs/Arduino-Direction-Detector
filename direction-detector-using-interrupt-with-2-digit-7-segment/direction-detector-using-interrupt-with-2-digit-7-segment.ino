/*
 * Direction Detector using Interrupt
 * 
 * Designed for Amran Syahni Putra Harada's Final Project
 * 
 * Uses built-in analog comparator and
 * one additional analog comparator 1/2 LM393.
 * It can detects very fast moving objects,
 * due to the use of hardware interrupt (very fast response).
 * 
 * Sensor-A           tied to D2  (INT0 on PD2); it sensed by hardware external interrupt
 * Sensor-B           tied to D6/D7 (Non Inverting Input on PD6 / Inverting Input on PD7); it sensed by internal analog comparator interrupt
 * Segment-A          tied to A0  (PC0) via a resistor
 * Segment-B          tied to A1  (PC1) via a resistor
 * Segment-C          tied to A2  (PC2) via a resistor
 * Segment-D          tied to A3  (PC3) via a resistor
 * Segment-E          tied to A4  (PC4) via a resistor
 * Segment-F          tied to A5  (PC5) via a resistor
 * Segment-G          tied to D8  (PB0) via a resistor
 * Common-Digit-1     tied to D9  (PB1)
 * Common-Digit-2     tied to D10 (PB2)
 * Input-Relay-Driver tied to D11 (PB3)
 *
 * Created 19 July 2018
 * @Gorontalo, Indonesia
 * by ZulNs
 */


#
#define SEGMENT_A_PIN      0   // PORTC (A0)
#define SEGMENT_B_PIN      1   // PORTC (A1)
#define SEGMENT_C_PIN      2   // PORTC (A2)
#define SEGMENT_D_PIN      3   // PORTC (A3)
#define SEGMENT_E_PIN      4   // PORTC (A4)
#define SEGMENT_F_PIN      5   // PORTC (A5)
#define SEGMENT_G_PIN      0   // PORTB (D8)
#define COMMON_DIGIT_1_PIN 1   // PORTB (D9)
#define COMMON_DIGIT_2_PIN 2   // PORTB (D10)
#define RELAY_PIN          3   // PORTB (D11)

#define SENSOR_A_BIT       0
#define SENSOR_B_BIT       1
#define SENSOR_THRESHOLD   700
#define STATE_NO_SENSE     0
#define STATE_SENSE_A      1
#define STATE_SENSE_B      2
#define BLANK              10
#define CATHODE            LOW
#define ANODE              HIGH
#define COMMON_DIGIT_MODE  ANODE
#define NUMBER_OF_DIGIT    2
#define REFRESH_RATE       60  // in Hertz

#if COMMON_DIGIT_MODE == CATHODE
const uint8_t DIGIT[] =
{
  0x3F,  // digit 0
  0x06,  // digit 1
  0x5B,  // digit 2
  0x4F,  // digit 3
  0x66,  // digit 4
  0x6D,  // digit 5
  0x7D,  // digit 6
  0x07,  // digit 7
  0x7F,  // digit 8
  0x6F,  // digit 9
  0x00   // BLANK
};
#elif COMMON_DIGIT_MODE == ANODE
const uint8_t DIGIT[] =
{
  0xC0,  // digit 0
  0xF9,  // digit 1
  0xA4,  // digit 2
  0xB0,  // digit 3
  0x99,  // digit 4
  0x92,  // digit 5
  0x82,  // digit 6
  0xF8,  // digit 7
  0x80,  // digit 8
  0x90,  // digit 9
  0xFF   // BLANK
};
#endif

const uint32_t REFRESH_INTERVAL = 1000 / REFRESH_RATE / NUMBER_OF_DIGIT;  // in millisecond

uint8_t  sensorState;
uint8_t  oldSensorState;
boolean  isIncoming;
boolean  isLeaving;
uint32_t refreshStartTime;
uint8_t  digitCounter;
uint8_t  incomingCounter;
volatile uint8_t displayBuffer[NUMBER_OF_DIGIT];

ISR(INT0_vect)
{
  bitWrite(sensorState, SENSOR_A_BIT, bitRead(PIND, 2));
  determineDirection();
}

ISR(ANALOG_COMP_vect)
{
  bitWrite(sensorState, SENSOR_B_BIT, bitRead(ACSR, ACO));
  bitWrite(PORTB, 5, !bitRead(ACSR, ACO));
  determineDirection();
}

void determineDirection()
{
  if (sensorState == STATE_NO_SENSE)
  {
    if (isIncoming && oldSensorState == STATE_SENSE_B)
    {
      incomingCounter++;
      if (incomingCounter == 1)
      {
        bitSet(PORTB, RELAY_PIN);
      }
      updateDisplayBuffer();
    }
    else if (isLeaving && oldSensorState == STATE_SENSE_A)
    {
      incomingCounter--;
      if (incomingCounter == 0)
      {
        bitClear(PORTB, RELAY_PIN);
      }
      updateDisplayBuffer();
    }
    isIncoming = false;
    isLeaving = false;
  }
  
  else if (oldSensorState == STATE_NO_SENSE)
  {
    if (sensorState == STATE_SENSE_A)
    {
      isIncoming = true;
    }
    else if (sensorState == STATE_SENSE_B)
    {
      isLeaving = true;
    }
  }
  
  oldSensorState = sensorState;
}

uint8_t binToBcd()
{
  uint8_t nibble1 = (incomingCounter / 10) % 10;
  uint8_t nibble0 = incomingCounter % 10;
  if (incomingCounter < 10)
  {
    nibble1 = BLANK;
  }
  return (nibble1 << 4) + nibble0;
}

void updateDisplayBuffer()
{
  uint8_t bcd = binToBcd();
  displayBuffer[0] = DIGIT[bcd & 15];
  displayBuffer[1] = DIGIT[bcd >> 4];
}

void refreshDisplay()
{
  PORTC &= 0xC0;
  PORTC |= displayBuffer[digitCounter] & 0x3F;
  bitWrite(PORTB, SEGMENT_G_PIN, bitRead(displayBuffer[digitCounter], 6));
  if (digitCounter == 0)
  {
    bitWrite(PORTB, COMMON_DIGIT_1_PIN, COMMON_DIGIT_MODE);
    bitWrite(PORTB, COMMON_DIGIT_2_PIN, !COMMON_DIGIT_MODE);
  }
  else
  {
    bitWrite(PORTB, COMMON_DIGIT_1_PIN, !COMMON_DIGIT_MODE);
    bitWrite(PORTB, COMMON_DIGIT_2_PIN, COMMON_DIGIT_MODE);
  }
  digitCounter++;
  digitCounter %= NUMBER_OF_DIGIT;
}

void setup()
{
  Serial.begin(115200);
  Serial.println(F("*** Direction Detector using Interrupt ***"));
  Serial.println();
  
  DDRC |= 0x3F; // Pin A0 to A5 as output
  DDRB |= 0x0F; // Pin D8 to D10 as output
  
  bitSet(DDRB, 5);
  bitWrite(PORTB, 5, !bitRead(ACSR, ACO));
  
  cli();                 // disable all interrupts
  bitSet(EICRA, ISC00);  // set INT0 interrupt to any logical change
  bitSet(EIFR, INTF0);   // clear any outstanding INT0 interrupt
  bitSet(EIMSK, INT0);   // enable INT0 interrupt
  ACSR = bit(ACI) | bit(ACIE);
  //bitSet(ACSR, ACI);   // clear any outstanding analog comparator interrupt
  //bitSet(ACSR, ACIE);  // enable analog comparator interrupt
  
  updateDisplayBuffer();
  sei();                 // enable all interrupts
  refreshDisplay();
}

void loop()
{
  if (millis() - refreshStartTime >= REFRESH_INTERVAL)
  {
    refreshStartTime = millis();
    refreshDisplay();
  }
  /*delay(1);
  if (millis() % 1000 == 0)
  {
    incomingCounter++;
    updateDisplayBuffer();
  }*/
}

