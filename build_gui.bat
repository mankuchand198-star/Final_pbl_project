@echo off
g++ -std=c++17 -Wall -Wextra -mwindows -static -o casex_gui.exe GUI/Gui.cpp GUI/FormDialog.cpp SampleData.cpp Utils.cpp Storage/HistoryStore.cpp CaseSuspect/Person.cpp CaseSuspect/Suspect.cpp CaseSuspect/Witness.cpp CaseSuspect/CrimeCase.cpp Evidence/Evidence.cpp Evidence/PhysicalEvidence.cpp Evidence/DigitalEvidence.cpp Evidence/TestimonialEvidence.cpp Evidence/EvidenceList.cpp Analysis/AnalysisEngine.cpp Report/InvestigationEvent.cpp Report/InvestigationQueue.cpp Report/ReportGenerator.cpp -lgdi32 -luser32
if errorlevel 1 (echo Build FAILED) else (echo Built casex_gui.exe)
