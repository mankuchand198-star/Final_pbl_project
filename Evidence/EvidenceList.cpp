#include "EvidenceList.h"
#include <iostream>

EvidenceList::EvidenceList() : head(nullptr), tail(nullptr), count(0) {}

EvidenceList::~EvidenceList() {
    EvidenceNode* current = head;
    while (current != nullptr) {
        EvidenceNode* next = current->next;
        delete current->data;
        delete current;
        current = next;
    }
    head = tail = nullptr;
}

void EvidenceList::addEvidence(Evidence* e) {
    EvidenceNode* node = new EvidenceNode(e);
    if (head == nullptr) {
        head = tail = node;
    } else {
        tail->next = node;
        tail = node;
    }
    count++;
}

Evidence* EvidenceList::searchById(int id) const {
    EvidenceNode* current = head;
    while (current != nullptr) {
        if (current->data->getId() == id) return current->data;
        current = current->next;
    }
    return nullptr;
}

void EvidenceList::displayAll() const {
    if (head == nullptr) {
        std::cout << "No evidence recorded yet.\n";
        return;
    }
    std::cout << "===============================================\n";
    std::cout << "EVIDENCE LIST (" << count << " items)\n";
    std::cout << "===============================================\n";
    EvidenceNode* current = head;
    while (current != nullptr) {
        current->data->display();
        current = current->next;
    }
}

void EvidenceList::displayForSuspect(const std::string& suspectName) const {
    bool found = false;
    EvidenceNode* current = head;
    while (current != nullptr) {
        if (current->data->isRelatedTo(suspectName)) {
            current->data->display();
            found = true;
        }
        current = current->next;
    }
    if (!found) {
        std::cout << "  (No evidence linked to " << suspectName << ")\n";
    }
}

int EvidenceList::countForSuspect(const std::string& suspectName) const {
    int n = 0;
    EvidenceNode* current = head;
    while (current != nullptr) {
        if (current->data->isRelatedTo(suspectName)) n++;
        current = current->next;
    }
    return n;
}

const EvidenceNode* EvidenceList::getHead() const { return head; }

int EvidenceList::getCount() const { return count; }

int EvidenceList::nextAvailableId() const {
    int maxId = 0;
    for (EvidenceNode* n = head; n != nullptr; n = n->next) {
        if (n->data->getId() > maxId) maxId = n->data->getId();
    }
    return maxId + 1;
}

bool EvidenceList::deleteById(int id) {
    EvidenceNode* prev = nullptr;
    EvidenceNode* cur = head;
    while (cur != nullptr && cur->data->getId() != id) {
        prev = cur;
        cur = cur->next;
    }
    if (cur == nullptr) return false;
    if (prev == nullptr) head = cur->next;
    else prev->next = cur->next;
    if (cur == tail) tail = prev;
    delete cur->data;
    delete cur;
    count--;
    return true;
}
