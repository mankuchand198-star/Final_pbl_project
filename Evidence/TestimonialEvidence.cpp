#include "TestimonialEvidence.h"
#include <iostream>

TestimonialEvidence::TestimonialEvidence(int id, const std::string& description,
                                          Reliability reliability, bool supportsSuspicion,
                                          const std::string& sourcePersonName)
    : Evidence(id, description, reliability, supportsSuspicion),
      sourcePersonName(sourcePersonName) {}

std::string TestimonialEvidence::getSourcePersonName() const { return sourcePersonName; }

std::string TestimonialEvidence::getType() const { return "Testimonial Evidence"; }

double TestimonialEvidence::getStrengthScore() const {
    double base = Evidence::getStrengthScore();
    return base * 0.8;
}

void TestimonialEvidence::display() const {
    Evidence::display();
    std::cout << "       Source Person: " << sourcePersonName << "\n";
}
