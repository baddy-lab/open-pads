/*
 ________      ________    _______       ________           ________    ________      ________      ________      
|\   __  \    |\   __  \  |\  ___ \     |\   ___  \        |\   __  \  |\   __  \    |\   ___ \    |\   ____\     
\ \  \|\  \   \ \  \|\  \ \ \   __/|    \ \  \\ \  \       \ \  \|\  \ \ \  \|\  \   \ \  \_|\ \   \ \  \___|_    
 \ \  \\\  \   \ \   ____\ \ \  \_|/__   \ \  \\ \  \       \ \   ____\ \ \   __  \   \ \  \ \\ \   \ \_____  \   
  \ \  \\\  \   \ \  \___|  \ \  \_|\ \   \ \  \\ \  \       \ \  \___|  \ \  \ \  \   \ \  \_\\ \   \|____|\  \  
   \ \_______\   \ \__\      \ \_______\   \ \__\\ \__\       \ \__\      \ \__\ \__\   \ \_______\    ____\_\  \ 
    \|_______|    \|__|       \|_______|    \|__| \|__|        \|__|       \|__|\|__|    \|_______|   |\_________\
                                                                                                      \|_________|
                                                                                                                                                                                                                            
    ###########################################################################
    OPEN_PADS_CODE_ESP_V01.ino - Embedded source code of OPEN PADS project
    Copyright (C) 2020  GRESLEBIN Benoit
    ###########################################################################
    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU Affero General Public License as published by
    the Free Software Foundation, either version 3 of the License, or any later version.
    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU Affero General Public License for more details.
    You should have received a copy of the GNU Affero General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
    ######################################
    ADDITIONAL TERMS
    ######################################
    Specific non commercial use term: This source code may be copied, distributed, modified by any individual
    and non profit organizations (Fablab, Hackerspaces, Makerspaces, universities, Schools), as long
    as it follows the GNU GPL license terms here attached.
    Corporate institutions and private companies may only copy and distribute the source code without modification.
    Therefore, they are not allowed to modify the source code prior to distributing it for commercial purpose.
    So for example, if a user embeds or modifies or relies on the source code in a product that is
    then sold to a third party, this would be a violation of the Non-Commercial Use License
    additional term, although this type of use would be permitted under the standard GNU GPL
    (assuming the other terms and conditions of the GNU GPL v.3 were followed). By adding this additional
    restriction, it is intended to encourage the evolution of OPEN PAdS source code to be driven
    by the Hacker and makers' communities.
    ######################################
    COPYRIGHT NOTICE
    ######################################
    Ultrasonic.h is a library by Paul Badger
    Adafruit_NeoPixel.h is a Library by Adafruit
    ArduinoJson.h is a software library, Copyright (c) Benoit Blanchon
    ESPAsyncWebServer.h and ESPAsyncTCP.h are software libraries, Copyright (c) by Hristo Gochkov
    ######################################
    HISTORY
    ######################################
    Mar 2020 - First iteration during lockdown...
*/

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include "FS.h"
#include <ESP8266HTTPClient.h>
#include <ESPAsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ArduinoOTA.h>
#include <TM1637Display.h>
//#include <CapacitiveSensor.h>
#include <Ultrasonic.h>

#include "Adafruit_NeoPixel.h"
#ifdef __AVR__
#include <avr/power.h>
#endif

// ESP Pin configurations

#define LED_PIN D3
#define SENSOR_PIN D4

#define NUM_LEDS 12
#define TIMER_MENU 50
#define LED_REFRESH_TIMER 7
#define LED_MAX_INTENSITY 100
#define LED_LOW_INTENSITY 0
#define SENSOR_SENSITIVENESS 30
#define SENSOR_THRESHOLD 50
#define SONIC_SENSOR_THRESHOLD 25 // this is the distance in centimeter triggering the sensor
#define SONIC_SENSOR_THRESHOLD_LONG_TOUCH 40 // 
#define GAME1_TIMER_THRESHOLD 1500
#define GAME1_TIMER_GAME_OVER_THRESHOLD 3000
#define GAME2_TIMER_THRESHOLD 25000
#define GAME2_TIMER_GAME_OVER_THRESHOLD 40000

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

#define ANALOG_READ_BATTERY A0

// Define the pins for TM1637 Display
#define CLK D5
#define DIO D6

// ESP Pin configurations end

// Led configuration

// CAPACITIVE SENSOR 

// CapacitiveSensor capSensor1 = CapacitiveSensor(D0, D1); // Not using capacitive sensor anymore

// CAPACITIVE SENSOR END

// ULTRASONIC SENSOR PIN DEFINITIONS

Ultrasonic sensor_sonic(D0,D1);

// ULTRASONIC SENSOR PIN DEFINITIONS END


// TM1637 display instance:
TM1637Display display(CLK, DIO);

const uint8_t SEG_EMPTY [] = { 
    0,
    0,
    0,
    0
};

const uint8_t SEG_LINE [] = { 
    SEG_G,
    SEG_G,
    SEG_G,
    SEG_G
};

const uint8_t SEG_MENU [] = { 
    SEG_A | SEG_B | SEG_C | SEG_E | SEG_F,
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,
    SEG_A | SEG_B | SEG_C | SEG_E | SEG_F,
    SEG_B | SEG_C | SEG_D | SEG_E | SEG_F
};

const uint8_t SEG_GAME [] = { 
    SEG_A | SEG_C | SEG_D | SEG_E | SEG_F | SEG_G,
    SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,
    SEG_A | SEG_B | SEG_C | SEG_E | SEG_F,
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G
};

const uint8_t SEG_OVER [] = { 
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,
    SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,
    SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G
};

const uint8_t SEG_PLAY [] = { 
    SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,
    SEG_D | SEG_E | SEG_F,
    SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,
    SEG_B | SEG_C | SEG_F | SEG_G
};

const uint8_t SEG_Set [] = { 
    SEG_A | SEG_C | SEG_D | SEG_F | SEG_G,
    SEG_A | SEG_B | SEG_D | SEG_E | SEG_F | SEG_G,
    SEG_D | SEG_E | SEG_F | SEG_G,
    SEG_B | SEG_C
};

const uint8_t SEG_PEEr [] = { 
    SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,
    SEG_E | SEG_G
};

const uint8_t SEG_Init [] = { 
    SEG_B | SEG_C,
    SEG_A | SEG_B | SEG_C | SEG_E | SEG_F,
    SEG_B | SEG_C,
    SEG_D | SEG_E | SEG_F | SEG_G
};

const uint8_t SEG_LOAD [] = { 
    SEG_D | SEG_E | SEG_F,
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,
    SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,
    SEG_B | SEG_C | SEG_D | SEG_E| SEG_G
};

const uint8_t SEG_PADS [] = { 
    SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,
    SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,
    SEG_B | SEG_C | SEG_D | SEG_E | SEG_G,
    SEG_A | SEG_C | SEG_D | SEG_F | SEG_G
};

const uint8_t SEG_OPEN [] = { 
    SEG_A | SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,
    SEG_A | SEG_B | SEG_E | SEG_F | SEG_G,
    SEG_A | SEG_D | SEG_E | SEG_F | SEG_G,
    SEG_A | SEG_B | SEG_C | SEG_E | SEG_F
};

const uint8_t SEG_TEST [] = { 
    SEG_B | SEG_C | SEG_D | SEG_E | SEG_F,
    SEG_A | SEG_B | SEG_C | SEG_E | SEG_F | SEG_G,
    SEG_B | SEG_C,
    SEG_D | SEG_E | SEG_F | SEG_G
};

const uint8_t SEG_HAIR_SHORT [] = { 
    SEG_C | SEG_E ,
    SEG_C | SEG_E ,
    SEG_C | SEG_E ,
    SEG_C | SEG_E 
};

const uint8_t SEG_HAIR [] = { 
    SEG_B | SEG_C | SEG_E | SEG_F ,
    SEG_B | SEG_C | SEG_E | SEG_F ,
    SEG_B | SEG_C | SEG_E | SEG_F ,
    SEG_B | SEG_C | SEG_E | SEG_F 
};

// TM1637 end

String firmware_version = "1.0.0";

//Variables for movement detection:

bool master_padlight;
bool flag_light_touched = 0; // used to distinguish 
bool reset_flag=0;
bool i_am_the_one_to_catch;
bool flag_select_light = 0;
bool flag_broadcast_delta_time = 0;
bool update_delta_time_display = 0;
bool broadcast_register_flag = 0;
bool flag_broadcast_selected_game = 0;
bool flag_broadcast_count = 0;
bool flag_update_count = 0;

bool flag_game_menu = 1;
bool flag_game_started = 0;
bool idle_flag=0;
bool flag_game_over=0;
int game_1_timer_between_lights = 0;
int flag_game = 0;
int sensor_read =0; // for debug purpose

int random_wait_time_game2 = 0;
bool flag_light_up = 0;
int count_game_2 = 0;

int light_colour;
int count_game_3_colour1;
int count_game_3_colour2;
String concatenated_count;

unsigned long previousMillis = 0;
unsigned long currentMillis = 0;
unsigned long deltaMillis = 0;
unsigned long start_timer = 0;
String delta_time;

int randNumber;
int prevRand = 0;
int just_an_int = 0;
int another_int = 0;
int read = 0;
int count_game_1 = 0;

// End of Variables for movement detection

String connect_mode = "";
bool flag_new_sequence = false;
bool station_connected = false;
int buddylight_around = 0; // inits the number of buddy lights present in the network

//Wifi related static values for PAD as a server
const char *PADS_ID = "OPENPADS";
String ssid;
const char *password_init = "";
String password;

WiFiUDP udp;
String my_ip_address;
String buddylight_ip_address;
String next_light_ip_address;
String broadcastaddressregister[10]; // register that lists all IP addresses but the address of the actual light

// Those are the addresses when PAD in AP/Wifi access point mode (happens when PAD can't connect on Hot spot/Client mode)
IPAddress pads_ip(192,168,1,2);
IPAddress pads_gateway(192,168,1,1);
IPAddress pads_subnet(255,255,255,0);

HTTPClient buddylight;
WiFiClient client_buddylight;

// Wifi event handler functions

WiFiEventHandler stationConnectedHandler;
WiFiEventHandler stationDisconnectedHandler;


// Wifi event handler functions end

// Wifi related static values end

// Create an instance of the server
AsyncWebServer server(80);

/////////////////////////////JSON DOCUMENT FORMAT//////////////////////////////////

String json_config; // Needed to expose variable to Rest API
String json_status;
String json_playmode_dump;

int battery_level;     // Battery level management


///////////////////////////////////////////////////////////////////////////////////
/////////////////////////////CONFIGURATION PARAMETERS - FACTORY SETTINGS //////////////////////////////
///////////////////////////////////////////////////////////////////////////////////

bool format_file_system_spifs() {
    // We format the file system
    SPIFFS.format();
    return 1;
}

///////////////////////////////////////////////////////////////////////////////////
/////////////////////////////CONFIGURATION PARAMETERS END /////////////////////////
///////////////////////////////////////////////////////////////////////////////////



///////////////////////////////////////////////////////////////////////////////
//////////////////// OTHER FUNCTIONS DECLARATION///////////////////////////////
///////////////////////////////////////////////////////////////////////////////


///////////////////////////////////////////////////////////////////////////////////////////
//////////////////////     Funtions related to LED     ////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////

void anime_ring_low_intensity(int i)
{
    if (i==8)
    {
        display.setSegments(SEG_OPEN);
        pixels.setPixelColor(0, pixels.Color(50,50,50));
        pixels.setPixelColor(1, pixels.Color(0,0,0));
        pixels.setPixelColor(2, pixels.Color(0,0,0));
        pixels.setPixelColor(3, pixels.Color(0,0,0));
        pixels.setPixelColor(4, pixels.Color(0,0,0));
        pixels.setPixelColor(5, pixels.Color(0,0,0));
        pixels.setPixelColor(6, pixels.Color(0,0,0));
        pixels.setPixelColor(7, pixels.Color(0,0,0));
        pixels.setPixelColor(8, pixels.Color(0,0,0));
        pixels.setPixelColor(9, pixels.Color(0,0,0));
        pixels.setPixelColor(10, pixels.Color(0,0,0));
        pixels.setPixelColor(11, pixels.Color(0,0,0));
        pixels.show();
    }
    if (i==16)
    {
        pixels.setPixelColor(0, pixels.Color(50,50,50));
        pixels.setPixelColor(1, pixels.Color(50,50,50));
        pixels.setPixelColor(2, pixels.Color(0,0,0));
        pixels.setPixelColor(3, pixels.Color(0,0,0));
        pixels.setPixelColor(4, pixels.Color(0,0,0));
        pixels.setPixelColor(5, pixels.Color(0,0,0));
        pixels.setPixelColor(6, pixels.Color(0,0,0));
        pixels.setPixelColor(7, pixels.Color(0,0,0));
        pixels.setPixelColor(8, pixels.Color(0,0,0));
        pixels.setPixelColor(9, pixels.Color(0,0,0));
        pixels.setPixelColor(10, pixels.Color(0,0,0));
        pixels.setPixelColor(11, pixels.Color(0,0,0));
        pixels.show();
    }
    if (i==24)
    {
        pixels.setPixelColor(0, pixels.Color(50,50,50));
        pixels.setPixelColor(1, pixels.Color(50,50,50));
        pixels.setPixelColor(2, pixels.Color(50,50,50));
        pixels.setPixelColor(3, pixels.Color(0,0,0));
        pixels.setPixelColor(4, pixels.Color(0,0,0));
        pixels.setPixelColor(5, pixels.Color(0,0,0));
        pixels.setPixelColor(6, pixels.Color(0,0,0));
        pixels.setPixelColor(7, pixels.Color(0,0,0));
        pixels.setPixelColor(8, pixels.Color(0,0,0));
        pixels.setPixelColor(9, pixels.Color(0,0,0));
        pixels.setPixelColor(10, pixels.Color(0,0,0));
        pixels.setPixelColor(11, pixels.Color(0,0,0));
        pixels.show();
    }
    if (i==32)
    {
        pixels.setPixelColor(0, pixels.Color(50,50,50));
        pixels.setPixelColor(1, pixels.Color(50,50,50));
        pixels.setPixelColor(2, pixels.Color(50,50,50));
        pixels.setPixelColor(3, pixels.Color(50,50,50));
        pixels.setPixelColor(4, pixels.Color(0,0,0));
        pixels.setPixelColor(5, pixels.Color(0,0,0));
        pixels.setPixelColor(6, pixels.Color(0,0,0));
        pixels.setPixelColor(7, pixels.Color(0,0,0));
        pixels.setPixelColor(8, pixels.Color(0,0,0));
        pixels.setPixelColor(9, pixels.Color(0,0,0));
        pixels.setPixelColor(10, pixels.Color(0,0,0));
        pixels.setPixelColor(11, pixels.Color(0,0,0));
        pixels.show();
    }
    if (i==40)
    {
        pixels.setPixelColor(0, pixels.Color(50,50,50));
        pixels.setPixelColor(1, pixels.Color(50,50,50));
        pixels.setPixelColor(2, pixels.Color(50,50,50));
        pixels.setPixelColor(3, pixels.Color(50,50,50));
        pixels.setPixelColor(4, pixels.Color(50,50,50));
        pixels.setPixelColor(5, pixels.Color(0,0,0));
        pixels.setPixelColor(6, pixels.Color(0,0,0));
        pixels.setPixelColor(7, pixels.Color(0,0,0));
        pixels.setPixelColor(8, pixels.Color(0,0,0));
        pixels.setPixelColor(9, pixels.Color(0,0,0));
        pixels.setPixelColor(10, pixels.Color(0,0,0));
        pixels.setPixelColor(11, pixels.Color(0,0,0));
        pixels.show();
    }

    if (i==48)
    {
        display.setSegments(SEG_PADS);
        pixels.setPixelColor(0, pixels.Color(50,50,50));
        pixels.setPixelColor(1, pixels.Color(50,50,50));
        pixels.setPixelColor(2, pixels.Color(50,50,50));
        pixels.setPixelColor(3, pixels.Color(50,50,50));
        pixels.setPixelColor(4, pixels.Color(50,50,50));
        pixels.setPixelColor(5, pixels.Color(50,50,50));
        pixels.setPixelColor(6, pixels.Color(0,0,0));
        pixels.setPixelColor(7, pixels.Color(0,0,0));
        pixels.setPixelColor(8, pixels.Color(0,0,0));
        pixels.setPixelColor(9, pixels.Color(0,0,0));
        pixels.setPixelColor(10, pixels.Color(0,0,0));
        pixels.setPixelColor(11, pixels.Color(0,0,0));
        pixels.show();
    }
    if (i==56)
    {
        pixels.setPixelColor(0, pixels.Color(50,50,50));
        pixels.setPixelColor(1, pixels.Color(50,50,50));
        pixels.setPixelColor(2, pixels.Color(50,50,50));
        pixels.setPixelColor(3, pixels.Color(50,50,50));
        pixels.setPixelColor(4, pixels.Color(50,50,50));
        pixels.setPixelColor(5, pixels.Color(50,50,50));
        pixels.setPixelColor(6, pixels.Color(50,50,50));
        pixels.setPixelColor(7, pixels.Color(0,0,0));
        pixels.setPixelColor(8, pixels.Color(0,0,0));
        pixels.setPixelColor(9, pixels.Color(0,0,0));
        pixels.setPixelColor(10, pixels.Color(0,0,0));
        pixels.setPixelColor(11, pixels.Color(0,0,0));
        pixels.show();
    }
    if (i==64)
    {
        pixels.setPixelColor(0, pixels.Color(50,50,50));
        pixels.setPixelColor(1, pixels.Color(50,50,50));
        pixels.setPixelColor(2, pixels.Color(50,50,50));
        pixels.setPixelColor(3, pixels.Color(50,50,50));
        pixels.setPixelColor(4, pixels.Color(50,50,50));
        pixels.setPixelColor(5, pixels.Color(50,50,50));
        pixels.setPixelColor(6, pixels.Color(50,50,50));
        pixels.setPixelColor(7, pixels.Color(50,50,50));
        pixels.setPixelColor(8, pixels.Color(0,0,0));
        pixels.setPixelColor(9, pixels.Color(0,0,0));
        pixels.setPixelColor(10, pixels.Color(0,0,0));
        pixels.setPixelColor(11, pixels.Color(0,0,0));
        pixels.show();
    }
    if (i==72)
    {
        pixels.setPixelColor(0, pixels.Color(50,50,50));
        pixels.setPixelColor(1, pixels.Color(50,50,50));
        pixels.setPixelColor(2, pixels.Color(50,50,50));
        pixels.setPixelColor(3, pixels.Color(50,50,50));
        pixels.setPixelColor(4, pixels.Color(50,50,50));
        pixels.setPixelColor(5, pixels.Color(50,50,50));
        pixels.setPixelColor(6, pixels.Color(50,50,50));
        pixels.setPixelColor(7, pixels.Color(50,50,50));
        pixels.setPixelColor(8, pixels.Color(50,50,50));
        pixels.setPixelColor(9, pixels.Color(0,0,0));
        pixels.setPixelColor(10, pixels.Color(0,0,0));
        pixels.setPixelColor(11, pixels.Color(0,0,0));
        pixels.show();
    }
    if (i==80)
    {
        pixels.setPixelColor(0, pixels.Color(50,50,50));
        pixels.setPixelColor(1, pixels.Color(50,50,50));
        pixels.setPixelColor(2, pixels.Color(50,50,50));
        pixels.setPixelColor(3, pixels.Color(50,50,50));
        pixels.setPixelColor(4, pixels.Color(50,50,50));
        pixels.setPixelColor(5, pixels.Color(50,50,50));
        pixels.setPixelColor(6, pixels.Color(50,50,50));
        pixels.setPixelColor(7, pixels.Color(50,50,50));
        pixels.setPixelColor(8, pixels.Color(50,50,50));
        pixels.setPixelColor(9, pixels.Color(50,50,50));
        pixels.setPixelColor(10, pixels.Color(0,0,0));
        pixels.setPixelColor(11, pixels.Color(0,0,0));
        pixels.show();
    }
    if (i==88)
    {
        pixels.setPixelColor(0, pixels.Color(50,50,50));
        pixels.setPixelColor(1, pixels.Color(50,50,50));
        pixels.setPixelColor(2, pixels.Color(50,50,50));
        pixels.setPixelColor(3, pixels.Color(50,50,50));
        pixels.setPixelColor(4, pixels.Color(50,50,50));
        pixels.setPixelColor(5, pixels.Color(50,50,50));
        pixels.setPixelColor(6, pixels.Color(50,50,50));
        pixels.setPixelColor(7, pixels.Color(50,50,50));
        pixels.setPixelColor(8, pixels.Color(50,50,50));
        pixels.setPixelColor(9, pixels.Color(50,50,50));
        pixels.setPixelColor(10, pixels.Color(50,50,50));
        pixels.setPixelColor(11, pixels.Color(0,0,0));
        pixels.show();
    }
    if (i==96)
    {
        pixels.setPixelColor(0, pixels.Color(50,50,50));
        pixels.setPixelColor(1, pixels.Color(50,50,50));
        pixels.setPixelColor(2, pixels.Color(50,50,50));
        pixels.setPixelColor(3, pixels.Color(50,50,50));
        pixels.setPixelColor(4, pixels.Color(50,50,50));
        pixels.setPixelColor(5, pixels.Color(50,50,50));
        pixels.setPixelColor(6, pixels.Color(50,50,50));
        pixels.setPixelColor(7, pixels.Color(50,50,50));
        pixels.setPixelColor(8, pixels.Color(50,50,50));
        pixels.setPixelColor(9, pixels.Color(50,50,50));
        pixels.setPixelColor(10, pixels.Color(50,50,50));
        pixels.setPixelColor(11, pixels.Color(50,50,50));
        pixels.show();
    }
}


void anime_ring(int i)
{
    if (i==8)
    {
        pixels.setPixelColor(0, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(1, pixels.Color(0,0,0));
        pixels.setPixelColor(2, pixels.Color(0,0,0));
        pixels.setPixelColor(3, pixels.Color(0,0,0));
        pixels.setPixelColor(4, pixels.Color(0,0,0));
        pixels.setPixelColor(5, pixels.Color(0,0,0));
        pixels.setPixelColor(6, pixels.Color(0,0,0));
        pixels.setPixelColor(7, pixels.Color(0,0,0));
        pixels.setPixelColor(8, pixels.Color(0,0,0));
        pixels.setPixelColor(9, pixels.Color(0,0,0));
        pixels.setPixelColor(10, pixels.Color(0,0,0));
        pixels.setPixelColor(11, pixels.Color(0,0,0));
        pixels.show();
    }
    if (i==16)
    {
        pixels.setPixelColor(0, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(1, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(2, pixels.Color(0,0,0));
        pixels.setPixelColor(3, pixels.Color(0,0,0));
        pixels.setPixelColor(4, pixels.Color(0,0,0));
        pixels.setPixelColor(5, pixels.Color(0,0,0));
        pixels.setPixelColor(6, pixels.Color(0,0,0));
        pixels.setPixelColor(7, pixels.Color(0,0,0));
        pixels.setPixelColor(8, pixels.Color(0,0,0));
        pixels.setPixelColor(9, pixels.Color(0,0,0));
        pixels.setPixelColor(10, pixels.Color(0,0,0));
        pixels.setPixelColor(11, pixels.Color(0,0,0));
        pixels.show();
    }
    if (i==24)
    {
        pixels.setPixelColor(0, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(1, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(2, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(3, pixels.Color(0,0,0));
        pixels.setPixelColor(4, pixels.Color(0,0,0));
        pixels.setPixelColor(5, pixels.Color(0,0,0));
        pixels.setPixelColor(6, pixels.Color(0,0,0));
        pixels.setPixelColor(7, pixels.Color(0,0,0));
        pixels.setPixelColor(8, pixels.Color(0,0,0));
        pixels.setPixelColor(9, pixels.Color(0,0,0));
        pixels.setPixelColor(10, pixels.Color(0,0,0));
        pixels.setPixelColor(11, pixels.Color(0,0,0));
        pixels.show();
    }
    if (i==32)
    {
        pixels.setPixelColor(0, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(1, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(2, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(3, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(4, pixels.Color(0,0,0));
        pixels.setPixelColor(5, pixels.Color(0,0,0));
        pixels.setPixelColor(6, pixels.Color(0,0,0));
        pixels.setPixelColor(7, pixels.Color(0,0,0));
        pixels.setPixelColor(8, pixels.Color(0,0,0));
        pixels.setPixelColor(9, pixels.Color(0,0,0));
        pixels.setPixelColor(10, pixels.Color(0,0,0));
        pixels.setPixelColor(11, pixels.Color(0,0,0));
        pixels.show();
    }
    if (i==40)
    {
        pixels.setPixelColor(0, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(1, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(2, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(3, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(4, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(5, pixels.Color(0,0,0));
        pixels.setPixelColor(6, pixels.Color(0,0,0));
        pixels.setPixelColor(7, pixels.Color(0,0,0));
        pixels.setPixelColor(8, pixels.Color(0,0,0));
        pixels.setPixelColor(9, pixels.Color(0,0,0));
        pixels.setPixelColor(10, pixels.Color(0,0,0));
        pixels.setPixelColor(11, pixels.Color(0,0,0));
        pixels.show();
    }
    if (i==48)
    {
        pixels.setPixelColor(0, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(1, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(2, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(3, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(4, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(5, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(6, pixels.Color(0,0,0));
        pixels.setPixelColor(7, pixels.Color(0,0,0));
        pixels.setPixelColor(8, pixels.Color(0,0,0));
        pixels.setPixelColor(9, pixels.Color(0,0,0));
        pixels.setPixelColor(10, pixels.Color(0,0,0));
        pixels.setPixelColor(11, pixels.Color(0,0,0));
        pixels.show();
    }
    if (i==56)
    {
        pixels.setPixelColor(0, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(1, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(2, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(3, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(4, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(5, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(6, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(7, pixels.Color(0,0,0));
        pixels.setPixelColor(8, pixels.Color(0,0,0));
        pixels.setPixelColor(9, pixels.Color(0,0,0));
        pixels.setPixelColor(10, pixels.Color(0,0,0));
        pixels.setPixelColor(11, pixels.Color(0,0,0));
        pixels.show();
    }
    if (i==64)
    {
        pixels.setPixelColor(0, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(1, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(2, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(3, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(4, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(5, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(6, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(7, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(8, pixels.Color(0,0,0));
        pixels.setPixelColor(9, pixels.Color(0,0,0));
        pixels.setPixelColor(10, pixels.Color(0,0,0));
        pixels.setPixelColor(11, pixels.Color(0,0,0));
        pixels.show();
    }
    if (i==72)
    {
        pixels.setPixelColor(0, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(1, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(2, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(3, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(4, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(5, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(6, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(7, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(8, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(9, pixels.Color(0,0,0));
        pixels.setPixelColor(10, pixels.Color(0,0,0));
        pixels.setPixelColor(11, pixels.Color(0,0,0));
        pixels.show();
    }
    if (i==80)
    {
        pixels.setPixelColor(0, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(1, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(2, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(3, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(4, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(5, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(6, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(7, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(8, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(9, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(10, pixels.Color(0,0,0));
        pixels.setPixelColor(11, pixels.Color(0,0,0));
        pixels.show();
    }
    if (i==88)
    {
        pixels.setPixelColor(0, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(1, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(2, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(3, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(4, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(5, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(6, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(7, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(8, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(9, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(10, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(11, pixels.Color(0,0,0));
        pixels.show();
    }
    if (i==96)
    {
        pixels.setPixelColor(0, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(1, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(2, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(3, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(4, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(5, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(6, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(7, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(8, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(9, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(10, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.setPixelColor(11, pixels.Color(LED_MAX_INTENSITY,LED_MAX_INTENSITY,LED_MAX_INTENSITY));
        pixels.show();
    }
}

void smile_white_fade()
{
    for (int i=0; i<LED_MAX_INTENSITY; i++)
    {
        pixels.setPixelColor(0, pixels.Color(i,i,i));
        pixels.setPixelColor(1, pixels.Color(i,i,i));
        pixels.setPixelColor(2, pixels.Color(i,i,i));
        pixels.setPixelColor(3, pixels.Color(i,i,i));
        pixels.setPixelColor(4, pixels.Color(0,0,0));
        pixels.setPixelColor(5, pixels.Color(0,0,0));
        pixels.setPixelColor(6, pixels.Color(0,0,0));
        pixels.setPixelColor(7, pixels.Color(0,0,0));
        pixels.setPixelColor(8, pixels.Color(0,0,0));
        pixels.setPixelColor(9, pixels.Color(i,i,i));
        pixels.setPixelColor(10, pixels.Color(i,i,i));
        pixels.setPixelColor(11, pixels.Color(i,i,i));
        pixels.show();
    }
}

void smile_white()
{

        pixels.setPixelColor(0, pixels.Color(80,80,80));
        pixels.setPixelColor(1, pixels.Color(80,80,80));
        pixels.setPixelColor(2, pixels.Color(80,80,80));
        pixels.setPixelColor(3, pixels.Color(80,80,80));
        pixels.setPixelColor(4, pixels.Color(0,0,0));
        pixels.setPixelColor(5, pixels.Color(0,0,0));
        pixels.setPixelColor(6, pixels.Color(0,0,0));
        pixels.setPixelColor(7, pixels.Color(0,0,0));
        pixels.setPixelColor(8, pixels.Color(0,0,0));
        pixels.setPixelColor(9, pixels.Color(80,80,80));
        pixels.setPixelColor(10, pixels.Color(80,80,80));
        pixels.setPixelColor(11, pixels.Color(80,80,80));
        pixels.show();

}

void smile_red()
{
        pixels.setPixelColor(0, pixels.Color(100,0,0));
        pixels.setPixelColor(1, pixels.Color(100,0,0));
        pixels.setPixelColor(2, pixels.Color(100,0,0));
        pixels.setPixelColor(3, pixels.Color(100,0,0));
        pixels.setPixelColor(4, pixels.Color(0,0,0));
        pixels.setPixelColor(5, pixels.Color(0,0,0));
        pixels.setPixelColor(6, pixels.Color(0,0,0));
        pixels.setPixelColor(7, pixels.Color(0,0,0));
        pixels.setPixelColor(8, pixels.Color(0,0,0));
        pixels.setPixelColor(9, pixels.Color(100,0,0));
        pixels.setPixelColor(10, pixels.Color(100,0,0));
        pixels.setPixelColor(11, pixels.Color(100,0,0));
        pixels.show();

}

void turn_light_red_fade()
{
    Serial.println("Fade Light Red");

    for (int i=0; i<LED_MAX_INTENSITY; i++)
    {
        pixels.setPixelColor(0, pixels.Color(i,0,0));
        pixels.setPixelColor(1, pixels.Color(i,0,0));
        pixels.setPixelColor(2, pixels.Color(i,0,0));
        pixels.setPixelColor(3, pixels.Color(i,0,0));
        pixels.setPixelColor(4, pixels.Color(i,0,0));
        pixels.setPixelColor(5, pixels.Color(i,0,0));
        pixels.setPixelColor(6, pixels.Color(i,0,0));
        pixels.setPixelColor(7, pixels.Color(i,0,0));
        pixels.setPixelColor(8, pixels.Color(i,0,0));
        pixels.setPixelColor(9, pixels.Color(i,0,0));
        pixels.setPixelColor(10, pixels.Color(i,0,0));
        pixels.setPixelColor(11, pixels.Color(i,0,0));
        pixels.show();
        delay(LED_REFRESH_TIMER);
    }

    for (int i=LED_MAX_INTENSITY; i>0; i--)
    {
        pixels.setPixelColor(0, pixels.Color(i,0,0));
        pixels.setPixelColor(1, pixels.Color(i,0,0));
        pixels.setPixelColor(2, pixels.Color(i,0,0));
        pixels.setPixelColor(3, pixels.Color(i,0,0));
        pixels.setPixelColor(4, pixels.Color(i,0,0));
        pixels.setPixelColor(5, pixels.Color(i,0,0));
        pixels.setPixelColor(6, pixels.Color(i,0,0));
        pixels.setPixelColor(7, pixels.Color(i,0,0));
        pixels.setPixelColor(8, pixels.Color(i,0,0));
        pixels.setPixelColor(9, pixels.Color(i,0,0));
        pixels.setPixelColor(10, pixels.Color(i,0,0));
        pixels.setPixelColor(11, pixels.Color(i,0,0));
        pixels.show();
        delay(LED_REFRESH_TIMER);

    }
}

void turn_light_green_fade()
{
    Serial.println("Fade light Green");

    for (int i=0; i<LED_MAX_INTENSITY; i++)
    {
    pixels.setPixelColor(0, pixels.Color(0,i,0));
    pixels.setPixelColor(1, pixels.Color(0,i,0));
    pixels.setPixelColor(2, pixels.Color(0,i,0));
    pixels.setPixelColor(3, pixels.Color(0,i,0));
    pixels.setPixelColor(4, pixels.Color(0,i,0));
    pixels.setPixelColor(5, pixels.Color(0,i,0));
    pixels.setPixelColor(6, pixels.Color(0,i,0));
    pixels.setPixelColor(7, pixels.Color(0,i,0));
    pixels.setPixelColor(8, pixels.Color(0,i,0));
    pixels.setPixelColor(9, pixels.Color(0,i,0));
    pixels.setPixelColor(10, pixels.Color(0,i,0));
    pixels.setPixelColor(11, pixels.Color(0,i,0));
    pixels.show();

    delay(LED_REFRESH_TIMER);
    }
    
    for (int i=LED_MAX_INTENSITY; i>0; i--)
    {
    pixels.setPixelColor(0, pixels.Color(0,i,0));
    pixels.setPixelColor(1, pixels.Color(0,i,0));
    pixels.setPixelColor(2, pixels.Color(0,i,0));
    pixels.setPixelColor(3, pixels.Color(0,i,0));
    pixels.setPixelColor(4, pixels.Color(0,i,0));
    pixels.setPixelColor(5, pixels.Color(0,i,0));
    pixels.setPixelColor(6, pixels.Color(0,i,0));
    pixels.setPixelColor(7, pixels.Color(0,i,0));
    pixels.setPixelColor(8, pixels.Color(0,i,0));
    pixels.setPixelColor(9, pixels.Color(0,i,0));
    pixels.setPixelColor(10, pixels.Color(0,i,0));
    pixels.setPixelColor(11, pixels.Color(0,i,0));
    pixels.show();
    delay(LED_REFRESH_TIMER);
    }
}

void turn_light_blue_fade()
{
    Serial.println("Fade light Blue");

    for (int i=0; i<LED_MAX_INTENSITY; i++)
    {
    pixels.setPixelColor(0, pixels.Color(0,0,i));
    pixels.setPixelColor(1, pixels.Color(0,0,i));
    pixels.setPixelColor(2, pixels.Color(0,0,i));
    pixels.setPixelColor(3, pixels.Color(0,0,i));
    pixels.setPixelColor(4, pixels.Color(0,0,i));
    pixels.setPixelColor(5, pixels.Color(0,0,i));
    pixels.setPixelColor(6, pixels.Color(0,0,i));
    pixels.setPixelColor(7, pixels.Color(0,0,i));
    pixels.setPixelColor(8, pixels.Color(0,0,i));
    pixels.setPixelColor(9, pixels.Color(0,0,i));
    pixels.setPixelColor(10, pixels.Color(0,0,i));
    pixels.setPixelColor(11, pixels.Color(0,0,i));
    pixels.show();
    delay(LED_REFRESH_TIMER);
    }
    for (int i=LED_MAX_INTENSITY; i>0; i--)
    {
    pixels.setPixelColor(0, pixels.Color(0,0,i));
    pixels.setPixelColor(1, pixels.Color(0,0,i));
    pixels.setPixelColor(2, pixels.Color(0,0,i));
    pixels.setPixelColor(3, pixels.Color(0,0,i));
    pixels.setPixelColor(4, pixels.Color(0,0,i));
    pixels.setPixelColor(5, pixels.Color(0,0,i));
    pixels.setPixelColor(6, pixels.Color(0,0,i));
    pixels.setPixelColor(7, pixels.Color(0,0,i));
    pixels.setPixelColor(8, pixels.Color(0,0,i));
    pixels.setPixelColor(9, pixels.Color(0,0,i));
    pixels.setPixelColor(10, pixels.Color(0,0,i));
    pixels.setPixelColor(11, pixels.Color(0,0,i));
    pixels.show();
    delay(LED_REFRESH_TIMER);
    }

}

void turn_light_magenta_fade()
{
    Serial.println("Fade light magenta");

    for (int i=0; i<LED_MAX_INTENSITY; i++)
    {
    pixels.setPixelColor(0, pixels.Color(i,0,i));
    pixels.setPixelColor(1, pixels.Color(i,0,i));
    pixels.setPixelColor(2, pixels.Color(i,0,i));
    pixels.setPixelColor(3, pixels.Color(i,0,i));
    pixels.setPixelColor(4, pixels.Color(i,0,i));
    pixels.setPixelColor(5, pixels.Color(i,0,i));
    pixels.setPixelColor(6, pixels.Color(i,0,i));
    pixels.setPixelColor(7, pixels.Color(i,0,i));
    pixels.setPixelColor(8, pixels.Color(i,0,i));
    pixels.setPixelColor(9, pixels.Color(i,0,i));
    pixels.setPixelColor(10, pixels.Color(i,0,i));
    pixels.setPixelColor(11, pixels.Color(i,0,i));
    pixels.show();
    delay(LED_REFRESH_TIMER);
    }
    for (int i=LED_MAX_INTENSITY; i>0; i--)
    {
    pixels.setPixelColor(0, pixels.Color(i,0,i));
    pixels.setPixelColor(1, pixels.Color(i,0,i));
    pixels.setPixelColor(2, pixels.Color(i,0,i));
    pixels.setPixelColor(3, pixels.Color(i,0,i));
    pixels.setPixelColor(4, pixels.Color(i,0,i));
    pixels.setPixelColor(5, pixels.Color(i,0,i));
    pixels.setPixelColor(6, pixels.Color(i,0,i));
    pixels.setPixelColor(7, pixels.Color(i,0,i));
    pixels.setPixelColor(8, pixels.Color(i,0,i));
    pixels.setPixelColor(9, pixels.Color(i,0,i));
    pixels.setPixelColor(10, pixels.Color(i,0,i));
    pixels.setPixelColor(11, pixels.Color(i,0,i));
    pixels.show();
    delay(LED_REFRESH_TIMER);
    }

}

void turn_light_white_fade()
{
    Serial.println("Fade White");

    for (int i=0; i<LED_MAX_INTENSITY; i++)
    {
        pixels.setPixelColor(0, pixels.Color(i,i,i));
        pixels.setPixelColor(1, pixels.Color(i,i,i));
        pixels.setPixelColor(2, pixels.Color(i,i,i));
        pixels.setPixelColor(3, pixels.Color(i,i,i));
        pixels.setPixelColor(4, pixels.Color(i,i,i));
        pixels.setPixelColor(5, pixels.Color(i,i,i));
        pixels.setPixelColor(6, pixels.Color(i,i,i));
        pixels.setPixelColor(7, pixels.Color(i,i,i));
        pixels.setPixelColor(8, pixels.Color(i,i,i));
        pixels.setPixelColor(9, pixels.Color(i,i,i));
        pixels.setPixelColor(10, pixels.Color(i,i,i));
        pixels.setPixelColor(11, pixels.Color(i,i,i));
        pixels.show();
        delay(LED_REFRESH_TIMER);
    }
    for (int i=LED_MAX_INTENSITY; i>0; i--)
    {
        pixels.setPixelColor(0, pixels.Color(i,i,i));
        pixels.setPixelColor(1, pixels.Color(i,i,i));
        pixels.setPixelColor(2, pixels.Color(i,i,i));
        pixels.setPixelColor(3, pixels.Color(i,i,i));
        pixels.setPixelColor(4, pixels.Color(i,i,i));
        pixels.setPixelColor(5, pixels.Color(i,i,i));
        pixels.setPixelColor(6, pixels.Color(i,i,i));
        pixels.setPixelColor(7, pixels.Color(i,i,i));
        pixels.setPixelColor(8, pixels.Color(i,i,i));
        pixels.setPixelColor(9, pixels.Color(i,i,i));
        pixels.setPixelColor(10, pixels.Color(i,i,i));
        pixels.setPixelColor(11, pixels.Color(i,i,i));
        pixels.show();
        delay(LED_REFRESH_TIMER);
    }

}

void turn_light_off()
{
    //Serial.println("Turning Light off");

    pixels.setPixelColor(0, pixels.Color(0,0,0));
    pixels.setPixelColor(1, pixels.Color(0,0,0));
    pixels.setPixelColor(2, pixels.Color(0,0,0));
    pixels.setPixelColor(3, pixels.Color(0,0,0));
    pixels.setPixelColor(4, pixels.Color(0,0,0));
    pixels.setPixelColor(5, pixels.Color(0,0,0));
    pixels.setPixelColor(6, pixels.Color(0,0,0));
    pixels.setPixelColor(7, pixels.Color(0,0,0));
    pixels.setPixelColor(8, pixels.Color(0,0,0));
    pixels.setPixelColor(9, pixels.Color(0,0,0));
    pixels.setPixelColor(10, pixels.Color(0,0,0));
    pixels.setPixelColor(11, pixels.Color(0,0,0));
    pixels.show(); // This sends the updated pixel color to the hardware.

}

void turn_light_red()
{
    Serial.println("Turning Light Red");

    pixels.setPixelColor(0, pixels.Color(60,0,0));
    pixels.setPixelColor(1, pixels.Color(60,0,0));
    pixels.setPixelColor(2, pixels.Color(60,0,0));
    pixels.setPixelColor(3, pixels.Color(60,0,0));
    pixels.setPixelColor(4, pixels.Color(60,0,0));
    pixels.setPixelColor(5, pixels.Color(60,0,0));
    pixels.setPixelColor(6, pixels.Color(60,0,0));
    pixels.setPixelColor(7, pixels.Color(60,0,0));
    pixels.setPixelColor(8, pixels.Color(60,0,0));
    pixels.setPixelColor(9, pixels.Color(60,0,0));
    pixels.setPixelColor(10, pixels.Color(60,0,0));
    pixels.setPixelColor(11, pixels.Color(60,0,0));
    pixels.show(); // This sends the updated pixel color to the hardware.

}

void turn_light_green()
{
    Serial.println("Turning Light Green");

    pixels.setPixelColor(0, pixels.Color(0,60,0));
    pixels.setPixelColor(1, pixels.Color(0,60,0));
    pixels.setPixelColor(2, pixels.Color(0,60,0));
    pixels.setPixelColor(3, pixels.Color(0,60,0));
    pixels.setPixelColor(4, pixels.Color(0,60,0));
    pixels.setPixelColor(5, pixels.Color(0,60,0));
    pixels.setPixelColor(6, pixels.Color(0,60,0));
    pixels.setPixelColor(7, pixels.Color(0,60,0));
    pixels.setPixelColor(8, pixels.Color(0,60,0));
    pixels.setPixelColor(9, pixels.Color(0,60,0));
    pixels.setPixelColor(10, pixels.Color(0,60,0));
    pixels.setPixelColor(11, pixels.Color(0,60,0));
    pixels.show(); // This sends the updated pixel color to the hardware.

}

void turn_light_blue()
{
    Serial.println("Turning Light Blue");

    pixels.setPixelColor(0, pixels.Color(0,0,60));
    pixels.setPixelColor(1, pixels.Color(0,0,60));
    pixels.setPixelColor(2, pixels.Color(0,0,60));
    pixels.setPixelColor(3, pixels.Color(0,0,60));
    pixels.setPixelColor(4, pixels.Color(0,0,60));
    pixels.setPixelColor(5, pixels.Color(0,0,60));
    pixels.setPixelColor(6, pixels.Color(0,0,60));
    pixels.setPixelColor(7, pixels.Color(0,0,60));
    pixels.setPixelColor(8, pixels.Color(0,0,60));
    pixels.setPixelColor(9, pixels.Color(0,0,60));
    pixels.setPixelColor(10, pixels.Color(0,0,60));
    pixels.setPixelColor(11, pixels.Color(0,0,60));
    pixels.show(); // This sends the updated pixel color to the hardware.

}

void blink_red()
{
    Serial.println("Turning Light Red");

    pixels.setPixelColor(0, pixels.Color(LED_MAX_INTENSITY,0,0));
    pixels.setPixelColor(1, pixels.Color(LED_MAX_INTENSITY,0,0));
    pixels.setPixelColor(2, pixels.Color(LED_MAX_INTENSITY,0,0));
    pixels.setPixelColor(3, pixels.Color(LED_MAX_INTENSITY,0,0));
    pixels.setPixelColor(4, pixels.Color(LED_MAX_INTENSITY,0,0));
    pixels.setPixelColor(5, pixels.Color(LED_MAX_INTENSITY,0,0));
    pixels.setPixelColor(6, pixels.Color(LED_MAX_INTENSITY,0,0));
    pixels.setPixelColor(7, pixels.Color(LED_MAX_INTENSITY,0,0));
    pixels.setPixelColor(8, pixels.Color(LED_MAX_INTENSITY,0,0));
    pixels.setPixelColor(9, pixels.Color(LED_MAX_INTENSITY,0,0));
    pixels.setPixelColor(10, pixels.Color(LED_MAX_INTENSITY,0,0));
    pixels.setPixelColor(11, pixels.Color(LED_MAX_INTENSITY,0,0));
    pixels.show(); // This sends the updated pixel color to the hardware.

    delay(100);

    pixels.setPixelColor(0, pixels.Color(0,0,0));
    pixels.setPixelColor(1, pixels.Color(0,0,0));
    pixels.setPixelColor(2, pixels.Color(0,0,0));
    pixels.setPixelColor(3, pixels.Color(0,0,0));
    pixels.setPixelColor(4, pixels.Color(0,0,0));
    pixels.setPixelColor(5, pixels.Color(0,0,0));
    pixels.setPixelColor(6, pixels.Color(0,0,0));
    pixels.setPixelColor(7, pixels.Color(0,0,0));
    pixels.setPixelColor(8, pixels.Color(0,0,0));
    pixels.setPixelColor(9, pixels.Color(0,0,0));
    pixels.setPixelColor(10, pixels.Color(0,0,0));
    pixels.setPixelColor(11, pixels.Color(0,0,0));
    pixels.show(); // This sends the updated pixel color to the hardware.

}    

void blink_short_white()
{
        Serial.println("Blinking leds...");

    pixels.setPixelColor(0, pixels.Color(50,50,50));
    pixels.setPixelColor(1, pixels.Color(50,50,50));
    pixels.setPixelColor(2, pixels.Color(50,50,50));
    pixels.setPixelColor(3, pixels.Color(50,50,50));
    pixels.setPixelColor(4, pixels.Color(50,50,50));
    pixels.setPixelColor(5, pixels.Color(50,50,50));
    pixels.setPixelColor(6, pixels.Color(50,50,50));
    pixels.setPixelColor(7, pixels.Color(50,50,50));
    pixels.setPixelColor(8, pixels.Color(50,50,50));
    pixels.setPixelColor(9, pixels.Color(50,50,50));
    pixels.setPixelColor(10, pixels.Color(50,50,50));
    pixels.setPixelColor(11, pixels.Color(50,50,50));
    pixels.show(); // This sends the updated pixel color to the hardware.
    delay(50);
    pixels.setPixelColor(0, pixels.Color(0,0,0));
    pixels.setPixelColor(1, pixels.Color(0,0,0));
    pixels.setPixelColor(2, pixels.Color(0,0,0));
    pixels.setPixelColor(3, pixels.Color(0,0,0));
    pixels.setPixelColor(4, pixels.Color(0,0,0));
    pixels.setPixelColor(5, pixels.Color(0,0,0));
    pixels.setPixelColor(6, pixels.Color(0,0,0));
    pixels.setPixelColor(7, pixels.Color(0,0,0));
    pixels.setPixelColor(8, pixels.Color(0,0,0));
    pixels.setPixelColor(9, pixels.Color(0,0,0));
    pixels.setPixelColor(10, pixels.Color(0,0,0));
    pixels.setPixelColor(11, pixels.Color(0,0,0));
    pixels.show(); // This sends the updated pixel color to the hardware.
    display.clear();
    delay(50);
    pixels.setPixelColor(0, pixels.Color(50,50,50));
    pixels.setPixelColor(1, pixels.Color(50,50,50));
    pixels.setPixelColor(2, pixels.Color(50,50,50));
    pixels.setPixelColor(3, pixels.Color(50,50,50));
    pixels.setPixelColor(4, pixels.Color(50,50,50));
    pixels.setPixelColor(5, pixels.Color(50,50,50));
    pixels.setPixelColor(6, pixels.Color(50,50,50));
    pixels.setPixelColor(7, pixels.Color(50,50,50));
    pixels.setPixelColor(8, pixels.Color(50,50,50));
    pixels.setPixelColor(9, pixels.Color(50,50,50));
    pixels.setPixelColor(10, pixels.Color(50,50,50));
    pixels.setPixelColor(11, pixels.Color(50,50,50));
    pixels.show(); // This sends the updated pixel color to the hardware.
    display.showNumberDec(deltaMillis,true);
    delay(50);
    pixels.setPixelColor(0, pixels.Color(0,0,0));
    pixels.setPixelColor(1, pixels.Color(0,0,0));
    pixels.setPixelColor(2, pixels.Color(0,0,0));
    pixels.setPixelColor(3, pixels.Color(0,0,0));
    pixels.setPixelColor(4, pixels.Color(0,0,0));
    pixels.setPixelColor(5, pixels.Color(0,0,0));
    pixels.setPixelColor(6, pixels.Color(0,0,0));
    pixels.setPixelColor(7, pixels.Color(0,0,0));
    pixels.setPixelColor(8, pixels.Color(0,0,0));
    pixels.setPixelColor(9, pixels.Color(0,0,0));
    pixels.setPixelColor(10, pixels.Color(0,0,0));
    pixels.setPixelColor(11, pixels.Color(0,0,0));
    pixels.show(); // This sends the updated pixel color to the hardware.
    display.clear();
    delay(50);

}

void blink()
{
        Serial.println("Blinking leds...");

    pixels.setPixelColor(0, pixels.Color(50,50,50));
    pixels.setPixelColor(1, pixels.Color(50,50,50));
    pixels.setPixelColor(2, pixels.Color(50,50,50));
    pixels.setPixelColor(3, pixels.Color(50,50,50));
    pixels.setPixelColor(4, pixels.Color(50,50,50));
    pixels.setPixelColor(5, pixels.Color(50,50,50));
    pixels.setPixelColor(6, pixels.Color(50,50,50));
    pixels.setPixelColor(7, pixels.Color(50,50,50));
    pixels.setPixelColor(8, pixels.Color(50,50,50));
    pixels.setPixelColor(9, pixels.Color(50,50,50));
    pixels.setPixelColor(10, pixels.Color(50,50,50));
    pixels.setPixelColor(11, pixels.Color(50,50,50));
    pixels.show(); // This sends the updated pixel color to the hardware.
    delay(100);
    pixels.setPixelColor(0, pixels.Color(0,0,0));
    pixels.setPixelColor(1, pixels.Color(0,0,0));
    pixels.setPixelColor(2, pixels.Color(0,0,0));
    pixels.setPixelColor(3, pixels.Color(0,0,0));
    pixels.setPixelColor(4, pixels.Color(0,0,0));
    pixels.setPixelColor(5, pixels.Color(0,0,0));
    pixels.setPixelColor(6, pixels.Color(0,0,0));
    pixels.setPixelColor(7, pixels.Color(0,0,0));
    pixels.setPixelColor(8, pixels.Color(0,0,0));
    pixels.setPixelColor(9, pixels.Color(0,0,0));
    pixels.setPixelColor(10, pixels.Color(0,0,0));
    pixels.setPixelColor(11, pixels.Color(0,0,0));
    pixels.show(); // This sends the updated pixel color to the hardware.
    delay(100);
        pixels.setPixelColor(0, pixels.Color(50,50,50));
    pixels.setPixelColor(1, pixels.Color(50,50,50));
    pixels.setPixelColor(2, pixels.Color(50,50,50));
    pixels.setPixelColor(3, pixels.Color(50,50,50));
    pixels.setPixelColor(4, pixels.Color(50,50,50));
    pixels.setPixelColor(5, pixels.Color(50,50,50));
    pixels.setPixelColor(6, pixels.Color(50,50,50));
    pixels.setPixelColor(7, pixels.Color(50,50,50));
    pixels.setPixelColor(8, pixels.Color(50,50,50));
    pixels.setPixelColor(9, pixels.Color(50,50,50));
    pixels.setPixelColor(10, pixels.Color(50,50,50));
    pixels.setPixelColor(11, pixels.Color(50,50,50));
    pixels.show(); // This sends the updated pixel color to the hardware.

    pixels.setPixelColor(0, pixels.Color(0,0,0));
    pixels.setPixelColor(1, pixels.Color(0,0,0));
    pixels.setPixelColor(2, pixels.Color(0,0,0));
    pixels.setPixelColor(3, pixels.Color(0,0,0));
    pixels.setPixelColor(4, pixels.Color(0,0,0));
    pixels.setPixelColor(5, pixels.Color(0,0,0));
    pixels.setPixelColor(6, pixels.Color(0,0,0));
    pixels.setPixelColor(7, pixels.Color(0,0,0));
    pixels.setPixelColor(8, pixels.Color(0,0,0));
    pixels.setPixelColor(9, pixels.Color(0,0,0));
    pixels.setPixelColor(10, pixels.Color(0,0,0));
    pixels.setPixelColor(11, pixels.Color(0,0,0));
    pixels.show(); // This sends the updated pixel color to the hardware.
    delay(100);

        pixels.setPixelColor(0, pixels.Color(50,50,50));
    pixels.setPixelColor(1, pixels.Color(50,50,50));
    pixels.setPixelColor(2, pixels.Color(50,50,50));
    pixels.setPixelColor(3, pixels.Color(50,50,50));
    pixels.setPixelColor(4, pixels.Color(50,50,50));
    pixels.setPixelColor(5, pixels.Color(50,50,50));
    pixels.setPixelColor(6, pixels.Color(50,50,50));
    pixels.setPixelColor(7, pixels.Color(50,50,50));
    pixels.setPixelColor(8, pixels.Color(50,50,50));
    pixels.setPixelColor(9, pixels.Color(50,50,50));
    pixels.setPixelColor(10, pixels.Color(50,50,50));
    pixels.setPixelColor(11, pixels.Color(50,50,50));
    pixels.show(); // This sends the updated pixel color to the hardware.
    delay(100);
    pixels.setPixelColor(0, pixels.Color(0,0,0));
    pixels.setPixelColor(1, pixels.Color(0,0,0));
    pixels.setPixelColor(2, pixels.Color(0,0,0));
    pixels.setPixelColor(3, pixels.Color(0,0,0));
    pixels.setPixelColor(4, pixels.Color(0,0,0));
    pixels.setPixelColor(5, pixels.Color(0,0,0));
    pixels.setPixelColor(6, pixels.Color(0,0,0));
    pixels.setPixelColor(7, pixels.Color(0,0,0));
    pixels.setPixelColor(8, pixels.Color(0,0,0));
    pixels.setPixelColor(9, pixels.Color(0,0,0));
    pixels.setPixelColor(10, pixels.Color(0,0,0));
    pixels.setPixelColor(11, pixels.Color(0,0,0));
    pixels.show(); // This sends the updated pixel color to the hardware.
    delay(100);
        pixels.setPixelColor(0, pixels.Color(50,50,50));
    pixels.setPixelColor(1, pixels.Color(50,50,50));
    pixels.setPixelColor(2, pixels.Color(50,50,50));
    pixels.setPixelColor(3, pixels.Color(50,50,50));
    pixels.setPixelColor(4, pixels.Color(50,50,50));
    pixels.setPixelColor(5, pixels.Color(50,50,50));
    pixels.setPixelColor(6, pixels.Color(50,50,50));
    pixels.setPixelColor(7, pixels.Color(50,50,50));
    pixels.setPixelColor(8, pixels.Color(50,50,50));
    pixels.setPixelColor(9, pixels.Color(50,50,50));
    pixels.setPixelColor(10, pixels.Color(50,50,50));
    pixels.setPixelColor(11, pixels.Color(50,50,50));
    pixels.show(); // This sends the updated pixel color to the hardware.

    pixels.setPixelColor(0, pixels.Color(0,0,0));
    pixels.setPixelColor(1, pixels.Color(0,0,0));
    pixels.setPixelColor(2, pixels.Color(0,0,0));
    pixels.setPixelColor(3, pixels.Color(0,0,0));
    pixels.setPixelColor(4, pixels.Color(0,0,0));
    pixels.setPixelColor(5, pixels.Color(0,0,0));
    pixels.setPixelColor(6, pixels.Color(0,0,0));
    pixels.setPixelColor(7, pixels.Color(0,0,0));
    pixels.setPixelColor(8, pixels.Color(0,0,0));
    pixels.setPixelColor(9, pixels.Color(0,0,0));
    pixels.setPixelColor(10, pixels.Color(0,0,0));
    pixels.setPixelColor(11, pixels.Color(0,0,0));
    pixels.show(); // This sends the updated pixel color to the hardware.

}

void turn_light_anime()
{
    pixels.setPixelColor(just_an_int, pixels.Color(10,10,10)); // Moderately bright white color.
    pixels.show(); // This sends the updated pixel color to the hardware.

    delay(LED_REFRESH_TIMER);

    if (just_an_int >0)
    {
        pixels.setPixelColor(just_an_int-1, pixels.Color(0,0,0)); // off.
        pixels.show(); // This sends the updated pixel color to the hardware.
    }
    just_an_int++;

    if (just_an_int == NUM_LEDS +1)
    {
        just_an_int = 0;
    }
}

void turn_light_game_over()
{
    display.setSegments(SEG_GAME);
    blink();

    for (int i=LED_MAX_INTENSITY; i>50; i--)
    {
        pixels.setPixelColor(0, pixels.Color(i,i,i));
        pixels.setPixelColor(1, pixels.Color(i,i,i));
        pixels.setPixelColor(2, pixels.Color(i,i,i));
        pixels.setPixelColor(3, pixels.Color(i,i,i));
        pixels.setPixelColor(4, pixels.Color(i,i,i));
        pixels.setPixelColor(5, pixels.Color(i,i,i));
        pixels.setPixelColor(6, pixels.Color(i,i,i));
        pixels.setPixelColor(7, pixels.Color(i,i,i));
        pixels.setPixelColor(8, pixels.Color(i,i,i));
        pixels.setPixelColor(9, pixels.Color(i,i,i));
        pixels.setPixelColor(10, pixels.Color(i,i,i));
        pixels.setPixelColor(11, pixels.Color(i,i,i));
        pixels.show();
        delay(LED_REFRESH_TIMER*2);
    }
    display.setSegments(SEG_OVER);

    for (int i=50; i>0; i--)
    {
        pixels.setPixelColor(0, pixels.Color(i,i,i));
        pixels.setPixelColor(1, pixels.Color(i,i,i));
        pixels.setPixelColor(2, pixels.Color(i,i,i));
        pixels.setPixelColor(3, pixels.Color(i,i,i));
        pixels.setPixelColor(4, pixels.Color(i,i,i));
        pixels.setPixelColor(5, pixels.Color(i,i,i));
        pixels.setPixelColor(6, pixels.Color(i,i,i));
        pixels.setPixelColor(7, pixels.Color(i,i,i));
        pixels.setPixelColor(8, pixels.Color(i,i,i));
        pixels.setPixelColor(9, pixels.Color(i,i,i));
        pixels.setPixelColor(10, pixels.Color(i,i,i));
        pixels.setPixelColor(11, pixels.Color(i,i,i));
        pixels.show();
        delay(LED_REFRESH_TIMER*2);
    }
    display.setSegments(SEG_EMPTY);
}

void turn_light_game_over_display_count()
{
    display.setSegments(SEG_GAME);
    blink();

    for (int i=LED_MAX_INTENSITY; i>50; i--)
    {
        pixels.setPixelColor(0, pixels.Color(i,i,i));
        pixels.setPixelColor(1, pixels.Color(i,i,i));
        pixels.setPixelColor(2, pixels.Color(i,i,i));
        pixels.setPixelColor(3, pixels.Color(i,i,i));
        pixels.setPixelColor(4, pixels.Color(i,i,i));
        pixels.setPixelColor(5, pixels.Color(i,i,i));
        pixels.setPixelColor(6, pixels.Color(i,i,i));
        pixels.setPixelColor(7, pixels.Color(i,i,i));
        pixels.setPixelColor(8, pixels.Color(i,i,i));
        pixels.setPixelColor(9, pixels.Color(i,i,i));
        pixels.setPixelColor(10, pixels.Color(i,i,i));
        pixels.setPixelColor(11, pixels.Color(i,i,i));
        pixels.show();
        delay(LED_REFRESH_TIMER*2);
    }
    display.setSegments(SEG_OVER);

    for (int i=50; i>0; i--)
    {
        pixels.setPixelColor(0, pixels.Color(i,i,i));
        pixels.setPixelColor(1, pixels.Color(i,i,i));
        pixels.setPixelColor(2, pixels.Color(i,i,i));
        pixels.setPixelColor(3, pixels.Color(i,i,i));
        pixels.setPixelColor(4, pixels.Color(i,i,i));
        pixels.setPixelColor(5, pixels.Color(i,i,i));
        pixels.setPixelColor(6, pixels.Color(i,i,i));
        pixels.setPixelColor(7, pixels.Color(i,i,i));
        pixels.setPixelColor(8, pixels.Color(i,i,i));
        pixels.setPixelColor(9, pixels.Color(i,i,i));
        pixels.setPixelColor(10, pixels.Color(i,i,i));
        pixels.setPixelColor(11, pixels.Color(i,i,i));
        pixels.show();
        delay(LED_REFRESH_TIMER*2);
    }
    display.setSegments(SEG_LINE);
    display.showNumberDec(count_game_2,true, 2, 1);
}

void blink_smile_red(){

    smile_red();
    display.setSegments(SEG_HAIR);
    delay(50);
    turn_light_off();
    display.setSegments(SEG_EMPTY);
    delay(50);
    smile_red();
    display.setSegments(SEG_HAIR);
    delay(50);
    turn_light_off();
    display.setSegments(SEG_EMPTY);
    delay(50);
    smile_red();
    display.setSegments(SEG_HAIR);
    delay(50);
    turn_light_off();
    display.setSegments(SEG_EMPTY);

}
///////////////////////////////////////////////////////////////////////////////////////////
//////////////////////     END of Funtions related to LED     ////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////////////////


String ipToString(IPAddress ip){
  String s="";
  for (int i=0; i<4; i++)
    s += i  ? "." + String(ip[i]) : String(ip[i]);
  return s;
}

unsigned char h2int(char c)
{
    if (c >= '0' && c <='9'){
        return((unsigned char)c - '0');
    }
    if (c >= 'a' && c <='f'){
        return((unsigned char)c - 'a' + 10);
    }
    if (c >= 'A' && c <='F'){
        return((unsigned char)c - 'A' + 10);
    }
    return(0);
}

String urlencode(String str)
{
    String encodedString="";
    char c;
    char code0;
    char code1;
    // char code2;
    for (unsigned i =0; i < str.length(); i++){
      c=str.charAt(i);
      if (c == ' '){
        encodedString+= '+';
      } else if (isalnum(c)){
        encodedString+=c;
      } else{
        code1=(c & 0xf)+'0';
        if ((c & 0xf) >9){
            code1=(c & 0xf) - 10 + 'A';
        }
        c=(c>>4)&0xf;
        code0=c+'0';
        if (c > 9){
            code0=c - 10 + 'A';
        }
        // code2='\0';
        encodedString+='%';
        encodedString+=code0;
        encodedString+=code1;
        //encodedString+=code2;
      }
      yield();
    }
    return encodedString;

}

String urldecode(String str)
{
    
    String encodedString="";
    char c;
    char code0;
    char code1;
    for (unsigned i =0; i < str.length(); i++){
        c=str.charAt(i);
      if (c == '+'){
        encodedString+=' ';  
      }else if (c == '%') {
        i++;
        code0=str.charAt(i);
        i++;
        code1=str.charAt(i);
        c = (h2int(code0) << 4) | h2int(code1);
        encodedString+=c;
      } else{
        
        encodedString+=c;  
      }
      
     // yield();
    }
    
   return encodedString;
}

void buddy_light_send_json(String json)
{
    buddylight.begin(client_buddylight, "http://"+buddylight_ip_address+"/sequence?data="+ urlencode(json));

    //buddy.sendRequest("POST","/status");
    //buddy.addHeader("content-type","text/plain");
    int http_code = buddylight.GET();

    Serial.print("Http code value: ");
    Serial.println(http_code);

    String Payload = buddylight.getString();
    Serial.println(Payload);

    buddylight.end();
}

///////////////////////////////////////////////////////////////////////////////
//////////////////// END OF OTHER FUNCTIONS DECLARATION////////////////////////
///////////////////////////////////////////////////////////////////////////////


String macToString(const unsigned char* mac) {
  char buf[20];
  snprintf(buf, sizeof(buf), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
  return String(buf);
}

void onStationConnected(const WiFiEventSoftAPModeStationConnected& evt) {
    Serial.println("Station connected: ");
    Serial.println(macToString(evt.mac));
    station_connected = true;
}

void onStationDisconnected(const WiFiEventSoftAPModeStationDisconnected& evt) {
    Serial.println("Station disconnected: ");
    Serial.println(macToString(evt.mac));
}

bool set_new_master(String IP_Address, String register_json){

    buddylight.begin(client_buddylight, "http://"+IP_Address+"/set_new_master?data="+ register_json);

    int http_code = buddylight.GET();

    Serial.print("Http code value: ");
    Serial.println(http_code);

    buddylight.end();

    return 1; // Successful

}

bool update_broadcast_register() {

    Serial.println("Updating the broadcast register, containing all the IP addresses but the IP address of the actual Light");
   // Create Json object to dump config file
    DynamicJsonBuffer jsonConfig;
    JsonObject& Config = jsonConfig.parseObject(json_config);

    if (master_padlight)
    {
        // do nothing
    }
    else
    {
        buddylight_around = Config["Number_of_buddy_lights"].as<int>()-1;// updates the number of lights present in network in global variable, minus 1 because we remove master
    }

    Serial.print("Value of buddylight_around: ");
    Serial.println(buddylight_around);
    
    
    int j = 0; // initialize integer that will parse the broadcast register

    for (int i=0; i<=buddylight_around; i++) // start from 0 since we take master into account
    {

        if (Config["RegisterProfile"][i].as<String>() == my_ip_address)
        {
                // do nothing
        }
        else
        {
            broadcastaddressregister[j]  = Config["RegisterProfile"][i].as<String>();
            Serial.print("IP address with ID ");
            Serial.print(j);
            Serial.print(" printed in the broadcast register: ");
            Serial.println(broadcastaddressregister[j]);
            j++;
        }

    }

    return 1;

}

bool update_json_register(String method, int index, String IP_address)
{
    
    Serial.println("update Json register function called...");
    
    // Create Json object to dump config file
    DynamicJsonBuffer jsonConfig;
    JsonObject& Config = jsonConfig.parseObject(json_config);

    if (method == "add")
    {
        // Checking if IP address already in register (this may happen if light has been turned on/off)
        for (int i=1; i<Config["Number_of_buddy_lights"].as<int>(); i++)
        {
            if (Config["RegisterProfile"][index] == IP_address)
            {   
                Serial.println("IP address already present, light already registered");
                return false;
            }
        }

        Serial.print("Adding new buddy light to the register:");
        Serial.println(IP_address);
        Config["RegisterProfile"][index]=IP_address;

        Serial.print("Updating number of buddy lights: ");
        Config["Number_of_buddy_lights"] = Config["Number_of_buddy_lights"].as<int>() + 1; 
        Config["Number_of_buddy_lights"].prettyPrintTo(Serial);

        buddylight_around = buddylight_around+1;

        Serial.println();

        json_config =""; // empty first
        Config.printTo(json_config);
        Serial.print("New json config object: ");
        Serial.println(json_config);

    }
    else
    {
        Serial.println("Removing from register buddy light that is not active anymore...");

        int index_2 = index;
        // Redefine the register
        while ((Config["RegisterProfile"][index_2+1].as<String>() != "0") &&(index_2 < 10)) {

            Config["RegisterProfile"][index_2] = Config["RegisterProfile"][index_2+1];
            index_2++;
        }
        Config["Number_of_buddy_lights"] = Config["Number_of_buddy_lights"].as<int>() -1;

        json_config ="";
        Config.printTo(json_config);
        Serial.print("New json config object: ");
        Serial.println(json_config);

        buddylight_around = buddylight_around -1;

        if (index == 0)
        {
            Serial.println("Master was disconnected, need to define another Master");
            set_new_master(Config["RegisterProfile"][0].as<String>(),json_config);
            return 1;
        }

    }

    update_broadcast_register(); // updating broadcast register as well

    broadcast_register_flag = 1; // broadcast register to all lights

    return 1; // successful

}

bool broadcast_register(){

    // Create Json object to dump config file
    DynamicJsonBuffer jsonConfig;
    JsonObject& Config = jsonConfig.parseObject(json_config);
    for (int i=1; i<Config["Number_of_buddy_lights"].as<int>(); i++){ // The reason why we loop from 1 and not 0, is that 0 is the master

        Serial.print("Broadcast register message to slave light: ");
        Serial.println(Config["RegisterProfile"][i].as<String>());

        buddylight.begin(client_buddylight, "http://"+Config["RegisterProfile"][i].as<String>()+"/broadcast_update_register?register_json="+ json_config);

        int http_code = buddylight.GET();

        Serial.print("Http code value: ");
        Serial.println(http_code);

        if (http_code != 200)
        {
            update_json_register("Remove", i, Config["RegisterProfile"][i].as<String>());
        }
        else
        {
            String Payload = buddylight.getString();
            Serial.println(Payload);
        }
        buddylight.end();

    }

    return 1; //success
}

bool broadcast_delta_time (){

    // Create Json object to dump config file
    DynamicJsonBuffer jsonConfig;
    JsonObject& Config = jsonConfig.parseObject(json_config);

    for (int i=1; i<Config["Number_of_buddy_lights"].as<int>(); i++)

    {
        Serial.print("Sending delta time to slave light: ");
        Serial.println(Config["RegisterProfile"][i].as<String>());

        buddylight.begin(client_buddylight, "http://"+Config["RegisterProfile"][i].as<String>()+"/update_delta_time?data="+ delta_time);

        int http_code = buddylight.GET();

        Serial.print("Http code value: ");
        Serial.println(http_code);

        if (http_code != 200)
        {
            update_json_register("Remove", i, Config["RegisterProfile"][i].as<String>());
        }
        else
        {
            String Payload = buddylight.getString();
            Serial.println(Payload);
        }
        buddylight.end();
    }

    return 1;
}

bool broadcast_count(){

    for (int i=0; i< buddylight_around; i++)
    {

        buddylight.begin(client_buddylight, "http://"+broadcastaddressregister[i]+"/broadcast_count");

        int http_code = buddylight.GET();

        Serial.print("Http code value: ");
        Serial.println(http_code);

    }

    buddylight.end();

    return 1;
}

bool broadcast_game_over(){

    for (int i=0; i< buddylight_around; i++)
    {

        buddylight.begin(client_buddylight, "http://"+broadcastaddressregister[i]+"/broadcast_game_over?flag_game="+flag_game);

        int http_code = buddylight.GET();

        Serial.print("Http code value: ");
        Serial.println(http_code);

    }

    buddylight.end();

    return 1;

}

bool broadcast_selected_game(){

    Serial.println("broadcasting selected game to other lights: ");

    for (int i=0; i< buddylight_around; i++) // start from 0 since we report as well to master
    {
        buddylight.begin(client_buddylight, "http://"+broadcastaddressregister[i]+"/broadcast_selected_game?flag_game="+flag_game);

        int http_code = buddylight.GET();

        Serial.print("Http code value: ");
        Serial.println(http_code);
    
    }
        
    buddylight.end();
    flag_broadcast_selected_game = 0; // make sure this is turned off after operation

    return 1;
}

bool report_to_master(){

    // Create Json object to dump config file
    DynamicJsonBuffer jsonConfig;
    JsonObject& Config = jsonConfig.parseObject(json_config);

    Serial.print("Report to Master light delta time:");
    Serial.println(deltaMillis);

    buddylight.begin(client_buddylight, "http://"+Config["RegisterProfile"][0].as<String>()+"/report_to_master?data="+ deltaMillis);
    int http_code = buddylight.GET();

    Serial.print("Http code value: ");
    Serial.println(http_code);

    if (http_code!=200)
    {
        Serial.println("Master is unavailable!");
        Serial.println("Set a new one, and report deltamillis");
        update_json_register("Remove", 0, Config["RegisterProfile"][0].as<String>());

        JsonObject& Config = jsonConfig.parseObject(json_config); // Parse upated json_config

        buddylight.begin(client_buddylight, "http://"+Config["RegisterProfile"][0].as<String>()+"/report_to_master?data="+ deltaMillis);
        http_code = buddylight.GET();

        Serial.print("Http code value: ");
        Serial.println(http_code);
    }   

    String Payload = buddylight.getString();
    Serial.println(Payload);

    buddylight.end();

    return 1; // Success

}

bool select_next_light_random() {

    Serial.println("Entering select next light random function...");

    // Selects a random number between lights present in network
    // We exclude the actual light from the random number extraction, since, 
    if (game_1_timer_between_lights>0) // timer between lights, and possibility to consecutively repeat the same light 
    {
        // Create Json object to dump config file
        DynamicJsonBuffer jsonConfig;
        JsonObject& Config = jsonConfig.parseObject(json_config);
        randNumber = random(0,Config["Number_of_buddy_lights"].as<int>()); // generate random number between 0 & max number of lights (minimum is inclusive, maximum is exclusive)

        Serial.print("randNumber = ");
        Serial.println(randNumber);

        next_light_ip_address = Config["RegisterProfile"][randNumber].as<String>();
        Serial.println("Ip address selected: ");
        Serial.print(next_light_ip_address);

        if (next_light_ip_address==my_ip_address)
        {
            Serial.println("Same light to catch!");
            i_am_the_one_to_catch = 1;
            start_timer = millis();
            turn_light_red();
        }
        else
        {
            buddylight.begin(client_buddylight, "http://"+next_light_ip_address+"/light_to_catch_transfer?data="+ deltaMillis);

            int http_code = buddylight.GET(); // removing this as waiting for return code may freeze. Not good as real time needed here

            Serial.print("Http code value returned: ");
            Serial.println(http_code);
        }   
    }
    else
    {

        randNumber = random(0,buddylight_around); // generate random number between 0 & max number of lights (minimum is inclusive, maximum is exclusive). We exclude the actual light
        next_light_ip_address = broadcastaddressregister[randNumber]; // already a String
        Serial.println("Ip address selected: ");
        Serial.print(next_light_ip_address);

        buddylight.begin(client_buddylight, "http://"+next_light_ip_address+"/light_to_catch_transfer?data="+ deltaMillis);

        int http_code = buddylight.GET(); // removing this as waiting for return code may freeze. Not good as real time needed here
        Serial.print("Http code value returned: ");
        Serial.println(http_code);

    }

    buddylight.end();

    return 1;

}   

bool select_next_light() {

    // Create Json object to dump config file
    DynamicJsonBuffer jsonConfig;
    JsonObject& Config = jsonConfig.parseObject(json_config);

    do
    {
        // DEBUG purpose
        Serial.print("prevRand = ");
        Serial.println(prevRand);
        Serial.print("buddylight_around = ");
        Serial.println(Config["Number_of_buddy_lights"].as<int>());

        randNumber = random(0,Config["Number_of_buddy_lights"].as<int>()); // generate random number between 0 & max number of lights (minimum is inclusive, maximum is exclusive)

        Serial.print("randNumber = ");
        Serial.println(randNumber);

    } while (randNumber == prevRand);

    prevRand = randNumber;
    Serial.print("Random number selected: ");
    Serial.println(randNumber); // show the value in Serial Monitor

    if (randNumber == 0) // Master to be lighted on 
    {
        i_am_the_one_to_catch = 1;
        start_timer = millis();
        turn_light_red();
    }
    else
    {
        // DEBUG PURPOSE:

        Serial.print("Sent to end point: ");
        String debug_string = "http://"+Config["RegisterProfile"][randNumber].as<String>()+"/light_to_catch_transfer?data="+ deltaMillis;
        Serial.println(debug_string);

        buddylight.begin(client_buddylight, "http://"+Config["RegisterProfile"][randNumber].as<String>()+"/light_to_catch_transfer?data="+ deltaMillis);

        int http_code = buddylight.GET();

        Serial.print("Http code value: ");
        Serial.println(http_code);

        if (http_code != 200)
        {
            Serial.print("Problem with sending message to buddy light: ");
            Serial.println(randNumber);

            update_json_register("remove", randNumber, Config["RegisterProfile"][randNumber].as<String>());

            Serial.println("Selecting another buddy light...");

            do {

                // DEBUG purpose
                Serial.print("prevRand = ");
                Serial.println(prevRand);
                Serial.print("buddylight_around = ");
                Serial.println(buddylight_around);

                random(0,Config["Number_of_buddy_lights"].as<int>());  // generate random number between 0 & max number of lights (minimum is inclusive, maximum is exclusive)

                Serial.print("randNumber = ");
                Serial.println(randNumber);

            } while (randNumber == prevRand);

            prevRand = randNumber;
            Serial.print("Random number selected: ");
            Serial.println(randNumber); // show the value in Serial Monitor

            if (randNumber == 0) // Master to be lighted on 
            {
                i_am_the_one_to_catch = 1;
                //delta_time = deltaMillis;
                //flag_broadcast_delta_time = 1;
            }
            else
            {
                buddylight.begin(client_buddylight, "http://"+Config["RegisterProfile"][randNumber].as<String>()+"/light_to_catch_transfer?data="+ deltaMillis);

                int http_code = buddylight.GET();

                Serial.print("Http code value: ");
                Serial.println(http_code);

                if (http_code != 200)
                {
                    Serial.print("Problem with sending message to buddy light: ");
                    Serial.println(randNumber);
                    update_json_register("remove", randNumber, Config["RegisterProfile"][randNumber].as<String>());
                
                }
            }
        }
        String Payload = buddylight.getString();
        Serial.println(Payload);

        buddylight.end();
    }

    return 1;

}

bool start_game_broadcast() {

    Serial.println("broadcast game started event to other lights: ");

    for (int i=0; i< buddylight_around; i++) // start from 0 since we report as well to master
    {
        buddylight.begin(client_buddylight, "http://"+broadcastaddressregister[i]+"/broadcast_game_start?flag_game="+flag_game);

        int http_code = buddylight.GET();

        Serial.print("Http code value: ");
        Serial.println(http_code);
    
    }
        
    buddylight.end();

    return 1;
}


bool broadcast_count_battle(int colour) {

    Serial.print("broadcast count battle to other lights, update counter for colour: ");
    Serial.println(colour);

    for (int i=0; i< buddylight_around; i++) // start from 0 since we report as well to master
    {
        buddylight.begin(client_buddylight, "http://"+broadcastaddressregister[i]+"/broadcast_count_battle?colour_code="+colour);

        int http_code = buddylight.GET();

        Serial.print("Http code value: ");
        Serial.println(http_code);
    
    }
        
    buddylight.end();

    return 1;
}



void setup() {

    Serial.begin(9600);

    //Start Led stripe

    pixels.begin();

    //Allocate pin for movement sensor

    pinMode(D4,INPUT);

    // Manage PAD config file stored in File system
    SPIFFS.begin();
    //SPIFFS.format();

    // Monitor station events
    stationConnectedHandler = WiFi.onSoftAPModeStationConnected(&onStationConnected);
    // Call "onStationDisconnected" each time a station disconnects
    stationDisconnectedHandler = WiFi.onSoftAPModeStationDisconnected(&onStationDisconnected);

    // End of PAD config file management

    Serial.println("Wifi client connecting...");

    const char* ssidlight     = "OPEN_PADS";
    const char* passwordlight = "PADS1234";

    //WiFi.disconnect(true);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssidlight,passwordlight);

    // add cors headers
    DefaultHeaders::Instance().addHeader("Access-Control-Allow-Origin", "*");

    int counter_connection=0;
    server.begin();
    Serial.println("HTTP server started BEFORE mDns assign");
    Serial.println("Server listening");
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        if (counter_connection==10)
        {
            break;
        }
        counter_connection++;
    }

   if (counter_connection==10) // There is no hotspot around
    {
        //WiFi.disconnect(true);
        Serial.println("No Master network detected, so switching Pad to AP mode");

        connect_mode = "AP";

        // Sets wifi network in AP mode
        WiFi.softAPConfig(pads_ip,pads_gateway,pads_subnet);
        WiFi.mode(WIFI_AP);
        WiFi.softAP(ssidlight,passwordlight);
        master_padlight = 1; // I AM THE MASTERRRRRR !!!
        Serial.println("Master flag set!");

        my_ip_address = pads_ip.toString(); // Will be used for IP random selection for game 1

        // Initiate JSON Object for PAD light configuration
        DynamicJsonBuffer jsonConfig;
        JsonObject& Config = jsonConfig.createObject();

        Config["Number_of_buddy_lights"] = 1; //initialize
        JsonArray& RegisterProfile = Config.createNestedArray("RegisterProfile");
        RegisterProfile.add(pads_ip.toString()); // This is the master
        RegisterProfile.add("0");
        RegisterProfile.add("0");
        RegisterProfile.add("0");
        RegisterProfile.add("0");
        RegisterProfile.add("0");
        RegisterProfile.add("0");
        RegisterProfile.add("0");
        RegisterProfile.add("0");
        RegisterProfile.add("0");

        JsonArray& BroadcastProfile = Config.createNestedArray("BroadcastProfile");
        BroadcastProfile.add("0");
        BroadcastProfile.add("0");
        BroadcastProfile.add("0");
        BroadcastProfile.add("0");
        BroadcastProfile.add("0");
        BroadcastProfile.add("0");
        BroadcastProfile.add("0");
        BroadcastProfile.add("0");

        json_config ="";
        Config.printTo(json_config);

        Serial.println("This is the register profile: ");
        Config.prettyPrintTo(Serial);

    }
    else
    {

        Serial.print("IP address is: ");
        Serial.println(WiFi.localIP().toString());

        my_ip_address = WiFi.localIP().toString();

        connect_mode = "HotSpot";
        // Create and initiate register

        // Initiate JSON Object for PAD light configuration
        DynamicJsonBuffer jsonConfig;
        JsonObject& Config = jsonConfig.createObject();

        Config["Number_of_buddy_lights"] = 1; //initiate
        JsonArray& RegisterProfile = Config.createNestedArray("RegisterProfile");
        RegisterProfile.add(pads_ip.toString()); // This is the master
        RegisterProfile.add("0");
        RegisterProfile.add("0");
        RegisterProfile.add("0");
        RegisterProfile.add("0");
        RegisterProfile.add("0");
        RegisterProfile.add("0");
        RegisterProfile.add("0");
        RegisterProfile.add("0");
        RegisterProfile.add("0");

        JsonArray& BroadcastProfile = Config.createNestedArray("BroadcastProfile");
        BroadcastProfile.add("0");
        BroadcastProfile.add("0");
        BroadcastProfile.add("0");
        BroadcastProfile.add("0");
        BroadcastProfile.add("0");
        BroadcastProfile.add("0");
        BroadcastProfile.add("0");
        BroadcastProfile.add("0");

        json_config ="";
        Config.printTo(json_config);

        Serial.println("This is the register profile: ");
        Serial.print(json_config);


        Serial.println();

        Serial.println("padlight Master already present, send our IP address");


        buddylight.begin(client_buddylight, "http://"+pads_ip.toString()+"/buddylight_is_here?buddy_ip="+ urlencode(WiFi.localIP().toString()));
        int http_code = buddylight.GET();

        Serial.print("Http code value: ");
        Serial.println(http_code);

        String Payload = buddylight.getString();
        Serial.println(Payload);

        buddylight.end();

        master_padlight = 0; // I AM A SLAVE
        Serial.println("Slave flag set!");
        idle_flag = 1; // be idle waiting for selected game information

    }


////////////////////////////////////////////////////////////////////////////////
//////////////////////// SERVER END POINTS DECLARATION//////////////////////////
////////////////////////////////////////////////////////////////////////////////


    server.on("/broadcast_game_start", HTTP_ANY, [](AsyncWebServerRequest *request){

        Serial.println("broadcast_game_start end point called");
        flag_game = request->arg("flag_game").toInt();
        request->send(200,"text/plain","Game Start event received");
        Serial.print("Game started event for game: ");
        Serial.println(flag_game);

        flag_game_started = 1;

        if (flag_game==2)
        {
            start_timer=millis();
            count_game_2 = 0 ;
            another_int = 0; // init parameters
            flag_light_up =0; // init flags and parameters
            turn_light_off();
            display.setSegments(SEG_LINE);
            display.showNumberDec(count_game_2,true, 2, 1);
        }

        if (flag_game==3)
        {
            turn_light_off();
            flag_game_started = 1;
            start_timer = millis();
            another_int = 0; // init parameters
            flag_light_up = 0; // init flags and parameters
            count_game_3_colour1 = 0;
            count_game_3_colour2 = 0;
            flag_update_count = 1;
        }

    });

    server.on("/broadcast_count", HTTP_ANY, [](AsyncWebServerRequest *request){

        Serial.println("broadcast_count end point called");
        request->send(200,"text/plain","Counter updated");

        count_game_1++; // update count
        count_game_2++; // update count
        Serial.print("Game 1 counter updated value is: ");
        Serial.println(count_game_1);
        Serial.print("Game 2 counter updated value is: ");
        Serial.println(count_game_2);

        if (i_am_the_one_to_catch == 0)
        {
            flag_update_count = 1; // display update count
        }

        if (flag_game == 2)
        {
            flag_update_count = 1; // display update count
        }

    });

    server.on("/broadcast_count_battle", HTTP_ANY, [](AsyncWebServerRequest *request){

        Serial.println("broadcast_count_battle end point called");
        int colour_code = request->arg("colour_code").toInt();
        request->send(200,"text/plain","Counter color updated");

        Serial.print("Colour code value extracted from web parameter: ");
        Serial.println(colour_code);

        if (colour_code == 0)
        {
            count_game_3_colour1++;
            flag_update_count = 1; // display update count
            Serial.print("Game 3 counter colour 1 updated value is: ");
            Serial.println(count_game_3_colour1);
        }
        else
        {
            count_game_3_colour2++;
            flag_update_count = 1; // display update count
            Serial.print("Game 3 counter colour 2 updated value is: ");
            Serial.println(count_game_3_colour2);
        }

    });


    server.on("/broadcast_game_over", HTTP_ANY, [](AsyncWebServerRequest *request){

        Serial.println("broadcast_game_over end point called");
        flag_game = request->arg("flag_game").toInt();
        request->send(200,"text/plain","Game over notification received");
        Serial.print("Game over received for game: ");
        Serial.println(flag_game);

        if (flag_game == 0)
        {
            idle_flag = 1; // idle, waiting for game selection
            flag_game_menu = 0;
        }
        else
        {
            flag_game_menu = 0; // exit menu mode
            idle_flag = 0;

            if (flag_game == 1)
            {
                flag_game_over = 1;
                count_game_1 = 0; // reset parameter
                deltaMillis = 0; // reset parameter
                flag_game_started = 1; // the light pad is going to directly in game 1 routine, waiting for first touch
                i_am_the_one_to_catch = 0; // the light pad is going to game 1 run time
            }

        }

    });


    server.on("/broadcast_selected_game", HTTP_ANY, [](AsyncWebServerRequest *request){

        Serial.println("broadcast_selected_game end point called");
        flag_game = request->arg("flag_game").toInt();
        request->send(200,"text/plain","Game selected received");
        Serial.print("Game selected: ");
        Serial.println(flag_game);

        if (flag_game == 0)
        {
            idle_flag = 1; // idle, waiting for game selection
            flag_game = 0; // avoids going to previous game routine
            flag_game_over = 0; // avoid game over anime at startup ...
            flag_game_menu = 0; // avoids haveing multiple lights with menu display
            count_game_1 = 0; // resets all game parameters
            count_game_2 = 0; // resets all game parameters
            deltaMillis = 0; // resets all game parameters
            start_timer = 0; // resets all game parameters
            display.setSegments(SEG_MENU);
        }
        else
        {
            flag_game_menu = 0; // exit menu mode
            idle_flag = 0;

            if (flag_game == 1)
            {
                // We directly go to the game 1 routine, waiting for the player to touch the pad and start game
                count_game_1 = 0; // init game parameters
                deltaMillis = 0; // init game parameters
                flag_game_started = 1; //to enter start game 1 menu
                i_am_the_one_to_catch = 0; //will put the light pod in wait status
                display.setSegments(SEG_EMPTY); // erase all
            }
            if (flag_game == 2)
            {
                // We directly go to the game 1 routine, waiting for the player to touch the pad and start game
                count_game_2 = 0; // init game parameters
                display.setSegments(SEG_PLAY); // erase all
            }
        }
    });

    server.on("/buddylight_is_here", HTTP_ANY, [](AsyncWebServerRequest *request){

        Serial.println("buddylight_is_here end point called");
        buddylight_ip_address = urldecode(request->arg("buddy_ip"));
        request->send(200,"text/plain","Buddylight IP Address received");
        Serial.print("buddylight IP Address is: ");
        Serial.println(buddylight_ip_address);
        Serial.print("buddy light around used for update register function: ");
        Serial.println(buddylight_around+1);

        update_json_register("add", buddylight_around+1, buddylight_ip_address);

        flag_broadcast_selected_game = 1; // update game status to all lights
        broadcast_register_flag = 1; // updating register to all lights
    });

    server.on("/set_new_master", HTTP_ANY, [](AsyncWebServerRequest *request){

        Serial.println("set new master end point called");
        master_padlight = 1;
        json_config = "";
        json_config = request->arg("register_json");
        request->send(200,"text/plain","New master set");
        Serial.println("I am the new master!");
        broadcast_register_flag = 1;

    });

    server.on("/broadcast_update_register", HTTP_ANY, [](AsyncWebServerRequest *request){

        Serial.println("broadcast_update_register end point called");
        request->send(200,"text/plain","Broadcast Register update acknowledgedby peer");
        json_config = "";
        json_config = request->arg("register_json");


        update_broadcast_register();

        Serial.println("register updated");
        Serial.println(json_config);

    });

    server.on("/light_to_catch_transfer", HTTP_ANY, [](AsyncWebServerRequest *request){

        Serial.println("Light to catch transfer end point triggered");
        request->send(200,"text/plain","New light to catch request received");
        i_am_the_one_to_catch = 1;
        start_timer=millis();
        just_an_int = 0; // initialize
        turn_light_red();

    });

    server.on("/report_to_master", HTTP_ANY, [](AsyncWebServerRequest *request){

        Serial.println("Feedback received from buddy light");
        delta_time = request->arg("data");
        request->send(200,"text/plain","Feedback received");
        //flag_broadcast_delta_time = 1; //need to broadcast the new delta time to all lights // No use of this any more

        // In case we are at game 1 menu
        if ((flag_game==1)&&(flag_game_started == 0))
        {
            flag_game_started = 1;
            i_am_the_one_to_catch =1; // sets himself as the one to catch
            start_timer = millis(); // initialise timer displayed on device
            just_an_int = 0;
            turn_light_red();
        }
        else
        {
            /*
            flag_broadcast_delta_time = 1;
            flag_select_light = 1;
            Serial.print("delta millis value received: ");
            Serial.println(delta_time.toInt());
            display.showNumberDec(delta_time.toInt(),true);
            */
        }
        
    });

    server.on("/update_delta_time", HTTP_ANY, [](AsyncWebServerRequest *request){

    Serial.println("Delta time update received");
    delta_time = request->arg("data");
    request->send(200,"text/plain","Feedback received");
    display.showNumberDec(delta_time.toInt(),true);

    });

///////////////////////////////////////////////////////////////////////////////
//////////////////// END OF SERVER END POINTS DECLARATION//////////////////////
///////////////////////////////////////////////////////////////////////////////


    // TM Led display clear and brightness set-up

    display.setBrightness(10);

    battery_level = analogRead(ANALOG_READ_BATTERY);
    Serial.print("Battery Level:");
    Serial.println(battery_level);
    //Serial.setDebugOutput(true);

    ArduinoOTA.setHostname(PADS_ID);
    ArduinoOTA.begin();

}

void loop() {

    // TESTING CAPACITY SENSOR SKETCH
    /*
    long sensorValue1 = capSensor1.capacitiveSensor(30);
    Serial.println(sensorValue1);
    delay(10);
    */
    // TESTING CAPACITY SENOR SKETCH END

    // TESTING ULTRASONIC SENSOR SKETCH
    
    //Serial.println(sensor_sonic.read());
    //display.showNumberDec(sensor_sonic.read(),true);
    //delay(10);
    
    // TESTING ULTRASONIC SENSOR SKETCH END

    //ArduinoOTA.handle();
    // If 1, then no movement. If 0, movement detected

    // DETECTING LONG PRESS TO ENTER MENU AND SELECT GAMES
    
    // while (capSensor1.capacitiveSensor(SENSOR_SENSITIVENESS)>SENSOR_THRESHOLD) // if movement
    while(sensor_sonic.read()<SONIC_SENSOR_THRESHOLD_LONG_TOUCH)
    {
        just_an_int++;

        // Anime purpose
        anime_ring(just_an_int);
        // End of anime

        //while ((just_an_int > 200)&&(capSensor1.capacitiveSensor(SENSOR_SENSITIVENESS)>SENSOR_THRESHOLD))
        if ((just_an_int > 100)&&(sensor_sonic.read()<SONIC_SENSOR_THRESHOLD_LONG_TOUCH)) // Putting an if instead of a while...
        {
            Serial.println("Detected long button touch");
            blink();
            flag_game_menu = 1;
            flag_game = 0;
            flag_game_started = 0;
            just_an_int = 0;
            flag_broadcast_selected_game = 1; // let others light know that I am in the menu now
            break;
        }
        delay(10);
    }
    just_an_int = 0; // reset the counter when while loop finished

    if (broadcast_register_flag)
    {
        broadcast_register();
        broadcast_register_flag = 0;
    }

    if (flag_broadcast_selected_game)
    {
        broadcast_selected_game();
        flag_broadcast_selected_game = 0;
    }

    while (idle_flag)
    {
        turn_light_white_fade();
    }

    while (((flag_game_menu)&&(buddylight_around!=0))||((flag_game_menu)&&(master_padlight==0))) // takes into account cases where light is master or slave
    {

        display.setSegments(SEG_MENU);

        //manage the case when register update needed on the fly (as an example, when new light registered in network)
        if (broadcast_register_flag)
        {
            broadcast_register();
            broadcast_register_flag = 0;
        }

        turn_light_red_fade();
        flag_game = 1;

        //if(capSensor1.capacitiveSensor(SENSOR_SENSITIVENESS)>SENSOR_THRESHOLD)
        if(sensor_sonic.read()<SONIC_SENSOR_THRESHOLD)
        {
            blink();
            flag_game_menu = 0;
            broadcast_selected_game();
            break;
        }
        turn_light_green_fade();
        flag_game = 2;

        //if(capSensor1.capacitiveSensor(SENSOR_SENSITIVENESS)>SENSOR_THRESHOLD)
        if(sensor_sonic.read()<SONIC_SENSOR_THRESHOLD)
        {
            blink();
            flag_game_menu = 0;
            display.setSegments(SEG_PLAY);
            broadcast_selected_game();
            break;
        }
        turn_light_blue_fade();
        flag_game = 3;

        //if(capSensor1.capacitiveSensor(SENSOR_SENSITIVENESS)>SENSOR_THRESHOLD)
        if(sensor_sonic.read()<SONIC_SENSOR_THRESHOLD)
        {
            blink();
            flag_game_menu = 0;
            broadcast_selected_game();
            break;
        }

        turn_light_magenta_fade();
        flag_game = 4;
        if(sensor_sonic.read()<SONIC_SENSOR_THRESHOLD)
        {
            blink();
            flag_game_menu = 0;
            broadcast_selected_game();
            break;
        }

    }

    while ((flag_game==1)&&(flag_game_started ==0)) // We are at root menu game 1
    {
        /* //Don't need this anymore
        if (flag_game_over)
        {
            turn_light_game_over(); // plays game over light once and then go back to main routine
            flag_game_over = 0;
        }
        */ 

        //init game parameters
        deltaMillis = 0;
        count_game_1 = 0;
        flag_game_over = 0;

        Serial.println("Game 1 selected and ready to start...");

        //if(capSensor1.capacitiveSensor(SENSOR_SENSITIVENESS)>SENSOR_THRESHOLD) // Movement
        if(sensor_sonic.read()<SONIC_SENSOR_THRESHOLD)
        {
            // START THE GAME!!!
            blink();
            flag_game_started = 1;
            select_next_light_random();
            break;
        }

        turn_light_red_fade();
        display.setSegments(SEG_PLAY);

    }

    while ((flag_game==2)&&(flag_game_started ==0)) // We are at root menu game 2
    {
        Serial.println("Game 2 selected and ready to start...");
        if (flag_game_over)
        {
            turn_light_game_over(); // plays game over light once and then go back to main routine
            flag_game_over = 0;
        }

        turn_light_green_fade();

        //if(capSensor1.capacitiveSensor(SENSOR_SENSITIVENESS)>SENSOR_THRESHOLD) // Movement
        if(sensor_sonic.read()<SONIC_SENSOR_THRESHOLD)
        {
            // START THE GAME!!!
            Serial.println("Game 2 started!");
            blink();
            turn_light_off();
            flag_game_started = 1;
            start_game_broadcast(); // Game 2 start broadcast to all pads
            start_timer = millis();
            another_int = 0; // init parameters
            flag_light_up = 0; // init flags and parameters
            count_game_2 = 0; // init parameters
            display.setSegments(SEG_LINE);
            display.showNumberDec(count_game_2,true, 2, 1);
            break;
        }
    }

    while ((flag_game==3)&&(flag_game_started ==0)) // We are at root menu game 3
    {
        Serial.println("Game 3 selected and ready to start...");
        turn_light_blue_fade();

        // format display correctly:

        if ((count_game_3_colour1 > 9)&&(count_game_3_colour2 > 9))
        {
            concatenated_count = String(count_game_3_colour1) + String (count_game_3_colour2);
            display.showNumberDecEx(concatenated_count.toInt(),0b01000000,1, 4, 0);
        }
        
        if ((count_game_3_colour1 > 9)&&(count_game_3_colour2 < 10))
        {
            concatenated_count = String(count_game_3_colour1) + "0" + String (count_game_3_colour2);
            display.showNumberDecEx(concatenated_count.toInt(),0b01000000,1, 4, 0);
        }

        if ((count_game_3_colour1 < 10)&&(count_game_3_colour2 > 9))
        {
            concatenated_count = "0" + String(count_game_3_colour1) + String (count_game_3_colour2);
            display.showNumberDecEx(concatenated_count.toInt(),0b01000000,1, 4, 0);
        }

        if ((count_game_3_colour1 < 10)&&(count_game_3_colour2 < 10))
        {
            concatenated_count = "0" + String(count_game_3_colour1) + "0" + String (count_game_3_colour2);
            display.showNumberDecEx(concatenated_count.toInt(),0b01000000,1, 4, 0);
        }
        
        if(sensor_sonic.read()<SONIC_SENSOR_THRESHOLD)
        {
            Serial.println("Game 3 started!");
            blink();
            turn_light_off();
            flag_game_started = 1;
            start_game_broadcast();
            start_timer = millis();
            another_int = 0; // init parameters
            flag_light_up = 0; // init flags and parameters
            count_game_3_colour1 = 0;
            count_game_3_colour2 = 0;

            concatenated_count = String(count_game_3_colour1) + String (count_game_3_colour2);
            display.showNumberDecEx(concatenated_count.toInt(),0b01000000,1, 4, 0);
        }
        //delay(1000);

    }

    while ((flag_game==4)&&(flag_game_started ==0)) // We are at root menu game 4
    {
        Serial.println("Game 4 selected and ready to start...");
        display.setSegments(SEG_PADS);
        turn_light_magenta_fade();

        // format display correctly:

        just_an_int = 0;
        while(sensor_sonic.read()<SONIC_SENSOR_THRESHOLD_LONG_TOUCH)
        {
            just_an_int++;

            // Anime purpose

            anime_ring(just_an_int);

            // End of anime
        
            //while ((just_an_int > 200)&&(capSensor1.capacitiveSensor(SENSOR_SENSITIVENESS)>SENSOR_THRESHOLD))
            if ((just_an_int > 100)&&(sensor_sonic.read()<SONIC_SENSOR_THRESHOLD_LONG_TOUCH)) // Putting an if instead of a while...
            {
                Serial.println("Detected long button touch");
                blink();
                flag_game_menu = 1;
                flag_game = 0;
                flag_game_started = 0;
                just_an_int = 0;
                flag_broadcast_selected_game = 1; // let others light know that I am in the menu now
                break;
            }
            delay(10);

        }
        just_an_int = 0; // reset the counter when while loop finished
        delay(10); // avoids crash

    }
    
    while ((buddylight_around==0)&&(master_padlight)) // do nothing until 2 PAD light are connected
    {
        just_an_int++;
        ArduinoOTA.handle();
        anime_ring_low_intensity(just_an_int);
        if(just_an_int == 96) {just_an_int =0;}

        // Test display
        //display.setSegments(SEG_Init);
        // Test display End

        delay(20);
    }

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////    GAME 1      ////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    while ((flag_game==1)&&(flag_game_started ==1)) // We are in game 1 running
    {
        if (flag_game_over)
        {
            turn_light_game_over(); // plays game over light once and then go back to main routine
            flag_game_over = 0;
        }
        if (flag_update_count)
        {
            display.showNumberDec(count_game_1,true);
            flag_update_count =0;
        }

        if (i_am_the_one_to_catch)
        {
            deltaMillis = millis() - start_timer;
            display.showNumberDec(deltaMillis,true);

            if (deltaMillis > GAME1_TIMER_THRESHOLD) // blinks when game timer is about to expire...
            {
                blink_red();   
            }

            if (deltaMillis > GAME1_TIMER_GAME_OVER_THRESHOLD) // Game over, play light anime and go back to start menu
            {
                broadcast_game_over(); // tells the other lights to go back to start game 1 step

                flag_game = 1; // stay with game 1
                flag_game_started = 0; // exit the while loop go back to start game 1 step
                count_game_1 = 0; // reset the counter
                deltaMillis = 0; // reset the timer
                i_am_the_one_to_catch = 0; // to avoid infinite loop at game restart...
                turn_light_game_over(); // game over anime...
            }
            
            // PLACE HOLDER Light management

            // Manage the case when first time launch for master
            //if (capSensor1.capacitiveSensor(SENSOR_SENSITIVENESS)>SENSOR_THRESHOLD) // if movement

            sensor_read = sensor_sonic.read();

            if((sensor_read<SONIC_SENSOR_THRESHOLD)&&(deltaMillis>300)) // This is to avoid any race conditions with small time intervals between shots
            {
                if (sensor_sonic.read()<SONIC_SENSOR_THRESHOLD) // make a second test to make sure sonic test is positive
                {
                    Serial.print("Value of sensor read: ");
                    Serial.println(sensor_read);
                    select_next_light_random();
                    blink_short_white();
                    i_am_the_one_to_catch = 0;

                    count_game_1++;
                    display.showNumberDec(count_game_1,true);
                    flag_broadcast_count=1;
                    deltaMillis = 0; // reset 

                    break;
                }
            }
        }
        else
        {
            turn_light_off();

            //Takes into account the long touch exit:

            // while (capSensor1.capacitiveSensor(SENSOR_SENSITIVENESS)>SENSOR_THRESHOLD) // if movement
            just_an_int = 0;
            while(sensor_sonic.read()<SONIC_SENSOR_THRESHOLD_LONG_TOUCH)
            {
                just_an_int++;

                // Anime purpose

                anime_ring(just_an_int);

                // End of anime
            
                //while ((just_an_int > 200)&&(capSensor1.capacitiveSensor(SENSOR_SENSITIVENESS)>SENSOR_THRESHOLD))
                if ((just_an_int > 100)&&(sensor_sonic.read()<SONIC_SENSOR_THRESHOLD_LONG_TOUCH)) // Putting an if instead of a while...
                {
                    Serial.println("Detected long button touch");
                    blink();
                    flag_game_menu = 1;
                    flag_game = 0;
                    flag_game_started = 0;
                    just_an_int = 0;
                    flag_broadcast_selected_game = 1; // let others light know that I am in the menu now
                    deltaMillis = 0;  // reset
                    count_game_1 = 0; // reset
                    break;
                }
                delay(10);

            }
            just_an_int = 0; // reset the counter when while loop finished

            // Long touch exit end
        }

        if (flag_select_light)
        {
            select_next_light_random(); // light up the next light
            flag_select_light = 0;
        }

        if (flag_broadcast_delta_time)
        {
            broadcast_delta_time();
            flag_broadcast_delta_time = 0;
        }
        delay(10); // avoid system to crash

    }
    ////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////    GAME 1 END    //////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////    GAME 2     ////////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    while ((flag_game==2)&&(flag_game_started ==1)) // We are in game 2 running
    {
        if (flag_update_count) // update counter
        {
            flag_update_count = 0;
            display.showNumberDec(count_game_2,true, 2, 1);
            delay(50);
            display.setSegments(SEG_EMPTY);
            delay(50);
            display.setSegments(SEG_LINE);
            display.showNumberDec(count_game_2,true, 2, 1);
        }

        if (flag_game_over)
        {
            turn_light_game_over_display_count(); // plays game over light once and then go back to main routine
            flag_game_over = 0;
        }

        if ((millis() - start_timer) > GAME2_TIMER_GAME_OVER_THRESHOLD)
        {
            flag_game_started = 0; // go back to menu, the game time is up
            count_game_2 = 0;
            turn_light_game_over_display_count();
        }

        another_int++; // counter that synchs all the timer instructions at run play

        if((flag_light_up==1)&&(sensor_sonic.read()<SONIC_SENSOR_THRESHOLD)) // Touch when light on
        {
            if (sensor_sonic.read()<SONIC_SENSOR_THRESHOLD) // make a second test to make sure sonic test is positive
            {
                Serial.println("LIGHT TOUCHED!");
                random_wait_time_game2 = 0;
                flag_light_up = 0;
                another_int = 0;
                blink_smile_red();
                count_game_2++;
                broadcast_count();
                display.showNumberDec(count_game_2,true, 2, 1);
                delay(50);
                display.setSegments(SEG_EMPTY);
                delay(50);
                display.setSegments(SEG_LINE);
                display.showNumberDec(count_game_2,true, 2, 1);
            }
        }

        just_an_int = 0; // reset this counter to detect long touch

        while((flag_light_up==0)&&(sensor_sonic.read()<SONIC_SENSOR_THRESHOLD_LONG_TOUCH)) // Long touch to exit game by player, and we aloow this operation when light down
        {
            just_an_int++;

            // Anime purpose

            anime_ring(just_an_int);

            // End of anime
        
            //while ((just_an_int > 200)&&(capSensor1.capacitiveSensor(SENSOR_SENSITIVENESS)>SENSOR_THRESHOLD))
            if ((just_an_int > 100)&&(sensor_sonic.read()<SONIC_SENSOR_THRESHOLD_LONG_TOUCH)) // Putting an if instead of a while...
            {
                Serial.println("Detected long button touch");
                blink();
                flag_game_menu = 1;
                flag_game = 0;
                flag_game_started = 0;
                just_an_int = 0;
                flag_broadcast_selected_game = 1; // let others light know that I am in the menu now
                break;
            }
            delay(10);
        }

        /*if (flag_light_up == 0) // This to remove anime white dots 
        {
            turn_light_off();
        }*/

        if(random_wait_time_game2 == 0)
        {
            random_wait_time_game2 = random(10,150);
            Serial.print("Random wait time selected: ");
            Serial.println(random_wait_time_game2);
        }

        if (another_int > random_wait_time_game2)
        {
            Serial.print("Wait time is up!");
            random_wait_time_game2 = 0; // for next iteration of the loop, the random time will be computed again

            if (flag_light_up) // if light urned up, turn it down
            {
                turn_light_off();
                flag_light_up = 0;
            }
            else
            {
                turn_light_green();
                flag_light_up = 1; // next time random time is passed, light will be down
            }

            another_int = 0; // reset the counter
        }

        delay(10); // avoid system to crash
    }   

    ////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////    GAME 2 END    //////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////


    ////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////    GAME 3    //////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////

    while ((flag_game==3)&&(flag_game_started ==1)) // We are in game 3 running
    {
        if (flag_update_count) // update counter
        {

            Serial.print("Value of counter colour_1:  ");
            Serial.print(count_game_3_colour1);
            Serial.print(" , Value of counter colour_2:  ");
            Serial.println(count_game_3_colour2);         

            flag_update_count = 0;

            if ((count_game_3_colour1 > 9)&&(count_game_3_colour2 > 9))
            {
                concatenated_count = String(count_game_3_colour1) + String (count_game_3_colour2);
                display.showNumberDecEx(concatenated_count.toInt(),0b01000000,1, 4, 0);
            }
            
            if ((count_game_3_colour1 > 9)&&(count_game_3_colour2 < 10))
            {
                concatenated_count = String(count_game_3_colour1) + "0" + String (count_game_3_colour2);
                display.showNumberDecEx(concatenated_count.toInt(),0b01000000,1, 4, 0);
            }

            if ((count_game_3_colour1 < 10)&&(count_game_3_colour2 > 9))
            {
                concatenated_count = "0" + String(count_game_3_colour1) + String (count_game_3_colour2);
                display.showNumberDecEx(concatenated_count.toInt(),0b01000000,1, 4, 0);
            }

            if ((count_game_3_colour1 < 10)&&(count_game_3_colour2 < 10))
            {
                concatenated_count = "0" + String(count_game_3_colour1) + "0" + String (count_game_3_colour2);
                display.showNumberDecEx(concatenated_count.toInt(),0b01000000,1, 4, 0);
            }

        }

        if (flag_game_over)
        {
            turn_light_game_over(); // plays game over light once and then go back to main routine
            flag_game_over = 0;
        }

        if ((millis() - start_timer) > GAME2_TIMER_GAME_OVER_THRESHOLD)
        {
            flag_game_started = 0; // go back to menu, the game time is up
            turn_light_game_over();
        }

        another_int++; // counter that synchs all the timer instructions at run play

        if((flag_light_up==1)&&(sensor_sonic.read()<SONIC_SENSOR_THRESHOLD)) // Touch when light on
        {
            if (sensor_sonic.read()<SONIC_SENSOR_THRESHOLD) // make a second test to make sure sonic test is positive
            {
                Serial.println("LIGHT TOUCHED!");
                random_wait_time_game2 = 0;
                flag_light_up = 0;
                another_int = 0;
                blink_smile_red();

                if (light_colour == 0)
                {
                    count_game_3_colour1++;
                    flag_update_count = 1;
                }
                else
                {
                    count_game_3_colour2++;
                    flag_update_count = 1;
                }
                broadcast_count_battle(light_colour); // needs to specify the colour of the light touched in the counter
            }
        }

        just_an_int = 0; // reset this counter to detect long touch

        while((flag_light_up==0)&&(sensor_sonic.read()<SONIC_SENSOR_THRESHOLD_LONG_TOUCH)) // Long touch to exit game by player, and we aloow this operation when light down
        {
            just_an_int++;

            // Anime purpose

            anime_ring(just_an_int);

            // End of anime
        
            //while ((just_an_int > 200)&&(capSensor1.capacitiveSensor(SENSOR_SENSITIVENESS)>SENSOR_THRESHOLD))
            if ((just_an_int > 100)&&(sensor_sonic.read()<SONIC_SENSOR_THRESHOLD_LONG_TOUCH)) // Putting an if instead of a while...
            {
                Serial.println("Detected long button touch");
                blink();
                flag_game_menu = 1;
                flag_game = 0;
                flag_game_started = 0;
                just_an_int = 0;
                flag_broadcast_selected_game = 1; // let others light know that I am in the menu now
                break;
            }
            delay(10);
        }

        /*if (flag_light_up == 0) // This to remove anime white dots 
        {
            turn_light_off();
        }*/

        if(random_wait_time_game2 == 0)
        {
            random_wait_time_game2 = random(10,150);
            Serial.print("Random wait time selected: ");
            Serial.println(random_wait_time_game2);
        }

        if (another_int > random_wait_time_game2)
        {
            Serial.print("Wait time is up!");
            random_wait_time_game2 = 0; // for next iteration of the loop, the random time will be computed again

            if (flag_light_up) // if light urned up, turn it down
            {
                turn_light_off();
                flag_light_up = 0;
            }
            else
            {
                light_colour = random(0,2); // choose between O and 1, as last member exclusive 
                Serial.print("Chosen colour: ");
                Serial.println(light_colour);

                if (light_colour ==0)
                {
                    turn_light_blue();
                }
                else
                {
                    turn_light_red();
                }
 
                flag_light_up = 1; // next time random time is passed, light will be down
            }

            another_int = 0; // reset the counter
        }

        delay(10); // avoid system to crash
    }


    ////////////////////////////////////////////////////////////////////////////////////////////////////
    ////////////////////////////////////////    GAME 3 END  //////////////////////////////////////////
    ////////////////////////////////////////////////////////////////////////////////////////////////////


    if (broadcast_register_flag)
    {
        broadcast_register();
        broadcast_register_flag = 0;
    }

    if (update_delta_time_display)
    {
        display.showNumberDec(delta_time.toInt(),true);
        update_delta_time_display = 0;
    }

    if (flag_broadcast_selected_game)
    {
        broadcast_selected_game();
        flag_broadcast_selected_game = 0;
    }

    if (flag_broadcast_count)
    {
        delay(200); // hack to avoid loss of web messages when 2 consecutives touches are close from each other
        broadcast_count();
        flag_broadcast_count = 0;
    }
    if (flag_update_count)
    {
        display.showNumberDec(count_game_1,true);
        flag_update_count =0;
    }
}
