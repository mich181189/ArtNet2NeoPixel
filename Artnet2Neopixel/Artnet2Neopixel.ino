#include <NeoPixelBus.h>
#include <WiFi.h>
#include <WiFiUdp.h>

#define PIN        12
#define NUMPIXELS (60*15)
const uint16_t UNIVERSE = 0;
const uint16_t CHANNEL = 1;

// WiFi network name and password:
const char * networkName = "SSID";
const char * networkPswd = "PASSWORD";

NeoPixelBus<NeoGrbFeature, Neo800KbpsMethod> strip(NUMPIXELS, PIN);

bool wifiConnected = false;
WiFiUDP udp;

RgbColor red(128, 0, 0);
RgbColor green(0, 128, 0);
RgbColor blue(0, 0, 128);

RgbColor off(0,0,0);

void WiFiEvent(WiFiEvent_t event){
    switch(event) {
      case SYSTEM_EVENT_STA_GOT_IP:
          udp.begin(6454);
          wifiConnected = true;
          Serial.println("Wifi Connected");
          // clear the strip, ready for packets
          strip.ClearTo(off);
          strip.Show();
          break;
      case SYSTEM_EVENT_STA_DISCONNECTED:
          Serial.println("WiFi lost connection");
          wifiConnected = false;
          break;
      default: break;
    }
}

void setup() {
  Serial.begin(115200);
  
  strip.Begin();
  strip.Show();

  // connect to wifi
  WiFi.disconnect(true);
  WiFi.onEvent(WiFiEvent);
  WiFi.begin(networkName, networkPswd);
}

void setStrip(const RgbColor &c)
{
  for(int i = 0; i < NUMPIXELS;++i)
  {
    strip.SetPixelColor(i,c);
  }
  strip.Show();
}

void parsePacket(char* pkt, size_t s)
{
  if(s < 8)
  {
    Serial.print("Got invalid packet! Size is ");
    Serial.println(s);
    return;
  }

  if(strncmp("Art-Net", pkt, 8) == 0)
  {
    uint16_t opcode = uint16_t(pkt[9] << 8) + pkt[8];
    if(opcode == 0x5000)
    {
      uint16_t universe = uint16_t(pkt[15] << 8) + pkt[14];
      uint16_t length = uint16_t(pkt[17] << 8) + pkt[16];
      if(universe == UNIVERSE && length <= (CHANNEL+2))
      {
        RgbColor col(pkt[18+(CHANNEL-1)], pkt[18+(CHANNEL-1)+1], pkt[18+(CHANNEL-1)+2]);
        setStrip(col);
      }
    }
  }
}

void loop() {
  if(wifiConnected)
  {
      size_t pktSize = udp.parsePacket();
      if(pktSize)
      {
        char* buffer = (char*)malloc(pktSize);
        udp.read(buffer, pktSize);
        parsePacket(buffer, pktSize);
        free(buffer);
      }
  }
  else 
  {
    setStrip(red);
    delay(500);
    setStrip(green);
    delay(500);
    setStrip(blue);
    delay(500); 
  }
}
