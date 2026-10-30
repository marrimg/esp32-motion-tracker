#include "esp_camera.h"
#include <ESP32Servo.h>

// --- Servo Configuration ---
#define PAN_PIN  12
#define TILT_PIN 13
// #define BLINK_PIN 15

Servo panServo;
Servo tiltServo;

int currentPanAngle = 90;
int currentTiltAngle = 90;

int xSlewCount = 0;
int ySlewCount = 0;
int slewMax = 10;
float xValues[10];
float yValues[10];

// Time tracking for comparison interval
unsigned long previousMillis = 0;

// --- Motion Detection Configurations ---
#define WIDTH 160
#define HEIGHT 120
#define BRIGHTNESS_THRESHOLD 20     // Pixel brightness change sensitivity
#define CHANGED_PIXELS_THRESHOLD 50 // Minimum changed pixels to qualify as motion
#define COMPARISON_INTERVAL 20 // Sets the speed of the comparison

uint8_t prev_frame[WIDTH * HEIGHT] = {0};

// --- Camera Pin Definition (Freenove / WROVER CAM typical pins) ---
#define PWDN_GPIO_NUM  -1
#define RESET_GPIO_NUM -1
#define XCLK_GPIO_NUM  21
#define SIOD_GPIO_NUM  26
#define SIOC_GPIO_NUM  27
#define Y9_GPIO_NUM    35
#define Y8_GPIO_NUM    34
#define Y7_GPIO_NUM    39
#define Y6_GPIO_NUM    36
#define Y5_GPIO_NUM    19
#define Y4_GPIO_NUM    18
#define Y3_GPIO_NUM    5
#define Y2_GPIO_NUM    4
#define VSYNC_GPIO_NUM 25
#define HREF_GPIO_NUM  23
#define PCLK_GPIO_NUM  22

void setup() {
  Serial.begin(115200);

  // Initialize Servos
  panServo.attach(PAN_PIN);
  tiltServo.attach(TILT_PIN);
  panServo.write(currentPanAngle);
  tiltServo.write(currentTiltAngle);

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
  if (err != ESP_OK) {
    Serial.printf("Camera init failed with error 0x%x", err);
    return;
  }
  Serial.println("Camera initialized successfully.");
}

void loop() {

  //unsigned long currentMillis = millis();

  // Runs frame comparison logic at the speed of COMPARISON_INTERVAL
  // if (currentMillis - previousMillis >= COMPARISON_INTERVAL) {

      // Gets a still image frame from the camera and saves it to the variable fb
      camera_fb_t *fb = esp_camera_fb_get();

      if (!fb) {
        Serial.println("Camera capture failed");
        return;
      }

      long sumX = 0;
      long sumY = 0;
      int changedPixelCount = 0;


      // Loops through all of the pixels in the frame buffer (fb)
      for (int y = 0; y < fb->height; y++) {
        for (int x = 0; x < fb->width; x++) {
          // Compares the brightness of the current pixel with the brightness of the pixel in the same location in the previous frame.
          // If the difference between the two values is greater than the BRIGHTNESS_THRESHOLD,
          // 1. Add the value of x to sumX
          // 2. Add the value of y to sumY
          // 3. Increment changedPixelCount
          int loc = x + y * fb->width;
          uint8_t current_pixel = fb->buf[loc];
          uint8_t old_pixel = prev_frame[loc];
          if (abs(current_pixel - old_pixel) > BRIGHTNESS_THRESHOLD) {
            sumX += x;
            sumY += y;
            changedPixelCount++;
          }
          
          // Save current frame pixel to prev_frame
          prev_frame[loc] = current_pixel;
        }
      }

      // If changedPixelCount is greater than the CHANGED_PIXELS_THRESHOLD,
      // 
      if (changedPixelCount > CHANGED_PIXELS_THRESHOLD) {


        float avgX = sumX / changedPixelCount;
        float avgY = sumY / changedPixelCount;

        xSlewCount += 1;
        if(xSlewCount > slewMax) xSlewCount = 0;
        xValues[xSlewCount] = avgX;
        float xSmoov = 0;

        ySlewCount += 1;
        if(ySlewCount > slewMax) ySlewCount = 0;
        yValues[ySlewCount] = avgY;
        float ySmoov = 0;

        for(int s = 0; s < slewMax; s++) {
          xSmoov +=  xValues[s];
          ySmoov +=  yValues[s];
        }
        xSmoov /= slewMax;
        ySmoov /= slewMax;
        Serial.println(ySmoov);

        // // Calculate center deviation error (Target frame center is 80, 60)
        // int errorX = avgX - (WIDTH / 2);
        // int errorY = avgY - (HEIGHT / 2);

        // // Serial.printf("Motion Detected! Center: X=%d, Y=%d | Error: X=%d, Y=%d\n", avgX, avgY, errorX, errorY);

        // // Smooth proportional adjustments to servo angles
        // if (abs(errorX) > 10) { // Deadzone buffer to prevent servo jittering
        //   currentPanAngle -= (errorX > 0) ? 20 : -20; // Reverse signs if servos move backward
        //   currentPanAngle = constrain(currentPanAngle, 10, 170);
        //   panServo.write(currentPanAngle);
        // }

        // if (abs(errorY) > 10) {
        //   currentTiltAngle += (errorY > 0) ? 20 : -20; 
        //   currentTiltAngle = constrain(currentTiltAngle, 10, 170);
        //   tiltServo.write(currentTiltAngle);
        // }
      }

          //blink
        // unsigned long currentMillis = millis(); // Get the current time
        //     if (currentMillis - previousMillis >= interval) {
        //       long newInterval = random(3, 6); 
        //       previousMillis = currentMillis; // Save the last time you blinked the LED
        //       // interval = newInterval;

        //   }

      esp_camera_fb_return(fb); // Release memory buffer
      // delay(20); // Control sample rate loop timing
  // }
}

// void compare_frames(const camera_fb_t *fb, const uint8_t *prev_frame){

// }
