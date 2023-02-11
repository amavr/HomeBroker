#include <Arduino.h>

#include <SubList.h>
#include <TopicList.h>
#include <EventBroker.h>

EventBroker broker;

void cbSub(const char *id)
{
    Serial.printf("  %s\n", id);
}

void cbTopic(Topic *topic)
{
    Serial.println(topic->name);
}

void cbTopicSubs(Topic *topic)
{
    Serial.println(topic->name);
    topic->subscribers->forEach(&cbSub);
}

void setup()
{
    Serial.begin(115200);
    delay(100);

    Serial.println("started");

    // TopicList topics;
    // Topic *t = topics.add("test");

    // t->subscribers->add("111");
    // t->subscribers->add("111");
    // t->subscribers->add("222");
    // Serial.printf("subs count %d\n", t->subscribers->Count());
    // t->subscribers->forEach(&cbSub);

    // topics.add("test-2");
    // topics.add("test-3");
    // Serial.printf("topic count %d\n", topics.Count());
    // topics.forEach(&cbTopic);

    broker.Subscribe("first", "OUTSIDE.TEMP");
    broker.Subscribe("second", "OUTSIDE.TEMP");
    broker.Subscribe("first", "INSIDE.TEMP");
    broker.forEachTopic(&cbTopicSubs);
}

void loop()
{
    for(int i = 0; i < 1000; i++){
        long x = random(0, 10);
        const char *topicName = x < 6 ? "OUTSIDE.TEMP" : "INSIDE.TEMP";

        char data[20] = "";
        ultoa(millis(), data, DEC);

        // Serial.printf("\n%s - %s\n", topicName, data);

        SubList *subs = broker.newEvent(topicName, data);
        // subs->forEach(&cbSub);
    }

    uint32_t freeHeap = ESP.getFreeHeap();
    uint8_t frag = ESP.getHeapFragmentation();
    Serial.printf("%ld\t%d\t%d\n", millis(), frag, freeHeap);

    delay(1000);
}