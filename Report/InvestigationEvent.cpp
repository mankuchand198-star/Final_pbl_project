#include "InvestigationEvent.h"
#include <iostream>

InvestigationEvent::InvestigationEvent() : sequenceNumber(0), description("") {}

InvestigationEvent::InvestigationEvent(int sequenceNumber, const std::string& description)
    : sequenceNumber(sequenceNumber), description(description) {}

int InvestigationEvent::getSequenceNumber() const { return sequenceNumber; }
std::string InvestigationEvent::getDescription() const { return description; }

void InvestigationEvent::display() const {
    std::cout << "  [" << sequenceNumber << "] " << description << "\n";
}
