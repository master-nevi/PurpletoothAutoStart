PurpletoothAutoStart
====================

An Arduino sketch that will automatically begin Bluetooth streaming playback in a fading fashion at startup on the SparkFun Purpletooth Jamboree.

#### Required libraries:
* bc127: https://github.com/sparkfun/BC127_Breakout_Board
* QueueList: http://playground.arduino.cc/Code/QueueList
* SoftwareSerial: Included with Arduino IDE

#### Optional Requirements:
The Purpletooth board sends a lot of AVRCP data over the serial port which provides metadata information about the currently playing track, i.e. song title, artist, etc. Because of this, the default buffer hardcoded in the SoftwareSerial library is usually insufficient and will result in buffer overflows that produce garbled strings. To fix this, bump up the _SS_MAX_RX_BUFF constant in SoftwareSerial.h to something greater than or equal to 256 (make sure your Arduino has enough memory for this). This is especially useful in case you want to display this metadata on a display.

#### Useful Documents:
* [BC127 Module Firmware Manual](http://dlnmh9ip6v2uc.cloudfront.net/datasheets/Wireless/Bluetooth/Melody_5.0_Manual-RevD-RC10-Release.pdf)
* [Understanding the BC127 Bluetooth Module](https://learn.sparkfun.com/tutorials/understanding-the-bc127-bluetooth-module/arduino-library?_ga=1.261420174.265801078.1423708522)
