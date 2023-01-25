//Importing all the necessary libraries
#define USE_ARDUINO_INTERRUPTS false
#include <PulseSensorPlayground.h>
#include <WiFi.h>
#include <WiFiClient.h>
#include <WebServer.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>
#include <OneWire.h>
#include <DallasTemperature.h>

#define DHTPIN 18                                                               // Digital pin connected to the DHT sensor
#define DHTTYPE    DHT11                                                        //Definition of type of DHT sensor
DHT_Unified dht(DHTPIN, DHTTYPE);                                               // Initialize Unified DHT sensor.

#define OneWirePin 19                                                           //Data wire is plugged into GPIO 19(Pin : D19) on the ESP32. 
OneWire oneWire(OneWirePin);                                                    //DS18B20 probe on pin 19 (a 4.7K resistor is necessary)
DallasTemperature sensors(&oneWire);                                            //Pass our oneWire reference to Dallas Temperature.

PulseSensorPlayground pulseSensor;                                              // Creates an instance of the PulseSensorPlayground object called "pulseSensor"
const int PULSE_INPUT = 34;                                                     //Connected to the pulse sensor purple (signal) wire.
const int PULSE_BLINK = 2;                                                      //Pin 13 is the on-board LED, flash on each detected pulse
const int PULSE_FADE = 5;                                                       //PWM pin onnected to an LED that will smoothly fade with each pulse.
const int THRESHOLD = 550;                                                      // Adjust this number to avoid noise when idle
int myBPM =0;                                                                   //"myBPM" holds BPM value
byte samplesUntilReport;                                                        //number of samples remaining to read until we want to report a sample over the serial connection.
const byte SAMPLES_PER_SERIAL_SAMPLE = 10;                                      //report a sample value over the serial port only once every 20 milliseconds (10 samples)
int Threshold = 550;                                                            //Threshold value for beat/no beat, you can change to adjust specificity

double temp, BodyTemp, hum;                                                     //Variables associated with different sensors

const char* ssid = "2.4 G";                                         // Set your access point network credentials
const char* password = "Mm00276437";                               //(i.e. your wifi ID and passpharse)
 
WebServer server(80);                                                           //Creates the WebServer class object on Port number 80(HTTP port)
String text= "";                                                                //With this object, we can create a web page on Port 80(using HTTP protocol)
 
const String page PROGMEM = "<head>"
  " <script src=\"https://ajax.googleapis.com/ajax/libs/jquery/3.4.1/jquery.min.js\"></script>"   //Including jQuery from cache (using CDN) for speedy loading of our webpage
  "<!DOCTYPE html>"                                                             //Doc type is set to HTML
  "<html lang='en' >"                                                           //HTML tag with language specification
  "<head>"                                                                      //Head tag
  "<link href='https://fonts.googleapis.com/css?family=Open+Sans' rel='stylesheet' type='text/css'>"      //Stylesheet link from Google Font API for Open Sans 
  "<meta charset='UTF-8'>"                                                      //Metadata specifies the character encoding for the HTML document
  "<meta http-equiv='X-UA-Compatible' content='IE=edge'>"                       //Metadata to instruct Internet Explorer to use latest rendering web engine(edge). Can be set IE=5 for legacy browser
  "<meta name='viewport' content='width=device-width, initial-scale=1.0'>"      //Metadata to initialize viewport of the user specific device screem
  "<title>Health Monitoring System</title>"                                     //Webpage Title tag
  "<style>"                                                                     //Styling of Cards
  "* {box-sizing:border-box;}"                                                  //Allows to include padding and border in an element's total width and height
  "body{background-color: rgb(230, 235, 222);}"                                 //Background color is set to light gray. You can change this to any color by specifing the rgb color code
  ".column {float: left;width: 25%;padding: 0 10px;}"                           //Float four columns side by side 
  ".row {margin: 0 -5px;}"                                                      //Remove extra left and right margins, due to padding
  ".row:after {content: "";display: table;clear: both;}"                        //Ensures all the floating elements are cleared and element after it will render below it 
                                                                                //To make Responsive columns as per as your device.   
  "@media screen and (max-width: 600px) {"                                      //On screens that are 600px or less.
  ".column {width: 100%;display: block;margin-bottom: 20px;}}"                  //Style colunm to take 100% width and margin set to 20pixel.
  ".heading{text-align: center;color: rgb(44, 44, 44)}"                         //align html heading to center and set color to black.   
                                                                                //Style the counter cards
  ".card {margin-top: 40%;box-shadow: 0 20px 10px 0 rgba(0, 0, 0, 0.2);padding: 16px;padding-bottom: 15%;text-align: center ;border-top-right-radius: 30%;border-bottom-left-radius: 30%;background-color: rgb(155,193,188);color: white;}"
                                                                                //To style all cards 40% margin from top  , set shadow under cards with grey color , bottom margin of each cards is 15% , align text in center under each cards , roundness of corners of cards is 30% , set each cards color to sky blue and text color to black.  
  "</style>"                                                                    //closing of style tag.
  " </head>"                                                                    //closing of head tag.
  " <body>"                                                                     //opening of head tag. 
  "<div class='heading' >"                                                      //starting division and class for heading.
  "<h1>Patient Health Monitoring Dashboard</h1>"                                //heading of our project using h1 tag for heading and to set text size to 1.
  "<h3>Made With  ❤️ By ZARONE PROJECTS</h3>"                                  // sub heading of our project  using h3 tag for heading and to set text size to 3.
  "</div>"                                                                      //ending division for heading.

  //HTML code for displaying Room Temperature                                   
  "<div class='row'>"                                                          //division class row.
  "<div class='column'>"                                                       //division class column.
  "<div class='card'>"                                                         //division class card.
  "<h3>Room Temperature</h3>"                                                  //To Show Room Temprature as a heading into the first card. 
  "<img src='https://cdn-icons-png.flaticon.com/512/6074/6074032.png 'alt='roomtemp' width='45%'>" //source link to Display SVG icon of Room Temperature and set width of image to 45%.
  "<br>"                                                                       //tag to add break in line.
  "<h3 id=\"temp\">""</h3>"                                                    //to add celcius sign.                                                    
  "</div>"                                                                     //closing division class card.
  "</div>"                                                                     //closing division class colunm.

  //HTML code for displaying Humidity
  "<div class='column'>"                                                       //division class column.
  "<div class='card'>"                                                         //division class card.
  " <h3>Room Humidity</h3>"                                                    //To display Humidity as a heading into the Second card.
  "<img src='https://icon-library.com/images/humidity-icon/humidity-icon-7.jpg' alt='roomhumi' width='45%'>"          //source link to Display SVG icon of Humidity and set width of image to 45%.
  "<br>"                                                                       //tag to add break in line.
  "<h3 id=\"hum\">""</h3>"                                                     //to add % sign.    
  "</div>"                                                                     //closing division class card.
  "</div>"                                                                     //closing division class colunm.

  //HTML code for displaying Body Temperature
  "<div class='column'>"                                                        //division class column.
  "<div class='card'>"                                                          //division class card.
  "<h3>Body Temperature</h3>"                                                   //To show Body Temperature as a heading into the third card.
  "<img src='https://cdn.iconscout.com/icon/free/png-256/temperature-1575480-1331753.png' alt='bodytemp' width='45%'>"          //source link to Display SVG icon of Body Temperature and set width of image to 45%.
  "<br>"                                                                        //tag to add break in line.
  "<h3 id=\"BodyTemp\">""</h3>"                                                 //to add celcius sign.   
  " </div>"                                                                     //closing division class card.
  " </div>"                                                                     //closing division class colunm.
  //HTML code for displaying Heart beat per minute
  "<div class='column'>"                                                         //division class column.
  "<div class='card'>"                                                           //division class card.
  "<h3>Heart Rate</h3>"                                                          //To show Heart Rate as a heading into the fourth card.
  "<img src='https://cdn.iconscout.com/icon/free/png-256/heart-beat-21-1118106.png' alt='heartrate' width='45%'>"              //source link to Display SVG icon of Heart beat and set width of image to 45%.
  "<br>"                                                                         //tag to add break in line.
  "<h3 id=\"myBPM\">""</h3>"                                                     //to add BPM sign.   
  "</div>"                                                                       //closing division class card.
  "</div>"                                                                       //closing division class colunm.

            
  " <script>"
  " $(document).ready(function(){"                                           //Used to make function available when DOM(Document Object Model) is ready
  " setInterval(getData,1000);"                                              //Calls getData function after every 1 second (1000 ms)  
          
  " function getData(){"                                                     //Function getData
  " $.ajax({"                                                                //Ajax = Asynchronous JavaScript and XML, is used to fetch realtime data. This is AJAX function     
  "  type:\"GET\","                                                          //Loads data from a server using an AJAX HTTP GET request, used for requesting data from the server TYPE : GET/POST
  "  url:\"data\","                                                          //A string containing the URL to which the request is sent. Data to be sent to the server if GET function is used, which cannot have the entity body
  "  success: function(data){"                                               //This event is only called if the request was successful and returns the value send by the server(in our case ESP 32)
  "  var s = data.split(\'-\');"                                             //Data sent by our server was separated with '-' for different values which is stored in variable s 
  "  $('#temp').html(s[0]);"                                                 //Temperature
  "  $('#hum').html(s[1]);"                                                  //Humidity
  "  $('#BodyTemp').html(s[2]);"                                             //Body Temperature
  "  $('#myBPM').html(s[3]);"                                                //BPM
  "}"
  "}).done(function() {"                                                     //done function
  "  console.log('ok');"
  "})"
  "}"
  "});"
    
  "</script>"
  "</body>";
 
void setup(void) {
  Serial.begin(115200);                                                         //Initializing the Serial Monitor for recording reponse of ESP32
                                                                                //Use 115200 baud because that's what the Processing Sketch expects to read,
                                                                                //and because that speed provides about 11 bytes per millisecond. 
  // This segment directs the ESP32 to connect to Wi-Fi Hotspot.
  //Please select 2.4Ghz band for the connection as 5Ghz band is not supported by ESP32
  WiFi.mode(WIFI_STA);                                                          //ESP32 is set as Wi-Fi Station to connect to other networks(routers)
  WiFi.begin(ssid, password);                                                   //SSID and password is passed to ESP on which network you want to connect it.
  Serial.println("");                                                           //line break
  dht.begin();                                                                  //Initializing the DHT library
  sensors.begin();                                                              //Start up the Dallas temperature library
  sensor_t sensor;
  
  // Configure the PulseSensor manager.
  pulseSensor.analogInput(PULSE_INPUT);                                         //Gets input from pulse sensor 
  pulseSensor.blinkOnPulse(PULSE_BLINK);                                        //auto-magically blink ESP's on-board LED with heartbeat.
  pulseSensor.fadeOnPulse(PULSE_FADE);                                          //Initializing fade with beat that will smoothly fade with each pulse.
  pulseSensor.setSerial(Serial);                                                //Ouptput set to serial monitor
  pulseSensor.setThreshold(THRESHOLD);                                          //Sets the threshold level for pulse
  samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;                               // Skip the first SAMPLES_PER_SERIAL_SAMPLE in the loop().
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {                                       //This will print ERROR message till the ESP is connected to the specified network
    delay(500);                                                                 //Wait for 500ms before checking connection status
    Serial.print(".");                                                          //Print Waiting(dots) till connection is established
  }
  Serial.println("");                                                           //line break
  Serial.print("Connected to ");                                                //On successful connection to the specified network
  Serial.println(ssid);                                                         //It will print the name(ssid) of the network
  Serial.print("IP address: ");                                                 //and IP address on which our html page is hosted
  Serial.println(WiFi.localIP());                                               //Copy this IP address on the browser and hit run to get realtime update from ESP32(IP address of ESP32)

 if (!pulseSensor.begin()) {                                                    //Error Sign if pulseSensor initialization fails
    for(;;) {                                                                   //loop for blinking on-board led if pulse sensor initialization failed
      digitalWrite(PULSE_BLINK, LOW);                                           //Simple blink algo with delay of 50ms
      delay(50);
      digitalWrite(PULSE_BLINK, HIGH);                                     
      delay(50);
    } }
    /*
       PulseSensor initialization failed,
       likely because our Arduino platform interrupts
       aren't supported yet.
       If your Sketch hangs here, try changing USE_PS_INTERRUPT to true
       and remove sampling statement.   
    */
  
  server.on("/data", [](){                                                      //Client request Server for "data" every 1000ms which is handled and is "text" is send back to the client  
    sensors_event_t event;                                                      //function to encapsulate a specific sensor reading, called an 'event', and contains a data from the sensor from a specific moment in time.
    dht.temperature().getEvent(&event);                                         //read a new set of values from dht sensor for temperature, convert them to the appropriate SI units(°C) and scale, and then assign the results to a "event" object.
    temp =  (double)event.temperature;                                          //stores temperature data to "temp" from event temperature 
    dht.humidity().getEvent(&event);                                            //read a new set of values from dht sensor for humidity, convert them to the appropriate SI units(%) and scale, and then assign the results to a "event" object.
    hum =  (double)event.relative_humidity;                                     //stores humidity data to "hum" from event temperature
    sensors.requestTemperatures();                                              // Call sensors.requestTemperatures() to issue a global temperature and Requests to all devices on the bus
    BodyTemp = sensors.getTempCByIndex(0);                                      // Why "byIndex"? If you have more than one IC on the same bus. 0 refers to the first IC on the wire, i.e. DS18B20 
       
    
    text = (String)temp;                                                        //Store temp in string "test"
    text += " °C";                                                              //Along with °C sign as suffix
    text += "-";                                                                //Separated by '-' sign
    text += (String)hum;                                                        //Store humidity in string "test"
    text += " %";                                                               //Along with % sign as suffix
    text += "-";                                                                //Separated by '-' sign
    text += (String)BodyTemp;                                                   //Store temp in string "test"
    text += " °C";                                                              //Along with °C sign as suffix
    text += "-";                                                                //Separated by '-' sign
    text += (String)myBPM;                                                      //Store BPM in string "test"
    text += " BPM";                                                             //Along with BPM suffix
    
    Serial.println(text);                                                       //Print appended "text" on serial monitor
    server.send(200, "text/plain", text);                                       //Data as "text" is send to url : data(or function getData) which is called every 1000ms(or 1 second)
  });
 
  server.on("/", []() {                                                         //Server at root sends request to print the html webpage as "page" and is called once during execution of program
   server.send(200, "text/html", page);                                         //"page" is send to root to display the webpage 
  });
 
  server.begin();                                                              //Start server
  Serial.println("HTTP server started");                                       //Print message to denote server is successfully started 
}
 
void loop() {
  server.handleClient();                                                        //Handle client request
  if (pulseSensor.sawNewSample()) {                                             //Checking if finger is put on the pulse sensor
      if (--samplesUntilReport == (byte) 0) {                
      samplesUntilReport = SAMPLES_PER_SERIAL_SAMPLE;                           //send the latest Sample, we don't print every sample, because our baud rate won't support that much I/O.
      pulseSensor.outputSample();                                               //At about the beginning of every heartbeat,report the heart rate and inter-beat-interval.
      myBPM = pulseSensor.getBeatsPerMinute();                                  // Calls function on our pulseSensor object that returns BPM as an "int"
      Serial.print("BPM: ");                                                    // Print phrase "BPM: " 
      Serial.println(myBPM);                                                    // Print the value inside of myBPM.
     } }
}
