/**
 * Author: Kai Wong
 * CMPT422 Instructor: Brian Gormanly
 * Final Project Part 1 Fall 2017
 * Binary addition calculator
 */
#include <LiquidCrystal.h>
const int dseven = 13; // seven display pin
const int dsix = 12; // six display pin
const int dfive = 10; // five display pin
const int dfour = 9; // four display pin
const int vzero = 6; // vzero pin
const int rs = 7; // register select pin
const int en = 4; // enabler pin
const int leftbut = 2; // interrupt pin
const int rightbut = 3; // interrupt pin 
int contrast = 20; // the contrast for the LCD
volatile int leftButtonState = 0; // used to keep track of the left button state
volatile int rightButtonState = 0; // used to keep track of the right button state
volatile int byteOne = 0; // stores the value for byte one
volatile int byteTwo = 0; // stores the value for byte two
boolean bothPressed = false; // used to keep track if both buttons are pressed
boolean leftPressed = false; // used to keep track if the left button is pressed down
boolean leftReleased = false; // used to keep track if the left button has been released
boolean rightPressed = false; // used to keep track if the right button is pressed down
boolean rightReleased = false; // used to keep track if the right button has been released
boolean rightBothReleased = false; // used to keep track if both buttons were pressed and the right button has been released
boolean leftBothReleased = false; // used to keep track if both buttons were pressed and the left button has been released

LiquidCrystal lcd(rs, en, dfour, dfive, dsix, dseven);
void setup() {
  Serial.begin(9600);
  // Set the contrast
  pinMode(vzero, OUTPUT);
  analogWrite(vzero, contrast);
  lcd.begin(16, 2);
  // Setup buttons
  pinMode(leftbut,INPUT);
  pinMode(rightbut,INPUT);
  // Attach interrupts to the ISR vector
  attachInterrupt(0, pin_leftbut_ISR, CHANGE); // issue interrrupt on state change
  attachInterrupt(1, pin_rightbut_ISR, CHANGE); // issue interrupt on state change
}

void loop() {
  // Break from each loop to advance to the next loop
  splash();
  byteOneInput();
  byteTwoInput();
  displaySolution();
  adjustContrast();
}

// -------------------------------------------------------
// HELPER METHODS
// -------------------------------------------------------

// Interrupt for left button press on button state rising
void pin_leftbut_ISR() {
  Serial.print("Left button interrupt");
  leftButtonState = digitalRead(leftbut);
  if(leftButtonState == HIGH){
    leftPressed = true;
  }
  // Register change from pressed to not pressed
  if(leftButtonState == LOW && leftPressed){
    if(rightPressed && !rightBothReleased){
      // The right button is still being pressed, so we're in a both button pressed situation
      // Must not register any button presses until all buttons are released!
      leftBothReleased = true;
    }
    else if(rightBothReleased){
      // The right button was also released, so we can set bothPressed = true
      bothPressed = true;
    }
    else{
      leftReleased = true;
    }
    leftPressed = false;
  }
}
// Interrupt for right button press on button state rising
void pin_rightbut_ISR() {
  Serial.print("Right button interrupt");
  rightButtonState = digitalRead(rightbut);
  if(rightButtonState == HIGH){
    rightPressed = true;
  }
  // Register change from pressed to not pressed
  if(rightButtonState == LOW && rightPressed){
    if(leftPressed && !leftBothReleased){
      // The left button is still being pressed, so we're in a both button pressed situation
      // Must not register any button presses until all buttons are released!
      rightBothReleased = true;
    }
    else if(leftBothReleased){
      // The left button was also released, so we can set bothPressed = true
      bothPressed = true;
    }
    else{
      rightReleased = true;
    }
    rightPressed = false;
  }
}

// Converts a char[] byte string to a decimal number
int binToDec(char byteString[]) {
  int dec = 0;
  int weights[8] = {128, 64, 32, 16, 8, 4, 2, 1};
  // For each place in array, multiply it by the weight
  for(int i=0; i<8; i++){
    if(byteString[i] == '1'){
      dec += weights[i];
    }
  }
  return dec;
}

// Converts a decimal number to a passed char[] byte string
// by dividing decimal number by binary place weight values
void decToBin(int dec, char byteString[]) {
  int ptr = 0;
  // Check for overflow
  if(dec / 256 == 1){
    byteString[ptr] = '1';
    dec -= 256;
  }
  ptr++;
  if(dec / 128 == 1){
    byteString[ptr] = '1';
    dec -= 128;
  }
  ptr++;
  if(dec / 64 == 1){
    byteString[ptr] = '1';
    dec -= 64;
  }
  ptr++;
  if(dec / 32 == 1){
    byteString[ptr] = '1';
    dec -= 32;
  }
  ptr++;
  if(dec / 16 == 1){
    byteString[ptr] = '1';
    dec -= 16;
  }
  ptr++;
  if(dec / 8 == 1){
    byteString[ptr] = '1';
    dec -= 8;
  }
  ptr++;
  if(dec / 4 == 1){
    byteString[ptr] = '1';
    dec -= 4;
  }
  ptr++;
  if(dec / 2 == 1){
    byteString[ptr] = '1';
    dec -= 2;
  }
  ptr++;
  if(dec / 1 == 1){
    byteString[ptr] = '1';
    dec -= 1;
  }
}

// Method dedicated to resetting the booleans used for button press detection
// Resets buttons to their 'default' state typically after a button press is registered
// Implemented for cleaner-looking code, even if it might introduce some redundancies
void resetFlags() {
  leftPressed = false;
  rightPressed = false;
  leftReleased = false;
  rightReleased = false;
  bothPressed = false;
  leftBothReleased = false;
  rightBothReleased = false;
}

// Method to display first screen (splash screen)
void splash() {
  lcd.clear();
  lcd.print("Hello! I'm a");
  lcd.setCursor(0,1);
  lcd.print("calculator. :D");
  while (true) {
    delay(100); // This delay is oddly needed to make this work
    // Detect button presses, if both buttons pressed at same time, break to go to next screen
    if(bothPressed){
      resetFlags();
      break;
    }
  }
}

// Method to display second screen (first byte input)
void byteOneInput() {
  lcd.clear();
  lcd.print("Byte one input:");
  lcd.setCursor(0,1);
  char byteDisplay[9] = "00000000"; // This will be the displayed byte
  int ctr = 0; // Pointer to current bit to flip
  lcd.print(byteDisplay);
  lcd.setCursor(0,1);
  lcd.blink(); // Provide a blinking cursor for the current bit being edited
  while (true) {
    delay(100); // This delay is oddly needed to make this work
    // Left button press flips bit value
    // Right button moves to the next bit until reaching the lowest order bit, which will break to go to the next screen
    // Have flashing cursor
    if(leftReleased) { 
      resetFlags();
      if(byteDisplay[ctr] == '0'){
        byteDisplay[ctr] = '1';
      }
      else{
        byteDisplay[ctr] = '0';
      }
      // Reprint the byte
      lcd.setCursor(0,1);
      lcd.print(byteDisplay);
      lcd.setCursor(ctr,1);
    }
    if(rightReleased) {
      resetFlags();
      // We've reached the end of the byte, so proceed to next screen
      if(ctr == 7){
        // Convert the char array representing the byte to a decimal number
        byteOne = binToDec(byteDisplay);
        break;
      }
      ctr++;
      // Set the blinking cursor to the appropriate position
      lcd.setCursor(ctr,1);
    }
    // Pressing both buttons does nothing
    if(bothPressed){
      resetFlags();
    }
  }
}

// Method to display third screen (second byte input)
void byteTwoInput() {
  lcd.clear();
  lcd.print("Byte two input:");
  lcd.setCursor(0,1);
  char byteDisplay[9] = "00000000"; // This will be the displayed byte
  int ctr = 0; // Pointer to current bit to flip
  lcd.print(byteDisplay);
  lcd.setCursor(0,1);
  lcd.blink(); // Provide a blinking cursor for the current bit being edited
  while (true) {
    delay(100); // This delay is oddly needed to make this work
    // Left button press flips bit value
    // Right button moves to the next bit until reaching the lowest order bit, which will break to go to the next screen
    if(leftReleased) { 
      resetFlags();
      if(byteDisplay[ctr] == '0'){
        byteDisplay[ctr] = '1';
      }
      else{
        byteDisplay[ctr] = '0';
      }
      // Reprint the byte
      lcd.setCursor(0,1);
      lcd.print(byteDisplay);
      lcd.setCursor(ctr,1);
    }
    if(rightReleased) {
      resetFlags();
      // We've reached the end of the byte, so proceed to next screen
      if(ctr == 7){
        // Convert the char array representing the byte to a decimal number
        byteTwo = binToDec(byteDisplay);
        break;
      }
      ctr++;
      // Set the blinking cursor to the appropriate position
      lcd.setCursor(ctr,1);
    }
    // Pressing both buttons does nothing
    if(bothPressed){
      resetFlags();
    }
  }
}

// Method to display fourth screen (solution screen)
void displaySolution() {
  lcd.noBlink(); // removes cursor
  lcd.clear();
  // Perform addition of the two integer values of the byte values
  int res = byteOne + byteTwo;
  // Convert the result into a char array representation and print it
  char byteResult[9] = "000000000";
  decToBin(res, byteResult); // modifies the arr variable
  lcd.setCursor(0,0);
  lcd.print("Solution:");
  lcd.setCursor(0,1);
  lcd.print(byteResult);
  while (true) {
    delay(100); // This delay is oddly needed to make this work
    if(bothPressed){
      resetFlags();
      break;
    }
  }
}

// Method to display fifth screen (contrast adjustment screen)
void adjustContrast() {
  printContrast();
  while (true) {
    delay(100); // This delay is oddly needed to make this work
    // If both buttons pressed at same time, break to go back to beginning
    if(bothPressed){
      resetFlags();
      break;
    }
    // Pressing the left button will decrease the contrast
    if(leftReleased){
      resetFlags();
      if(contrast > 0){
        contrast -= 5;
      }
      analogWrite(vzero, contrast);
      printContrast();
    }
    // Pressing the right button will increase the contrast
    else if(rightReleased){
      resetFlags();
      if(contrast < 255){
        contrast += 5;
      }
      analogWrite(vzero, contrast);
      printContrast();
    }
  }
}

void printContrast() {
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Contrast: ");
  lcd.print(contrast);
  lcd.setCursor(0,1);
  lcd.print("Adj. w/ btns");
}