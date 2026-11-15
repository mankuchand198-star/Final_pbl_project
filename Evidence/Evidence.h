#ifndef EVIDENCE_H
#define EVIDENCE_H

#include <string>

enum class Reliability { LOW = 1, MEDIUM = 2, HIGH = 3 };

std::string reliabilityText(Reliability r);

class Evidence {
protected:
    int id;
    std::string description;
    Reliability reliability;
    bool supportsSuspicion;
    static const int MAX_RELATED = 4;
    std::string relatedSuspects[MAX_RELATED];
    int relatedCount;

public:
    Evidence(int id, const std::string& description, Reliability reliability,
             bool supportsSuspicion);
    virtual ~Evidence();

    int getId() const;
    std::string getDescription() const;
    Reliability getReliability() const;
    bool getSupportsSuspicion() const;

    bool addRelatedSuspect(const std::string& suspectName);
    bool isRelatedTo(const std::string& suspectName) const;
    int getRelatedCount() const;
    std::string getRelatedSuspect(int index) const;

    virtual std::string getType() const = 0;

    virtual double getStrengthScore() const;

    virtual void display() const;
};

#endif
