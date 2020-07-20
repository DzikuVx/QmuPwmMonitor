#include "SSD1306.h"

SSD1306 display(0x3c, 4, 15);

#define PWM_CHANNEL_COUNT 6
const byte channel_pin[] = {13, 12, 14, 27, 26, 25};
volatile unsigned long rising_start[] = {0, 0, 0, 0, 0, 0};
volatile long channel_length[] = {0, 0, 0, 0, 0, 0};

long smooth_channel_length[] = {0, 0, 0, 0, 0, 0};

int32_t smooth(uint32_t data, float filterVal, float smoothedVal)
{
    if (filterVal > 1)
    {
        filterVal = .99;
    }
    else if (filterVal <= 0)
    {
        filterVal = 0;
    }

    smoothedVal = (data * (1 - filterVal)) + (smoothedVal * filterVal);

    return (uint32_t)smoothedVal;
}

void processPin(byte pin) {

    uint8_t state = digitalRead(channel_pin[pin]);

    if (state == HIGH) {
        rising_start[pin] = micros();
    } else if (state == LOW) {
        channel_length[pin] = micros() - rising_start[pin];
    }
}

void onRising0(void) {
  processPin(0);
}

void onRising1(void) {
  processPin(1);
}

void onRising2(void) {
  processPin(2);
}

void onRising3(void) {
  processPin(3);
}

void onRising4(void) {
  processPin(4);
}

void onRising5(void) {
  processPin(5);
}

void setup()
{

    pinMode(16, OUTPUT);
    digitalWrite(16, LOW); // set GPIO16 low to reset OLED
    delay(50);
    digitalWrite(16, HIGH); // while OLED is running, must set GPIO16 to high
    Wire.begin(4, 15);

    display.init();
    display.flipScreenVertically();
    display.setFont(ArialMT_Plain_10);

    pinMode(channel_pin[0], INPUT);
    pinMode(channel_pin[1], INPUT);
    pinMode(channel_pin[2], INPUT);
    pinMode(channel_pin[3], INPUT);
    pinMode(channel_pin[4], INPUT);
    pinMode(channel_pin[5], INPUT);
    attachInterrupt(digitalPinToInterrupt(channel_pin[0]), onRising0, CHANGE);
    attachInterrupt(digitalPinToInterrupt(channel_pin[1]), onRising1, CHANGE);
    attachInterrupt(digitalPinToInterrupt(channel_pin[2]), onRising2, CHANGE);
    attachInterrupt(digitalPinToInterrupt(channel_pin[3]), onRising3, CHANGE);
    attachInterrupt(digitalPinToInterrupt(channel_pin[4]), onRising4, CHANGE);
    attachInterrupt(digitalPinToInterrupt(channel_pin[5]), onRising5, CHANGE);
}

#define TASK_PERIOD_OLED 100
#define TASK_SMOOTH_PERIOD 20

uint32_t nextOledTask = 0;
uint32_t nextSmoothTask = 0;

void loop()
{

    if (millis() > nextSmoothTask) {
        for (uint8_t i = 0; i < PWM_CHANNEL_COUNT; i++) {
            smooth_channel_length[i] = smooth(channel_length[i], 0.5, smooth_channel_length[i]);
        }

        nextSmoothTask = millis() + TASK_SMOOTH_PERIOD;
    }

    if (millis() > nextOledTask) {

        display.clear();
        display.drawString(0, 0, "S1: " + String(smooth_channel_length[0]));
        display.drawString(0, 10, "S2: " + String(smooth_channel_length[1]));
        display.drawString(0, 20, "S3: " + String(smooth_channel_length[2]));
        display.drawString(0, 30, "S4: " + String(smooth_channel_length[3]));
        display.drawString(0, 40, "S5: " + String(smooth_channel_length[4]));
        display.drawString(0, 50, "S6: " + String(smooth_channel_length[5]));
        display.display();

        nextOledTask = millis() + TASK_PERIOD_OLED;

    }
}
