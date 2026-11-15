#include <iostream>
#include <cstdlib>
#include <string>

#include "CaseSuspect/CrimeCase.h"
#include "CaseSuspect/Witness.h"
#include "Evidence/EvidenceList.h"
#include "Evidence/PhysicalEvidence.h"
#include "Evidence/DigitalEvidence.h"
#include "Evidence/TestimonialEvidence.h"
#include "Analysis/AnalysisEngine.h"
#include "Report/InvestigationQueue.h"
#include "Report/ReportGenerator.h"
#include "SampleData.h"
#include "Utils.h"

namespace {

void printBanner() {
    std::cout << "\n========================================\n";
    std::cout << "               CASE-X\n";
    std::cout << "   CRIME CASE ANALYSIS SYSTEM\n";
    std::cout << "   (Fictional / Academic OOP+DSA Project)\n";
    std::cout << "========================================\n";
}

void printMenu() {
    std::cout << "\n----------------- MAIN MENU -----------------\n";
    std::cout << "1. Create New Case\n";
    std::cout << "2. Load Sample Case\n";
    std::cout << "3. View Case\n";
    std::cout << "4. View Suspects\n";
    std::cout << "5. View Evidence\n";
    std::cout << "6. Analyze Case\n";
    std::cout << "7. View Final Report\n";
    std::cout << "8. Exit\n";
    std::cout << "----------------------------------------------\n";
    std::cout << "Enter choice: ";
}

void discardLine() {
    char c;
    while (std::cin.get(c) && c != '\n') {}
}

int readInt(const std::string& prompt, int minVal, int maxVal) {
    int value;
    while (true) {
        std::cout << prompt;
        if (std::cin >> value && value >= minVal && value <= maxVal) {
            discardLine();
            return value;
        }
        if (std::cin.eof()) {
            std::cout << "\nInput ended. Exiting CASE-X.\n";
            std::exit(0);
        }
        std::cout << "Invalid input. Please enter a number between "
                  << minVal << " and " << maxVal << ".\n";
        std::cin.clear();
        discardLine();
    }
}

std::string readLine(const std::string& prompt) {
    std::string line;
    std::cout << prompt;
    std::getline(std::cin, line);
    return line;
}

Suspect* readSuspectFromUser(int index) {
    std::cout << "\n-- Suspect " << index << " of 4 --\n";
    std::string name = readLine("  Name: ");
    int age = readInt("  Age: ", 1, 120);
    std::string occupation = readLine("  Occupation: ");
    std::string location = readLine("  Location during crime (free text): ");

    std::cout << "  Distance from crime scene:\n"
                 "    1=At scene 2=<100m 3=100-500m 4=>500m 5=Unknown\n";
    int distance = readInt("  Choice: ", 1, 5);

    std::cout << "  Access to crime scene: 0=No 1=Possible 2=Confirmed\n";
    int access = readInt("  Choice: ", 0, 2);

    std::cout << "  Motive strength: 0=None 1=Weak 2=Moderate 3=Strong 4=Very Strong\n";
    int motive = readInt("  Choice: ", 0, 4);

    std::cout << "  Opportunity level: 0=None 1=Low 2=Moderate 3=High 4=Very High\n";
    int opportunity = readInt("  Choice: ", 0, 4);

    std::cout << "  Alibi status: 0=Strong 1=Moderate 2=Weak 3=None\n";
    int alibiStatus = readInt("  Choice: ", 0, 3);

    std::string alibiStatement = readLine("  Alibi statement (what the suspect claims): ");

    return new Suspect(name, age, occupation, location, distance, access,
                        motive, opportunity, alibiStatus, alibiStatement);
}

Evidence* readEvidenceFromUser(int id) {
    std::cout << "\n-- Evidence E" << id << " --\n";
    std::cout << "  Type: 1=Physical 2=Digital 3=Testimonial\n";
    int type = readInt("  Choice: ", 1, 3);

    std::string description = readLine("  Description: ");

    std::cout << "  Reliability: 1=Low 2=Medium 3=High\n";
    int relChoice = readInt("  Choice: ", 1, 3);
    Reliability reliability = static_cast<Reliability>(relChoice);

    int effectChoice = readInt("  Effect: 1=Supports suspicion 2=Weakens suspicion\n  Choice: ", 1, 2);
    bool supports = (effectChoice == 1);

    Evidence* e = nullptr;
    if (type == 1) {
        std::string locationFound = readLine("  Location found: ");
        e = new PhysicalEvidence(id, description, reliability, supports, locationFound);
    } else if (type == 2) {
        std::string source = readLine("  Source device: ");
        int verified = readInt("  Verified authentic? 1=Yes 0=No: ", 0, 1);
        e = new DigitalEvidence(id, description, reliability, supports, source, verified == 1);
    } else {
        std::string sourcePerson = readLine("  Source person: ");
        e = new TestimonialEvidence(id, description, reliability, supports, sourcePerson);
    }

    std::string relatedName = readLine("  Related suspect name (must match a suspect exactly): ");
    e->addRelatedSuspect(relatedName);

    return e;
}

void createNewCase(CrimeCase& crimeCase, EvidenceList& evidenceList,
                    InvestigationQueue& eventQueue, bool& caseReady, bool& analyzed) {
    std::cout << "\n=== CREATE NEW CASE ===\n";
    std::string type = readLine("Crime type: ");
    std::string location = readLine("Crime location: ");
    std::string when = readLine("Date/time: ");
    std::string desc = readLine("Crime description: ");
    crimeCase.setCaseInfo(type, location, when, desc);
    eventQueue.enqueue(InvestigationEvent(eventQueue.getCount() + 1, "Case Created"));

    std::cout << "\nNow enter exactly 4 suspects.\n";
    for (int i = 1; i <= MAX_SUSPECTS; ++i) {
        crimeCase.addSuspect(readSuspectFromUser(i));
    }
    eventQueue.enqueue(InvestigationEvent(eventQueue.getCount() + 1, "4 Suspects Added"));

    int evidenceCount = readInt("\nHow many pieces of evidence would you like to add? ", 0, 20);
    for (int i = 1; i <= evidenceCount; ++i) {
        Evidence* e = readEvidenceFromUser(evidenceList.nextAvailableId());
        evidenceList.addEvidence(e);
        eventQueue.enqueue(InvestigationEvent(eventQueue.getCount() + 1,
                            "Evidence Added: E" + intToString(e->getId())));
    }

    caseReady = true;
    analyzed = false;
    std::cout << "\nCase created successfully.\n";
}

void analyzeCase(CrimeCase& crimeCase, EvidenceList& evidenceList,
                  AnalysisResult results[], int& resultCount,
                  InvestigationQueue& eventQueue, bool caseReady, bool& analyzed) {
    if (!caseReady) {
        std::cout << "\nNo case loaded yet. Use option 1 or 2 first.\n";
        return;
    }
    AnalysisEngine::analyzeCase(crimeCase, evidenceList, results, resultCount);
    AnalysisEngine::rankSuspects(results, resultCount);
    analyzed = true;
    eventQueue.enqueue(InvestigationEvent(eventQueue.getCount() + 1, "Analysis Performed"));

    std::cout << "\n=== ANALYSIS COMPLETE ===\n";
    for (int i = 0; i < resultCount; ++i) {
        std::cout << "  " << (i + 1) << ". " << results[i].suspectName
                  << " -- " << results[i].suspicionScore << "/100\n";
    }
    std::cout << "(Use option 7 to view the full explainable report.)\n";
}

void resetSession(CrimeCase*& c, EvidenceList*& e, InvestigationQueue*& q) {
    delete c;
    delete e;
    delete q;
    c = new CrimeCase();
    e = new EvidenceList();
    q = new InvestigationQueue();
}

}

int main() {
    CrimeCase* crimeCase = new CrimeCase();
    EvidenceList* evidenceList = new EvidenceList();
    InvestigationQueue* eventQueue = new InvestigationQueue();
    AnalysisResult results[MAX_SUSPECTS];
    int resultCount = 0;

    bool caseReady = false;
    bool analyzed = false;

    printBanner();

    bool running = true;
    while (running) {
        printMenu();
        int choice = readInt("", 1, 8);

        switch (choice) {
            case 1:
                resetSession(crimeCase, evidenceList, eventQueue);
                analyzed = false;
                caseReady = false;
                createNewCase(*crimeCase, *evidenceList, *eventQueue, caseReady, analyzed);
                break;

            case 2:
                resetSession(crimeCase, evidenceList, eventQueue);
                loadSampleCase(*crimeCase, *evidenceList, *eventQueue);
                caseReady = true;
                analyzed = false;
                std::cout << "\nSample case loaded: \"" << crimeCase->getCrimeType()
                          << "\" at " << crimeCase->getCrimeLocation() << ".\n";
                std::cout << "Try: 6 (Analyze Case) then 7 (View Final Report).\n";
                break;

            case 3:
                if (!caseReady) std::cout << "\nNo case loaded yet.\n";
                else crimeCase->displayCaseInfo();
                break;

            case 4:
                if (!caseReady) {
                    std::cout << "\nNo case loaded yet.\n";
                } else {
                    std::cout << "\n  1. View all suspects (and witnesses)\n";
                    std::cout << "  2. Search suspect by name\n";
                    int sub = readInt("  Choice: ", 1, 2);
                    if (sub == 1) {
                        crimeCase->displaySuspects();
                        crimeCase->displayWitnesses();
                    } else {
                        std::string name = readLine("  Enter suspect name: ");
                        Suspect* found = crimeCase->searchSuspectByName(name);
                        if (found) found->display();
                        else std::cout << "  No suspect found with that name.\n";
                    }
                }
                break;

            case 5:
                if (!caseReady) {
                    std::cout << "\nNo case loaded yet.\n";
                } else {
                    std::cout << "\n  1. View all evidence\n";
                    std::cout << "  2. Search evidence by ID\n";
                    std::cout << "  3. View evidence linked to a suspect\n";
                    std::cout << "  4. Delete evidence by ID\n";
                    int sub = readInt("  Choice: ", 1, 4);
                    if (sub == 1) {
                        evidenceList->displayAll();
                    } else if (sub == 2) {
                        int id = readInt("  Enter evidence ID: ", 1, 9999);
                        Evidence* e = evidenceList->searchById(id);
                        if (e) e->display();
                        else std::cout << "  No evidence found with that ID.\n";
                    } else if (sub == 3) {
                        std::string name = readLine("  Enter suspect name: ");
                        evidenceList->displayForSuspect(name);
                    } else {
                        int id = readInt("  Enter evidence ID to delete: ", 1, 9999);
                        if (evidenceList->deleteById(id)) {
                            analyzed = false;
                            eventQueue->enqueue(InvestigationEvent(eventQueue->getCount() + 1,
                                                "Evidence Deleted: E" + intToString(id)));
                            std::cout << "  Evidence E" << id << " deleted. Run Analyze Case again.\n";
                        } else {
                            std::cout << "  No evidence found with that ID.\n";
                        }
                    }
                }
                break;

            case 6:
                analyzeCase(*crimeCase, *evidenceList, results, resultCount, *eventQueue, caseReady, analyzed);
                break;

            case 7:
                if (!analyzed) {
                    std::cout << "\nPlease run option 6 (Analyze Case) before viewing the report.\n";
                } else {
                    eventQueue->enqueue(InvestigationEvent(eventQueue->getCount() + 1, "Report Generated"));
                    ReportGenerator::generateFinalReport(*crimeCase, *evidenceList, results,
                                                          resultCount, *eventQueue);
                }
                break;

            case 8:
                std::cout << "\nExiting CASE-X. Goodbye!\n";
                running = false;
                break;
        }
    }

    delete crimeCase;
    delete evidenceList;
    delete eventQueue;
    return 0;
}
