#ifndef ANALYSISENGINE_H
#define ANALYSISENGINE_H

#include <string>
#include "../CaseSuspect/CrimeCase.h"
#include "../Evidence/EvidenceList.h"

const int MAX_EXPLANATION_LINES = 10;
const int MAX_CONTRADICTIONS = 10;

struct AnalysisResult {
    std::string suspectName;
    int suspicionScore = 0;
    std::string explanationLines[MAX_EXPLANATION_LINES];
    int explanationCount = 0;
    std::string contradictions[MAX_CONTRADICTIONS];
    int contradictionCount = 0;
};

class AnalysisEngine {
public:

    static AnalysisResult analyzeSuspect(Suspect* s, const EvidenceList& evidenceList);

    static void analyzeCase(CrimeCase& crimeCase, const EvidenceList& evidenceList,
                             AnalysisResult results[], int& resultCount);

    static void rankSuspects(AnalysisResult results[], int count);
};

#endif
