#include <Wire.h>
#include <esp_timer.h>

// ADXL345 I2C address
#define ADXL345_ADDRESS 0x53
#define DATA_FORMAT 0x31
#define BW_RATE 0x2C
#define POWER_CTL 0x2D
#define DATAX0 0x32

volatile bool readFlag = false;
portMUX_TYPE mux = portMUX_INITIALIZER_UNLOCKED;

// Timer handle
esp_timer_handle_t timer;

volatile int sampleNumber = 0;
volatile unsigned long lastTimestamp = 0;
volatile float gx, gy, gz;

void IRAM_ATTR onTimer(void* arg) 
{
  portENTER_CRITICAL_ISR(&mux);
  readFlag = true;
  portEXIT_CRITICAL_ISR(&mux);
}

void setup() {
  Serial.begin(115200);
  Wire.begin(21, 22); // SDA, SCL

  // Initialize ADXL345
  Wire.beginTransmission(ADXL345_ADDRESS);
  Wire.write(POWER_CTL);
  Wire.write(0x08); // Set measurement mode
  Wire.endTransmission();

  Wire.beginTransmission(ADXL345_ADDRESS);
  Wire.write(DATA_FORMAT);
  Wire.write(0x0B); // Full resolution, ±16g
  Wire.endTransmission();

  Wire.beginTransmission(ADXL345_ADDRESS);
  Wire.write(BW_RATE);
  Wire.write(0x0A); // 100 Hz data rate
  Wire.endTransmission();

  //timer init
  const esp_timer_create_args_t timer_args = 
  {
      .callback = &onTimer,
      .name = "sample_timer"};
  esp_timer_create(&timer_args, &timer);
  esp_timer_start_periodic(timer, 10000); // 10 ms = 10000 us

  Serial.println("Sample,Time (ms),X (g),Y (g),Z (g)");
}

void loop() 
{
  if (readFlag) 
  {
    portENTER_CRITICAL(&mux);
    readFlag = false;
    portEXIT_CRITICAL(&mux);

    sampleNumber++;

    //data read ADXL345
    int16_t x, y, z;
    readADXL345(x, y, z);
    //conversion to g
    gx = x * 0.004;
    gy = y * 0.004;
    gz = z * 0.004;

    lastTimestamp = millis();

    Serial.printf("%d,%lu,%.3f,%.3f,%.3f\n", sampleNumber, lastTimestamp, gx, gy, gz);
  }
}

void readADXL345(int16_t &x, int16_t &y, int16_t &z) 
{
  Wire.beginTransmission(ADXL345_ADDRESS);
  Wire.write(DATAX0);
  Wire.endTransmission(false);
  Wire.requestFrom(ADXL345_ADDRESS, 6);

  if (Wire.available() == 6) 
  {
    x = Wire.read() | (Wire.read() << 8);
    y = Wire.read() | (Wire.read() << 8);
    z = Wire.read() | (Wire.read() << 8);
  }
}
