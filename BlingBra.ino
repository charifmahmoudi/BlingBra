#include <WiFi101.h>
#include <WiFiClient.h>
#include <WiFiServer.h>
#include <WiFiSSLClient.h>
#include <WiFiUdp.h>

#include <Adafruit_NeoPixel.h>
#ifdef __AVR__
#include <avr/power.h>
#endif

#include <SPI.h>
#include <WiFi101.h>

// NeoPixel pin
#define PIN            4

#define NUMPIXELS      2

char ssid[] = "charif";      //  your network SSID (name)
char pass[] = "mahmoudi";   // your network password
int keyIndex = 0;                 // your network key Index number (needed only for WEP)
int ledpin = 6;
bool val = true;

Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);
int status = WL_IDLE_STATUS;
WiFiServer server(80);

void setup() {
    Serial.begin(9600);      // initialize serial communication
    Serial.print("Start Serial ");
    pinMode(ledpin, OUTPUT);      // set the LED pin mode
    pixels.begin(); // This initializes the NeoPixel library.
    // Check for the presence of the shield
    Serial.print("WiFi101 shield: ");
    if (WiFi.status() == WL_NO_SHIELD) {
        Serial.println("NOT PRESENT");
        return; // don't continue
    }
    Serial.println("DETECTED");
    
    // attempt to connect to Wifi network:
    while ( status != WL_CONNECTED) {
        
        digitalWrite(ledpin, LOW);
        Serial.print("Attempting to connect to Network named: ");
        Serial.println(ssid);                   // print the network name (SSID);
        digitalWrite(ledpin, HIGH);
        // Connect to WPA/WPA2 network. Change this line if using open or WEP network:
        status = WiFi.begin(ssid, pass);
        // wait 10 seconds for connection:
        delay(10000);
    }
    server.begin();                           // start the web server on port 80
    printWifiStatus();                        // you're connected now, so print out the status
    digitalWrite(ledpin, HIGH);
}


void loop() {
    WiFiClient client = server.available();   // listen for incoming clients
    
    if (client) {                             // if you get a client,
        digitalWrite(ledpin, HIGH);
        Serial.println("new client");           // print a message out the serial port
        String currentLine = "";                // make a String to hold incoming data from the client
        while (client.connected()) {            // loop while the client's connected
            if (client.available()) {             // if there's bytes to read from the client,
                char c = client.read();             // read a byte, then
                Serial.write(c);                    // print it out the serial monitor
                if (c == '\n') {                    // if the byte is a newline character
                                       
                    // if the current line is blank, you got two newline characters in a row.
                    // that's the end of the client HTTP request, so send a response:
                    if (currentLine.length() != 0) {
                        if(currentLine.indexOf("GET") != -1){
                          if (currentLine.indexOf("/neo") != -1) {
                            sendStatus(currentLine,client);
                          }else {
                            sendPage(client);
                          }
                          break;
                        }
                        
                    }
                    else {      // if you got a newline, then clear currentLine:
                        currentLine = "";
                    }
                }
                else if (c != '\r') {    // if you got anything else but a carriage return character,
                    currentLine += c;      // add it to the end of the currentLine
                }
                
            }
            else{
              break;
            }
        }
        // close the connection:
        client.stop();
        Serial.println("client disonnected");
        digitalWrite(ledpin, LOW);
    }
}

void sendStatus(String currentLine, WiFiClient client){
  Serial.println("processing the url params from :");
  Serial.println(currentLine);
  int r = currentLine.indexOf("/");
      r = currentLine.indexOf("/", r + 1);
  int g = currentLine.indexOf("/", r + 1);
  int b = currentLine.indexOf("/", g + 1);
  int e = currentLine.indexOf(" ", b + 1);
  r = currentLine.substring(r + 1, g).toInt();
  g = currentLine.substring(g + 1, b).toInt();
  b = currentLine.substring(b + 1, e).toInt();
  
  Serial.println("r=" + String(r) + ", g=" + String(g) + ", b=" + String(b)); 
  
  for(int i=0;i<NUMPIXELS;i++){
      // pixels.Color takes RGB values, from 0,0,0 up to 255,255,255
      pixels.setPixelColor(i, pixels.Color(r,g,b));
      pixels.show(); // This sends the updated pixel color to the hardware.
  }
    // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
  // and a content-type so the client knows what's coming, then a blank line:
  client.println("HTTP/1.1 201 OK");
  client.println("Content-type:application/json");
  client.println("Connection: close");
  client.println();
    // the content of the HTTP response follows the header:
  client.print("{NeoPixelsStatus:updated,r:"+String(r)+",g:"+String(g)+",b:"+String(b)+"}");
  client.println();
}
void sendPage(WiFiClient client){
  Serial.println("Sending Main Page");
  // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
  // and a content-type so the client knows what's coming, then a blank line:
  client.println("HTTP/1.1 200 OK");
  client.println("Content-type:text/html");
  client.println("Connection: close");
  client.println();
  
  // the content of the HTTP response follows the header:
client.print("<!DOCTYPE html>\n");
client.print("<html>\n");
client.print("<head>\n");
client.print("  <meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">\n");
client.print("  <link href=\"http://code.jquery.com/mobile/1.3.2/jquery.mobile-1.3.2.min.css\" rel=\"stylesheet\" type=\"text/css\" />\n");
client.print("  <script src=\"http://code.jquery.com/jquery-1.9.1.min.js\"></script>\n");
client.print("  <script src=\"http://code.jquery.com/mobile/1.3.2/jquery.mobile-1.3.2.min.js\"></script>\n");
client.print("  <script src=\"http://jscolor.com/release/2.0/jscolor-2.0.4/jscolor.js\"></script>\n");
client.print("  <meta charset=utf-8 />\n");
client.print("  <title>Neo Bling Bra</title>\n");
client.print("  <script>\n");
client.print("\n");
client.print("    $(document).bind('pageinit', function() {\n");
client.print("      $(\"#neocolor\").on(\"change\", function() {\n");
client.print("        var hex = $.trim($(\"#neocolor\").val());\n");
client.print("        var rgburl = \"neo/\"+parseInt(hex.substring(0, 2), 16)+\"/\"+parseInt(hex.substring(2, 4), 16)+\"/\"+parseInt(hex.substring(4, 6), 16);\n");
client.print("        $.get(rgburl, function(data, status){\n");
client.print("          alert(\"Data: \" + data + \"Status: \" + status);\n");
client.print("        });\n");
client.print("        $(\"#footerH3 span\").text( rgburl );\n");
client.print("      });\n");
client.print("    });\n");
client.print("  </script>\n");
client.print("</head>\n");
client.print("<body>\n");
client.print("<div data-role=\"page\">\n");
client.print("  <div data-role=\"header\"><h1>Neo Bling Bra</h1></div>\n");
client.print("  <div data-role=\"content\">\n");
client.print("    <input class=\"jscolor\" id=\"neocolor\" value=\"99CC00\" />\n");
client.print("  </div>\n");
client.print("  <div data-role=\"footer\">\n");
client.print("    <h3 id=\"footerH3\" >Push color <span></span> to the Bra</h3>\n");
client.print("  </div>\n");
client.print("</div>\n");
client.print("</body>\n");
client.print("</html>\n");
  
  
  
  // The HTTP response ends with another blank line:
  client.println();
  // break out of the while loop:
}
void printWifiStatus() {
    // print the SSID of the network you're attached to:
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());
    
    // print your WiFi shield's IP address:
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
    
    // print the received signal strength:
    long rssi = WiFi.RSSI();
    Serial.print("signal strength (RSSI):");
    Serial.print(rssi);
    Serial.println(" dBm");
    // print where to go in a browser:
    Serial.print("To see this page in action, open a browser to http://");
    Serial.println(ip);
}
