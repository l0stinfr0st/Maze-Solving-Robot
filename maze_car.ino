#include <Servo.h>
#include <EEPROM.h>

#define enA 5
#define in1 8
#define in2 9
#define enB 4
#define in3 10
#define in4 11

const int servoPin = 6;
const int trigPin = A0;
const int echoPin = A1;
const int lightSensorPin = A2;
const int buzzerPin = 7;

Servo myServo;

const int lightThreshold = 900;
volatile bool systemHalted = false;
bool isAutonomous = true;

unsigned long lastLightCheck = 0;
const unsigned long lightCheckInterval = 100;

int moveCount = 0;
const int maxMoves = 100;
const int eepromStartAddress = 0;

int leftMotorSpeed = 255;
int rightMotorSpeed = 128;
int moveDelay = 200;
int turnDelay = 150;
int spinDelay = 260;

void setup() {
  Serial.begin(9600);
  myServo.attach(servoPin);

  pinMode(trigPin, OUTPUT);
  pinMode(echoPin, INPUT);
  pinMode(buzzerPin, OUTPUT);
  pinMode(in1, OUTPUT);
  pinMode(in2, OUTPUT);
  pinMode(enA, OUTPUT);
  pinMode(in3, OUTPUT);
  pinMode(in4, OUTPUT);
  pinMode(enB, OUTPUT);

  analogWrite(buzzerPin, 0);
  clearMoveHistory();
  
  lastLightCheck = millis();
  
  Serial.println(F("Ready! M=Manual A=Auto H=Help"));
  Serial.print(F("Light check interval: "));
  Serial.print(lightCheckInterval);
  Serial.println(F("ms"));
}

void printHelp() {
  Serial.println(F("AL###=LMotor AR###=RMotor AB###=Both"));
  Serial.println(F("DM###=MoveTime DT###=TurnTime DS###=SpinTime"));
  Serial.println(F("LI###=LightInterval C=Settings H=Help F/L/R/S=Move"));
}

void showSettings() {
  Serial.print(F("L:"));
  Serial.print(leftMotorSpeed);
  Serial.print(F(" R:"));
  Serial.print(rightMotorSpeed);
  Serial.print(F(" M:"));
  Serial.print(moveDelay);
  Serial.print(F(" T:"));
  Serial.print(turnDelay);
  Serial.print(F(" S:"));
  Serial.print(spinDelay);
  Serial.print(F(" LI:"));
  Serial.println(lightCheckInterval);
}

void handleCalibration(String cmd) {
  int val = cmd.substring(2).toInt();
  
  if (cmd.startsWith("AL") && val <= 255) {
    leftMotorSpeed = val;
    Serial.print(F("L="));
    Serial.println(val);
  } else if (cmd.startsWith("AR") && val <= 255) {
    rightMotorSpeed = val;
    Serial.print(F("R="));
    Serial.println(val);
  } else if (cmd.startsWith("AB") && val <= 255) {
    leftMotorSpeed = rightMotorSpeed = val;
    Serial.print(F("Both="));
    Serial.println(val);
  } else if (cmd.startsWith("DM") && val <= 5000) {
    moveDelay = val;
    Serial.print(F("Move="));
    Serial.println(val);
  } else if (cmd.startsWith("DT") && val <= 5000) {
    turnDelay = val;
    Serial.print(F("Turn="));
    Serial.println(val);
  } else if (cmd.startsWith("DS") && val <= 5000) {
    spinDelay = val;
    Serial.print(F("Spin="));
    Serial.println(val);
  } else if (cmd.startsWith("LI") && val >= 50 && val <= 2000) {
    const_cast<unsigned long&>(lightCheckInterval) = val;
    Serial.print(F("LightInterval="));
    Serial.println(val);
  }
}

long readDistanceCM() {
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  
  long duration = pulseIn(echoPin, HIGH);
  return duration * 0.034 / 2;
}

void checkLightSensorTimed() {
  unsigned long currentTime = millis();
  
  if (currentTime - lastLightCheck >= lightCheckInterval) {
    int lightLevel = analogRead(lightSensorPin);
    
    Serial.print(F("Light: "));
    Serial.println(lightLevel);
    
    if (lightLevel > lightThreshold && !systemHalted) {
      Serial.print(F("Light detected: "));
      Serial.println(lightLevel);
      haltSystem();
    }
    
    lastLightCheck = currentTime;
  }
}

void storeMoveInEEPROM(char move) {
  if (moveCount < maxMoves) {
    EEPROM.write(eepromStartAddress + moveCount, move);
    moveCount++;
  }
}

void clearMoveHistory() {
  moveCount = 0;
  for (int i = 0; i < maxMoves; i++) {
    EEPROM.write(eepromStartAddress + i, 0);
  }
}

void playMoveSequence() {
  Serial.print(F("Playing "));
  Serial.print(moveCount);
  Serial.println(F(" moves"));
  
  for (int i = 0; i < moveCount; i++) {
    char move = EEPROM.read(eepromStartAddress + i);
    int freq;
    
    switch (move) {
      case 'L': freq = 262; break;
      case 'R': freq = 330; break;
      case 'F': freq = 392; break;
      case 'S': freq = 523; break;
      default: freq = 220; break;
    }
    
    tone(buzzerPin, freq);
    delay(500);
    noTone(buzzerPin);
    delay(200);
    
    checkLightSensorTimed();
  }
}

void haltSystem() {
  systemHalted = true;
  stopMotors();
  Serial.println(F("SYSTEM HALTED - Light detected!"));
  delay(1000);
  playMoveSequence();
  
  Serial.println(F("System ready - send 'A' for auto or 'M' for manual"));
  systemHalted = false;
}

void stopMotors() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, LOW);
  analogWrite(enA, 0);
  analogWrite(enB, 0);
}

void moveForward() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(enA, leftMotorSpeed);
  analogWrite(enB, rightMotorSpeed);
  storeMoveInEEPROM('F');
}

void turnLeft() {
  digitalWrite(in1, HIGH);
  digitalWrite(in2, LOW);
  digitalWrite(in3, LOW);
  digitalWrite(in4, HIGH);
  analogWrite(enA, leftMotorSpeed);
  analogWrite(enB, rightMotorSpeed);
  storeMoveInEEPROM('L');
}

void turnRight() {
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(enA, leftMotorSpeed);
  analogWrite(enB, rightMotorSpeed);
  storeMoveInEEPROM('R');
}

void spin180InPlace() {
  storeMoveInEEPROM('S');
  digitalWrite(in1, LOW);
  digitalWrite(in2, HIGH);
  digitalWrite(in3, HIGH);
  digitalWrite(in4, LOW);
  analogWrite(enA, leftMotorSpeed);
  analogWrite(enB, rightMotorSpeed);
  
  unsigned long spinStart = millis();
  while (millis() - spinStart < spinDelay) {
    checkLightSensorTimed();
    delay(10);
  }
  
  stopMotors();
  delay(200);
}

void processCommands() {
  if (Serial.available()) {
    String cmd = Serial.readString();
    cmd.trim();
    
    if (cmd.length() == 1) {
      char c = cmd.charAt(0);
      
      switch (c) {
        case 'M': case 'm':
          isAutonomous = false;
          systemHalted = false;
          Serial.println(F("Manual mode"));
          break;
        case 'A': case 'a':
          isAutonomous = true;
          systemHalted = false;
          Serial.println(F("Autonomous mode"));
          break;
        case 'F': case 'f':
          if (!isAutonomous && !systemHalted) {
            moveForward();
            delay(moveDelay);
            stopMotors();
            delay(200);
          }
          break;
        case 'L': case 'l':
          if (!isAutonomous && !systemHalted) {
            turnLeft();
            delay(turnDelay+10);
            stopMotors();
            delay(200);
          }
          break;
        case 'R': case 'r':
          if (!isAutonomous && !systemHalted) {
            turnRight();
            delay(turnDelay);
            stopMotors();
            delay(200);
          }
          break;
        case 'S': case 's':
          if (!isAutonomous && !systemHalted) {
            spin180InPlace();
          }
          break;
        case 'C': case 'c':
          showSettings();
          break;
        case 'H': case 'h':
          printHelp();
          break;
      }
    } else if (cmd.length() > 2) {
      cmd.toUpperCase();
      handleCalibration(cmd);
    }
  }
}

void loop() {
  checkLightSensorTimed();
  
  processCommands();
  
  if (systemHalted || !isAutonomous) return;

  long distance;
  
  // Check left
  myServo.write(180);
  delay(500);
  distance = readDistanceCM();
  
  if (distance > 20) {
    turnLeft();
    delay(turnDelay+10);
    stopMotors();
    delay(200); 
    moveForward();
    delay(moveDelay+10);
    stopMotors();
    delay(200);
    return;
  }

  // Check front
  myServo.write(90);
  delay(500);
  distance = readDistanceCM();
  
  if (distance > 20) {
    moveForward();
    delay(moveDelay);
    stopMotors();
    delay(200);
    return;
  }

  // Check right
  myServo.write(0);
  delay(500);
  distance = readDistanceCM();
  
  if (distance > 20) {
    turnRight();
    delay(turnDelay);
    stopMotors();
    delay(200); 
    moveForward();
    delay(moveDelay);
    stopMotors();
    delay(200);
    return;
  } else { //All paths blocked
    spin180InPlace();
    delay(200);
    return;
  }

  stopMotors();
}