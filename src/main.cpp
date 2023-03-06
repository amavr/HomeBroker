#include <Arduino.h>

#include <SubList.h>
#include <TopicList.h>
#include <EventBroker.h>
#include <ATools.h>
#include <GParser.h>

EventBroker broker;

#include "EspNowConnector.h"
EspNowConnector conn;

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

void setup()
{
    Serial.begin(115200);
    delay(100);

    // восстановление подписок
    broker.load();

    // запуск протокола ESP-NOW
    conn.start();
    conn.setReceiveCallback(OnDataRecv);
    conn.setSendCallback(OnDataSent);
    // сразу установить связь со всеми подписчиками
    // иначе им не отправить сообщение
    broker.forEachTopic(OnTopic);
}

void loop()
{
}