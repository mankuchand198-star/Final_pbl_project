#ifndef INVESTIGATIONQUEUE_H
#define INVESTIGATIONQUEUE_H

#include "InvestigationEvent.h"

struct QueueNode {
    InvestigationEvent data;
    QueueNode* next;
    explicit QueueNode(const InvestigationEvent& d) : data(d), next(nullptr) {}
};

class InvestigationQueue {
private:
    QueueNode* front;
    QueueNode* rear;
    int count;

public:
    InvestigationQueue();
    ~InvestigationQueue();

    void enqueue(const InvestigationEvent& e);
    bool dequeue(InvestigationEvent& out);
    bool isEmpty() const;
    int getCount() const;
    const QueueNode* getFront() const;
    void displayAll() const;
};

#endif
