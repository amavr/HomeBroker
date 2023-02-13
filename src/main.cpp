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

// u8 *mac_addr, u8 *data, u8 len
void OnDataRecv(uint8_t *mac, uint8_t *data, uint8_t len)
{
    char text[len];
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
            char addr[18];
            ATools::macToChars(mac, addr);
            broker.subscribe(addr, rest);
        }
    }
    // subscription command ("pub <topic>:<any data>")
    else if (ATools::isCmd("pub ", text, rest))
    {
        // отделить данные от топика
        GParser data(rest, ':');
        int count = data.split();
        if (count == 2)
        {
            // выделить топик
            GParser ldata(data[0], ' ');
            int lcount = ldata.split();
            if (lcount == 2)
            {
                // опубликовать данные для топика
                SubList *subs = broker.publish(ldata[1], data[1]);
                // получить подписчиков
                if (subs->Count() > 0)
                {
                    int count = subs->Count();
                    char *ids[count];
                    subs->fill(ids);
                    uint8_t mac[6];
                    // разослать данные топика подписчикам
                    for (int i = 0; i < count; i++)
                    {
                        ATools::macToBytes(ids[i], mac);
                        // "pub <topic>:<any data>"
                        // except "pub "
                        esp_now_send(mac, (uint8_t *)(text + 4), strlen(text) - 4);
                    }
                }
            }
        }
    }
}

void setup()
{
    Serial.begin(115200);
    delay(100);

    EEPROM.begin(sizeEEPROM);

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

    // регистрация в шлюзе
    setBrokerAddr();
    char addr[0x20];
    if (getBrokerAddr(addr))
    {
        Serial.println(addr);
    }

    // запуск протокола ESP-NOW
    conn.setReceiveCallback(OnDataRecv);

    // broker.Subscribe("first", "OUTSIDE.TEMP");
    // broker.Subscribe("second", "OUTSIDE.TEMP");
    // broker.Subscribe("first", "INSIDE.TEMP");
    // broker.forEachTopic(&cbTopicSubs);
}

void loop()
{
    // проверка сети
    ctrl.tick();
    delay(1000);
}