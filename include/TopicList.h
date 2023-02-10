#pragma once

#include <Arduino.h>
#include "SubList.h"

struct Topic
{
    const char *name;
    const char *data;
    SubList *subscribers;
    Topic *next;
};

class TopicList
{
private:
    Topic *head;
    int count;

public:
    TopicList()
    {
        head = NULL;
        count = 0;
    }

    int Count()
    {
        return count;
    }

    Topic *find(const char *topicName)
    {
        Topic *curr = head;
        while (curr != NULL)
        {
            if (strcmp(curr->name, topicName) == 0)
            {
                return curr;
            }
            curr = curr->next;
        }
        return NULL;
    }

    bool exists(const char *topicName)
    {
        return find(topicName) != NULL;
    }

    Topic *add(const char *topicName)
    {
        Topic *topic = find(topicName);
        // если не найден, добавляется в конец
        if (topic == NULL)
        {
            count++;
            // создание нового узла
            topic = new Topic;
            topic->name = topicName;
            topic->subscribers = new SubList;
            topic->next = NULL;

            Topic *curr = head, *prev = NULL;
            while (curr != NULL)
            {
                prev = curr;
                curr = curr->next;
            }
            // нет предпоследнего элемента - список пуст
            if(prev == NULL)
            {
                head = topic;
            }
            // указать в предпоследнем элементе на новый последний
            else
            {
                curr = topic;
                prev->next = curr;
            }
        }
        return topic;
    }

    void forEach(void (*callback)(Topic *topic))
    {
        Topic *curr = head;
        while (curr != NULL)
        {
            callback(curr);
            curr = curr->next;
        }
    }
};
