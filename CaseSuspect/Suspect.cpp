#include "Suspect.h"
#include <iostream>

Suspect::Suspect()
    : Person(), locationDuringCrime("Unknown"), distanceLevel(5),
      accessLevel(0), motiveLevel(0), opportunityLevel(0), alibiStatus(0),
      alibiStatement("") {}

Suspect::Suspect(const std::string& name, int age, const std::string& occupation,
                  const std::string& locationDuringCrime, int distanceLevel,
                  int accessLevel, int motiveLevel, int opportunityLevel,
                  int alibiStatus, const std::string& alibiStatement)
    : Person(name, age, occupation),
      locationDuringCrime(locationDuringCrime),
      distanceLevel(distanceLevel),
      accessLevel(accessLevel),
      motiveLevel(motiveLevel),
      opportunityLevel(opportunityLevel),
      alibiStatus(alibiStatus),
      alibiStatement(alibiStatement) {}

std::string Suspect::getLocationDuringCrime() const { return locationDuringCrime; }
int Suspect::getDistanceLevel() const { return distanceLevel; }
int Suspect::getAccessLevel() const { return accessLevel; }
int Suspect::getMotiveLevel() const { return motiveLevel; }
int Suspect::getOpportunityLevel() const { return opportunityLevel; }
int Suspect::getAlibiStatus() const { return alibiStatus; }
std::string Suspect::getAlibiStatement() const { return alibiStatement; }

std::string Suspect::distanceLevelText() const {
    switch (distanceLevel) {
        case 1: return "At the scene";
        case 2: return "Less than 100m away";
        case 3: return "100-500m away";
        case 4: return "More than 500m away";
        default: return "Unknown";
    }
}

std::string Suspect::accessLevelText() const {
    switch (accessLevel) {
        case 0: return "No access";
        case 1: return "Possible access";
        case 2: return "Confirmed access";
        default: return "Unknown";
    }
}

std::string Suspect::motiveLevelText() const {
    switch (motiveLevel) {
        case 0: return "None";
        case 1: return "Weak";
        case 2: return "Moderate";
        case 3: return "Strong";
        case 4: return "Very Strong";
        default: return "Unknown";
    }
}

std::string Suspect::opportunityLevelText() const {
    switch (opportunityLevel) {
        case 0: return "None";
        case 1: return "Low";
        case 2: return "Moderate";
        case 3: return "High";
        case 4: return "Very High";
        default: return "Unknown";
    }
}

std::string Suspect::alibiStatusText() const {
    switch (alibiStatus) {
        case 0: return "Strong";
        case 1: return "Moderate";
        case 2: return "Weak";
        case 3: return "None";
        default: return "Unknown";
    }
}

void Suspect::display() const {
    std::cout << "---------------------------------------------\n";
    std::cout << "SUSPECT: " << name << "\n";
    std::cout << "  Age            : " << age << "\n";
    std::cout << "  Occupation     : " << occupation << "\n";
    std::cout << "  Location       : " << locationDuringCrime << "\n";
    std::cout << "  Distance       : " << distanceLevelText() << "\n";
    std::cout << "  Access         : " << accessLevelText() << "\n";
    std::cout << "  Motive         : " << motiveLevelText() << "\n";
    std::cout << "  Opportunity    : " << opportunityLevelText() << "\n";
    std::cout << "  Alibi Status   : " << alibiStatusText() << "\n";
    std::cout << "  Alibi Statement: \"" << alibiStatement << "\"\n";
}
