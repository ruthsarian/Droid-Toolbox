# Droid Toolbox
This is an [Arduino IDE](https://www.arduino.cc/en/software) sketch for ESP32-based devices to scan for nearby [Galaxy's Edge droids](https://disneyworld.disney.go.com/shops/hollywood-studios/droid-depot/) and display a list of those that were found. It can also emit a beacon to which Galaxy's Edge droids will react.

This code was specifically designed for [LILYGO TTGO](http://www.lilygo.cn/prod_view.aspx?Id=1126) devices and may not display correctly on other ESP32 devices without modification.

Use at your own risk.

## Version 0.1
Proof-of-concept. Needs better handling of displaying when more than 2 droids are found. Beacon needs to be customizable.

![Image of a TTGO with the Droid Toolbox boot screen.](images/v0.1_01_start.jpg)
![Image of a TTGO with the menu screen showing SCANNER band BEACON options.](images/v0.1_02_menu.jpg)
![Image of a TTGO with the text 'SCANNING' across the screen.](images/v0.1_03_scanning.jpg)
![Image of a TTGO showing a list of nearby droids including their bluetooth address, their affiliation, and personality chip.](images/v0.1_04_scan_results.jpg)
![Image of a TTGO showing "BEACON ON" on the display.](images/v0.1_05_beacon.jpg)
