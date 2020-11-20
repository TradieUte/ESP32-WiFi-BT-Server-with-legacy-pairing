# ESP32Server
Use ESP32 for WiFi or Classic BT (with pin code pairing) data passthrough servers. 

Folder 'ESP32' contains a Platformio project with the code to run both servers, plus code for a simple editor to change Network Name,
WiFi password, Bluetooth PIN, WiFi Server port, and baudrate. (all in main.ino).
Folder 'framework-arduinoespressif32' holds the two files with the required changes to enable Classic BT pin code pairing. These files
replace the ones in ~/.platformio/packages/framework-arduinoespressif32.
Folder 'bin' contains the 3 binaries created from the compile and may be loaded directly to an ESP32 module compatible with ESP32 devkit V1.

3 GPIO pins on the ESP32 control module configuration:-

GPIO13 - WiFi or BT; default WiFi, pull pin low for BT.

GPIO12 - Pull low to enter parameter edit mode.

GPIO14 - Pull low to divert data I/O to Serial2. Default - all I/O to Serial (USB if devkit)

The editor may be used to change the operating parameters.
Only printable characters (excluding space) are allowed.
Changes are stored in individual files in an onboard spiffs file system; if a file does not exist, the default is used.

Defaults:-

Baudrate, 115200      - editor supplies a list

Network Name, "ESP32" - maximum name size 16 chars. (used by WiFi and BT)

WiFi pswd, "pswd"     - maximum password size 16 chars.

BT Pin, "1234"        - maximum pin size is four chars. (eg "A0z9")

IP address, 192.168.4.1 (currently fixed)

In the ESP32 project, 'sdkconfig.defaults' contain the defines required to build the system. These values override the default values 
that would normally be placed in 'sdkconfig' by menuconfig ('pio run -t menuconfig' NB, build will fail if window size not large enough).
Menuconfig must be run to generate 'sdkconfig', prior to initial build.
The file 'partitions.csv' holds the ESP32 memory partition definitions and is currently setup for 4MB flash.
Note the 'framework', 'board_build.partitions' and 'platform_packages' entries in file platform.ini

The 'flash.sh' (Linux) shell script will flash the binary files to /dev/ttyUSB0.
If using a different port, alter the script. 
A Windows O/S flash tool is available from https://www.espressif.com/en/products/socs/esp32/resources. The parameters in 'flash.sh' may be used with the tool, excepting the port, which must match that on Windows.
