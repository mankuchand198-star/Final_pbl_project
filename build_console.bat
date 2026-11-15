@echo off
g++ -std=c++17 -Wall -Wextra -o casex.exe main.cpp SampleData.cpp Utils.cpp CaseSuspect/Person.cpp CaseSuspect/Suspect.cpp CaseSuspect/Witness.cpp CaseSuspect/CrimeCase.cpp Evidence/Evidence.cpp Evidence/PhysicalEvidence.cpp Evidence/DigitalEvidence.cpp Evidence/TestimonialEvidence.cpp Evidence/EvidenceList.cpp Analysis/AnalysisEngine.cpp Report/InvestigationEvent.cpp Report/InvestigationQueue.cpp Report/ReportGenerator.cpp
if errorlevel 1 (echo Build FAILED) else (echo Built casex.exe)
