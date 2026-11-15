#ifndef DIGITALEVIDENCE_H
#define DIGITALEVIDENCE_H

#include "Evidence.h"

class DigitalEvidence : public Evidence {
private:
    std::string sourceDevice;
    bool verifiedAuthentic;

public:
    DigitalEvidence(int id, const std::string& description, Reliability reliability,
                     bool supportsSuspicion, const std::string& sourceDevice,
                     bool verifiedAuthentic);

    std::string getSourceDevice() const;
    bool isVerifiedAuthentic() const;

    std::string getType() const override;
    double getStrengthScore() const override;
    void display() const override;
};

#endif
