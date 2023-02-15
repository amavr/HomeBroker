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
            if (count > 0)
            {
                char *ids[count];
                subs->fill(ids);
                // uint8_t mac[6];
                // разослать данные топика подписчикам
                for (int i = 0; i < count; i++)
                {
                    conn.send(ids[i], (char *)(text + 4));
                    // ATools::macToBytes(ids[i], mac);
                    // // "<topic>:<any data>"
                    // esp_now_send(mac, (uint8_t *)(text + 4), strlen(text) - 4);
                }
                for (int i = 0; i < count; i++)
                {
                    free(ids[i]);
                }
            }
        }
    }
}

void OnDataSent(uint8_t *mac, uint8_t status)
{
    Serial.printf("%s\n", status == 0 ? "success" : "fail");
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

    // восстановление подписок
    broker.load();

    // регистрация в шлюзе
    setBrokerAddr();
    char addr[0x20];
    if (getBrokerAddr(addr))
    {
        Serial.println(addr);
    }

    WiFi.disconnect();

    // запуск протокола ESP-NOW
    conn.start();
    conn.setReceiveCallback(OnDataRecv);
    conn.setSendCallback(OnDataSent);
}

void loop()
{
    // проверка сети (брокер должен работать без WiFi)
    // ctrl.tick();
    delay(1000);
}