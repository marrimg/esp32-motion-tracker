#include "esp_camera.h"
#include <ESP32Servo.h>

//--------Cam vars
unsigned long lastFrameTime = 0;
// --- Motion Detection Configurations ---
#define WIDTH 160
#define HEIGHT 120
#define BRIGHTNESS_THRESHOLD 100    // Pixel brightness change sensitivity
#define CHANGED_PIXELS_THRESHOLD 25 // Minimum changed pixels to qualify as motion
#define FRAME_CAPTURE_FREQUENCY 200
uint8_t prev_frame[WIDTH * HEIGHT] = {0};
#define PWDN_GPIO_NUM -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM 21
#define SIOD_GPIO_NUM 26
#define SIOC_GPIO_NUM 27
#define Y9_GPIO_NUM 35
#define Y8_GPIO_NUM 34
#define Y7_GPIO_NUM 39
#define Y6_GPIO_NUM 36
#define Y5_GPIO_NUM 19
#define Y4_GPIO_NUM 18
#define Y3_GPIO_NUM 5
#define Y2_GPIO_NUM 4
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM 23
#define PCLK_GPIO_NUM 22

float avgX = 0;
float avgY = 0;
int angleX = 0;
int angleY = 0;

//------Servo vars
Servo panServo;
Servo tiltServo;

// int mini = 100;
// int maxi = 100;
// unsigned long prevMillis = 0;
// const long interval = 1000; // Minimal delay time between servo changes
// int flip = 0; // For testing, flip between min/max values per servo change

void setup()
{
  // initialize serial:
  Serial.begin(115200); // set serial monitor baud rate to match
  panServo.write(90);
  panServo.attach(12, 544, 2400);

  tiltServo.write(90);
  tiltServo.attach(13, 544, 2400);

  // Camera config
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sccb_sda = SIOD_GPIO_NUM;
  config.pin_sccb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_GRAYSCALE; // Grayscale is faster for delta calculations
  config.frame_size = FRAMESIZE_QQVGA;       // 160x120 resolution
  config.jpeg_quality = 12;
  config.fb_count = 1;

  // Camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK)
  {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  Serial.println("Camera initialized successfully.");
}

void loop()
{
  unsigned long currentMillis = millis();

  // Camera
  if (currentMillis - lastFrameTime >= FRAME_CAPTURE_FREQUENCY)
  {
    lastFrameTime = currentMillis;

    camera_fb_t *fb = esp_camera_fb_get(); // Gets image frame from camera and saves to variable fb

    if (!fb)
    {
      Serial.println("Camera capture failed");
      return;
    }

    long sumX = 0;
    long sumY = 0;
    int changedPixelCount = 0;

    // Loops through all of the pixels in the frame buffer (fb)
    for (int y = 0; y < fb->height; y++)
    {
      for (int x = 0; x < fb->width; x++)
      {

        int loc = x + y * fb->width;
        uint8_t current_pixel = fb->buf[loc];
        uint8_t old_pixel = prev_frame[loc];
        if (abs(current_pixel - old_pixel) > BRIGHTNESS_THRESHOLD)
        {
          sumX += x;
          sumY += y;
          changedPixelCount++;
        }

        prev_frame[loc] = current_pixel; // Save current frame pixel to prev_frame
      }
    }

    if (changedPixelCount > CHANGED_PIXELS_THRESHOLD)
    {

      avgX = sumX / changedPixelCount;
      avgY = sumY / changedPixelCount;

      angleX = map(avgX, 0, fb->width, 0, 180);
      angleY = map(avgY, 0, fb->height, 0, 180);

      // 3. Constrain angles to safe servo limits
      angleX = constrain(angleX, 0, 180);
      angleY = constrain(angleY, 0, 180);
    }
    // Serial.print(">");
    // Serial.print("var1:");
    // Serial.print(angleY);
    // Serial.println();

    panServo.write(angleX);
    tiltServo.write(angleY);

    esp_camera_fb_return(fb); // Release memory buffer
  }

  // Write to servos
}

/*
// grab test servo vals
    if (Serial.available() > 0)
    {

      maxi = Serial.parseInt();
      mini = Serial.parseInt();

      // Look for the newline/carriage return character to clear the buffer
      if (Serial.read() == '\n' || Serial.read() == '\r')
      {
        // Data fully read
      }

      if (maxi != 0)
      {
        Serial.print("Maxi: ");
        Serial.println(maxi);
      }

      Serial.print("Mini: ");
      Serial.println(mini);
    }


    if ((currentMillis - prevMillis) > interval)
    {
      prevMillis = millis();

      flip = !flip; // Flip is just for this test

      if (flip == 0)
        servo.write(mini);
      // delay(1000);
      if (flip == 1)
        servo.write(maxi);
      // delay(1000);
    }

*/