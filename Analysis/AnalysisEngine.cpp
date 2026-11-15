#include "AnalysisEngine.h"
#include "../Utils.h"

namespace {

void addLine(AnalysisResult& r, const std::string& line) {
    if (r.explanationCount < MAX_EXPLANATION_LINES) r.explanationLines[r.explanationCount++] = line;
}

void addContradiction(AnalysisResult& r, const std::string& text) {
    if (r.contradictionCount < MAX_CONTRADICTIONS) r.contradictions[r.contradictionCount++] = text;
}

int motivePoints(int level) { return level * 5; }
int opportunityPoints(int level) { return level * 5; }

int locationPoints(int distanceLevel) {
    static const int table[6] = {0, 15, 11, 7, 3, 5};
    if (distanceLevel < 1 || distanceLevel > 5) return 5;
    return table[distanceLevel];
}

int accessPoints(int level) {
    static const int table[3] = {0, 7, 15};
    if (level < 0 || level > 2) return 0;
    return table[level];
}

int alibiPoints(int status) {
    static const int table[4] = {0, 3, 7, 10};
    if (status < 0 || status > 3) return 0;
    return table[status];
}

}

AnalysisResult AnalysisEngine::analyzeSuspect(Suspect* s, const EvidenceList& evidenceList) {
    AnalysisResult result;
    result.suspectName = s->getName();

    int motive = motivePoints(s->getMotiveLevel());
    int opportunity = opportunityPoints(s->getOpportunityLevel());
    int location = locationPoints(s->getDistanceLevel());
    int access = accessPoints(s->getAccessLevel());
    int alibi = alibiPoints(s->getAlibiStatus());

    int linkedCount = 0;
    double rawEvidence = 0.0;
    for (const EvidenceNode* n = evidenceList.getHead(); n != nullptr; n = n->next) {
        Evidence* e = n->data;
        if (!e->isRelatedTo(s->getName())) continue;
        linkedCount++;
        rawEvidence += e->getStrengthScore() * 3.0;
    }
    if (rawEvidence > 20.0) rawEvidence = 20.0;
    if (rawEvidence < -20.0) rawEvidence = -20.0;

    int evidencePoints = static_cast<int>(rawEvidence >= 0 ? rawEvidence + 0.5 : rawEvidence - 0.5);

    std::string alibiText = s->getAlibiStatement();
    int contradictionPenalty = 0;
    bool claimsElsewhere = containsIgnoreCase(alibiText, "home") || containsIgnoreCase(alibiText, "away") ||
                            containsIgnoreCase(alibiText, "not near") || containsIgnoreCase(alibiText, "not at");
    if (claimsElsewhere) {
        for (const EvidenceNode* n = evidenceList.getHead(); n != nullptr; n = n->next) {
            Evidence* e = n->data;
            if (!e->isRelatedTo(s->getName())) continue;
            if (e->getSupportsSuspicion() &&
                (containsIgnoreCase(e->getDescription(), "scene") || containsIgnoreCase(e->getDescription(), "cctv"))) {
                addContradiction(result,
                    "Suspect claims to have been elsewhere (\"" + alibiText +
                    "\"), but " + e->getType() + " [E" + intToString(e->getId()) +
                    "] places them at/near the crime scene.");
                contradictionPenalty += 8;
            }
        }
    }

    if (s->getAlibiStatus() == 0 && s->getDistanceLevel() == 1) {
        addContradiction(result,
            "Suspect's alibi is recorded as Strong, but case data places them "
            "at the scene of the crime, which is inconsistent.");
        contradictionPenalty += 8;
    }

    int total = motive + opportunity + location + access + alibi + evidencePoints + contradictionPenalty;
    if (total > 100) total = 100;
    if (total < 0) total = 0;
    result.suspicionScore = total;

    addLine(result, "Motive (" + s->motiveLevelText() + "): +" + intToString(motive) + " pts");
    addLine(result, "Opportunity (" + s->opportunityLevelText() + "): +" + intToString(opportunity) + " pts");
    addLine(result, "Location (" + s->distanceLevelText() + "): +" + intToString(location) + " pts");
    addLine(result, "Access (" + s->accessLevelText() + "): +" + intToString(access) + " pts");
    addLine(result, "Alibi (" + s->alibiStatusText() + "): +" + intToString(alibi) + " pts");
    addLine(result, "Evidence (" + intToString(linkedCount) + " item(s) linked): " +
                    (evidencePoints >= 0 ? "+" : "") + intToString(evidencePoints) + " pts");
    if (contradictionPenalty > 0) {
        addLine(result, "Contradiction penalty: +" + intToString(contradictionPenalty) + " pts");
    }
    addLine(result, "TOTAL Suspicion Score: " + intToString(result.suspicionScore) + " / 100");

    return result;
}

void AnalysisEngine::analyzeCase(CrimeCase& crimeCase, const EvidenceList& evidenceList,
                                  AnalysisResult results[], int& resultCount) {
    resultCount = crimeCase.getSuspectCount();
    for (int i = 0; i < resultCount; ++i) {
        results[i] = analyzeSuspect(crimeCase.getSuspect(i), evidenceList);
    }
}

void AnalysisEngine::rankSuspects(AnalysisResult results[], int count) {
    for (int i = 0; i < count - 1; ++i) {
        int maxIndex = i;
        for (int j = i + 1; j < count; ++j) {
            if (results[j].suspicionScore > results[maxIndex].suspicionScore) {
                maxIndex = j;
            }
        }
        if (maxIndex != i) {
            AnalysisResult temp = results[i];
            results[i] = results[maxIndex];
            results[maxIndex] = temp;
        }
    }
}
