#include<ESP8266WiFi.h>
#include<SoftwareSerial.h>
#include <PubSubClient.h>
#include <time.h>
#include <TZ.h>
#include <FS.h>
#include <LittleFS.h>
#include <stdlib.h>
#include <CertStoreBearSSL.h>
#define MSG_BUFFER_SIZE	(100)

char *remoteValues[2];
int previous=0;
int counter=0;
int publishFlag=0;

BearSSL::WiFiClientSecure espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
char msg[MSG_BUFFER_SIZE];
int value = 0;
int retryCounter =0;
//SoftwareSerial dataSerial(D9, D10);  // Rx and Tx pins
//WiFi----------------------------------
const char* ssid="DebS1";
const char* pwd="Baku1234";
//MQTT-----------------------------------
const char* mqtt_server ="6aae6a3e4d20489eab946259ec102e0f.s1.eu.hivemq.cloud";

//BearSSL::CertStore certStore;

static const char ca_cert[] PROGMEM = R"EOF(
-----BEGIN CERTIFICATE-----
MIIFazCCA1OgAwIBAgIRAIIQz7DSQONZRGPgu2OCiwAwDQYJKoZIhvcNAQELBQAw
TzELMAkGA1UEBhMCVVMxKTAnBgNVBAoTIEludGVybmV0IFNlY3VyaXR5IFJlc2Vh
cmNoIEdyb3VwMRUwEwYDVQQDEwxJU1JHIFJvb3QgWDEwHhcNMTUwNjA0MTEwNDM4
WhcNMzUwNjA0MTEwNDM4WjBPMQswCQYDVQQGEwJVUzEpMCcGA1UEChMgSW50ZXJu
ZXQgU2VjdXJpdHkgUmVzZWFyY2ggR3JvdXAxFTATBgNVBAMTDElTUkcgUm9vdCBY
MTCCAiIwDQYJKoZIhvcNAQEBBQADggIPADCCAgoCggIBAK3oJHP0FDfzm54rVygc
h77ct984kIxuPOZXoHj3dcKi/vVqbvYATyjb3miGbESTtrFj/RQSa78f0uoxmyF+
0TM8ukj13Xnfs7j/EvEhmkvBioZxaUpmZmyPfjxwv60pIgbz5MDmgK7iS4+3mX6U
A5/TR5d8mUgjU+g4rk8Kb4Mu0UlXjIB0ttov0DiNewNwIRt18jA8+o+u3dpjq+sW
T8KOEUt+zwvo/7V3LvSye0rgTBIlDHCNAymg4VMk7BPZ7hm/ELNKjD+Jo2FR3qyH
B5T0Y3HsLuJvW5iB4YlcNHlsdu87kGJ55tukmi8mxdAQ4Q7e2RCOFvu396j3x+UC
B5iPNgiV5+I3lg02dZ77DnKxHZu8A/lJBdiB3QW0KtZB6awBdpUKD9jf1b0SHzUv
KBds0pjBqAlkd25HN7rOrFleaJ1/ctaJxQZBKT5ZPt0m9STJEadao0xAH0ahmbWn
OlFuhjuefXKnEgV4We0+UXgVCwOPjdAvBbI+e0ocS3MFEvzG6uBQE3xDk3SzynTn
jh8BCNAw1FtxNrQHusEwMFxIt4I7mKZ9YIqioymCzLq9gwQbooMDQaHWBfEbwrbw
qHyGO0aoSCqI3Haadr8faqU9GY/rOPNk3sgrDQoo//fb4hVC1CLQJ13hef4Y53CI
rU7m2Ys6xt0nUW7/vGT1M0NPAgMBAAGjQjBAMA4GA1UdDwEB/wQEAwIBBjAPBgNV
HRMBAf8EBTADAQH/MB0GA1UdDgQWBBR5tFnme7bl5AFzgAiIyBpY9umbbjANBgkq
hkiG9w0BAQsFAAOCAgEAVR9YqbyyqFDQDLHYGmkgJykIrGF1XIpu+ILlaS/V9lZL
ubhzEFnTIZd+50xx+7LSYK05qAvqFyFWhfFQDlnrzuBZ6brJFe+GnY+EgPbk6ZGQ
3BebYhtF8GaV0nxvwuo77x/Py9auJ/GpsMiu/X1+mvoiBOv/2X/qkSsisRcOj/KK
NFtY2PwByVS5uCbMiogziUwthDyC3+6WVwW6LLv3xLfHTjuCvjHIInNzktHCgKQ5
ORAzI4JMPJ+GslWYHb4phowim57iaztXOoJwTdwJx4nLCgdNbOhdjsnvzqvHu7Ur
TkXWStAmzOVyyghqpZXjFaH3pO3JLF+l+/+sKAIuvtd7u+Nxe5AW0wdeRlN8NwdC
jNPElpzVmbUq4JUagEiuTDkHzsxHpFKVK7q4+63SM1N95R1NbdWhscdCb+ZAJzVc
oyi3B43njTOQ5yOf+1CceWxG1bQVs5ZufpsMljq4Ui0/1lvh+wjChP4kqKOJ2qxq
4RgqsahDYVvTH9w7jXbyLeiNdd8XM2w9U/t7y0Ff/9yi0GE44Za4rF2LN9d11TPA
mRGunUHBcnWEvgJBQl9nJEiU0Zsnvgc/ubhPgXRR4Xq37Z0j4r7g1SgEEzwxA57d
emyPxgcYxn/eR44/KJ4EBs+lVDR3veyJm+kXQ99b21/+jh5Xos1AnX5iItreGCc=
-----END CERTIFICATE-----
)EOF";

int ledPin=13;
void setup() {
  pinMode(ledPin, OUTPUT);
  digitalWrite(ledPin, LOW);
  Serial.begin(115200);
  connectToWiFi();
  setClock();
  BearSSL::X509List *serverTrustedCA = new BearSSL::X509List(ca_cert);
  espClient.setTrustAnchors(serverTrustedCA);
  //dataSerial.begin(115200); // Same baud rate as aurdino
  
  client.setServer(mqtt_server, 8883);
  
}

void loop() {
  // put your main code here, to run repeatedly:
    //String msg = dataSerial.readStringUntil('\r');
    delay(500);
    if (!client.connected()) {
              Serial.print("Connection state: ");
              Serial.println(client.connected());
              reconnect();
           }
           
    char buffer[100]="";
    char msg[80] = "There is an obstacle detected @distance (cm): ";
    if(Serial.available()>0)
    {
      //char data = Serial.read();
      Serial.readBytesUntil('\n', buffer, 100);
      Serial.println(buffer);
      //delay(1000);

      if(counter==1)
      {
        
        remoteValues[counter]= buffer;
         publishFlag = (atoi(remoteValues[counter])==previous) ? 0 : 1;
         previous = atoi(remoteValues[counter]);
         Serial.println(publishFlag);
         if(publishFlag==1)
         {
          
           concatenateStrings(msg, remoteValues[counter]);
         
           Serial.println(msg); 
            
           client.publish("test/baku/mytopic", msg);
           msg[0]='\0'; 
         }
        
      }
      if(counter==0)
      {
       
       remoteValues[counter]= buffer;
       previous = atoi(remoteValues[counter]);
       concatenateStrings(msg, remoteValues[counter]);
         
       Serial.println(msg);
       msg[0]='\0';
       counter++;
      }

    }

    
    
    
    
}

void connectToWiFi()
{
   Serial.println();
   Serial.print("WiFi connecting to ");
   Serial.println(ssid);
   WiFi.begin(ssid, pwd);
   Serial.println();
   Serial.print("Connecting...");

   while(WiFi.status()!= WL_CONNECTED )
   {
    delay(500);
    Serial.print(".");
   }
   digitalWrite(ledPin, HIGH);
   Serial.println();

   Serial.println("WiFi connected successfully!!");
   Serial.print("NodeMCU IP Address: ");
   Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client - MyClient";
    // Attempt to connect
    if (client.connect(clientId.c_str(), "debarshi", "Baku1234.")) {
      Serial.println("Connected to MQTT cloud!!!!");
      // Once connected, publish an announcement...
      //client.publish("test/baku/mytopic", "hello");
      // ... and resubscribe
      //client.subscribe("test/baku/mytopic");
    } else {
      
      Serial.print("failed, rc=");
      Serial.print(client.state());
      //espClient.getLastSSLError(err_buf, sizeof(err_buf));
      //Serial.print("Error: ");
      //Serial.println(err_buf);
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(30000);
      retryCounter++;
      if(retryCounter>3)
      break;
    }
  }
}

void setClock()
{
  configTime(TZ_Asia_Kolkata, "in.pool.ntp.org", "time.nist.gov");

  Serial.print("Waiting for NTP time sync: ");
  time_t now = time(nullptr);
  while (now < 8 * 3600 * 2) {
    delay(500);
    Serial.print(".");
    now = time(nullptr);
  }
  Serial.println("");
  struct tm timeinfo;
  gmtime_r(&now, &timeinfo);
  Serial.print("Current time: ");
  Serial.print(asctime(&timeinfo));
}

void concatenateStrings(char* str1, char* str2) 
{ 
    // Navigate to the end of the first string 
    while (*str1) { 
        ++str1; 
    } 
  
    // Copy characters of the second string to the end of 
    // the first string 
    while (*str2) { 
        *str1++ = *str2++; 
    } 
  
    // Add the null-terminating character 
    *str1 = '\0'; 
} 
