# Sending_Data_To_Thingsboard

This is a working code which can send data to [thingsboard.io](https://thingsboard.io/) from [Arduino Mega 2560](https://store.arduino.cc/usa/arduino-mega-2560-rev3).

Harware Used:
1.  Arduino Mega 2560
2.  SIM900A / SIM800

Implementation Done:
1.  Connected SIM900A / SIM800 to Serial1.
2.  Read the output obtained from that over Serial.
3.  TCP is implemented and Data is sent over HTTP request to the [thingsboard](https://thingsboard.io/) server
