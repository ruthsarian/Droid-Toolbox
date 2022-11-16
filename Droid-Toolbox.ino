/* Droid Toolbox v0.1 : ruthsarian@gmail.com
 * 
 * A program to work with droids from the Droid Depot at Galaxy's Edge.
 * 
 * Features
 *   - Scan for nearby droids
 *   - Generate location and droid beacons
 *   - Control audio produced by droids
 * 
 * Designed to be used with a TTGO / LILYGO ESP32 Module with LCD display
 *   see: https://www.amazon.com/dp/B099MPFJ9M or https://www.amazon.com/dp/B098PYJ7ZL
 * 
 * References
 *   Arduino IDE setup: https://www.youtube.com/watch?v=b8254--ibmM
 *   Arduino TFT_eSPI library: https://github.com/Bodmer/TFT_eSPI
 *   Arduino ESP32 core: https://github.com/espressif/arduino-esp32
 *   TTGO pinout: https://i.redd.it/1usgojazvq561.jpg
 *   
 *   Misc:
 *     https://github.com/Bodmer/TFT_eSPI/blob/master/TFT_eSPI.h
 *     https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/BLEScan.h
 *     https://programmer.ink/think/arduino-development-tft_espi-library-learning.html
 *     https://programmer.ink/think/color-setting-and-text-display-esp32-learning-tour-arduino-version.html.
 *     https://github.com/nkolban/esp32-snippets/blob/fe3d318acddf87c6918944f24e8b899d63c816dd/cpp_utils/BLEAdvertisedDevice.h
 *   
 * Notes
 *   tft.width(), tft.height()
 *   tft.textWidth(), tft.fontHeight()
 *   tft.print() vs tft.println()
 *   
 *   keep text size to no smaller than 2
 *
 * TODO
 *   sort by RSSI (signal strength) so closest appear at the top
 *   display RSSI
 *   display 1 droid at a time, use button press to step to next droid
 */

#include <SPI.h>
#include <TFT_eSPI.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#define BUTTON1_PIN   0
#define BUTTON2_PIN   35
#define LAZY_DEBOUNCE 10

#define MAX_DROIDS 5
#define BLE_SCAN_TIME 5   // maximum number of droids to report on

BLEScan* pBLEScan;
BLEAdvertising* pAdvertising;
BLEAdvertisementData oAdvertisementData = BLEAdvertisementData();
BLEAdvertisementData oScanResponseData = BLEAdvertisementData();

typedef struct droid_t {
    uint8_t chipid;
    uint8_t affid;
    int rssi;
    char addr[20];
} Droid;

Droid droids[MAX_DROIDS];
uint8_t droid_count = 0;

/*
const uint8_t SWGE_LOCATION_BEACON_PAYLOAD[] = {
  0x83, 0x01, // manufacturer's id: 0x0183
  0x0A,       // type of beacon (location beacon)
  0x04,       // length of beacon data
  0x01,       // location; also corresponds to the audio group the droid will select a sound from
  0x02,       // minimum interval between droid reactions to the beacon; this value is multiplied by 5 to determine the interval in seconds. droids have a minimum reaction time of 60 seconds
  0xA6,       // expected RSSI, beacon is ignored if weaker than value specified
  0x01,       // ? 0 or 1 otherwise droid will ignore the beacon
};

const uint8_t SWGE_DROID_BEACON_PAYLOAD[] = {
  0x83, 0x01, // manufacturer's id: 0x0183
  0x03,       // type of beacon (droid beacon)
  0x04,       // length of beacon data
  0x44,       // ??
  0x81,       // 0x01 + ( 0x80 if droid is paired with a remote)
  0x82,       // a combination of personality chip and affiliation IDs
  0x01,       // personality chip ID
};

const uint8_t SWGE_BEACON_NAME[] = {
  0x44, 0x54, 0x4F, 0x49, 0x44   // 'DROID'
};
*/

uint8_t payload[] = {
  0x83, 0x01, 0x0A, 0x04, 0x01, 0x02, 0xA6, 0x01                // Location Beacon
};
uint8_t payload_size = (sizeof(payload)/sizeof(uint8_t));

typedef enum {
  SPLASH,
  TOP_MENU,
  MODE_SCANNER,
  MODE_SCANNER_SCANNING,
  MODE_SCANNER_RESULTS,
  MODE_BEACON,
  MODE_BEACON_OFF,
  MODE_BEACON_ON
} system_state;

system_state state = SPLASH;

TFT_eSPI tft = TFT_eSPI();    // display interface
bool tft_update = true;       // flag to inidcate display needs to be updated

uint8_t selected_item = 0;

const char msg_HELLO[] = "HELLO";

const char msg_title[] = "Droid Toolbox";
const char msg_email[] = "ruthsarian@gmail.com";
const char msg_continue1[] = "press any button";
const char msg_continue2[] = "to continue...";

const char msg_select[] = "select an option";

const char msg_beacon[] = "BEACON";
const char msg_beacon_off[] = "OFF";
const char msg_beacon_on[] = "ON";

const char msg_scanner[] = "SCANNER";
const char msg_scanner_active[] = "SCANNING";
const char msg_scanner_results[] = "DROID REPORT";

void updateDisplay() {

  uint16_t y = 0;
  uint16_t w = 0;
  uint8_t i;

  if (tft_update != true) {
    return;
  }

  // reset screen
  tft.fillScreen(TFT_BLACK);

  switch (state) {

    case MODE_BEACON:
    case MODE_BEACON_ON:
    case MODE_BEACON_OFF:
      tft.setTextSize(5);
      tft.setTextColor(TFT_BLUE);
      y = (tft.height() / 2) - tft.fontHeight() - 5;
      tft.setCursor((tft.width() / 2) - (tft.textWidth(msg_beacon) / 2), y);
      tft.print(msg_beacon);
      y += tft.fontHeight() + 10;

      if (state == MODE_BEACON_ON) {
        tft.setTextColor(TFT_GREEN);
        tft.setCursor((tft.width() / 2) - (tft.textWidth(msg_beacon_on) / 2), y);
        tft.print(msg_beacon_on);
      } else {
        tft.setTextColor(TFT_DARKGREEN);
        tft.setCursor((tft.width() / 2) - (tft.textWidth(msg_beacon_off) / 2), y);
        tft.print(msg_beacon_off);
      }
      break;

    case MODE_SCANNER_RESULTS:
      y = 0;

      tft.setTextSize(3);
      tft.setTextColor(TFT_RED);
      tft.setCursor((tft.width() / 2) - (tft.textWidth(msg_scanner_results) / 2), y);
      tft.print(msg_scanner_results);
      y += (tft.fontHeight()*2);

      tft.setCursor(0, y);
      if (droid_count > 0) {
        for(i=0; i<droid_count; i++) {
          tft.setTextSize(2);
          tft.setTextColor(TFT_BLUE);
          tft.println(droids[i].addr);
          tft.setTextColor(TFT_GREEN);

          switch(droids[i].affid) {
            case 1:
              tft.print("Scoundrel");
              break;
            case 3:
              tft.print("Ruthsarian");
              break;
            case 5:
              tft.print("Resistance");
              break;
            case 9:
              tft.print("First Order");
              break;
            default: 
              tft.print("Unknown (");
              tft.print(droids[i].affid);
              tft.print(")");
              break;    
          }
          tft.print(" - ");
          switch(droids[i].chipid) {
            case 1:
              tft.print("R Unit");
              break;
            case 2:
              tft.print("BB Unit");
              break;
            case 3:
              tft.print("Blue");
              break;
            case 4:
              tft.print("Gray");
              break;
            case 5:
              tft.print("Red");
              break;
            case 6:
              tft.print("Orange");
              break;
            case 7:
              tft.print("Purple");
              break;
            case 8:
              tft.print("Black");
              break;
            case 9:
              tft.print("CB-23");
              break;
            case 11:
              tft.print("CH1-10P");
              break;
            default: 
              tft.print("Unknown (");
              tft.print(droids[i].chipid);
              tft.print(")");
              break;
          }
          tft.println();
          tft.setCursor(0, tft.getCursorY()+5);
        }
      } else {
        tft.setTextColor(TFT_YELLOW);
        tft.println("No Droids In Area");
      }

      break;

    case MODE_SCANNER:
      tft.setTextSize(5);
      tft.setTextColor(TFT_GREEN);
      tft.setCursor((tft.width() / 2) - (tft.textWidth(msg_scanner_active) / 2), (tft.height() / 2) - (tft.fontHeight() / 2));
      tft.print(msg_scanner_active);
      state = MODE_SCANNER_SCANNING;
      break;

    case TOP_MENU:

      // surely there's an easier way to vertically position the splash screen text
      y = 0;
      tft.setTextSize(2);
      y += (tft.fontHeight()*2);
      tft.setTextSize(4);
      y += ((tft.fontHeight() + 10) * 2);
      y = (tft.height() / 2) - (y/2);

      // how wide to make the select box
      w = tft.textWidth(msg_scanner) + 20;

      // display 'select an option' message
      tft.setTextSize(2);
      tft.setTextColor(TFT_LIGHTGREY);
      tft.setCursor((tft.width() / 2) - (tft.textWidth(msg_select) / 2), y);
      tft.print(msg_select);
      y += (tft.fontHeight()*2);

      // display SCANNER option
      y += 5;
      tft.setTextSize(4);
      if (selected_item == MODE_SCANNER) {
        tft.setTextColor(TFT_GREEN);
      } else {
        tft.setTextColor(TFT_DARKGREEN);
      }
      tft.setCursor((tft.width() / 2) - (tft.textWidth(msg_scanner) / 2), y);
      tft.print(msg_scanner);
      y += tft.fontHeight() + 5;

      // display BEACON option
      y += 5;
      tft.setTextSize(4);
      if (selected_item == MODE_BEACON) {
        tft.setTextColor(TFT_GREEN);
      } else {
        tft.setTextColor(TFT_DARKGREEN);
      }
      tft.setCursor((tft.width() / 2) - (tft.textWidth(msg_beacon) / 2), y);
      tft.print(msg_beacon);

      // draw box around selected option
      y -= 6;
      if (selected_item == MODE_SCANNER) {
        y -= (tft.fontHeight() + 11);
      }
      tft.drawRect((tft.width() / 2) - (w / 2), y, w, tft.fontHeight() + 10, TFT_YELLOW);

      break;

    case SPLASH:

      // surely there's an easier way to vertically position the splash screen text
      y = 0;
      tft.setTextSize(3);
      y += tft.fontHeight();
      tft.setTextSize(2);
      y += (tft.fontHeight()*4);
      y = (tft.height() / 2) - (y/2);

      // title
      tft.setTextSize(3);
      tft.setTextColor(TFT_RED);
      tft.setCursor((tft.width() / 2) - (tft.textWidth(msg_title) / 2), y);
      tft.print(msg_title);
      y += tft.fontHeight();

      // contact
      tft.setTextSize(2);
      tft.setTextColor(TFT_ORANGE);
      tft.setCursor((tft.width() / 2) - (tft.textWidth(msg_email) / 2), y);
      tft.print(msg_email);
      y += (tft.fontHeight() * 2);

      // press any button...
      tft.setTextColor(TFT_LIGHTGREY);
      tft.setCursor((tft.width() / 2) - (tft.textWidth(msg_continue1) / 2), y);
      tft.print(msg_continue1);
      y += tft.fontHeight();
      tft.setCursor((tft.width() / 2) - (tft.textWidth(msg_continue2) / 2), y);
      tft.print(msg_continue2);
      break;
  }

  tft_update = false;
}


// BLE Callback
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      uint8_t *rawdata, rawdata_len, i;
      uint16_t mfid;
      char tmp[20];

      // DROID CHECK ONE 
      //   - we haven't maxed our droid cound
      //   - object has the name "DROID"
      //   - object has manfuacturer data
      if (droid_count < MAX_DROIDS && advertisedDevice.getName() == "DROID" && advertisedDevice.haveManufacturerData()) {

        // exract manufacturer's id from data
        rawdata = (uint8_t*)advertisedDevice.getManufacturerData().data();
        rawdata_len = advertisedDevice.getManufacturerData().length();
        mfid = rawdata[0] + (rawdata[1]<<8);

        // DROID CHECK TWO
        //   - is manufacturer id 0x0183 (Disney)
        if (rawdata_len == 8 && mfid == 0x0183) {

          // get the (unique) address of the device
          strncpy(tmp, advertisedDevice.getAddress().toString().c_str(), 20);

          // loop through the droids we already know about and make sure we don't already know about THIS droid
          for (i=0;i<droid_count;i++) {

            // if we already know this droid, exit the function
            if (memcmp(droids[i].addr, tmp, 20) == 0) {
              return;
            }
          }

          // store found droid's information
          droids[droid_count].chipid = rawdata[rawdata_len-1];
          droids[droid_count].affid = (rawdata[rawdata_len-2] - 0x80) / 2;
          strncpy(droids[droid_count].addr, tmp, 20);
          droids[droid_count].rssi = advertisedDevice.getRSSI();

          // increment counter
          droid_count++;
        }
      }
    }
};

void ble_scan() {
  // get ready to count some droids
  droid_count = 0;

  // BLE scan
  BLEScanResults foundDevices = pBLEScan->start(BLE_SCAN_TIME, false);

  // Report results via Serial
  Serial.print(foundDevices.getCount());
  Serial.println(" BLE devices seen.");

  // delete results fromBLEScan buffer to release memory
  pBLEScan->clearResults();
}

// ISR for button 1 press
void IRAM_ATTR button1() {
  static uint32_t last_time_btn1 = 0;
  if (millis() - last_time_btn1 > LAZY_DEBOUNCE) {

    switch(state) {
      case SPLASH:
        state = TOP_MENU;
        tft_update = true;
        selected_item = MODE_SCANNER;
        break;
      case TOP_MENU:
        if (selected_item == MODE_SCANNER) {
          state = MODE_SCANNER;
        } else {
          state = MODE_BEACON;
        }
        tft_update = true;
        break;
      case MODE_SCANNER_RESULTS:
        state = MODE_SCANNER;
        tft_update = true;
        break;
      case MODE_BEACON:
      case MODE_BEACON_OFF:
        pAdvertising->start();
        state = MODE_BEACON_ON;
        tft_update = true;
        break;
      case MODE_BEACON_ON:
        pAdvertising->stop();
        state = MODE_BEACON_OFF;
        tft_update = true;
        break;
    }

    last_time_btn1 = millis();
  }
}

// ISR for button 2 press
void IRAM_ATTR button2() {
  static uint32_t last_time_btn2 = 0;
  if (millis() - last_time_btn2 > LAZY_DEBOUNCE) {

    // do button 2 stuff
    switch(state) {
      case SPLASH:
        state = TOP_MENU;
        tft_update = true;
        selected_item = MODE_SCANNER;
        break;

      case TOP_MENU:
        if (selected_item == MODE_SCANNER) {
          selected_item = MODE_BEACON;
        } else {
          selected_item = MODE_SCANNER;
        }
        tft_update = true;
        break;
    }

    last_time_btn2 = millis();
  }
}

void setup() {
  // setup display
  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(3);

  // setup buttons
  pinMode(BUTTON1_PIN, INPUT);
  pinMode(BUTTON2_PIN, INPUT);
  attachInterrupt(BUTTON1_PIN, button1, RISING);
  attachInterrupt(BUTTON2_PIN, button2, RISING);

  // init bluetooth
  BLEDevice::init("");

  // setup BLE scanner
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value

  // setup BLE advertising (beacon)
  oAdvertisementData.setManufacturerData(std::string(reinterpret_cast<char*>(payload), payload_size));
  oAdvertisementData.setName("DROIDBOX");
  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->setAdvertisementData(oAdvertisementData);
  pAdvertising->setScanResponseData(oScanResponseData);

  // init serial monitor
  Serial.begin(115200);
  Serial.println("Ready!");
}

void loop() {

  switch (state) {
    case MODE_SCANNER_SCANNING:
      ble_scan();
      state = MODE_SCANNER_RESULTS;
      tft_update = true;
      break;
    default:
      break;
  }

  updateDisplay();
}
