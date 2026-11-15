#include "HistoryStore.h"
#include "../Utils.h"
#include "../Evidence/PhysicalEvidence.h"
#include "../Evidence/DigitalEvidence.h"
#include "../Evidence/TestimonialEvidence.h"
#include <cstdio>

namespace {

const char* RECORD_MARK = "CASEX-RECORD";
const char* BODY_MARK = "CASEDATA 1";

std::string oneLine(const std::string& v) {
    std::string out = v;
    for (size_t i = 0; i < out.size(); ++i) {
        if (out[i] == '\n' || out[i] == '\r') out[i] = ' ';
    }
    return out;
}

void putLine(std::string& out, const std::string& v) {
    out += oneLine(v);
    out += '\n';
}

void putInt(std::string& out, int v) {
    putLine(out, intToString(v));
}

class LineReader {
private:
    const std::string& text;
    size_t pos;

public:
    explicit LineReader(const std::string& t) : text(t), pos(0) {}

    bool next(std::string& out) {
        if (pos >= text.size()) return false;
        size_t end = text.find('\n', pos);
        if (end == std::string::npos) {
            out = text.substr(pos);
            pos = text.size();
        } else {
            out = text.substr(pos, end - pos);
            pos = end + 1;
        }
        return true;
    }

    bool nextInt(int& v) {
        std::string s;
        return next(s) && parseInt(s, v);
    }
};

bool readFileLine(FILE* f, std::string& out) {
    out.clear();
    bool any = false;
    int c;
    while ((c = fgetc(f)) != EOF) {
        any = true;
        if (c == '\n') return true;
        if (c != '\r') out += static_cast<char>(c);
    }
    return any;
}

bool readFileInt(FILE* f, int& v) {
    std::string s;
    return readFileLine(f, s) && parseInt(s, v);
}

int countLines(const std::string& s) {
    int n = 0;
    for (size_t i = 0; i < s.size(); ++i) {
        if (s[i] == '\n') n++;
    }
    return n;
}

}

HistoryStore::HistoryStore(const std::string& filePath) : path(filePath), head(nullptr), count(0) {}

HistoryStore::~HistoryStore() {
    clear();
}

void HistoryStore::clear() {
    HistoryEntry* cur = head;
    while (cur != nullptr) {
        HistoryEntry* next = cur->next;
        delete cur;
        cur = next;
    }
    head = nullptr;
    count = 0;
}

bool HistoryStore::load() {
    clear();
    FILE* f = fopen(path.c_str(), "r");
    if (f == NULL) return false;

    HistoryEntry* tail = nullptr;
    std::string line;
    while (readFileLine(f, line)) {
        if (line != RECORD_MARK) continue;
        HistoryEntry* e = new HistoryEntry();
        int bodyLines = 0;
        bool ok = readFileInt(f, e->id) && readFileLine(f, e->savedAt) && readFileLine(f, e->title) &&
                  readFileLine(f, e->topName) && readFileInt(f, e->topScore) &&
                  readFileInt(f, e->suspectCount) && readFileInt(f, e->evidenceCount) &&
                  readFileInt(f, bodyLines) && bodyLines >= 0 && bodyLines <= 100000;
        for (int i = 0; ok && i < bodyLines; ++i) {
            std::string b;
            if (!readFileLine(f, b)) {
                ok = false;
            } else {
                e->body += b;
                e->body += '\n';
            }
        }
        if (!ok) {
            delete e;
            break;
        }
        if (tail == nullptr) head = e;
        else tail->next = e;
        tail = e;
        count++;
    }
    fclose(f);
    return true;
}

bool HistoryStore::save() const {
    FILE* f = fopen(path.c_str(), "w");
    if (f == NULL) return false;
    for (const HistoryEntry* e = head; e != nullptr; e = e->next) {
        fprintf(f, "%s\n%d\n%s\n%s\n%s\n%d\n%d\n%d\n%d\n", RECORD_MARK, e->id,
                oneLine(e->savedAt).c_str(), oneLine(e->title).c_str(), oneLine(e->topName).c_str(),
                e->topScore, e->suspectCount, e->evidenceCount, countLines(e->body));
        fputs(e->body.c_str(), f);
    }
    bool ok = (ferror(f) == 0);
    if (fclose(f) != 0) ok = false;
    return ok;
}

int HistoryStore::nextId() const {
    int maxId = 0;
    for (const HistoryEntry* e = head; e != nullptr; e = e->next) {
        if (e->id > maxId) maxId = e->id;
    }
    return maxId + 1;
}

void HistoryStore::upsert(int id, const std::string& savedAt, const std::string& title,
                          const std::string& topName, int topScore, int suspectCount,
                          int evidenceCount, const std::string& body) {
    removeById(id);
    HistoryEntry* e = new HistoryEntry();
    e->id = id;
    e->savedAt = oneLine(savedAt);
    e->title = oneLine(title);
    e->topName = oneLine(topName);
    e->topScore = topScore;
    e->suspectCount = suspectCount;
    e->evidenceCount = evidenceCount;
    e->body = body;
    e->next = head;
    head = e;
    count++;
}

bool HistoryStore::removeById(int id) {
    HistoryEntry* prev = nullptr;
    HistoryEntry* cur = head;
    while (cur != nullptr && cur->id != id) {
        prev = cur;
        cur = cur->next;
    }
    if (cur == nullptr) return false;
    if (prev == nullptr) head = cur->next;
    else prev->next = cur->next;
    delete cur;
    count--;
    return true;
}

const HistoryEntry* HistoryStore::findById(int id) const {
    for (const HistoryEntry* e = head; e != nullptr; e = e->next) {
        if (e->id == id) return e;
    }
    return nullptr;
}

const HistoryEntry* HistoryStore::getAt(int index) const {
    if (index < 0) return nullptr;
    const HistoryEntry* e = head;
    for (int i = 0; e != nullptr && i < index; ++i) e = e->next;
    return e;
}

int HistoryStore::getCount() const { return count; }

std::string serializeCase(const CrimeCase& c, const EvidenceList& e, const InvestigationQueue& q) {
    std::string out;
    putLine(out, BODY_MARK);
    putLine(out, c.getCrimeType());
    putLine(out, c.getCrimeLocation());
    putLine(out, c.getDateTime());
    putLine(out, c.getDescription());

    putInt(out, c.getSuspectCount());
    for (int i = 0; i < c.getSuspectCount(); ++i) {
        const Suspect* s = c.getSuspect(i);
        putLine(out, s->getName());
        putInt(out, s->getAge());
        putLine(out, s->getOccupation());
        putLine(out, s->getLocationDuringCrime());
        putInt(out, s->getDistanceLevel());
        putInt(out, s->getAccessLevel());
        putInt(out, s->getMotiveLevel());
        putInt(out, s->getOpportunityLevel());
        putInt(out, s->getAlibiStatus());
        putLine(out, s->getAlibiStatement());
    }

    putInt(out, c.getWitnessCount());
    for (int i = 0; i < c.getWitnessCount(); ++i) {
        const Witness* w = c.getWitness(i);
        putLine(out, w->getName());
        putInt(out, w->getAge());
        putLine(out, w->getOccupation());
        putLine(out, w->getStatement());
        putLine(out, w->getRelationToCase());
    }

    putInt(out, e.getCount());
    for (const EvidenceNode* n = e.getHead(); n != nullptr; n = n->next) {
        Evidence* ev = n->data;
        int kind = 2;
        std::string extra;
        int verified = 0;
        if (PhysicalEvidence* p = dynamic_cast<PhysicalEvidence*>(ev)) {
            kind = 0;
            extra = p->getLocationFound();
        } else if (DigitalEvidence* d = dynamic_cast<DigitalEvidence*>(ev)) {
            kind = 1;
            extra = d->getSourceDevice();
            verified = d->isVerifiedAuthentic() ? 1 : 0;
        } else if (TestimonialEvidence* t = dynamic_cast<TestimonialEvidence*>(ev)) {
            extra = t->getSourcePersonName();
        }
        putInt(out, kind);
        putInt(out, ev->getId());
        putLine(out, ev->getDescription());
        putInt(out, static_cast<int>(ev->getReliability()));
        putInt(out, ev->getSupportsSuspicion() ? 1 : 0);
        putLine(out, extra);
        putInt(out, verified);
        putInt(out, ev->getRelatedCount());
        for (int i = 0; i < ev->getRelatedCount(); ++i) putLine(out, ev->getRelatedSuspect(i));
    }

    putInt(out, q.getCount());
    for (const QueueNode* n = q.getFront(); n != nullptr; n = n->next) {
        putLine(out, n->data.getDescription());
    }
    return out;
}

bool deserializeCase(const std::string& body, CrimeCase& c, EvidenceList& e, InvestigationQueue& q) {
    LineReader r(body);
    std::string s;
    if (!r.next(s) || s != BODY_MARK) return false;

    std::string type, loc, when, desc;
    if (!r.next(type) || !r.next(loc) || !r.next(when) || !r.next(desc)) return false;
    c.setCaseInfo(type, loc, when, desc);

    int n = 0;
    if (!r.nextInt(n) || n < 0 || n > MAX_SUSPECTS) return false;
    for (int i = 0; i < n; ++i) {
        std::string name, occ, place, alibi;
        int age, dist, access, motive, opp, status;
        if (!r.next(name) || !r.nextInt(age) || !r.next(occ) || !r.next(place) || !r.nextInt(dist) ||
            !r.nextInt(access) || !r.nextInt(motive) || !r.nextInt(opp) || !r.nextInt(status) ||
            !r.next(alibi)) return false;
        if (dist < 1 || dist > 5 || access > 2 || motive > 4 || opp > 4 || status > 3) return false;
        Suspect* sp = new Suspect(name, age, occ, place, dist, access, motive, opp, status, alibi);
        if (!c.addSuspect(sp)) {
            delete sp;
            return false;
        }
    }

    if (!r.nextInt(n) || n < 0 || n > MAX_WITNESSES) return false;
    for (int i = 0; i < n; ++i) {
        std::string name, occ, statement, relation;
        int age;
        if (!r.next(name) || !r.nextInt(age) || !r.next(occ) || !r.next(statement) || !r.next(relation)) return false;
        Witness* w = new Witness(name, age, occ, statement, relation);
        if (!c.addWitness(w)) {
            delete w;
            return false;
        }
    }

    if (!r.nextInt(n) || n < 0 || n > 1000) return false;
    for (int i = 0; i < n; ++i) {
        int kind, id, rel, supports, verified, related;
        std::string description, extra;
        if (!r.nextInt(kind) || !r.nextInt(id) || !r.next(description) || !r.nextInt(rel) ||
            !r.nextInt(supports) || !r.next(extra) || !r.nextInt(verified) || !r.nextInt(related)) return false;
        if (kind > 2 || rel < 1 || rel > 3 || related > 4) return false;
        Reliability reliability = static_cast<Reliability>(rel);
        Evidence* ev = nullptr;
        if (kind == 0) ev = new PhysicalEvidence(id, description, reliability, supports == 1, extra);
        else if (kind == 1) ev = new DigitalEvidence(id, description, reliability, supports == 1, extra, verified == 1);
        else ev = new TestimonialEvidence(id, description, reliability, supports == 1, extra);
        for (int k = 0; k < related; ++k) {
            std::string name;
            if (!r.next(name)) {
                delete ev;
                return false;
            }
            ev->addRelatedSuspect(name);
        }
        e.addEvidence(ev);
    }

    if (!r.nextInt(n) || n < 0 || n > 1000) return false;
    for (int i = 0; i < n; ++i) {
        std::string text;
        if (!r.next(text)) return false;
        q.enqueue(InvestigationEvent(q.getCount() + 1, text));
    }
    return true;
}
