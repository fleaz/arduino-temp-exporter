/**
 * Ethernet shield attached to pins 10, 11, 12, 13
 * Analog inputs attached to pins A0 through A5 (optional)
 */

#include <SPI.h>
#include <Ethernet.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define ONE_WIRE_BUS 2
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

float temp;
char mac_string[20];

// Read sensor every x ms
const int sensor_delay = 15000;


byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};

// StaticIP (1) or DHCP (0)
#define STATIC 1

#if STATIC
// MAC address (printed on a sticker on the shield)
IPAddress ip(192, 168, 1, 200);
#endif

EthernetServer server(80); // (port 80 is default for HTTP)

void setup()
{
    Serial.println("Start");
    // Convert MAC to nicely formated string
    sprintf(mac_string, "%x:%x:%x:%x:%x:%x", mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);

    Serial.begin(9600);
    #if STATIC
    Serial.println("COnfiguring staticIP:" + String(Ethernet.localIP()));
    Ethernet.begin(mac, ip);
    #else
    Serial.println("Configuring with DHCP");
    Ethernet.begin(mac);
    Serial.println("Got the following IP" + String(Ethernet.localIP()));
    #endif

    // Initially fill varieble with a value
    Serial.println("Initialization of the ds18b20 sensor");
    sensors.begin();
    sensors.requestTemperatures();
    temp = sensors.getTempCByIndex(0);

    Serial.println("Starting webserver");
    server.begin();
}

void loop()
{
  // Update sensor only every (sensor_delay/1000) seconds
  if(millis() % sensor_delay == 0)
  {
    sensors.requestTemperatures();
    temp = sensors.getTempCByIndex(0);
    Serial.println("sensor polled");
  }
    // listen for incoming clients
    EthernetClient client = server.available();

    if (client) {
        Serial.println("New client");
        boolean currentLineIsBlank = true; // an http request ends with a blank line

        while (client.connected()) {
            if (client.available()) {
                char c = client.read();
                Serial.write(c);

                // if you've gotten to the end of the line (received a newline character) and
                // the line is blank, the http request has ended, so you can send a reply.
                if (c == '\n' && currentLineIsBlank) {
                    // send a standard http response header
                    client.println("HTTP/1.1 200 OK");
                    client.println("Content-Type: text/html");
                    client.println("Connection: close");  // the connection will be closed after completion of the response
                    client.println();
                    client.print("arduino_temperature{sensor=\"ds18b20\",mac=\"" + String(mac_string) + "\"} " + String(temp) + "\n");
                    break;
                }

                if (c == '\n') {
                    currentLineIsBlank = true; // you're starting a new line
                } else if (c != '\r') {
                    currentLineIsBlank = false; // you've gotten a character on the current line
                }
            }
        }

        // give the web browser time to receive the data
        delay(100); // 1?

        client.stop();
        Serial.println("disconnected.");
    }
}

