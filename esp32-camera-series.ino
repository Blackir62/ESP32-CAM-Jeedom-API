#include "WiFi.h"
#include "OneButton.h"
#include "freertos/event_groups.h"
#include "Wire.h"
#include "esp_camera.h"
#include "esp_wifi.h"
#include "HTTPClient.h"

/***************************************
    WiFi
 **************************************/
#define WIFI_SSID   "XXXXXXXXXXXXXX"
#define WIFI_PASSWD "XXXXXXXXXXXXXX"

/***************************************
    PinOUT
 **************************************/
#define PWDN_GPIO_NUM       26
#define RESET_GPIO_NUM      -1
#define XCLK_GPIO_NUM       32
#define SIOD_GPIO_NUM       13
#define SIOC_GPIO_NUM       12
#define Y9_GPIO_NUM         39
#define Y8_GPIO_NUM         36
#define Y7_GPIO_NUM         23
#define Y6_GPIO_NUM         18
#define Y5_GPIO_NUM         15
#define Y4_GPIO_NUM         4
#define Y3_GPIO_NUM         14
#define Y2_GPIO_NUM         5
#define VSYNC_GPIO_NUM      27
#define HREF_GPIO_NUM       25
#define PCLK_GPIO_NUM       19
#define AS312_PIN           33
#define BUTTON_1            34
#define I2C_SDA             21
#define I2C_SCL             22

/***************************************
    Jeedom Trigger
 **************************************/
const char* host  = "XXX.XXX.XXX.XXX";
const int   port  = 80;
String      url   = "/core/api/jeeApi.php?apikey=XXXXXXXXXXXXXXXXXXXX&type=camera&id=XXX&value=1";

void startCameraServer();
char buff[128];

HTTPClient http;

void setup() {
  Serial.begin(115200);
  pinMode(AS312_PIN, INPUT);

  // camera config
  camera_config_t config;
  config.ledc_channel = LEDC_CHANNEL_0;
  config.ledc_timer = LEDC_TIMER_0;
  config.pin_d0 = Y2_GPIO_NUM;
  config.pin_d1 = Y3_GPIO_NUM;
  config.pin_d2 = Y4_GPIO_NUM;
  config.pin_d3 = Y5_GPIO_NUM;
  config.pin_d4 = Y6_GPIO_NUM;
  config.pin_d5 = Y7_GPIO_NUM;
  config.pin_d6 = Y8_GPIO_NUM;
  config.pin_d7 = Y9_GPIO_NUM;
  config.pin_xclk = XCLK_GPIO_NUM;
  config.pin_pclk = PCLK_GPIO_NUM;
  config.pin_vsync = VSYNC_GPIO_NUM;
  config.pin_href = HREF_GPIO_NUM;
  config.pin_sscb_sda = SIOD_GPIO_NUM;
  config.pin_sscb_scl = SIOC_GPIO_NUM;
  config.pin_pwdn = PWDN_GPIO_NUM;
  config.pin_reset = RESET_GPIO_NUM;
  config.xclk_freq_hz = 20000000;
  config.pixel_format = PIXFORMAT_JPEG;
  //init with high specs to pre-allocate larger buffers
  config.frame_size = FRAMESIZE_UXGA;
  config.jpeg_quality = 10;
  config.fb_count = 2;

  // camera init
  esp_err_t err = esp_camera_init(&config);
  if (err != ESP_OK) {
    Serial.printf("Camera init Fail");
    while (1);
  }

  // drop down frame size for higher initial frame rate
  sensor_t *s = esp_camera_sensor_get();
  s->set_framesize(s, FRAMESIZE_VGA);
  s->set_vflip(s, 1);
  s->set_hmirror(s, 1);

  // connecting WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWD);
  Serial.print("WiFi connecting ");
  while (WiFi.status() != WL_CONNECTED) {
    Serial.print(".");
    delay(500);
  }
  Serial.println("");
  Serial.print("WiFi connected ");
  Serial.println(WiFi.localIP());

  startCameraServer();
  delay(50);
}

void loop() {
  if (WiFi.status() != WL_CONNECTED) {
    WiFi.begin(WIFI_SSID, WIFI_PASSWD);
    Serial.println("Wifi lost, reconnect...");
  }

  while (WiFi.status() == WL_CONNECTED) {
    if (digitalRead(AS312_PIN)) {
      Serial.println("AS312 Trigger");
      delay(100);
      SendToJeedom(url);
    }
    delay(3000);
  }
  delay(3000);
}

boolean SendToJeedom(String url) {
  Serial.print("Requesting URL: ");
  Serial.print(host);
  Serial.println(url);
  http.begin(host, port, url);
  int httpCode = http.GET();
  Serial.println("Closing connection");
  http.end();
}
