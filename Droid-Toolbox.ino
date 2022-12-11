/* Droid Toolbox v0.3 : ruthsarian@gmail.com
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
 * Required Boards
 *  Arduino ESP32 core: https://github.com/espressif/arduino-esp32
 *    1. add https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json to the additional board manager URLs in the preferences window
 *    2. go to boards manager
 *    3. search for and install esp32
 * 
 * Required Libraries
 *  Arduino TFT_eSPI library: https://github.com/Bodmer/TFT_eSPI
 *  
 *    !! NOTICE !! 
 *    
 *    After installing or updating the TFT_eSPI library you MUST edit User_Setup_Select.h as follows 
 *      1. comment out the line "#include <User_Setup.h>" (line 22-ish)
 *      2. uncomment the line "#include <User_Setups/Setup25_TTGO_T_Display.h>" (line 53-ish)
 *      
 *    Possible path for Windows users: %USERPROFILE%\Documents\Arduino\libraries\TFT_eSPI\User_Setup_Select.h
 *
 * A BLE library is included in the Arduino ESP32 core. If you have ArduinoBLE already installed you will need
 * to temporarily uninstall it in order for this code to compile correctly.
 *
 * Arduino IDE Board Configuration (defaults)
 *  Board: ESP32 Dev Module
 *  Upload Speed: 921600
 *  CPU Freq: 240MHz (WiFI/BT)
 *  Flash Freq: 80MHz
 *  Flash Mode: QIO
 *  Flash Size: 4MB (32Mb)
 *  Partition Scheme: Default 4MB with spiffs
 *  Core Debug Level: None
 *  PSRAM: Disabled
 * 
 * References
 *   Arduino IDE setup: https://www.youtube.com/watch?v=b8254--ibmM
 *   TTGO pinout: https://i.redd.it/1usgojazvq561.jpg
 *   Programming the bin w/o Arduino IDE: https://www.aranacorp.com/en/generating-and-uploading-bin-files-for-esp32/
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
  *   keep text size to no smaller than 2
 *
 * TODO
 *   beacon:
 *     allow for beacon customization
 *       choose between droid and location beacons
 *       droid beacon:
 *         set affiliation
 *         set personality chip
 *       location beacon:
 *         set location
 *         set minimum reaction interval 
 *   scanner:
 *     ability to connect to a single droid and send 'raw' commands
 *        play specific track from specific audio group
 *        control LEDs (?)
 *        control motors (is this a GOOD idea? probably not...)
 *        other ??
 *   some kind of 'sleep' feature for power saving? would that even be useful?
 *   screensaver ?
 *
 * HISTORY
 *   v0.3 : Long/Short button press detection
 *          Droid report is paged; shows 1 droid at a time
 *          Droid report sorts droids by RSSI value
 *          Added version to splash screen
 *   v0.2 : Added back button from both beacon and scanner.
 *          Location beacon location is randomly selected.
 *   v0.1 : Initial Release
 */

#include <SPI.h>
#include <TFT_eSPI.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#define MSG_VERSION       "v0.3"

#define BUTTON1_PIN       0
#define BUTTON2_PIN       35
#define LAZY_DEBOUNCE     10  // time to wait after a button press before considering it a good press
#define SHORT_PRESS_TIME  500 // maximum time, in milliseconds, that a button can be held before release and be considered a SHORT press

#define MAX_DROIDS        20  // maximum number of droids to report on
#define BLE_SCAN_TIME     5   // how many seconds to scan

#define PAYLOAD_SIZE      8   // size, in bytes, of a beacon payload
#define MSG_LEN_MAX       16
#define DROID_ADDR_LEN    20

BLEScan* pBLEScan;
BLEAdvertising* pAdvertising;
BLEAdvertisementData oScanResponseData = BLEAdvertisementData();
BLEAdvertisementData* pAdvertisementData = NULL;   // must be pointer so i can delete class then recreate it every time beacon changes
                                                   // should i be using smart pointers?

typedef struct droid_t {
    uint8_t chipid;
    uint8_t affid;
    int rssi;
    char addr[DROID_ADDR_LEN];
    //BLEAddress address;
} Droid;

Droid droids[MAX_DROIDS];
uint8_t droid_count = 0;
uint8_t current_droid = 0;

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

/*
const uint8_t SWGE_BEACON_NAME[] = {
  0x44, 0x54, 0x4F, 0x49, 0x44   // 'DROID'
};
*/

uint8_t payload[PAYLOAD_SIZE];

typedef enum {
  SPLASH,
  TOP_MENU,
  MODE_SCANNER,
  MODE_SCANNER_SCANNING,
  MODE_SCANNER_RESULTS,
  MODE_SCANNER_CONNECTING,
  MODE_SCANNER_CONNECTED,
  MODE_BEACON,
  MODE_BEACON_OFF,
  MODE_BEACON_ON
} system_state_t;

system_state_t state = SPLASH;

TFT_eSPI tft = TFT_eSPI();    // display interface
bool tft_update = true;       // flag to inidcate display needs to be updated

uint8_t selected_item = 0;

const char msg_title[] = "Droid Toolbox";
const char msg_email[] = "ruthsarian@gmail.com";
const char msg_continue1[] = "press any button";
const char msg_continue2[] = "to continue...";
const char msg_version[] = MSG_VERSION;
const char msg_select[] = "select an option";
const char msg_beacon[] = "BEACON";
const char msg_beacon_off[] = "OFF";
const char msg_beacon_on[] = "ON";
const char msg_scanner[] = "SCANNER";
const char msg_scanner_active[] = "SCANNING";
const char msg_scanner_results[] = "DROID REPORT";

void tft_println_center(const char *msg) {
  tft.setCursor((tft.width() / 2) - (tft.textWidth(msg) / 2), tft.getCursorY());
  tft.println(msg);
}

void display_scanner_results() {
  char msg[MSG_LEN_MAX];
  uint16_t y = 0;
  uint8_t i;

  // display header
  tft.setTextSize(2);
  tft.setTextColor(TFT_DARKGREY);
  tft_println_center(msg_scanner_results);

  // find where to start printing droid details so that it is vertically centered
  y = tft.fontHeight();
  tft.setTextSize(2);
  y += (tft.fontHeight() * 3);
  tft.setCursor(0, (tft.height()/2) - (y/2));

  // add a gap after the header
  if (droid_count > 0) {

    // print droid personality
    tft.setTextSize(3);
    tft.setTextColor(TFT_RED);
    switch(droids[current_droid].chipid) {
      case 1:
        tft_println_center("R Unit");
        break;
      case 2:
        tft_println_center("BB Unit");
        break;
      case 3:
        tft_println_center("Blue");
        break;
      case 4:
        tft_println_center("Gray");
        break;
      case 5:
        tft_println_center("Red");
        break;
      case 6:
        tft_println_center("Orange");
        break;
      case 7:
        tft_println_center("Purple");
        break;
      case 8:
        tft_println_center("Black");
        break;
      case 9:
        tft_println_center("CB-23");
        break;
      case 10:
        tft_println_center("Yellow");
        break;
      case 11:
        tft_println_center("CH1-10P");
        break;
      case 13:
        tft_println_center("Blue");
        break;
      default:
        snprintf(msg, MSG_LEN_MAX, "Unknown (%d)", droids[current_droid].chipid);
        tft_println_center(msg);
        break;
    }

    // print droid affiliation
    tft.setTextSize(2);
    tft.setTextColor(TFT_GREEN);
    switch(droids[current_droid].affid) {
      case 1:
        tft_println_center("Scoundrel");
        break;
      case 3:
        tft_println_center("Ruthsarian");
        break;
      case 5:
        tft_println_center("Resistance");
        break;
      case 9:
        tft_println_center("First Order");
        break;
      default:
        snprintf(msg, MSG_LEN_MAX, "Unknown (%d)", droids[current_droid].affid);
        tft_println_center(msg);
        break;
    }

    // print Bluetooth MAC address
    tft.setTextSize(2);
    tft.setTextColor(TFT_BLUE);
    tft_println_center(droids[current_droid].addr);

    // print RSSI
    tft.setTextSize(2);
    tft.setTextColor(TFT_PURPLE);
    snprintf(msg, MSG_LEN_MAX, "rssi: %ddBm", droids[current_droid].rssi);
    tft_println_center(msg);

    // print 
    snprintf(msg, MSG_LEN_MAX, "%d of %d", current_droid + 1, droid_count);
    tft.setTextSize(2);
    tft.setTextColor(TFT_DARKGREY);
    tft.setCursor((tft.width() - tft.textWidth(msg))/2, tft.height() - tft.fontHeight());
    tft.print(msg);

  } else {
    tft.setTextColor(TFT_YELLOW);
    tft_println_center("No Droids");
    tft_println_center("In Area");
  }
}

// display the menu where the user selects between beacon and scanner
void display_top_menu() {
  uint16_t y = 0;
  uint16_t w = 0;

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
}

// display the splash screen seen when the program starts
void display_splash() {
  uint16_t y = 0;
  
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
  
  // version
  tft.setTextColor(TFT_DARKGREY);
  tft.setCursor(tft.width() - tft.textWidth(msg_version), tft.height() - tft.fontHeight());
  tft.print(msg_version);
}


void update_display() {

  uint16_t y = 0;

  if (tft_update != true) {
    return;
  }

  // reset screen
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);

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
      display_scanner_results();
      break;

    case MODE_SCANNER:
      tft.setTextSize(5);
      tft.setTextColor(TFT_GREEN);
      tft.setCursor((tft.width() / 2) - (tft.textWidth(msg_scanner_active) / 2), (tft.height() / 2) - (tft.fontHeight() / 2));
      tft.print(msg_scanner_active);
      state = MODE_SCANNER_SCANNING;
      break;

    case TOP_MENU:
      display_top_menu();
      break;

    case SPLASH:
      display_splash();
      break;
  }

  tft_update = false;
}

// BLE Callback
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    uint8_t *rawdata, rawdata_len, i, pos;
    uint16_t mfid;
    char tmp[20];
  
    // do not add this object if it doesn't have the name DROID or if it does not have manufacturer data
    if (advertisedDevice.getName() != "DROID" || !advertisedDevice.haveManufacturerData()) {
      return;
    }
  
    // exract manufacturer's id from device's manufacturer data
    rawdata = (uint8_t*)advertisedDevice.getManufacturerData().data();
    rawdata_len = advertisedDevice.getManufacturerData().length();
    mfid = rawdata[0] + (rawdata[1]<<8);

    // do not add this device if it does not have a manufacturer's id of 0x0183 (Disney)
    if (rawdata_len != 8 || mfid != 0x0183) {
      return;
    }

    Serial.print("Found DROID: ");
    Serial.print(advertisedDevice.getAddress().toString().c_str());
    Serial.print(", RSSI: ");
    Serial.println(advertisedDevice.getRSSI());

    // the droid list is sorted from strongest to weakest signal strength (RSSI) value
    // locate where in the list this new droid will be inserted
    pos = 0;
    if (droid_count > 0) {

      // find where to insert droid into list; higher RSSI come first in list
      for (pos=0;pos<droid_count;pos++) {
        if (droids[pos].rssi < advertisedDevice.getRSSI()) {
          break;
        }
      }

      // this droid's signal strength is too weak, do not insert it into the list
      if (droid_count == MAX_DROIDS && pos == MAX_DROIDS) {
        return;
      }

      // push droids with a lower RSSI down the list
      for (i = droid_count;i>pos;i--) {
        if (i < MAX_DROIDS) {
          droids[i] = droids[i-1];
        }
      }
    }

    // store found droid's information
    droids[pos].chipid = rawdata[rawdata_len-1];
    droids[pos].affid = (rawdata[rawdata_len-2] - 0x80) / 2;
    strncpy(droids[pos].addr, advertisedDevice.getAddress().toString().c_str(), DROID_ADDR_LEN);
    droids[pos].rssi = advertisedDevice.getRSSI();

    // increment counter
    if (droid_count < MAX_DROIDS) {
      droid_count++;
    }
  }
};

//
// TODO
//  review: https://circuitdigest.com/microcontroller-projects/ble-based-proximity-control-using-esp32
//  probably need to revisit how we scan and process results 
//
//
// BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
//
// but is it useful? BLEScanResults only has a count and an index function? hmm...
// see: https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/BLEScan.h
//


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

typedef enum {
  SHORT_PRESS,
  LONG_PRESS
} button_press_t;

void button1(button_press_t press_type);    // trying to use an enum as a parameter triggers a bug in arduino. adding an explicit prototype resolves the issue.
void button1(button_press_t press_type) {

  static uint32_t last_time_btn1 = 0;
  static uint32_t last_time_btn1_down = 0;

  if (press_type == LONG_PRESS) {
    Serial.println("BUTTON 1 PRESS: LONG");
  } else {
    Serial.println("BUTTON 1 PRESS: SHORT");
  }

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
      if (press_type == SHORT_PRESS && droid_count > 0) {
        current_droid = (current_droid + 1) % droid_count;
      } else {
        state = MODE_SCANNER;
      }
      tft_update = true;
      break;
    case MODE_BEACON:
    case MODE_BEACON_OFF:
      init_advertisement_data();
      set_payload_location_beacon(esp_random());
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
}

void button2(button_press_t press_type);    // trying to use an enum as a parameter triggers a bug in arduino. adding an explicit prototype resolves the issue.
void button2(button_press_t press_type) {

  static uint32_t last_time_btn2 = 0;
  static uint32_t last_time_btn2_down = 0;

  if (press_type == LONG_PRESS) {
    Serial.println("BUTTON 2 PRESS: LONG");
  } else {
    Serial.println("BUTTON 2 PRESS: SHORT");
  }

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

    case MODE_BEACON:
    case MODE_BEACON_ON:
    case MODE_BEACON_OFF:
    case MODE_SCANNER_RESULTS:
      pAdvertising->stop();
      state = TOP_MENU;
      tft_update = true;
      break;
  }
}

void button_handler() {
  static uint32_t last_btn1_time = 0;
  static uint8_t  last_btn1_state = HIGH;
  static uint32_t last_btn2_time = 0;
  static uint8_t  last_btn2_state = HIGH;

  // gather current state of things
  uint8_t  now_btn1_state = digitalRead(BUTTON1_PIN);
  uint8_t  now_btn2_state = digitalRead(BUTTON2_PIN);
  uint32_t now_time = millis();

  if (now_btn1_state != last_btn1_state && now_time - last_btn1_time > LAZY_DEBOUNCE) {
    if (now_btn1_state == HIGH) {
      if (now_time - last_btn1_time > SHORT_PRESS_TIME) {
        button1(LONG_PRESS);
      } else {
        button1(SHORT_PRESS);
      }
    }
    last_btn1_state = now_btn1_state;
    last_btn1_time = now_time;
  }

  if (now_btn2_state != last_btn2_state && now_time - last_btn2_time > LAZY_DEBOUNCE) {
    if (now_btn2_state == HIGH) {
      if (now_time - last_btn2_time > SHORT_PRESS_TIME) {
        button2(LONG_PRESS);
      } else {
        button2(SHORT_PRESS);
      }
    }
    last_btn2_state = now_btn2_state;
    last_btn2_time = now_time;
  }
}

void load_payload_location_beacon_data() {
  memcpy(payload, SWGE_LOCATION_BEACON_PAYLOAD, sizeof(uint8_t) * PAYLOAD_SIZE);
}

void load_payload_droid_beacon_data() {
  memcpy(payload, SWGE_DROID_BEACON_PAYLOAD, sizeof(uint8_t) * PAYLOAD_SIZE);
}

void set_payload_droid_beacon() {
  load_payload_droid_beacon_data();
}

void init_advertisement_data() {
  if (pAdvertisementData != NULL) {
    delete pAdvertisementData;
  }
  pAdvertisementData = new BLEAdvertisementData();
  pAdvertisementData->setName("DROIDBOX");
}

void set_payload_location_beacon(uint8_t location) {
  load_payload_location_beacon_data();
  payload[4] = (location % 7) + 1;
  pAdvertisementData->setManufacturerData(std::string(reinterpret_cast<char*>(payload), PAYLOAD_SIZE));
  pAdvertising->setAdvertisementData(*pAdvertisementData);
}

void setup() {
  // setup display
  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(3);

  // setup buttons as input
  pinMode(BUTTON1_PIN, INPUT);
  pinMode(BUTTON2_PIN, INPUT);

  // init bluetooth
  BLEDevice::init("");

  // setup BLE scanner
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster
  pBLEScan->setInterval(100);
  pBLEScan->setWindow(99);  // less or equal setInterval value

  // setup BLE advertising (beacon)
  pAdvertising = BLEDevice::getAdvertising();
  pAdvertising->setScanResponseData(oScanResponseData);

  // setup beacon payload
  init_advertisement_data();
  set_payload_location_beacon(esp_random());

  // init serial monitor
  Serial.begin(115200);
  Serial.println("Ready!");
}

void loop() {

  button_handler();

  switch (state) {
    case MODE_SCANNER_SCANNING:
      ble_scan();
      current_droid = 0;
      state = MODE_SCANNER_RESULTS;
      tft_update = true;
      break;
    default:
      break;
  }

  update_display();
}
