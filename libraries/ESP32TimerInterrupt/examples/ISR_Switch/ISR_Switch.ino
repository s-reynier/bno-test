/****************************************************************************************************************************
   ISR_Switch.ino
   For ESP32 boards
   Written by Khoi Hoang

   Built by Khoi Hoang https://github.com/khoih-prog/ESP32TimerInterrupt
   Licensed under MIT license

   The ESP32 has two timer groups, each one with two general purpose hardware timers. All the timers are based on 64 bits
   counters and 16 bit prescalers. The timer counters can be configured to count up or down and support automatic reload
   and software reload. They can also generate alarms when they reach a specific value, defined by the software. The value
   of the counter can be read by the software program.

   Now even you use all these new 16 ISR-based timers,with their maximum interval practically unlimited (limited only by
   unsigned long miliseconds), you just consume only one ESP32 timer and avoid conflicting with other cores' tasks.
   The accuracy is nearly perfect compared to software timers. The most important feature is they're ISR-based timers
   Therefore, their executions are not blocked by bad-behaving functions / tasks.
   This important feature is absolutely necessary for mission-critical tasks.

   Based on SimpleTimer - A timer library for Arduino.
   Author: mromani@ottotecnica.com
   Copyright (c) 2010 OTTOTECNICA Italy

   Based on BlynkTimer.h
   Author: Volodymyr Shymanskyy

   Version: 1.1.1

   Version Modified By   Date      Comments
   ------- -----------  ---------- -----------
    1.0.0   K Hoang      23/11/2019 Initial coding
    1.0.1   K Hoang      27/11/2019 No v1.0.1. Bump up to 1.0.2 to match ESP8266_ISR_TimerInterupt library
    1.0.2   K.Hoang      03/12/2019 Permit up to 16 super-long-time, super-accurate ISR-based timers to avoid being blocked
    1.0.3   K.Hoang      17/05/2020 Restructure code. Add examples. Enhance README.
    1.1.0   K.Hoang      27/10/2020 Restore cpp code besides Impl.h code to use if Multiple-Definition linker error.
    1.1.1   K.Hoang      06/12/2020 Add Version String and Change_Interval example to show how to change TimerInterval
*****************************************************************************************************************************/
/* Notes:
   Special design is necessary to share data between interrupt code and the rest of your program.
   Variables usually need to be "volatile" types. Volatile tells the compiler to avoid optimizations that assume
   variable can not spontaneously change. Because your function may change variables while your program is using them,
   the compiler needs this hint. But volatile alone is often not enough.
   When accessing shared variables, usually interrupts must be disabled. Even with volatile,
   if the interrupt changes a multi-byte variable between a sequence of instructions, it can be read incorrectly.
   If your data is multiple variables, such as an array and a count, usually interrupts need to be disabled
   or the entire sequence of your code which accesses the data.

   ISR_Switch demontrates the use of ISR to avoid being blocked by other CPU-monopolizing task

   In this complex example: CPU is connecting to WiFi, Internet and finally Blynk service (https://docs.blynk.cc/)
   Many important tasks are fighting for limited CPU resource in this no-controlled single-tasking environment.
   In certain period, mission-critical tasks (you name it) could be deprived of CPU time and have no chance
   to be executed. This can lead to disastrous results at critical time.
   We hereby will use interrupt to detect whenever the SW is active, then switch ON/OFF a sample relay (lamp)
   We'll see this ISR-based operation will have highest priority, preempts all remaining tasks to assure its
   functionality.
*/

#ifndef ESP32
  #error This code is designed to run on ESP32 platform, not Arduino nor ESP8266! Please check your Tools->Board setting.
#endif

// These define's must be placed at the beginning before #include "ESP32TimerInterrupt.h"
// Don't define TIMER_INTERRUPT_DEBUG > 2. Only for special ISR debugging only. Can hang the system.
#define TIMER_INTERRUPT_DEBUG      0

#include "ESP32TimerInterrupt.h"

#define BLYNK_PRINT Serial
//#define BLYNK_DEBUG true

#include <WiFi.h>

#define USE_SSL     false

#if USE_SSL
  #include <BlynkSimpleEsp32_SSL.h>
  #define BLYNK_HARDWARE_PORT     9443
#else
  #include <BlynkSimpleEsp32.h>
  #define BLYNK_HARDWARE_PORT     8080
#endif

#define USE_LOCAL_SERVER    true

// If local server
#if USE_LOCAL_SERVER
  char blynk_server[]   = "account.duckdns.org";
  //char blynk_server[]   = "192.168.2.110";
#else
  char blynk_server[]   = "";
#endif

char auth[]     = "****";
char ssid[]     = "****";
char pass[]     = "****";

#define DEBOUNCE_TIME               25
#define LONG_BUTTON_PRESS_TIME_MS   100
#define DEBUG_ISR                   0

#define VPIN                        V1
#define LAMPSTATE_PIN               V2

//Blynk Color in format #RRGGBB
#define BLYNK_GREEN     "#23C48E"
#define BLYNK_RED       "#D3435C"

WidgetLED  LampStatus(LAMPSTATE_PIN);

volatile unsigned long  lastDebounceTime  = 0;
volatile bool           buttonPressed     = false;
volatile bool           alreadyTriggered  = false;

volatile bool LampState    = false;
volatile bool SwitchReset  = true;

 
#define RELAY_PIN     32            // Pin D32 mapped to pin GPIO32/ADC4/TOUCH9 of ESP32
#define BUTTON_PIN    33            // Pin D33 mapped to pin GPIO33/ADC5/TOUCH8 of ESP32

BlynkTimer Timer;

unsigned int myWiFiTimeout        =  3200L;  //  3.2s WiFi connection timeout   (WCT)
unsigned int buttonInterval       =  511L;  //   0.5s update button state

void IRAM_ATTR Falling();
void IRAM_ATTR Rising();

void IRAM_ATTR lightOn();
void IRAM_ATTR lightOff();
void IRAM_ATTR ButtonCheck();
void IRAM_ATTR ToggleRelay();

BLYNK_CONNECTED()
{
  LampStatus.on();
  Blynk.setProperty(LAMPSTATE_PIN, "color", LampState ? BLYNK_RED : BLYNK_GREEN );
  Blynk.syncVirtual(VPIN);
}

// Make this a autoreleased pushbutton
BLYNK_WRITE(VPIN)
{
  if (param.asInt())
    ToggleRelay();
}

void IRAM_ATTR Rising()
{
  unsigned long currentTime  = millis();
  unsigned long TimeDiff;

  TimeDiff = currentTime - lastDebounceTime;
  if ( digitalRead(BUTTON_PIN) && (TimeDiff > DEBOUNCE_TIME) )
  {
    buttonPressed = false;
    ButtonCheck();
    lastDebounceTime = currentTime;
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), Falling, FALLING);
  }
}

void IRAM_ATTR Falling()
{
  unsigned long currentTime  = millis();

  if ( !digitalRead(BUTTON_PIN) && (currentTime > lastDebounceTime + DEBOUNCE_TIME))
  {
    lastDebounceTime = currentTime;
    ButtonCheck();
    buttonPressed = true;
    attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), Rising, RISING);
  }
}

void heartBeatPrint(void)
{
  static int num = 1;

  if (Blynk.connected())
  {
    Serial.print("B");
  }
  else
  {
    Serial.print("F");
  }
  
  if (num == 80)
  {
    Serial.println();
    num = 1;
  }
  else if (num++ % 10 == 0)
  {
    Serial.print(" ");
  }
}

void checkButton()
{
  heartBeatPrint();

  if (LampState)
    Blynk.setProperty(LAMPSTATE_PIN, "color", BLYNK_RED);
  else
    Blynk.setProperty(LAMPSTATE_PIN, "color", BLYNK_GREEN);
}

void IRAM_ATTR ButtonCheck()
{
  boolean SwitchState = (digitalRead(BUTTON_PIN));

  if (!SwitchState && SwitchReset)
  {
    ToggleRelay();
    SwitchReset = false;
  }
  else if (SwitchState)
  {
    SwitchReset = true;
  }
}

void IRAM_ATTR ToggleRelay()
{
  if (LampState)
    lightOff();
  else
    lightOn();
}

void IRAM_ATTR lightOn()
{
  digitalWrite(RELAY_PIN, HIGH);
  LampState = true;
}

void IRAM_ATTR lightOff()
{
  digitalWrite(RELAY_PIN, LOW);
  LampState = false;
}

void setup()
{
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(BUTTON_PIN, INPUT_PULLUP);

  digitalWrite(RELAY_PIN, LOW);
    
  Serial.begin(115200);
  while (!Serial);

  Serial.println("\nStarting ISR_Switch on " + String(ARDUINO_BOARD));
  Serial.println(ESP32_TIMER_INTERRUPT_VERSION);
  Serial.printf("CPU Frequency = %ld MHz\n", F_CPU / 1000000);

  LampState = false;

  attachInterrupt(digitalPinToInterrupt(BUTTON_PIN), Falling, FALLING);

  unsigned long startWiFi = millis();

  WiFi.begin(ssid, pass);

  do
  {
    delay(200);
    if ( (WiFi.status() == WL_CONNECTED) || (millis() > startWiFi + myWiFiTimeout) )
      break;
  } while (WiFi.status() != WL_CONNECTED);

  Blynk.config(auth, blynk_server, BLYNK_HARDWARE_PORT);
  Blynk.connect();

  if (Blynk.connected())
    Serial.println("Blynk connected");
  else
    Serial.println("Blynk not connected yet");

  Timer.setInterval(buttonInterval, checkButton);
}

void loop()
{
  Blynk.run();
  Timer.run();
}
