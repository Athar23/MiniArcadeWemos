#include <Arduino.h>
#include <SPI.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include "Adafruit_SSD1306.h"
#include <ESP8266WiFi.h>
#include <PubSubClient.h>

#define OLED_RESET 0  // GPIO0
#define LOGO16_GLCD_HEIGHT 16
#define LOGO16_GLCD_WIDTH  16

WiFiClient espClient;
PubSubClient client(espClient);
unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE	(50)
char msg[MSG_BUFFER_SIZE];

//Wifi config
const char* ssid = "AndroidAPA1B6";
const char* password = "12345679";
const char* mqtt_server = "192.168.43.121";
//Ball
int x,y,radius=3;
int xSpeed,ySpeed;

//Player
float userX=0,userY=30;
float comX=62,comY=30;
float userSpeed=1.23,comSpeed=1.23;

//GUI
int border=9;//Upper border for Score
int maxHeight=47,maxWidth=63;//Screen Size
int padHeight=10,padWidth=2;//Pad Size
int gameHeight=maxHeight-border,
gameWidth=maxWidth-padWidth;//Game Border
int playerScore=0, comScore=0,maxScore=5;
String displayScore="Scores :";

//Display class
Adafruit_SSD1306 display(OLED_RESET);

#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2

void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }

  Serial.println();
  if ((char)payload[0] == 'U') {
    userY+=userSpeed;
    Serial.println("Up");
  }else if((char)payload[0] == 'D'){
    userY-=userSpeed;
    Serial.println("Down");
  }
  if(userY>=gameHeight)userY=gameHeight;
  if(userY<=border)userY=border;
}

void reconnect() {
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      client.subscribe("mini-arcade");
      Serial.println(client.subscribe("mini-arcade"));
      client.publish("mini-arcade", "hello");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void setup_wifi() {

  delay(10);
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3C (for the 64x48)
  display.display();
  delay(2000);
  display.drawPixel(10, 10, WHITE);
 // Show the display buffer on the hardware.
 // NOTE: You _must_ call display after making any drawing commands
 // to make them visible on the display hardware!
  display.display();
  delay(2000);
  //Clear the OLED screen
  display.clearDisplay();


  //ball position & speed initialization
  x=random(20,40);
  y=random(border+radius+5,53-5);
  xSpeed=1;
  ySpeed=1;

}

void loop() {
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  display.println(displayScore);
  // put your main code here, to run repeatedly:
  display.drawCircle(x, y, radius, WHITE);//draw Ball
  display.fillRect(userX, userY, padWidth, padHeight, WHITE);//draw Player Pad
  display.fillRect(comX, comY, padWidth, padHeight, WHITE);//draw COM Pad
  display.display();
  display.clearDisplay();

  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  //Check if ball is in the edge of game screen
  if(x+radius>gameWidth||x-radius<padWidth){
    xSpeed=-xSpeed;
  }
  if(y+radius>=maxHeight||y-radius<=border){
    ySpeed=-ySpeed;
  }
  //change the ball's position according to speed
  x+=xSpeed;
  y+=ySpeed;

  //move the COM pad with simple logic
  if(xSpeed){
    if(ySpeed>0) comY+=comSpeed;//if ball go down pad go down
    else comY-=comSpeed;//vice versa

    if(comY>=gameHeight)comY=gameHeight;//make sure pad doesnt go off screen
    if(comY<=border)comY=border;
  }

  //Check Score
  //COM score
  if((x-radius)<=(userX+1)){
    if(!(y+radius>=userY&&y-radius<=userY+padHeight)){
      comScore++;
      delay(1000);
      x=random(20,40);
      y=random(border+radius+5,53-5);
    }
  }
  //Player score
  if((comX+1)<=(x-radius)){
    if(!(y+radius>=comY&&y-radius<=comY+padHeight)){
      playerScore++;
      delay(1000);
      x=random(20,40);
      y=random(border+radius+5,53-5);
    }
  }
  displayScore=String(playerScore)+"      "+String(comScore);
}
