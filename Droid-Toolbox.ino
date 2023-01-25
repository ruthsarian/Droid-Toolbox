/* Droid Toolbox v0.53 : ruthsarian@gmail.com
 * 
 * A program to work with droids from the Droid Depot at Galaxy's Edge.
 * NOTE: your droid remote MUST BE OFF for this to work!
 * 
 * Features
 *   - Scan for nearby droids
 *   - Generate location and droid beacons
 *   - Control audio produced by droids
 * 
 * Designed to be used with a LilyGO TTGO T-Display or LilyGO T-Display-S3 which are ESP32-based modules with an LCD display, although
 * it should work with any ESP32 module and some small code changes.
 *   see: https://www.amazon.com/dp/B099MPFJ9M (TTGO T-Display)
 *        https://www.amazon.com/dp/B0BF542H39 (T-Display-S3)
 * 
 * Required Boards
 *  Arduino ESP32 core: https://github.com/espressif/arduino-esp32
 *    1. add https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json to the additional board manager URLs in the preferences window
 *    2. go to boards manager
 *    3. search for and install esp32
 *    4. when compiling, select either the ESP32 Dev Module (for T-Display) or ESP32S3 Dev Module (for T-Display-32)
 * 
 * Requires the TFT_eSPI Library
 *  see: https://github.com/Bodmer/TFT_eSPI
 *  
 *  NOTE 1: 
 *    the T-Display-S3, as of writing, requires a modified copy of TFT_eSPI which you can get
 *    at https://github.com/Xinyuan-LilyGO/T-Display-S3
 *  
 *  NOTE 2: 
 *    After installing or updating the TFT_eSPI library you MUST edit User_Setup_Select.h as follows 
 *      1. comment out the line "#include <User_Setup.h>" (line 22-ish)
 *      2. uncomment the line "#include <User_Setups/Setup25_TTGO_T_Display.h>" (line 61-ish) for T-Display
 *         or "#include <User_Setups/Setup206_LilyGo_T_Display_S3.h>" for the T-Display-32
 *      
 *    Possible path for Windows users: %USERPROFILE%\Documents\Arduino\libraries\TFT_eSPI\User_Setup_Select.h
 *
 * A BLE library is included in the Arduino ESP32 core. If you have ArduinoBLE already installed you will need
 * to uninstall it in order for this code to compile correctly. To uninstall a library locate your arduino 
 * libraries folder and delete the ArduinoBLE folder.
 *
 * TTGO T-Display Board Configuration (defaults)
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
 * T-Display-S3 Board Configuration (defaults)
 *  Board: ESP32S3 Dev Module
 *  Upload Speed: 921600
 *  CPU Freq: 240MHz (WiFI/BT)
 *  Flash Freq: 80MHz
 *  Flash Mode: QIO
 *  Flash Size: 16MB (128Mb)
 *  Partition Scheme: Huge App (3MB No OTA/1MB SPIFFS)
 *  Core Debug Level: None
 *  PSRAM: OPI PSRAM 
 *   
 * References
 *   Arduino IDE setup: https://www.youtube.com/watch?v=b8254--ibmM
 *   TTGO T-Display Github Repository: https://github.com/Xinyuan-LilyGO/TTGO-T-Display
 *   T-Display-S3 Github Repository: https://github.com/Xinyuan-LilyGO/T-Display-S3
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
 *     search for specific droid/location
 *        constant scan; alert when encountering the specified beacon
 *        maybe proximity alert too? getting closer or farther away?
 *     ability to scan for location beacons
 *        based on RSSI, identify current 'zone'
 *        identify nearby zones 
 *        maybe a constant-scan mode that alerts when the rare zones appear (4, 5)
 *     ability to connect to a single droid and send 'raw' commands
 *        play specific track from specific audio group
 *        control LEDs (?)
 *        control motors (is this a GOOD idea? probably not...)
 *        other ??
 *   sleep/wake system to conserve power
 *
 * HISTORY
 *   v0.53 : Added support for T-Display-S3 devices
 *           T-Display-S3 currently requires a modified version of TFT_eSPI which you can get from the T-Display-S3
 *           github repository here: https://github.com/Xinyuan-LilyGO/T-Display-S3 under the lib directory
 *   v0.52 : Fixed typo "CH1-10P" => "C1-10P"
 *   v0.51 : Put BLE notifications back into the code. Any notifications received are displayed in the serial monitor.
 *           Added note to connecting string so people see the droid remote needs to be off before connecting
 *   v0.5  : Added ability to connect to droid from scan results using a long-press of button 1
 *           Droid plays activation sound upon connection
 *           Group and track can be selected and played through the droid.
 *   v0.4  : Added deep sleep/hibernation
 *           Added initial ability to connect to droid with long button 1 press while viewing droid in scan results
 *            Connection is currently a demo; connect, tell droid to play a sound, then disconnect. 
 *            Will improve upon this in the next version.
 *   v0.3  : Long/Short button press detection
 *           Droid report is paged; shows 1 droid at a time
 *           Droid report sorts droids by RSSI value
 *           Added version to splash screen
 *   v0.2  : Added back button from both beacon and scanner.
 *           Location beacon location is randomly selected.
 *   v0.1  : Initial Release
 */

#include <TFT_eSPI.h>
#include <SPI.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#define MSG_VERSION       "v0.53"

#ifdef ARDUINO_ESP32S3_DEV    // this is assuming you're compiling for T-Display-S3 using the "ESP32S3 Dev Module" board. 
  #define TDISPLAYS3
#endif

#define BUTTON1_PIN       0   // button 1 on the TTGO is GPIO 0 
#ifdef TDISPLAYS3             // button 2 on the TTGO is GPIO 35; GPIO14 for T-Display-S3
  #define BUTTON2_PIN     14
#else
  #define BUTTON2_PIN     35
#endif
#define LAZY_DEBOUNCE     10  // time to wait after a button press before considering it a good press
#define SHORT_PRESS_TIME  500 // maximum time, in milliseconds, that a button can be held before release and be considered a SHORT press

#define MAX_DROIDS        20  // maximum number of droids to report on
#define BLE_SCAN_TIME     5   // how many seconds to scan

#define PAYLOAD_SIZE      8   // size, in bytes, of a beacon payload
#define MSG_LEN_MAX       32
#define DROID_ADDR_LEN    20

#define SLEEP_AFTER       5 * 60 * 1000   // how many milliseconds of inactivity before going to sleep/hibernation
#define WAKEUP_BUTTON     GPIO_NUM_0      // wake up when button 1 is pressed _ONLY_IF_ it's been enabled in setup(); otherwise the reset button will wake up the TTGO
#define WAKEUP_LEVEL      LOW             // wake up from sleep when the button is pressed (LOW)

uint32_t  last_activity;

BLEUUID serviceUUID("09b600a0-3e42-41fc-b474-e9c0c8f0c801");
BLEUUID     cmdUUID("09b600b1-3e42-41fc-b474-e9c0c8f0c801");
BLEUUID  notifyUUID("09b600b0-3e42-41fc-b474-e9c0c8f0c801");    // not used, but keeping it for future reference

BLEScan* pBLEScan = nullptr;
BLEClient* pClient = nullptr;
BLERemoteService* pRemoteService = nullptr;
BLERemoteCharacteristic* pRemoteCharacteristicCmd = nullptr;
BLERemoteCharacteristic* pRemoteCharacteristicNotify = nullptr;
BLEAdvertising* pAdvertising = nullptr;
BLEAdvertisementData oScanResponseData = BLEAdvertisementData();
BLEAdvertisementData* pAdvertisementData = nullptr; // must be pointer so i can delete class then recreate it every time beacon changes
                                                    // should i be using smart pointers?

typedef struct droid_t {
    uint8_t chipid;
    uint8_t affid;
    BLEAdvertisedDevice* pAdvertisedDevice;
} Droid;

Droid droids[MAX_DROIDS];
uint8_t droid_count = 0;
uint8_t current_droid = 0;
uint8_t current_group = 0;
uint8_t current_track = 0;

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

uint8_t payload[PAYLOAD_SIZE];

typedef enum {
  SHORT_PRESS,
  LONG_PRESS
} button_press_t;

typedef enum {
  SPLASH,
  TOP_MENU,
  MODE_SCANNER,
  MODE_SCANNER_SCANNING,
  MODE_SCANNER_RESULTS,
  MODE_SCANNER_CONNECTING,
  MODE_SCANNER_CONNECT_FAILED,
  MODE_SCANNER_CONNECTED,
  MODE_SOUND_SELECT_GROUP,
  MODE_SOUND_SELECT_TRACK,
  MODE_SOUND_SELECT_PLAY,
  MODE_SOUND_PLAY,
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

  // add a gap after the header
  if (droid_count > 0) {

    // find where to start printing droid details so that it is vertically centered
    y = tft.fontHeight();
    tft.setTextSize(2);
    y += (tft.fontHeight() * 3);
    tft.setCursor(0, (tft.height()/2) - (y/2));

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
        tft_println_center("C1-10P");
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
    tft_println_center(droids[current_droid].pAdvertisedDevice->getAddress().toString().c_str());

    // print RSSI
    tft.setTextSize(2);
    tft.setTextColor(TFT_PURPLE);
    snprintf(msg, MSG_LEN_MAX, "rssi: %ddBm", droids[current_droid].pAdvertisedDevice->getRSSI());
    tft_println_center(msg);

    // print 
    snprintf(msg, MSG_LEN_MAX, "%d of %d", current_droid + 1, droid_count);
    tft.setTextSize(2);
    tft.setTextColor(TFT_DARKGREY);
    tft.setCursor((tft.width() - tft.textWidth(msg))/2, tft.height() - tft.fontHeight());
    tft.print(msg);

  // display message that no droids were found
  } else {
    tft.setTextSize(3);
    tft.setTextColor(TFT_YELLOW);
    tft.setCursor(0, (tft.height()/2) - tft.fontHeight());
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

void display_track_select() {
  char msg[MSG_LEN_MAX];
  char M[] = "M";

  uint8_t gap = 0;

  // display instruction
  tft.setTextSize(3);

  gap = (tft.height() - (tft.fontHeight() * 3))/3;

  tft.setCursor(tft.textWidth(M)*2, tft.getCursorY() + gap - 2);
  tft.setTextColor(TFT_BLUE);
  tft.print("Group: ");
  tft.setTextColor((state == MODE_SOUND_SELECT_GROUP) ? TFT_GREEN : TFT_DARKGREEN);
  snprintf(msg, MSG_LEN_MAX, "%d", (current_group + 1));
  tft.println(msg);

  tft.setCursor(tft.textWidth(M)*2, tft.getCursorY() + 5);
  tft.setTextColor(TFT_RED);
  tft.print("Track: ");
  tft.setTextColor((state == MODE_SOUND_SELECT_TRACK) ? TFT_GREEN : TFT_DARKGREEN);
  snprintf(msg, MSG_LEN_MAX, "%d", (current_track + 1));
  tft.println(msg);

  tft.setCursor(0, tft.getCursorY() + gap);
  tft.setTextColor((state == MODE_SOUND_SELECT_PLAY) ? TFT_GREEN : TFT_DARKGREEN);
  tft_println_center("PLAY");
}

void update_display() {
  uint16_t y;

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

    case MODE_SCANNER_CONNECTING:
      tft.setTextSize(2);
      tft.setTextColor(TFT_DARKGREY);
      tft_println_center("TURN OFF YOUR");
      tft_println_center("DROID REMOTE");

      tft.setTextSize(3);
      tft.setTextColor(TFT_ORANGE);
      tft.setCursor(0, (tft.height() / 2) - (tft.fontHeight()/2));
      tft_println_center("CONNECTING");
      break;

    case MODE_SCANNER_CONNECTED:
      tft.setTextSize(3);
      tft.setTextColor(TFT_GREEN);
      tft.setCursor(0, (tft.height() / 2) - (tft.fontHeight()/2));
      tft_println_center("CONNECTED");
      break;

    case MODE_SOUND_SELECT_GROUP:
    case MODE_SOUND_SELECT_TRACK:
    case MODE_SOUND_SELECT_PLAY:
    case MODE_SOUND_PLAY:
      display_track_select();
      break;

    case MODE_SCANNER_CONNECT_FAILED:
      tft.setTextSize(3);
      tft.setTextColor(TFT_RED);
      tft.setCursor(0, (tft.height() / 2) - tft.fontHeight());
      tft_println_center("CONNECT");
      tft_println_center("FAILED");
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
  if (pAdvertisementData != nullptr) {
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

// BLE Advertising Callback
class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
  void onResult(BLEAdvertisedDevice advertisedDevice) {
    uint8_t *rawdata, rawdata_len, i, pos;
    uint16_t mfid;
  
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

    // the droid list is sorted from strongest to weakest signal strength (RSSI) value
    // locate where in the list this new droid will be inserted
    pos = 0;
    if (droid_count > 0) {

      // find where to insert droid into list; higher RSSI come first in list
      for (pos=0;pos<droid_count;pos++) {
        if (droids[pos].pAdvertisedDevice->getRSSI() < advertisedDevice.getRSSI()) {
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
          droids[i] = droids[i-1];                  // move droid to new position in array
          droids[i-1].pAdvertisedDevice = nullptr;     // clear the previous position in prep for the new droid
        } else {
          delete droids[i-1].pAdvertisedDevice;
        }
      }
    }

    // store found droid's information
    droids[pos].chipid = rawdata[rawdata_len-1];
    droids[pos].affid = (rawdata[rawdata_len-2] - 0x80) / 2;

    // need to store a pointer to an AdvertisedDevice object for this device because
    // it's the only thing i can use to connect to the droid successfully

    // first check to make sure there isn't already a pointer and if there is delete it
    if ( droids[pos].pAdvertisedDevice != nullptr ) {
      Serial.println("delete old AdvertsidedDevice object");
      delete droids[pos].pAdvertisedDevice;
    }

    // store the droid's AdvertsiedDevice object
    droids[pos].pAdvertisedDevice = new BLEAdvertisedDevice(advertisedDevice);

    Serial.print("Droid Found -- BD_ADDR: ");
    Serial.print(droids[pos].pAdvertisedDevice->getAddress().toString().c_str());
    Serial.print(", RSSI: ");
    Serial.println(advertisedDevice.getRSSI());

    // increment counter
    if (droid_count < MAX_DROIDS) {
      droid_count++;
    }
  }
};

void notifyCallback(
  BLERemoteCharacteristic* pBLERemoteCharacteristic,
  uint8_t* pData,
  size_t length,
  bool isNotify) {
    uint8_t i, j; 

    Serial.print("Notify callback for characteristic: ");
    Serial.print(pBLERemoteCharacteristic->getUUID().toString().c_str());
    Serial.print(" of data length ");
    Serial.println(length);
    Serial.print("data: ");

    // Serial.println((char*)pData);
    for(i=0;i<length;i++) {

      // print hex value
      Serial.print(pData[i], HEX);
      Serial.print(" ");

      // newline
      if ((i % 16 == 0 && i > 0) || (i == (length - 1))) {
        if (i == (length - 1)) {
          j = i - (i % 16);
        } else {
          j = i - 16;
        }
        Serial.print("    [ ");
        for(;j<i;j++) {
          Serial.print((char)pData[j]);
        }
        Serial.println(" ]");
      }
    }
    Serial.println();
}

void droid_disconnect() {
  // disconnect from any previous connection 
  if (pClient != nullptr && pClient->isConnected()) {
    Serial.println("Disconnecting from droid.");
    pClient->disconnect();
  }
}

bool droid_connect() {
  uint8_t login_value[] = {0x22, 0x20, 0x01};
  uint8_t cmd_a[] = {0x27, 0x42, 0x0f, 0x44, 0x44, 0x00, 0x1f, 0x07};
  uint8_t cmd_b[] = {0x27, 0x42, 0x0f, 0x44, 0x44, 0x00, 0x18, 0x00};

  // end any current connection
  droid_disconnect();

  // create a new connection
  Serial.print("Connecting to ");
  Serial.println(droids[current_droid].pAdvertisedDevice->getAddress().toString().c_str());
  if (!pClient->connect( droids[current_droid].pAdvertisedDevice )) {
    Serial.println("Connection failed.");
    return false;
  }

  // locate the service we want to talk to
  pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    Serial.println("Failed to find droid service.");
    droid_disconnect();
    return false;
  }

  // get the characteristic that receives commands
  pRemoteCharacteristicCmd = pRemoteService->getCharacteristic(cmdUUID);
  if (pRemoteCharacteristicCmd == nullptr) {
    Serial.println("Failed to find droid chracteristic.");
    droid_disconnect();
    return false;
  }
  pRemoteCharacteristicCmd->registerForNotify(notifyCallback);

  // get the characteristic that receives notifications from the droid
  pRemoteCharacteristicNotify = pRemoteService->getCharacteristic(notifyUUID);
  if (pRemoteCharacteristicNotify == nullptr) {
    Serial.println("Failed to find droid chracteristic.");
    droid_disconnect();
    return false;
  }
  pRemoteCharacteristicNotify->registerForNotify(notifyCallback);

  // send command to 'login'; will also disable droid's beacon until disconnect()
  pRemoteCharacteristicCmd->writeValue(login_value, sizeof(login_value));
  delay(100);
  pRemoteCharacteristicCmd->writeValue(login_value, sizeof(login_value));
  delay(100);

  // make droid play activation noise
  pRemoteCharacteristicCmd->writeValue(cmd_a, sizeof(cmd_a));
  delay(100);
  pRemoteCharacteristicCmd->writeValue(cmd_b, sizeof(cmd_b));
  delay(100);

  return true;
}

void droid_play_track() {
  uint8_t cmd_set_group[]  = {0x27, 0x42, 0x0f, 0x44, 0x44, 0x00, 0x1f, 0x07};
  uint8_t cmd_play_track[] = {0x27, 0x42, 0x0f, 0x44, 0x44, 0x00, 0x18, 0x00};

  if (pClient->isConnected() && pRemoteCharacteristicCmd != nullptr) {
    cmd_set_group[7]  = current_group % 12;
    cmd_play_track[7] = current_track & 0xFF;

    pRemoteCharacteristicCmd->writeValue(cmd_set_group, sizeof(cmd_set_group));
    delay(100);
    pRemoteCharacteristicCmd->writeValue(cmd_play_track, sizeof(cmd_play_track));
    delay(100);
  }
}

void ble_scan() {

  // if we have any connections, disconnect.
  droid_disconnect();

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
      if (droid_count > 0) {
        if (press_type == SHORT_PRESS) {
          current_droid = (current_droid + 1) % droid_count;
        } else {
          state = MODE_SCANNER_CONNECTING;
        }
      } else {
        state = MODE_SCANNER;
      }
      tft_update = true;
      break;
    case MODE_SOUND_SELECT_GROUP:
      if (press_type == SHORT_PRESS) {
        current_group = (current_group + 1) % 12;
        current_track = 0;
      } else {
        state = MODE_SOUND_SELECT_TRACK;
      }
      tft_update = true;
      break;
    case MODE_SOUND_SELECT_TRACK:
      if (press_type == SHORT_PRESS) {
        current_track = (current_track + 1) % 99;
      } else {
        state = MODE_SOUND_SELECT_PLAY;
      }
      tft_update = true;
      break;
    case MODE_SOUND_SELECT_PLAY:
      Serial.println("Play selected!");
      state = MODE_SOUND_PLAY;
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

    case MODE_SOUND_SELECT_GROUP:
    case MODE_SOUND_SELECT_TRACK:
    case MODE_SOUND_SELECT_PLAY:
    case MODE_SOUND_PLAY:
      if (press_type == SHORT_PRESS) {
        switch (state) {
          case MODE_SOUND_SELECT_GROUP:
            state = MODE_SOUND_SELECT_TRACK;
            break;
          case MODE_SOUND_SELECT_TRACK:
            state = MODE_SOUND_SELECT_PLAY;
            break;
          case MODE_SOUND_SELECT_PLAY:
            state = MODE_SOUND_SELECT_GROUP;
            break;
        }
      } else {
        droid_disconnect();
        state = MODE_SCANNER_RESULTS;
      }
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

    last_activity = millis();
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

    last_activity = millis();
  }
}

void setup() {
  uint8_t i;

  // T-Display-S3 needs this in order to run off battery
  #ifdef TDISPLAYS3
      pinMode(15,OUTPUT);
      digitalWrite(15, HIGH);
  #endif

  // setup display
  tft.init();
  tft.fillScreen(TFT_BLACK);
  tft.setRotation(3);

  // setup buttons as input
  pinMode(BUTTON1_PIN, INPUT);
  pinMode(BUTTON2_PIN, INPUT);

  // init bluetooth
  BLEDevice::init("");

  // setup the BLE client
  pClient = BLEDevice::createClient();

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

  // define deep sleep wakeup trigger; if commented out ESP32 goes into hibernation instead of deep sleep and only wakes up with the reset button
  // memory is lost from deep sleep; for our purposes deep sleep and hibernation are the same thing
  //esp_sleep_enable_ext0_wakeup(WAKEUP_BUTTON, WAKEUP_LEVEL);

  // initialize the sleep monitor timer
  last_activity = millis();

  // initialize the droid array
  for(i=0;i<MAX_DROIDS;i++) {
    droids[i].pAdvertisedDevice = nullptr;
  }

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
    case MODE_SCANNER_CONNECTING:
      update_display();
      if( droid_connect() ) {
        state = MODE_SCANNER_CONNECTED;
      } else {
        state = MODE_SCANNER_CONNECT_FAILED;
      }
      tft_update = true;
      break;
    case MODE_SOUND_PLAY:
      update_display();
      Serial.println("Playing sound...");
      droid_play_track();
      delay(2000);
      state = MODE_SOUND_SELECT_PLAY;
      tft_update = true;
      break;
    case MODE_SCANNER_CONNECTED:
      current_group = 0;
      current_track = 0;
      delay(2000);
      state = MODE_SOUND_SELECT_GROUP;
      tft_update = true;
      break;
    case MODE_SCANNER_CONNECT_FAILED:
      delay(2000);
      state = MODE_SCANNER_RESULTS;
      tft_update = true;
      break;
    default:
      break;
  }

  update_display();

  // inactivity sleep check; screen will blank when ESP32 goes to sleep
  // press reset button to wakeup
  if (millis() - last_activity > SLEEP_AFTER) {

    // do not go to sleep if the beacon is active
    if (state != MODE_BEACON_ON) {
      Serial.println("Going to sleep.");
      delay(100);
      esp_deep_sleep_start();
    } else {
      last_activity = millis();
    }
  }
}
