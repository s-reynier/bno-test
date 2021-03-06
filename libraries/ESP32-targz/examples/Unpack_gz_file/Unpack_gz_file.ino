#include <SPIFFS.h>
#include <ESP32-targz.h>

// progress callback, leave empty for less console output
void myNullProgressCallback( uint8_t progress ) {
  // printf("Progress: %d", progress );
}
// error/warning/info logger, leave empty for less console output
void myNullLogger(const char* format, ...) {
  //va_list args;
  //va_start(args, format);
  //vprintf(format, args);
  //va_end(args);
}


void setup() {

  Serial.begin( 115200 );
  Serial.printf("Initializing SPIFFS...\n");

  if (!SPIFFS.begin(false)) {
     Serial.printf("SPIFFS Mount Failed\n");
  }
  else {
    Serial.printf("SPIFFS Mount Successful\n");
  }

  //setProgressCallback( myNullProgressCallback );
  //setLoggerCallback( myNullLogger );

  // extract content from gz file
  gzExpander(SPIFFS, "/index_html.gz", SPIFFS, "/index.html");

  gzExpander(SPIFFS, "/tbz.gz", SPIFFS, "/tbz.jpg");

  tarGzListDir( SPIFFS, "/");

}


void loop() {

}
