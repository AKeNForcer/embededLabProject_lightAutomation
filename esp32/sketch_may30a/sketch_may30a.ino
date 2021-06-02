#include <WiFi.h>
#include <PubSubClient.h>

#define RXD2 16
#define TXD2 17
#define LED_pin 12
#define LEDC_CHANNEL_0 0
#define LEDC_TIMER_BIT 8
#define LEDC_BASE_FREQ 5000

const char* ssid = "LEE2";
const char* password = "95979795";
const char* mqtt_server = "broker.netpie.io";
const int mqtt_port = 1883;
const char* mqtt_Client = "a2a029d2-0e78-42d7-8a91-b4b0eb2691f1";
const char* mqtt_username = "bV2WqjUhjnPz4kqZtJeMM84aEfbwiyF3";
const char* mqtt_password = ")nMV#WHPQ5HfZ7E!hXQ)seBRb5SxBYN~";

WiFiClient espClient;
PubSubClient client(espClient);
char msg[50];

int count_tmp = 0, ad_res_tmp = 0;
int count = 0, ad_res = 0;
int state = 0;

int upToDate = 1;

int setpoint = 0, mode_auto = 1, manual_level = 0, manual_level_temp = 0, stat_raise = 0;
float LED_level = 0;
String starting_word = "data ";

void sendData(){
  String json = "";
  json += "{\"mode_auto\": ";
  json += String(mode_auto);
  json += ", \"count\": ";
  json +=  String(count);
  json += "}";
  char send_json[100];
  json.toCharArray(send_json, 99);
  client.publish("@msg/data", send_json);
  Serial.println("Sent !"); 
}

void setup() {
    Serial.begin(115200);
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
    }
    Serial.println("");
    Serial.println("WiFi connected");
    Serial.println("IP address: ");
    Serial.println(WiFi.localIP());
    client.setServer(mqtt_server, mqtt_port);
    client.setCallback(callback);


    Serial2.begin(9600,SERIAL_8N1,RXD2,TXD2);
    ledcSetup(LEDC_CHANNEL_0, LEDC_BASE_FREQ, LEDC_TIMER_BIT);
    ledcAttachPin(LED_pin, LEDC_CHANNEL_0);
}

void resetData(){
  state = 0;
  count_tmp = 0, ad_res_tmp = 0;
}

void getSensorData(){
    // put your main code here, to run repeatedly:
  //Serial.print("SerialIn: ");
  if(Serial2.available()){
      while (Serial2.available()) {
        char ch = char(Serial2.read());
        if(state < 5){
          if(ch == starting_word[state]){
            state++;
          }
          else{
            resetData();
          }
        }
        else if(state == 5){
          if('0' <= ch && ch <= '9'){
            count_tmp *= 10;
            count_tmp += ch - '0';
          }
          else if(ch == ' '){
            state = 6;
          }
          else{
            resetData();
          }
        }
        else if(state == 6){
          if('0' <= ch && ch <= '9'){
            ad_res_tmp *= 10;
            ad_res_tmp += ch - '0';
          }
          else if(ch == ';'){
            if(count != count_tmp){
              upToDate = 0;
            }
            count = count_tmp;
            ad_res = ad_res_tmp;
            state = 0;
          }
          else{
            resetData();
          }
        }
      }
  }

  if(count > 0){
    if(mode_auto == 1){
      setpoint = (255*(ad_res - 500))/2000;
    }
    else{
      setpoint = (255 * manual_level) /100;
    }
  }
  else{
    setpoint = 0;
  }
  
  if(setpoint < 0) setpoint = 0;
  if(setpoint > 255) setpoint = 255;
  LED_level += 0.01 * (setpoint - LED_level);
  if(LED_level < 0) LED_level = 0;
  if(LED_level > 255) LED_level = 255;
  ledcWrite(LEDC_CHANNEL_0, (int)LED_level);

  if(stat_raise == 1000){
    stat_raise = 0;
    Serial.print("status: "); Serial.print(state);
    Serial.print(" Data: "); Serial.print(count); Serial.print(" "); Serial.print(ad_res); 
    Serial.print(" LED: "); Serial.print(setpoint); Serial.print(" "); Serial.print((int)LED_level);
    Serial.print(" Input: "); Serial.print(mode_auto); Serial.print(" "); Serial.println(manual_level);
  }
  else stat_raise++;

}

void loop() {
    while (!client.connected()) {
        Serial.print("Attempting MQTT connection…");
        if (client.connect(mqtt_Client, mqtt_username, mqtt_password)) {
            Serial.println("connected");
            client.subscribe("@msg/led");
        } else {
            Serial.print("failed, rc=");
            Serial.print(client.state());
            Serial.println("try again in 5 seconds");
            delay(5000);
            return;
        }
    }
    client.loop();

    getSensorData();

    if(upToDate == 0){
      upToDate = 1;
      sendData();
    }
}

void callback(char* topic, byte* payload, unsigned int length) {
    Serial.print("Message arrive [");
    Serial.print(topic);
    Serial.print("]");
    String message;
    for (int i = 0; i < length; i++) {
        message = message + (char)payload[i];
    }
    Serial.println(message);
    if (message == "GET") {
        sendData();
        return;
    }
    char ext[50];
    message.toCharArray(ext, 49);
    if (ext[0] == 'S' && ext[1] == 'E' && ext[2] == 'T') {
        if(ext[4] == '1'){
          mode_auto = 1;
        }
        else{
          mode_auto = 0;
        }
        manual_level_temp = 0;
        for(int i=6; ext[i]!='\0' && i < 49; i++){
          if(ext[i]==';'){
            manual_level = manual_level_temp;
            break;
          }
          manual_level_temp *= 10;
          manual_level_temp += ext[i] - '0';
          //Serial.println(ext[i]);
          //Serial.println(ext[i] - '0');
        }
    }
    else {
        //digitalWrite(LED1, 1);
        Serial.println("UNKNOW:");
        Serial.println(message);
    }
}
