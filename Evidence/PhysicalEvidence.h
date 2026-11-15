#ifndef PHYSICALEVIDENCE_H
#define PHYSICALEVIDENCE_H

#include "Evidence.h"

class PhysicalEvidence : public Evidence {
private:
    std::string locationFound;

public:
    PhysicalEvidence(int id, const std::string& description, Reliability reliability,
                      bool supportsSuspicion, const std::string& locationFound);

    std::string getLocationFound() const;

    std::string getType() const override;
    double getStrengthScore() const override;
    void display() const override;
};

#endif
