//Pins assigned to our components
#define PRESSURE_SENSOR 23
#define MIC 24

#define ROTARY_SENSOR_CLK 25

//#define ROTARY_SENSOR_DT 26 //MAY NOT BE NEEDED, WE ONLY NEED TO CHECK FOR ANY MOVEMENT

//UNDEFINED PINS
#define RED_LED
#define GREEN_LED
#define SPEAKER
#define START_BUTTON

//HEX PINS
#define TENS_A 2
#define TENS_B 3
#define TENS_C 4
#define TENS_D 5

#define ONES_A 11
#define ONES_B 12
#define ONES_C 13
#define ONES_D 14

//Score at 0
int score = 0;
//Start with three seconds per action
int timerInterval = 3000;
unsigned long startTime;
bool gameRunning = false;

//Lists each of the tasks
enum Task { SQUEEZE, YELL, CRANK };
Task currentTask;

//Use an unconnected pin to make sure each game is different, using this in setup makes sure we dont get same random sequence
const int RANDOM_SEED_PIN = A0;

// BCD Encoding for our hex displays
const byte segmentMap[10] = 
{ 
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

    //INPUTS
    pinMode(PRESSURE_SENSOR, INPUT);       
    pinMode(MIC, INPUT); //May have to switch to analog?                
    pinMode(ROTARY_SENSOR_CLK, INPUT);     
    pinMode(ROTARY_SENSOR_DT, INPUT);
    pinMode(START_BUTTON, INPUT_PULLUP); //Input pullup useful for buttons in general

    //Speaker and LED output
    pinMode(SPEAKER, OUTPUT);
    pinMode(RED_LED, OUTPUT);
    pinMode(GREEN_LED, OUTPUT);

    //HEX DISPLAYS
    pinMode(TENS_A, OUTPUT);
    pinMode(TENS_B, OUTPUT);
    pinMode(TENS_C, OUTPUT);
    pinMode(TENS_D, OUTPUT);

    pinMode(ONES_A, OUTPUT);
    pinMode(ONES_B, OUTPUT);
    pinMode(ONES_C, OUTPUT);
    pinMode(ONES_D, OUTPUT);

    //Set our random seed to the unconnected pin for true randomness
    randomSeed(analogRead(RANDOM_SEED_PIN));

    //Initialize our hex to be 0
    updateDisplay(0);

    //Sets baud rate, sstandard value
    Serial.begin(9600);
}

void loop() {

    //Before start or if they take too long
    if (!gameRunning) 
    {
        Serial.println("Press to Start!");
        //Input pullup check
        if (digitalRead(START_BUTTON) == LOW) 
        {
            delay(500); // Debounce
            startGame();
        }
    } 
    else 
    {
        //Check current time with no delay
        unsigned long currentTime = millis();
        
        //If the user takes too much time
        if (currentTime - startTime >= timerInterval)
        {
            //Flash the red led
            digitalWrite(RED_LED, HIGH);
            delay(200);
            digitalWrite(RED_LED, LOW);

            gameOver();
            return;
        }

        // Check if a sensor is triggered
        if (taskCompleted())
        {
            handleAction();
        }
    }
}

void startGame() 
{
    gameRunning = true;
    score = 0;
    timerInterval = 3000;
    startTime = millis();
    Serial.println("Game Started!");
}

//For score of 100 or when missed action
void gameOver() 
{
    Serial.println("Game Over!");
    gameRunning = false;
}

bool checkRotarySensor() 
{
    static int lastClkState = digitalRead(ROTARY_SENSOR_CLK); 
    int clkState = digitalRead(ROTARY_SENSOR_CLK);

    //Checks for any type of rotation
    //Reference the data sheet for directional input, must use SW then
    if (clkState != lastClkState) 
    {
        lastClkState = clkState;  // Update last state
        return true;  
    }
    
    return false;
}

//Handle an action triggered by any input
void handleAction() 
{
    //Increment score
    score++;
    Serial.print("Score: ");
    Serial.println(score);

    //Check if the score is 100
    if (score > 99) 
    {
        Serial.println("YOU WIN!");
        gameOver();
        return;
    }

    //Speeds up the time per action by 50 seconds with a minimum of half a second, adjust as needed
    timerInterval = max(500, timerInterval - 50);
    startTime = millis();

    //Flash the green led
    digitalWrite(GREEN_LED, HIGH);
    delay(200);
    digitalWrite(GREEN_LED, LOW);

    // Update hex display
    updateDisplay(score);

    //After successful attempt, we need a new task
    assignNewTask();
}

//Will update score for both hexes
void updateDisplay(int score) 
{
    int ones = score % 10;  // Extract ones digit
    int tens = (score / 10) % 10;  // Extract tens digit

    sendBCD(tens, TENS_A, TENS_B, TENS_C, TENS_D);
    sendBCD(ones, ONES_A, ONES_B, ONES_C, ONES_D);
}

// Will send our BCD values to the decoder chip
void sendBCD(int value, int pinA, int pinB, int pinC, int pinD)
{
    //Write to each pin, the >> is used to shift right, get the value we want in the LSB, then check it by using the mask of 0x1
    digitalWrite(pinA, value & 0x1);
    digitalWrite(pinB, (value >> 1) & 0x1);
    digitalWrite(pinC, (value >> 2) & 0x1);
    digitalWrite(pinD, (value >> 3) & 0x1);

}

//Check for the proper input depending on what is required    
bool taskCompleted() 
{
    switch (currentTask) 
    {
        case SQUEEZE: return digitalRead(PRESSURE_SENSOR) == LOW;
        //Analog input, adjust as needed, goes on scale of 0 to 1023
        case YELL: return analogRead(MIC) > 512;
        case CRANK: return checkRotarySensor();
    }
    return false;
}

void assignNewTask() 
{
    //Random task from our three options
    currentTask = static_cast<Task>(random(0, 3));
    
    //Serial print right now, use for the speaker later?
    switch (currentTask)
    {
        case SQUEEZE: Serial.println("Task: PRESS the pressure sensor!"); break;
        case YELL: Serial.println("Task: SHOUT into the mic!"); break;
        case CRANK: Serial.println("Task: TURN the rotary encoder!"); break;
    }
}