#include <ESP8266WiFi.h>                                                  //ใช้ Library เครือข่ายไร้สาย
extern "C" {                                                              
#include <espnow.h>                                                       //ใช้ Library ในการเชื่อมต่อแบบ Mac address
}
#define SENSOR1 D3                                                        //กำหนด Pin ให้เซนเซอร์ที่ขา D3 
uint8_t broadcastAddress[] = {0xE8, 0xDB, 0x84, 0xDD, 0x6B, 0x06};        //Mac address ของบอร์ดขาส่ง

long previousMillis1 = 0;                                                 //กำหนดตัวแปรรับค่าการหมุนของใบพัดในรอบถัดไป = 0
volatile byte pulseCount1;                                                //กำหนดตัวแปรรับค่าการหมุนของใบพัด
byte pulse1Sec1 = 0;                                                      //กำหนดตัวแปรรับค่าใบพัดในเวลา 1 วินาที = 0
float flowRate1;                                                          //กำหนดตัวแปรรับค่าอัตราการไหลของน้ำ

void IRAM_ATTR pulseCounter1()                                            //สร้างฟังก์ชั่นนับค่าการหมุนของใบพัด
{
  pulseCount1++;
}

typedef struct struct_message {                                           //กำหนดชุดข้อมูล
  int id;                                                                 //กำหนดหมายเลขของบอร์ด Client ที่ทำการส่งค่าไปยัง Server
  float x;                                                                //ค่า Flowrate 
} struct_message; 


struct_message myData;                                                    //กำหนดชื่อชุดข้อมูล
struct_message board1;                                                    //กำหนดชื่อบอร์ด Client 
struct_message boardsStruct[1] = {board1};                                //กำหนดปริมาณของบอร์ด Client ที่ใช้ส่งไปยัง Server


void OnDataRecv(uint8_t * mac_addr, uint8_t *incomingData, uint8_t len)   //ฟังก์ชั่นในการส่งค่าไปยัง Server
{
  char macStr[18];                                                        //กำหนดขนาดของ Mac address
  Serial.print("Packet received from: ");                                 //ปริ้นข้อความ
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",       //Size ของ Mac address
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);       //กำหนดตำแหน่งของ Mac address
  Serial.println(macStr);                                                 //ปริ้น Mac address
  memcpy(&myData, incomingData, sizeof(myData));                          //จดจำค่า Mac address
  Serial.printf("Board ID %u: %u bytes\n", myData.id, len);               //ปริ้น Mac address
  boardsStruct[myData.id - 1].x = myData.x;                               //ส่งค่า Flowrate ไปยัง Server
  Serial.printf("humidity: %f \n", boardsStruct[myData.id - 1].x);        //ปริ้นค่า Flowrate
  Serial.println();                                                       //เว้นช่องว่าง
}

unsigned long lastTime = 0;                                               //กำหนดเวลาเริ่มต้น
unsigned long timerDelay = 10000;                                         //Delay

void setup() 
{
  Serial.begin(115200);
  pinMode(SENSOR1, INPUT_PULLUP);                                                      //กำหนดให้รับค่าจากเซนเซอร์
  WiFi.mode(WIFI_STA);                                                                 //เข้า Mode การส่งค่าแบบไวไฟจำลองหรือส่งแบบ Mac address
  //WiFi.disconnect();                                     
  pulseCount1 = 0;                                                                     //กำหนดค่าจำนวนการหมุนของใบพัดให้เริ่มต้นที่ 0
  flowRate1 = 0.0;                                                                     //กำหนดให้ค่าอัตราการไหลของน้ำเริ่มต้นที่ 0
  previousMillis1 = 0;                                                                 //กำหนดค่าการหมุนของใบพัดในรอบถัดไปเริ่มต้นที่ 0
  attachInterrupt(digitalPinToInterrupt(SENSOR1), pulseCounter1, FALLING);             //รับค่าจากเซนเซอร์1,ค่าการหมุน,ค่าอนาล็อก
  if (esp_now_init() != 0) 
  {
    Serial.println("Error initializing ESP-NOW");
    return;
  }                                                 
  esp_now_set_self_role(ESP_NOW_ROLE_CONTROLLER);                       // Set ESP-NOW role // Once ESPNow is successfully init, we will register for Send CB to                                                                                                       
  esp_now_register_send_cb(OnDataSent);                                 // get the status of Trasnmitted packet           
  esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_SLAVE, 1, NULL, 0);   // Register peer  // Once ESPNow is successfully Init, we will register for recv CB to                                                                  
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);                            
  esp_now_register_recv_cb(OnDataRecv);                                 // get recv packer info
}

void loop() 
{
  if (millis() - previousMillis1 > 1000)                                             //เมื่อการนับครบ 1 วินาที
  {
    pulse1Sec1 = pulseCount1;                                                       //ค่าการหมุนใน 1 วิ = จำนวนการหมุนของใบพัด
    pulseCount1 = 0;                                                                //จำนวนการหมุน = 0 (เพื่อเริ่มนับใหม่ในรอบเวลา 1 วินาทีถัดไป)
    flowRate1 = ((1000.0 / (millis() - previousMillis1)) * pulse1Sec1) / 7.5;       //จากสูตรหาอัตราการไหลของน้ำ Q=AV
    previousMillis1 = millis();                                                     //จดจำค่าการหมุนของรอบที่ผ่านมา
    Serial.print("Flow rate1 :");                                                   //ปริ้นข้อความ Flow rate
    Serial.print(int(flowRate1));                                                   //ปริ้นค่าอัตราการไหลของน้ำ
    Serial.print(" L/min");                                                         //ปริ้น L/min
    Serial.print("\n");                                                             //ขึ้นบรรทัดใหม่
  }
  
  myData.id = 1;
  myData.x = flowRate1;
  esp_now_send(0, (uint8_t *) &myData, sizeof(myData));                             // Send message via ESP-NOW
}

void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) 
{
  Serial.print("\r\nLast Packet Send Status: ");
  if (sendStatus == 0) 
  {
    Serial.println("Delivery success");
  }
  else 
  {
    Serial.println("Delivery fail");
  }
}
