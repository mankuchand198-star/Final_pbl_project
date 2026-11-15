#ifndef TESTIMONIALEVIDENCE_H
#define TESTIMONIALEVIDENCE_H

#include "Evidence.h"

class TestimonialEvidence : public Evidence {
private:
    std::string sourcePersonName;

public:
    TestimonialEvidence(int id, const std::string& description, Reliability reliability,
                         bool supportsSuspicion, const std::string& sourcePersonName);

    std::string getSourcePersonName() const;

    std::string getType() const override;
    double getStrengthScore() const override;
    void display() const override;
};

#endif
