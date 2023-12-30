#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <Arduino.h>
#include <Servo.h>
#include <LiquidCrystal_I2C.h>
#include <ESP8266HTTPClient.h>
#include <ArduinoJson.h>
#include <string.h>
// Servo myservo;
long previousMillis=0;
int moisture=0;
int threshold=25;
const int soilMoisturePin = A0;
long interval=2000;
String wifi_ip_str="";
WiFiClient wifi_cli;
HTTPClient client;
int responseCode;
/*Put WiFi SSID & Password*/
const char* ssid = "RishiRealme";   // Enter SSID here
const char* wifi_password = "Rishi@5578"; // Enter Password here
ESP8266WebServer server(80);
wl_status_t last_status;
LiquidCrystal_I2C lcd(0x27,16,2);

StaticJsonDocument<512> doc;
String updateWebpage(String s,bool t);
void handle_home();
void control_motor();
void handle_NotFound();
void check_data();
void lcd_init();
boolean open_door=false;
void lcd_init(){
  // Serial.println("WiFi connected..!");
  // Serial.print("Got IP: ");  
  // Serial.println(WiFi.localIP());
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print(WiFi.localIP());
}
void setup() {
  Serial.begin(9600);
  pinMode(D3, OUTPUT);
  digitalWrite(D3, LOW);
  delay(100);
  // myservo.attach(D3, 500, 2400);
  // myservo.write(0);
  lcd.init();
  lcd.clear();         
  lcd.backlight();
  // lcd.setContrast(128);
  Serial.println("Connecting to ");
  Serial.println(ssid);
  lcd.setCursor(0,0);
  lcd.print("Connecting ...");
  //connect to your local wi-fi network
  WiFi.begin(ssid, wifi_password);
  // //check NodeMCU is connected to Wi-fi network
  while (WiFi.status() != WL_CONNECTED) {
  delay(1000);
  Serial.print(".");
  }
  Serial.print("IP : ");
  Serial.println(WiFi.localIP());
  lcd_init();

  server.on("/", handle_home);
  server.on("/control_motor",HTTP_POST ,control_motor);
  server.on("/check_data",HTTP_POST ,check_data);
  server.onNotFound(handle_NotFound);

  server.begin();
  Serial.println("HTTP Server Started");
}
void loop() {
  server.handleClient();
  long currentMillis = millis();
  if (currentMillis - previousMillis >=interval){
    switch (WiFi.status()){
      case WL_NO_SSID_AVAIL:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("WiFi Not Found!");
        break;
      case WL_CONNECTED:
      if(last_status!=WL_CONNECTED){
        lcd_init();
      }
        break;
      case WL_CONNECT_FAILED:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Failed to connect");
        break;
      case WL_IDLE_STATUS:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("WiFi Idle!");
        break;
      case WL_SCAN_COMPLETED:
        lcd.clear();
        lcd.print("Scan Completed!");
        break;
      case WL_CONNECTION_LOST:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Connection lost!");
        break;
      case WL_WRONG_PASSWORD:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Wrong Password!");
        break;
      case WL_DISCONNECTED:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("Disconnected!");
        break;
      case WL_NO_SHIELD:
        lcd.clear();
        lcd.setCursor(0,0);
        lcd.print("No Shield!");
        break;
    }
    // Serial.printf("Connection status: %d\n", WiFi.status());
    // Serial.print("RRSI: ");
    // Serial.println(WiFi.RSSI());
    last_status=WiFi.status();
    previousMillis = currentMillis;
  }
  // Serial.println(map(analogRead(soilMoisturePin), 0, 1023, 100, 0));
  lcd.setCursor(0,1);
  moisture=map(analogRead(soilMoisturePin), 0, 1023, 100, 0);
  lcd.print("Moisture: "+(String)moisture+"% ");
  if(moisture<threshold){
    digitalWrite(D3, HIGH);
    delay(10000);
  }
  digitalWrite(D3, LOW);
    delay(100);
}

void check_data(){
  int moisture=map(analogRead(soilMoisturePin), 0, 1023, 100, 0);
  server.send(200,"text/plain","{\"temp\":34,\"RH\":45,\"moisture\":"+(String)moisture+"}");
}

void handle_home() {
  server.send(200, "text/html", updateWebpage("/",false)); 
}

void control_motor() {
  String time = server.arg("time");
  digitalWrite(D3, HIGH);
  server.send(200,"text/plain","{\"success\":true}");
  delay(time.toInt()*1000);
}

void handle_NotFound(){
  server.send(404, "text/plain", "Not found");
}

String updateWebpage(String s,bool t){
  String ptr = "<!DOCTYPE html> <html data-bs-theme=\"dark\">\n";
  ptr +="<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr +="<link href=\"https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/css/bootstrap.min.css\" rel=\"stylesheet\" integrity=\"sha384-T3c6CoIi6uLrA9TneNEoa7RxnatzjcDSCmG1MXxSR1GAsXEV/Dwwykc2MPK8M2HN\" crossorigin=\"anonymous\">";
  ptr +="<link rel=\"stylesheet\" href=\"https://cdn.jsdelivr.net/npm/bootstrap-icons@1.11.2/font/bootstrap-icons.min.css\">";
  ptr +="<title>Door Control</title>\n";
  ptr +="</head>\n";
  ptr +="<body class=\"container d-flex flex-column align-items-center\">\n";
  ptr +="<div class=\"alert\" style=\"display:none;\" id=\"main_alert\"></div>";
  ptr +="<h1 class=\"mb-4\">Crop Watering System</h1>\n";
  ptr +="<h2>Run Manually</h2>\n";
  ptr +="<form><fieldset class=\"d-flex flex-column align-items-center w-50\" style=\"min-width:22rem\">";
  ptr +="<input type=\"number\" class=\"form-control mb-2\" name=\"time\" id=\"time\" placeholder=\"Enter time in seconds\" required>";
  // ptr +="<input type=\"password\" class=\"form-control mb-2\" id=\"password\" name=\"password\" placeholder=\"password\" required>";
  ptr +="<button type=\"submit\" id=\"submit_btn\" class=\"btn btn-primary\">Water</button>";
  // ptr +="<button type=\"button\" id=\"close_door_btn\" class=\"mt-4 btn btn-success\" style=\"display:";
  // if(!open_door)
  // ptr +="none";
  // ptr +=";\">Close Door</button>";
  ptr +="</fieldset></form>";
  ptr +="<div class=\"d-flex flex-column mt-4 align-items-center\">";
  ptr +="<h2 class=\"text-center\">Live Readings</h2>";
  ptr +="<div class=\"d-flex flex-row justify-content-evenly align-items-center\" style=\"width:6rem; font-size:1.25rem;\"><i class=\"bi bi-thermometer-half\"></i><span id=\"temp\"></div>";
  ptr +="<div class=\"d-flex flex-row justify-content-evenly align-items-center\" style=\"width:6rem; font-size:1.25rem;\"><i class=\"bi bi-cloud-drizzle-fill\"></i><span id=\"RH\"></div>";
  ptr +="<div class=\"d-flex flex-row justify-content-evenly align-items-center\" style=\"width:6rem; font-size:1.25rem;\"><i class=\"bi bi-moisture\"></i><span id=\"moisture\"></div>";
  ptr +="</h2>";
  ptr +="<script src=\"https://cdn.jsdelivr.net/npm/bootstrap@5.3.2/dist/js/bootstrap.bundle.min.js\" integrity=\"sha384-C6RzsynM9kWDrMNeT87bh95OGNyZPhcTNXj1NW7RuBCsyN/o0jlpcV8Qyq46cDfL\" crossorigin=\"anonymous\"></script>";
  ptr +="<script>function show_alert(type,message){alert=document.getElementById('main_alert'); alert.classList='alert alert-'+type; alert.innerText=message; alert.style.display=''; setTimeout(()=>{document.getElementById('main_alert').style.display='none'},5000)}";
  ptr +="function check_data(){const xhr=new XMLHttpRequest;xhr.open(\"POST\",\"/check_data\",true);xhr.setRequestHeader(\"Content-Type\",\"application/x-www-form-urlencoded\");xhr.onload=function(){const res=JSON.parse(this.responseText); document.getElementById('temp').innerHTML=res.temp+\" C\"; document.getElementById('RH').innerHTML=res.RH+\" %\"; document.getElementById('moisture').innerHTML=res.moisture+\" %\"; check_data();};xhr.send();} check_data();";
  ptr +="document.querySelector('form').addEventListener('submit',function (e){e.preventDefault(); document.getElementById('submit_btn').innerHTML='<span class=\"spinner-border spinner-border-sm\" aria-hidden=\"true\">Watering ....</span><span class=\"visually-hidden\" role=\"status\">Watering...</span>';document.querySelector('fieldset').setAttribute('disabled',true);const xhr=new XMLHttpRequest;xhr.open(\"POST\",\"/control_motor\",true);xhr.setRequestHeader(\"Content-Type\",\"application/x-www-form-urlencoded\");xhr.onload=function (){setTimeout(()=>{document.getElementById('submit_btn').innerHTML='Water';document.querySelector('fieldset').removeAttribute('disabled'); show_alert(\"success\",\"Plant Watered\");},document.getElementById('time').value*1000)};xhr.send(\"time=\"+document.getElementById('time').value);})</script>";
  ptr +="</body>\n";
  ptr +="</html>\n";
  return ptr;
}