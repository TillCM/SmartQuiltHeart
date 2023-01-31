
#define USE_ARDUINO_INTERRUPTS true  
#include <PulseSensorPlayground.h>

#include <WiFiEspClient.h>
#include <WiFiEspServer.h>
#include <SoftwareSerial.h>

#include <PulseSensorPlayground.h>     // Includes the PulseSensorPlayground Library.   

#include <SPI.h>
#include <WiFiEspUdp.h>
#define RX 10
#define TX 11 
  // Set-up low-level interrupts for most acurate BPM math.
//SoftwareSerial esp8266(RX,TX);


char ssid[] = "SmartQuilt"; //  your network SSID (name)
char pass[] = "CID3208till";    // your network password (use for WPA, or use as key for WEP)

int status = WL_IDLE_STATUS;
int temperature;
int knock =8;
int humidity;
int val=0;
int sensorVal;
int touchPin=9;
int reqCount = 0; 
int correct;
String json;
int ledpin = 7 ; // sets the LED @pin 7
int knockledpin = 10;
int touchpin = 9; // sets the KY-036 metal touch sensor @pin 9
int value ;         // defines the numeric variables as value

const int PulseWire = 10;       // PulseSensor PURPLE WIRE connected to ANALOG PIN 0
const int LED13 = 13;          // The on-board Arduino LED, close to PIN 13.
int Threshold = 550;           // Determine which Signal to "count as a beat" and which to ignore.
                               // Use the "Gettting Started Project" to fine-tune Threshold Value beyond default setting.
                               // Otherwise leave the default "550" value. 
                               
PulseSensorPlayground pulseSensor;  // Creates an instance of the PulseSensorPlayground object called "pulseSensor"


WiFiEspServer server(80);

RingBuffer buf(8);


void setup() 
{
  Serial.begin(9600);
  Serial1.begin(115200);
  pinMode (touchpin, INPUT) ; // sets the metal touch sensor as INPUT
  pinMode (ledpin, OUTPUT) ; // sets LED as the OUTPUT
  pinMode (knock, INPUT);
  pinMode (knockledpin,OUTPUT);


  pulseSensor.analogInput(PulseWire);   
  pulseSensor.blinkOnPulse(LED13);       //auto-magically blink Arduino's LED with heartbeat.
  pulseSensor.setThreshold(Threshold);

   if (pulseSensor.begin()) {
    Serial.println("We created a pulseSensor Object !");  //This prints one time at Arduino power-up,  or on Arduino reset.  
  }

  // Init variables and expose them to REST API


   WiFi.init(&Serial1);    // initialize ESP module
   while (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    delay(1000);
  }

  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    while (true); // don't continue
  }

  Serial.print("Attempting to start AP ");
  Serial.println(ssid);

  // uncomment these two lines if you want to set the IP address of the AP
  IPAddress localIp(192, 168, 8, 1);
 WiFi.configAP(localIp);
  
  // start access point
  status = WiFi.beginAP(ssid, 10, pass, ENC_TYPE_WPA2_PSK);

  Serial.println("Access point started");
  printWifiStatus();
  
  // start the web server on port 80
  server.begin();
  Serial.println("Server started");

  

  
}

void loop() 
{ 
  readSensor();
  readHeart();
  readKnock();

 WiFiEspClient client = server.available();  // listen for incoming clients
  
  if (client) {                               // if you get a client,
    Serial.println("New client");             // print a message out the serial port
    buf.init();                               // initialize the circular buffer
    while (client.connected()) {              // loop while the client's connected
      if (client.available()) {               // if there's bytes to read from the client,
        Serial.println("available");
        char c = client.read();               // read a byte, then
        buf.push(c);                          // push it to the ring buffer

        // you got two newline characters in a row
        // that's the end of the HTTP request, so send a response
        if (buf.endsWith("\r\n")) {
          delay(1000);
          sendHttpResponse(client);
          break;
        }
      }
    }
    
    // give the web browser time to receive the data
    delay(10);

    // close the connection
    client.stop();
    Serial.println("Client disconnected");
  }

  }  



int getSensorData(){
  val = digitalRead(touchPin); 
  if(val ==1){
     
  }
     return val;
}

void printWifiStatus()
{
  // print your WiFi shield's IP address
  IPAddress ip = WiFi.localIP();
  Serial.print("IP Address: ");
  Serial.println(ip);

  // print where to go in the browser
  Serial.println();
  Serial.print("To see this page in action, connect to ");
  Serial.print(ssid);
  Serial.print(" and open a browser to http://");
  Serial.println(ip);
  Serial.println();
}

void readSensor()
{

  value = digitalRead (touchpin) ; // reads the value of the touchpin
  if (value == HIGH)      // If the value is HIGH 
  {
    digitalWrite (ledpin, HIGH);   // It will turn the LED ON, indicating that the sensor has been triggered
    sensorVal++;
    Serial.print(sensorVal);
  }
  else        //otherwise
  {
    digitalWrite (ledpin, LOW);  // LED is turned off if sensor is not triggered
  }
  
}

void readKnock()
{
  value = digitalRead(knock);

  if (value== HIGH)
  {
    digitalWrite(knockledpin, HIGH);
    Serial.print("KNOCK KNOCK");
    
  }

  else
  {
    digitalWrite(knockledpin, LOW);
  }
}

void readHeart()
{
   int myBPM = pulseSensor.getBeatsPerMinute();  // Calls function on our pulseSensor object that returns BPM as an "int".
                                               // "myBPM" hold this BPM value now. 

if (pulseSensor.sawStartOfBeat()) {            // Constantly test to see if "a beat happened". 
 Serial.println("♥  A HeartBeat Happened ! "); // If test is "true", print a message "a heartbeat happened".
 Serial.print("BPM: ");                        // Print phrase "BPM: " 
 Serial.println(myBPM);                        // Print the value inside of myBPM. 
}

  delay(20);   }

void sendHttpResponse(WiFiEspClient client)
{
     
  client.print(
    "HTTP/1.1 200 OK\r\n"
    "Content-Type: application/json\r\n"
    "Connection: close\r\n"  // the connection will be closed after completion of the response
    "Refresh: 20\r\n"        // refresh the page automatically every 20 sec
    "\r\n");
  
 
  //delay(10000);
  client.print("{\n \"LearnShapes\":\n  {\"activityID\": \"001\",\n    \"acticityName\": \"shapes\",\n    \"date\":\"12/12/2021\",\n    \"correct\": \"8\",\n     \"timeOnTask\":\"5\"\n },\n \"LearnNumbers\":\n  {\"activityID\": \"002\",\n    \"acticityName\": \"numbers\",\n    \"date\":\"12/12/2021\",\n    \"correct\": \"5\",\n     \"timeOnTask\":\"1.5\"\n },\n \"MarchShapes\":\n  {\"activityID\": \"003\",\n    \"acticityName\": \"match\",\n    \"date\":\"12/12/2021\",\n    \"correct\": \+""sensorVal\,\n     \"timeOnTask\":\"3\"\n },\n \"Love\":\n  {\"activityID\": \"004\",\n    \"acticityName\": \"loves\",\n    \"date\":\"12/12/2021\",\n    \"correct\": \"8\",\n     \"timeOnTask\":\"5\"\n }\n\n}\n\n");
  client.print(sensorVal);

}
