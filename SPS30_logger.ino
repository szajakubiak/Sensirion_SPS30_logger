#include <Wire.h>
#include <SPI.h>
#include <sps30.h>
#include <RTClib.h>
#include <SD.h>
#include <Adafruit_NeoPixel.h>

// RTC
RTC_PCF8523 rtc;

// Interval between consecutive measurements (in miliseconds)
const long interval = 60000;
// Variable to store time when last measurement was done
unsigned long previousMillis = 0;

// Variable to store filename in the format of YYYYMMDD
String file_name = "";

// NeoPixel
Adafruit_NeoPixel pixel(1, 8, NEO_GRB + NEO_KHZ800);

void setup()
{
  // Serial
  Serial.begin(9600);
  
  // NeoPixel
  pixel.begin();
  pixel.clear();
  pixel.show();
  
  // SPS30
  int16_t ret;
  uint8_t auto_clean_days = 4;
  uint32_t auto_clean;
  sensirion_i2c_init();
  sps30_set_fan_auto_cleaning_interval_days(auto_clean_days);

  while (sps30_probe() != 0) {
    Serial.println("SPS sensor probing failed");
    // Turn the NeoPixel on - red
    pixel.setPixelColor(0, pixel.Color(255, 0, 0));
    pixel.show();
    delay(500);
    // Turn the NeoPixel off
    pixel.clear();
    pixel.show();
    delay(500);
  }

  // RTC
  rtc.begin();
  DateTime now = rtc.now();
  char date_buf[] = "YYYYMMDD";
  file_name += now.toString(date_buf);
  file_name += ".txt";

  // SD card
  SD.begin(10);
}

void loop()
{
  // Read and store current time
  unsigned long currentMillis = millis();
  // Check if enough time has passed
  if (currentMillis - previousMillis >= interval || previousMillis == 0)
  {
    // Store time when measurement was performed
    previousMillis = currentMillis;
    
    // Turn the NeoPixel on - blue
    pixel.setPixelColor(0, pixel.Color(0, 0, 255));
    pixel.show();
    
    // SPS30
    struct sps30_measurement measurement;
    char serial[SPS30_MAX_SERIAL_LEN];
    int16_t ret;
    
    ret = sps30_start_measurement();
    
    if (ret < 0)
    {
      Serial.println("Error starting SPS measurement");
      // Turn the NeoPixel on - red
      pixel.setPixelColor(0, pixel.Color(255, 0, 0));
      pixel.show();
      delay(500);
      // Turn the NeoPixel off
      pixel.clear();
      pixel.show();
      delay(500);
    } else
    {
      delay(250);
      // Turn the NeoPixel off
      pixel.clear();
      pixel.show();
      delay(9750);
  
      ret = sps30_read_measurement(&measurement);
  
      if (ret < 0)
      {
        Serial.println("Error reading data from SPS");
        // Turn the NeoPixel on - red
        pixel.setPixelColor(0, pixel.Color(255, 0, 0));
        pixel.show();
        delay(500);
        // Turn the NeoPixel off
        pixel.clear();
        pixel.show();
        delay(500);
      } else
      {
        String dataString = "";
        dataString += String(measurement.mc_1p0);
        dataString += ",";
        dataString += String(measurement.mc_2p5);
        dataString += ",";
        dataString += String(measurement.mc_4p0);
        dataString += ",";
        dataString += String(measurement.mc_10p0);
        dataString += ",";
        dataString += String(measurement.nc_0p5);
        dataString += ",";
        dataString += String(measurement.nc_1p0  - measurement.nc_0p5);
        dataString += ",";
        dataString += String(measurement.nc_2p5  - measurement.nc_1p0);
        dataString += ",";
        dataString += String(measurement.nc_4p0  - measurement.nc_2p5);
        dataString += ",";
        dataString += String(measurement.nc_10p0  - measurement.nc_4p0);
        dataString += ",";
        dataString += String(measurement.typical_particle_size);
        sps30_stop_measurement();
      
        // RTC
        DateTime now = rtc.now();
        char buf_rtc[] = "YYYY.MM.DD,hh:mm:ss,";
      
        // SD card
        File dataFile = SD.open(file_name.c_str(), FILE_WRITE);
        if (dataFile)
        {
          // Turn the NeoPixel on - green
          pixel.setPixelColor(0, pixel.Color(0, 255, 0));
          pixel.show();
          
          dataFile.print(now.toString(buf_rtc));
          dataFile.println(dataString);
          dataFile.close();
    
          delay(250);
          // Turn the NeoPixel off
          pixel.clear();
          pixel.show();
  
          // Serial
          Serial.print(now.toString(buf_rtc));
          Serial.println(dataString);
        } else
        {
          Serial.println("Error during writing to file");
          // Turn the NeoPixel on - red
          pixel.setPixelColor(0, pixel.Color(255, 0, 0));
          pixel.show();
          delay(500);
          // Turn the NeoPixel off
          pixel.clear();
          pixel.show();
          delay(500);
        }
      }
    }
  }
  else
  {
    delay(5);
  }
}
