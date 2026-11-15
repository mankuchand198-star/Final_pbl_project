#ifndef SUSPECT_H
#define SUSPECT_H

#include "Person.h"
#include <string>

class Suspect : public Person {
private:
    std::string locationDuringCrime;
    int distanceLevel;
    int accessLevel;
    int motiveLevel;
    int opportunityLevel;
    int alibiStatus;
    std::string alibiStatement;

public:
    Suspect();
    Suspect(const std::string& name, int age, const std::string& occupation,
            const std::string& locationDuringCrime, int distanceLevel,
            int accessLevel, int motiveLevel, int opportunityLevel,
            int alibiStatus, const std::string& alibiStatement);

    std::string getLocationDuringCrime() const;
    int getDistanceLevel() const;
    int getAccessLevel() const;
    int getMotiveLevel() const;
    int getOpportunityLevel() const;
    int getAlibiStatus() const;
    std::string getAlibiStatement() const;

    std::string distanceLevelText() const;
    std::string accessLevelText() const;
    std::string motiveLevelText() const;
    std::string opportunityLevelText() const;
    std::string alibiStatusText() const;

    void display() const override;
};

#endif
