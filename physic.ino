#include <M5StickC.h>

#define MAX_BALL_COUNT 500 // 最大ボール数
#define BALL_RADIUS 2      // ボールの半径 (2ピクセル = 2cm)
#define SCREEN_WIDTH 80    // 画面の幅 (80ピクセル = 80cm)
#define SCREEN_HEIGHT 160  // 画面の高さ (160ピクセル = 160cm)
#define GRAVITY 981.0      // 重力加速度 (cm/s^2)

bool isBrah = false;

float ballX[MAX_BALL_COUNT];
float ballY[MAX_BALL_COUNT];
float velocityX[MAX_BALL_COUNT];
float velocityY[MAX_BALL_COUNT];
uint16_t ballColors[MAX_BALL_COUNT]; // ボールの色

int ballCount = 10;             // 初期ボール数
unsigned long previousTime = 0; // 前回のフレーム計算時間

void initializeBall(int index)
{
    ballX[index] = random(BALL_RADIUS, SCREEN_WIDTH - BALL_RADIUS);
    ballY[index] = random(BALL_RADIUS, SCREEN_HEIGHT - BALL_RADIUS);
    velocityX[index] = random(-50, 50) / 10.0; // 初期速度 (cm/s)
    velocityY[index] = random(-50, 50) / 10.0;
    ballColors[index] = M5.Lcd.color565(random(0, 256), random(0, 256), random(0, 256)); // ランダムカラー
}

void setup()
{
    M5.begin();
    M5.Imu.Init();
    M5.Lcd.fillScreen(BLACK);

    for (int i = 0; i < ballCount; i++)
    {
        initializeBall(i);
    }
    previousTime = millis();
}

void loop()
{
    unsigned long currentTime = millis();
    float deltaTime = (currentTime - previousTime) / 1000.0; // デルタ時間
    previousTime = currentTime;

    float accX, accY, accZ;
    M5.update();

    // ボール数の増減
    if (M5.BtnA.wasPressed())
    {
        if (ballCount < MAX_BALL_COUNT)
        {
            ballCount *= 2; // ボール数を倍
            if (ballCount > MAX_BALL_COUNT)
                ballCount = MAX_BALL_COUNT; // 最大数を超えないように
            for (int i = 0; i < ballCount; i++)
            {
                if (i >= ballCount / 2)
                    initializeBall(i); // 新しいボールを初期化
            }
        }
    }

    if (M5.BtnB.wasPressed())
    {
        if (ballCount > 1)
        {
            ballCount /= 2; // ボール数を半分
            for (int i = ballCount; i < MAX_BALL_COUNT; i++)
            {
                M5.Lcd.fillCircle((int)ballX[i], (int)ballY[i], BALL_RADIUS + 2, BLACK); // 現在のボールを消去
            }
        }
    }
    if (M5.BtnA.isPressed() && M5.BtnB.wasPressed())
    {
        isBrah = !isBrah;
        M5.Lcd.fillScreen(BLACK);
    }

    M5.Imu.getAccelData(&accX, &accY, &accZ);

    // ボールの更新と描画
    for (int i = 0; i < ballCount; i++)
    {
        if (!isBrah)
        {
            M5.Lcd.fillCircle((int)ballX[i], (int)ballY[i], BALL_RADIUS + 2, BLACK);
        }

        // 速度の更新
        velocityX[i] += accX * GRAVITY * deltaTime;
        velocityY[i] += accY * GRAVITY * deltaTime;

        // 位置の更新
        ballX[i] -= velocityX[i] * deltaTime; // X軸は上下が反転するため減算
        ballY[i] += velocityY[i] * deltaTime;

        // 壁との衝突処理
        if (ballX[i] - BALL_RADIUS < 0 || ballX[i] + BALL_RADIUS > SCREEN_WIDTH)
        {
            velocityX[i] *= -0.8;
            ballX[i] = constrain(ballX[i], BALL_RADIUS, SCREEN_WIDTH - BALL_RADIUS);
        }
        if (ballY[i] - BALL_RADIUS < 0 || ballY[i] + BALL_RADIUS > SCREEN_HEIGHT)
        {
            velocityY[i] *= -0.8;
            ballY[i] = constrain(ballY[i], BALL_RADIUS, SCREEN_HEIGHT - BALL_RADIUS);
        }

        M5.Lcd.fillCircle((int)ballX[i], (int)ballY[i], BALL_RADIUS, ballColors[i]);
    }

    // ボール同士の当たり判定
    for (int i = 0; i < ballCount; i++)
    {
        for (int j = i + 1; j < ballCount; j++)
        {
            float dx = ballX[i] - ballX[j];
            float dy = ballY[i] - ballY[j];
            float distance = sqrt(dx * dx + dy * dy);

            if (distance < 2 * BALL_RADIUS)
            {
                float tempVx = velocityX[i];
                float tempVy = velocityY[i];

                velocityX[i] = velocityX[j];
                velocityY[i] = velocityY[j];
                velocityX[j] = tempVx;
                velocityY[j] = tempVy;

                float overlap = 2 * BALL_RADIUS - distance;
                float correctionX = (dx / distance) * overlap / 2.0;
                float correctionY = (dy / distance) * overlap / 2.0;

                ballX[i] += correctionX;
                ballY[i] += correctionY;
                ballX[j] -= correctionX;
                ballY[j] -= correctionY;
            }
        }
    }

    displayFPS(currentTime);
}

void displayFPS(unsigned long start)
{
    static unsigned long lastTime = 0;
    static int frameCount = 0, currentFrameRate = 0;

    frameCount++;
    if (millis() - lastTime >= 1000)
    {
        currentFrameRate = frameCount;
        frameCount = 0;
        lastTime = millis();
    }
    M5.Lcd.setCursor(1, 1);
    M5.Lcd.setTextColor(WHITE, BLACK);
    M5.Lcd.printf("FPS: %d\n", currentFrameRate);
}
