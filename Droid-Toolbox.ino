/* Droid Toolbox v0.64 : ruthsarian@gmail.com
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
 *    After installing or updating the TFT_eSPI library you MUST edit User_Setup_Select.h as follows 
 *      1. comment out the line "#include <User_Setup.h>" (line 22-ish)
 *      2. uncomment the line "#include <User_Setups/Setup25_TTGO_T_Display.h>" (line 61-ish) for T-Display
 *         or "#include <User_Setups/Setup206_LilyGo_T_Display_S3.h>" for the T-Display-32
 *
 *    Possible path for Windows users: %USERPROFILE%\Documents\Arduino\libraries\TFT_eSPI\User_Setup_Select.h
 *
 *  NOTE 2:
 *    The T-Display-S3 may display the wrong colors. If this happens you'll also need to modify 
 *    User_Setups/Setup206_LilyGo_T_Display_S3.h in the TFT_eSPI library, locate the two lines, one commented out,
 *    that begin with "#define TFT_RGB_ORDER". Uncomment the commented-out line, and comment out the uncommented line.
 *    Then reupload the sketch and the colors should be correct.
 *
 *    #define TFT_RGB_ORDER TFT_RGB
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
 *   consider a method for setting font sizes based on screen size
 *   is there any value in scanning for SWGE East/West beacon (used by the Disney Play app) and identifying which location you're in based off that?
 *   ability save beacons that are defined in EXPERT mode?
 *
 * HISTORY
 *   v0.64 : added a BLE advertising parameter that should prevent other devices from connecting to the toolbox while it is advertising
 *           a beacon. previously, if a device did attempt such a connection, the beacon on the toolbox would stop, but you wouldn't know it.
 *   v0.63 : expert mode can now create droid beacons that will be seen by other toolboxes; you cannot connect to emulated beacons
 *           adding this feature lead me to rewrite a lot of the expert beacon display code
 *           added SERIAL_DEBUG_ENABLE which, if not defined, will prevent messages from being sent over the serial monitor
 *           used the newly created droid beacon feature to identify and a bug with the scanner that caused a crash when encountering an unknown personality chip
 *   v0.62 : added battery/power voltage display on the splash screen. 
 *           displayed value is only updated when splash screen is loaded; this is not a realtime monitor.
 *           thanks to Tom Harris for suggesting this feature and providing some code to work with!
 *   v0.61 : reworked how personality, location, and affiliation data is stored in the code to make modification of that data a little easier.
 *           added ability to display custom names for droids based off their bluetooth address. custom names can be added to the named_droids[] array in the code below.
 *   v0.60 : added ability to select which beacon the droid toolbox will produce
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
 *           Set delay between beacon reactions to 1 minute; BD units CORRECTLY interpret the value of 0x02 as a 10 second delay.
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

#define C565(r,g,b)                         ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)    // macro to convert RGB values to TFT_eSPI color value

// CUSTOMIZATIONS BEGIN -- These values can be changed to alter Droid Toolbox's behavior.

#define MSG_VERSION                         "v0.64"                 // the version displayed on the splash screen at the lower right

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

#define SOUNDS_TEXT_SIZE                    (DEFAULT_TEXT_SIZE + 1)
#define SOUNDS_TEXT_PADDING                 DEFAULT_TEXT_PADDING
#define SOUNDS_GROUP_COLOR                  TFT_BLUE
#define SOUNDS_TRACK_COLOR                  TFT_RED
#define SOUNDS_VALUE_COLOR                  TFT_DARKGREY
#define SOUNDS_VALUE_SELECTED_COLOR         TFT_GREEN

#define VOLUME_TEXT_SIZE                    (DEFAULT_TEXT_SIZE + 1)
#define VOLUME_TEXT_PADDING                 DEFAULT_TEXT_PADDING
#define VOLUME_LOW_COLOR                    TFT_CYAN
#define VOLUME_MED_COLOR                    TFT_YELLOW
#define VOLUME_HIGH_COLOR                   TFT_RED
#define VOLUME_MAX_COLOR                    TFT_MAGENTA
#define VOLUME_TEXT_COLOR                   TFT_DARKGREY
#define VOLUME_SELECTED_TEXT_COLOR          TFT_GREEN

#define SLEEP_AFTER                         5 * 60 * 1000  // how many milliseconds of inactivity before going to sleep/hibernation
#define SERIAL_DEBUG_ENABLE                 // uncomment to enable serial debug/monitor messages

// static strings used throughout DroidToolbox
const char ble_adv_name[]               = "DROIDTLBX";              // this is the name the toolbox's beacon will appear as, keep to 10 characters or less
const char ble_adv_name_droid[]         = "DROID";
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
const char msg_int_minutes[]            = "%d mins";
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
const char msg_group[]                  = "Group";
const char msg_track[]                  = "Track";
const char msg_play[]                   = "PLAY";
const char msg_vol_inc[]                = "VOL+";
const char msg_vol_dec[]                = "VOL-";
const char msg_set_vol[]                = "SET VOLUME";

typedef struct {
  uint8_t id;           // personality ID, unique for each personality
  const char *name;     // the personality name; displayed on screen
  uint8_t affiliation;  // the group affiliation (see droid_affiliations[] for relevant values); only used when emulating droid beacons
  uint8_t emulatable;   // 0 = will not be included in list of droid beacons available for emulation
} personality_t;

//
// Droid Personalities
//
// Add custom personality chips to this array!
//
// the (E)mulatable flag identifies whether or not that personality will be available as a droid beacon
//
// WHY WOULD I NOT WANT A PERSONALITY TO BE EMULATABLE?
//   - to customize the droid beacon list to something smaller/manageable; removing those you don't use
//   - to hide custom personality chip identities from being emulatable
//   - because the programmer created this 'feature' for another reason, then rewrote the code making the 'feature' useless
//
personality_t droid_personalities[] = {
//  ID,   NAME,       AFF,  E
  { 0x01, "R Unit",   0x01, 1},
  { 0x02, "BB Unit",  0x01, 1},
  { 0x03, "Blue",     0x05, 1},
  { 0x04, "Gray",     0x01, 1},
  { 0x05, "Red",      0x09, 1},
  { 0x06, "Orange",   0x05, 1},
  { 0x07, "Purple",   0x01, 1},
  { 0x08, "Black",    0x09, 1},
  { 0x09, "CB-23",    0x01, 1},
  { 0x0A, "Yellow",   0x05, 1},
  { 0x0B, "C1-10P",   0x05, 1},
  { 0x0C, "D-O",      0x05, 1},
  { 0x0D, "Blue 2",   0x01, 1},
  { 0x0E, "BD Unit",  0x05, 1},
};

#define DROID_PERSONALITIES_SIZE sizeof(droid_personalities)/sizeof(personality_t)
personality_t** emulatable_personalities;
uint8_t emulatable_personalities_size;

typedef struct {
  const char* name;
  esp_bd_addr_t bleaddr;
} name_by_bleaddr_t;

//
// Custom Droid Names
//
// Use this array to add custom names for individual droids. This is based on bluetooth address.
// The bluetooth address is included in the scanner results
name_by_bleaddr_t named_droids[] = {
// LABEL,      6-BYTE BLEADDR
  {"Frank",    {0xc2, 0xb1, 0x05, 0x2b, 0x1f, 0x91}}, // "c2:b1:05:2b:1f:91"
};

#define NAMED_DROID_SIZE  sizeof(named_droids)/sizeof(name_by_bleaddr_t)

typedef struct {
  uint8_t id;
  const char *name;
} affiliaton_t;

//
// Droid Affiliations
//
// You could add custom droid affiliations here if you so choose, but they may not play well
// with droids. I recommend values between 0x00 and 0x0B. A value from 0x0C to 0x0F will cause 
// a droid to NOT react to the droid beacon. A value up to 0x3F could be used, but the issue
// of no reaction for values from 0x_C to 0x_F will persist. 
//
// Droids are programmed to only recognize the three base affiliations. Custom affiliation
// values will fall into one of these groups:
//     Scoundrel: 0x00 - 0x03
//    Resistance: 0x04 - 0x07
//   First Order: 0x08 - 0x0B
//   No Reaction: 0x0C - 0x0F
//
affiliaton_t droid_affiliations[] = {
// ID,    NAME
  { 0x01, "Scoundrel"   },
  { 0x05, "Resistance"  },
  { 0x09, "First Order" },
};

#define DROID_AFFILIATIONS_SIZE  sizeof(droid_affiliations)/sizeof(affiliaton_t)

typedef struct {
  uint8_t id;
  const char *name;
} location_t;

//
// Location Beacons
//
// The beacons given below are known to exist in Galaxy's Edge (except for 0x04 which has
// not yet been observed in Galaxy's Edge.) There's probably not much reason to add 'custom'
// location beacons as your droid is programmed to ignore any value at or above 0x08. However 
// you could change the names of the beacons to anything you want, especially if you're using 
// these beacons in an environment outside of Galaxy's Edge!
//
// NOTE: a droid does "see" a location beacon with a value of 0, however it will not react
//
location_t locations[] = {
// ID,    NAME
  { 0x01, "Marketplace"  },
  { 0x02, "Droid Depot"  },
  { 0x03, "Resistance"   },
  { 0x04, "Unknown"      },
  { 0x05, "Alert"        },
  { 0x06, "Dok Ondar's"  },
  { 0x07, "First Order"  },
};

#define LOCATIONS_SIZE  sizeof(locations)/sizeof(location_t)

#define NUM_BEACON_PARAMS 4

// next two arrays are labels for the expert beacon screen
const char* msg_beacon_droid_param[NUM_BEACON_PARAMS] = {
  "CHIP",
  "AFFL",
  "PAIRED",
  "B DROID"
};
const char* msg_beacon_location_param[NUM_BEACON_PARAMS] = {
  "LCTN", 
  "REACT INT", 
  "MIN RSSI",
  nullptr
};

// CUSTOMIZATIONS END -- In theory you shouldn't have to edit anything below this line.

// there has got to be a better way to manage expert beacon parameter variables
// like determinig their length on-the-fly rather than hard-coded NUM_BEACON_PARAMS
// but for now it's all about just making it work. i make it pretty later on.

// define array index values for msg_beacon_droid_param[]
#define BEACON_PARAM_DROID_ID     0
#define BEACON_PARAM_DROID_AFFL   1
#define BEACON_PARAM_DROID_PAIRED 2
#define BEACON_PARAM_DROID_BDROID 3

// define array index values for msg_beacon_location_param[]
#define BEACON_PARAM_LCTN_ID      0
#define BEACON_PARAM_LCTN_REACT   1
#define BEACON_PARAM_LCTN_RSSI    2
#define BEACON_PARAM_LCTN_PARAM4  3

typedef enum {
  DROID,
  LOCATION,
} beacon_type_t;

typedef struct {
  beacon_type_t type;                 // DROID or LOCATION
  uint8_t setting[NUM_BEACON_PARAMS]; // 0 = droid:personality[1:14],  location:location[1:7]
                                      // 1 = droid:affiliation[1,5,9], location:interval[1:20]
                                      // 2 = droid:paired[0,1],        location:minimum_rssi[58:28]
                                      // 3 = droid:bdroid[0,1],        location:unused
} beacon_t;
beacon_t beacon;

#if (defined(ARDUINO_ESP32S3_DEV) || defined(ARDUINO_LILYGO_T_DISPLAY_S3))  // this is assuming you're compiling for T-Display-S3 using the "ESP32S3 Dev Module" or "LilyGo T-Display-S3" board.
  #define TDISPLAYS3
#endif

#define BUTTON1_PIN       0   // button 1 on the TTGO T-Display and T-Display-S3 is GPIO 0
#ifdef TDISPLAYS3          
  #define BUTTON2_PIN     14  // button 2 on the T-Display-S3 is GPIO14
  #define BAT_ADC_PIN     4   // battery monitor pin
#else
  #define BUTTON2_PIN     35  // button 2 on the TTGO T-Display is GPIO 35
  #define BAT_ADC_PIN     34  // battery monitor pin
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

// Using these macros to print debug messages will make it easier to disable the printing of those messages by undefining SERIAL_DEBUG_ENABLE
#ifdef SERIAL_DEBUG_ENABLE
  #define SERIAL_BEGIN(x)       Serial.begin(x)
  #define SERIAL_PRINT(x)       Serial.print(x)
  #define SERIAL_PRINTLN(x)     Serial.println(x)
  #define SERIAL_PRINT2(x,y)    Serial.print(x,y)
  #define SERIAL_PRINTLN2(x,y)  Serial.println(x,y)
#else
  #define SERIAL_BEGIN(x)
  #define SERIAL_PRINT(x)
  #define SERIAL_PRINTLN(x)
  #define SERIAL_PRINT2(x,y)
  #define SERIAL_PRINTLN2(x,y)
#endif

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

TFT_eSPI tft = TFT_eSPI();      // display interface
bool tft_update = true;         // flag to indicate display needs to be updated

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

personality_t* get_droid_personality(uint8_t id) {
  uint8_t i;
  for (i=0; i<DROID_PERSONALITIES_SIZE; i++) {
    if (droid_personalities[i].id == id) {
      return(&droid_personalities[i]);
    }
  }
  return nullptr;
}

affiliaton_t* get_droid_affiliation(uint8_t id) {
  uint8_t i;
  for (i=0; i<DROID_AFFILIATIONS_SIZE; i++) {
    if (droid_affiliations[i].id == id) {
      return(&droid_affiliations[i]);
    }
  }
  return nullptr;
}

location_t* get_location(uint8_t id) {
  uint8_t i;
  for (i=0; i<LOCATIONS_SIZE; i++) {
    if (locations[i].id == id) {
      return(&locations[i]);
    }
  }
  return nullptr;
}

void init_advertisement_data(const char* name) {
  if (pAdvertisementData != nullptr) {
    delete pAdvertisementData;
  }
  pAdvertisementData = new BLEAdvertisementData();
  if (name == nullptr) {
    pAdvertisementData->setName(ble_adv_name);
  } else {
    pAdvertisementData->setName(name);
  }
}

void load_payload_location_beacon_data() {
  memcpy(payload, SWGE_LOCATION_BEACON_PAYLOAD, sizeof(uint8_t) * PAYLOAD_SIZE);
}

void load_payload_droid_beacon_data() {
  memcpy(payload, SWGE_DROID_BEACON_PAYLOAD, sizeof(uint8_t) * PAYLOAD_SIZE);
}

// populate beacon struct with a droid beacon
void set_droid_beacon(uint8_t personality) {
  uint8_t i;
  personality_t* p;
  affiliaton_t* a; 
  char msg[MSG_LEN_MAX];

  // get the personality
  p = get_droid_personality(personality);

  // do we have a valid personality?
  if (p != nullptr && p->emulatable != 0) {

    // create a droid beacon
    SERIAL_PRINTLN("Creating a DROID beacon.");
    beacon.type = DROID;

    // populate the beacon values
    beacon.setting[BEACON_PARAM_DROID_ID]     = p->id;
    beacon.setting[BEACON_PARAM_DROID_AFFL]   = p->affiliation;
    beacon.setting[BEACON_PARAM_DROID_PAIRED] = 1;
    beacon.setting[BEACON_PARAM_DROID_BDROID] = 0;

    // output personality name to serial
    SERIAL_PRINT("  Personality: ");
    SERIAL_PRINTLN(p->name);

    // output affiliation to serial
    SERIAL_PRINT("  Affiliation: ");
    a = get_droid_affiliation(p->affiliation);
    if (a != nullptr) {
      SERIAL_PRINTLN(a->name);
    } else {
      snprintf(msg, MSG_LEN_MAX, msg_unknown_int, p->affiliation);
      SERIAL_PRINTLN(msg);
    }
  } else {
    // pick a random droid beacon, only pick from emulatable personalities
    set_droid_beacon(emulatable_personalities[esp_random() % emulatable_personalities_size]->id);
  }
}

void set_location_beacon(uint8_t location) {
  uint8_t i;
  location_t* l;

  // get the location
  l = get_location(location);

  // do we have a valid location?
  if (l != nullptr) {

    // create a location beacon
    beacon.type = LOCATION;
    SERIAL_PRINTLN("Creating a LOCATION beacon.");

    // set the bacon location value
    beacon.setting[BEACON_PARAM_LCTN_ID] = l->id;
    SERIAL_PRINT("  Location: ");
    SERIAL_PRINTLN(l->name);

    // set reaction interval (in minutes), could go as high as 19, but keeping it low on purpose
    //beacon.setting[BEACON_PARAM_LCTN_REACT] = (esp_random() % 3) + 1;
    beacon.setting[BEACON_PARAM_LCTN_REACT] = 2;
    SERIAL_PRINT("  Interval: ");
    SERIAL_PRINTLN(beacon.setting[BEACON_PARAM_LCTN_REACT]);

    // set minimum RSSI for droid to react; while this value is stored as an unsigned value, think of it as a negative value in dBm; e.g. 38 = -38dBm
    beacon.setting[BEACON_PARAM_LCTN_RSSI] = 38;
    SERIAL_PRINT("  Minimum RSSI: -");
    SERIAL_PRINT(beacon.setting[BEACON_PARAM_LCTN_RSSI]);
    SERIAL_PRINTLN("dBm");

    // is this even necessary? we don't use this...
    beacon.setting[BEACON_PARAM_LCTN_PARAM4] = 0;
  } else {

    // if no valid location was found, select one of the known locations at random
    set_location_beacon(locations[esp_random() % LOCATIONS_SIZE].id);
  }
}

// populate the global beacon variable with random(ish) values
void set_random_beacon() {
  SERIAL_PRINTLN("Generating a random beacon.");
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
    payload[5] = 0x01 + (beacon.setting[BEACON_PARAM_DROID_PAIRED] > 0 ? 0x80 : 0x00);

    // set affiliation
    payload[6] = 0x80 + (beacon.setting[BEACON_PARAM_DROID_AFFL] * 2);

    // set personality chip id
    payload[7] = beacon.setting[BEACON_PARAM_DROID_ID];

  // LOCATION beacon type
  } else {
    load_payload_location_beacon_data();

    // set location
    payload[4] = beacon.setting[BEACON_PARAM_LCTN_ID];

    // set reaction interval
    payload[5] = (beacon.setting[BEACON_PARAM_LCTN_REACT] % 20) * 12;

    // set minimum RSSI for reaction
    payload[6] = 0x80 + beacon.setting[BEACON_PARAM_LCTN_RSSI];
  }

  // load the payload into the advertisement data
  pAdvertisementData->setManufacturerData(std::string(reinterpret_cast<char*>(payload), PAYLOAD_SIZE));
  pAdvertising->setAdvertisementData(*pAdvertisementData);

  // prevent connection attempts while advertising a beacon
  pAdvertising->setAdvertisementType(ADV_TYPE_NONCONN_IND);
  pAdvertising->setScanFilter(false, true);
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
      SERIAL_PRINTLN("delete old AdvertsidedDevice object");
      delete droids[pos].pAdvertisedDevice;
    }

    // store the droid's AdvertsiedDevice object
    droids[pos].pAdvertisedDevice = new BLEAdvertisedDevice(advertisedDevice);

    SERIAL_PRINT("Droid Found -- BD_ADDR: ");
    SERIAL_PRINT(droids[pos].pAdvertisedDevice->getAddress().toString().c_str());
    SERIAL_PRINT(", RSSI: ");
    SERIAL_PRINTLN(advertisedDevice.getRSSI());

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

  SERIAL_PRINT("Notify callback for characteristic: ");
  SERIAL_PRINT(pBLERemoteCharacteristic->getUUID().toString().c_str());
  SERIAL_PRINT(" of data length ");
  SERIAL_PRINTLN(length);
  SERIAL_PRINT("data: ");

  // SERIAL_PRINTLN((char*)pData);
  for (i = 0; i < length; i++) {

    // print hex value
    SERIAL_PRINT2(pData[i], HEX);
    SERIAL_PRINT(" ");

    // newline
    if ((i % 16 == 0 && i > 0) || (i == (length - 1))) {
      if (i == (length - 1)) {
        j = i - (i % 16);
      } else {
        j = i - 16;
      }
      SERIAL_PRINT("    [ ");
      for (; j < i; j++) {
        SERIAL_PRINT((char)pData[j]);
      }
      SERIAL_PRINTLN(" ]");
    }
  }
  SERIAL_PRINTLN();
}

void droid_disconnect() {
  // disconnect from any previous connection
  if (pClient != nullptr && pClient->isConnected()) {
    SERIAL_PRINTLN("Disconnecting from droid.");
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
  SERIAL_PRINT("Connecting to ");
  SERIAL_PRINTLN(droids[current_droid].pAdvertisedDevice->getAddress().toString().c_str());
  if (!pClient->connect(droids[current_droid].pAdvertisedDevice)) {
    SERIAL_PRINTLN("Connection failed.");
    return false;
  }

  // locate the service we want to talk to
  pRemoteService = pClient->getService(serviceUUID);
  if (pRemoteService == nullptr) {
    SERIAL_PRINTLN("Failed to find droid service.");
    droid_disconnect();
    return false;
  }

  // get the characteristic that receives commands
  pRemoteCharacteristicCmd = pRemoteService->getCharacteristic(cmdUUID);
  if (pRemoteCharacteristicCmd == nullptr) {
    SERIAL_PRINTLN("Failed to find droid chracteristic.");
    droid_disconnect();
    return false;
  }
  pRemoteCharacteristicCmd->registerForNotify(notifyCallback);

  // get the characteristic that receives notifications from the droid
  pRemoteCharacteristicNotify = pRemoteService->getCharacteristic(notifyUUID);
  if (pRemoteCharacteristicNotify == nullptr) {
    SERIAL_PRINTLN("Failed to find droid chracteristic.");
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

    SERIAL_PRINT("Playing track ");
    SERIAL_PRINT(current_group);
    SERIAL_PRINT("-");
    SERIAL_PRINTLN(current_track);

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

    SERIAL_PRINT("New volume: 0x");
    SERIAL_PRINTLN2(cmd_set_volume[7], HEX);

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
  SERIAL_PRINT(foundDevices.getCount());
  SERIAL_PRINTLN(" BLE devices seen.");

  // delete results fromBLEScan buffer to release memory
  pBLEScan->clearResults();
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
    tft.drawString(items[i], tft.getViewportWidth()/2, y + row_padding + 1);    // the +1 here is to compensate for the box being slightly vertically off-center

    // increment y for the next list item
    y += row_height;
  }
}

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

void display_beacon_menu(const char* caption, const char** list_items, uint8_t num_items) {
  uint8_t h, i;

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

  // display the menu
  display_list(list_items, num_items);
}

void display_droid_beacon_list() {

  uint8_t i, num_items = 0;
  const char** list_items;

  num_items = emulatable_personalities_size;

  // create a char* array to pass to display_list()
  list_items = (const char**)malloc(sizeof(char*)*num_items);

  for(i=0; i<num_items; i++) {
    list_items[i] = emulatable_personalities[i]->name;
  }

  // display the menu
  display_beacon_menu(msg_select_beacon, list_items, num_items);

  // free the malloc'd memory
  free(list_items);
}

// generate a list of locations to choose from and display them in a menu
void display_location_beacon_list() {
  uint8_t i, num_items;
  const char** list_items;

  num_items = LOCATIONS_SIZE;

  // create a char* array to pass to display_list()
  list_items = (const char**)malloc(sizeof(char*)*num_items);

  for(i=0; i<num_items; i++) {
    list_items[i] = locations[i].name;
  }

  // display the menu
  display_beacon_menu(msg_select_beacon, list_items, num_items);

  // free the malloc'd memory
  free(list_items);
}

void display_beacon_control() {
  uint16_t y, content_height;
  uint8_t gap, bfs;
  char msg[MSG_LEN_MAX];
  personality_t *p;
  location_t* l;

  // line 1: beacon type
  // line 2: beacon id
  // line 3: gap
  // line 3: beacon state

  tft.setTextSize(1);
  bfs = tft.fontHeight();
  
  gap = (bfs * BEACON_CONTROL_TEXT_SIZE) / 2;
  SERIAL_PRINT("BC:gap = ");
  SERIAL_PRINTLN(gap);

  content_height = (bfs * BEACON_CONTROL_TYPE_TEXT_SIZE) + (bfs * BEACON_CONTROL_TEXT_SIZE *2) + gap + (BEACON_CONTROL_TEXT_PADDING * 3);
  SERIAL_PRINT("BC:content_height = ");
  SERIAL_PRINTLN(content_height);

  // calculate where to begin drawing text  
  y = (tft.getViewportHeight() - content_height)/2;

  SERIAL_PRINT("BC:y = ");
  SERIAL_PRINTLN(y);
  SERIAL_PRINTLN();

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
    p = get_droid_personality(beacon.setting[BEACON_PARAM_DROID_ID]);
    if (p != nullptr) {
      snprintf(msg, MSG_LEN_MAX, "%s", p->name);
    } else {
      snprintf(msg, MSG_LEN_MAX, msg_unknown_int, beacon.setting[BEACON_PARAM_DROID_ID]);
    }
  } else {
    l = get_location(beacon.setting[BEACON_PARAM_LCTN_ID]);
    if (l != nullptr) {
      snprintf(msg, MSG_LEN_MAX, "%s", l->name);
    } else {
      snprintf(msg, MSG_LEN_MAX, msg_unknown_int, beacon.setting[BEACON_PARAM_LCTN_ID]);
    }
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
  char msg[MSG_LEN_MAX];

  // surely there's an easier way to vertically position the splash screen text
  tft.setTextSize(1);
  y = (tft.height() - ((tft.fontHeight() * (SPLASH_TEXT_SIZE + 1)) + (tft.fontHeight() * SPLASH_TEXT_SIZE * 5))) / 2;
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
  tft.drawString(msg_version, tft.width()-0, tft.height());

  // battery voltage
  tft.setTextDatum(BL_DATUM);
  y = (analogRead(BAT_ADC_PIN) * 2 * 3.3 * 1000) / 4096;
  if (y < 3400) {
    tft.setTextColor(C565(128,0,0));        // need to charge the battery
  } else if (y < 3800) {
    tft.setTextColor(C565(96,96,0));        // battery is getting low
  } else if (y < 4400) {
    tft.setTextColor(C565(0,128,0));        // battery is charged
  } else {
    tft.setTextColor(SPLASH_VERSION_COLOR); // you're probably on USB
  }
  snprintf(msg, MSG_LEN_MAX, "%s:%.2fV", (y<4400 ? "BAT" : "PWR"), (y / (float)1000));
  tft.drawString(msg, 0, tft.height());
}

void display_scanner_results() {
  char msg[MSG_LEN_MAX];
  uint16_t y = 0;
  uint8_t i;
  const char* name = nullptr;
  affiliaton_t* a = nullptr;
  personality_t* p = nullptr;

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

    // look for a known ble address first; this takes precedence over anything else
    for (i=0; i<NAMED_DROID_SIZE; i++) {
      if (memcmp(named_droids[i].bleaddr, droids[current_droid].pAdvertisedDevice->getAddress().getNative(), ESP_BD_ADDR_LEN) == 0) {
        name = named_droids[i].name;
        break;
      }
    }

    // if a ble address match was not found, grab the details of the droid based on ID
    if (name == nullptr) {

      // search known personalities
      p = get_droid_personality(droids[current_droid].chipid);

      // if a known personality was found, extract its name
      if (p != nullptr) {
        name = get_droid_personality(droids[current_droid].chipid)->name;
      }
    }

    // print personality name
    if (name != nullptr) {
      tft.drawString(name, tft.width()/2, y);
    } else {
      snprintf(msg, MSG_LEN_MAX, msg_unknown_int, droids[current_droid].chipid);
      tft.drawString(msg, tft.width()/2, y);
    }

    // print droid affiliation
    y += tft.fontHeight();
    tft.setTextColor(DROID_REPORT_AFFILIATION_COLOR);
    tft.setTextSize(DROID_REPORT_TEXT_SIZE);

    // stock affiliation
    a = get_droid_affiliation(droids[current_droid].affid);
    if (a != nullptr) {
      tft.drawString(a->name, tft.width()/2, y);
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

void display_sounds() {
  char msg[MSG_LEN_MAX];
  uint16_t x, y;

  // line1: group
  // line2: track
  // line3: gap
  // line4: play

  // display instruction
  tft.setTextSize(SOUNDS_TEXT_SIZE);

  // let's figure out how to center all this 
  snprintf(msg, MSG_LEN_MAX, "%s: ", msg_group);
  x = tft.textWidth(msg);
  snprintf(msg, MSG_LEN_MAX, "%s: ", msg_track);
  if (tft.textWidth(msg) > x) {
    x = tft.textWidth(msg);
  }

  // the vertical split is located
  x = ((tft.getViewportWidth() - (x + tft.textWidth("88")))/2) + x;

  // find the starting point to draw text
  y = (tft.fontHeight() * 4) + SOUNDS_TEXT_PADDING;
  y = (tft.getViewportHeight() - y)/2;

  // display group label
  snprintf(msg, MSG_LEN_MAX, "%s: ", msg_group);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(SOUNDS_GROUP_COLOR);
  tft.drawString(msg, x, y);

  // display group value
  snprintf(msg, MSG_LEN_MAX, "%d", (current_group + 1));
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor((state == SOUND_GROUP) ? SOUNDS_VALUE_SELECTED_COLOR : SOUNDS_VALUE_COLOR);
  tft.drawString(msg, x, y);

  // move next line
  y += tft.fontHeight() + SOUNDS_TEXT_PADDING;

  // display track label
  snprintf(msg, MSG_LEN_MAX, "%s: ", msg_track);
  tft.setTextDatum(TR_DATUM);
  tft.setTextColor(SOUNDS_TRACK_COLOR);
  tft.drawString(msg, x, y);

  // display track value
  snprintf(msg, MSG_LEN_MAX, "%d", (current_track + 1));
  tft.setTextDatum(TL_DATUM);
  tft.setTextColor((state == SOUND_TRACK) ? SOUNDS_VALUE_SELECTED_COLOR : SOUNDS_VALUE_COLOR);
  tft.drawString(msg, x, y); 

  // move down to draw play button
  y += (tft.fontHeight() * 2);

  // display play button
  tft.setTextDatum(TC_DATUM);
  tft.setTextColor((state == SOUND_PLAY) ? SOUNDS_VALUE_SELECTED_COLOR : SOUNDS_VALUE_COLOR);
  tft.drawString(msg_play, tft.getViewportWidth()/2, y);
}

void display_volume() {
  char msg[MSG_LEN_MAX];
  uint16_t content_height;
  uint16_t x, y;

  // line1: volume
  // line2: gap
  // line3: vol+ / vol-
  // line4: set volume

  // find where to start rendering content vertically
  tft.setTextSize(1);
  content_height = (tft.fontHeight() * VOLUME_TEXT_SIZE * 3) + VOLUME_TEXT_PADDING + (tft.fontHeight() * (VOLUME_TEXT_SIZE + 2));
  y = (tft.getViewportHeight() - content_height) / 2;

  // set volume color based on its value
  if (droid_volume < 50) {
    tft.setTextColor(VOLUME_LOW_COLOR);
  } else if (droid_volume < 80) {
    tft.setTextColor(VOLUME_MED_COLOR);
  } else if (droid_volume < 100) {
    tft.setTextColor(VOLUME_HIGH_COLOR);
  } else {
    tft.setTextColor(VOLUME_MAX_COLOR);
  }

  // display volume
  snprintf(msg, MSG_LEN_MAX, "%d", droid_volume);
  tft.setTextDatum(TC_DATUM);
  tft.setTextSize(VOLUME_TEXT_SIZE + 2);
  tft.drawString(msg, tft.getViewportWidth()/2, y);

  // next line
  y += tft.fontHeight();
  
  // gap
  tft.setTextSize(VOLUME_TEXT_SIZE);
  y += tft.fontHeight();

  // vol+
  tft.setTextColor((state == VOLUME_UP ? VOLUME_SELECTED_TEXT_COLOR : VOLUME_TEXT_COLOR));
  tft.setTextDatum(TR_DATUM);
  tft.drawString(msg_vol_inc, (tft.getViewportWidth()/2) - VOLUME_TEXT_PADDING, y);

  // vol-
  tft.setTextColor((state == VOLUME_DOWN ? VOLUME_SELECTED_TEXT_COLOR : VOLUME_TEXT_COLOR));
  tft.setTextDatum(TL_DATUM);
  tft.drawString(msg_vol_dec, (tft.getViewportWidth()/2) + VOLUME_TEXT_PADDING, y);

  // next line
  y += (tft.fontHeight() + VOLUME_TEXT_PADDING);

  // set volume
  tft.setTextColor((state == VOLUME_TEST ? VOLUME_SELECTED_TEXT_COLOR : VOLUME_TEXT_COLOR));
  tft.setTextDatum(TC_DATUM);
  tft.drawString(msg_set_vol, tft.getViewportWidth()/2, y);
}

void display_beacon_expert() {
  char msg[MSG_LEN_MAX];
  const char* pname;
  uint8_t i;
  personality_t* p = nullptr;
  affiliaton_t* a = nullptr;
  location_t* l = nullptr;

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
  SERIAL_PRINT("label_width = ");
  SERIAL_PRINTLN(label_width);
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

  // loop through all the beacon parameters, printing out the name and value of each
  for (i=0; i<NUM_BEACON_PARAMS; i++) {

    // get current parameter name
    pname = (beacon.type == DROID) ? msg_beacon_droid_param[i] : msg_beacon_location_param[i];

    // skip parameters that have no name
    if (pname == nullptr) {
      SERIAL_PRINT("Parameter ");
      SERIAL_PRINT(i);
      SERIAL_PRINTLN(" is not present. Skipping.");
      continue;
    }

    // print the parameter name
    snprintf(msg, MSG_LEN_MAX, "%s: ", pname);
    tft.setTextColor(BEACON_EXPERT_LABEL_COLOR);
    tft.print(msg);

    // print the parameter value
    // how the value is displayed will depend on what type of beacon it is
    if (beacon.type == DROID) {
      switch(i) {

        case BEACON_PARAM_DROID_ID:   // PARAMETER: personality
          p = get_droid_personality(beacon.setting[i]);
          if (p != nullptr) {
            snprintf(msg, MSG_LEN_MAX, "%s", p->name);
          } else {
            snprintf(msg, MSG_LEN_MAX, msg_unknown_int, beacon.setting[i]);
          }
          break;

        case BEACON_PARAM_DROID_AFFL: // PARAMETER: affiliation
          a = get_droid_affiliation(beacon.setting[i]);
          if (a != nullptr) {
            snprintf(msg, MSG_LEN_MAX, "%s", a->name);
          } else {
            snprintf(msg, MSG_LEN_MAX, msg_unknown_int, beacon.setting[i]);
          }
          break;

        case BEACON_PARAM_DROID_PAIRED: // PARAMETER: paired
        case BEACON_PARAM_DROID_BDROID: // PARAMETER: be a droid
          if (beacon.setting[i] == 0) {
            snprintf(msg, MSG_LEN_MAX, "%s", msg_no);
          } else {
            snprintf(msg, MSG_LEN_MAX, "%s", msg_yes);
          }
          break;

        default:
          snprintf(msg, MSG_LEN_MAX, "%d", beacon.setting[i]);
          break;
      }

    // location beacon
    } else {
      switch(i) {

        case BEACON_PARAM_LCTN_ID: // PARAMETER: location
          l = get_location(beacon.setting[i]);
          if (l != nullptr) {
            snprintf(msg, MSG_LEN_MAX, "%s", l->name);
          } else {
            snprintf(msg, MSG_LEN_MAX, msg_unknown_int, beacon.setting[i]);
          }
          break;

        case BEACON_PARAM_LCTN_REACT: // PARAMETER: reaction interval in minutes
          snprintf(msg, MSG_LEN_MAX, msg_int_minutes, beacon.setting[i]);
          break;

        case BEACON_PARAM_LCTN_RSSI: // PARAMETER: minimum rssi
          snprintf(msg, MSG_LEN_MAX, "-%ddBm", beacon.setting[i]);
          break;

        default:
          snprintf(msg, MSG_LEN_MAX, "%d", beacon.setting[i]);
          break;
      }
    }

    // select color of text depending on whether or not the current parameter is selected
    if (selected_item == i+1) {
      tft.setTextColor(BEACON_EXPERT_VALUE_SELECTED_COLOR);
    } else {
      tft.setTextColor(BEACON_EXPERT_VALUE_COLOR);
    }

    // display parameter value
    tft.println(msg);

    // relocate cursor to next line
    tft.setCursor(tft.getCursorX(), tft.getCursorY() + 2);
  }

  // display beacon state label
  snprintf(msg, MSG_LEN_MAX, "%s: ", msg_state);
  tft.setTextColor(BEACON_EXPERT_LABEL_COLOR);
  tft.print(msg);

  // set text color based on whether or not beacon state is currently selected
  if (selected_item == NUM_BEACON_PARAMS+1) {
    tft.setTextColor(BEACON_EXPERT_VALUE_SELECTED_COLOR);
  } else {
    tft.setTextColor(BEACON_EXPERT_VALUE_COLOR);
  }

  // display the beacon state value
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

    case SOUND_GROUP:            // display_sounds()
    case SOUND_TRACK:
    case SOUND_PLAY:
    case SOUND_PLAYING:
      display_sounds();
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
      display_droid_beacon_list();
      break;

    case BEACON_LOCATION_LIST:   // display_location_beacon_list()
      display_location_beacon_list();
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

  uint8_t i;

  SERIAL_PRINTLN("Button 1 Press");

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
        //
        // in its respective beacon list. this is being done so when you go back (long button 2 press)
        // you'll be brought to the element in the beacon list where the randomly selected beacon is
        if (beacon.type == DROID) {
          for(i=0; i<emulatable_personalities_size; i++) {
            if (emulatable_personalities[i]->id == beacon.setting[BEACON_PARAM_DROID_ID]) {
              selected_item = i;
              break;
            }
          }
        } else {
          for(i=0; i<LOCATIONS_SIZE; i++) {
            if (locations[i].id == beacon.setting[BEACON_PARAM_LCTN_ID]) {
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
      set_location_beacon(locations[selected_item].id);
      state = BEACON_ACTIVATE;
      tft_update = true;
      break;

    case BEACON_DROID_LIST:
      set_droid_beacon(emulatable_personalities[selected_item]->id);
      state = BEACON_ACTIVATE;
      tft_update = true;
      break;

    case BEACON_ACTIVATE:
      init_advertisement_data(nullptr);
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

      // change beacon type; (re)initialize the beacon object
      if (selected_item == 0) {

        if (beacon.type == DROID) {
          beacon.type = LOCATION;
          beacon.setting[BEACON_PARAM_DROID_ID]     = 1;  // default location
          beacon.setting[BEACON_PARAM_DROID_AFFL]   = 2;  // default interval
          beacon.setting[BEACON_PARAM_DROID_PAIRED] = 38; // default minimim rssi
          beacon.setting[BEACON_PARAM_DROID_BDROID] = 0;  // be a droid
        } else {
          beacon.type = DROID;
          beacon.setting[BEACON_PARAM_LCTN_ID]      = 1; // default personality
          beacon.setting[BEACON_PARAM_LCTN_REACT]   = 1; // default affiliation
          beacon.setting[BEACON_PARAM_LCTN_RSSI]    = 1; // default paired
          beacon.setting[BEACON_PARAM_LCTN_PARAM4]  = 0; // unused
        }

      // change beacon parameters
      } else if (selected_item-1 < NUM_BEACON_PARAMS) {

        // store the current beacon parameter index based on current selected_item value
        i = selected_item - 1;

        // on a long press, reset the beacon setting to 0
        if (press_type == LONG_PRESS) {
          beacon.setting[i] = 0;

        // otherwise, increment the parameter value by 1
        } else {
          beacon.setting[i]++;
        }

        // adjust parameter values as needed for a DROID beacon
        if (beacon.type == DROID) {
          switch(i) {

            // reset affiliation if affiliation goes above 0x3F
            case BEACON_PARAM_DROID_AFFL:
              if (beacon.setting[i] > 0x3F) {
                beacon.setting[i] = 0;
              }
              break;

            // these are yes/no parameters, keep their values as either 0 or 1
            case BEACON_PARAM_DROID_BDROID:
            case BEACON_PARAM_DROID_PAIRED:
              if (beacon.setting[i] > 1) {
                beacon.setting[i] = 0;
              }
              break;
          }

        // adjust parameter values as needed for a LOCATION beacon
        } else {
          switch(i) {

            // reset location if it goes above 7
            case BEACON_PARAM_LCTN_ID:
              if (beacon.setting[i] > 7) {
                beacon.setting[i] = 0;
              }

            // reset minimium rssi value if it goes outside of limits
            case BEACON_PARAM_LCTN_RSSI:

              // -20dBm is about 1ft from the droid and I think a reasonable lower limit
              // -80dBm is very weak and I think a reasonable upper limit
              // but you can change these limits if you want
              if (beacon.setting[i] < 20 || beacon.setting[i] > 80) {
                beacon.setting[i] = 20;
              }
              break;

          }
        }

      // if we get to here then we're on the activate function
      } else {

        // if a droid beacon with the be a droid parameter enabled
        if (beacon.type == DROID && beacon.setting[BEACON_PARAM_DROID_BDROID] != 0) {

          // create a beacon with the name DROID so as to appear as a droid to other toolboxes
          init_advertisement_data(ble_adv_name_droid);

        // otherwise initialize beacon as normal
        } else {
          init_advertisement_data(nullptr);
        }

        // set the payload based on the beacon data
        set_payload_from_beacon();

        // start advertising the beacon
        pAdvertising->start();

        // change state to active beacon
        state = BEACON_EXPERT_ACTIVE;
      }

      // set flag to update the display
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
      SERIAL_PRINTLN("Play selected!");
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

  uint8_t i;

  SERIAL_PRINTLN("Button 2 Press");

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
        if (selected_item >= LOCATIONS_SIZE) {
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
        if (selected_item >= emulatable_personalities_size) {
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
        init_advertisement_data(nullptr);
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

        // skip expert beacon parameters that have no label
        i = selected_item - 1;
        if (i < NUM_BEACON_PARAMS && ((beacon.type == DROID) ? msg_beacon_droid_param[i] : msg_beacon_location_param[i]) == nullptr) {
          selected_item++;
        }

        // return to the first beacon parameter after reaching the end
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

  // populate emulatable_personalities array
  emulatable_personalities_size = 0;

  // how many personalities are available for emulation?
  for (i=0; i<DROID_PERSONALITIES_SIZE; i++) {
    if (droid_personalities[i].emulatable != 0) {
      emulatable_personalities_size++;
    }
  }

  // allocate memory for the pointers
  emulatable_personalities = (personality_t**)malloc(sizeof(personality_t*) * emulatable_personalities_size);

  // assign pointers to members of droid_personalities[]
  emulatable_personalities_size = 0;
  for (i=0; i<DROID_PERSONALITIES_SIZE; i++) {
    if (droid_personalities[i].emulatable != 0) {
      emulatable_personalities[emulatable_personalities_size] = &droid_personalities[i];
      emulatable_personalities_size++;
    }
  }

  // just so there's no garbage in there if it gets used before being initialized.
  set_random_beacon();

  // init serial debug messaging
  SERIAL_BEGIN(115200);
  SERIAL_PRINTLN("Ready!");
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
      SERIAL_PRINTLN("Playing sound...");
      droid_play_track();
      delay(2000);
      state = SOUND_PLAY;
      tft_update = true;
      break;

    case VOLUME_TESTING:
      update_display();
      SERIAL_PRINTLN("Testing volume...");
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
      SERIAL_PRINTLN("Going to sleep.");
      delay(100);
      esp_deep_sleep_start();
    } else {
      last_activity = millis();
    }
  }
}
