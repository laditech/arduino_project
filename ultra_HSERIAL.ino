#include <LiquidCrystal_I2C.h>
//
LiquidCrystal_I2C lcd(0x3F,16,2); 
//LiquidCrystal_I2C lcd(0x20,16,2); 
float readA02Distance() {
  unsigned char data[4] = {};
  float distance = -1.0; // Default error value
  
  // Request measurement
  unsigned char cmd[4] = {0xFF, 0x01, 0x00, 0x00};
  Serial.write(cmd, 4);
  
  // Wait for response (adjust delay based on your needs)
  delay(100);
  
  // Read response if available
  if (Serial.available() >= 4) {
    for (int i = 0; i < 4; i++) {
      data[i] = Serial.read();
    }

    Serial.print("Data:");
    Serial.print(data[0]);
    Serial.print(", ");
    Serial.print(data[1]);
    Serial.print(", ");
    Serial.print(data[2]);
    Serial.print(", ");
    Serial.println(data[3]);
    // Verify header byte
    if (data[0] == 0xFF) {
      // Verify checksum
      int sum = (data[0] + data[1] + data[2]) & 0x00FF;
      
      if (sum == data[3]) {
        distance = ((data[1] << 8) + data[2]) / 10.0; // Convert to cm
      }
    }
  }
  
  return distance;
}

void setup() {
  Serial.begin(9600);
  //Serial.begin(9600);
  lcd.init();  
lcd.backlight();  
}


void loop() {
  float distance = readA02Distance();
  
  if (distance >= 0) {
    //Serial.print("Distance: ");
    //Serial.print(distance);
    //Serial.println(" cm");
  } else {
    //Serial.println("Error reading distance");
  }
  lcd.home();
  lcd.setCursor(3, 0);
  lcd.print(distance);
  delay(1000); // Delay between measurements
}