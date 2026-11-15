#include "InvestigationQueue.h"
#include <iostream>

InvestigationQueue::InvestigationQueue() : front(nullptr), rear(nullptr), count(0) {}

InvestigationQueue::~InvestigationQueue() {
    QueueNode* current = front;
    while (current != nullptr) {
        QueueNode* next = current->next;
        delete current;
        current = next;
    }
    front = rear = nullptr;
}

void InvestigationQueue::enqueue(const InvestigationEvent& e) {
    QueueNode* node = new QueueNode(e);
    if (rear == nullptr) {
        front = rear = node;
    } else {
        rear->next = node;
        rear = node;
    }
    count++;
}

bool InvestigationQueue::dequeue(InvestigationEvent& out) {
    if (front == nullptr) return false;
    QueueNode* temp = front;
    out = temp->data;
    front = front->next;
    if (front == nullptr) rear = nullptr;
    delete temp;
    count--;
    return true;
}

bool InvestigationQueue::isEmpty() const { return front == nullptr; }
int InvestigationQueue::getCount() const { return count; }

const QueueNode* InvestigationQueue::getFront() const { return front; }

void InvestigationQueue::displayAll() const {
    if (front == nullptr) {
        std::cout << "No investigation events recorded yet.\n";
        return;
    }
    std::cout << "===============================================\n";
    std::cout << "INVESTIGATION TIMELINE (" << count << " events)\n";
    std::cout << "===============================================\n";
    QueueNode* current = front;
    while (current != nullptr) {
        current->data.display();
        current = current->next;
    }
}
