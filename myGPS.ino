/**
 * Wiring Pins
 * 
 * Display
 *  SCL Analog B0
 *  SDA Digital B1
 *   
 * GPS Software serial
 *  11, 10 TX, RX
 *  
 */
 
#include <Wire.h>
#include <SoftwareSerial.h>


#include <Adafruit_GPS.h>
#include "Display.h"
#include "Button.h"
#include "GridReference.h"

const byte         DISPLAY_BUTTON_PIN = 2;
const long         DISPLAY_TIME = 10000;
const int          REFRESH_INTERVAL = 250;
const long         AUTO_RENDER_INTERVAL = 200000;
const unsigned int DISPLAY_BUTTON_HOLD_TIME = 1000;

const double MOCK_LAT = 53.4362297;
const double MOCK_LON = -1.9546493;

SoftwareSerial gpsSerial(11, 10);
Adafruit_GPS   gps(&gpsSerial);

unsigned long timeAtLastRender;
unsigned long timeAtLastRefresh;

byte toDisplay = 0;
boolean render = false;
boolean alwaysOn = false;

Display screen = Display();
Button displayButton = Button();

GridReference gridReference = GridReference();

void setup() {
  //Serial.begin(9600);
  screen.init();
  displayButton.init(DISPLAY_BUTTON_PIN, DISPLAY_BUTTON_HOLD_TIME);
  
  gps.begin(9600);
  gps.sendCommand(PMTK_SET_NMEA_OUTPUT_RMCGGA);
  gps.sendCommand(PMTK_SET_NMEA_UPDATE_1HZ);   // 1 Hz update rate

  screen.renderCharArray(" MYGPS  ");
  timeAtLastRender = millis();
  delay(2000);
  screen.clear();
}

void loop() {
  unsigned long now = millis();
  
  displayButton.tick();
  
  if(displayButton.isPressed()){
    render = true;
    timeAtLastRender = now;
  }
  
  if(displayButton.isHeld()) { 
    alwaysOn = !alwaysOn;

    /*toDisplay++;
    if(toDisplay > 1){
      toDisplay = 0;
    }
    */
  }

  if(now - timeAtLastRender > DISPLAY_TIME) { // reset the screen and turn it off
    render = false;
  }


  if((render == false) && (now - timeAtLastRender) > AUTO_RENDER_INTERVAL){
    render = true;
    timeAtLastRender = now;
  }
    
  if((now - timeAtLastRefresh) > REFRESH_INTERVAL){
   
    if(render || alwaysOn){ // only render the screen every n milliseconds
      renderLocation();
      /*
      switch (toDisplay) {
        case 0:
          renderLocation();
          break;
        case 1:
          renderAlt();
          break;
        default:
           renderLocation();
      } 
      */

    } else {
      screen.clear();
    }
   
    timeAtLastRefresh = now;   
   
  }
  
  char c = gps.read();
  
  if(gps.newNMEAreceived()){
    gps.parse(gps.lastNMEA()); 
  }
}

void renderLocation(){
  if(gps.fix){
    screen.renderString(gridReference.calculate(gps.latitudeDegrees, gps.longitudeDegrees));
  } else {
    screen.renderString(F("LOCATING"));
  }
}

void renderAlt(){
  //char toDisplay[8] = "ALT 1234";
  //screen.renderCharArray(toDisplay);
  screen.renderString("ALT " + String((int) gps.altitude));
}





