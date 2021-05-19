//
// Created by liuze on 2021/5/18.
//

#ifndef UDPSPEEDER_LRULINKEDLIST_H
#define UDPSPEEDER_LRULINKEDLIST_H

#include "sys/time.h"
#include <cstdint>
#include <map>
#include "LinkedNodeCallBack.h"

class LRULinkedList {
private:
    class LRULinkedListNode {
    public:
        uint16_t uuid;
        timeval lastSendTime;
        LRULinkedListNode *nextNode, *prevNode;
        LinkedNodeCallBack *obj;

        LRULinkedListNode(uint16_t uuid, LinkedNodeCallBack *obj) {
            this->uuid = uuid;
            this->obj = obj;
        }
    };

    LRULinkedListNode *virtualHead, *virtualTail;

    ~LRULinkedList();

    std::map<uint16_t, LRULinkedListNode *> uuid2node;
public:
    void checkTimeOut();

    void moveToTail(uint16_t uuid);

    void AddNewNode(uint16_t uuid, LinkedNodeCallBack *obj);

    LRULinkedList();
};


#endif //UDPSPEEDER_LRULINKEDLIST_H
