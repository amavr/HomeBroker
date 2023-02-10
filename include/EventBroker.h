#pragma once

#include <Arduino.h>
#include "TopicList.h"
#include "SubList.h"

class EventBroker
{
private:
    TopicList *topics = new TopicList;

public:
    EventBroker()
    {
    }

    SubList *newEvent(const char *topicName, const char *data)
    {
        Topic *topic = topics->add(topicName);
        topic->data = data;
        return topic->subscribers;
    }

    void Subscribe(const char *id, const char *topicName)
    {
        Topic *topic = topics->add(topicName);
        topic->subscribers->add(id);
    }

    void forEachTopic(void (*callback)(Topic *topic))
    {
        topics->forEach(callback);
    }
};