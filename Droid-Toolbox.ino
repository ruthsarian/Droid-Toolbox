/* Droid Toolbox v0.60.ALPHA : ruthsarian@gmail.com
 *
 *
 * v0.60 TODO:
 *  - finish moving all colors into the #define section at top of code
 *  - clean up/remove old formatting stuff
 * 
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
 *   consider a method for setting font sizes based on screen size
 *
 * HISTORY
 *   v0.57 : added ability to select which beacon the droid toolbox will produce
 *           expert beacon mode allows finer control over the beacon; i probably should have hidden it behind a key combination...
 *           added global beacon variable to store the details of the beacon that will be produced
 *           lots of work on the underlying menu system and helper functions
 *           added global variable to track currently selected item in a menu, rather than have a STATE for each menu option
 *           moving more control of the display options (text color, size, etc) to the block of #defines at the top of the code
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
 *   v0.50 : Added ability to connect to droid from scan results using a long-press of button 1
 *           Droid plays activation sound upon connection
 *           Group and track can be selected and played through the droid.
 *   v0.40 : Added deep sleep/hibernation
 *           Added initial ability to connect to droid with long button 1 press while viewing droid in scan results
 *            Connection is currently a demo; connect, tell droid to play a sound, then disconnect. 
 *            Will improve upon this in the next version.
 *   v0.30 : Long/Short button press detection
 *           Droid report is paged; shows 1 droid at a time
 *           Droid report sorts droids by RSSI value
 *           Added version to splash screen
 *   v0.20 : Added back button from both beacon and scanner.
 *           Location beacon location is randomly selected.
 *   v0.10 : Initial Release
 */

#include <TFT_eSPI.h>
#include <SPI.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#define NUM_BEACON_PARAMS                3
#define C565(r,g,b)                     ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)    // macro to convert RGB values to TFT_eSPI color value


// CUSTOMIZATIONS BEGIN -- These values can be changed to alter Droid Toolbox's behavior.

#define MSG_VERSION                         "v0.60.ALPHA"           // the version displayed on the splash screen at the lower right

#define DEFAULT_TEXT_SIZE                   2                       // a generic size used throughout 
#define DEFAULT_TEXT_COLOR                  TFT_DARKGREY            // e.g. 'turn off your droid remote'
#define DEFAULT_TEXT_PADDING                10                      // necessary? perhaps just use a formula throughout the code? e.g. (tft.fontHeight() / 3)
#define DEFAULT_SELECTED_TEXT_COLOR         TFT_WHITE
#define DEFAULT_SELECTED_BORDER_COLOR       TFT_YELLOW

#define SPLASH_TEXT_SIZE                    DEFAULT_TEXT_SIZE       // title is SPLASH_TEXT_SIZE+1 size, all other text is SPLASH_TEXT_SIZE
#define SPLASH_TITLE_COLOR                  TFT_RED
#define SPLASH_SUBTITLE_COLOR               TFT_ORANGE
#define SPLASH_TEXT_COLOR                   TFT_LIGHTGREY
#define SPLASH_VERSION_COLOR                C565(64,64,64)          // TFT_VERYDARKGREY

#define MENU_SELECT_TEXT_PADDING            DEFAULT_TEXT_PADDING
#define MENU_SELECT_CAPTION_TEXT_SIZE       DEFAULT_TEXT_SIZE
#define MENU_SELECT_CAPTION_TEXT_COLOR      TFT_WHITE
#define MENU_SELECT_TEXT_SIZE               (DEFAULT_TEXT_SIZE*2)
#define MENU_SELECT_TEXT_COLOR              C565(0,64,0)
#define MENU_SELECT_SELECTED_TEXT_COLOR     TFT_GREEN
#define MENU_SELECT_SELECTED_BORDER_COLOR   TFT_BLUE

#define BEACON_SELECT_TEXT_PADDING          8
#define BEACON_SELECT_CAPTION_TEXT_SIZE     DEFAULT_TEXT_SIZE
#define BEACON_SELECT_CAPTION_TEXT_COLOR    TFT_WHITE
#define BEACON_SELECT_TEXT_SIZE             (DEFAULT_TEXT_SIZE + 1)
#define BEACON_SELECT_TEXT_COLOR            C565(0,64,0)
#define BEACON_SELECT_SELECTED_TEXT_COLOR   TFT_GREEN
#define BEACON_SELECT_SELECTED_BORDER_COLOR TFT_BLUE

#define BEACON_CONTROL_TEXT_SIZE            (DEFAULT_TEXT_SIZE + 1)
#define BEACON_CONTROL_TEXT_PADDING         DEFAULT_TEXT_PADDING
#define BEACON_CONTROL_TYPE_TEXT_SIZE       DEFAULT_TEXT_SIZE
#define BEACON_CONTROL_TYPE_COLOR           TFT_BLUE
#define BEACON_CONTROL_ID_COLOR             TFT_RED
#define BEACON_CONTROL_ACTIVE_COLOR         TFT_YELLOW
#define BEACON_CONTROL_INACTIVE_COLOR       TFT_BROWN

#define BEACON_EXPERT_TEXT_SIZE             DEFAULT_TEXT_SIZE
#define BEACON_EXPERT_TEXT_PADDING          DEFAULT_TEXT_PADDING
#define BEACON_EXPERT_LABEL_COLOR           DEFAULT_TEXT_COLOR
#define BEACON_EXPERT_VALUE_COLOR           TFT_BROWN
#define BEACON_EXPERT_VALUE_SELECTED_COLOR  TFT_YELLOW

#define ACTION_TEXT_SIZE                    (DEFAULT_TEXT_SIZE*2)
#define ACTION_TEXT_COLOR                   TFT_ORANGE            // e.g. 'CONNECTING'
#define ACTION_RESULT_OK_TEXT_COLOR         TFT_GREEN             // e.g. 'CONNECTED'
#define ACTION_RESULT_NG_TEXT_COLOR         TFT_RED               // e.g. 'CONNECT FAILED'

#define DROID_REPORT_TEXT_SIZE              DEFAULT_TEXT_SIZE     // personality text size is +1, otherwise this value is used throughout the screen
#define DROID_REPORT_COLOR                  DEFAULT_TEXT_COLOR
#define DROID_REPORT_PERSONALITY_COLOR      TFT_RED
#define DROID_REPORT_AFFILIATION_COLOR      TFT_GREEN
#define DROID_REPORT_ADDRESS_COLOR          TFT_BLUE
#define DROID_REPORT_RSSI_COLOR             TFT_PURPLE

#define SETTING_NAME_COLOR                  TFT_DARKGREY
#define SETTING_VALUE_COLOR                 TFT_BROWN
#define SETTING_SELECTED_VALUE_COLOR        TFT_YELLOW

#define SLEEP_AFTER 5 * 60 * 1000  // how many milliseconds of inactivity before going to sleep/hibernation

// static strings used throughout DroidToolbox

const char ble_adv_name[]               = "DROIDTLBX";              // this is the name the toolbox's beacon will appear as, keep to 10 characters or less
const char msg_version[]                = MSG_VERSION;
const char msg_title[]                  = "Droid Toolbox";
const char msg_email[]                  = "ruthsarian@gmail.com";
const char msg_continue1[]              = "press any button";
const char msg_continue2[]              = "to continue...";
const char msg_select[]                 = "select an option";
const char msg_select_beacon_type[]     = "select beacon type";
const char msg_select_beacon[]          = "select a beacon";
const char msg_beacon[]                 = "BEACON";
const char msg_beacon_off[]             = "OFF";
const char msg_beacon_on[]              = "ON";
const char msg_scanner[]                = "SCANNER";
const char msg_scanner_active[]         = "SCANNING";
const char msg_droid_report[]           = "DROID REPORT";
const char msg_scanner_connecting[]     = "CONNECTING";
const char msg_turn_off_remote1[]       = "TURN OFF YOUR";
const char msg_turn_off_remote2[]       = "DROID REMOTE";
const char msg_scanner_connected[]      = "CONNECTED";
const char msg_connect[]                = "CONNECT";
const char msg_failed[]                 = "FAILED";
const char msg_no_droids1[]             = "No Droids";
const char msg_no_droids2[]             = "In Area";
const char msg_unknown_int[]            = "Unknown (%d)";           // probably need to remove the printf variables from these strings
const char msg_rssi[]                   = "rssi: %ddBm";
const char msg_d_of_d[]                 = "%d of %d";
const char msg_sounds[]                 = "SOUNDS";
const char msg_volume[]                 = "VOLUME";
const char msg_random[]                 = "RANDOM";
const char msg_droid[]                  = "DROID";
const char msg_location[]               = "LOCATION";
const char msg_beacon_settings[]        = "BEACON SETTINGS";
const char msg_activate_beacon[]        = "inactive";
const char msg_beacon_active[]          = "ACTIVE";
const char msg_expert[]                 = "EXPERT";
const char msg_yes[]                    = "YES";
const char msg_no[]                     = "NO";
const char msg_type[]                   = "TYPE";
const char msg_state[]                  = "STATE";

// the index of a personality name in this array should correspond to that personality's ID
// not following this will result in the wrong names being displayed
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
  "BD Unit"   // 0x0E, BD is a Droid Depot droid at heart!
};

// the index of a location name in this array should correspond to that location's beacon ID
// not following this will result in the wrong names being displayed
const char* msg_locations[] = {
  "The Void",         // 0x00, should never encounter this
  "Marketplace",      // 0x01
  "Behind Depot",     // 0x02
  "Resistance",       // 0x03
  "Unknown",          // 0x04, have never found this beacon inside GE
  "Droid Depot",      // 0x05
  "Dok Ondar's",      // 0x06
  "First Order"       // 0x07
};

const char* msg_droid_affiliation[] = {
  "Scoundrel",    // 0x01
  "Resistance",   // 0x05
  "First Order"   // 0x09
};

// next two arrays are labels for the expert beacon screen
const char* msg_beacon_droid_param[NUM_BEACON_PARAMS] = {
  "CHIP",
  "AFFL",
  "PAIRED"
};
const char* msg_beacon_location_param[NUM_BEACON_PARAMS] = {
  "LCTN", 
  "REACT INT", 
  "MIN RSSI"
};

// CUSTOMIZATIONS END -- In theory you shouldn't have to edit anything below this line.

#define DROID_ID_R_UNIT           0x01  // these values should match the ID of the corresponding personality
#define DROID_ID_BB_UNIT          0x02  // these values should ALSO match the index value of the 
#define DROID_ID_BLUE             0x03  // corresponding personality in the msg_droid_personalities[] array
#define DROID_ID_GRAY             0x04
#define DROID_ID_RED              0x05
#define DROID_ID_ORANGE           0x06
#define DROID_ID_PURPLE           0x07
#define DROID_ID_BLACK            0x08
#define DROID_ID_CB_23            0x09
#define DROID_ID_YELLOW           0x0A
#define DROID_ID_C1_10P           0x0B
#define DROID_ID_D_O              0x0C
#define DROID_ID_BLUE_2           0x0D
#define DROID_ID_BD_UNIT          0x0E

#define LOCATION_ID_MARKET        0x01  // these values should match the ID of the corresponding location
#define LOCATION_ID_BEHIND_DEPOT  0x02  // these values should ALSO match the index value of the 
#define LOCATION_ID_RESISTANCE    0x03  // corresponding location in the msg_locations[] array
#define LOCATION_ID_UNKNOWN       0x04
#define LOCATION_ID_DROID_DEPOT   0x05
#define LOCATION_ID_DOK_ONDAR     0x06
#define LOCATION_ID_FIRST_ORDER   0x07

#ifdef ARDUINO_ESP32S3_DEV  // this is assuming you're compiling for T-Display-S3 using the "ESP32S3 Dev Module" board.
  #define TDISPLAYS3
#endif

#define BUTTON1_PIN       0   // button 1 on the TTGO T-Display and T-Display-S3 is GPIO 0
#ifdef TDISPLAYS3          
  #define BUTTON2_PIN     14  // button 2 on the T-Display-S3 is GPIO14
#else
  #define BUTTON2_PIN     35  // button 2 on the TTGO T-Display is GPIO 35
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

const BLEUUID serviceUUID("09b600a0-3e42-41fc-b474-e9c0c8f0c801");
const BLEUUID cmdUUID("09b600b1-3e42-41fc-b474-e9c0c8f0c801");
const BLEUUID notifyUUID("09b600b0-3e42-41fc-b474-e9c0c8f0c801");  // not used, but keeping it for future reference

uint32_t last_activity;

BLEScan* pBLEScan = nullptr;
BLEClient* pClient = nullptr;
BLERemoteService* pRemoteService = nullptr;
BLERemoteCharacteristic* pRemoteCharacteristicCmd = nullptr;
BLERemoteCharacteristic* pRemoteCharacteristicNotify = nullptr;
BLEAdvertising* pAdvertising = nullptr;
BLEAdvertisementData oScanResponseData = BLEAdvertisementData();
BLEAdvertisementData* pAdvertisementData = nullptr;  // must be pointer so i can delete class then recreate it every time beacon changes
                                                     // should i be using smart pointers?

typedef struct {
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
  DROID,
  LOCATION
} beacon_type_t;

typedef struct {
  beacon_type_t type;                 // DROID or LOCATION
  uint8_t setting[NUM_BEACON_PARAMS]; // 0 = droid:personality[1:14],  location:location[1:7]
                                      // 1 = droid:affiliation[1,5,9], location:interval[1:20]
                                      // 2 = droid:paired[0,1],        location:minimum_rssi[58:28]
} beacon_t;
beacon_t beacon;

typedef enum {
  SHORT_PRESS,
  LONG_PRESS
} button_press_t;

typedef enum {
  SPLASH,                 // splash screen
  TOP_MENU,               // top menu; beacon or scanner
  BEACON_TYPE_MENU,       // display the types of beacons to pick from
  BEACON_DROID_LIST,      // display a list of droid beacons to pick from
  BEACON_LOCATION_LIST,   // display a list of location beacons to pick from
  BEACON_RANDOM,          // generate a random beacon
  BEACON_ACTIVATE,        // display the option to activate the beacon
  BEACON_ACTIVE,          // display the currently active beacon
  BEACON_EXPERT,          // display an 'expert mode' beacon control screen
  BEACON_EXPERT_ACTIVE,   // for when the expert mode beacon is active

  SCANNER_SCANNING,
  SCANNER_RESULTS,
  SCANNER_CONNECTING,
  SCANNER_CONNECTED,
  SCANNER_CONNECT_FAILED, 

  CONNECTED_MENU,         // options available after connecting to a droid; volume or sounds

  SOUND_GROUP,
  SOUND_TRACK,
  SOUND_PLAY,
  SOUND_PLAYING,

  VOLUME_UP,
  VOLUME_DOWN,
  VOLUME_TEST,
  VOLUME_TESTING,
} system_state_t;

typedef struct {
  system_state_t state;
  const char* text;
} menu_item_t;

menu_item_t top_menu[] = {
  { SCANNER_SCANNING, msg_scanner },
  { BEACON_TYPE_MENU, msg_beacon  }
};

menu_item_t beacon_type_menu[] = {
  { BEACON_LOCATION_LIST, msg_location },
  { BEACON_DROID_LIST,    msg_droid    },
  { BEACON_RANDOM,        msg_random   },
  { BEACON_EXPERT,        msg_expert   },
};

menu_item_t connected_menu[] = {
  { SOUND_GROUP, msg_sounds },
  { VOLUME_UP,   msg_volume },
};

typedef struct {
  uint8_t id;
  const char* label;
} beacon_item_t;

beacon_item_t droid_beacons[] = {
  { DROID_ID_R_UNIT,  msg_droid_personalities[DROID_ID_R_UNIT] },
  { DROID_ID_BB_UNIT, msg_droid_personalities[DROID_ID_BB_UNIT] },
  { DROID_ID_BLUE,    msg_droid_personalities[DROID_ID_BLUE] },
  { DROID_ID_GRAY,    msg_droid_personalities[DROID_ID_GRAY] },
  { DROID_ID_RED,     msg_droid_personalities[DROID_ID_RED] },
  { DROID_ID_ORANGE,  msg_droid_personalities[DROID_ID_ORANGE] },
  { DROID_ID_PURPLE,  msg_droid_personalities[DROID_ID_PURPLE] },
  { DROID_ID_BLACK,   msg_droid_personalities[DROID_ID_BLACK] },
  { DROID_ID_CB_23,   msg_droid_personalities[DROID_ID_CB_23] },
  { DROID_ID_YELLOW,  msg_droid_personalities[DROID_ID_YELLOW] },
  { DROID_ID_C1_10P,  msg_droid_personalities[DROID_ID_C1_10P] },
  { DROID_ID_D_O,     msg_droid_personalities[DROID_ID_D_O] },
  { DROID_ID_BLUE_2,  msg_droid_personalities[DROID_ID_BLUE_2] },
  { DROID_ID_BD_UNIT, msg_droid_personalities[DROID_ID_BD_UNIT] },
};

beacon_item_t location_beacons[] = {
  { LOCATION_ID_MARKET,         msg_locations[LOCATION_ID_MARKET] },
  { LOCATION_ID_BEHIND_DEPOT,   msg_locations[LOCATION_ID_BEHIND_DEPOT] },
  { LOCATION_ID_RESISTANCE,     msg_locations[LOCATION_ID_RESISTANCE] },
  { LOCATION_ID_UNKNOWN,        msg_locations[LOCATION_ID_UNKNOWN] },
  { LOCATION_ID_DROID_DEPOT,    msg_locations[LOCATION_ID_DROID_DEPOT] },
  { LOCATION_ID_DOK_ONDAR,      msg_locations[LOCATION_ID_DOK_ONDAR] },
  { LOCATION_ID_FIRST_ORDER,    msg_locations[LOCATION_ID_FIRST_ORDER] },
};

TFT_eSPI tft = TFT_eSPI();      // display interface
bool tft_update = true;         // flag to inidcate display needs to be updated

typedef struct {                // information for rendering lists; could probably define multiple list styles rather than a single global variable
  uint8_t  text_size;
  uint16_t text_color;
  uint8_t  text_padding;
  uint16_t selected_text_color;
  uint16_t selected_border_color;
} tft_list_options_t;
tft_list_options_t list_options;

int8_t droid_volume = 100;      // there is no way to 'read' the current volume setting, so we'll keep track with a variable and assume it starts at full (100) volume
uint8_t selected_item = 0;      // keep track of the currently selected option when displaying menus, options, etc.
system_state_t state = SPLASH;  // track the current state of the toolbox

void init_advertisement_data() {
  if (pAdvertisementData != nullptr) {
    delete pAdvertisementData;
  }
  pAdvertisementData = new BLEAdvertisementData();
  pAdvertisementData->setName(ble_adv_name);
}

void load_payload_location_beacon_data() {
  memcpy(payload, SWGE_LOCATION_BEACON_PAYLOAD, sizeof(uint8_t) * PAYLOAD_SIZE);
}

void load_payload_droid_beacon_data() {
  memcpy(payload, SWGE_DROID_BEACON_PAYLOAD, sizeof(uint8_t) * PAYLOAD_SIZE);
}

// populate beacon struct with a droid beacon
void set_droid_beacon(uint8_t personality) {

  beacon.type = DROID;
  Serial.println("Creating a DROID beacon.");

  // make sure the personality chip ID passed is a valid one
  if (personality > 0 && personality <= (sizeof(msg_droid_personalities) / sizeof(char*))) {
    beacon.setting[0] = personality;

  // otherwise generate a random beacon
  } else {
    beacon.setting[0] = (esp_random() % ((sizeof(msg_droid_personalities) / sizeof(char*)) - 1)) + 1;
  }
  Serial.print("  Personality: ");
  Serial.println(msg_droid_personalities[beacon.setting[0]]);

  // set affiliation based on personality
  Serial.print("  Affiliation: ");
  switch(beacon.setting[0]) {

    // resistance
    case 3:
    case 6:
    case 10:
    case 11:
    case 12:
    case 14:
      beacon.setting[1] = 5;
      Serial.println("Resistance");
      break;

    // first order
    case 5:
    case 8:
      beacon.setting[1] = 9;
      Serial.println("First Order");
      break;

    // scoundrel
    default:
      beacon.setting[1] = 1;
      Serial.println("Scoundrel");
      break;
  }

  // set paired to true
  beacon.setting[2] = 1;
}

void set_location_beacon(uint8_t location) {
  beacon.type = LOCATION;
  Serial.println("Creating a LOCATION beacon.");

  // set location
  if (location > 0 && location <= (sizeof(msg_locations) / sizeof(char*))) {
    beacon.setting[0] = location;
  } else {
    beacon.setting[0] = (esp_random() % ((sizeof(msg_locations) / sizeof(char*)) - 1)) + 1;
  }
  Serial.print("  Location: ");
  Serial.println(msg_locations[beacon.setting[0]]);

  // set reaction interval (in minutes), could go as high as 19, but keeping it low on purpose
  //beacon.setting[1] = (esp_random() % 3) + 1;
  beacon.setting[1] = 2;
  Serial.print("  Interval: ");
  Serial.println(beacon.setting[1]);

  // set minimum RSSI for droid to react; while this value is stored as an unsigned value, think of it as a negative value in dBm; e.g. 38 = -38dBm
  beacon.setting[2] = 38;
  Serial.print("  Minimum RSSI: -");
  Serial.print(beacon.setting[2]);
  Serial.println("dBm");
}

// populate the global beacon variable with random(ish) values
void set_random_beacon() {
  Serial.println("Generating a random beacon.");
  if (esp_random() % 2)  {
    set_droid_beacon(0);      // create a DROID beacon
  } else {
    set_location_beacon(0);   // create a LOCATION beacon
  }
}

// set the advertising payload based on the data in the global beacon variable
void set_payload_from_beacon() {

  // DROID beacon type
  if (beacon.type == DROID) {
    load_payload_droid_beacon_data();

    // set whether or not the droid appears as being paired with a remote
    payload[5] = 0x01 + (beacon.setting[2] > 0 ? 0x80 : 0x00);

    // set affiliation
    payload[6] = 0x80 + (beacon.setting[1] * 2);

    // set personality chip id
    payload[7] = beacon.setting[0];

  // LOCATION beacon type
  } else {
    load_payload_location_beacon_data();

    // set location
    payload[4] = beacon.setting[0];

    // set reaction interval
    payload[5] = (beacon.setting[1] % 20) * 12;

    // set minimum RSSI for reaction
    payload[6] = 0x80 + beacon.setting[2];
  }

  // load the payload into the advertisement data
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
  static uint8_t cmd_set_group[]  = { 0x27, 0x42, 0x0f, 0x44, 0x44, 0x00, 0x1f, 0x07 };
  static uint8_t cmd_play_track[] = { 0x27, 0x42, 0x0f, 0x44, 0x44, 0x00, 0x18, 0x00 };

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
  static uint8_t cmd_play_next_track[] = { 0x26, 0x42, 0x0f, 0x43, 0x44, 0x00, 0x1c };
  if (pClient->isConnected() && pRemoteCharacteristicCmd != nullptr) {
    pRemoteCharacteristicCmd->writeValue(cmd_play_next_track, sizeof(cmd_play_next_track));
    delay(100);
  }
}

void droid_set_volume() {
  static uint8_t cmd_set_volume[] = { 0x27, 0x42, 0x0f, 0x44, 0x44, 0x00, 0x0e, 0x1f };

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

// todo: deprecate this function
void tft_println_center(const char* msg) {
  tft.setCursor((tft.width() / 2) - (tft.textWidth(msg) / 2), tft.getCursorY());
  tft.println(msg);
}

void reset_screen(void) {
  tft.resetViewport();
  tft.fillScreen(TFT_BLACK);
  tft.setTextDatum(TL_DATUM);
  tft.setCursor(0, 0);
}

void display_list(const char **items, uint8_t num_items) {
  int16_t  y = 0;
  int16_t  w = 0;
  uint16_t row_height, row_width;
  uint8_t  row_padding, rows, i, selected;
  int8_t   max_padding;

  // set menu font size
  tft.setTextSize(list_options.text_size);

  // calculate row padding
  row_padding = list_options.text_padding;

  // since font size can be changed via a define, or that this code might run on a screen
  // that i haven't tested with, check to make sure padding isn't so big that we can't show 
  // at least 2 menu items on the screen.
  max_padding = (tft.getViewportHeight() - (tft.fontHeight() * 2) - 4)/6;
  if (max_padding < 1) {
    row_padding = 0;
  } else if (max_padding < row_padding) {
    row_padding = max_padding;
  }

  // calculate how tall each row will be
  row_height = tft.fontHeight() + (row_padding * 2);

  // calculate how many rows will fit within the screen
  rows = tft.getViewportHeight() / row_height;

  // find the widest menu item and use that to determine the width of the box for the selected item
  row_width = 0;
  for (i = 0; i < num_items; i++) {
    if (tft.textWidth(items[i]) > row_width) {
      row_width = tft.textWidth(items[i]);
    }
  }
  
  // add some horizontal padding to the box based on row padding
  row_width += (row_padding * 4);

  // datum tells tft.drawString() where to draw the string relative to the passed x,y values
  // TC = top, center; this helps center the string without having to calculate the position myself
  tft.setTextDatum(TC_DATUM);

  // LIST RENDERING METHODOLOGY:
  //   to give a sense of moving through a menu that's taller than the screen, i want the selected item to be
  //   in a position on the screen relative to its location in the list. so the first item is rendered at the top,
  //   the last item is rendered at the bottom, and intervening items render somewhere in the middle
  //
  //   i tested several different approaches and i found this "feels" right given the limited screen space

  // is the list larger than the screen?
  if (num_items > rows && rows >= 1)  {

    // where should the selected item appear on screen?
    y = ((tft.getViewportHeight() - row_height) / (num_items - 1)) * selected_item;

    // where should the list start on (or off) screen in order to put the selected item at the 
    // previously calculated location?
    y -= (row_height * selected_item);

  // entire list will fit on the screen, vertically center it
  } else {
    y = (tft.getViewportHeight() - (row_height * num_items))/2;
  }

  // draw the list, starting at the previously calculated position (y)
  for (i = 0; i < num_items; i++) {

    // is the current list item selected?
    if (i == selected_item) {

      // draw a box around the list item
      tft.drawRect(
        (tft.getViewportWidth() / 2) - (row_width / 2),
        y,
        row_width,
        row_height,
        list_options.selected_border_color
      );

      // set the text color for selected list items
      tft.setTextColor(list_options.selected_text_color);
    } else {

      // this item is not selected, set its text color to the normal color
      tft.setTextColor(list_options.text_color);
    }

    // draw the list item on the screen
    tft.drawString(items[i], tft.getViewportWidth()/2, y + row_padding + 1);    // the +1 here is to compensate for the box being slightly veritcally off-center

    // increment y for the next list item
    y += row_height;
  }
}

// TODO: display_captioned_menu() and display_beacon_menu() duplicate a lot of code
//       find a way to.. not do that

void display_captioned_menu(const char* caption, menu_item_t* menu_items, uint8_t num_items) {
  uint8_t h, i;

  const char** list_items;

  //reset_screen(); // necessary? we call this from update_display already...

  // set size, color, and datum
  tft.setTextSize(MENU_SELECT_CAPTION_TEXT_SIZE);
  tft.setTextColor(MENU_SELECT_CAPTION_TEXT_COLOR);
  tft.setTextDatum(TC_DATUM);

  // draw menu caption
  tft.drawString(caption, tft.width()/2, MENU_SELECT_TEXT_PADDING);

  // calculate viewport dimensions for subsequent menu
  h = tft.fontHeight() + (MENU_SELECT_TEXT_PADDING * 2);
  tft.setViewport(0, h, tft.width(), tft.height()-h);

  // set display options for the menu
  list_options.text_size = MENU_SELECT_TEXT_SIZE;
  list_options.text_color = MENU_SELECT_TEXT_COLOR;
  list_options.selected_text_color = MENU_SELECT_SELECTED_TEXT_COLOR;
  list_options.selected_border_color = MENU_SELECT_SELECTED_BORDER_COLOR;
  list_options.text_padding = MENU_SELECT_TEXT_PADDING;

  // i feel a little dirty about using malloc like this; maybe i should just declare a fixed array and assign as necessary???
  list_items = (const char**)malloc(sizeof(char*)*num_items);
  for(i=0;i<num_items;i++) {
    list_items[i] = menu_items[i].text;
  }

  // display the menu
  display_list(list_items, num_items);

  // remove all traces of my evildoing
  free(list_items);
}

void display_beacon_menu(const char* caption, beacon_item_t* menu_items, uint8_t num_items) {
  uint8_t h, i;

  const char** list_items;

  // set size, color, and datum
  tft.setTextSize(BEACON_SELECT_CAPTION_TEXT_SIZE);
  tft.setTextColor(BEACON_SELECT_CAPTION_TEXT_COLOR);
  tft.setTextDatum(TC_DATUM);

  // draw menu caption
  tft.drawString(caption, tft.width()/2, BEACON_SELECT_TEXT_PADDING);

  // calculate viewport dimensions for subsequent menu
  h = tft.fontHeight() + (BEACON_SELECT_TEXT_PADDING * 2);
  tft.setViewport(0, h, tft.width(), tft.height()-h);

  // set display options for the menu
  list_options.text_size = BEACON_SELECT_TEXT_SIZE;
  list_options.text_color = BEACON_SELECT_TEXT_COLOR;
  list_options.selected_text_color = BEACON_SELECT_SELECTED_TEXT_COLOR;
  list_options.selected_border_color = BEACON_SELECT_SELECTED_BORDER_COLOR;
  list_options.text_padding = BEACON_SELECT_TEXT_PADDING;

  // create a char* array to pass to display_list()
  list_items = (const char**)malloc(sizeof(char*)*num_items);
  for(i=0;i<num_items;i++) {
    list_items[i] = menu_items[i].label;
  }

  // display the menu
  display_list(list_items, num_items);

  // free the malloc'd memory
  free(list_items);
}

void display_beacon_control() {
  uint16_t y, content_height;
  uint8_t gap, bfs;
  char msg[MSG_LEN_MAX];

  // line 1: beacon type
  // line 2: beacon id
  // line 3: gap
  // line 3: beacon state

  tft.setTextSize(1);
  bfs = tft.fontHeight();
  
  gap = (bfs * BEACON_CONTROL_TEXT_SIZE) / 2;
  Serial.print("BC:gap = ");
  Serial.println(gap);

  content_height = (bfs * BEACON_CONTROL_TYPE_TEXT_SIZE) + (bfs * BEACON_CONTROL_TEXT_SIZE *2) + gap + (BEACON_CONTROL_TEXT_PADDING * 3);
  Serial.print("BC:content_height = ");
  Serial.println(content_height);

  // calculate where to begin drawing text  
  y = (tft.getViewportHeight() - content_height)/2;

  Serial.print("BC:y = ");
  Serial.println(y);
  Serial.println();

  // begin rendering the screen
  tft.setTextDatum(TC_DATUM);
  
  // display beacon type
  tft.setTextSize(BEACON_CONTROL_TYPE_TEXT_SIZE);
  tft.setTextColor(BEACON_CONTROL_TYPE_COLOR);
  if (beacon.type == DROID) {
    snprintf(msg, MSG_LEN_MAX, "%s %s", msg_droid, msg_beacon);
  } else {
    snprintf(msg, MSG_LEN_MAX, "%s %s", msg_location, msg_beacon);
  }
  tft.drawString(msg, tft.getViewportWidth()/2, y);

  // adjust position for next line
  y += tft.fontHeight() + BEACON_CONTROL_TEXT_PADDING;

  // display beacon id
  tft.setTextSize(BEACON_CONTROL_TEXT_SIZE);
  tft.setTextColor(BEACON_CONTROL_ID_COLOR);
  if (beacon.type == DROID) {
    snprintf(msg, MSG_LEN_MAX, "%s", msg_droid_personalities[beacon.setting[0]]);
  } else {
    snprintf(msg, MSG_LEN_MAX, "%s", msg_locations[beacon.setting[0]]);
  }
  tft.drawString(msg, tft.getViewportWidth()/2, y);

  // adjust position for next line
  y += tft.fontHeight() + (BEACON_CONTROL_TEXT_PADDING * 2) + gap;

  // display beacon state
  tft.setTextSize(BEACON_CONTROL_TEXT_SIZE);
  if (state == BEACON_ACTIVE) {
    tft.setTextColor(BEACON_CONTROL_ACTIVE_COLOR);
    tft.drawString(msg_beacon_active, tft.getViewportWidth()/2, y);
  } else {
    tft.setTextColor(BEACON_CONTROL_INACTIVE_COLOR);
    tft.drawString(msg_activate_beacon, tft.getViewportWidth()/2, y);
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

    // stock affiliation
    if ( droids[current_droid].affid == 1 || droids[current_droid].affid == 5 || droids[current_droid].affid == 9) {
      tft.drawString(msg_droid_affiliation[droids[current_droid].affid >> 2], tft.width()/2, y);

    // non-stock affiliation
    } else {
      snprintf(msg, MSG_LEN_MAX, msg_unknown_int, droids[current_droid].affid);
      tft.drawString(msg, tft.width()/2, y);
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
  tft.setTextColor((state == SOUND_GROUP) ? TFT_GREEN : TFT_DARKGREY);
  snprintf(msg, MSG_LEN_MAX, "%d", (current_group + 1));
  tft.println(msg);

  tft.setCursor(hgap, tft.getCursorY() + 5);
  tft.setTextColor(TFT_RED);
  tft.print("Track: ");
  tft.setTextColor((state == SOUND_TRACK) ? TFT_GREEN : TFT_DARKGREY);
  snprintf(msg, MSG_LEN_MAX, "%d", (current_track + 1));
  tft.println(msg);

  tft.setCursor(0, tft.getCursorY() + vgap);
  tft.setTextColor((state == SOUND_PLAY) ? TFT_GREEN : TFT_DARKGREY);
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

  tft.setTextColor((state == VOLUME_UP ? TFT_GREEN : TFT_DARKGREY));
  tft.print(msg);
  tft.setCursor(tft.getCursorX() + hgap, tft.getCursorY());

  tft.setTextColor((state == VOLUME_DOWN ? TFT_GREEN : TFT_DARKGREY));
  snprintf(msg, MSG_LEN_MAX, "VOL-");
  tft.println(msg);
  tft.setCursor(tft.getCursorX(), tft.getCursorY() + (vgap * 0.3));

  tft.setTextColor((state == VOLUME_TEST ? TFT_GREEN : TFT_DARKGREY));
  tft_println_center("SET VOLUME");
}

void display_beacon_expert() {
  char msg[MSG_LEN_MAX];
  uint8_t i, j;

  /*
  // figure out the widest label so all the labels can be right-aligned; don't need to do this but...
  uint16_t label_width = 0;
  tft.setTextSize(BEACON_EXPERT_TEXT_SIZE);
  for(i=0;i<NUM_BEACON_PARAMS;i++) {
    if (tft.textWidth(msg_beacon_droid_param[i])>label_width) {
      label_width = tft.textWidth(msg_beacon_droid_param[i]);
    }
    if (tft.textWidth(msg_beacon_location_param[i])>label_width) {
      label_width = tft.textWidth(msg_beacon_location_param[i]);
    }
  }
  Serial.print("label_width = ");
  Serial.println(label_width);
  */

  // shrink the screen to give a little gap between the content and screen edge
  tft.setViewport(10, 10, tft.width()-20, tft.height()-20);
  tft.setTextSize(BEACON_EXPERT_TEXT_SIZE);
  tft.setCursor(0,0);

  // print the beacon type label
  snprintf(msg, MSG_LEN_MAX, "%s: ", msg_type);
  tft.setTextColor(BEACON_EXPERT_LABEL_COLOR);
  tft.print(msg);

  // print the beacon type
  if (selected_item == 0) {
    tft.setTextColor(BEACON_EXPERT_VALUE_SELECTED_COLOR);
  } else {
    tft.setTextColor(BEACON_EXPERT_VALUE_COLOR);
  }
  if (beacon.type == DROID) {
    snprintf(msg, MSG_LEN_MAX, "%s", msg_droid);
  } else {
    snprintf(msg, MSG_LEN_MAX, "%s", msg_location);
  }
  tft.println(msg);
  tft.setCursor(tft.getCursorX(), tft.getCursorY() + 2);

  // print the various parameters of the beacon
  for (i=0; i<NUM_BEACON_PARAMS; i++) {

    // label
    if (beacon.type == DROID) {
      snprintf(msg, MSG_LEN_MAX, "%s: ", msg_beacon_droid_param[i]);
    } else {
      snprintf(msg, MSG_LEN_MAX, "%s: ", msg_beacon_location_param[i]);
    }
    tft.setTextColor(BEACON_EXPERT_LABEL_COLOR);
    tft.print(msg);

    // value
    snprintf(msg, MSG_LEN_MAX, "%d", beacon.setting[i]);

    // droid beacon
    if (beacon.type == DROID) {

      // personality
      if (i==0) {
          for (j=0; j<sizeof(droid_beacons)/sizeof(beacon_item_t); j++) {
            if (droid_beacons[j].id == beacon.setting[i]) {
              snprintf(msg, MSG_LEN_MAX, "%s", droid_beacons[j].label);
              break;
            }
          }

      // affiliation
      } else if (i==1) {
        switch(beacon.setting[i]) {
          case 1:
          case 5:
          case 9:
            snprintf(msg, MSG_LEN_MAX, "%s", msg_droid_affiliation[beacon.setting[i] >> 2]);
            break;
        }

      // paired
      } else if (i==2) {
        if (beacon.setting[i] == 0) {
          snprintf(msg, MSG_LEN_MAX, "%s", msg_no);
        } else {
          snprintf(msg, MSG_LEN_MAX, "%s", msg_yes);
        }
      }

    // location beacon
    } else {

      // location
      if (i==0) {
          for (j=0; j<sizeof(location_beacons)/sizeof(beacon_item_t); j++) {
            if (location_beacons[j].id == beacon.setting[i]) {
              snprintf(msg, MSG_LEN_MAX, "%s", location_beacons[j].label);
              break;
            }
          }

      // reaction interval (in minutes)
      } else if (i==1) {
        ;

      // minimum rssi
      } else if (i==2) {
        snprintf(msg, MSG_LEN_MAX, "-%ddBm", beacon.setting[i]);
      }
    }
    if (selected_item == i+1) {
      tft.setTextColor(BEACON_EXPERT_VALUE_SELECTED_COLOR);
    } else {
      tft.setTextColor(BEACON_EXPERT_VALUE_COLOR);
    }
    tft.println(msg);
    tft.setCursor(tft.getCursorX(), tft.getCursorY() + 2);
  }
  
  // display beacon state (active/inactive)
  snprintf(msg, MSG_LEN_MAX, "%s: ", msg_state);
  tft.setTextColor(BEACON_EXPERT_LABEL_COLOR);
  tft.print(msg);

  if (selected_item == NUM_BEACON_PARAMS+1) {
    tft.setTextColor(BEACON_EXPERT_VALUE_SELECTED_COLOR);
  } else {
    tft.setTextColor(BEACON_EXPERT_VALUE_COLOR);
  }
  
  if (state == BEACON_EXPERT_ACTIVE) {
    snprintf(msg, MSG_LEN_MAX, "%s", msg_beacon_active);
  } else {
    snprintf(msg, MSG_LEN_MAX, "%s", msg_activate_beacon);
  }
  tft.println(msg);
}

void update_display() {
  uint16_t y;

  if (tft_update != true) {
    return;
  }

  reset_screen();

  switch (state) {

    case VOLUME_UP:              // display_volume()
    case VOLUME_DOWN:
    case VOLUME_TEST:
    case VOLUME_TESTING:
      display_volume();
      break;

    case SOUND_GROUP:            // display_track_select()
    case SOUND_TRACK:
    case SOUND_PLAY:
    case SOUND_PLAYING:
      display_track_select();
      break;

    case CONNECTED_MENU:         // display_connected_menu()
      display_captioned_menu(msg_select, connected_menu, sizeof(connected_menu) / sizeof(menu_item_t));
      break;

    case SCANNER_CONNECTED:      // display_connected()
      tft.setTextDatum(MC_DATUM);
      tft.setTextSize(ACTION_TEXT_SIZE);
      tft.setTextColor(ACTION_RESULT_OK_TEXT_COLOR);
      tft.drawString(msg_scanner_connected, tft.width()/2, tft.height()/2);
      break;

    case SCANNER_CONNECT_FAILED: // display_connect_failed()
      tft.setTextDatum(TC_DATUM);
      tft.setTextSize(ACTION_TEXT_SIZE);
      tft.setTextColor(ACTION_RESULT_NG_TEXT_COLOR);
      tft.drawString(msg_connect, tft.width()/2, (tft.height()/2) - tft.fontHeight());
      tft.drawString(msg_failed, tft.width()/2, (tft.height()/2));
      break;

    case SCANNER_CONNECTING:     // display_connecting()
      tft.setTextDatum(TC_DATUM);
      tft.setTextSize(DEFAULT_TEXT_SIZE);
      tft.setTextColor(DEFAULT_TEXT_COLOR);
      tft.drawString(msg_turn_off_remote1, tft.width()/2, 0);
      tft.drawString(msg_turn_off_remote2, tft.width()/2, tft.fontHeight());
      tft.setTextDatum(MC_DATUM);
      tft.setTextSize(ACTION_TEXT_SIZE);
      tft.setTextColor(ACTION_TEXT_COLOR);
      tft.drawString(msg_scanner_connecting, tft.width()/2, tft.height()/2);
      break;

    case SCANNER_RESULTS:        // display_scanner_results()
      display_scanner_results();
      break;

    case SCANNER_SCANNING:       // display_scanning()
      tft.setTextDatum(MC_DATUM);
      tft.setTextSize(ACTION_TEXT_SIZE);
      tft.setTextColor(ACTION_TEXT_COLOR);
      tft.drawString(msg_scanner_active, tft.width()/2, tft.height()/2);
      break;

    case BEACON_EXPERT_ACTIVE:  // display_beacon_expert()
    case BEACON_EXPERT:
      display_beacon_expert();
      break;

    case BEACON_ACTIVATE:        // display_beacon_control()
    case BEACON_ACTIVE:
      display_beacon_control();
      break;

    case BEACON_DROID_LIST:      // display_droid_beacon_list()
      display_beacon_menu(msg_select_beacon, droid_beacons, sizeof(droid_beacons) / sizeof(beacon_item_t));
      break;

    case BEACON_LOCATION_LIST:   // display_location_beacon_list()
      display_beacon_menu(msg_select_beacon, location_beacons, sizeof(location_beacons) / sizeof(beacon_item_t));
      break;

    case BEACON_TYPE_MENU:       // display_beacon_type_menu()
      display_captioned_menu(msg_select_beacon_type, beacon_type_menu, sizeof(beacon_type_menu) / sizeof(menu_item_t));
      break;

    case TOP_MENU:               // display_top_menu()
      display_captioned_menu(msg_select, top_menu, sizeof(top_menu) / sizeof(menu_item_t));
      break;

    case SPLASH:                 // display_splash()
      display_splash();
      break;
  }

  tft_update = false;
}

void button1(button_press_t press_type);  // trying to use an enum as a parameter triggers a bug in arduino. adding an explicit prototype resolves the issue.
void button1(button_press_t press_type) {

  static uint32_t last_time_btn1 = 0;
  static uint32_t last_time_btn1_down = 0;
  uint8_t i;

  switch (state) {
    case SPLASH:
      state = TOP_MENU;
      selected_item = 0;
      tft_update = true;
      break;

    case TOP_MENU:
      state = top_menu[selected_item].state;
      selected_item = 0;
      tft_update = true;
      break;

    case BEACON_TYPE_MENU:
      state = beacon_type_menu[selected_item].state;
      if (state == BEACON_RANDOM) {
        set_random_beacon();

        // the following if/else block is setting selected_item to the value of the randomly selected beacon
        // in its respective beacon list. this is being done so when you go back (long button 2 press)
        // you'll be brought to the element in the beacon list where the randomly selected beacon is
        if (beacon.type == DROID) {
          for(i=0; i<sizeof(droid_beacons) / sizeof(beacon_item_t); i++) {
            if (droid_beacons[i].id == beacon.setting[0]) {
              selected_item = i;
              break;
            }
          }
        } else {
          for(i=0; i<sizeof(location_beacons) / sizeof(beacon_item_t); i++) {
            if (location_beacons[i].id == beacon.setting[0]) {
              selected_item = i;
              break;
            }
          }
        }
        state = BEACON_ACTIVATE;
      } else if (state == BEACON_EXPERT) {

        // let whatever is currently in beacon be displayed on the expert screen.
        // let's users see what the stock beacons look like. if they figure out this little trick of set a beacon, then back out to expert mode.
        //set_random_beacon();

        selected_item = 0;
      } else {
        selected_item = 0;
      }
      tft_update = true;
      break;

    case BEACON_LOCATION_LIST:
      set_location_beacon(location_beacons[selected_item].id);
      state = BEACON_ACTIVATE;
      tft_update = true;
      break;

    case BEACON_DROID_LIST:
      set_droid_beacon(droid_beacons[selected_item].id);
      state = BEACON_ACTIVATE;
      tft_update = true;
      break;

    case BEACON_ACTIVATE:
      init_advertisement_data();
      set_payload_from_beacon();
      pAdvertising->start();
      state = BEACON_ACTIVE;
      tft_update = true;
      break;

    case BEACON_ACTIVE:
      pAdvertising->stop();
      state = BEACON_ACTIVATE;
      tft_update = true;
      break;

    case BEACON_EXPERT:
      // change beacon type
      if (selected_item == 0) {
        if (beacon.type == DROID) {
          beacon.type = LOCATION;
          beacon.setting[0] = 1;  // default location
          beacon.setting[1] = 2;  // default interval
          beacon.setting[2] = 38; // default minimim rssi
        } else {
          beacon.type = DROID;
          beacon.setting[0] = 1;  // default personality
          beacon.setting[1] = 1;  // default affiliation
          beacon.setting[2] = 1;  // default paired
        }

      // change beacon parameter 1
      } else if (selected_item == 1) {
         beacon.setting[0]++; 

        // droid:personality
        if (beacon.type == DROID) {
          if (press_type == LONG_PRESS) {
            beacon.setting[0] = 0;  // personality 0 doesn't exist...
          }

        // location:location
        } else {
          if (press_type == LONG_PRESS) {
            beacon.setting[0] = 0;  // location 0 doesn't exist...
          }
        }

      // change beacon parameter 2
      } else if (selected_item == 2) {
        beacon.setting[1]++;

        // droid:affiliation
        if (beacon.type == DROID) {
          if (press_type == LONG_PRESS || beacon.setting[1] > 0x3F) {
            beacon.setting[1] = 0;  // affiliation values beyond 0x3F break the formula
          }

        // location: reaction interval
        } else {
          if (press_type == LONG_PRESS || beacon.setting[1] > 0x33) {
            beacon.setting[1] = 0;  // can't have a reaction interval value greater than 0x33 (51) as it breaks the formula
          }
        }

      // change beacon parameter 3
      } else if (selected_item == 3) {
        beacon.setting[2]++;

        // droid:paired with remote?
        if (beacon.type == DROID) {
          if (beacon.setting[2] > 1) {    // swap betweeen paired (1) and not paired (0)
            beacon.setting[2] = 0;
          }

        // location:minimum rssi for a reaction
        } else {
          if (press_type == LONG_PRESS || beacon.setting[2] > 80) {
                                          // -80dBm is a pretty weak signal
            beacon.setting[2] = 20;       // from my experience an rssi of -28dBm is about 1 foot from the droid
                                          // is there any reason to go lower than -20dBm?
          }
        }

      // change beacon state (active or inactive)
      } else if (selected_item == 4) {
        init_advertisement_data();
        set_payload_from_beacon();
        pAdvertising->start();
        state = BEACON_EXPERT_ACTIVE;
      }
      tft_update = true;
      break;

    case BEACON_EXPERT_ACTIVE:
      pAdvertising->stop();
      state = BEACON_EXPERT;
      tft_update = true;
      break;

    case SCANNER_RESULTS:
      if (droid_count > 0) {
        if (press_type == LONG_PRESS) {
          state = SCANNER_CONNECTING;
          tft_update = true;
        } else if (droid_count > 1) {
          current_droid = (current_droid + 1) % droid_count;
          tft_update = true;
        }
      } else {
        state = SCANNER_SCANNING;
        tft_update = true;
      }
      break;

    case CONNECTED_MENU:
      state = connected_menu[selected_item].state;
      tft_update = true;
      break;

    case SOUND_GROUP:
      if (press_type == SHORT_PRESS) {
        current_group = (current_group + 1) % 12;
        current_track = 0;
      } else {
        state = SOUND_TRACK;
      }
      tft_update = true;
      break;

    case SOUND_TRACK:
      if (press_type == SHORT_PRESS) {
        current_track = (current_track + 1) % 99;
      } else {
        state = SOUND_PLAY;
      }
      tft_update = true;
      break;

    case SOUND_PLAY:
      Serial.println("Play selected!");
      state = SOUND_PLAYING;
      tft_update = true;
      break;

    case VOLUME_UP:
      if (droid_volume > 90) {
        droid_volume = 100;
      } else {
        droid_volume += 10;
      }
      tft_update = true;
      break;

    case VOLUME_DOWN:
      if (droid_volume < 10) {
        droid_volume = 0;
      } else {
        droid_volume -= 10;
      }
      tft_update = true;
      break;

    case VOLUME_TEST:
      state = VOLUME_TESTING;
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
      state = TOP_MENU;
      selected_item = 0;
      tft_update = true;
      break;

    case TOP_MENU:
      if (press_type == LONG_PRESS) {
        state = SPLASH;
        selected_item = 0;
      } else {
        selected_item++;
        if (selected_item >= sizeof(top_menu) / sizeof(menu_item_t)) {
          selected_item = 0;
        }
      }
      tft_update = true;
      break;

    case BEACON_TYPE_MENU:
      if (press_type == LONG_PRESS) {
        state = TOP_MENU;
        selected_item = 1;  // beacon
      } else {
        selected_item++;
        if (selected_item >= sizeof(beacon_type_menu) / sizeof(menu_item_t)) {
          selected_item = 0;
        }
      }
      tft_update = true;
      break;

    case BEACON_LOCATION_LIST:
      if (press_type == LONG_PRESS) {
        state = BEACON_TYPE_MENU;
        selected_item = 0;
      } else {
        selected_item++;
        if (selected_item >= sizeof(location_beacons) / sizeof(beacon_item_t)) {
          selected_item = 0;
        }
      }
      tft_update = true;
      break;

    case BEACON_DROID_LIST:
      if (press_type == LONG_PRESS) {
        state = BEACON_TYPE_MENU;
        selected_item = 1;
      } else {
        selected_item++;
        if (selected_item >= sizeof(droid_beacons) / sizeof(beacon_item_t)) {
          selected_item = 0;
        }
      }
      tft_update = true;
      break;

    case BEACON_ACTIVATE:
      if (press_type == LONG_PRESS) {
        if (beacon.type == DROID) {
          state = BEACON_DROID_LIST;
        } else {
          state = BEACON_LOCATION_LIST;
        }
      } else {
        init_advertisement_data();
        set_payload_from_beacon();
        pAdvertising->start();
        state = BEACON_ACTIVE;
      }
      tft_update = true;
      break;

    case BEACON_ACTIVE:
      pAdvertising->stop();
      if (press_type == LONG_PRESS) {
        if (beacon.type == DROID) {
          state = BEACON_DROID_LIST;
        } else {
          state = BEACON_LOCATION_LIST;
        }
      } else {
        state = BEACON_ACTIVATE;
      }
      tft_update = true;
      break;

    case BEACON_EXPERT:
      if (press_type == LONG_PRESS) {
        state = BEACON_TYPE_MENU;
        selected_item = 3;
      } else {
        selected_item++;
        if (selected_item >= NUM_BEACON_PARAMS+2) {
          selected_item = 0;
        }
      }
      tft_update = true;
      break;

    case BEACON_EXPERT_ACTIVE:
      pAdvertising->stop();
      if (press_type == LONG_PRESS) {
        state = BEACON_TYPE_MENU;
        selected_item = 3;
      } else {
        state = BEACON_EXPERT;
      }
      tft_update = true;
      break;

    case SCANNER_RESULTS:
      if (press_type == LONG_PRESS || droid_count < 1) {
        state = TOP_MENU;
        selected_item = 0;    // scanner
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

    case CONNECTED_MENU:
      if (press_type == SHORT_PRESS) {
        selected_item++;
        if (selected_item >= sizeof(connected_menu) / sizeof(menu_item_t)) {
          selected_item = 0;
        }
      } else {
        droid_disconnect();
        state = SCANNER_RESULTS;
      }
      tft_update = true;
      break;

    case SOUND_GROUP:
    case SOUND_TRACK:
    case SOUND_PLAY:
      if (press_type == SHORT_PRESS) {
        switch (state) {
          case SOUND_GROUP:
            state = SOUND_TRACK;
            break;
          case SOUND_TRACK:
            state = SOUND_PLAY;
            break;
          case SOUND_PLAY:
            state = SOUND_GROUP;
            break;
        }
      } else {
        state = CONNECTED_MENU;
        selected_item = 0;          // todo: don't hardcode this
      }
      tft_update = true;
      break;

    case VOLUME_UP:
    case VOLUME_DOWN:
    case VOLUME_TEST:
      if (press_type == SHORT_PRESS) {
        switch(state) {
          case VOLUME_UP:
            state = VOLUME_DOWN;
            break;
          case VOLUME_DOWN:
            state = VOLUME_TEST;
            break;
          case VOLUME_TEST:
            state = VOLUME_UP;
            break;
        }
      } else {
        state = CONNECTED_MENU;
        selected_item = 1;          // todo: don't hardcode this
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

  // define deep sleep wakeup trigger; if commented out ESP32 goes into hibernation instead of deep sleep and only wakes up with the reset button
  // memory is lost from deep sleep; for our purposes deep sleep and hibernation are the same thing
  //esp_sleep_enable_ext0_wakeup(WAKEUP_BUTTON, WAKEUP_LEVEL);

  // initialize the sleep monitor timer
  last_activity = millis();

  // initialize the droid array
  for (i = 0; i < MAX_DROIDS; i++) {
    droids[i].pAdvertisedDevice = nullptr;
  }

  // just so there's no garbage in there if it gets used before being initilized.
  set_random_beacon();

  // init serial monitor
  Serial.begin(115200);
  Serial.println("Ready!");
}

void loop() {

  button_handler();

  switch (state) {

    case SCANNER_SCANNING:
      update_display();
      ble_scan();
      current_droid = 0;
      state = SCANNER_RESULTS;
      tft_update = true;
      break;

    case SCANNER_CONNECTING:
      update_display();
      if (droid_connect()) {
        state = SCANNER_CONNECTED;
      } else {
        state = SCANNER_CONNECT_FAILED;
      }
      tft_update = true;
      break;

    case SCANNER_CONNECTED:
      delay(2000);
      state = CONNECTED_MENU;
      tft_update = true;
      break;

    case SCANNER_CONNECT_FAILED:
      delay(2000);
      state = SCANNER_RESULTS;
      tft_update = true;
      break;

    case SOUND_PLAYING:
      update_display();
      Serial.println("Playing sound...");
      droid_play_track();
      delay(2000);
      state = SOUND_PLAY;
      tft_update = true;
      break;

    case VOLUME_TESTING:
      update_display();
      Serial.println("Testing volume...");
      droid_set_volume();
      droid_play_track();
      //droid_play_next_track();
      delay(2000);
      state = VOLUME_TEST;
      tft_update = true;
      break;
  }

  update_display();

  // inactivity sleep check; screen will blank when ESP32 goes to sleep
  // press reset button to wakeup
  if (millis() - last_activity > SLEEP_AFTER) {

    // do not go to sleep if the beacon is active
    if (state != BEACON_ACTIVE && state != BEACON_EXPERT_ACTIVE) {
      Serial.println("Going to sleep.");
      delay(100);
      esp_deep_sleep_start();
    } else {
      last_activity = millis();
    }
  }
}
