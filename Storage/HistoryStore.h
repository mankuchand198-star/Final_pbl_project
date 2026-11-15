#ifndef HISTORYSTORE_H
#define HISTORYSTORE_H

#include <string>
#include "../CaseSuspect/CrimeCase.h"
#include "../Evidence/EvidenceList.h"
#include "../Report/InvestigationQueue.h"

struct HistoryEntry {
    int id;
    std::string savedAt;
    std::string title;
    std::string topName;
    int topScore;
    int suspectCount;
    int evidenceCount;
    std::string body;
    HistoryEntry* next;
    HistoryEntry() : id(0), topScore(0), suspectCount(0), evidenceCount(0), next(nullptr) {}
};

class HistoryStore {
private:
    std::string path;
    HistoryEntry* head;
    int count;
    void clear();

public:
    explicit HistoryStore(const std::string& filePath);
    ~HistoryStore();
    HistoryStore(const HistoryStore&) = delete;
    HistoryStore& operator=(const HistoryStore&) = delete;

    bool load();
    bool save() const;
    int nextId() const;
    void upsert(int id, const std::string& savedAt, const std::string& title,
                const std::string& topName, int topScore, int suspectCount,
                int evidenceCount, const std::string& body);
    bool removeById(int id);
    const HistoryEntry* findById(int id) const;
    const HistoryEntry* getAt(int index) const;
    int getCount() const;
};

std::string serializeCase(const CrimeCase& c, const EvidenceList& e, const InvestigationQueue& q);
bool deserializeCase(const std::string& body, CrimeCase& c, EvidenceList& e, InvestigationQueue& q);

#endif
