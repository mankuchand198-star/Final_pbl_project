#ifndef EVIDENCELIST_H
#define EVIDENCELIST_H

#include "Evidence.h"
#include <string>

struct EvidenceNode {
    Evidence* data;
    EvidenceNode* next;
    explicit EvidenceNode(Evidence* d) : data(d), next(nullptr) {}
};

class EvidenceList {
private:
    EvidenceNode* head;
    EvidenceNode* tail;
    int count;

public:
    EvidenceList();
    ~EvidenceList();

    void addEvidence(Evidence* e);
    Evidence* searchById(int id) const;
    bool deleteById(int id);
    void displayAll() const;
    void displayForSuspect(const std::string& suspectName) const;
    int countForSuspect(const std::string& suspectName) const;

    const EvidenceNode* getHead() const;
    int getCount() const;
    int nextAvailableId() const;
};

#endif
