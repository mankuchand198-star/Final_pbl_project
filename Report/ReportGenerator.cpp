#include "ReportGenerator.h"
#include <iostream>

void ReportGenerator::generateFinalReport(CrimeCase& crimeCase, const EvidenceList& evidenceList,
                                           AnalysisResult results[], int resultCount,
                                           const InvestigationQueue& eventQueue) {
    std::cout << "\n";
    std::cout << "========================================\n";
    std::cout << "              CASE-X REPORT              \n";
    std::cout << "========================================\n";

    crimeCase.displayCaseInfo();

    std::cout << "\n";
    crimeCase.displaySuspects();

    std::cout << "\n";
    evidenceList.displayAll();

    std::cout << "\n===============================================\n";
    std::cout << "SUSPICION SCORES & RANKING\n";
    std::cout << "===============================================\n";
    for (int i = 0; i < resultCount; ++i) {
        std::cout << "  " << (i + 1) << ". " << results[i].suspectName
                  << " -- " << results[i].suspicionScore << "/100\n";
    }

    std::cout << "\n===============================================\n";
    std::cout << "DETAILED EXPLANATION PER SUSPECT\n";
    std::cout << "===============================================\n";
    for (int i = 0; i < resultCount; ++i) {
        std::cout << "\n(" << (i + 1) << ") " << results[i].suspectName
                  << " -- " << results[i].suspicionScore << "/100\n";
        std::cout << "  Score breakdown:\n";
        for (int k = 0; k < results[i].explanationCount; ++k) {
            std::cout << "    - " << results[i].explanationLines[k] << "\n";
        }
        std::cout << "  Linked evidence:\n";
        evidenceList.displayForSuspect(results[i].suspectName);
        if (results[i].contradictionCount > 0) {
            std::cout << "  Contradictions detected:\n";
            for (int k = 0; k < results[i].contradictionCount; ++k) {
                std::cout << "    ! " << results[i].contradictions[k] << "\n";
            }
        } else {
            std::cout << "  Contradictions detected: none\n";
        }
    }

    std::cout << "\n===============================================\n";
    if (resultCount > 0) {
        std::cout << "HIGHEST-RANKED SUSPECT: " << results[0].suspectName
                  << " (" << results[0].suspicionScore << "/100)\n";
    } else {
        std::cout << "No suspects have been analyzed yet.\n";
    }
    std::cout << "===============================================\n";

    std::cout << "\n";
    eventQueue.displayAll();

    std::cout << "\n===============================================\n";
    std::cout << "DISCLAIMER\n";
    std::cout << "===============================================\n";
    std::cout << "This ranking is based on predefined rules applied to fictional,\n";
    std::cout << "user-supplied case data. It is a rule-based academic exercise in\n";
    std::cout << "OOP and Data Structures, produces only a relative \"Suspicion\n";
    std::cout << "Score\", and does NOT establish actual guilt.\n";
    std::cout << "===============================================\n\n";
}
