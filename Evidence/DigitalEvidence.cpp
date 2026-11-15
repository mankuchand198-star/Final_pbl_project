#include "DigitalEvidence.h"
#include <iostream>

DigitalEvidence::DigitalEvidence(int id, const std::string& description,
                                  Reliability reliability, bool supportsSuspicion,
                                  const std::string& sourceDevice, bool verifiedAuthentic)
    : Evidence(id, description, reliability, supportsSuspicion),
      sourceDevice(sourceDevice), verifiedAuthentic(verifiedAuthentic) {}

std::string DigitalEvidence::getSourceDevice() const { return sourceDevice; }
bool DigitalEvidence::isVerifiedAuthentic() const { return verifiedAuthentic; }

std::string DigitalEvidence::getType() const { return "Digital Evidence"; }

double DigitalEvidence::getStrengthScore() const {
    double base = Evidence::getStrengthScore();
    return verifiedAuthentic ? base * 1.3 : base * 0.7;
}

void DigitalEvidence::display() const {
    Evidence::display();
    std::cout << "       Source     : " << sourceDevice
              << " | Verified: " << (verifiedAuthentic ? "Yes" : "No") << "\n";
}
