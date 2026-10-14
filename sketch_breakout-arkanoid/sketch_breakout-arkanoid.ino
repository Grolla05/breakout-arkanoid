#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

// Breakout / Arkanoid para Arduino Uno + OLED SSD1306 128x64.
// O estado usa arrays fixos para limitar o uso de SRAM.
constexpr uint8_t SCREEN_WIDTH = 128;
constexpr uint8_t SCREEN_HEIGHT = 64;
constexpr int8_t OLED_RESET = -1;
constexpr uint8_t OLED_ADDRESS = 0x3C;
constexpr uint8_t POT_PIN = A0;
constexpr uint8_t BUZZER_PIN = 9;

constexpr uint8_t BRICK_COLUMNS = 8;
constexpr uint8_t BRICK_ROWS = 3;
constexpr uint8_t BRICK_WIDTH = 14;
constexpr uint8_t BRICK_HEIGHT = 5;
constexpr uint8_t BRICK_GAP = 2;
constexpr uint8_t BRICK_LEFT = 1;
constexpr uint8_t BRICK_TOP = 13;
constexpr uint8_t BALL_RADIUS = 2;
constexpr uint8_t PADDLE_WIDTH = 20;
constexpr uint8_t PADDLE_HEIGHT = 3;
constexpr uint8_t PADDLE_Y = 59;
constexpr uint16_t FRAME_MS = 20;

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

uint8_t brickHp[BRICK_ROWS][BRICK_COLUMNS];
float ballX = 64.0f;
float ballY = 51.0f;
float velocityX = 1.35f;
float velocityY = -1.55f;
uint8_t paddleX = 54;
uint8_t lives = 3;
uint16_t score = 0;
uint32_t lastFrameAt = 0;
uint32_t toneUntil = 0;
uint32_t restartAt = 0;
bool gameOver = false;
bool gameWon = false;

struct Rect {
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;
};

bool overlaps(const Rect &a, const Rect &b) {
  return a.x < b.x + b.w && a.x + a.w > b.x &&
         a.y < b.y + b.h && a.y + a.h > b.y;
}

Rect ballRect() {
  return {static_cast<int16_t>(ballX - BALL_RADIUS),
          static_cast<int16_t>(ballY - BALL_RADIUS),
          static_cast<int16_t>(BALL_RADIUS * 2 + 1),
          static_cast<int16_t>(BALL_RADIUS * 2 + 1)};
}

Rect brickRect(uint8_t row, uint8_t column) {
  const int16_t x = BRICK_LEFT + column * (BRICK_WIDTH + BRICK_GAP);
  const int16_t y = BRICK_TOP + row * (BRICK_HEIGHT + BRICK_GAP);
  return {x, y, BRICK_WIDTH, BRICK_HEIGHT};
}

void playEffect(uint16_t frequency, uint16_t duration) {
  tone(BUZZER_PIN, frequency);
  toneUntil = millis() + duration;
}

void serviceTone(uint32_t now) {
  if (toneUntil != 0 && static_cast<int32_t>(now - toneUntil) >= 0) {
    noTone(BUZZER_PIN);
    toneUntil = 0;
  }
}

void resetBall() {
  ballX = 64.0f;
  ballY = 51.0f;
  velocityX = (random(0, 2) == 0) ? -1.35f : 1.35f;
  velocityY = -1.55f;
}

void resetGame() {
  for (uint8_t row = 0; row < BRICK_ROWS; ++row) {
    const uint8_t hp = BRICK_ROWS - row;
    for (uint8_t column = 0; column < BRICK_COLUMNS; ++column) {
      brickHp[row][column] = hp;
    }
  }
  score = 0;
  lives = 3;
  gameOver = false;
  gameWon = false;
  restartAt = 0;
  resetBall();
}

int8_t damageBrickAt(const Rect &ball, int8_t alreadyHit) {
  for (uint8_t row = 0; row < BRICK_ROWS; ++row) {
    for (uint8_t column = 0; column < BRICK_COLUMNS; ++column) {
      if (brickHp[row][column] == 0) continue;
      if (!overlaps(ball, brickRect(row, column))) continue;

      const int8_t index = row * BRICK_COLUMNS + column;
      // A corner contact can overlap the same block in both axis passes.
      if (index == alreadyHit) return index;
      --brickHp[row][column];
      if (brickHp[row][column] == 0) score += 10;
      playEffect(brickHp[row][column] == 0 ? 880 : 520, 24);
      return index;
    }
  }
  return -1;
}

bool allBricksCleared() {
  for (uint8_t row = 0; row < BRICK_ROWS; ++row) {
    for (uint8_t column = 0; column < BRICK_COLUMNS; ++column) {
      if (brickHp[row][column] != 0) return false;
    }
  }
  return true;
}

void updatePaddle() {
  const int raw = analogRead(POT_PIN);
  paddleX = static_cast<uint8_t>(map(raw, 0, 1023, 0, SCREEN_WIDTH - PADDLE_WIDTH));
}

void updateBall() {
  int8_t hitBrick = -1;
  // Resolve X first: only horizontal velocity changes on a side collision.
  ballX += velocityX;
  if (ballX - BALL_RADIUS <= 0 || ballX + BALL_RADIUS >= SCREEN_WIDTH - 1) {
    velocityX = -velocityX;
    ballX = constrain(ballX, BALL_RADIUS, SCREEN_WIDTH - 1 - BALL_RADIUS);
    playEffect(420, 20);
  } else if ((hitBrick = damageBrickAt(ballRect(), hitBrick)) >= 0) {
    velocityX = -velocityX;
  }

  // Resolve Y separately for ceiling, bricks and paddle.
  ballY += velocityY;
  if (ballY - BALL_RADIUS <= 10) {
    velocityY = abs(velocityY);
    ballY = 10 + BALL_RADIUS;
    playEffect(360, 20);
  } else if ((hitBrick = damageBrickAt(ballRect(), hitBrick)) >= 0) {
    velocityY = -velocityY;
  }

  const Rect paddle = {paddleX, PADDLE_Y, PADDLE_WIDTH, PADDLE_HEIGHT};
  if (velocityY > 0 && overlaps(ballRect(), paddle)) {
    const float hitOffset = (ballX - (paddleX + PADDLE_WIDTH / 2.0f)) /
                            (PADDLE_WIDTH / 2.0f);
    velocityX = hitOffset * 2.0f;
    if (velocityX > 2.0f) velocityX = 2.0f;
    if (velocityX < -2.0f) velocityX = -2.0f;
    velocityY = -1.7f;
    ballY = PADDLE_Y - BALL_RADIUS - 1;
    playEffect(660, 30);
  } else if (ballY - BALL_RADIUS > SCREEN_HEIGHT) {
    if (lives > 0) --lives;
    if (lives == 0) {
      gameOver = true;
      restartAt = millis() + 2500;
    } else {
      resetBall();
      playEffect(240, 100);
    }
  }

  if (allBricksCleared()) {
    gameWon = true;
    restartAt = millis() + 2500;
  }
}

void drawGame() {
  display.clearDisplay();
  display.setTextSize(1);
  display.setTextColor(SSD1306_WHITE);
  display.setCursor(1, 1);
  display.print(F("PTS:"));
  display.print(score);
  display.setCursor(91, 1);
  display.print(F("V:"));
  display.print(lives);
  display.drawFastHLine(0, 9, SCREEN_WIDTH, SSD1306_WHITE);

  for (uint8_t row = 0; row < BRICK_ROWS; ++row) {
    for (uint8_t column = 0; column < BRICK_COLUMNS; ++column) {
      if (brickHp[row][column] == 0) continue;
      const Rect brick = brickRect(row, column);
      display.drawRect(brick.x, brick.y, brick.w, brick.h, SSD1306_WHITE);
      if (brickHp[row][column] > 1) {
        display.drawPixel(brick.x + brick.w / 2, brick.y + 2, SSD1306_WHITE);
      }
    }
  }

  display.fillRect(paddleX, PADDLE_Y, PADDLE_WIDTH, PADDLE_HEIGHT, SSD1306_WHITE);
  display.fillCircle(static_cast<int16_t>(ballX), static_cast<int16_t>(ballY),
                     BALL_RADIUS, SSD1306_WHITE);

  if (gameOver || gameWon) {
    display.fillRect(20, 27, 88, 18, SSD1306_BLACK);
    display.drawRect(20, 27, 88, 18, SSD1306_WHITE);
    display.setCursor(33, 33);
    display.print(gameWon ? F("VOCE VENCEU!") : F("FIM DE JOGO"));
  }
  display.display();
}

void setup() {
  pinMode(BUZZER_PIN, OUTPUT);
  randomSeed(analogRead(A1));

  if (!display.begin(SSD1306_SWITCHCAPVCC, OLED_ADDRESS)) {
    while (true) { delay(100); }
  }

  resetGame();
  lastFrameAt = millis();
}

void loop() {
  const uint32_t now = millis();
  serviceTone(now);

  if ((gameOver || gameWon) && static_cast<int32_t>(now - restartAt) >= 0) {
    resetGame();
  }

  if (now - lastFrameAt < FRAME_MS) return;
  lastFrameAt = now;

  updatePaddle();
  if (!gameOver && !gameWon) updateBall();
  drawGame();
}
