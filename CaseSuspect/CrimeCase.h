#ifndef CRIMECASE_H
#define CRIMECASE_H

#include <string>
#include "Suspect.h"
#include "Witness.h"

const int MAX_SUSPECTS = 4;
const int MAX_WITNESSES = 3;

class CrimeCase {
private:
    std::string crimeType;
    std::string crimeLocation;
    std::string dateTime;
    std::string description;

    Suspect* suspects[MAX_SUSPECTS];
    int suspectCount;

    Witness* witnesses[MAX_WITNESSES];
    int witnessCount;

public:
    CrimeCase();
    ~CrimeCase();

    void setCaseInfo(const std::string& type, const std::string& location,
                      const std::string& when, const std::string& desc);
    void displayCaseInfo() const;

    bool addSuspect(Suspect* s);
    Suspect* getSuspect(int index) const;
    int getSuspectCount() const;

    Suspect* searchSuspectByName(const std::string& name) const;
    void displaySuspects() const;

    bool addWitness(Witness* w);
    int getWitnessCount() const;
    Witness* getWitness(int index) const;
    void displayWitnesses() const;

    std::string getCrimeType() const;
    std::string getCrimeLocation() const;
    std::string getDateTime() const;
    std::string getDescription() const;
};

#endif
