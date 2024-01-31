/* Droid Toolbox v0.70 : ruthsarian@gmail.com
 * 
 * A program to work with droids from the Droid Depot at Galaxy's Edge.
 * 
 * Features
 *   - Scan for nearby droids
 *   - Generate location and droid beacons
 *   - Control audio produced by droids
 *   - Selectable display fonts
 * 
 * Designed to be used with a LilyGO TTGO T-Display or LilyGO T-Display-S3 which are ESP32-based modules with an LCD display, although
 * it should work with any ESP32 module and some small code changes.
 *   TTGO T-Display: https://www.lilygo.cc/products/lilygo%C2%AE-ttgo-t-display-1-14-inch-lcd-esp32-control-board
 *   T-Display-S3: https://www.lilygo.cc/products/t-display-s3
 * 
 * Required Arduino Board
 *  Arduino ESP32 core: https://github.com/espressif/arduino-esp32
 *    1. add https://raw.githubusercontent.com/espressif/arduino-esp32/gh-pages/package_esp32_index.json to the additional board manager URLs in the preferences window
 *    2. go to boards manager
 *    3. search for and install esp32
 *    4. when compiling, select either the ESP32 Dev Module (for T-Display) or ESP32S3 Dev Module (for T-Display-32)
 * 
 * Required Arduino Libraries
 *  TFT_eSPI: https://github.com/Bodmer/TFT_eSPI
 *  OpenFontRender: https://github.com/takkaO/OpenFontRender
 *  
 * NOTE 1: 
 *  After installing _OR_UPDATING_ the TFT_eSPI library you _MUST_ edit User_Setup_Select.h as follows 
 *     1. comment out the line "#include <User_Setup.h>" (line 22-ish)
 *     2. uncomment the line "#include <User_Setups/Setup25_TTGO_T_Display.h>" (line 61-ish) for T-Display
 *        or "#include <User_Setups/Setup206_LilyGo_T_Display_S3.h>" for the T-Display-32
 *
 *   Possible path for Windows users: %USERPROFILE%\Documents\Arduino\libraries\TFT_eSPI\User_Setup_Select.h
 *
 * NOTE 2:
 *   The T-Display-S3 may display the wrong colors. If this happens you'll also need to modify 
 *   User_Setups/Setup206_LilyGo_T_Display_S3.h in the TFT_eSPI library, locate the two lines, one commented out,
 *   that begin with "#define TFT_RGB_ORDER". Uncomment the commented-out line, and comment out the uncommented line.
 *   Then reupload the sketch and the colors should be correct.
 *
 *   #define TFT_RGB_ORDER TFT_RGB
 *
 * NOTE 3:
 *   A BLE library is included in the Arduino ESP32 core. If you have ArduinoBLE already installed you will need
 *   to uninstall it in order for this code to compile correctly. To uninstall a library locate your arduino 
 *   libraries folder and delete the ArduinoBLE folder.
 *
 * NOTE 4:
 *   It seems when the serial port is open, button 0 doesn't work. If your T-Display is connected via USB to 
 *   your computer and button 0 does not work, change the port in Arduino IDE to something else. Don't forget
 *   to change it back when you want to upload new code to the T-Display.
 *
 * TTGO T-Display Board Configuration (defaults)
 *   Board: ESP32 Dev Module
 *   Upload Speed: 921600
 *   CPU Freq: 240MHz (WiFI/BT)
 *   Flash Freq: 80MHz
 *   Flash Mode: QIO
 *   Flash Size: 4MB (32Mb)
 *   Partition Scheme: Huge App (3MB No OTA/1MB SPIFFS)
 *   Core Debug Level: None
 *   PSRAM: Disabled
 * 
 * T-Display-S3 Board Configuration (defaults)
 *   Board: ESP32S3 Dev Module
 *   Upload Speed: 921600
 *   CPU Freq: 240MHz (WiFI/BT)
 *   Flash Mode: QIO 80MHz
 *   Flash Size: 16MB (128Mb)
 *   Partition Scheme: Huge App (3MB No OTA/1MB SPIFFS)
 *   Core Debug Level: None
 *   PSRAM: OPI PSRAM 
 *   USB CDC On Boot: Enabled
 *   JTAG Adapter: Integrated USB JTAG
 *   USB Mode: Hardware CDC and JTAG
 *   
 * References
 *   Arduino IDE setup: https://www.youtube.com/watch?v=b8254--ibmM
 *   TTGO T-Display Github Repository: https://github.com/Xinyuan-LilyGO/TTGO-T-Display
 *   T-Display-S3 Github Repository: https://github.com/Xinyuan-LilyGO/T-Display-S3
 *   
 *   Misc:
 *     https://github.com/Bodmer/TFT_eSPI/blob/master/TFT_eSPI.h
 *     https://github.com/takkaO/OpenFontRender/blob/master/src/OpenFontRender.h
 *     https://github.com/nkolban/esp32-snippets/blob/master/cpp_utils/BLEScan.h
 *     https://programmer.ink/think/arduino-development-tft_espi-library-learning.html
 *     https://programmer.ink/think/color-setting-and-text-display-esp32-learning-tour-arduino-version.html.
 *     https://github.com/nkolban/esp32-snippets/blob/fe3d318acddf87c6918944f24e8b899d63c816dd/cpp_utils/BLEAdvertisedDevice.h
 *     https://randomnerdtutorials.com/esp32-save-data-permanently-preferences/
 *     https://espressif-docs.readthedocs-hosted.com/projects/arduino-esp32/en/latest/api/preferences.html
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
 *        control LEDs
 *        control motors (is this a GOOD idea? probably not...)
 *   other: 
 *     is there any value in scanning for SWGE East/West beacon (used by the Disney Play app) and identifying which location you're in based off that?
 *     ability save beacons that are defined in EXPERT mode?
 *     revisit auto shutoff. can it not require a reset to wake up?
 *     add option, through defines, to rotate display 180 degrees so buttons are on the right
 *
 * HISTORY
 *   v0.70 : Fixed beacon menu font size issues with TTGO T-Display
 *           thanks to Knucklebuster620 for bringing this issue to my attention
 *   v0.69 : The Wayfinder Version 
 *           Toolbox remembers font selection through reboot/power cycle
 *   v0.68 : limited rotating beacons to just location beacon types. reason is that droids will not respond to a droid beacon if it's seen a location beacon within the last 2 hours. 
 *           added a few defines to let you control the interval settings for short and long presses
 *           changed initial interval to 60 seconds
 *   v0.67 : added rotating beacon option; includes abilty to set beacon interval between 60 and 1440 seconds.
 *   v0.66 : added TFGunray font; originally added for demonstration 
 *   v0.65 : added support for custom fonts via OpenFontRenderer (https://github.com/takkaO/OpenFontRender)
 *             - added a few fonts from aurekfonts.github.io that were labeled as free for personal and commercial use
 *             - added several functions (dtb_*) to assist in supporting custom fonts
 *             - complicated display lists further by adding a step to precalculate some font dimensions on font change to help speed up font rendering
 *             - consolidated some defines and functions related to lists
 *             - fell into a stupor trying to get fonts behaving nicely; came out of it with little understanding how this code works
 *           changed the names of a couple location beacons after reviewing existing beacon location data (see: https://www.google.com/maps/d/edit?mid=1pdCcMcTHQzcOOTIz-Lv1uYqqjWI-jDQ)
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
 *             Connection is currently a demo; connect, tell droid to play a sound, then disconnect. 
 *             Will improve upon this in the next version.
 *   v0.30 : Long/Short button press detection
 *           Droid report is paged; shows 1 droid at a time
 *           Droid report sorts droids by RSSI value
 *           Added version to splash screen
 *   v0.20 : Added back button from both beacon and scanner.
 *           Location beacon location is randomly selected.
 *   v0.10 : Initial Release
 */

#define USE_OFR_FONTS           // uncomment to use openFontRenderer (see notes above on how to install this library)
#define USE_NVS                 // uncomment to enable use of non-volatile storage (NVS) to save preferences (last font used);
//#define SERIAL_DEBUG_ENABLE     // uncomment to enable serial debug/monitor messages
                                // if using seridal debug, you may need to select HUGE APP from the partition scheme option under tools menu

#include <TFT_eSPI.h>
#include <SPI.h>
#include <BLEDevice.h>
#include <BLEUtils.h>
#include <BLEScan.h>
#include <BLEAdvertisedDevice.h>

#ifdef USE_OFR_FONTS            // load OpenFontRenderer and some fonts from https://aurekfonts.github.io/
  #include "OpenFontRender.h"
  #include "Aurebesh-English.h"
  #include "Aurebeshred-Bold.h"
  #include "Aurabesh.h"
  #include "DroidobeshDepot-RegularModified.h"
  #include "TFGunray-Bold.h"
#endif

#if defined (USE_NVS) && defined (USE_OFR_FONTS)  // no point in enabling saving of font preferences if OFR_FONTS aren't being used
  #include <Preferences.h>
  #define PREF_APP_NAME     "droid-toolbox"       // the name should not be changed
  Preferences preferences;                        // create a preferences object to store variables in non-volatile storage (NVS)
#endif

#define C565(r,g,b)                         ((r & 0xF8) << 8) | ((g & 0xFC) << 3) | (b >> 3)    // macro to convert RGB values to TFT_eSPI color value

// CUSTOMIZATIONS BEGIN -- These values can be changed to alter Droid Toolbox's behavior.

#define MSG_VERSION                         "v0.70"                 // the version displayed on the splash screen at the lower right; β

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

#define MENU_SELECT_CAPTION_TEXT_SIZE       DEFAULT_TEXT_SIZE
#define MENU_SELECT_CAPTION_TEXT_PADDING    DEFAULT_TEXT_PADDING
#define MENU_SELECT_CAPTION_TEXT_COLOR      TFT_WHITE
#define MENU_SELECT_TEXT_SIZE               (DEFAULT_TEXT_SIZE*2)
#define MENU_SELECT_TEXT_PADDING            DEFAULT_TEXT_PADDING
#define MENU_SELECT_TEXT_COLOR              C565(0,64,0)
#define MENU_SELECT_SELECTED_TEXT_COLOR     TFT_GREEN
#define MENU_SELECT_SELECTED_BORDER_COLOR   TFT_BLUE

#define BEACON_MENU_SELECT_TEXT_PADDING     8                       // beacon menus use MENU_SELECT_* values except for these two
#define BEACON_MENU_SELECT_TEXT_SIZE        (DEFAULT_TEXT_SIZE + 1)

#define BEACON_INTERVAL_TITLE_TEXT_SIZE     DEFAULT_TEXT_SIZE
#define BEACON_INTERVAL_TITLE_COLOR         TFT_BLUE
#define BEACON_INTERVAL_VALUE_TEXT_SIZE     (DEFAULT_TEXT_SIZE + 1)
#define BEACON_INTERVAL_VALUE_COLOR         TFT_RED
#define BEACON_INTERVAL_VALUE_COLOR_S       TFT_GREEN
#define BEACON_INTERVAL_SET_TEXT_SIZE       (DEFAULT_TEXT_SIZE + 1)
#define BEACON_INTERVAL_SET_COLOR           TFT_BROWN
#define BEACON_INTERVAL_SET_COLOR_S         TFT_YELLOW
#define BEACON_INTERVAL_TEXT_PADDING        DEFAULT_TEXT_PADDING

#define BEACON_CONTROL_TEXT_SIZE            (DEFAULT_TEXT_SIZE + 1)
#define BEACON_CONTROL_TEXT_PADDING         DEFAULT_TEXT_PADDING
#define BEACON_CONTROL_TYPE_TEXT_SIZE       DEFAULT_TEXT_SIZE
#define BEACON_CONTROL_TYPE_COLOR           TFT_BLUE
#define BEACON_CONTROL_ID_COLOR             TFT_RED
#define BEACON_CONTROL_ACTIVE_COLOR         TFT_YELLOW
#define BEACON_CONTROL_INACTIVE_COLOR       TFT_BROWN
#define BEACON_CONTROL_COUNTDOWN_COLOR      C565(64,64,64)
#define BEACON_CONTROL_COUNTDOWN_SIZE       DEFAULT_TEXT_SIZE

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

#define DEFAULT_TEXT_FIT_WIDTH              (tft.getViewportWidth() * 0.8)    // used to control the available width value used to calculate the font size needed for a string of text to fit within that space

#define SLEEP_AFTER                         5 * 60 * 1000   // how many milliseconds of inactivity before going to sleep/hibernation
#define DEFAULT_BEACON_REACTION_TIME        2               // how many minutes to wait between reactions to the beacon being broadcast; ((esp_random() % 3) + 1)
#define MAX_BEACON_CHANGE_INTERVAL          120             // this is multiplied by 10. should be no larger than 250.
#define SHORT_PRESS_INTERVAL_INC            1               // this is multiplied by 10. 
#define LONG_PRESS_INTERVAL_INC             10              // this is multiplied by 10

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
const char msg_rotating[]               = "ROTATING";
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
const char msg_set_beacon_interval[]    = "SET BEACON INTERVAL";
const char msg_int_sec[]                = "%d sec";
const char msg_set_interval[]           = "START BEACON";

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
//       therefore I do not include it in this list, but location 0 is available in the 
//       expert beacon mode.
//
location_t locations[] = {
// ID,    NAME
  { 0x01, "Marketplace" },
  { 0x02, "Droid Depot" },
  { 0x03, "Resistance"  },
  { 0x04, "Unknown"     },
  { 0x05, "Alert"       },
  { 0x06, "Dok Ondar's" },
  { 0x07, "First Order" },
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

//
// Custom Fonts
// The OpenFontRenderer is used to display TTF fonts.
// https://github.com/takkaO/OpenFontRender
// 
// How To Make A Font Header File
// 1) convert to TTF (if needed)
//    https://cloudconvert.com/otf-to-ttf
// 2) reduce to only those characters we care about (if needed)
//    https://products.aspose.app/font/generator/ttf-to-ttf
//      !"#$%&'()*+,-./0123456789:;<=>?@ABCDEFGHIJKLMNOPQRSTUVWXYZ[\]^_`abcdefghijklmnopqrstuvwxyz{|}~
// 3) convert to header (.h) file
//    https://notisrac.github.io/FileToCArray/
//      uncheck static
//      save .h file to code tree
//      make sure font variable name in header file has no dashes in it
//      make sure font variable in header file is type const unsigned char
//      look at existing font header files for an example
// 4) add #include "header.h" line near the top where other fonts are included
// 5) add font to dtb_fonts array below
//
#ifdef USE_OFR_FONTS
  typedef struct {
    const unsigned char* data;
    uint32_t size;
    float y_scale;    // a workaround for font rendering issues; the font will be scaled by this value when rendered
    float y_offset;   // another workaround to help fix baseline alignment issues
  } dtb_font_t;

  dtb_font_t dtb_fonts[] = {
    { AurebeshEnglish,                sizeof(AurebeshEnglish),                 0.9, -0.08 },
    { DroidobeshDepotRegularModified, sizeof(DroidobeshDepotRegularModified),  0.9, -0.06 },
    { Aurabesh,                       sizeof(Aurabesh),                        1.0, -0.06 },
    { AurebeshRedBold,                sizeof(AurebeshRedBold),                 1.0, -0.14 },
    { TFGunrayBold,                   sizeof(TFGunrayBold),                    1.0,  0    },
  };

  #define NUM_FONTS (sizeof(dtb_fonts)/sizeof(dtb_font_t))
  OpenFontRender ofr;
  uint8_t dtb_font = 0;
#endif // USE_OFR_FONTS

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
  //#undef USE_OFR_FONTS        // force T-Display to not use OFR? i don't think this is necessary.
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
  BEACON_ROTATING,        // the beacon rotates (changes) at some regular interval
  BEACON_SET_INTERVAL,    // set the interval between beacon rotations
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

//
// menus consist of a string and a state
// the strings of the menu are displayed as a list
// the state of the code changes to the state of string that is selected
//
typedef struct {
  system_state_t state;
  const char* text;
} menu_item_t;

const menu_item_t top_menu[] = {
  { SCANNER_SCANNING,     msg_scanner   },
  { BEACON_TYPE_MENU,     msg_beacon    }
};

const menu_item_t beacon_type_menu[] = {
  { BEACON_LOCATION_LIST, msg_location  },
  { BEACON_DROID_LIST,    msg_droid     },
  { BEACON_RANDOM,        msg_random    },
  { BEACON_ROTATING,      msg_rotating  },
  { BEACON_EXPERT,        msg_expert    },
};

const menu_item_t connected_menu[] = {
  { SOUND_GROUP,          msg_sounds    },
  { VOLUME_UP,            msg_volume    },
};

//
// Lists
//
// a list is an array of strings which is used to render a list of options on the display
// they are derrived from menus, beacons, locations, etc.
//
// with the introduction of custom fonts, i need to precalculate some properties of each
// list that is going to be rendered in order to speed-up rendering times.
//
// thus i've created the list object which will contain a pointer to the menu or string array 
// that contains the contents of the list along with properties that will be used when 
// rendering the list on screen.
// 

// this struct contains information for rendering lists
// ofr_font_size and row_width will be calculated, for every list, every time the system font changes. 
typedef struct {
  uint8_t  text_size;
  uint16_t text_color;
  uint8_t  text_padding;
  uint16_t selected_text_color;
  uint16_t selected_border_color;
  uint16_t ofr_font_size;
  uint16_t ofr_font_height;
  uint16_t row_width;
} list_render_options_t;

// TODO: remove this global once lists are fully incorporated into the code
list_render_options_t list_options;

//
// this is the list object
//
// it will have an array of pointers to strings that make up the content of the list
// this array must be populated at runtime because we need to extract the strings from different
// object types (menus, personalities, locations, etc.)
//
// the render_options object sets how the list will be displayed (color, default font size, etc.)
// these will be set set based on #define lines under the customizations section at the top of this code.
//
typedef struct {
  const char**          items;
  uint8_t               num_items;
  list_render_options_t render_options;
} list_t;

// these #define statements are index values of the global lists[] array. they will be used to indentify an individual list within the lists[] array.
#define LIST_TOP_MENU           0
#define LIST_BEACON_TYPE_MENU   1
#define LIST_CONNECTED_MENU     2
#define LIST_LOCATIONS          3
#define LIST_PERSONALITIES      4
#define NUM_LISTS               5

// lists will be loaded via the load_lists() function. 
list_t lists[NUM_LISTS];

  int8_t droid_volume = 100;          // there is no way to 'read' the current volume setting, so we'll keep track with a variable and assume it starts at full (100) volume
 uint8_t selected_item = 0;           // keep track of the currently selected option when displaying menus, options, etc.
 uint8_t beacon_rotate_interval = 6;  // this value, multiplied by 10, defines the number of seconds before the current beacon changes; when set to 0 the beacon rotation feature is disabled
uint32_t next_beacon_time = 0;        // the time, in ms, when the next beacon change will occur

TFT_eSPI tft = TFT_eSPI();      // display interface
bool tft_update = true;         // flag to indicate display needs to be updated

system_state_t state = SPLASH;  // track the current state of the toolbox

//
// rendering lists is done with an array of strings (the items in the list)
// these lists are coming from different variable types (menus, beacons, personalities, etc.)
//
// i need this function in order to extract pointers to the list item strings in each of those 
// objects and put them into a shared type (const char**) so all the list rendering code doesn't
// require a ton of if/else statements depending on the type of object being rendered.
//
// it's important the order of the items in the parent is shared in the list object so that 
// they can share an index variable (e.g. selected_item global)
//
// i suppose if you want to further customize how lists displays (e.g. beacons and personalities
// have different text colors) then you could alter this function to change those properties.
//
// there's probably a better way to do this, but i want to get this working
// i'm open to suggestions. ruthsarian@gmail.com
//
void list_init() {
  uint8_t i;

  // populate TOP MENU list

  // calculate num_items and allocate memory for items
  lists[LIST_TOP_MENU].num_items = sizeof(top_menu) / sizeof(menu_item_t);
  lists[LIST_TOP_MENU].items = (const char**)malloc(sizeof(char*) * lists[LIST_TOP_MENU].num_items);

  // populate items
  for (i=0; i<lists[LIST_TOP_MENU].num_items; i++) {
    lists[LIST_TOP_MENU].items[i] = top_menu[i].text;
  }

  // set render options
  lists[LIST_TOP_MENU].render_options.text_size             = MENU_SELECT_TEXT_SIZE;
  lists[LIST_TOP_MENU].render_options.text_color            = MENU_SELECT_TEXT_COLOR;
  lists[LIST_TOP_MENU].render_options.text_padding          = MENU_SELECT_TEXT_PADDING;
  lists[LIST_TOP_MENU].render_options.selected_text_color   = MENU_SELECT_SELECTED_TEXT_COLOR;
  lists[LIST_TOP_MENU].render_options.selected_border_color = MENU_SELECT_SELECTED_BORDER_COLOR;
  lists[LIST_TOP_MENU].render_options.ofr_font_size         = 0;
  lists[LIST_TOP_MENU].render_options.ofr_font_height       = 0;
  lists[LIST_TOP_MENU].render_options.row_width             = 0;

  //
  // rinse and repeat
  //

  // populate BEACON TYPE MENU list
  lists[LIST_BEACON_TYPE_MENU].num_items = sizeof(beacon_type_menu) / sizeof(menu_item_t);
  lists[LIST_BEACON_TYPE_MENU].items = (const char**)malloc(sizeof(char*) * lists[LIST_BEACON_TYPE_MENU].num_items);
  for (i=0; i<lists[LIST_BEACON_TYPE_MENU].num_items; i++) {
    lists[LIST_BEACON_TYPE_MENU].items[i] = beacon_type_menu[i].text;
  }
  lists[LIST_BEACON_TYPE_MENU].render_options.text_size             = MENU_SELECT_TEXT_SIZE;
  lists[LIST_BEACON_TYPE_MENU].render_options.text_color            = MENU_SELECT_TEXT_COLOR;
  lists[LIST_BEACON_TYPE_MENU].render_options.text_padding          = MENU_SELECT_TEXT_PADDING;
  lists[LIST_BEACON_TYPE_MENU].render_options.selected_text_color   = MENU_SELECT_SELECTED_TEXT_COLOR;
  lists[LIST_BEACON_TYPE_MENU].render_options.selected_border_color = MENU_SELECT_SELECTED_BORDER_COLOR;
  lists[LIST_BEACON_TYPE_MENU].render_options.ofr_font_size         = 0;
  lists[LIST_BEACON_TYPE_MENU].render_options.ofr_font_height       = 0;
  lists[LIST_BEACON_TYPE_MENU].render_options.row_width             = 0;

  // populate CONNECTED MENU list
  lists[LIST_CONNECTED_MENU].num_items = sizeof(connected_menu) / sizeof(menu_item_t);
  lists[LIST_CONNECTED_MENU].items = (const char**)malloc(sizeof(char*) * lists[LIST_CONNECTED_MENU].num_items);
  for (i=0; i<lists[LIST_CONNECTED_MENU].num_items; i++) {
    lists[LIST_CONNECTED_MENU].items[i] = connected_menu[i].text;
  }
  lists[LIST_CONNECTED_MENU].render_options.text_size             = MENU_SELECT_TEXT_SIZE;
  lists[LIST_CONNECTED_MENU].render_options.text_color            = MENU_SELECT_TEXT_COLOR;
  lists[LIST_CONNECTED_MENU].render_options.text_padding          = MENU_SELECT_TEXT_PADDING;
  lists[LIST_CONNECTED_MENU].render_options.selected_text_color   = MENU_SELECT_SELECTED_TEXT_COLOR;
  lists[LIST_CONNECTED_MENU].render_options.selected_border_color = MENU_SELECT_SELECTED_BORDER_COLOR;
  lists[LIST_CONNECTED_MENU].render_options.ofr_font_size         = 0;
  lists[LIST_CONNECTED_MENU].render_options.ofr_font_height       = 0;
  lists[LIST_CONNECTED_MENU].render_options.row_width             = 0;

  // populate LOCATIONS list
  lists[LIST_LOCATIONS].num_items = LOCATIONS_SIZE;
  lists[LIST_LOCATIONS].items = (const char**)malloc(sizeof(char*) * lists[LIST_LOCATIONS].num_items);
  for (i=0; i<lists[LIST_LOCATIONS].num_items; i++) {
    lists[LIST_LOCATIONS].items[i] = locations[i].name;
  }
  lists[LIST_LOCATIONS].render_options.text_size              = BEACON_MENU_SELECT_TEXT_SIZE;
  lists[LIST_LOCATIONS].render_options.text_color             = MENU_SELECT_TEXT_COLOR;
  lists[LIST_LOCATIONS].render_options.text_padding           = BEACON_MENU_SELECT_TEXT_PADDING;
  lists[LIST_LOCATIONS].render_options.selected_text_color    = MENU_SELECT_SELECTED_TEXT_COLOR;
  lists[LIST_LOCATIONS].render_options.selected_border_color  = MENU_SELECT_SELECTED_BORDER_COLOR;
  lists[LIST_LOCATIONS].render_options.ofr_font_size          = 0;
  lists[LIST_LOCATIONS].render_options.ofr_font_height        = 0;
  lists[LIST_LOCATIONS].render_options.row_width              = 0;

  // populate DROID PERSONALITIES list
  lists[LIST_PERSONALITIES].num_items = DROID_PERSONALITIES_SIZE;
  lists[LIST_PERSONALITIES].items = (const char**)malloc(sizeof(char*) * lists[LIST_PERSONALITIES].num_items);
  for (i=0; i<lists[LIST_PERSONALITIES].num_items; i++) {
    lists[LIST_PERSONALITIES].items[i] = droid_personalities[i].name;
  }
  lists[LIST_PERSONALITIES].render_options.text_size              = BEACON_MENU_SELECT_TEXT_SIZE;
  lists[LIST_PERSONALITIES].render_options.text_color             = MENU_SELECT_TEXT_COLOR;
  lists[LIST_PERSONALITIES].render_options.text_padding           = BEACON_MENU_SELECT_TEXT_PADDING;
  lists[LIST_PERSONALITIES].render_options.selected_text_color    = MENU_SELECT_SELECTED_TEXT_COLOR;
  lists[LIST_PERSONALITIES].render_options.selected_border_color  = MENU_SELECT_SELECTED_BORDER_COLOR;
  lists[LIST_PERSONALITIES].render_options.ofr_font_size          = 0;
  lists[LIST_PERSONALITIES].render_options.ofr_font_height        = 0;
  lists[LIST_PERSONALITIES].render_options.row_width              = 0;
}

// calculate the current font height
uint16_t dtb_get_font_height() {
  static const char test_str[] = "Hy";
  #ifdef USE_OFR_FONTS
    if (dtb_font != 0) {
      return((uint16_t)(ofr.getTextHeight(test_str) & 0x0000FFFF));
    } else {
  #endif
      return((uint16_t)(tft.fontHeight() & 0x7FFF));
  #ifdef USE_OFR_FONTS
    }
  #endif
}

// calculate the current font height
uint16_t dtb_get_text_width(const char* msg) {
  static const char test_str[] = "Hy";
  #ifdef USE_OFR_FONTS
    if (dtb_font != 0) {
      return((uint16_t)(ofr.getTextWidth(msg) & 0x0000FFFF));
    } else {
  #endif
      return((uint16_t)(tft.textWidth(msg) & 0x0000FFFF));
  #ifdef USE_OFR_FONTS
    }
  #endif
}

// this function is precalculating ofr_font_size and row_width for all lists
// it should be called only one time, after the system font is set/changed.
// this front-loads all the processor-intensive calculations required to determine the
// correct font size and render area for each list.
//
// if this was done on-the-fly everytime a list was rendered then we would see obvious
// screen lag/delay every time the list was drawn on screen. so this is being done to 
// improve UI performance
//
// this function needs to know how wide the area where the font will be rendered is.
// this is typically tft.width(), but I've made it a function argument so we can do 
// things like add gutterspace on either side of a list that this function
// doesn't need to know about.
//
// the ofr font should already be loaded at this point
void list_calculate_dynamic_font_properties() {
  uint8_t curr_list, curr_item, num_items;
  uint16_t font_height = 0, ofs_tmp;

  #ifdef USE_OFR_FONTS
    SERIAL_PRINT("dtb_font = ");
    SERIAL_PRINTLN(dtb_font);
  #endif

  // loop through all lists
  for (curr_list=0; curr_list<NUM_LISTS; curr_list++) {

    // initialize the render options we'll be calculating in a bit
    lists[curr_list].render_options.ofr_font_size = 0;
    lists[curr_list].render_options.row_width = 0;

    // set ofr_font_size to 0 as it will not be used in this instance
    // set the GLCD font size and calculate font_height from that
    //tft.setTextFont(1);
    tft.setTextSize(lists[curr_list].render_options.text_size);
    font_height = tft.fontHeight();

    #ifdef USE_OFR_FONTS
      if (dtb_font == 0) {
    #endif

        // no need to calculate ofr_font_size since it won't be used when dtb_font is 0
        //
        // we are ignoring max_width completely. the assumption is you've already done your homework.
        // but perhaps we could dynamically determine font size when dtb_font is 0... later.

        // find the list item with the largest width when rendered and record to row_width
        for (curr_item=0; curr_item<lists[curr_list].num_items; curr_item++) {
          if (tft.textWidth(lists[curr_list].items[curr_item]) > lists[curr_list].render_options.row_width) {
            lists[curr_list].render_options.row_width = tft.textWidth(lists[curr_list].items[curr_item]);
          }
        }

    #ifdef USE_OFR_FONTS
      } else {

        // determine the font size to use when rendering this list
        for (curr_item=0; curr_item<lists[curr_list].num_items; curr_item++) {

          // calculate the font size that would be needed to fit within the dimensions of the viewport width and tft.fontHeight()
          //
          // would caluclateBoundingBox() be a better method?
          ofs_tmp = ofr.calculateFitFontSize(DEFAULT_TEXT_FIT_WIDTH, tft.fontHeight(), ofr.getLayout(), lists[curr_list].items[curr_item]);

          // if this value is smaller than what is currently stored, record the font size
          if (lists[curr_list].render_options.ofr_font_size < 1 || ofs_tmp < lists[curr_list].render_options.ofr_font_size) {
            lists[curr_list].render_options.ofr_font_size = ofs_tmp;
          }
        }

        // set the font size to the newly determined ofr_font_size
        ofr.setFontSize(lists[curr_list].render_options.ofr_font_size);

        // record the font height
        lists[curr_list].render_options.ofr_font_height = dtb_get_font_height();

        // now determine the list item with the largest width when rendered with the currently set font size
        for (curr_item=0; curr_item<lists[curr_list].num_items; curr_item++) {
          ofs_tmp = ofr.getTextWidth(lists[curr_list].items[curr_item]);
          if (ofs_tmp > lists[curr_list].render_options.row_width) {
            lists[curr_list].render_options.row_width = ofs_tmp;
          }
        }
      }
    #endif
  }
}

// calculate and set the font size
void dtb_set_font_size(uint8_t text_size, uint16_t width_fit, const char* str) {
	static const char test_str[] = "Hy";
  uint32_t tmp_size = 0;

  if (text_size > 0 && text_size < 8) {
    tft.setTextSize(text_size);
  }

  #ifdef USE_OFR_FONTS
    if (dtb_font != 0) {
      if (text_size > 7) {
        ofr.setFontSize(text_size);
      } else {

        // stupid bug; ofr.calculateFitFontSize() doesn't light a height that's below something like 16, so...
        if (text_size == 1) {
          tmp_size = tft.fontHeight() * 2;
        } else {
          tmp_size = tft.fontHeight();
        }
      
        if (str == nullptr) {
          tmp_size = ofr.calculateFitFontSize(width_fit, tmp_size, ofr.getLayout(), test_str);
        } else {
          tmp_size = ofr.calculateFitFontSize(width_fit, tmp_size, ofr.getLayout(), str);
        }

        // readajust tmp_size if we're doing the bug workaround
        if (text_size == 1) {
          tmp_size = tmp_size / 2;
        }
        ofr.setFontSize(tmp_size);
      }
    }
  #endif
}

// set alignment of ofr using the tft datum values
void set_ofr_alignment_by_datum(uint8_t d) {
  #ifdef USE_OFR_FONTS
  switch (d) {
    default:
    case L_BASELINE:
      ofr.setAlignment(Align::Left);
      break;
    case C_BASELINE:
      ofr.setAlignment(Align::Center);
      break;
    case R_BASELINE:
      ofr.setAlignment(Align::Right);
      break;
    case TL_DATUM:
      ofr.setAlignment(Align::TopLeft);
      break;
    case TC_DATUM:
      ofr.setAlignment(Align::TopCenter);
      break;
    case TR_DATUM:
      ofr.setAlignment(Align::TopRight);
      break;
    case ML_DATUM:
      ofr.setAlignment(Align::MiddleLeft);
      break;
    case MC_DATUM:
      ofr.setAlignment(Align::MiddleCenter);
      break;
    case MR_DATUM:
      ofr.setAlignment(Align::MiddleRight);
      break;
    case BL_DATUM:
      ofr.setAlignment(Align::BottomLeft);
      break;
    case BC_DATUM:
      ofr.setAlignment(Align::BottomCenter);
      break;
    case BR_DATUM:
      ofr.setAlignment(Align::BottomRight);
      break;
  }
  #endif // USE_OFR_FONTS
}

// NOTES
//   text_size can be a pixel height or it can be a GLCD multiplier. GLCD multiplier cannot be larger than 7, nor zero. 
//   draw_width is only used if it's a non-GLCD font and we're being asked to calculate the font size (based on GLCD text_size)
//
void dtb_draw_string(const char* str, int32_t draw_x, int32_t draw_y, uint32_t draw_width, uint16_t text_size, uint16_t text_color, uint8_t text_datum) {
  uint8_t height_offset = 0;

  // if text_size is 0 then assume the font has already been set outside of this function and it just needs to be drawn.
  if (text_size > 0) {

    // set GLCD font; not necessary, we're never setting this value to anything else in the code
    //tft.setTextFont(1);

    #ifdef USE_OFR_FONTS
      // if text_size is less than 8 assume it's a GLCD multiplier, otherwise assume it's a pixel height for use with OFR
      if (dtb_font == 0 || text_size < 8) {
    #endif

        tft.setTextSize((uint8_t)(text_size & 0x00FF));

    #ifdef USE_OFR_FONTS
      }
    #endif
  }

  #ifdef USE_OFR_FONTS

  // if we're using the GLCD font
  if (dtb_font == 0) {

  #endif  // USE_OFR_FONTS

    // set alignment
    tft.setTextDatum(text_datum);

    // set color
    tft.setTextColor(text_color);

    // draw the string
    tft.drawString(str, draw_x, draw_y);

  #ifdef USE_OFR_FONTS

  // else we're using a TTF font
  } else {

    // set text alignment
    set_ofr_alignment_by_datum(text_datum);

    if (text_size != 0) {

      // if text_size is less than 8 it's a GLCD font size
      // calculate and set the actual font size based on tft.fontHeight()
      if (text_size < 8) {

        if (dtb_fonts[dtb_font - 1].y_scale != 1) {
          ofr.setFontSize(ofr.calculateFitFontSize(draw_width, (uint32_t)(tft.fontHeight() * dtb_fonts[dtb_font - 1].y_scale), ofr.getLayout(), str));
        } else {
          ofr.setFontSize(ofr.calculateFitFontSize(draw_width, tft.fontHeight(), ofr.getLayout(), str));
        }

        // so this is dumb. some TTF fonts don't behave as expected with ofr.calculateFitFontSize() and the 
        // calculated font size is TOO BIG. this is a check to resolve those situations and, as a backup, set
        // the font size to tft.fontHeight(); -- it's a rough workaround, that's what i get for trying to use
        // dynamic font sizes. 
        if (dtb_get_font_height() > tft.fontHeight()) {
            ofr.setFontSize(tft.fontHeight());
        }

        // we're sizing based on the GLCD font size. our new font height will likely be smaller than the
        // GLCD font height, so we add a small offset as half the difference between the two font heights
        // to vertically center this text within the area where the GLCD font would have been
        height_offset = (tft.fontHeight() - dtb_get_font_height()) / 2;

      // else assume text_size is a pixel height
      } else {
        ofr.setFontSize(text_size);
      }
    }

    // adjust text position
    draw_y += height_offset;

    // further text position adjustments
    if (dtb_fonts[dtb_font - 1].y_offset != 0) {
      draw_y += (dtb_get_font_height() * dtb_fonts[dtb_font - 1].y_offset);
    }

    // draw the string
    ofr.drawString(str, draw_x, draw_y, text_color, ofr.getBackgroundColor(), ofr.getLayout());
  }

  #endif  // USE_OFR_FONTS
}

// load the font specified by dtb_font
void dtb_load_font() {

  #ifdef USE_OFR_FONTS

    // load the new font
    ofr.unloadFont();
    if (dtb_font != 0 && ofr.loadFont(dtb_fonts[dtb_font - 1].data, dtb_fonts[dtb_font - 1].size)) {
      dtb_font = 0;
    }

    // recalculate font properties based on new font
    list_calculate_dynamic_font_properties();
  #endif
}

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
    beacon.setting[BEACON_PARAM_LCTN_REACT] = DEFAULT_BEACON_REACTION_TIME;
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

// generate a random location beacon for the next rotating beacon. make sure we don't repeat locations.
void set_rotating_beacon() {
  static uint8_t old_id = 0;

  SERIAL_PRINTLN("Generating the next rotating beacon.");

  do {
    set_location_beacon(0);

  // generate a random beacon, again, if it's the same value as the previous (old) beacon
  } while (old_id == beacon.setting[BEACON_PARAM_LCTN_ID]);

  old_id = beacon.setting[BEACON_PARAM_LCTN_ID];
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

//void display_list(const char **items, uint8_t num_items) {
void display_list(uint8_t list_index) {
  uint8_t rows, row_padding, max_padding, i;
  uint16_t row_height, row_width, text_color, font_height;
  int16_t  y = 0;

  // get the pixel height of the font
  #ifdef USE_OFR_FONTS
    if (dtb_font != 0) {
      SERIAL_PRINT("ofr_font_size: ");
      SERIAL_PRINTLN(lists[list_index].render_options.ofr_font_size);
      font_height = lists[list_index].render_options.ofr_font_height; //lists[list_index].render_options.ofr_font_size;
    } else {
  #endif
      SERIAL_PRINT("text_size: ");
      SERIAL_PRINTLN(lists[list_index].render_options.text_size);
      tft.setTextSize(lists[list_index].render_options.text_size);
      font_height = tft.fontHeight();
  #ifdef USE_OFR_FONTS
    }
  #endif

  // calculate vertical padding (aka row_padding) limits and update row_padding
  // if currently set value is outside of available limits
  // 
  // since font size can be changed via a define, or that this code might run on a screen
  // that i haven't tested with, check to make sure padding isn't so big that we can't show 
  // at least 2 menu items on the screen.
  //
  // divide the available space by 6; padding top/bottom of each item (4) + leave an extra gap at the very top and very bottom
  row_padding = lists[list_index].render_options.text_padding;
  max_padding = (tft.getViewportHeight() - (font_height * 2))/6;
  if (max_padding < 1) {
    row_padding = 0;
  } else if (max_padding < row_padding) {
    row_padding = max_padding;
  }

  // calculate how tall each row will be; font height + top and bottom padding + border width
  row_height = font_height + (row_padding * 2);

  SERIAL_PRINT("row_height: ");
  SERIAL_PRINTLN(row_height);
  SERIAL_PRINT("font_height: ");
  SERIAL_PRINTLN(font_height);
  SERIAL_PRINT("row_padding: ");
  SERIAL_PRINTLN(row_padding);

  // calculate how many rows will fit within the screen
  rows = tft.getViewportHeight() / row_height;

  // pickup the precalculated row_width
  row_width = lists[list_index].render_options.row_width;

  // add some horizontal padding, if there's room for it.
  //
  // should calculate a tft.getViewPortWidth() value earlier in this function
  // which has side gutter space subtracted ahead of time...
  if (row_width < tft.getViewportWidth()) {
    row_width = row_width + ((tft.getViewportWidth() - row_width)/4);
  }

  // LIST RENDERING METHODOLOGY:
  //   to give a sense of moving through a menu that's taller than the screen, i want the selected item to be
  //   in a position on the screen relative to its location in the list. so the first item is rendered at the top,
  //   the last item is rendered at the bottom, and intervening items render somewhere in the middle
  //
  //   i tested several different approaches and i found this "feels" right given the limited screen space

  // is the list larger than the screen?
  if (lists[list_index].num_items > rows && rows >= 1)  {

    // where should the selected item appear on screen?
    y = ((tft.getViewportHeight() - row_height) / (lists[list_index].num_items - 1)) * selected_item;

    // where should the list start on (or off) screen in order to put the selected item at the 
    // previously calculated location?
    y -= (row_height * selected_item);

  // entire list will fit on the screen, vertically center it
  } else {

    // y is set to where the top of the first menu item will be rendered
    y = (tft.getViewportHeight() - (row_height * lists[list_index].num_items))/2;
  }

  // draw the list, starting at the previously calculated position (y)
  for (i = 0; i < lists[list_index].num_items; i++) {

    // is the current list item selected?
    if (i == selected_item) {

      // draw a box around the list item
      tft.drawRect(
        ((tft.getViewportWidth() - row_width) / 2),
        y,
        row_width,
        row_height,
        lists[list_index].render_options.selected_border_color
      );

      // set the text color for selected list items
      //tft.setTextColor(list_options.selected_text_color);
      text_color = lists[list_index].render_options.selected_text_color;
    } else {

      // this item is not selected, set its text color to the normal color
      //tft.setTextColor(list_options.text_color);
      text_color = lists[list_index].render_options.text_color;
    }

    // draw the list item on the screen
    if (lists[list_index].render_options.ofr_font_size != 0) {
      dtb_draw_string(lists[list_index].items[i], tft.getViewportWidth()/2,  y + row_padding + 1, row_width, lists[list_index].render_options.ofr_font_size, text_color, TC_DATUM);   // TC_DATUM
    } else {
      dtb_draw_string(lists[list_index].items[i], tft.getViewportWidth()/2,  y + row_padding + 1, row_width, lists[list_index].render_options.text_size, text_color, TC_DATUM);       // TC_DATUM
    }

    // increment y for the next list item
    y += row_height;
  }
}

void display_captioned_menu(const char* caption, uint8_t list_index) {
  uint8_t h;

  // draw menu caption
  dtb_draw_string(caption, tft.width()/2, MENU_SELECT_CAPTION_TEXT_PADDING, DEFAULT_TEXT_FIT_WIDTH, MENU_SELECT_CAPTION_TEXT_SIZE, MENU_SELECT_CAPTION_TEXT_COLOR, TC_DATUM);

  // calculate viewport dimensions for subsequent menu
  h = tft.fontHeight() + (MENU_SELECT_CAPTION_TEXT_PADDING * 2);
  tft.setViewport(0, h, tft.width(), tft.height()-h);

  // display the menu
  display_list(list_index);
}


void display_set_interval() {
  char msg[MSG_LEN_MAX];
  uint16_t y, content_height;
  uint8_t gap, bfs;

  // line 1: set beacon interval
  // line 2: %d sec
  // line 3: gap
  // line 4: start 

  // set the default font size to 1 and get the pixel height of the text at this size; this will be the base unit used to calculate the size of everything on the screen
  tft.setTextSize(1);
  bfs = tft.fontHeight();

  // calclualte a gap between the beacon title and the activate option
  gap = (bfs * BEACON_CONTROL_TEXT_SIZE) / 2;

  // calculate the height of the content to vertically align everything
  content_height = (bfs * BEACON_INTERVAL_TITLE_TEXT_SIZE) + (bfs * BEACON_INTERVAL_VALUE_TEXT_SIZE) + gap + (bfs * BEACON_INTERVAL_SET_TEXT_SIZE) + (BEACON_INTERVAL_TEXT_PADDING * 3);

  // calculate where to begin drawing text  
  y = (tft.getViewportHeight() - content_height)/2;

  // display title
  dtb_draw_string(msg_set_beacon_interval, tft.getViewportWidth()/2, y, DEFAULT_TEXT_FIT_WIDTH, BEACON_INTERVAL_TITLE_TEXT_SIZE, BEACON_INTERVAL_TITLE_COLOR, TC_DATUM);
  y += (bfs * BEACON_INTERVAL_TITLE_TEXT_SIZE) + BEACON_INTERVAL_TEXT_PADDING;

  // display current interval
  snprintf(msg, MSG_LEN_MAX, msg_int_sec, beacon_rotate_interval * 10);
  dtb_draw_string(msg, tft.getViewportWidth()/2, y, DEFAULT_TEXT_FIT_WIDTH, BEACON_INTERVAL_VALUE_TEXT_SIZE, (selected_item == 0 ? BEACON_INTERVAL_VALUE_COLOR_S : BEACON_INTERVAL_VALUE_COLOR), TC_DATUM);
  y += (bfs * BEACON_INTERVAL_VALUE_TEXT_SIZE) + gap + (BEACON_INTERVAL_TEXT_PADDING * 2) ;

  // display set interval action
  dtb_draw_string(msg_set_interval, tft.getViewportWidth()/2, y, DEFAULT_TEXT_FIT_WIDTH, BEACON_INTERVAL_SET_TEXT_SIZE, (selected_item == 1 ? BEACON_INTERVAL_SET_COLOR_S : BEACON_INTERVAL_SET_COLOR), TC_DATUM);
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

  // set the default font size to 1 and get the pixel height of the text at this size; this will be the base unit used to calculate the size of everything on the screen
  tft.setTextSize(1);
  bfs = tft.fontHeight();
  
  // calclualte a gap between the beacon title and the activate option
  gap = (bfs * BEACON_CONTROL_TEXT_SIZE) / 2;

  // calculate the height of the content to vertically align everything
  content_height = (bfs * BEACON_CONTROL_TYPE_TEXT_SIZE) + (bfs * BEACON_CONTROL_TEXT_SIZE *2) + gap + (BEACON_CONTROL_TEXT_PADDING * 3);

  // calculate where to begin drawing text  
  y = (tft.getViewportHeight() - content_height)/2;

  // display beacon type
  if (beacon.type == DROID) {
    snprintf(msg, MSG_LEN_MAX, "%s %s", msg_droid, msg_beacon);
  } else {
    snprintf(msg, MSG_LEN_MAX, "%s %s", msg_location, msg_beacon);
  }
  dtb_draw_string(msg, tft.getViewportWidth()/2, y, DEFAULT_TEXT_FIT_WIDTH, BEACON_CONTROL_TYPE_TEXT_SIZE, BEACON_CONTROL_TYPE_COLOR, TC_DATUM);

  // adjust position for next line
  y += tft.fontHeight() + BEACON_CONTROL_TEXT_PADDING;

  // display beacon id
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
  dtb_draw_string(msg, tft.getViewportWidth()/2, y, DEFAULT_TEXT_FIT_WIDTH, BEACON_CONTROL_TEXT_SIZE, BEACON_CONTROL_ID_COLOR, TC_DATUM);

  // adjust position for next line
  y += tft.fontHeight() + (BEACON_CONTROL_TEXT_PADDING * 2) + gap;

  // display beacon state
  if (state == BEACON_ACTIVE) {
    dtb_draw_string(msg_beacon_active,   tft.getViewportWidth()/2, y, DEFAULT_TEXT_FIT_WIDTH, BEACON_CONTROL_TEXT_SIZE, BEACON_CONTROL_ACTIVE_COLOR,   TC_DATUM);
  } else {
    dtb_draw_string(msg_activate_beacon, tft.getViewportWidth()/2, y, DEFAULT_TEXT_FIT_WIDTH, BEACON_CONTROL_TEXT_SIZE, BEACON_CONTROL_INACTIVE_COLOR, TC_DATUM);
  }
}

// display the splash screen seen when the program starts
void display_splash() {
  uint16_t y = 0, c;
  uint8_t tmp_font;
  char msg[MSG_LEN_MAX];

  // location the Y position to begin drawing to center vertically the text
  tft.setTextSize(1);
  y = (tft.height() - (tft.fontHeight() * ((SPLASH_TEXT_SIZE * 6) + 1))) / 2;
  tft.setCursor(0, y);

  // title
  dtb_draw_string(msg_title, tft.getViewportWidth()/2, y, tft.getViewportWidth(), SPLASH_TEXT_SIZE + 1, SPLASH_TITLE_COLOR, TC_DATUM);
  y += tft.fontHeight();

  // contact
  dtb_draw_string(msg_email, tft.getViewportWidth()/2, y, DEFAULT_TEXT_FIT_WIDTH, SPLASH_TEXT_SIZE, SPLASH_SUBTITLE_COLOR, TC_DATUM);
  y += (tft.fontHeight() * 2);

  // press any button...
  dtb_draw_string(msg_continue1, tft.getViewportWidth()/2, y, DEFAULT_TEXT_FIT_WIDTH, SPLASH_TEXT_SIZE, SPLASH_TEXT_COLOR, TC_DATUM);
  y += tft.fontHeight();

  // passing a text size of 0, causing dtb_draw_string() to use whatever the current font size is; this way both lines of 'press any button...' will
  // be the same size.
  dtb_draw_string(msg_continue2, tft.getViewportWidth()/2, y, DEFAULT_TEXT_FIT_WIDTH, 0, SPLASH_TEXT_COLOR, TC_DATUM);

  #ifdef USE_OFR_FONTS
    // backup dtb_font
    tmp_font = dtb_font;

    // set dtb_font to glcd so version and bat are always displayed in GLCD
    // if this is useless, just comment out the line below.
    dtb_font = 0;
  #endif

  // version
  dtb_draw_string(msg_version, tft.getViewportWidth(), tft.getViewportHeight(), tft.getViewportWidth()/2, SPLASH_TEXT_SIZE, SPLASH_VERSION_COLOR, BR_DATUM);

  // battery voltage
  y = (analogRead(BAT_ADC_PIN) * 2 * 3.3 * 1000) / 4096;
  c = SPLASH_VERSION_COLOR;
  if (y < 3400) {
    c = C565(128,0,0);        // need to charge the battery
  } else if (y < 3800) {
    c = C565(96,96,0);        // battery is getting low
  } else if (y < 4400) {
    c = C565(0,128,0);        // battery is charged
  } else {
    c = SPLASH_VERSION_COLOR; // you're probably on USB
  }
  snprintf(msg, MSG_LEN_MAX, "%s:%.2fV", (y<4400 ? "BAT" : "PWR"), (y / (float)1000));
  dtb_draw_string(msg, 0, tft.getViewportHeight(), tft.getViewportWidth()/2, SPLASH_TEXT_SIZE, c, BL_DATUM);

  #ifdef USE_OFR_FONTS
    // restore dtb_font
    dtb_font = tmp_font;
  #endif
}

void display_scanner_results() {
  char msg[MSG_LEN_MAX];
  uint16_t y = 0;
  uint8_t i;
  const char* name = nullptr;
  affiliaton_t* a = nullptr;
  personality_t* p = nullptr;

  // display header
  dtb_draw_string(msg_droid_report, tft.getViewportWidth()/2, 0, DEFAULT_TEXT_FIT_WIDTH, DROID_REPORT_TEXT_SIZE, DROID_REPORT_COLOR, TC_DATUM);

  // add a gap after the header
  if (droid_count > 0) {

    // find where to start printing droid details so that it is vertically centered
    tft.setTextSize(1);
    y = (tft.height() - ((tft.fontHeight() * (DROID_REPORT_TEXT_SIZE + 1)) + (tft.fontHeight() * DROID_REPORT_TEXT_SIZE * 3)))/2;

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
      dtb_draw_string(name, tft.getViewportWidth()/2, y, DEFAULT_TEXT_FIT_WIDTH, DROID_REPORT_TEXT_SIZE + 1, DROID_REPORT_PERSONALITY_COLOR, TC_DATUM);
    } else {
      snprintf(msg, MSG_LEN_MAX, msg_unknown_int, droids[current_droid].chipid);
      dtb_draw_string(msg, tft.getViewportWidth()/2, y, DEFAULT_TEXT_FIT_WIDTH, DROID_REPORT_TEXT_SIZE + 1, DROID_REPORT_PERSONALITY_COLOR, TC_DATUM);
    }

    // print droid affiliation
    y += tft.fontHeight();
    a = get_droid_affiliation(droids[current_droid].affid);
    if (a != nullptr) {
      dtb_draw_string(a->name, tft.getViewportWidth()/2, y, DEFAULT_TEXT_FIT_WIDTH, DROID_REPORT_TEXT_SIZE, DROID_REPORT_AFFILIATION_COLOR, TC_DATUM);
    } else {
      snprintf(msg, MSG_LEN_MAX, msg_unknown_int, droids[current_droid].affid);
      dtb_draw_string(msg, tft.getViewportWidth()/2, y, DEFAULT_TEXT_FIT_WIDTH, DROID_REPORT_TEXT_SIZE, DROID_REPORT_AFFILIATION_COLOR, TC_DATUM);
    }

    // print Bluetooth MAC address
    y += tft.fontHeight();
    dtb_draw_string(droids[current_droid].pAdvertisedDevice->getAddress().toString().c_str(), tft.getViewportWidth()/2, y, DEFAULT_TEXT_FIT_WIDTH, DROID_REPORT_TEXT_SIZE, DROID_REPORT_ADDRESS_COLOR, TC_DATUM);

    // print RSSI
    y += tft.fontHeight();
    snprintf(msg, MSG_LEN_MAX, msg_rssi, droids[current_droid].pAdvertisedDevice->getRSSI());
    dtb_draw_string(msg, tft.getViewportWidth()/2, y, DEFAULT_TEXT_FIT_WIDTH, DROID_REPORT_TEXT_SIZE, DROID_REPORT_RSSI_COLOR, TC_DATUM);

    // print
    snprintf(msg, MSG_LEN_MAX, msg_d_of_d, current_droid + 1, droid_count);
    dtb_draw_string(msg, tft.getViewportWidth()/2, tft.getViewportHeight(), DEFAULT_TEXT_FIT_WIDTH, DROID_REPORT_TEXT_SIZE, DROID_REPORT_COLOR, BC_DATUM);

  // display message that no droids were found
  } else {

    // set the font size
    dtb_set_font_size(DROID_REPORT_TEXT_SIZE + 1, DEFAULT_TEXT_FIT_WIDTH, msg_no_droids1);

    // calculate where to start rendering the message
    y = (tft.height()/2) - dtb_get_font_height();

    // render the message
    dtb_draw_string(msg_no_droids1, 
      tft.width()/2, y, 0, 0, ACTION_RESULT_NG_TEXT_COLOR, TC_DATUM
    );
    dtb_draw_string(msg_no_droids2, 
      tft.width()/2, y + dtb_get_font_height(), 0, 0, ACTION_RESULT_NG_TEXT_COLOR, TC_DATUM
    );
  }
}

void display_sounds() {
  const char tst_msg[] = "88";
  char msg[MSG_LEN_MAX];
  uint16_t x, y, w;

  // line1: group
  // line2: track
  // line3: gap
  // line4: play

  // figure out a font size to use
  dtb_set_font_size(SOUNDS_TEXT_SIZE, DEFAULT_TEXT_FIT_WIDTH/2, msg_group);

  // let's figure out how to center all this 
  snprintf(msg, MSG_LEN_MAX, "%s: ", msg_group);
  x = dtb_get_text_width(msg);
  snprintf(msg, MSG_LEN_MAX, "%s: ", msg_track);
  y = dtb_get_text_width(msg);
  if (y > x) {
    x = y;
  }

  // the vertical split is located
  x = ((tft.getViewportWidth() - (x + dtb_get_text_width(tst_msg) + 10))/2) + x;

  // find the starting point to draw text
  y = (tft.fontHeight() * 4) + SOUNDS_TEXT_PADDING;
  y = (tft.getViewportHeight() - y)/2;

  // display group label
  snprintf(msg, MSG_LEN_MAX, "%s: ", msg_group);
  dtb_draw_string(msg, x, y, 0, 0, SOUNDS_GROUP_COLOR, TR_DATUM);

  // display group value
  snprintf(msg, MSG_LEN_MAX, "%d", (current_group + 1));
  dtb_draw_string(msg, x+10, y, 0, 0, (state == SOUND_GROUP ? SOUNDS_VALUE_SELECTED_COLOR : SOUNDS_VALUE_COLOR), TL_DATUM);

  // move next line
  y += tft.fontHeight() + SOUNDS_TEXT_PADDING;

  // display track label
  snprintf(msg, MSG_LEN_MAX, "%s: ", msg_track);
  dtb_draw_string(msg, x, y, 0, 0, SOUNDS_TRACK_COLOR, TR_DATUM);

  // display track value
  snprintf(msg, MSG_LEN_MAX, "%d", (current_track + 1));
  dtb_draw_string(msg, x+10, y, 0, 0, (state == SOUND_TRACK ? SOUNDS_VALUE_SELECTED_COLOR : SOUNDS_VALUE_COLOR), TL_DATUM);

  // move down to draw play button
  y += (tft.fontHeight() * 2);

  // display play button
  dtb_draw_string(msg_play, tft.getViewportWidth()/2, y, 0, 0, (state == SOUND_PLAY ? SOUNDS_VALUE_SELECTED_COLOR : SOUNDS_VALUE_COLOR), TC_DATUM);
}

void display_volume() {
  char msg[MSG_LEN_MAX];
  uint16_t content_height;
  uint16_t x, y, text_color;

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
    text_color = VOLUME_LOW_COLOR;
  } else if (droid_volume < 80) {
    text_color = VOLUME_MED_COLOR;
  } else if (droid_volume < 100) {
    text_color = VOLUME_HIGH_COLOR;
  } else {
    text_color = VOLUME_MAX_COLOR;
  }

  // display volume
  snprintf(msg, MSG_LEN_MAX, "%d", droid_volume);
  dtb_draw_string(msg, tft.getViewportWidth()/2, y, DEFAULT_TEXT_FIT_WIDTH, VOLUME_TEXT_SIZE + 2, text_color, TC_DATUM);

  // next line
  y += tft.fontHeight();
  
  // gap
  tft.setTextSize(VOLUME_TEXT_SIZE);
  y += tft.fontHeight();

  // why am i setting size with this instead of setting it in dtb_draw_string()?
  // because i'm dumb.
  // and dtb_draw_string calculates height_offset only if font size and render width are passed to dtb_set_font_size
  // so either i set font size here and ignore height_offset
  // or i pass width and size to all of these functions
  //
  // i should probably revisit dtb_draw_string and see if this goofball limitation/issue is relevant.. 
  // it only exists to operate under the assumption that an ofr fontsize can be passed, which I don't think i'm doing.
  //
  // oh, and hi, if you're reading comments. i apologize for the code. it's long. convoluted. and probably overly complicated
  // but this is how i code; i make it work, then i figure out if it can be done better. then i figure out if it can be done cleaner.
  // i haven't gotten to the better and cleaner parts, yet.
  dtb_set_font_size(VOLUME_TEXT_SIZE, DEFAULT_TEXT_FIT_WIDTH/2.5, msg_vol_inc);

  // vol+
  text_color = (state == VOLUME_UP ? VOLUME_SELECTED_TEXT_COLOR : VOLUME_TEXT_COLOR);
  dtb_draw_string(msg_vol_inc, (tft.getViewportWidth()/2) - VOLUME_TEXT_PADDING, y, 0, 0, text_color, TR_DATUM);

  // vol-
  text_color = (state == VOLUME_DOWN ? VOLUME_SELECTED_TEXT_COLOR : VOLUME_TEXT_COLOR);
  dtb_draw_string(msg_vol_dec, (tft.getViewportWidth()/2) + VOLUME_TEXT_PADDING, y, 0, 0, text_color, TL_DATUM);

  // next line
  y += (tft.fontHeight() + VOLUME_TEXT_PADDING);

  // set volume
  text_color = (state == VOLUME_TEST ? VOLUME_SELECTED_TEXT_COLOR : VOLUME_TEXT_COLOR);
  dtb_draw_string(msg_set_vol, tft.getViewportWidth()/2, y, 0, 0, text_color, TC_DATUM);
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

void sneaky_beacon_countdown() {
  static char msg[16];
  static uint16_t last_s = 0;
  uint16_t s;

/*
  #ifdef USE_OFR_FONTS
    BgFillMethod fm;
  #endif
*/

  s = (uint16_t)((next_beacon_time - millis())/1000);

  if (s != last_s) {

    // add leading spaces so it draws over the previous number if the number width shrinks
    snprintf(msg, 16, "    %d", s);

/*
    #ifdef USE_OFR_FONTS
      if (dtb_font != 0) {
        fm = ofr.getBackgroundFillMethod();
        ofr.setBackgroundFillMethod(BgFillMethod::Block);
        dtb_draw_string(msg, tft.getViewportWidth(), tft.getViewportHeight(), tft.getViewportWidth(), BEACON_CONTROL_COUNTDOWN_SIZE, BEACON_CONTROL_COUNTDOWN_COLOR, BR_DATUM);
        ofr.setBackgroundFillMethod(fm);
      } else {
    #endif
*/

        // have to bypass dtb_draw_string() in this case as it will not fill in the background when drawing text using tft.drawString()
        // and i do not feel like adding another parameter to that function and having to update a bunch of function calls
        //
        // also, when i make a rotating beacon inactive, should i reset the timer when i make it active again? (probably)
        tft.setTextDatum(BR_DATUM);
        tft.setTextSize(BEACON_CONTROL_COUNTDOWN_SIZE);
        tft.setTextColor(BEACON_CONTROL_COUNTDOWN_COLOR, TFT_BLACK, true);
        tft.drawString(msg, tft.getViewportWidth(), tft.getViewportHeight());

/*
    #ifdef USE_OFR_FONTS
      }
    #endif
*/

    last_s = s;
  }
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
      display_captioned_menu(msg_select, LIST_CONNECTED_MENU);
      break;

    case SCANNER_CONNECTED:      // display_connected()

      // display connected message
      dtb_draw_string(msg_scanner_connected, 
        tft.getViewportWidth()/2, (tft.getViewportHeight()/2) - (dtb_get_font_height()/2), DEFAULT_TEXT_FIT_WIDTH, ACTION_TEXT_SIZE, ACTION_RESULT_OK_TEXT_COLOR, TC_DATUM
      );
      break;

    case SCANNER_CONNECT_FAILED: // display_connect_failed()
      dtb_set_font_size(ACTION_TEXT_SIZE, DEFAULT_TEXT_FIT_WIDTH, msg_scanner_connected);
      dtb_draw_string(msg_connect, 
        tft.getViewportWidth()/2, (tft.getViewportHeight()/2) - dtb_get_font_height(), 0, 0, ACTION_RESULT_NG_TEXT_COLOR, TC_DATUM
      );
      dtb_draw_string(msg_failed, 
        tft.getViewportWidth()/2, (tft.getViewportHeight()/2), 0, 0, ACTION_RESULT_NG_TEXT_COLOR, TC_DATUM
      );
      break;

    case SCANNER_CONNECTING:     // display_connecting()

      // display message to turn off droid remote at top of screen
      dtb_draw_string(msg_turn_off_remote1, tft.getViewportWidth()/2, 0, DEFAULT_TEXT_FIT_WIDTH, DROID_REPORT_TEXT_SIZE, DROID_REPORT_COLOR, TC_DATUM);
      dtb_draw_string(msg_turn_off_remote2, tft.getViewportWidth()/2, tft.fontHeight(), DEFAULT_TEXT_FIT_WIDTH, 0, DROID_REPORT_COLOR, TC_DATUM);

      // display CONNECTING message
      dtb_draw_string(msg_scanner_connecting, 
        tft.getViewportWidth()/2, (tft.getViewportHeight()/2) - (dtb_get_font_height()/2), DEFAULT_TEXT_FIT_WIDTH, ACTION_TEXT_SIZE, ACTION_TEXT_COLOR, TC_DATUM
      );
      break;

    case SCANNER_RESULTS:        // display_scanner_results()
      display_scanner_results();
      break;

    case SCANNER_SCANNING:       // display_scanning()

      // for wahtever reason, using TC_DATUM instead of MC_DATUM renders more vertically-centered
      dtb_set_font_size(ACTION_TEXT_SIZE, DEFAULT_TEXT_FIT_WIDTH, msg_scanner_active);
      dtb_draw_string(msg_scanner_active, 
        tft.getViewportWidth()/2, (tft.getViewportHeight()/2) - (dtb_get_font_height()/2), 0, 0, ACTION_TEXT_COLOR, TC_DATUM
      );
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
      display_captioned_menu(msg_select_beacon, LIST_PERSONALITIES);
      break;

    case BEACON_LOCATION_LIST:   // display_location_beacon_list()
      display_captioned_menu(msg_select_beacon, LIST_LOCATIONS);
      break;

    case BEACON_SET_INTERVAL:    // display_set_interval()
      display_set_interval();
      break;

    case BEACON_TYPE_MENU:       // display_beacon_type_menu()
      display_captioned_menu(msg_select_beacon_type, LIST_BEACON_TYPE_MENU);
      break;

    case TOP_MENU:               // display_top_menu()
      display_captioned_menu(msg_select, LIST_TOP_MENU);
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

    // on splash screen
    case SPLASH:

      // only swap fonts if USE_OFR_FONTS is defined
      #ifdef USE_OFR_FONTS

        // a long button1 press will cause the font to change
        if (press_type == LONG_PRESS) {
            dtb_font++;
            if (dtb_font > NUM_FONTS) {
              dtb_font = 0;
            }

            // load the new font
            dtb_load_font();

        // otherwise go to the top menu
        } else {

          // before exiting the splash screen, store the current font to NVS
          #ifdef USE_NVS
            preferences.putUChar("dtb_font", dtb_font);
          #endif
      #endif

        state = TOP_MENU;
        selected_item = 0;

      #ifdef USE_OFR_FONTS
        }
      #endif

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

      // initiate a rotating beacon
      } else if (state == BEACON_ROTATING) {
        state = BEACON_SET_INTERVAL;
        selected_item = 0;

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

    case BEACON_SET_INTERVAL:
      if (selected_item == 0) {
        if (beacon_rotate_interval >= MAX_BEACON_CHANGE_INTERVAL) {
          beacon_rotate_interval = 6;
        } else {
          if (press_type == SHORT_PRESS) {
            beacon_rotate_interval += SHORT_PRESS_INTERVAL_INC;
          } else {
            beacon_rotate_interval += LONG_PRESS_INTERVAL_INC;
          }
          if (beacon_rotate_interval > MAX_BEACON_CHANGE_INTERVAL) {
            beacon_rotate_interval = MAX_BEACON_CHANGE_INTERVAL;
          }
        }
      } else {
        set_rotating_beacon();
        next_beacon_time = 1;
        state = BEACON_ACTIVATE;
        selected_item = 0;
        button1(SHORT_PRESS);
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
      if (next_beacon_time != 0) {
        next_beacon_time = millis() + (beacon_rotate_interval * 10000); 
      }
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
          beacon.setting[BEACON_PARAM_LCTN_ID]      = 1;  // default location
          beacon.setting[BEACON_PARAM_LCTN_REACT]   = 2;  // default interval
          beacon.setting[BEACON_PARAM_LCTN_RSSI]    = 38; // default minimim rssi
          beacon.setting[BEACON_PARAM_LCTN_PARAM4]  = 0;  // unused
        } else {
          beacon.type = DROID;
          beacon.setting[BEACON_PARAM_DROID_ID]     = 1; // default personality
          beacon.setting[BEACON_PARAM_DROID_AFFL]   = 1; // default affiliation
          beacon.setting[BEACON_PARAM_DROID_PAIRED] = 1; // default paired
          beacon.setting[BEACON_PARAM_DROID_BDROID] = 0; // be a droid
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
              break;

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

      // before exiting the splash screen, store the current font to NVS
      #if defined (USE_OFR_FONTS) && defined (USE_NVS)
        preferences.putUChar("dtb_font", dtb_font);
      #endif

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

    case BEACON_SET_INTERVAL:

      // if a back menu button press...
      if (press_type == LONG_PRESS) {

        // find the location of rotating beacon in the beacon type menu
        for (selected_item=0; selected_item < (sizeof(beacon_type_menu) / sizeof(menu_item_t)); selected_item++) {
          if (beacon_type_menu[selected_item].state == BEACON_ROTATING) {
            break;
          }
        }

        // if rotating beacon type wasn't found in the menu then just set selected_item to the first item in the list
        if (selected_item >= (sizeof(beacon_type_menu) / sizeof(menu_item_t))) {
          next_beacon_time = 0;
          selected_item = 0;
        }
        state = BEACON_TYPE_MENU;

      } else {
        selected_item = (selected_item+1) % 2;
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

    case BEACON_ACTIVE:
    case BEACON_ACTIVATE:

      // stop advertising the active beacon
      if (state == BEACON_ACTIVE) {
        pAdvertising->stop();
      }

      // if a back menu button press...
      if (press_type == LONG_PRESS) {

        // if exiting a rotating beacon
        if (next_beacon_time != 0) {
          state = BEACON_SET_INTERVAL;
          next_beacon_time = 0;
          selected_item = 1;
        } else if (beacon.type == DROID) {
          state = BEACON_DROID_LIST;
        } else {
          state = BEACON_LOCATION_LIST;
        }

      // change state to an inactive beacon
      } else if (state == BEACON_ACTIVE) {
        state = BEACON_ACTIVATE;

      // activate the beacon
      } else {
        if (next_beacon_time != 0) {
          next_beacon_time = millis() + (beacon_rotate_interval * 10000); 
        }
        init_advertisement_data(nullptr);
        set_payload_from_beacon();
        pAdvertising->start();
        state = BEACON_ACTIVE;
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

  // init serial
  SERIAL_BEGIN(115200);

  // T-Display-S3 needs this in order to run off battery
  #ifdef TDISPLAYS3
    pinMode(15, OUTPUT);
    digitalWrite(15, HIGH);
  #endif

  // setup display
  tft.init();
  tft.setRotation(3);
  reset_screen();

  // attach ofr to tft
  #ifdef USE_OFR_FONTS
    ofr.setDrawer(tft);  
  #endif

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

  // initialize the lists used for rendering menus
  list_init();
  list_calculate_dynamic_font_properties();

  // initialize NVS
  #if defined (USE_NVS) && defined (USE_OFR_FONTS)
    preferences.begin(PREF_APP_NAME, false); 
    //preferences.clear();

    // load stored font
    dtb_font = (uint8_t)preferences.getUChar("dtb_font", 0);
    dtb_load_font();
  #endif

  // end of setup
  SERIAL_PRINTLN("Ready!");
}

void loop() {
  static uint16_t s = 0;

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

    case BEACON_ACTIVE:

      // test if we're in rotating beacon mode
      if (next_beacon_time != 0) {

        // is it time to change the beacon?
        if (millis() >= next_beacon_time) {

          // stop the current beacon
          button1(SHORT_PRESS);

          // set a new beacon at random
          // TODO: add some code to make sure we don't repeat the same beacon twice
          set_rotating_beacon();
          next_beacon_time = millis() + (beacon_rotate_interval * 10000);

          // start the new beacon
          state = BEACON_ACTIVATE;
          button1(SHORT_PRESS);

        // print a sneaky countdown to the next beacon in the corner
        } else if (tft_update != true) {
          sneaky_beacon_countdown();
        }
      }
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
