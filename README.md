![Image](https://github.com/user-attachments/assets/00b93f89-7881-438d-a1f3-57e3f0499749)
![Image](https://github.com/VQey/OseeUdpController/blob/main/Firmware%20and%20uploader/1.png)

Osee Go Stream Controller

You can use Wired system (Arduino Mega + ethernet Shield - Coming Soon!) or by wireless (Esp8266/Nodemcu)

There is 10 buttons from left to right:

PGM 1 - 2 - 3 - 4 - PVW 1 - 2 - 3 - 4 -AUTO - CUT

Neopixel led to monitoring PGM or PVW Position

Instruction:
Upload the 'Esp8266NeopixelButtonOseeControl.ino.nodemcu.bin' using uploader by running the flash_download_tool_3.9.8_w1.exe
use the image as guide setup how to upload:


![Image](https://github.com/VQey/OseeUdpController/blob/main/Firmware%20and%20uploader/2.png)
![Image](https://github.com/VQey/OseeUdpController/blob/main/Firmware%20and%20uploader/3.png)

Press 'start' to start the upload. wait until finish, then press stop.

After upload, search and connect to the ESP Access point "OseeGoControl", with the ssid password "oseegostream".
Then open your smartphone and browse the ip 50.50.50.50 to access the setup webmenu.

![Image](https://github.com/user-attachments/assets/76383bbf-2180-41b6-bb8b-d44ce04797d5)

Enter your WiFi ssid, password, and Switcher ip address (make sure the switcher are connected on the same wifi network's lan port)
save and restart to boot the new configuration, done!
