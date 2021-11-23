#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>                                    //เรียกใช้ Library ของ Blynk เพื่อใช้งานร่วมกับบอร์ด 
extern "C" {
  #include <espnow.h>
}
WidgetLED LED_onBlynk1(V3);       //กำหนด Pin ให้ LED ใน Blynk เพื่อแสดงผลที่ V3
WidgetLCD LCD(V1);                //กำหนด Pin ให้ LCD ใน Blynk เพื่อแสดงผลที่ V1

char auth[] = "qXxTQkBNcVDbyFJJbQwhTy4Z6M3pMLVK";   //auth สำหรับเชื่อมต่อกับ Blynk
char ssid[] = "Jefyboy(2.4G)";                      //ชื่อ WIFI ที่ใช้เชื่อมต่อกัน
char pass[] = "0841852030";                         //รหัสผ่าน

typedef struct struct_message {
    int id;
    float x;
} struct_message;


struct_message myData;
struct_message board1;
struct_message boardsStruct[1] = {board1};


void OnDataRecv(uint8_t * mac_addr, uint8_t *incomingData, uint8_t len) 
{
  char macStr[18];
  Serial.print("Packet received from: ");
  snprintf(macStr, sizeof(macStr), "%02x:%02x:%02x:%02x:%02x:%02x",
           mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);
  Serial.println(macStr);
  memcpy(&myData, incomingData, sizeof(myData));
  Serial.printf("Board ID %u: %u bytes\n", myData.id, len);
  // Update the structures with the new incoming data
  boardsStruct[myData.id-1].x = myData.x;
  Serial.printf("FLOWRATE_1: %f \n", boardsStruct[myData.id-1].x);
  
}

void setup() 
{ 
  Serial.begin(115200);
  Blynk.begin(auth, ssid, pass);       //ตรวจสอบ auth,ssid,pass
  Blynk.run();                         //ให้ Blynk เริ่มต้นการทำงาน
  LCD.clear();                         //ล้างค่าหน้าจอ
  LCD.print(0,0,"System Ready ...");   //ปริ้นข้อความทางหน้าจอ LCD
  Blynk.notify("เชื่อมต่อระบบได้แล้ว");     //แจ้งเตือนเป็นข้อความ                            *************** แจ้งเตือน ****************
  WiFi.mode(WIFI_STA);                              // Set device as a Wi-Fi Station
  //WiFi.disconnect();
  if (esp_now_init() != 0)                          // Init ESP-NOW
  { 
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  
  esp_now_set_self_role(ESP_NOW_ROLE_SLAVE);        // Once ESPNow is successfully Init, we will register for recv CB to
  esp_now_register_recv_cb(OnDataRecv);            // get recv packer info

}

void loop()
{
  Blynk.virtualWrite(V2, boardsStruct[myData.id-1].x);                                              //Blynk แสดงค่าอัตราการไหลของน้ำผ่าน Widget แบบกราฟครึ่งวงกลม
  
  if (boardsStruct[myData.id-1].x > 0.5 && boardsStruct[myData.id-1].x < 4)
    {
      LED_onBlynk1.on();
      LCD.clear();                                                                  //ล้างค่าหน้าจอ
      LCD.print(0,0,"System Working..");                                            //ปริ้นข้อความทางหน้าจอ LCD
    }
    else if (boardsStruct[myData.id-1].x > 4 )                                                        //สร้างเงื่อนไขหากอัตราการไหลของน้ำมากกว่า 0.5 L/min
    {
      LCD.clear();
      LCD.print(2,0,"SENSOR 1");
      LCD.print(0,1,"Problem !!!");
      Blynk.notify("แถวที่ 1 สปริงเกอร์มีปัญหา !!!");                 
    }
    else
    { 
      LED_onBlynk1.off();
    }
  Serial.println();
}
