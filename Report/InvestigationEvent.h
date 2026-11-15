#ifndef INVESTIGATIONEVENT_H
#define INVESTIGATIONEVENT_H

#include <string>

class InvestigationEvent {
private:
    int sequenceNumber;
    std::string description;

public:
    InvestigationEvent();
    InvestigationEvent(int sequenceNumber, const std::string& description);

    int getSequenceNumber() const;
    std::string getDescription() const;

    void display() const;
};

#endif
