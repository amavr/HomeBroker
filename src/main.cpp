#include <Arduino.h>

#include <SubList.h>
#include <TopicList.h>
#include <EventBroker.h>
#include <WiFiController.h>
#include <UDPRequest.h>
#include <ATools.h>
#include <GParser.h>

EventBroker broker;
WiFiController ctrl;

#include "EspNowConnector.h"
EspNowConnector conn;

uint16_t sizeEEPROM = 512;
uint8_t gateMac[6];
bool gateErr = false;
uint32_t last_time;

// u8 *mac_addr, u8 *data, u8 len
void OnDataRecv(uint8_t *mac, uint8_t *data, uint8_t len)
{
    data[len] = 0;
    char text[len + 1];
    char rest[len];
    ATools::normalize((char *)data, text);
    Serial.println(text);

    // subscription command ("sub <topic>")
    if (ATools::isCmd("sub ", text, rest))
    {
        GParser data(rest, ' ');
        int count = data.split();
        if (count == 1)
        {
            // со всеми подписчиками устанавливается связь
            conn.pair(mac);

            char addr[18];
            ATools::macToChars(mac, addr);
            broker.subscribe(addr, rest);
        }
    }
    // subscription command ("pub <topic>:<any data>")
    // rest = "<topic>:<any data>"
    else if (ATools::isCmd("pub ", text, rest))
    {
        // отделить данные от топика
        GParser data(rest, ':');
        int count = data.split();
        if (count == 2)
        {
            // опубликовать данные для топика
            SubList *subs = broker.publish(data[0], data[1]);
            int count = subs->Count();
            // получить подписчиков
            Serial.printf("Found %d subsribers\n", count);
            if (count > 0)
            {
                char *ids[count];
                subs->fill(ids);
                // uint8_t mac[6];
                // разослать данные топика подписчикам
                for (int i = 0; i < count; i++)
                {
                    Serial.printf("Send to %s\n", ids[i]);
                    // "<topic>:<any data>"
                    conn.send(ids[i], (char *)(text + 4));
                }
                for (int i = 0; i < count; i++)
                {
                    free(ids[i]);
                }
            }
        }
    }
    else if(ATools::isCmd("/state", text, rest))
    {
        
    }
}

void OnDataSent(uint8_t *mac, uint8_t status)
{
    if(memcmp(mac, gateMac, 6)== 0)
    {
        gateErr = status != 0;
    }
    Serial.printf("%s\n", status == 0 ? "success" : "fail");
}

void OnSubscriber(const char *subMac)
{
    conn.pair(subMac);
}

void OnTopic(Topic *topic)
{
    topic->subscribers->forEach(OnSubscriber);
}

void initGate()
{
    // Признак первого запуска
    bool isFirstTime = EEPROM[0] != 0x22;
    Serial.printf("isFirstTime: %d\n", isFirstTime);

    // подключение к WiFi
    ctrl.connect(isFirstTime);

    // после инициализации подключения и старта информера
    // EEPROM проинициализирована, поэтому сброс флага первого запуска
    if (isFirstTime)
    {
        EEPROM[0] = 0x22;
        EEPROM.commit();
    }


    char sMac[18];
    // регистрация в шлюзе
    setBrokerAddr();
    Serial.println("Registrated in Gate");

    // регистрация в шлюзе
    Serial.print("Request Gate mac address");
    while (!getGateAddr(sMac))
    {
        Serial.print(".");
        delay(500);
    }
    Serial.printf("done. %s\n", sMac);
    ATools::macToBytes(sMac, gateMac);

    WiFi.disconnect();
    Serial.println("WiFi disconnected");
}

void setup()
{
    Serial.begin(115200);
    delay(100);

    // восстановление подписок
    broker.load();

    EEPROM.begin(sizeEEPROM);


    initGate();

    // запуск протокола ESP-NOW
    conn.start();
    conn.setReceiveCallback(OnDataRecv);
    conn.setSendCallback(OnDataSent);
    // сразу установить связь со всеми подписчиками
    // иначе им не отправить сообщение
    broker.forEachTopic(OnTopic);

    last_time = millis();
}

void loop()
{
    // Serial.println("setBrokerAddr()");
    // setBrokerAddr();
    // delay(6000);
}