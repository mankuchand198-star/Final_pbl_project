#ifndef SAMPLEDATA_H
#define SAMPLEDATA_H

#include "CaseSuspect/CrimeCase.h"
#include "Evidence/EvidenceList.h"
#include "Report/InvestigationQueue.h"

void loadSampleCase(CrimeCase& crimeCase, EvidenceList& evidenceList,
                     InvestigationQueue& eventQueue);

#endif
