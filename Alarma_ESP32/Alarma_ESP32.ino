
//-------------------------------------------------------ALARM-------------------------------------------------------\\

// Definimos las librerias a usar en el proyecto
#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>
#include <ArduinoJson.h>

const char* ssid = "S20";
const char* password = "88888888";

#define BOTtoken "5051918205:AAAAiM95jUzFCjlweEA2IXbh3q8sFx_42ZM"
#define CHAT_ID "769999985"

#ifdef ESP8266
X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// Revisa si hay mensajes cada 1 segundo
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

//Entradas teclado
const int CLK = 34;
const int Dato_1 = 35;
const int Dato_2 = 36;


//Entradas sensores
const int Deposito = 13; //no tiene pull up
const int PatioFrente = 14;
const int PatioTrasero = 16;
const int Comedor = 17;
const int Quincho = 18;
const int Lavadero = 19;
const int Living = 21;
const int Taller = 22;
const int Pieza = 23;
const int Bano = 25;  //no tiene pull up

//salidas
const int Detonador = 26;
const int Sirena = 27;
const int ledPin = 32;


bool ledState = HIGH;
bool notificacion = LOW;
bool onn = HIGH;

// Cuando recibe un mensaje

void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i = 0; i < numNewMessages; i++)
  {
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID)
    {
      bot.sendMessage(chat_id, "USUARIO NO AUTORIZADO", "");
      continue;
    }

    String text = bot.messages[i].text;
    Serial.println(text);

    String from_name = bot.messages[i].from_name;

    if (text == "/start") {
      String welcome = "BIENVENIDO, " + from_name + ".\n";
      welcome += "USE LOS SIGUIENTES COMANDOS PARA CONTROLAR SU ALARMA.\n\n";
      welcome += "/activar PARA ACTIVAR LA ALARMA \n";
      welcome += "/desactivar PARA APAGARLA \n";
      welcome += "/estado PARA SABER EL ESTADO DE LA ALARMA \n";
      bot.sendMessage(chat_id, welcome, "");
    }

    if (text == "/activar") {
      bot.sendMessage(chat_id, "PRENDIDO", "");
      ledState = HIGH;
      onn = HIGH;
      digitalWrite(ledPin, ledState);
    }

    if (text == "/desactivar") {
      bot.sendMessage(chat_id, "APAGADO", "");
      ledState = LOW;
      onn = LOW;
      notificacion = LOW;
      digitalWrite(Sirena, LOW);
      digitalWrite(ledPin, ledState);
    }

    if (text == "/estado") {
      if (digitalRead(ledPin)) {
        bot.sendMessage(chat_id, "LA ALARMA ESTA ACTIVADA", "");
      }
      else {
        bot.sendMessage(chat_id, "LA ALARMA ESTA DESACTIVADA", "");
      }
    }
  }
}

void IRAM_ATTR P() {
  if (notificacion == LOW) {
    if (onn == HIGH) {
      notificacion = HIGH;
      digitalWrite(Detonador, HIGH);
      bot.sendMessage(CHAT_ID, "----SE DETECTO MOVIMIENTO EN EL PATIO DEL FRENTE----", "");
      digitalWrite(Sirena, HIGH);
    }
  }
}

void setup() {
  Serial.begin(115200);

#ifdef ESP8266
  configTime(0, 0, "pool.ntp.org");
  client.setTrustAnchors(&cert);
#endif

  pinMode(ledPin, OUTPUT);
  pinMode(Sirena, OUTPUT);
  pinMode(Detonador, OUTPUT);
  pinMode(PatioFrente, INPUT_PULLUP);
  pinMode(PatioTrasero, INPUT_PULLUP);
  pinMode(Comedor, INPUT_PULLUP);
  pinMode(Quincho, INPUT_PULLUP);
  pinMode(Lavadero, INPUT_PULLUP);
  pinMode(Living, INPUT_PULLUP);
  pinMode(Pieza, INPUT_PULLUP);
  pinMode(Deposito, INPUT);
  pinMode(Bano, INPUT);

  attachInterrupt(PatioFrente, P, RISING);

  digitalWrite(ledPin, ledState);

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);

#ifdef ESP32
  client.setCACert(TELEGRAM_CERTIFICATE_ROOT);
#endif

  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("CONECTANDO A SU RED WI-FI..");
  }
  Serial.println(WiFi.localIP());
  bot.sendMessage(CHAT_ID, "----LA ALARMA ESTA CONECTADA----", "");
}

void loop() {

  if ((millis()) > (lastTimeBotRan + botRequestDelay))  {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    while (numNewMessages) {
      // Serial.println("RESPUESTA OBTENIDA");
      handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }
}
