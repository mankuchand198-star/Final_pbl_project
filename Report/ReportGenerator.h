#ifndef REPORTGENERATOR_H
#define REPORTGENERATOR_H

#include "../CaseSuspect/CrimeCase.h"
#include "../Evidence/EvidenceList.h"
#include "../Analysis/AnalysisEngine.h"
#include "InvestigationQueue.h"

class ReportGenerator {
public:
    static void generateFinalReport(CrimeCase& crimeCase, const EvidenceList& evidenceList,
                                     AnalysisResult results[], int resultCount,
                                     const InvestigationQueue& eventQueue);
};

#endif
