#include "PhysicalEvidence.h"
#include <iostream>

PhysicalEvidence::PhysicalEvidence(int id, const std::string& description,
                                    Reliability reliability, bool supportsSuspicion,
                                    const std::string& locationFound)
    : Evidence(id, description, reliability, supportsSuspicion),
      locationFound(locationFound) {}

std::string PhysicalEvidence::getLocationFound() const { return locationFound; }

std::string PhysicalEvidence::getType() const { return "Physical Evidence"; }

double PhysicalEvidence::getStrengthScore() const {
    double base = Evidence::getStrengthScore();
    return base * 1.2;
}

void PhysicalEvidence::display() const {
    Evidence::display();
    std::cout << "       Found at   : " << locationFound << "\n";
}
