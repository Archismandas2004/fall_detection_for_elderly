#define BLYNK_PRINT Serial
#define BLYNK_TEMPLATE_ID "TMPLw-44Rorf"
#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>
#include <Wire.h>
#include <WiFi.h>
char auth[] = "YhMbQrmIKg_scwMOPGJruBm_VRBWJXAY";
char ssid[] = "Arai Tahzim";
char pass[] = "lantaklah";
const int MPU_addr = 0x68; // I2C address of the MPU-6050
int16_t AcX, AcY, AcZ, Tmp, GyX, GyY, GyZ;
float ax = 0, ay = 0, az = 0, gx = 0, gy = 0, gz = 0;
boolean fall = false; //stores if a fall has occurred
boolean trigger1 = false; //stores if first trigger (lower
threshold) has occurred
boolean trigger2 = false; //stores if second trigger (upper
threshold) has occurred
boolean trigger3 = false; //stores if third trigger (orientation
change) has occurred
byte trigger1count = 0; //stores the counts past since trigger 1
was set true
byte trigger2count = 0; //stores the counts past since trigger 2
was set true
byte trigger3count = 0; //stores the counts past since trigger 3
was set true
int angleChange = 0;
int ledBuzzer = 25;
int led = 27;
63
bool eventTrigger = false;
WidgetLED red(V0);
WidgetLED green(V1);
WidgetLCD lcd(V2);
void setup() {
 Serial.begin(115200);
 Blynk.begin(auth, ssid, pass);
 pinMode(ledBuzzer, OUTPUT);
 pinMode(led, OUTPUT);
 Wire.begin();
 Wire.beginTransmission(MPU_addr);
 Wire.write(0x6B);
 Wire.write(0);
 Wire.endTransmission(true);
 Serial.println("Wrote to IMU");
 for ( int X = 0; X < 2; X++)
 {
 digitalWrite(ledBuzzer, HIGH);
 digitalWrite(led, LOW);
 delay(500);
 digitalWrite(ledBuzzer, LOW);
 digitalWrite(led, HIGH);
 delay(500);
 }
 digitalWrite(led, LOW);
 lcd.clear();
 red.off();
 green.on();
 lcd.print( 0, 0, "Status : Normal ");
 delay(1000);
64
}
void loop() {
 Blynk.run();
 mpu_read();
 ax = (AcX - 2050) / 16384.00;
 ay = (AcY - 77) / 16384.00;
 az = (AcZ - 1947) / 16384.00;
 gx = (GyX + 270) / 131.07;
 gy = (GyY - 351) / 131.07;
 gz = (GyZ + 136) / 131.07;
 // calculating Amplitute vactor for 3 axis
 float Raw_Amp = pow(pow(ax, 2) + pow(ay, 2) + pow(az, 2), 0.5);
 int Amp = Raw_Amp * 10; // Mulitiplied by 10 bcz values are
between 0 to 1
 Serial.println(Amp);
 if (Amp <= 2 && trigger2 == false) { //if AM breaks lower
threshold (0.4g)
 trigger1 = true;
 Serial.println("TRIGGER 1 ACTIVATED");
 }
 if (trigger1 == true) {
 trigger1count++;
 if (Amp >= 12) { //if AM breaks upper threshold (3g)
 trigger2 = true;
 Serial.println("TRIGGER 2 ACTIVATED");
 trigger1 = false;
 trigger1count = 0;
 }
 }
65
 if (trigger2 == true) {
 trigger2count++;
 angleChange = pow(pow(gx, 2) + pow(gy, 2) + pow(gz, 2), 0.5);
 Serial.println(angleChange);
 if (angleChange >= 30 && angleChange <= 400) { //if orientation
changes by between 80-100 degrees
 trigger3 = true;
 trigger2 = false;
 trigger2count = 0;
 Serial.println(angleChange);
 Serial.println("TRIGGER 3 ACTIVATED");
 }
 }
 if (trigger3 == true) {
 trigger3count++;
 if (trigger3count >= 10) {
 angleChange = pow(pow(gx, 2) + pow(gy, 2) + pow(gz, 2), 0.5);
 //delay(10);
 Serial.println(angleChange);
 if ((angleChange >= 0) && (angleChange <= 10)) { //if
orientation changes remains between 0-10 degrees
 fall = true;
 trigger3 = false;
 trigger3count = 0;
 Serial.println(angleChange);
 }
 else { //user regained normal orientation
 trigger3 = false;
 trigger3count = 0;
 Serial.println("TRIGGER 3 DEACTIVATED");
 }
 }
66
 }
 if (fall == true) { //in event of a fall detection
 lcd.print( 0, 0, "Status : ");
 lcd.print( 0, 1, "Someone fell down ");
 Blynk.logEvent("Alert", "Someone fell down ");
 Serial.println("FALL DETECTED");
 for ( int X = 0; X < 30; X++)
 {
 digitalWrite(ledBuzzer, HIGH);
 digitalWrite(led, LOW);
 delay(50);
 digitalWrite(ledBuzzer, LOW);
 digitalWrite(led, HIGH);
 delay(50);
 }
 digitalWrite(led, LOW);
 red.on();
 green.off();
 delay(1000);
 lcd.clear();
 lcd.print( 0, 0, "Status : Normal");
 red.off();
 green.on();
 fall = false;
 }

 if (trigger2count >= 6) { //allow 0.5s for orientation change
 trigger2 = false;
 trigger2count = 0;
 Serial.println("TRIGGER 2 DECACTIVATED");
 }
67
 if (trigger1count >= 6) { //allow 0.5s for AM to break upper
threshold
 trigger1 = false;
 trigger1count = 0;
 Serial.println("TRIGGER 1 DECACTIVATED");
 }
 delay(100);
}
void mpu_read() {
 Wire.beginTransmission(MPU_addr);
 Wire.write(0x3B); // starting with register 0x3B (ACCEL_XOUT_H)
 Wire.endTransmission(false);
 Wire.requestFrom(MPU_addr, 14, true); // request a total of 14
registers
 AcX = Wire.read() << 8 | Wire.read(); // 0x3B (ACCEL_XOUT_H) &
0x3C (ACCEL_XOUT_L)
 AcY = Wire.read() << 8 | Wire.read(); // 0x3D (ACCEL_YOUT_H) &
0x3E (ACCEL_YOUT_L)
 AcZ = Wire.read() << 8 | Wire.read(); // 0x3F (ACCEL_ZOUT_H) &
0x40 (ACCEL_ZOUT_L)
 Tmp = Wire.read() << 8 | Wire.read(); // 0x41 (TEMP_OUT_H) & 0x42
(TEMP_OUT_L)
 GyX = Wire.read() << 8 | Wire.read(); // 0x43 (GYRO_XOUT_H) & 0x44
(GYRO_XOUT_L)
 GyY = Wire.read() << 8 | Wire.read(); // 0x45 (GYRO_YOUT_H) & 0x46
(GYRO_YOUT_L)
 GyZ = Wire.read() << 8 | Wire.read(); // 0x47 (GYRO_ZOUT_H) & 0x48
(GYRO_ZOUT_L)
}
