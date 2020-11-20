#include <Arduino.h>
#include "WiFi.h"
#include "BluetoothSerial.h"
#include "esp_spiffs.h"

// LED_BUILTIN is used to indicate WiFi/BT is running (blink), and to indicate connection (constant)
#define LED_BUILTIN 2       // program running indicator

#define SELECT_PIN 13       // ESP32 GPIO pin for WiFi/Bluetooth selection
#define RUN_WR_PIN 12       // ESP32 GPIO pin for changing auth codes
#define SER2EN_PIN 14       // ESP32 GPIO pin to enable Serial 2 for output
#define NAME "ESP32"        // default - can be changed by user
#define PSWD "pswd"         // default - can be changed by user
#define PIN  "1234"         // default - can be changed by user
#define PORT "2000"         // default - can be changed by user
#define BAUDRATE "115200"   // default - can be changed by user
#define FNAME "/name"       // file that contains user-set network name   (16chars)
#define FPSWD "/pswd"       // file that contains user-set WiFi password  (16chars)
#define FPIN  "/pin"        // file that contains user-set BT pin         (4chars)
#define FPORT "/port"       // file that contains user-set WiFi server port number (5chars)
#define FBAUD "/baudrate"   // file that contains user-set baud rate
#define BUF_SZ 17           // 16 alphanumeric; 0-9, [aA] to [zZ]
#define PIN_SZ 5            // 4 digits 0 to 9999
#define PORT_SZ 6           // 5 digits 0 to 2^16

char netName[BUF_SZ];     // WiFi and BT network name
char pswd[BUF_SZ];         // WiFi password  
char pin[PIN_SZ];         // BT pin
char portVal[PORT_SZ];         // WiFi port
int  baudrate;

WiFiServer server(atoi(portVal));  // default - changed by user if required
BluetoothSerial BTSerial;
int wifi;
int run;
int count;

HardwareSerial CONSOLE = Serial;
HardwareSerial UART = Serial;

int looptimer = 0;                    // blink-time counter

void setup()
{
  bool start = true;
  bool stop  = false;

  pinMode(SELECT_PIN,INPUT_PULLUP);   // WiFi - default
  pinMode(RUN_WR_PIN,INPUT_PULLUP);   // Run WiFi/BT - default
  pinMode(SER2EN_PIN,INPUT_PULLUP);   // Select modem I/O UART
  pinMode(LED_BUILTIN, OUTPUT);

  // load default vals
  strcpy(netName, NAME);
  strcpy(pswd, PSWD);
  strcpy(pin, PIN);
  strcpy(portVal, PORT);
  baudrate = atoi(BAUDRATE);

  // use IO pin to select WiFi or BT: 1 for WiFi; 0 for BT.
  wifi = digitalRead(SELECT_PIN);
  // use IO pin to run prog or write Auth Values: 1 for run; 0 for write.
  run = digitalRead(RUN_WR_PIN);

  // If unit is devkit, USB uses Serial and I/O is to Serial2 (no serial1)
  // If unit is module, no USB - all I/O to Serial
  if (digitalRead(SER2EN_PIN) == 0)   // pull pin low to enable Serial2
  {
    UART = Serial2;
  }

  CONSOLE.begin(baudrate);
  UART.begin(baudrate);

  if (fileSystem(start))
  {
    loadParamVals();
  }

  // loadParamVals may have changed baud rate from default, so update
  CONSOLE.updateBaudRate(baudrate);  // Serial
  UART.updateBaudRate(baudrate);     // Serial (module) or Serial2 (devKit)

  if(run)
  {
    if (wifi)
    {
      count = 0x10000;
      WiFi.softAP(netName, pswd);   // uses default 192.168.4.1 and DHCP
      server.begin(atoi(portVal));  // portVal can be changed
      server.setNoDelay(true);      // do not aggregate small packets
    }
    else
    {
      count = 0x20000;
      BTSerial.begin(netName); //Bluetooth device name, slave mode
      BTSerial.setPin(pin); //The pairing PIN
    }
  }
  else
  {
    sleep(2);
    updateParmValues();
  }
  fileSystem(stop);
}

void loop()
{
  looptimer++;
  if (looptimer >= count)
  {
    looptimer = 0;
    digitalWrite(LED_BUILTIN, !(digitalRead(LED_BUILTIN)));  // indicate program is running
  }

  if (run)
  {
    if (wifi)
    {
      WiFiClient client = server.available();

      if (client)
      {
        while(client.connected())
        {
          digitalWrite(LED_BUILTIN, 1);
          while (client.available()) { UART.write(client.read()); }
          while (UART.available()) { client.write(UART.read()); }
//          while (client.available()) { CONSOLE.write(client.read()); }
//          while (CONSOLE.available()) { client.write(CONSOLE.read()); }
        }
        client.stop();
      }
    }
    else
    {
      while (BTSerial.connected())
      {
        digitalWrite(LED_BUILTIN, 1);
        while (UART.available()) { BTSerial.write(UART.read()); }
        while (BTSerial.available()) { UART.write(BTSerial.read()); }
//        while (CONSOLE.available()) { BTSerial.write(CONSOLE.read()); }
//        while (BTSerial.available()) { CONSOLE.write(BTSerial.read()); }
      }
    }
  }
}
//printf '\033[2J'
void loadParamVals()
{
  // Clear the screen
  if (!run) {CONSOLE.printf("\033[2J");}
  // On read error, default values remain in buffers
  loadString((char *)FNAME, netName, NAME);
  loadString((char *)FPSWD, pswd, PSWD);
  loadString((char *)FPIN,  pin, PIN);
  loadString((char *)FPORT, portVal, PORT);
  baudrate = loadInteger((char *)FBAUD, baudrate, BAUDRATE);
}

void updateParmValues()
{
  char rd;
  showMenu();

  while (1)
  {
    if (!CONSOLE.available())
    {
      // spin wheels
    }
    else
    {
      char buf[BUF_SZ];
      int val1 = 115200;  // baudrate values
      int val2 = 230400;
      int val3 = 460800;
      int val4 = 576000;
      int val5 = 921600;
      int val;
  
      rd = CONSOLE.read();
      CONSOLE.print(rd);  // echo the character
      
      switch (rd)
      {
        case '1':
          if (selectedItem(FNAME, buf, BUF_SZ))
          {
            updateValue(FNAME, buf);
          }
          break;
        case '2':
          if (selectedItem(FPSWD, buf, BUF_SZ))
          {
            updateValue(FPSWD, buf);
          }
          break;
        case '3':
          if (selectedItem(FPIN, buf, PIN_SZ))
          {
            updateValue(FPIN, buf);
          }
          break;
        case '4':
          if (selectedItem(FPORT, buf, PORT_SZ))
          {
            updateValue(FPORT, buf);
          }
          break;
        case '5':
          CONSOLE.printf("\nValues: %d %d %d %d %d\n",val1,val2,val3,val4,val5);
          if (selectedItem(FBAUD, buf, BUF_SZ))
          {
            val = atoi(buf);
            if (
              (val == val1) ||
              (val == val2) ||
              (val == val3) ||
              (val == val4) ||
              (val == val5)
            )
            {
              updateValue(FBAUD, buf);
            }
            else
            {
              CONSOLE.printf("\n### Invalid value \"%d\".\n",val);
              sleep(2);
            }
          }
          break;

        case 'x':
        case 'X':
          CONSOLE.printf("\nOperation complete:- Remove jumper; Reset ESP32\n");
          return;
          break;
        default:
          CONSOLE.printf("\n\nINVALID Selection\n\n");
          break;
      }
      showMenu();
    }
  }
}

void showMenu()
{
  CONSOLE.printf("\n\n");
  loadParamVals();    // display current
  CONSOLE.printf("\nUpdate ESP32 parameters.\n");
  CONSOLE.printf("Select item to change.\n\n");
  CONSOLE.printf("Enter 1 for Network name.\n");
  CONSOLE.printf("Enter 2 for WiFi password.\n");
  CONSOLE.printf("Enter 3 for Bluetooth pin.\n");
  CONSOLE.printf("Enter 4 for WiFi port.\n");
  CONSOLE.printf("Enter 5 for Baud Rate.\n");  
  CONSOLE.printf("Enter x for exit.\n\n");
}

bool selectedItem(const char *name, char *buf, int bufsz)
{
  char rd;
  int i = 0;

  CONSOLE.printf("\nChanging value of %s.\n", name);
  CONSOLE.printf("\"Esc\" to abort; \"Enter\" to save, \"CtrlD\" to restore default value\n");
  while (1)
  {
    if (!CONSOLE.available())
    {
        // spin wheels
    }
    else
    {
      rd = CONSOLE.read();

      switch(rd)
      {
        case 0x04:  // CtrlD
          if (remove(name) != 0)
          {
            CONSOLE.printf("\nUnable to remove file \"%s\".\n", name);
          }
          else
          {
            CONSOLE.printf("Default value restored.\n");
          }
          sleep(2);
          return(false);
          break;

        case 0x1B:  // Esc
          CONSOLE.printf("\nEntry discarded.\n");
          sleep(2);
          return(false);
          break;

        case 0x0D:  // Enter
        case 0x0A:  // Line Feed
          if (i == 0)
          {
            CONSOLE.printf("\nMust enter at least 1 character.\n");
            sleep(2);
            return(false);
          }
          buf[i] = '\0';  // complete the string
          return(true);
          break;

        case 0x08:  // back space
          CONSOLE.printf("\b \b");
          if (i > 0) {--i;}
          break;

        default:
          if (isgraph(rd) > 0)  // printable - exclude space
          {
            if (i >= bufsz)
            {
              CONSOLE.printf("\n### Max %d characters ###\n",bufsz-1);  // (last space for '\0')
              sleep(2);
              return(false);
            }
            CONSOLE.print(rd);  // echo the character
            buf[i++] = rd;
          }
          break;
      }
    }
  }
}

void updateValue(const char *name, char *buf)
{
  FILE* file = fopen(name, "w");
  if (file)
  { 
    fputs(buf, file);
    fputc('\0', file);
    fclose(file);
  }
  else
  {
    log_e("Error opening file - \"%s\" for writing.",name);
  }
}

bool loadString(char *name, char *buf, const char *dflt)
{
  FILE* file = fopen(name, "r");
  if (file)
  {
    fgets(buf, BUF_SZ, file);
    fclose(file);
    if (!run) {CONSOLE.printf("Current value \"%s\" from file \"%s\".\n",buf,name);}
    return(true);
  }
  else
  {
    strlcpy(buf, dflt, BUF_SZ);
    if (!run) {CONSOLE.printf("Default value \"%s\"; (file \"%s\" not found).\n", buf, name);}
    return(false);
  }
}

int loadInteger(char *name, int val, const char *dflt)
{
  int rate = val; 
  char buf[BUF_SZ];
  itoa(rate, buf, 10);
  if (loadString(name, buf, dflt))
  {
    rate = atoi(buf);
  }
  return(rate);
}

bool fileSystem(bool start)
{
  esp_vfs_spiffs_conf_t cfg =
      {
        .base_path = "",
        .partition_label = NULL,
        .max_files = 5,
        .format_if_mount_failed = true
      };

  if (start)
  {
    esp_err_t ret = esp_vfs_spiffs_register(&cfg);

    if (ret != ESP_OK)
    switch (ret)
    {
      case ESP_FAIL:
        log_e("Error mounting or formatting Spiffs\n");
        return (false);
        break;
      case ESP_ERR_NOT_FOUND:
        log_e("Spiffs initialisation error (%s)\n", esp_err_to_name(ret));
        return (false);
        break;
      default:
        log_e("Spiffs mount error\n");
        return (false);
        break;
    }
    return (true);
  }
  else
  {
    esp_vfs_spiffs_unregister(cfg.partition_label);
    return(true);
  }
}

