# CASE-X - Rule-Based Crime Case Analysis and Suspect Ranking System

Final version (Phase 3): C++ console application plus a Windows GUI front end written only in C++ (Win32 API).
No STL containers or algorithms are used (only std::string and iostream).

## Build (Windows, MinGW g++, run inside this folder)

GUI version:

    .\build_gui.bat
    .\casex_gui.exe

Console version:

    .\build_console.bat
    .\casex.exe

If the static build fails, remove `-static` from build_gui.bat.

## Quick demo

GUI: Start Investigation -> Load Sample Case -> Analyze Case -> Final Report
Then open History from the top bar to see the saved case.
Console: 2 -> 6 -> 7 -> 8

## Folders

- CaseSuspect/  Module 1: Person, Suspect, Witness, CrimeCase
- Evidence/     Module 2: Evidence hierarchy and custom linked list (add, search, delete)
- Analysis/     Module 3: scoring, contradiction detection, selection sort ranking
- Report/       Module 4: custom queue and report generator
- Storage/      Saves analysed cases to casex_history.dat (plain text, next to the exe) and loads them back
- GUI/          Front end: Win32 window with Home, Workspace and History pages, reusable form dialog, output capture
- main.cpp      Console menu

## Case history

Every time Analyze Case is run, the case (suspects, evidence, timeline) is saved to casex_history.dat
in the same folder as casex_gui.exe. Delete that file to clear the history.
