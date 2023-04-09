#include <Arduino.h>
/* Droid Toolbox v0.56 : ruthsarian@gmail.com
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
 *  Flash Mode: QIO 80MHz
 *  Flash Size: 16MB (128Mb)
 *  Partition Scheme: Huge App (3MB No OTA/1MB SPIFFS)
 *  Core Debug Level: None
 *  PSRAM: OPI PSRAM 
 *  USB CDC On Boot: Enabled
 *  JTAG Adapter: Integrated USB JTAG
 *  USB Mode: Hardware CDC and JTAG
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
 *   TFT_eSPI:
 *     https://github.com/Bodmer/TFT_eSPI/blob/master/TFT_eSPI.h
 *     https://github.com/Bodmer/TFT_eSPI/blob/master/TFT_eSPI.cpp
 *     https://github.com/Bodmer/TFT_eSPI/blob/master/examples/Generic/Viewport_Demo/Viewport_commands.ino
 *     https://github.com/Bodmer/TFT_eSPI/blob/master/examples/Generic/TFT_Button_Label_Datum/TFT_Button_Label_Datum.ino
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
 *   internationalization/localization; 
 *     put all static strings into a location (an array, a bunch of #defines) that allows for easier
 *     translation to other languages without having to sift through all the code
 *
 * HISTORY
 *   v0.56 : added a caption to select menus; this puts back functionality present in earlier versions that i just prefer
 *           short button 2 press on scan results now goes back 1 droid (if droids are found) instead of returning to main menu; long button 2 press will return to main menu
 *           created some #defines to control color and font to make customization a little easier
 *           personality chip ID 0x0C now identified as 'D-O' thanks to cre8or on swgediscord.com
 *   v0.55 : Added BD personality to know personalities
 *           Set delay between beacon reacons to 1 minute; BD units CORRECTLY interpret the value of 0x02 as a 10 second delay.
 *   v0.54 : Added volume control
 *           Started work on creating a generic menu system
 *           Centered the play track screen 
 *           Current group and track no longer reset to 1-1 when exiting the play track screen
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

#define C565(r,g,b) ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)    // macro to convert RGB values to TFT_eSPI color value

// CUSTOMIZATIONS BEGIN -- These values can be changed to alter Droid Toolbox's behavior.

#define MSG_VERSION "v0.56"

#define PLAIN_TEXT_SIZE                   2                   // a generic size used throughout 
#define PLAIN_TEXT_COLOR                  TFT_DARKGREY        // e.g. 'turn off your droid remote'

#define SPLASH_TEXT_SIZE                  PLAIN_TEXT_SIZE     // title is SPLASH_TEXT_SIZE+1 size, all other text is SPLASH_TEXT_SIZE
#define SPLASH_TITLE_COLOR                TFT_RED
#define SPLASH_SUBTITLE_COLOR             TFT_ORANGE
#define SPLASH_TEXT_COLOR                 TFT_LIGHTGREY
#define SPLASH_VERSION_COLOR              C565(64,64,64)      // TFT_VERYDARKGREY

#define MENU_SELECT_TEXT_PADDING          10
#define MENU_SELECT_CAPTION_TEXT_SIZE     PLAIN_TEXT_SIZE
#define MENU_SELECT_CAPTION_TEXT_COLOR    TFT_WHITE
#define MENU_SELECT_TEXT_SIZE             (PLAIN_TEXT_SIZE*2)
#define MENU_SELECT_TEXT_COLOR            C565(0,64,0)
#define MENU_SELECT_SELECTED_TEXT_COLOR   TFT_GREEN
#define MENU_SELECT_SELECTED_BORDER_COLOR TFT_BLUE

#define ACTION_TEXT_SIZE                  (PLAIN_TEXT_SIZE*2)
#define ACTION_TEXT_COLOR                 TFT_ORANGE          // e.g. 'CONNECTING'
#define ACTION_RESULT_OK_TEXT_COLOR       TFT_GREEN           // e.g. 'CONNECTED'
#define ACTION_RESULT_NG_TEXT_COLOR       TFT_RED             // e.g. 'CONNECT FAILED'

#define DROID_REPORT_TEXT_SIZE            PLAIN_TEXT_SIZE     // personality text size is +1, otherwise this value is used throughout the screen
#define DROID_REPORT_COLOR                PLAIN_TEXT_COLOR
#define DROID_REPORT_PERSONALITY_COLOR    TFT_RED
#define DROID_REPORT_AFFILIATION_COLOR    TFT_GREEN
#define DROID_REPORT_ADDRESS_COLOR        TFT_BLUE
#define DROID_REPORT_RSSI_COLOR           TFT_PURPLE          

#define SLEEP_AFTER 5 * 60 * 1000  // how many milliseconds of inactivity before going to sleep/hibernation

// static strings used throughout DroidToolbox

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
const char msg_droid_report[] = "DROID REPORT";
const char msg_scanner_connecting[] = "CONNECTING";
const char msg_turn_off_remote1[] = "TURN OFF YOUR";
const char msg_turn_off_remote2[] = "DROID REMOTE";
const char msg_scanner_connected[] = "CONNECTED";
const char msg_connect[] = "CONNECT";
const char msg_failed[]= "FAILED";
const char msg_no_droids1[] = "No Droids";
const char msg_no_droids2[] = "In Area";
const char msg_unknown_int[] = "Unknown (%d)";
const char msg_rssi[] = "rssi: %ddBm";
const char msg_d_of_d[] = "%d of %d";
    
const char* msg_droid_personalities[] = {
  "00",       // 0x00, should never encounter this
  "R Unit",   // 0x01
  "BB Unit",  // 0x02
  "Blue",     // 0x03
  "Gray",     // 0x04
  "Red",      // 0x05
  "Orange",   // 0x06
  "Purple",   // 0x07
  "Black",    // 0x08
  "CB-23",    // 0x09, aka "Red 2"
  "Yellow",   // 0x0A
  "C1-10P",   // 0x0B
  "D-O",      // 0x0C, 
  "Blue 2",   // 0x0D
  "BD Unit",  // 0x0E, BD is a Droid Depot droid at heart!
};

// CUSTOMIZATIONS END -- In theory you shouldn't have to edit anything below this line.

#ifdef ARDUINO_ESP32S3_DEV  // this is assuming you're compiling for T-Display-S3 using the "ESP32S3 Dev Module" board.
#define TDISPLAYS3
#endif

#define BUTTON1_PIN       0   // button 1 on the TTGO T-Display and T-Display-S3 is GPIO 0
#ifdef TDISPLAYS3          
#define BUTTON2_PIN       14  // button 2 on the T-Display-S3 is GPIO14
#else
#define BUTTON2_PIN       35  // button 2 on the TTGO T-Display is GPIO 35
#endif

#define LAZY_DEBOUNCE     10  // time to wait after a button press before considering it a good press
#define SHORT_PRESS_TIME  500 // maximum time, in milliseconds, that a button can be held before release and be considered a SHORT press

#define MAX_DROIDS        20  // maximum number of droids to report on
#define BLE_SCAN_TIME     5   // how many seconds to scan

#define PAYLOAD_SIZE      8   // size, in bytes, of a beacon payload
#define MSG_LEN_MAX       32
#define DROID_ADDR_LEN    20

#define WAKEUP_BUTTON     GPIO_NUM_0  // wake up when button 1 is pressed _ONLY_IF_ it's been enabled in setup(); otherwise the reset button will wake up the TTGO
#define WAKEUP_LEVEL      LOW         // wake up from sleep when the button is pressed (LOW)

uint32_t last_activity;

const BLEUUID serviceUUID("09b600a0-3e42-41fc-b474-e9c0c8f0c801");
const BLEUUID cmdUUID("09b600b1-3e42-41fc-b474-e9c0c8f0c801");
const BLEUUID notifyUUID("09b600b0-3e42-41fc-b474-e9c0c8f0c801");  // not used, but keeping it for future reference

BLEScan* pBLEScan = nullptr;
BLEClient* pClient = nullptr;
BLERemoteService* pRemoteService = nullptr;
BLERemoteCharacteristic* pRemoteCharacteristicCmd = nullptr;
BLERemoteCharacteristic* pRemoteCharacteristicNotify = nullptr;
BLEAdvertising* pAdvertising = nullptr;
BLEAdvertisementData oScanResponseData = BLEAdvertisementData();
BLEAdvertisementData* pAdvertisementData = nullptr;  // must be pointer so i can delete class then recreate it every time beacon changes
                                                     // should i be using smart pointers?

typedef struct droid {
  uint8_t chipid;
  uint8_t affid;
  BLEAdvertisedDevice* pAdvertisedDevice;
} droid_t;

droid_t droids[MAX_DROIDS];
uint8_t droid_count = 0;
uint8_t current_droid = 0;
uint8_t current_group = 0;
uint8_t current_track = 0;

const uint8_t SWGE_LOCATION_BEACON_PAYLOAD[] = {
  0x83, 0x01,  // manufacturer's id: 0x0183
  0x0A,        // type of beacon (location beacon)
  0x04,        // length of beacon data
  0x01,        // location; also corresponds to the audio group the droid will select a sound from
  0x0C,        // minimum interval between droid reactions to the beacon; this value is multiplied by 5 to determine the interval in seconds. droids have a minimum reaction time of 60 seconds (except for BD)
  0xA6,        // expected RSSI, beacon is ignored if weaker than value specified
  0x01,        // ? 0 or 1 otherwise droid will ignore the beacon
};

const uint8_t SWGE_DROID_BEACON_PAYLOAD[] = {
  0x83, 0x01,  // manufacturer's id: 0x0183
  0x03,        // type of beacon (droid beacon)
  0x04,        // length of beacon data
  0x44,        // ??
  0x81,        // 0x01 + ( 0x80 if droid is paired with a remote)
  0x82,        // a combination of personality chip and affiliation IDs
  0x01,        // personality chip ID
};

uint8_t payload[PAYLOAD_SIZE];

typedef enum {
  SHORT_PRESS,
  LONG_PRESS
} button_press_t;


/*
 * Menu/State Hierarchy
 * 
 * SPLASH 
 *  MODE_SCANNER_SELECTED
 *    MODE_SCANNER_SCANNING
 *      MODE_SCANNER_RESULTS
 *        MODE_SCANNER_CONNECTING
 *          MODE_SCANNER_CONNECTED
 *            MODE_SOUND_SELECTED
 *              MODE_SOUND_GROUP
 *              MODE_SOUND_TRACK
 *              MODE_SOUND_PLAY
 *                MODE_SOUND_PLAYING
 *            MODE_VOLUME_SELECTED
 *              MODE_VOLUME_UP
 *              MODE_VOLUME_DOWN
 *              MODE_VOLUME_TEST
 *                MODE_SOUND_TESTING
 *          MODE_SCANNER_CONNECT_FAILED
 *  MODE_BEACON_SELECTED
 *    MODE_BEACON_OFF
 *    MODE_BEACON_ON
 *    
 */

typedef enum {
  SPLASH,

  MODE_SCANNER_SELECTED,
  MODE_SCANNER_SCANNING,
  MODE_SCANNER_RESULTS,
  MODE_SCANNER_CONNECTING,
  MODE_SCANNER_CONNECTED,
  MODE_SCANNER_CONNECT_FAILED,

  MODE_SOUND_SELECTED,
  MODE_SOUND_GROUP,
  MODE_SOUND_TRACK,
  MODE_SOUND_PLAY,
  MODE_SOUND_PLAYING,

  MODE_VOLUME_SELECTED,
  MODE_VOLUME_UP,
  MODE_VOLUME_DOWN,
  MODE_VOLUME_TEST,
  MODE_VOLUME_TESTING,

  MODE_BEACON_SELECTED,
  MODE_BEACON_OFF,
  MODE_BEACON_ON
} system_state_t;

typedef struct menu_item {
  system_state_t state;
  char* text;
} menu_item_t;

menu_item_t top_menu[] = {
  { MODE_SCANNER_SELECTED, "SCANNER" },
  { MODE_BEACON_SELECTED, "BEACON" }
};

menu_item_t connected_menu[] = {
  { MODE_SOUND_SELECTED, "SOUNDS" },
  { MODE_VOLUME_SELECTED, "VOLUME" }
};

system_state_t state = SPLASH;

TFT_eSPI tft = TFT_eSPI();  // display interface
bool tft_update = true;     // flag to inidcate display needs to be updated

uint8_t selected_item = 0;
 int8_t droid_volume = 100; // there is no way to 'read' the current volume setting, so we'll keep track with a variable

const char* msg_droid_affiliation[] = {
  "Scoundrel",    // 0x01
  "Resistance",   // 0x05
  "First Order",  // 0x09
};

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
class MyAdvertisedDeviceCallbacks : public BLEAdvertisedDeviceCallbacks {
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
    mfid = rawdata[0] + (rawdata[1] << 8);

    // do not add this device if it does not have a manufacturer's id of 0x0183 (Disney)
    if (rawdata_len != 8 || mfid != 0x0183) {
      return;
    }

    // the droid list is sorted from strongest to weakest signal strength (RSSI) value
    // locate where in the list this new droid will be inserted
    pos = 0;
    if (droid_count > 0) {

      // find where to insert droid into list; higher RSSI come first in list
      for (pos = 0; pos < droid_count; pos++) {
        if (droids[pos].pAdvertisedDevice->getRSSI() < advertisedDevice.getRSSI()) {
          break;
        }
      }

      // this droid's signal strength is too weak, do not insert it into the list
      if (droid_count == MAX_DROIDS && pos == MAX_DROIDS) {
        return;
      }

      // push droids with a lower RSSI down the list
      for (i = droid_count; i > pos; i--) {
        if (i < MAX_DROIDS) {
          droids[i] = droids[i - 1];                  // move droid to new position in array
          droids[i - 1].pAdvertisedDevice = nullptr;  // clear the previous position in prep for the new droid
        } else {
          delete droids[i - 1].pAdvertisedDevice;
        }
      }
    }

    // store found droid's information
    droids[pos].chipid = rawdata[rawdata_len - 1];
    droids[pos].affid = (rawdata[rawdata_len - 2] - 0x80) / 2;

    // need to store a pointer to an AdvertisedDevice object for this device because
    // it's the only thing i can use to connect to the droid successfully

    // first check to make sure there isn't already a pointer and if there is delete it
    if (droids[pos].pAdvertisedDevice != nullptr) {
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
  for (i = 0; i < length; i++) {

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
      for (; j < i; j++) {
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
  uint8_t login_value[] = { 0x22, 0x20, 0x01 };
  uint8_t cmd_a[] = { 0x27, 0x42, 0x0f, 0x44, 0x44, 0x00, 0x1f, 0x07 };
  uint8_t cmd_b[] = { 0x27, 0x42, 0x0f, 0x44, 0x44, 0x00, 0x18, 0x00 };

  // end any current connection
  droid_disconnect();

  // create a new connection
  Serial.print("Connecting to ");
  Serial.println(droids[current_droid].pAdvertisedDevice->getAddress().toString().c_str());
  if (!pClient->connect(droids[current_droid].pAdvertisedDevice)) {
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
  uint8_t cmd_set_group[] = { 0x27, 0x42, 0x0f, 0x44, 0x44, 0x00, 0x1f, 0x07 };
  uint8_t cmd_play_track[] = { 0x27, 0x42, 0x0f, 0x44, 0x44, 0x00, 0x18, 0x00 };

  if (pClient->isConnected() && pRemoteCharacteristicCmd != nullptr) {
    cmd_set_group[7] = current_group % 12;
    cmd_play_track[7] = current_track & 0xFF;

    Serial.print("Playing track ");
    Serial.print(current_group);
    Serial.print("-");
    Serial.println(current_track);

    pRemoteCharacteristicCmd->writeValue(cmd_set_group, sizeof(cmd_set_group));
    delay(100);
    pRemoteCharacteristicCmd->writeValue(cmd_play_track, sizeof(cmd_play_track));
    delay(100);
  }
}

void droid_play_next_track() {
  uint8_t cmd_play_next_track[] = { 0x26, 0x42, 0x0f, 0x43, 0x44, 0x00, 0x1c };

  if (pClient->isConnected() && pRemoteCharacteristicCmd != nullptr) {
    pRemoteCharacteristicCmd->writeValue(cmd_play_next_track, sizeof(cmd_play_next_track));
    delay(100);
  }
}

void droid_set_volume() {
  uint8_t cmd_set_volume[] = { 0x27, 0x42, 0x0f, 0x44, 0x44, 0x00, 0x0e, 0x1f };

  if (pClient->isConnected() && pRemoteCharacteristicCmd != nullptr) {
    cmd_set_volume[7] = (uint8_t)((float)droid_volume / 3.2); // where's the 3.2 come from? 
                                                              // assumed good values for volume are 0x00 - 0x1f (31).
                                                              // 100 / 31 = 3.2 (ish)
                                                              // the 0 to 100 volume scale is just cosmetic

    Serial.print("New volume: 0x");
    Serial.println(cmd_set_volume[7], HEX);

    pRemoteCharacteristicCmd->writeValue(cmd_set_volume, sizeof(cmd_set_volume));
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

void tft_println_center(const char* msg) {
  tft.setCursor((tft.width() / 2) - (tft.textWidth(msg) / 2), tft.getCursorY());
  tft.println(msg);
}

void reset_screen(void) {
  tft.resetViewport();
  tft.fillScreen(TFT_BLACK);
  tft.setCursor(0, 0);
  tft.setTextDatum(TL_DATUM);
}

void display_captioned_menu (const char* caption, menu_item_t* items, uint8_t num_items) {
  uint8_t h;

  reset_screen(); // necessary? we call this from update_display already...

  tft.setTextSize(MENU_SELECT_CAPTION_TEXT_SIZE);
  tft.setTextColor(MENU_SELECT_CAPTION_TEXT_COLOR);
  tft.setTextDatum(TC_DATUM);
  tft.drawString(caption, tft.width()/2, MENU_SELECT_TEXT_PADDING);

  h = tft.fontHeight() + MENU_SELECT_TEXT_PADDING;
  tft.setViewport(0, h, tft.width(), tft.height()-h);

  display_menu(items, num_items);
}

void display_menu(menu_item_t* items, uint8_t num_items) {
  int16_t y = 0;
  int16_t w = 0;
  uint8_t row_padding, rows, row_height, row_width, i, offset;
  int8_t max_padding;

  // reset_screen();

  // set menu font size
  tft.setTextSize(MENU_SELECT_TEXT_SIZE);

  row_padding = MENU_SELECT_TEXT_PADDING + 1;  // +1 for the border

  // since font size can be changed via a define, or that this code might run on a screen
  // that i haven't tested with, check to make sure padding isn't so big that we can't show 
  // at least 2 menu items on the screen.
  max_padding = (tft.getViewportHeight() - (tft.fontHeight() * 2) - 4)/6;
  if (max_padding < 1) {
    row_padding = 0;
  } else if (max_padding < row_padding) {
    row_padding = max_padding;
  }

  row_height = tft.fontHeight() + (row_padding * 2);
  rows = tft.getViewportHeight() / row_height;  // how many rows in the menu can fit on a single screen?

  // find the widest menu item and use that to determine the width of the box for the selected item
  row_width = 0;
  for (i = 0; i < num_items; i++) {
    if (tft.textWidth(items[i].text) > row_width) {
      row_width = tft.textWidth(items[i].text);
    }
  }
  row_width += (row_padding * 4);

  // calculate an offset such that when it comes time
  // to draw the menu, the selected item appears in the middle
  // of the screen
  //
  // only do that in instances where the entire menu will NOT fit on the screen
  if (num_items > rows) {
    for (offset = 0; offset < num_items; offset++) {
      if (state == items[offset].state) {
        break;
      }
    }
    offset -= ((num_items - 1) / 2);
    offset %= num_items;
  } else {
    offset = 0;
  }

  // where do we start drawing the menu?
  y = (tft.getViewportHeight() / 2) - ((num_items * row_height) / 2);

  if (num_items > rows && rows % 2 == 0) {
    y += row_height / 2;
  }

  // draw the menu
  for (i = offset; i < (num_items + offset); i++) {
    if (state == items[i % num_items].state) {
      tft.setTextColor(MENU_SELECT_SELECTED_TEXT_COLOR);
      tft.drawRect(
        (tft.getViewportWidth() / 2) - (row_width / 2),
        y,
        row_width,
        row_height,
        MENU_SELECT_SELECTED_BORDER_COLOR);
    } else {
      tft.setTextColor(MENU_SELECT_TEXT_COLOR);
    }
    tft.setCursor((tft.getViewportWidth() / 2) - (tft.textWidth(items[i % num_items].text) / 2), y + row_padding + 1);
    tft.print(items[i % num_items].text);

    y += row_height;
  }
}

// display the splash screen seen when the program starts
void display_splash() {
  uint16_t y = 0;

  // surely there's an easier way to vertically position the splash screen text
  tft.setTextSize(1);
  y = (tft.height() - ((tft.fontHeight() * (SPLASH_TEXT_SIZE + 1)) + (tft.fontHeight() * SPLASH_TEXT_SIZE * 4))) / 2;
  tft.setCursor(0, y);

  tft.setTextDatum(TC_DATUM);

  // title
  tft.setTextSize(SPLASH_TEXT_SIZE + 1);
  tft.setTextColor(SPLASH_TITLE_COLOR);
  tft.drawString(msg_title, tft.width()/2, y);
  y += tft.fontHeight();

  // contact
  tft.setTextSize(SPLASH_TEXT_SIZE);
  tft.setTextColor(SPLASH_SUBTITLE_COLOR);
  tft.drawString(msg_email, tft.width()/2, y);
  y += (tft.fontHeight() * 2);

  // press any button...
  tft.setTextColor(SPLASH_TEXT_COLOR);
  tft.drawString(msg_continue2, tft.width()/2, y);
  y += tft.fontHeight();
  tft.drawString(msg_continue1, tft.width()/2, y);

  // version
  tft.setTextColor(SPLASH_VERSION_COLOR);
  tft.setTextDatum(BR_DATUM);
  tft.drawString(msg_version, tft.width(), tft.height());
}

void display_scanner_results() {
  char msg[MSG_LEN_MAX];
  uint16_t y = 0;
  uint8_t i;

  // display header
  tft.setTextSize(DROID_REPORT_TEXT_SIZE);
  tft.setTextColor(DROID_REPORT_COLOR);
  tft.setTextDatum(TC_DATUM);
  tft.drawString(msg_droid_report, tft.width()/2, 0);

  // add a gap after the header
  if (droid_count > 0) {

    // find where to start printing droid details so that it is vertically centered
    tft.setTextSize(1);
    y = (tft.height() - ((tft.fontHeight() * (DROID_REPORT_TEXT_SIZE + 1)) + (tft.fontHeight() * DROID_REPORT_TEXT_SIZE * 3)))/2;

    // print droid personality
    tft.setTextSize(DROID_REPORT_TEXT_SIZE + 1);
    tft.setTextColor(DROID_REPORT_PERSONALITY_COLOR);
    if (droids[current_droid].chipid < (sizeof(msg_droid_personalities) / sizeof(char*))) {
      tft.drawString(msg_droid_personalities[droids[current_droid].chipid], tft.width()/2, y);
    } else {
      snprintf(msg, MSG_LEN_MAX, msg_unknown_int, droids[current_droid].chipid);
      tft.drawString(msg, tft.width()/2, y);
    }

    // print droid affiliation
    y += tft.fontHeight();
    tft.setTextColor(DROID_REPORT_AFFILIATION_COLOR);
    tft.setTextSize(DROID_REPORT_TEXT_SIZE);
    switch (droids[current_droid].affid) {
      case 1:
        tft.drawString(msg_droid_affiliation[0], tft.width()/2, y);
        break;
      case 5:
        tft.drawString(msg_droid_affiliation[1], tft.width()/2, y);
        break;
      case 9:
        tft.drawString(msg_droid_affiliation[2], tft.width()/2, y);
        break;
      default:
        snprintf(msg, MSG_LEN_MAX, msg_unknown_int, droids[current_droid].affid);
        tft.drawString(msg, tft.width()/2, y);
        break;
    }

    // print Bluetooth MAC address
    y += tft.fontHeight();
    tft.setTextColor(DROID_REPORT_ADDRESS_COLOR);
    tft.drawString(droids[current_droid].pAdvertisedDevice->getAddress().toString().c_str(), tft.width()/2, y);

    // print RSSI
    y += tft.fontHeight();
    tft.setTextColor(DROID_REPORT_RSSI_COLOR);
    snprintf(msg, MSG_LEN_MAX, msg_rssi, droids[current_droid].pAdvertisedDevice->getRSSI());
    tft.drawString(msg, tft.width()/2, y);

    // print
    tft.setTextDatum(BC_DATUM);
    tft.setTextColor(DROID_REPORT_COLOR);
    snprintf(msg, MSG_LEN_MAX, msg_d_of_d, current_droid + 1, droid_count);
    tft.drawString(msg, tft.width()/2, tft.height());

  // display message that no droids were found
  } else {
    tft.setTextSize(DROID_REPORT_TEXT_SIZE + 1);
    tft.setTextColor(ACTION_RESULT_NG_TEXT_COLOR);
    tft.setTextDatum(TC_DATUM);
    y = (tft.height()/2) - tft.fontHeight();
    tft.drawString(msg_no_droids1, tft.width()/2, y);
    tft.drawString(msg_no_droids2, tft.width()/2, y + tft.fontHeight());
  }
}

void display_track_select() {
  char msg[MSG_LEN_MAX];
  
  uint8_t vgap = 0;
  uint8_t hgap = 0;

  // display instruction
  tft.setTextSize(3);

  vgap = (tft.height() - (tft.fontHeight() * 3)) / 3;

  snprintf(msg, MSG_LEN_MAX, "Group: 88");
  hgap = (tft.width() - tft.textWidth(msg))/2;

  tft.setCursor(hgap, tft.getCursorY() + vgap - 2);
  tft.setTextColor(TFT_BLUE);
  tft.print("Group: ");
  tft.setTextColor((state == MODE_SOUND_GROUP) ? TFT_GREEN : TFT_DARKGREY);
  snprintf(msg, MSG_LEN_MAX, "%d", (current_group + 1));
  tft.println(msg);

  tft.setCursor(hgap, tft.getCursorY() + 5);
  tft.setTextColor(TFT_RED);
  tft.print("Track: ");
  tft.setTextColor((state == MODE_SOUND_TRACK) ? TFT_GREEN : TFT_DARKGREY);
  snprintf(msg, MSG_LEN_MAX, "%d", (current_track + 1));
  tft.println(msg);

  tft.setCursor(0, tft.getCursorY() + vgap);
  tft.setTextColor((state == MODE_SOUND_PLAY) ? TFT_GREEN : TFT_DARKGREY);
  tft_println_center("PLAY");
}

void display_volume() {
  char msg[MSG_LEN_MAX];
  uint8_t hgap = 0;
  uint8_t vgap = 0;
  uint8_t v2 = 0;

  // sizes 3 and 4 are used on this screen. we need to take the heights of both sizes into account
  // when calculating the size of the vertical gaps.
  tft.setTextSize(3);
  vgap = tft.fontHeight() * 2;

  tft.setTextSize(5);
  vgap = (tft.height() - tft.fontHeight() - vgap) / 3;
  
  tft.setCursor(tft.getCursorX(), vgap * 0.9);

  // change volume color based on its value
  if (droid_volume < 50) {
    tft.setTextColor(TFT_CYAN);
  } else if (droid_volume < 80) {
    tft.setTextColor(TFT_YELLOW);
  } else if (droid_volume < 100) {
    tft.setTextColor(TFT_RED);
  } else {
    tft.setTextColor(TFT_MAGENTA);
  }

  snprintf(msg, MSG_LEN_MAX, "%d", droid_volume);
  tft_println_center(msg);
  tft.setCursor(tft.getCursorX(), tft.getCursorY() + (vgap * 0.9));
  
  tft.setTextSize(3);
  tft.setTextColor(TFT_GREEN);
  snprintf(msg, MSG_LEN_MAX, "VOL+");
  hgap = (tft.width() - (tft.textWidth(msg) * 2)) / 3;
  tft.setCursor(hgap, tft.getCursorY());

  tft.setTextColor((state == MODE_VOLUME_UP ? TFT_GREEN : TFT_DARKGREY));
  tft.print(msg);
  tft.setCursor(tft.getCursorX() + hgap, tft.getCursorY());

  tft.setTextColor((state == MODE_VOLUME_DOWN ? TFT_GREEN : TFT_DARKGREY));
  snprintf(msg, MSG_LEN_MAX, "VOL-");
  tft.println(msg);
  tft.setCursor(tft.getCursorX(), tft.getCursorY() + (vgap * 0.3));

  tft.setTextColor((state == MODE_VOLUME_TEST ? TFT_GREEN : TFT_DARKGREY));
  tft_println_center("SET VOLUME");
}

void update_display() {
  uint16_t y;

  if (tft_update != true) {
    return;
  }

  reset_screen();

  switch (state) {

    case MODE_BEACON_ON:              // display_beacon
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

    case MODE_VOLUME_UP:              // display_volume()
    case MODE_VOLUME_DOWN:
    case MODE_VOLUME_TEST:
    case MODE_VOLUME_TESTING: {
      display_volume();
      break;
    }

    case MODE_SOUND_GROUP:            // display_track_select()
    case MODE_SOUND_TRACK:
    case MODE_SOUND_PLAY:
    case MODE_SOUND_PLAYING:
      display_track_select();
      break;

    case MODE_SOUND_SELECTED:         // display_connected_menu()
    case MODE_VOLUME_SELECTED:
      display_captioned_menu(msg_select, connected_menu, sizeof(connected_menu) / sizeof(menu_item_t));
      break;

    case MODE_SCANNER_CONNECTED:      // display_connected()
      tft.setTextDatum(MC_DATUM);
      tft.setTextSize(ACTION_TEXT_SIZE);
      tft.setTextColor(ACTION_RESULT_OK_TEXT_COLOR);
      tft.drawString(msg_scanner_connected, tft.width()/2, tft.height()/2);
      break;

    case MODE_SCANNER_CONNECT_FAILED: // display_connect_failed()
      tft.setTextDatum(TC_DATUM);
      tft.setTextSize(ACTION_TEXT_SIZE);
      tft.setTextColor(ACTION_RESULT_NG_TEXT_COLOR);
      tft.drawString(msg_connect, tft.width()/2, (tft.height()/2) - tft.fontHeight());
      tft.drawString(msg_failed, tft.width()/2, (tft.height()/2));
      break;

    case MODE_SCANNER_CONNECTING:     // display_connecting()
      tft.setTextDatum(TC_DATUM);
      tft.setTextSize(PLAIN_TEXT_SIZE);
      tft.setTextColor(PLAIN_TEXT_COLOR);
      tft.drawString(msg_turn_off_remote1, tft.width()/2, 0);
      tft.drawString(msg_turn_off_remote2, tft.width()/2, tft.fontHeight());
      tft.setTextDatum(MC_DATUM);
      tft.setTextSize(ACTION_TEXT_SIZE);
      tft.setTextColor(ACTION_TEXT_COLOR);
      tft.drawString(msg_scanner_connecting, tft.width()/2, tft.height()/2);
      break;

    case MODE_SCANNER_RESULTS:        // display_scanner_results()
      display_scanner_results();
      break;

    case MODE_SCANNER_SCANNING:       // display_scanning()
      tft.setTextDatum(MC_DATUM);
      tft.setTextSize(ACTION_TEXT_SIZE);
      tft.setTextColor(ACTION_TEXT_COLOR);
      tft.drawString(msg_scanner_active, tft.width()/2, tft.height()/2);
      break;

    case MODE_SCANNER_SELECTED:       // display_top_menu()
    case MODE_BEACON_SELECTED:
      display_captioned_menu(msg_select, top_menu, sizeof(top_menu) / sizeof(menu_item_t));
      break;

    case SPLASH:                      // display_splash()
      display_splash();
      break;
  }

  tft_update = false;
}

void button1(button_press_t press_type);  // trying to use an enum as a parameter triggers a bug in arduino. adding an explicit prototype resolves the issue.
void button1(button_press_t press_type) {

  static uint32_t last_time_btn1 = 0;
  static uint32_t last_time_btn1_down = 0;

  switch (state) {
    case SPLASH:
      state = MODE_SCANNER_SELECTED;
      tft_update = true;
      break;

    case MODE_SCANNER_SELECTED:
      state = MODE_SCANNER_SCANNING;
      tft_update = true;
      break;

    case MODE_SCANNER_RESULTS:
      if (droid_count > 0) {
        if (press_type == LONG_PRESS) {
          state = MODE_SCANNER_CONNECTING;
          tft_update = true;
        } else if (droid_count > 1) {
          current_droid = (current_droid + 1) % droid_count;
          tft_update = true;
        }
      } else {
        state = MODE_SCANNER_SCANNING;
        tft_update = true;
      }
      break;

    case MODE_SOUND_SELECTED:
      state = MODE_SOUND_GROUP;
      tft_update = true;
      break;

    case MODE_SOUND_GROUP:
      if (press_type == SHORT_PRESS) {
        current_group = (current_group + 1) % 12;
        current_track = 0;
      } else {
        state = MODE_SOUND_TRACK;
      }
      tft_update = true;
      break;

    case MODE_SOUND_TRACK:
      if (press_type == SHORT_PRESS) {
        current_track = (current_track + 1) % 99;
      } else {
        state = MODE_SOUND_PLAY;
      }
      tft_update = true;
      break;

    case MODE_SOUND_PLAY:
      Serial.println("Play selected!");
      state = MODE_SOUND_PLAYING;
      tft_update = true;
      break;

    case MODE_VOLUME_SELECTED:
      state = MODE_VOLUME_UP;
      tft_update = true;
      break;

    case MODE_VOLUME_UP:
      if (droid_volume > 90) {
        droid_volume = 100;
      } else {
        droid_volume += 10;
      }
      tft_update = true;
      break;

    case MODE_VOLUME_DOWN:
      if (droid_volume < 10) {
        droid_volume = 0;
      } else {
        droid_volume -= 10;
      }
      tft_update = true;
      break;

    case MODE_VOLUME_TEST:
      state = MODE_VOLUME_TESTING;
      tft_update = true;
      break;

    case MODE_BEACON_OFF:
      init_advertisement_data();
      set_payload_location_beacon(esp_random());
      pAdvertising->start();
      state = MODE_BEACON_ON;
      tft_update = true;
      break;

    case MODE_BEACON_SELECTED:
    case MODE_BEACON_ON:
      pAdvertising->stop();
      state = MODE_BEACON_OFF;
      tft_update = true;
      break;
  }
}

void button2(button_press_t press_type);  // trying to use an enum as a parameter triggers a bug in arduino. adding an explicit prototype resolves the issue.
void button2(button_press_t press_type) {

  static uint32_t last_time_btn2 = 0;
  static uint32_t last_time_btn2_down = 0;

  // do button 2 stuff
  switch (state) {
    case SPLASH:
      state = MODE_SCANNER_SELECTED;
      tft_update = true;
      break;

    case MODE_SOUND_SELECTED:
    case MODE_VOLUME_SELECTED:
      if (press_type == SHORT_PRESS) {
        switch (state) {
          case MODE_SOUND_SELECTED:
            state = MODE_VOLUME_SELECTED;
            break;
          default:
            state = MODE_SOUND_SELECTED;
            break;
        }
      } else {
        droid_disconnect();
        state = MODE_SCANNER_RESULTS;
      }
      tft_update = true;
      break;

    case MODE_SCANNER_SELECTED:
      state = MODE_BEACON_SELECTED;
      tft_update = true;
      break;

    case MODE_BEACON_SELECTED:
    case MODE_BEACON_ON:
    case MODE_BEACON_OFF:
      pAdvertising->stop();
      state = MODE_SCANNER_SELECTED;
      tft_update = true;
      break;

    case MODE_SCANNER_RESULTS:
      if (press_type == LONG_PRESS || droid_count < 1) {
        state = MODE_SCANNER_SELECTED;
        tft_update = true;
      } else {
        if (droid_count > 1) {
          if (current_droid == 0) {
            current_droid = droid_count - 1;
          } else {
            current_droid = (current_droid - 1) % droid_count;
          }
          tft_update = true;
        }
      }
      break;

    case MODE_SOUND_GROUP:
    case MODE_SOUND_TRACK:
    case MODE_SOUND_PLAY:
      if (press_type == SHORT_PRESS) {
        switch (state) {
          case MODE_SOUND_GROUP:
            state = MODE_SOUND_TRACK;
            break;
          case MODE_SOUND_TRACK:
            state = MODE_SOUND_PLAY;
            break;
          case MODE_SOUND_PLAY:
            state = MODE_SOUND_GROUP;
            break;
        }
      } else {
        state = MODE_SOUND_SELECTED;
      }
      tft_update = true;
      break;

    case MODE_VOLUME_UP:
    case MODE_VOLUME_DOWN:
    case MODE_VOLUME_TEST:
      if (press_type == SHORT_PRESS) {
        switch(state) {
          case MODE_VOLUME_UP:
            state = MODE_VOLUME_DOWN;
            break;
          case MODE_VOLUME_DOWN:
            state = MODE_VOLUME_TEST;
            break;
          case MODE_VOLUME_TEST:
            state = MODE_VOLUME_UP;
            break;
        }
      } else {
        state = MODE_VOLUME_SELECTED;
      }
    tft_update = true;
    break;
  }
}

void button_handler() {
  static uint32_t last_btn1_time = 0;
  static uint8_t last_btn1_state = HIGH;
  static uint32_t last_btn2_time = 0;
  static uint8_t last_btn2_state = HIGH;

  // gather current state of things
  uint8_t now_btn1_state = digitalRead(BUTTON1_PIN);
  uint8_t now_btn2_state = digitalRead(BUTTON2_PIN);
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
  pinMode(15, OUTPUT);
  digitalWrite(15, HIGH);
#endif

  // setup display
  tft.init();
  tft.setRotation(3);
  reset_screen();

  // setup buttons as input
  pinMode(BUTTON1_PIN, INPUT);
  pinMode(BUTTON2_PIN, INPUT);

  // init bluetooth
  BLEDevice::init("");

  // setup the BLE client
  pClient = BLEDevice::createClient();

  // setup BLE scanner
  pBLEScan = BLEDevice::getScan();  //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());
  pBLEScan->setActiveScan(true);  //active scan uses more power, but get results faster
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
  for (i = 0; i < MAX_DROIDS; i++) {
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
      update_display();
      ble_scan();
      current_droid = 0;
      state = MODE_SCANNER_RESULTS;
      tft_update = true;
      break;

    case MODE_SCANNER_CONNECTING:
      update_display();
      if (droid_connect()) {
        state = MODE_SCANNER_CONNECTED;
      } else {
        state = MODE_SCANNER_CONNECT_FAILED;
      }
      tft_update = true;
      break;

    case MODE_SCANNER_CONNECTED:
      delay(2000);
      state = MODE_SOUND_SELECTED;
      tft_update = true;
      break;

    case MODE_SCANNER_CONNECT_FAILED:
      delay(2000);
      state = MODE_SCANNER_RESULTS;
      tft_update = true;
      break;

    case MODE_SOUND_PLAYING:
      update_display();
      Serial.println("Playing sound...");
      droid_play_track();
      delay(2000);
      state = MODE_SOUND_PLAY;
      tft_update = true;
      break;

    case MODE_VOLUME_TESTING:
      update_display();
      Serial.println("Testing volume...");
      droid_set_volume();
      droid_play_track();
      //droid_play_next_track();
      delay(2000);
      state = MODE_VOLUME_TEST;
      tft_update = true;
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
