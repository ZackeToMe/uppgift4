#include <Servo.h>
#include <EEPROM.h>

#define GREEN_LED 11
#define RED_LED 10

#define BTN_1 5
#define BTN_2 4
#define BTN_3 3
#define BTN_4 6
#define BTN_5 2

Servo myservo;

int passcode[] ={4, 2, 1, 4};
int userInput[] = {0, 0, 0, 0};

// System State
bool isLocked = true;
bool wasLocked = false;

const static bool PRESSED = LOW;
const static bool RELEASED = HIGH;

// Debounce variables
unsigned long debounceDelay = 30;
uint16_t buttonCounter1 = 0;
uint16_t buttonCounter2 = 0;
uint16_t buttonCounter3 = 0;
uint16_t buttonCounter4 = 0;
uint16_t buttonCounter5 = 0;
bool buttonState1 = true;
bool buttonState2 = true;
bool buttonState3 = true;
bool buttonState4 = true;
bool buttonState5 = true;
volatile int buttonStateChanger = HIGH;

bool buttonChanged1 = false;
bool buttonChanged2 = false;
bool buttonChanged3 = false;
bool buttonChanged4 = false;
bool buttonChanged5 = false;

const int ARRAY_SIZE = 4;
const int STARTING_EEPROM_ADDRESS = 17;

int passcodeCount = 0;

void setup()
{
  //Init servo
  myservo.attach(9);
  // Init lights
  pinMode(GREEN_LED, OUTPUT);
  pinMode(RED_LED, OUTPUT);

  // Calling function to assign code variable from EEPROM memory
  passFromEEPROM();
  
  // Init buttons
  pinMode(BTN_1, INPUT_PULLUP);
  pinMode(BTN_2, INPUT_PULLUP);
  pinMode(BTN_3, INPUT_PULLUP);
  pinMode(BTN_4, INPUT_PULLUP);
  pinMode(BTN_5, INPUT_PULLUP);
  
  digitalWrite(RED_LED, isLocked);
  Serial.begin(9600);

  // Catching interrupt on BTN_5
  attachInterrupt(digitalPinToInterrupt(BTN_5), ISR_button, RISING);
}

// Catching interrupt on button 5 and assigning the opposite value.
void ISR_button() 
{
  buttonStateChanger = !buttonStateChanger;
}

void userCode(int x){
    userInput[0] = userInput[1];
    userInput[1] = userInput[2];
    userInput[2] = userInput[3];
    userInput[3] = x;
}

bool readButton(int buttonVar, uint16_t &buttonCounter, bool &buttonState, bool &buttonChanged)
{
  if (buttonCounter > millis())
  {
    
  }
  else if (digitalRead(buttonVar) != buttonState)
  {
    buttonCounter = millis() + debounceDelay;
    buttonState = !buttonState;
    buttonChanged = true;
  }
  return buttonState;
}

bool hasChanged(bool &buttonChanged)
{
  if (buttonChanged)
  {
    buttonChanged = false;
    return true;
  }
  return false;
}

bool buttonReleased(int buttonVar, uint16_t &buttonCounter, bool &buttonState, bool &buttonChanged)
{
  return (readButton(buttonVar, buttonCounter, buttonState, buttonChanged) == RELEASED && hasChanged(buttonChanged));
}

void buttonFeederFunction()
{
  if(buttonReleased(BTN_1, buttonCounter1, buttonState1, buttonChanged1))
  {
    userCode(1);
    Serial.println("1");
        if(buttonChanged5 == true)
    {
      passcode[passcodeCount] = userInput[3];
      passcodeCount++;
    }
  }
  else if(buttonReleased(BTN_2, buttonCounter2, buttonState2, buttonChanged2))
  {
    userCode(2);
    Serial.println("2");
        if(buttonChanged5 == true)
    {
      passcode[passcodeCount] = userInput[3];
      passcodeCount++;
    }
  }
  else if(buttonReleased(BTN_3, buttonCounter3, buttonState3, buttonChanged3))
  {
    userCode(3);
    Serial.println("3");
    if(buttonChanged5 == true)
    {
      passcode[passcodeCount] = userInput[3];
      passcodeCount++;
    }
  }
  else if(buttonReleased(BTN_4, buttonCounter4, buttonState4, buttonChanged4))
  {
    userCode(4);
    Serial.println("4");
    if(buttonChanged5 == true)
    {
      passcode[passcodeCount] = userInput[3];
      passcodeCount++;
    }

  }
  else if(buttonReleased(BTN_5, buttonCounter5, buttonState5, buttonChanged5))
  {
    if(buttonStateChanger == LOW)
    {
      Serial.println("Code change initiated please type a new 4 digit code.");
    }
  }
}

void writeIntArrayIntoEEPROM(int address, int numbers[], int arraySize)
{
  int addressIndex = address;
  for (int i = 0; i < arraySize; i++) 
  {
    EEPROM.write(addressIndex, numbers[i] >> 8);
    EEPROM.write(addressIndex + 1, numbers[i] & 0xFF);
    addressIndex += 2;
  }
}

void readIntArrayFromEEPROM(int address, int numbers[], int arraySize)
{
  int addressIndex = address;
  for (int i = 0; i < arraySize; i++)
  {
    numbers[i] = (EEPROM.read(addressIndex) << 8) + EEPROM.read(addressIndex + 1);
    addressIndex += 2;
  }
}

// Function overwriting the previous passcode with the new user input.
void passFromEEPROM(){
      int newPass[ARRAY_SIZE];
      readIntArrayFromEEPROM(STARTING_EEPROM_ADDRESS, newPass, ARRAY_SIZE);
      for (int i = 0; i < ARRAY_SIZE; i++)
      {
        passcode[i] = newPass[i];
      }
}


void loop()
{
  myservo.write(0);
  bool code = false;
  while (code == false) {
    // Checking if buttonStateChanger has been changed  indicating an interruption.
    if (buttonStateChanger == HIGH) {
      // Setting buttonChanged5 to true until 4 digits have been input by user.
      buttonChanged5 = true;
    }
    
    // Checking if new code has been input by user. 
    if(passcodeCount == 4){
      Serial.println("Code changed");
      writeIntArrayIntoEEPROM(STARTING_EEPROM_ADDRESS, passcode, ARRAY_SIZE);
      buttonState5 = false;
      passcodeCount = 0;
    }
    buttonFeederFunction();

    // For loop checking if input matches the lock code.
    int count = 0;
    for (int i = 0; i < 4; i++) {
      if (passcode[i] == userInput[i]) {
        count++;
        if (count == 4) {
          code = true;
        }
      }
    }
  }

  digitalWrite(RED_LED, !isLocked);
  digitalWrite(GREEN_LED, isLocked);
  myservo.write(180);
  Serial.println();
  Serial.print("Locking in 5");
  delay(1000);
  Serial.print("..4");
  delay(1000);
  Serial.print("..3");
  delay(1000);
  Serial.print("..2");
  delay(1000);
  Serial.println("..1");
  delay(1000);
  Serial.println("Locked!");
  digitalWrite(RED_LED, isLocked);
  digitalWrite(GREEN_LED, !isLocked);

  //Reset code so the lock doesn't loop its locking functionality
  userInput[0] = 0;
  userInput[1] = 0;
  userInput[2] = 0;
  userInput[3] = 0;
}