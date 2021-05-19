//
// Created by liuze on 2021/5/18.
//

#include "LRULinkedList.h"

#define BufferThreshold 10

#include "Connection.h"

long long operator-(const timeval &a, const timeval &b) {
    long long t = (a.tv_sec - b.tv_sec) * 1000 + (a.tv_usec - b.tv_usec) / 1000;
    if (t < 0) {
        t += 60 * 60 * 24 * 1000;
    }
    return t;
}

LRULinkedList::LRULinkedList() {
    virtualHead = new LRULinkedListNode(-1, nullptr);
    virtualTail = new LRULinkedListNode(-1, nullptr);
    virtualTail->prevNode = virtualHead;
    virtualHead->nextNode = virtualTail;
}

LRULinkedList::~LRULinkedList() {
    LRULinkedListNode *temp = virtualHead;
    while (temp != virtualTail) {
        auto tempNode = temp;
        temp = temp->nextNode;
        delete tempNode;
    }
    delete virtualTail;
}

void LRULinkedList::moveToTail(uint16_t uuid) {
    auto *tempNode = uuid2node[uuid];
    if (tempNode == nullptr) {
        printf("fatal error uuid %d no node\n", uuid);
        return;
    }
    if (tempNode->prevNode != nullptr && tempNode->nextNode != nullptr) {
        tempNode->prevNode->nextNode = tempNode->nextNode;
        tempNode->nextNode->prevNode = tempNode->prevNode;
    }
    virtualTail->prevNode->nextNode = tempNode;
    tempNode->prevNode = virtualTail->prevNode;
    virtualTail->prevNode = tempNode;
    tempNode->nextNode = virtualTail;
    gettimeofday(&tempNode->lastSendTime, nullptr);
}

void LRULinkedList::checkTimeOut() {
    timeval curTime{};
    gettimeofday(&curTime, nullptr);
    for (auto *node = virtualHead->nextNode; node != virtualTail;) {
        if (curTime - node->lastSendTime > BufferThreshold) {
            auto tempNode = node->nextNode;
            node->prevNode = nullptr;
            node->nextNode = nullptr;
            node->obj->callBack();
            node = tempNode;
        } else {
            break;
        }
        node->prevNode = virtualHead;
        virtualHead->nextNode = node;
    }
}

void LRULinkedList::AddNewNode(uint16_t uuid, LinkedNodeCallBack *obj) {
    if (uuid2node[uuid] == nullptr) {
        uuid2node[uuid] = new LRULinkedListNode(uuid, obj);
    }
}


