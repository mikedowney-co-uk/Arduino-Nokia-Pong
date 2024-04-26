/*
 * From https://github.com/monur/Arduino-Uno-PCD8544-Pong and description at
 * https://makezine.com/projects/2-player-pong-game-with-arduino-uno/
 */


#include "nokia_5110.h"

#define PRO_MINI

#if defined(PRO_MINI)
// Pro Mini
#define PIN_SCE   7
#define PIN_RESET 6
#define PIN_DC    5
#define PIN_SDIN  4
#define PIN_SCLK  3
#define POT0      A0
#define POT1      A1
#elif defined(PICO)
// RPI Pico
#define PIN_SCE   7
#define PIN_RESET 6
#define PIN_DC    5
#define PIN_SDIN  4
#define PIN_SCLK  3
#define POT0      A0
#define POT1      A1
#endif

#define LCD_C     LOW
#define LCD_D     HIGH

#define LCD_X     84
#define LCD_Y     6

// button (Start/Pause/Mode)
#define BUTTON 11

// 2 possible computer opponents.
// COMPUTER is Player 1 (top of screen, used in demo or 1 player mode)
// DEMO is Player 2 (replaces HUMAN in demo mode)
#define HUMAN 0
#define COMPUTER 1
#define DEMO 2

nokia_5110 lcd(PIN_SCLK, PIN_SDIN, PIN_DC, PIN_RESET, PIN_SCE);

extern uint8_t MediumNumbers[];
extern uint8_t Sinclair_S[];

// Start with both as computer, for attract mode
uint8_t player1 = COMPUTER;
uint8_t player2 = DEMO;

int8_t barWidth1 = 16;
int8_t barWidth2 = 16;
int barHeight = 4;
int ballPerimeter = 4;
unsigned int bar1X = 0;
unsigned int bar1Y = 0;
unsigned int bar2X = 0;
unsigned int bar2Y = LCD_Y * 8 - barHeight;
int ballX = 0;
int ballY = 0;
boolean isBallUp = false;
boolean isBallRight = true;

byte pixels[LCD_X][LCD_Y];
unsigned long lastRefreshTime;
const int refreshInterval = 150;
byte gameState = 1;
byte ballSpeed = 2;
byte player1WinCount = 0;
byte player2WinCount = 0;
byte hitCount = 0;
byte game_limit = 10; // number of rounds per game

void setup() {
    lcd.InitLCD(LCD_CONTRAST);
    pinMode(BUTTON, INPUT);
    resetGame();
    restartGame();
}

void loop() {
    unsigned long now = millis();
    if (now - lastRefreshTime > refreshInterval) {
        update();
        refreshScreen();
        lastRefreshTime = now;
    }
}

void setupParams() {
    // loop until 'start game' button pressed

    lcd.setFont(Sinclair_S);
    lcd.print("PONG", CENTER, 0, 1);  // sinclair font needs rotating.
    for (int i = 0; i < 1000; i++) {
        if (digitalRead(BUTTON)) {
            lcd.print("GAME", CENTER, 16, 1);
            player1 = COMPUTER;
            player2 = HUMAN;
            delay(500);
            return;
        }
        delay(10);
    }
    lcd.print("DEMO", CENTER, 16, 1);
    player1 = COMPUTER;
    player2 = DEMO;
    delay(1000);
}


// Reset the scores after a game
void resetGame() {
    lcd.clrScr();
    setupParams();
    barWidth1 = 16;
    barWidth2 = 16;
    player1WinCount = 0;
    player2WinCount = 0;
}


void restartGame() {
    ballSpeed = 1;
    gameState = 1;
    ballX = random(0, 60);
    ballY = 20;
    isBallUp = random(2);
    isBallRight = random(2);
    hitCount = 0;
}

void refreshScreen() {
    if (gameState == 1) { // Game in progress
        for (uint8_t y = 0; y < LCD_Y; y++) {
            for (uint8_t x = 0; x < LCD_X; x++) {
                byte pixel = 0x00;
                uint8_t realY = y * 8;
                // draw ball if in the frame
                if (x >= ballX && x <= ballX + ballPerimeter - 1 && ballY + ballPerimeter > realY &&
                    ballY < realY + 8) {
                    byte ballMask = 0x00;
                    for (uint8_t i = 0; i < realY + 8 - ballY; i++) {
                        ballMask = ballMask >> 1;
                        if (i < ballPerimeter)
                            ballMask = 0x80 | ballMask;
                    }
                    pixel = pixel | ballMask;
                }

                // draw bars if in the frame
                if (x >= bar1X && x <= bar1X + barWidth1 - 1 && bar1Y + barHeight > realY && bar1Y < realY + 8) {
                    byte barMask = 0x00;
                    for (uint8_t i = 0; i < realY + 8 - bar1Y; i++) {
                        barMask = barMask >> 1;
                        if (i < barHeight)
                            barMask = 0x80 | barMask;
                    }
                    pixel = pixel | barMask;
                }

                if (x >= bar2X && x <= bar2X + barWidth2 - 1 && bar2Y + barHeight > realY && bar2Y < realY + 8) {
                    byte barMask = 0x00;
                    for (int i = 0; i < realY + 8 - bar2Y; i++) {
                        barMask = barMask >> 1;
                        if (i < barHeight)
                            barMask = 0x80 | barMask;
                    }
                    pixel = pixel | barMask;
                }
                LcdWrite(LCD_D, pixel);
            }
        }
    } else if (gameState == 2) {  // Someone has won
        LcdWrite(LCD_C, 0x0D);  // LCD in inverse mode.
        delay(300);
        LcdWrite(LCD_C, 0x0C);  // LCD in inverse mode.
        lcd.setFont(MediumNumbers);
        lcd.printNumI(player1WinCount, LEFT, 8);
        lcd.printNumI(player2WinCount, RIGHT, 8);
        delay(1000);
        if (player1WinCount == game_limit || player2WinCount == game_limit) {
            lcd.setFont(Sinclair_S);
            lcd.print("Winner:", CENTER, 32, 1);
            if (player1WinCount > player2WinCount) {
                lcd.print("Player 1", CENTER, 40, 1);
            } else {
                lcd.print("Player 2", CENTER, 40, 1);
            }
            delay(1000);
            resetGame();
            setupParams();
        }
    }
}
// Ultimately read bat speed from potentiometer to set computer player speed...
#define bat_speed 128

// Return a value between 0-1023 for bat position
int8_t move_bat(uint8_t player, uint8_t port, uint16_t bat_pos) {
    if (player == HUMAN) {
        uint8_t batpos = analogRead(port) / 12;
        if (batpos < 1) {
            return 1;
        }
        return batpos;
    }
    // computer player - move bar towards ball...
    // player 1 = top, player 2 = bottom
    //
    if (abs(bat_pos - ballX) < 4) {
        return bat_pos;
    }
    // Only move when the ball is heading towards the computer player
    // (gives the human a chance!)
    if ((!isBallUp && player == COMPUTER) || (isBallUp && player == DEMO)) {
        return bat_pos;
    }
    if (bat_pos < ballX) {
        return bat_pos + bat_speed / 64;
    } else {
        return bat_pos - bat_speed / 64;
    }
}


void update() {
    if (gameState == 1) {
        uint8_t barMargin1 = LCD_X - barWidth1;
        uint8_t barMargin2 = LCD_X - barWidth2;
        bar1X = move_bat(player1, A0, bar1X); //read potentiometers and set the bar positions
        bar2X = move_bat(player2, A1, bar2X);

        if (bar1X > barMargin1) bar1X = barMargin1;
        if (bar2X > barMargin2) bar2X = barMargin2;

        //move the ball now
        if (isBallUp)
            ballY -= ballSpeed;
        else
            ballY += ballSpeed;
        if (isBallRight)
            ballX += ballSpeed;
        else
            ballX -= ballSpeed;
        //check collisions
        if (ballX < 1) {
            isBallRight = true;
            ballX = 0;
        } else if (ballX > LCD_X - ballPerimeter - 1) {
            isBallRight = false;
            ballX = LCD_X - ballPerimeter;
        }
        if (ballY < barHeight) {
            if (ballX + ballPerimeter >= bar1X && ballX <= bar1X + barWidth1) { // ball bounces from bar1
                isBallUp = false;
                if (ballX + ballPerimeter / 2 < bar1X + barWidth1 / 2)
                    isBallRight = false;
                else
                    isBallRight = true;
                ballY = barHeight;
                if (++hitCount % 5 == 0 && ballSpeed < 5)
                    ballSpeed++;
            } else { //player2 wins
                gameState = 2;
                player2WinCount++;
            }
        }
        if (ballY + ballPerimeter > LCD_Y * 8 - barHeight) {
            if (ballX + ballPerimeter >= bar2X && ballX <= bar2X + barWidth2) { //ball bounces from bar2
                isBallUp = true;
                if (ballX + ballPerimeter / 2 < bar2X + barWidth2 / 2)
                    isBallRight = false;
                else
                    isBallRight = true;
                ballY = LCD_Y * 8 - barHeight - ballPerimeter;
                if (++hitCount % 10 == 0 && ballSpeed < 5)
                    ballSpeed++;
            } else { // player 1 wins
                gameState = 2;
                player1WinCount++;
            }
        }
    } else if (gameState == 2) {
        restartGame();
    }
}


void LcdWrite(byte dc, byte data) {
    digitalWrite(PIN_DC, dc);
    digitalWrite(PIN_SCE, LOW);
    shiftOut(PIN_SDIN, PIN_SCLK, MSBFIRST, data);
    digitalWrite(PIN_SCE, HIGH);
}
