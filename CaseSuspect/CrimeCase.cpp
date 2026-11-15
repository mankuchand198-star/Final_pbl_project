#include "CrimeCase.h"
#include <iostream>
#include "../Utils.h"

CrimeCase::CrimeCase()
    : crimeType(""), crimeLocation(""), dateTime(""), description(""),
      suspectCount(0), witnessCount(0) {
    for (int i = 0; i < MAX_SUSPECTS; ++i) suspects[i] = nullptr;
    for (int i = 0; i < MAX_WITNESSES; ++i) witnesses[i] = nullptr;
}

CrimeCase::~CrimeCase() {
    for (int i = 0; i < suspectCount; ++i) delete suspects[i];
    for (int i = 0; i < witnessCount; ++i) delete witnesses[i];
}

void CrimeCase::setCaseInfo(const std::string& type, const std::string& location,
                             const std::string& when, const std::string& desc) {
    crimeType = type;
    crimeLocation = location;
    dateTime = when;
    description = desc;
}

void CrimeCase::displayCaseInfo() const {
    std::cout << "===============================================\n";
    std::cout << "CASE FILE\n";
    std::cout << "===============================================\n";
    std::cout << "Crime Type   : " << crimeType << "\n";
    std::cout << "Location     : " << crimeLocation << "\n";
    std::cout << "Date/Time    : " << dateTime << "\n";
    std::cout << "Description  : " << description << "\n";
}

bool CrimeCase::addSuspect(Suspect* s) {
    if (suspectCount >= MAX_SUSPECTS) return false;
    suspects[suspectCount++] = s;
    return true;
}

Suspect* CrimeCase::getSuspect(int index) const {
    if (index < 0 || index >= suspectCount) return nullptr;
    return suspects[index];
}

int CrimeCase::getSuspectCount() const { return suspectCount; }

Suspect* CrimeCase::searchSuspectByName(const std::string& name) const {
    std::string target = toLowerStr(name);
    for (int i = 0; i < suspectCount; ++i) {
        if (toLowerStr(suspects[i]->getName()) == target) return suspects[i];
    }
    return nullptr;
}

void CrimeCase::displaySuspects() const {
    std::cout << "===============================================\n";
    std::cout << "SUSPECT LIST (" << suspectCount << ")\n";
    std::cout << "===============================================\n";
    for (int i = 0; i < suspectCount; ++i) {
        suspects[i]->display();
    }
}

bool CrimeCase::addWitness(Witness* w) {
    if (witnessCount >= MAX_WITNESSES) return false;
    witnesses[witnessCount++] = w;
    return true;
}

int CrimeCase::getWitnessCount() const { return witnessCount; }

Witness* CrimeCase::getWitness(int index) const {
    if (index < 0 || index >= witnessCount) return nullptr;
    return witnesses[index];
}

void CrimeCase::displayWitnesses() const {
    if (witnessCount == 0) {
        std::cout << "No witnesses recorded for this case.\n";
        return;
    }
    std::cout << "===============================================\n";
    std::cout << "WITNESS LIST (" << witnessCount << ")\n";
    std::cout << "===============================================\n";
    for (int i = 0; i < witnessCount; ++i) {
        witnesses[i]->display();
    }
}

std::string CrimeCase::getCrimeType() const { return crimeType; }
std::string CrimeCase::getCrimeLocation() const { return crimeLocation; }
std::string CrimeCase::getDateTime() const { return dateTime; }
std::string CrimeCase::getDescription() const { return description; }
