# Changelog

Changelog for [Droid-Toolbox](https://github.com/ruthsarian/droid-toolbox/) by ruthsarian@gmail.com.

Changelog format is based on [Keep a Changelog](https://keepachangelog.com/en/1.1.0/).

## [Unreleased]

### Changed

- Reorganizing the code by breaking it up into several, smaller files with
  each one containing shared functionatliy (ble.c, beacons.c, menus.c, etc.)

## [0.81] = 2025-07-28

### Fixed

- jmachuca77 noted that newer released of OpenFontRenderer were causing the 
  text to appear as only white. it seems that OFR's drawString() function is
  ignoring the text color value. however setting the color via 
  setFontColor() still works. I've updated dtb_draw_string() to do this
  prior to calling drawString().
- discovered newer versions of the Espressif ESP32 core are alignign their
  macros with the NimBLE standard. I've updated the code to use these new
  macros and to use the old macros if the new ones do not exist in order to
  keep older cores functional with the droid toolbox code.

## [0.80] = 2025-02-09

### Added

- AMOLED users can change the screen brightness with a short button 2 press
  on the splash screen. This setting will be retained through power cycles.

## [0.79] = 2025-02-03

### Fixed

- Fixed a compile error when building with ESP32 2.x.x.
  Thanks to Brandon for identifying this issue.
- Updated comments at top of INO file to indicate only required board settings 
  and removed the settings that aren't required.

## [0.78] - 2025-02-03

### Fixed

- Fixed a bug preventing the toolbox from staying in deep sleep until a button
  is pressed.

## [0.77] - 2025-01-29

### Added

- Added a define, BTN_UP_STATE, which should be set to HIGH or LOW to define
  what logic level should be used to identify a button is in the "up" or
  "not pressed" state.

### Changed

- Modified button initializion in setup() to set pinMode on input buttons as
  either INPUT_PULLUP or INPUT_PULLDOWN (if supported) based on the value
  of BTN_UP_STATE. If both defines do not exist, it just sets the pin mode as
  INPUT.

- Because of the pin mode change, button 1 on T-Display's can now be used
  to wake the device from sleep.

- Moved changelog into its own file and out of the the comments at the top 
  of the INO file.

## [0.76] - 2025-01-23

### Fixed

- Rolled back the automatic button detection for AMOLED devices because V1 
  basics, which have 2 buttons, are being detected as only having 1 button. 
  You'll have to manually set the IO pins for the buttons like everyone else.
  Thanks to Chrisedge for identifying this issue.

## [0.75] - 2025-01-23

### Added

- Added support for LilyGo T-Display AMOLED devices, although touch 
  functionality is not used at this time. f using a LilyGo T-Display AMOLED 
  device you need to have Setup206_LilyGo_T_Display_S3 uncommented in 
  user_setup_select.h for TFT_eSPI library.
- Added option to have BUTTON1 behave as both BUTTON1 and BUTTON2 for those 
  devices that have only one button. To enable this feature, uncomment
  SINGLE_BUTTON_MODE in the source code.

## [0.74] - 2025-01-19

### Added

- When connecting to a droid, the droid's BLE address is saved to 
  non-volatile memory. You can quick-connect to that droid in the future, 
  through power-cycles of the toolbox, by holding button 2 for more than 1/2 
  second then releasing it while on the splash screen. 
  This feature was suggested by HighVoltage on the SWGE discord server.

## [0.73] - 2024-08-08

### Added

- Added Drum Kit and A-LT personalities.
  Thanks to Nick T for providing the beacon data of the A-LT droid.

## [0.72] - 2023-06-08

### Fixed

- Fixed compatibility with recently released ESP32 3.0 core.
  Thanks to Richs1077 and Elcid8687 for bringing this issue to my attention.

## [0.71] - 2024-05-08

### Fixed

- Fixed heap corruption with BLE scan advertisement when building with 
  Arduino-ESP32 core >= 2.0.15.
  Thanks to dtshepherd for identifying this issue and developing the fix.

## [0.70] - 2024-01-30

### Fixed

- Fixed beacon menu font size issues with TTGO T-Display.
  Thanks to Knucklebuster620 for bringing this issue to my attention.

## [0.69] - 2023-11-05

The Wayfinder Version 

### Added

- Toolbox remembers font selection through reboot/power cycle.

## [0.68] - 2023-08-22 

### Added

- Added a few defines to let you control the interval settings for short and 
  long button presses.

### Changed

- Limited rotating beacons to just location beacon types. Reason is that 
  droids will not respond to a droid beacon if it's seen a location beacon 
  within the last 2 hours. 
- Changed initial interval for rotating beacons to 60 seconds.

## [0.67] - 2023-08-21

### Added

- Added rotating beacon option; includes ability to set beacon interval 
  between 60 and 1440 seconds.

## [0.66] - 2023-08-13

### Added

- Added TFGunray font; originally added for demonstration.

## [0.65] - 2023-08-12

### Added

- Added support for custom fonts via OpenFontRenderer.
  (https://github.com/takkaO/OpenFontRender)
- Added a few fonts from aurekfonts.github.io that were labeled as free for 
  personal and commercial use.
- Added several functions (dtb_*) to assist in supporting custom fonts.
- Complicated display lists further by adding a step to pre-calculate some font
  dimensions on font change to help speed up font rendering.
- Fell into a stupor trying to get fonts behaving nicely; came out of it with 
  little understanding how this code works.

### Changed

- Consolidated some defines and functions related to lists.
- Changed the names of a couple location beacons after reviewing existing 
   beacon location data (see: 
   https://www.google.com/maps/d/edit?mid=1pdCcMcTHQzcOOTIz-Lv1uYqqjWI-jDQ)

## [0.64] - 2023-07-22

### Fixed

- Added a BLE advertising parameter that should prevent other devices from 
  connecting to the toolbox while it is advertising a beacon. Previously, 
  if a device did attempt such a connection, the beacon on the toolbox 
  would stop, but you wouldn't know it.

## [0.63] - 2023-07-21

### Added

- Added SERIAL_DEBUG_ENABLE which, if not defined, will prevent messages from 
  being sent over the serial monitor

### Fixed

- Used the newly created droid beacon feature to identify and a bug with the 
  scanner that caused a crash when encountering an unknown personality chip.

### Changed

- Expert mode can now create droid beacons that will be seen by other 
  toolboxes; you cannot connect to emulated beacons. adding this feature 
  led me to rewrite a lot of the expert beacon display code.

## [0.62] - 2023-07-20

### Added

- Added battery/power voltage display on the splash screen. Displayed 
  value is only updated when splash screen is loaded; this is not a 
  real-time monitor.
  Thanks to Tom Harris for suggesting this feature and providing some code 
  to work with!

## [0.61] - 2023-07-18

### Added

- Added ability to display custom names for droids based off their Bluetooth
  address. Custom names can be added to the named_droids[] array in the code.

### Changed

- Reworked how personality, location, and affiliation data is stored in the 
  code to make modification of that data a little easier.

## [0.60] - 2023-04-16

### Added

- Added ability to select which beacon the droid toolbox will produce.
- Expert beacon mode allows finer control over the beacon; I probably should 
  have hidden it behind a key combination...
- Added global beacon variable to store the details of the beacon that will be 
  produced.
- Lots of work on the underlying menu system and helper functions.
- Added global variable to track currently selected item in a menu, rather 
  than have a STATE for each menu option.

### Changed

- Moving more control of the display options (text color, size, etc) to the 
  block of #defines at the top of the code

## [0.56] - 2023-04-09

### Added

- Added a caption to select menus; this puts back functionality present in 
  earlier versions that I just prefer.
- Created some #defines to control color and font to make customization a 
  little easier.

### Changed

- Short button 2 press on scan results now goes back 1 droid (if droids are 
  found) instead of returning to main menu; long button 2 press will return 
  to main menu.
- Personality chip ID 0x0C now identified as 'D-O'. Thanks to cre8or on swgediscord.com.

## [0.55] - 2023-03-30

### Added

- Added BD personality to know personalities.

### Changed

- Set delay between beacon reactions to 1 minute; BD units CORRECTLY interpret
  the value of 0x02 as a 10 second delay.

## [0.54] - 2023-03-17

### Added

- Added volume control
- Started work on creating a generic menu system

### Changed

- Centered the play track screen.
- Current group and track no longer reset to 1-1 when exiting the play track
  screen.

## [0.53] - 2023-01-24

### Added

- Added support for T-Display-S3 devices.
  T-Display-S3 currently requires a modified version of TFT_eSPI which you can
  get from the T-Display-S3 github repository here: 
  https://github.com/Xinyuan-LilyGO/T-Display-S3 under the lib directory.

## [0.52] - 2023-01-10

### Fixed

- Fixed typo "CH1-10P" => "C1-10P".

## [0.51] - 2022-12-18

### Added

- Put BLE notifications back into the code. Any notifications received are 
  displayed in the serial monitor.
- Added note to connecting string so people see the droid remote needs to be 
  off before connecting.

## [0.50] - 2022-12-15

### Added

- Added ability to connect to droid from scan results using a long-press of 
  button 1.
- Droid plays activation sound upon connection. 
- Group and track can be selected and played through the droid.

## [0.40] - 2022-12-12

### Added

- Added deep sleep/hibernation
- Added initial ability to connect to droid with long button 1 press while 
  viewing droid in scan results
- Connection is currently a demo; connect, tell droid to play a sound, then 
  disconnect. Will improve upon this in the next version.

## [0.30] - 2022-12-11

### Added

- Long/Short button press detection.
- Droid report is paged; shows 1 droid at a time.
- Droid report sorts droids by RSSI value.
- Added version to splash screen.

## [0.20] - 2022-11-16

### Added

- Added back button from both beacon and scanner.
- Location beacon location is randomly selected.

## [0.10] - 2022-11-15

### Added

- Initial code release.

