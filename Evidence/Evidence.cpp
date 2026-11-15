#include "Evidence.h"
#include <iostream>

std::string reliabilityText(Reliability r) {
    switch (r) {
        case Reliability::LOW: return "Low";
        case Reliability::MEDIUM: return "Medium";
        case Reliability::HIGH: return "High";
    }
    return "Unknown";
}

Evidence::Evidence(int id, const std::string& description, Reliability reliability,
                    bool supportsSuspicion)
    : id(id), description(description), reliability(reliability),
      supportsSuspicion(supportsSuspicion), relatedCount(0) {}

Evidence::~Evidence() {}

int Evidence::getId() const { return id; }
std::string Evidence::getDescription() const { return description; }
Reliability Evidence::getReliability() const { return reliability; }
bool Evidence::getSupportsSuspicion() const { return supportsSuspicion; }

bool Evidence::addRelatedSuspect(const std::string& suspectName) {
    if (relatedCount >= MAX_RELATED) return false;
    relatedSuspects[relatedCount++] = suspectName;
    return true;
}

bool Evidence::isRelatedTo(const std::string& suspectName) const {
    for (int i = 0; i < relatedCount; ++i) {
        if (relatedSuspects[i] == suspectName) return true;
    }
    return false;
}

int Evidence::getRelatedCount() const { return relatedCount; }

std::string Evidence::getRelatedSuspect(int index) const {
    if (index < 0 || index >= relatedCount) return "";
    return relatedSuspects[index];
}

double Evidence::getStrengthScore() const {
    double weight = static_cast<double>(reliability);
    return supportsSuspicion ? weight : -weight;
}

void Evidence::display() const {
    std::cout << "  [E" << id << "] (" << getType() << ") " << description << "\n";
    std::cout << "       Reliability: " << reliabilityText(reliability)
              << " | Effect: " << (supportsSuspicion ? "Supports suspicion" : "Weakens suspicion")
              << "\n";
    std::cout << "       Related to : ";
    if (relatedCount == 0) {
        std::cout << "(none)";
    } else {
        for (int i = 0; i < relatedCount; ++i) {
            std::cout << relatedSuspects[i];
            if (i + 1 < relatedCount) std::cout << ", ";
        }
    }
    std::cout << "\n";
}
