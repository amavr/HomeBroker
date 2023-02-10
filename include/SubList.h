#pragma once

#include <Arduino.h>
#include "TopicList.h"
#include <GParser.h>

struct Sub
{
    const char *id; // это адрес (mac, ip, ...)
    Sub *next;
};

class SubList
{
private:
    Sub *head;
    int count;

public:
    SubList()
    {
        count = 0;
        head = NULL;
    }

    int Count()
    {
        return count;
    }

    Sub *find(const char *id)
    {
        Sub *curr = head;
        while (curr != NULL)
        {
            if (strcmp(curr->id, id) == 0)
            {
                return curr;
            }
            curr = curr->next;
        }
        return NULL;
    }

    bool exists(const char *id)
    {
        return find(id) != NULL;
    }

    void add(const char *id)
    {
        // если не найден, то создать и добавить
        if (!exists(id))
        {
            // создание узла
            count++;
            Sub *sub = new Sub;
            sub->id = id;
            sub->next = NULL;

            // пустой список? тогда в самое начало
            if (head == NULL)
            {
                Serial.printf("as firts: %s\n", sub->id);
                head = sub;
            }
            else
            {
                // в конец
                Sub *curr = head;
                while (curr->next != NULL)
                {
                    curr = curr->next;
                }
                Serial.printf("as last: %s\n", sub->id);
                curr->next = sub;
            }
        }
    }

    void forEach(void (*callback)(const char *id))
    {
        Sub *curr = head;
        while (curr != NULL)
        {
            callback(curr->id);
            curr = curr->next;
        }
    }
};