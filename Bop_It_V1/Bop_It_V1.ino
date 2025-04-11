#include <DFRobotDFPlayerMini.h>

// Pins assigned to our components
#define PRESSURE_SENSOR 14       // Analog Input
#define MIC 15                  // Analog Equivalent is 1
#define ROTARY_SENSOR_CLK 16
#define START_BUTTON 4

// HEX PINS
#define RED_LED 18
#define GREEN_LED 19
#define TENS_A 9
#define TENS_B 10
#define TENS_C 2
#define TENS_D 3
#define ONES_A 5
#define ONES_B 6
#define ONES_C 7
#define ONES_D 8

DFRobotDFPlayerMini myDFPlayer;

// Score and game timer variables
int score = 0;
int timerInterval = 3000;
unsigned long startTime;
bool taskSuccess = false;

// Game states
enum GameState {
    WAITING_FOR_START,
    GAME_RUNNING,
    GAME_OVER
};

GameState currentState = WAITING_FOR_START;

// Task enum
enum Task { SQUEEZE, YELL, CRANK };
Task currentTask;

// BCD Encoding for hex displays
const byte segmentMap[10] = { 
    0b00111111, // 0
    0b00000110, // 1
    0b01011011, // 2
    0b01001111, // 3
    0b01100110, // 4
    0b01101101, // 5
    0b01111101, // 6
    0b00000111, // 7
    0b01111111, // 8
    0b01101111  // 9
};

void setup() {
    Serial.begin(9600);

    // INPUTS
    pinMode(PRESSURE_SENSOR, INPUT);       
    pinMode(MIC, INPUT);
    pinMode(ROTARY_SENSOR_CLK, INPUT);     
    pinMode(START_BUTTON, INPUT_PULLUP); // Input pullup useful for buttons

    // LED Output
    pinMode(RED_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);

    // HEX DISPLAY Pins
    pinMode(TENS_A, OUTPUT);
    pinMode(TENS_B, OUTPUT);
    pinMode(TENS_C, OUTPUT);
    pinMode(TENS_D, OUTPUT);
    pinMode(ONES_A, OUTPUT);
    pinMode(ONES_B, OUTPUT);
    pinMode(ONES_C, OUTPUT);
    pinMode(ONES_D, OUTPUT);

    // Wait for user to press start and use timing for random seed
    unsigned long waitStart = millis();
    while (digitalRead(START_BUTTON) == HIGH) {
      // Show a countdown or blink while waiting, optional
    }
    unsigned long seed = millis() - waitStart;
    randomSeed(seed);

    // Initialize the DFPlayer Mini
    if (!myDFPlayer.begin(Serial)) {
        while(true) {
            digitalWrite(RED_LED, HIGH);
            delay(1000);
            digitalWrite(RED_LED, LOW);
            delay(1000);
        }
    }
    delay(500);
    myDFPlayer.volume(15);
    updateDisplay(score);
}

void loop() {
    // Handle the current state of the game
    switch (currentState) {
        case WAITING_FOR_START:
            handleWaitingForStart();
            break;
        
        case GAME_RUNNING:
            handleGameRunning();
            break;
        
        case GAME_OVER:
            handleGameOver();
            break;
    }
}

void handleWaitingForStart() {
    if (digitalRead(START_BUTTON) == LOW) {
        digitalWrite(GREEN_LED, HIGH);
        delay(200);
        digitalWrite(GREEN_LED, LOW);
        delay(2000); // Debounce
        startGame();
        currentState = GAME_RUNNING; // Transition to GAME_RUNNING state
    }
}

void handleGameRunning() {
    unsigned long currentTime = millis();
    
    // Check if the player took too long
    if ((currentTime - startTime) >= timerInterval) {
        if (!taskSuccess) {
            digitalWrite(RED_LED, HIGH);
            delay(200);
            digitalWrite(RED_LED, LOW);
            currentState = GAME_OVER; // Transition to GAME_OVER state
        } else {
            handleAction();  // Handle the action if successful
        }
    } else if (!taskSuccess && taskCompleted()) {
        taskSuccess = true;
        digitalWrite(GREEN_LED, HIGH);  // Flash green for success
    }
}

void handleGameOver() {
    if (score == 100) {
        myDFPlayer.play(5);  // Play win sound
    } else {
        myDFPlayer.play(4);  // Play game over sound
    }
    delay(2000);  // Wait for sound to finish
    
    score = 0;
    updateDisplay(score);  // Reset display
    
    // Return to the waiting state
    currentState = WAITING_FOR_START;
}

void startGame() {
    score = 0;
    timerInterval = 3000;
    taskSuccess = false;
    assignNewTask();
    
    if (currentTask == SQUEEZE) {
        myDFPlayer.play(1);  // Play 0001.mp3
    } else if (currentTask == YELL) {
        myDFPlayer.play(2);  // Play 0002.mp3
    } else if (currentTask == CRANK) {
        myDFPlayer.play(3);  // Play 0003.mp3
    }
    
    startTime = millis();
    delay(200);
}

void handleAction() {
    score++;
    if (score > 99) {
        gameOver();
        return;
    }

    // Speed up the time per action by 50 milliseconds, with a minimum of 500ms
    timerInterval = max(500, timerInterval - 50);
    
    digitalWrite(GREEN_LED, HIGH);
    delay(200);
    digitalWrite(GREEN_LED, LOW);

    updateDisplay(score);
    delay(2000);  // Delay between actions

    // Assign a new task
    taskSuccess = false;
    assignNewTask();

    if (currentTask == SQUEEZE) {
        myDFPlayer.play(1);  // Play 0001.mp3
    } else if (currentTask == YELL) {
        myDFPlayer.play(2);  // Play 0002.mp3
    } else if (currentTask == CRANK) {
        myDFPlayer.play(3);  // Play 0003.mp3
    }

    startTime = millis();
}

void updateDisplay(int score) {
    int ones = score % 10;  // Extract ones digit
    int tens = (score / 10) % 10;  // Extract tens digit

    sendBCD(tens, TENS_A, TENS_B, TENS_C, TENS_D);
    sendBCD(ones, ONES_A, ONES_B, ONES_C, ONES_D);
}

void sendBCD(int value, int pinA, int pinB, int pinC, int pinD) {
    digitalWrite(pinA, value & 0x1);
    digitalWrite(pinB, (value >> 1) & 0x1);
    digitalWrite(pinC, (value >> 2) & 0x1);
    digitalWrite(pinD, (value >> 3) & 0x1);
}

bool taskCompleted() {
    switch (currentTask) {
        case SQUEEZE: return analogRead(PRESSURE_SENSOR) > 600;
        case YELL: return analogRead(MIC) > 1000;
        case CRANK: return checkRotarySensor();
    }
    return false;
}

void assignNewTask() {
    currentTask = static_cast<Task>(random(0, 3));
}

bool checkRotarySensor() {
    int lastClkState = digitalRead(ROTARY_SENSOR_CLK); 
    int clkState = digitalRead(ROTARY_SENSOR_CLK);

    if (clkState != lastClkState) {
        lastClkState = clkState;
        return true;  
    }

    return false;
}

void gameOver() {
    if (score == 100) {
        myDFPlayer.play(5);  // Play win sound
    } else {
        myDFPlayer.play(4);  // Play game over sound
    }
    delay(2000);  // Wait for sound to finish
    
    score = 0;
    updateDisplay(score);  // Reset display
    
    // Return to the waiting state
    currentState = WAITING_FOR_START;
}
