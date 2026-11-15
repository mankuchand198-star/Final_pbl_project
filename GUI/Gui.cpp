#include <windows.h>
#include <string>

#include "../CaseSuspect/CrimeCase.h"
#include "../Evidence/EvidenceList.h"
#include "../Evidence/PhysicalEvidence.h"
#include "../Evidence/DigitalEvidence.h"
#include "../Evidence/TestimonialEvidence.h"
#include "../Analysis/AnalysisEngine.h"
#include "../Report/InvestigationQueue.h"
#include "../Report/ReportGenerator.h"
#include "../Storage/HistoryStore.h"
#include "../SampleData.h"
#include "../Utils.h"
#include "FormDialog.h"
#include "OutputCapture.h"

#ifndef ODS_NOFOCUSRECT
#define ODS_NOFOCUSRECT 0x0200
#endif
#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x020A
#endif

namespace {

enum {
    ID_LOAD = 101, ID_NEW, ID_VIEWCASE, ID_VIEWSUS, ID_SEARCHSUS,
    ID_ADDEV, ID_VIEWEV, ID_SEARCHEV, ID_EVSUS, ID_DELEV,
    ID_ANALYZE, ID_REPORT, ID_TIMELINE, ID_EXIT,
    ID_HOME_START = 201, ID_HOME_HISTORY, ID_NAV_HOME, ID_NAV_HISTORY, ID_NAV_WORK,
    ID_HIST_OPEN, ID_HIST_DELETE
};

enum { PAGE_HOME = 0, PAGE_WORK = 1, PAGE_HISTORY = 2 };

const int SIDEBAR_W = 240;
const int HEADER_H = 76;
const char* MAIN_CLASS = "CaseXMainWindow";
const char* RANK_CLASS = "CaseXRankPanel";
const char* HEADER_CLASS = "CaseXHeaderBar";
const char* CARD_CLASS = "CaseXCard";
const char* HOME_CLASS = "CaseXHomePage";
const char* HIST_CLASS = "CaseXHistoryPage";

const COLORREF COL_NAVY = RGB(23, 32, 56);
const COLORREF COL_HERO = RGB(15, 23, 42);
const COLORREF COL_BG = RGB(243, 245, 249);
const COLORREF COL_WHITE = RGB(255, 255, 255);
const COLORREF COL_BORDER = RGB(224, 228, 236);
const COLORREF COL_TEXT = RGB(30, 41, 59);
const COLORREF COL_MUTED = RGB(100, 116, 139);
const COLORREF COL_ACCENT = RGB(59, 130, 246);

CrimeCase* g_case = nullptr;
EvidenceList* g_evidence = nullptr;
InvestigationQueue* g_queue = nullptr;
HistoryStore* g_store = nullptr;
AnalysisResult g_results[MAX_SUSPECTS];
int g_resultCount = 0;
int g_caseId = -1;
bool g_caseReady = false;
bool g_analyzed = false;
int g_page = PAGE_HOME;
int g_histSelected = -1;
int g_histScroll = 0;

HWND g_main = NULL;
HWND g_header = NULL;
HWND g_rank = NULL;
HWND g_card = NULL;
HWND g_output = NULL;
HWND g_home = NULL;
HWND g_hist = NULL;
HWND g_btnHomeStart = NULL;
HWND g_btnHomeHistory = NULL;
HWND g_btnWorkHome = NULL;
HWND g_btnWorkHistory = NULL;
HWND g_btnHistHome = NULL;
HWND g_btnHistWork = NULL;
HWND g_btnHistOpen = NULL;
HWND g_btnHistDelete = NULL;
HWND g_hoverWnd = NULL;
WNDPROC g_oldButtonProc = NULL;

HFONT g_font = NULL;
HFONT g_boldFont = NULL;
HFONT g_titleFont = NULL;
HFONT g_smallFont = NULL;
HFONT g_cardFont = NULL;
HFONT g_scoreFont = NULL;
HFONT g_monoFont = NULL;
HFONT g_heroFont = NULL;
HFONT g_leadFont = NULL;

const int MAX_PAGE_WINS = 32;
HWND g_pageWins[3][MAX_PAGE_WINS];
int g_pageWinCount[3] = {0, 0, 0};

struct SectionLabel {
    int y;
    const char* text;
};
SectionLabel g_sections[4];
int g_sectionCount = 0;
int g_sidebarBottom = 0;

std::string g_namesSoFar[MAX_SUSPECTS];
int g_namesCount = 0;

template <typename F>
std::string capture(F fn) {
    OutputCapture cap;
    fn();
    return cap.text();
}

std::string exeDirPath(const char* filename) {
    char buf[MAX_PATH];
    DWORD n = GetModuleFileNameA(NULL, buf, MAX_PATH);
    std::string p(buf, n);
    size_t slash = p.find_last_of("\\/");
    if (slash == std::string::npos) return std::string(filename);
    return p.substr(0, slash + 1) + filename;
}

void resetSession() {
    delete g_case;
    delete g_evidence;
    delete g_queue;
    g_case = new CrimeCase();
    g_evidence = new EvidenceList();
    g_queue = new InvestigationQueue();
    g_resultCount = 0;
    g_caseId = -1;
    g_caseReady = false;
    g_analyzed = false;
}

void showText(const std::string& text) {
    std::string out;
    out.reserve(text.size() + text.size() / 8);
    for (size_t i = 0; i < text.size(); ++i) {
        if (text[i] == '\n') out += "\r\n";
        else out += text[i];
    }
    SetWindowTextA(g_output, out.c_str());
    SendMessageA(g_output, EM_SETSEL, 0, 0);
    SendMessageA(g_output, EM_SCROLLCARET, 0, 0);
}

void updateStatus() {
    InvalidateRect(g_header, NULL, TRUE);
}

void refreshAll() {
    updateStatus();
    InvalidateRect(g_rank, NULL, TRUE);
}

bool needCase() {
    if (g_caseReady) return true;
    MessageBoxA(g_main, "No case is loaded yet.\n\nClick 'Load Sample Case' or 'New Case' first.",
                "CASE-X", MB_OK | MB_ICONINFORMATION);
    return false;
}

std::string suspectNames() {
    std::string names;
    for (int i = 0; i < g_case->getSuspectCount(); ++i) {
        names += g_case->getSuspect(i)->getName();
        if (i + 1 < g_case->getSuspectCount()) names += "|";
    }
    return names;
}

void saveCurrentCase() {
    if (!g_caseReady || g_resultCount == 0) return;
    if (g_caseId < 0) g_caseId = g_store->nextId();
    std::string body = serializeCase(*g_case, *g_evidence, *g_queue);
    std::string title = g_case->getCrimeType() + " at " + g_case->getCrimeLocation();
    g_store->upsert(g_caseId, currentTimestamp(), title, g_results[0].suspectName,
                    g_results[0].suspicionScore, g_case->getSuspectCount(), g_evidence->getCount(), body);
    if (!g_store->save()) {
        MessageBoxA(g_main, "Could not write the case history file.\n\nThe case stays open, but it will not be kept in History.",
                    "CASE-X", MB_OK | MB_ICONWARNING);
    }
}

bool validateNotEmpty(const std::string* v, int count, std::string& err) {
    for (int i = 0; i < count; ++i) {
        if (v[i].size() == 0) {
            err = "Please fill in every field.";
            return false;
        }
    }
    return true;
}

bool validatePositiveNumber(const std::string* v, int, std::string& err) {
    int n = 0;
    if (!parseInt(v[0], n) || n < 1) {
        err = "Please enter a whole number of 1 or more.";
        return false;
    }
    return true;
}

bool validateSuspect(const std::string* v, int, std::string& err) {
    if (v[0].size() == 0) {
        err = "Name cannot be empty.";
        return false;
    }
    for (int i = 0; i < g_namesCount; ++i) {
        if (toLowerStr(g_namesSoFar[i]) == toLowerStr(v[0])) {
            err = "Two suspects cannot have the same name.";
            return false;
        }
    }
    int age = 0;
    if (!parseInt(v[1], age) || age < 1 || age > 120) {
        err = "Age must be a number between 1 and 120.";
        return false;
    }
    if (v[2].size() == 0 || v[3].size() == 0 || v[9].size() == 0) {
        err = "Please fill in occupation, location and the alibi statement.";
        return false;
    }
    return true;
}

bool validateEvidence(const std::string* v, int, std::string& err) {
    if (v[1].size() == 0) {
        err = "Please describe the evidence.";
        return false;
    }
    if (v[4].size() == 0) {
        err = "Please fill in the location / device / person field.";
        return false;
    }
    return true;
}

int comboValue(const std::string& s) {
    int n = 0;
    parseInt(s, n);
    return n;
}

void onLoadSample() {
    resetSession();
    loadSampleCase(*g_case, *g_evidence, *g_queue);
    g_caseId = 0;
    g_caseReady = true;
    refreshAll();
    std::string t = "SAMPLE CASE LOADED\n\n";
    t += capture([&]() { g_case->displayCaseInfo(); });
    t += "\nNext: click 'Analyze Case', then 'Final Report'.\n";
    showText(t);
}

void onNewCase() {
    FormField caseFields[4] = {
        {"Crime type", FORM_TEXT, ""},
        {"Crime location", FORM_TEXT, ""},
        {"Date / time", FORM_TEXT, ""},
        {"Description", FORM_TEXT, ""}
    };
    std::string caseVals[4];
    if (!showForm(g_main, "New Case - Case Details", caseFields, 4, caseVals, validateNotEmpty)) return;

    FormField sf[10] = {
        {"Name", FORM_TEXT, ""},
        {"Age", FORM_TEXT, ""},
        {"Occupation", FORM_TEXT, ""},
        {"Location during the crime", FORM_TEXT, ""},
        {"Distance from crime scene", FORM_COMBO, "At the scene|Less than 100 m|100 - 500 m|More than 500 m|Unknown"},
        {"Access to crime scene", FORM_COMBO, "No access|Possible access|Confirmed access"},
        {"Motive strength", FORM_COMBO, "None|Weak|Moderate|Strong|Very Strong"},
        {"Opportunity", FORM_COMBO, "None|Low|Moderate|High|Very High"},
        {"Alibi status", FORM_COMBO, "Strong|Moderate|Weak|None"},
        {"Alibi statement", FORM_TEXT, ""}
    };
    std::string sv[MAX_SUSPECTS][10];
    g_namesCount = 0;
    for (int i = 0; i < MAX_SUSPECTS; ++i) {
        std::string title = "New Case - Suspect " + intToString(i + 1) + " of " + intToString(MAX_SUSPECTS);
        if (!showForm(g_main, title.c_str(), sf, 10, sv[i], validateSuspect)) {
            g_namesCount = 0;
            return;
        }
        g_namesSoFar[g_namesCount++] = sv[i][0];
    }
    g_namesCount = 0;

    resetSession();
    g_case->setCaseInfo(caseVals[0], caseVals[1], caseVals[2], caseVals[3]);
    g_queue->enqueue(InvestigationEvent(g_queue->getCount() + 1, "Case Created"));
    for (int i = 0; i < MAX_SUSPECTS; ++i) {
        g_case->addSuspect(new Suspect(sv[i][0], comboValue(sv[i][1]), sv[i][2], sv[i][3],
                                        comboValue(sv[i][4]) + 1, comboValue(sv[i][5]),
                                        comboValue(sv[i][6]), comboValue(sv[i][7]),
                                        comboValue(sv[i][8]), sv[i][9]));
    }
    g_queue->enqueue(InvestigationEvent(g_queue->getCount() + 1, "4 Suspects Added"));
    g_caseReady = true;
    refreshAll();

    std::string t = "NEW CASE CREATED\n\n";
    t += capture([&]() { g_case->displayCaseInfo(); });
    t += "\n";
    t += capture([&]() { g_case->displaySuspects(); });
    t += "\nNext: click 'Add Evidence' to add evidence, then 'Analyze Case'.\n";
    showText(t);
}

void onViewCase() {
    if (!needCase()) return;
    showText(capture([&]() { g_case->displayCaseInfo(); }));
}

void onViewSuspects() {
    if (!needCase()) return;
    std::string t = capture([&]() { g_case->displaySuspects(); });
    t += "\n";
    t += capture([&]() { g_case->displayWitnesses(); });
    showText(t);
}

void onSearchSuspect() {
    if (!needCase()) return;
    FormField f[1] = {{"Suspect name", FORM_TEXT, ""}};
    std::string v[1];
    if (!showForm(g_main, "Search Suspect by Name", f, 1, v, validateNotEmpty)) return;
    Suspect* found = g_case->searchSuspectByName(v[0]);
    if (found == nullptr) {
        showText("No suspect found with the name \"" + v[0] + "\".\n");
    } else {
        showText("SEARCH RESULT\n\n" + capture([&]() { found->display(); }));
    }
}

void onAddEvidence() {
    if (!needCase()) return;
    std::string names = suspectNames();
    FormField f[8] = {
        {"Evidence type", FORM_COMBO, "Physical|Digital|Testimonial"},
        {"Description", FORM_TEXT, ""},
        {"Reliability", FORM_COMBO, "Low|Medium|High"},
        {"Effect on suspicion", FORM_COMBO, "Supports suspicion|Weakens suspicion"},
        {"Location / device / person", FORM_TEXT, ""},
        {"Verified? (Digital only)", FORM_COMBO, "Yes|No"},
        {"Related suspect", FORM_COMBO, names},
        {"Also related to (optional)", FORM_COMBO, "(none)|" + names}
    };
    std::string v[8];
    if (!showForm(g_main, "Add Evidence", f, 8, v, validateEvidence)) return;

    int type = comboValue(v[0]);
    Reliability reliability = static_cast<Reliability>(comboValue(v[2]) + 1);
    bool supports = (comboValue(v[3]) == 0);
    bool verified = (comboValue(v[5]) == 0);
    int first = comboValue(v[6]);
    int second = comboValue(v[7]);
    int id = g_evidence->nextAvailableId();

    Evidence* e = nullptr;
    if (type == 0) e = new PhysicalEvidence(id, v[1], reliability, supports, v[4]);
    else if (type == 1) e = new DigitalEvidence(id, v[1], reliability, supports, v[4], verified);
    else e = new TestimonialEvidence(id, v[1], reliability, supports, v[4]);

    e->addRelatedSuspect(g_case->getSuspect(first)->getName());
    if (second > 0 && second - 1 != first) e->addRelatedSuspect(g_case->getSuspect(second - 1)->getName());
    g_evidence->addEvidence(e);
    g_queue->enqueue(InvestigationEvent(g_queue->getCount() + 1, "Evidence Added: E" + intToString(id)));
    g_analyzed = false;
    refreshAll();
    showText("EVIDENCE ADDED (E" + intToString(id) + ")\n\n" + capture([&]() { g_evidence->displayAll(); }) +
             "\nRun 'Analyze Case' again to update the ranking.\n");
}

void onViewEvidence() {
    if (!needCase()) return;
    showText(capture([&]() { g_evidence->displayAll(); }));
}

void onSearchEvidence() {
    if (!needCase()) return;
    FormField f[1] = {{"Evidence ID", FORM_TEXT, ""}};
    std::string v[1];
    if (!showForm(g_main, "Search Evidence by ID", f, 1, v, validatePositiveNumber)) return;
    Evidence* e = g_evidence->searchById(comboValue(v[0]));
    if (e == nullptr) {
        showText("No evidence found with ID " + v[0] + ".\n");
    } else {
        showText("SEARCH RESULT\n\n" + capture([&]() { e->display(); }));
    }
}

void onEvidenceOfSuspect() {
    if (!needCase()) return;
    FormField f[1] = {{"Suspect", FORM_COMBO, suspectNames()}};
    std::string v[1];
    if (!showForm(g_main, "Evidence Linked to a Suspect", f, 1, v, nullptr)) return;
    std::string name = g_case->getSuspect(comboValue(v[0]))->getName();
    showText("EVIDENCE LINKED TO " + name + "\n\n" + capture([&]() { g_evidence->displayForSuspect(name); }));
}

void onDeleteEvidence() {
    if (!needCase()) return;
    FormField f[1] = {{"Evidence ID to delete", FORM_TEXT, ""}};
    std::string v[1];
    if (!showForm(g_main, "Delete Evidence", f, 1, v, validatePositiveNumber)) return;
    int id = comboValue(v[0]);
    Evidence* e = g_evidence->searchById(id);
    if (e == nullptr) {
        MessageBoxA(g_main, "No evidence found with that ID.", "Delete Evidence", MB_OK | MB_ICONINFORMATION);
        return;
    }
    std::string question = "Delete evidence E" + intToString(id) + "?\n\n" + e->getDescription();
    if (MessageBoxA(g_main, question.c_str(), "Confirm delete", MB_YESNO | MB_ICONQUESTION) != IDYES) return;
    g_evidence->deleteById(id);
    g_queue->enqueue(InvestigationEvent(g_queue->getCount() + 1, "Evidence Deleted: E" + intToString(id)));
    g_analyzed = false;
    refreshAll();
    showText("EVIDENCE E" + intToString(id) + " DELETED\n\n" + capture([&]() { g_evidence->displayAll(); }) +
             "\nRun 'Analyze Case' again to update the ranking.\n");
}

void onAnalyze() {
    if (!needCase()) return;
    AnalysisEngine::analyzeCase(*g_case, *g_evidence, g_results, g_resultCount);
    AnalysisEngine::rankSuspects(g_results, g_resultCount);
    g_analyzed = true;
    g_queue->enqueue(InvestigationEvent(g_queue->getCount() + 1, "Analysis Performed"));
    saveCurrentCase();
    refreshAll();

    std::string t = "ANALYSIS COMPLETE\n\nRanking (Suspicion Score out of 100):\n";
    int flagged = 0;
    for (int i = 0; i < g_resultCount; ++i) {
        t += "  " + intToString(i + 1) + ". " + g_results[i].suspectName + "  -  " +
             intToString(g_results[i].suspicionScore) + "/100\n";
        flagged += g_results[i].contradictionCount;
    }
    t += "\nContradictions flagged: " + intToString(flagged) + "\n";
    t += "\nClick 'Final Report' for the full explanation of every score.\n";
    showText(t);
}

void onReport() {
    if (!needCase()) return;
    if (!g_analyzed) {
        MessageBoxA(g_main, "Please click 'Analyze Case' first.", "CASE-X", MB_OK | MB_ICONINFORMATION);
        return;
    }
    g_queue->enqueue(InvestigationEvent(g_queue->getCount() + 1, "Report Generated"));
    showText(capture([&]() {
        ReportGenerator::generateFinalReport(*g_case, *g_evidence, g_results, g_resultCount, *g_queue);
    }));
}

void onTimeline() {
    if (!needCase()) return;
    showText(capture([&]() { g_queue->displayAll(); }));
}

void fillRect(HDC dc, const RECT& r, COLORREF c) {
    HBRUSH b = CreateSolidBrush(c);
    FillRect(dc, &r, b);
    DeleteObject(b);
}

void fillRound(HDC dc, const RECT& r, COLORREF fill, COLORREF border, int radius) {
    HBRUSH brush = CreateSolidBrush(fill);
    HPEN pen = CreatePen(PS_SOLID, 1, border);
    HBRUSH oldBrush = static_cast<HBRUSH>(SelectObject(dc, brush));
    HPEN oldPen = static_cast<HPEN>(SelectObject(dc, pen));
    RoundRect(dc, r.left, r.top, r.right, r.bottom, radius, radius);
    SelectObject(dc, oldBrush);
    SelectObject(dc, oldPen);
    DeleteObject(brush);
    DeleteObject(pen);
}

void drawText(HDC dc, int x, int y, const std::string& s) {
    TextOutA(dc, x, y, s.c_str(), static_cast<int>(s.size()));
}

void drawTextIn(HDC dc, const RECT& r, const std::string& s, UINT flags) {
    RECT copy = r;
    DrawTextA(dc, s.c_str(), static_cast<int>(s.size()), &copy, flags);
}

COLORREF scoreColor(int score) {
    if (score >= 70) return RGB(239, 68, 68);
    if (score >= 40) return RGB(245, 158, 11);
    return RGB(34, 197, 94);
}

void drawBar(HDC dc, int left, int y, int width, int score) {
    RECT track = {left, y + 5, left + width, y + 19};
    fillRound(dc, track, RGB(236, 240, 246), RGB(236, 240, 246), 14);
    int fillW = (width * score) / 100;
    if (fillW < 14) fillW = 14;
    RECT bar = {left, y + 5, left + fillW, y + 19};
    COLORREF color = scoreColor(score);
    fillRound(dc, bar, color, color, 14);
}

typedef void (*PaintFn)(HDC dc, const RECT& rc);

void paintBuffered(HWND hwnd, PaintFn fn) {
    PAINTSTRUCT ps;
    HDC dc = BeginPaint(hwnd, &ps);
    RECT rc;
    GetClientRect(hwnd, &rc);
    if (rc.right > 0 && rc.bottom > 0) {
        HDC mem = CreateCompatibleDC(dc);
        HBITMAP bmp = CreateCompatibleBitmap(dc, rc.right, rc.bottom);
        HBITMAP old = static_cast<HBITMAP>(SelectObject(mem, bmp));
        fn(mem, rc);
        BitBlt(dc, 0, 0, rc.right, rc.bottom, mem, 0, 0, SRCCOPY);
        SelectObject(mem, old);
        DeleteObject(bmp);
        DeleteDC(mem);
    }
    EndPaint(hwnd, &ps);
}

struct HomeGeo {
    int pad;
    int contentRight;
    int textRight;
    int btnY;
    int footerY;
    int dy;
    bool showPreview;
    RECT preview;
    RECT feat[3];
};

HomeGeo homeGeometry(int w, int h) {
    HomeGeo g;
    int contentW = w - 112;
    if (contentW > 1180) contentW = 1180;
    g.pad = (w - contentW) / 2;
    g.contentRight = g.pad + contentW;
    g.dy = (h > 700) ? (h - 700) / 3 : 0;
    if (g.dy > 120) g.dy = 120;
    g.showPreview = (w >= 1100);
    const int previewW = 380;
    g.textRight = g.showPreview ? g.contentRight - previewW - 40 : g.contentRight;
    g.preview.left = g.contentRight - previewW;
    g.preview.top = 104 + g.dy;
    g.preview.right = g.contentRight;
    g.preview.bottom = 394 + g.dy;
    g.btnY = 362 + g.dy;
    int featTop = g.btnY + 52 + 70;
    if (featTop > h - 250) featTop = h - 250;
    if (featTop < 440 + g.dy) featTop = 440 + g.dy;
    int fw = (contentW - 40) / 3;
    for (int i = 0; i < 3; ++i) {
        g.feat[i].left = g.pad + i * (fw + 20);
        g.feat[i].top = featTop;
        g.feat[i].right = g.feat[i].left + fw;
        g.feat[i].bottom = featTop + 150;
    }
    g.footerY = h - 52;
    return g;
}

void paintHome(HDC dc, const RECT& rc) {
    HomeGeo g = homeGeometry(rc.right, rc.bottom);
    fillRect(dc, rc, COL_HERO);
    SetBkMode(dc, TRANSPARENT);

    SelectObject(dc, g_titleFont);
    SetTextColor(dc, COL_WHITE);
    drawText(dc, g.pad, 26, "CASE-X");
    SelectObject(dc, g_smallFont);
    SetTextColor(dc, RGB(148, 163, 184));
    drawText(dc, g.pad + 122, 36, "Crime Case Analysis");

    RECT tag = {g.pad, 104 + g.dy, g.pad + 250, 130 + g.dy};
    fillRound(dc, tag, RGB(30, 58, 138), RGB(30, 58, 138), 26);
    SelectObject(dc, g_boldFont);
    SetTextColor(dc, RGB(147, 197, 253));
    drawTextIn(dc, tag, "RULE-BASED CASE ANALYSIS", DT_CENTER | DT_VCENTER | DT_SINGLELINE);

    SelectObject(dc, g_heroFont);
    SetTextColor(dc, COL_WHITE);
    drawText(dc, g.pad, 140 + g.dy, "Rank suspects.");
    SetTextColor(dc, RGB(96, 165, 250));
    drawText(dc, g.pad, 204 + g.dy, "Explain every score.");

    SelectObject(dc, g_leadFont);
    SetTextColor(dc, RGB(203, 213, 225));
    RECT lead = {g.pad, 280 + g.dy, g.textRight, 358 + g.dy};
    drawTextIn(dc, lead,
               "CASE-X turns a case file into a transparent Suspicion Score for every suspect, "
               "flags alibis that the evidence contradicts, and writes a report you can trace line by line. "
               "No black box.",
               DT_LEFT | DT_WORDBREAK);

    if (g.showPreview) {
        RECT card = g.preview;
        fillRound(dc, card, COL_WHITE, COL_WHITE, 24);
        SelectObject(dc, g_cardFont);
        SetTextColor(dc, COL_TEXT);
        drawText(dc, card.left + 22, card.top + 16, "Sample ranking");
        SelectObject(dc, g_smallFont);
        SetTextColor(dc, COL_MUTED);
        drawText(dc, card.left + 22, card.top + 44, "Preview of the built-in sample case");

        const char* names[4] = {"Rahul Verma", "Aman Gupta", "Vijay Singh", "Rohit Malhotra"};
        const int scores[4] = {96, 44, 17, 3};
        for (int i = 0; i < 4; ++i) {
            int y = card.top + 84 + i * 40;
            RECT circle = {card.left + 22, y, card.left + 46, y + 24};
            fillRound(dc, circle, COL_NAVY, COL_NAVY, 24);
            SelectObject(dc, g_boldFont);
            SetTextColor(dc, COL_WHITE);
            drawTextIn(dc, circle, intToString(i + 1), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
            SelectObject(dc, g_font);
            SetTextColor(dc, COL_TEXT);
            RECT nameRect = {card.left + 56, y, card.left + 186, y + 24};
            drawTextIn(dc, nameRect, names[i], DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
            drawBar(dc, card.left + 192, y, 120, scores[i]);
            SelectObject(dc, g_scoreFont);
            drawText(dc, card.left + 324, y + 2, intToString(scores[i]));
        }
        SelectObject(dc, g_smallFont);
        SetTextColor(dc, COL_MUTED);
        drawText(dc, card.left + 22, card.bottom - 34, "Every score comes from fixed, visible rules.");
    }

    const char* titles[3] = {"Explainable scores", "Contradiction detection", "Saved case history"};
    const char* bodies[3] = {
        "Six fixed factors (motive, opportunity, location, access, alibi and evidence) add up to 100. Every point can be traced.",
        "Each alibi is checked against the evidence. Mismatches are flagged and shown in the report.",
        "Every analysed case is saved automatically. Reopen it any time from the History page."
    };
    for (int i = 0; i < 3; ++i) {
        RECT f = g.feat[i];
        fillRound(dc, f, RGB(24, 34, 62), RGB(51, 65, 105), 24);
        RECT dot = {f.left + 20, f.top + 20, f.left + 46, f.top + 46};
        fillRound(dc, dot, COL_ACCENT, COL_ACCENT, 26);
        SelectObject(dc, g_boldFont);
        SetTextColor(dc, COL_WHITE);
        drawTextIn(dc, dot, intToString(i + 1), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(dc, g_cardFont);
        SetTextColor(dc, COL_WHITE);
        RECT titleRect = {f.left + 58, f.top + 18, f.right - 12, f.top + 50};
        drawTextIn(dc, titleRect, titles[i], DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        SelectObject(dc, g_smallFont);
        SetTextColor(dc, RGB(148, 163, 184));
        RECT body = {f.left + 20, f.top + 62, f.right - 20, f.bottom - 12};
        drawTextIn(dc, body, bodies[i], DT_LEFT | DT_WORDBREAK);
    }

    SelectObject(dc, g_smallFont);
    SetTextColor(dc, RGB(100, 116, 139));
    RECT foot = {0, g.footerY, rc.right, g.footerY + 22};
    drawTextIn(dc, foot, "Academic project by Team Amon. Scores are rule-based and do not establish guilt.",
               DT_CENTER | DT_VCENTER | DT_SINGLELINE);
}

struct HistGeo {
    RECT card;
    int idxX, caseX, savedX, susX, evX, topX;
    int caseW, savedW, susW, evW, topW;
    int rowTop, rowH, visibleRows;
};

HistGeo histGeometry(int w, int h) {
    HistGeo g;
    g.card.left = 24;
    g.card.top = HEADER_H + 20;
    g.card.right = w - 24;
    g.card.bottom = h - 24;
    int iw = (g.card.right - g.card.left) - 48;
    int x = g.card.left + 24;
    g.idxX = x; x += 44;
    g.caseX = x; g.caseW = iw * 35 / 100; x += g.caseW + 12;
    g.savedX = x; g.savedW = iw * 19 / 100; x += g.savedW + 12;
    g.susX = x; g.susW = iw * 9 / 100; x += g.susW + 12;
    g.evX = x; g.evW = iw * 9 / 100; x += g.evW + 12;
    g.topX = x; g.topW = (g.card.right - 24) - x;
    if (g.topW < 80) g.topW = 80;
    g.rowTop = g.card.top + 52;
    g.rowH = 46;
    g.visibleRows = (g.card.bottom - g.rowTop - 12) / g.rowH;
    if (g.visibleRows < 1) g.visibleRows = 1;
    return g;
}

void histClamp(const HistGeo& g) {
    int count = g_store->getCount();
    int maxScroll = count - g.visibleRows;
    if (maxScroll < 0) maxScroll = 0;
    if (g_histScroll > maxScroll) g_histScroll = maxScroll;
    if (g_histScroll < 0) g_histScroll = 0;
    if (count == 0) g_histSelected = -1;
    else if (g_histSelected >= count) g_histSelected = count - 1;
}

void histEnsureVisible(const HistGeo& g) {
    if (g_histSelected < 0) return;
    if (g_histSelected < g_histScroll) g_histScroll = g_histSelected;
    else if (g_histSelected >= g_histScroll + g.visibleRows) g_histScroll = g_histSelected - g.visibleRows + 1;
    histClamp(g);
}

void histFix() {
    RECT rc;
    GetClientRect(g_hist, &rc);
    histClamp(histGeometry(rc.right, rc.bottom));
}

void paintHistory(HDC dc, const RECT& rc) {
    HistGeo g = histGeometry(rc.right, rc.bottom);
    histClamp(g);
    int count = g_store->getCount();
    fillRect(dc, rc, COL_BG);
    SetBkMode(dc, TRANSPARENT);

    RECT bar = {0, 0, rc.right, HEADER_H};
    fillRect(dc, bar, COL_WHITE);
    RECT line = {0, HEADER_H - 1, rc.right, HEADER_H};
    fillRect(dc, line, COL_BORDER);
    SelectObject(dc, g_titleFont);
    SetTextColor(dc, COL_TEXT);
    drawText(dc, 24, 10, "Case History");
    SelectObject(dc, g_smallFont);
    SetTextColor(dc, COL_MUTED);
    std::string sub = (count == 0) ? "No saved cases yet"
        : intToString(count) + " saved case(s)  |  newest first  |  double-click a row to open it";
    drawText(dc, 24, 48, sub);

    fillRound(dc, g.card, COL_WHITE, COL_BORDER, 16);

    SelectObject(dc, g_boldFont);
    SetTextColor(dc, COL_MUTED);
    int hy = g.card.top + 18;
    drawText(dc, g.idxX, hy, "#");
    drawText(dc, g.caseX, hy, "CASE");
    drawText(dc, g.savedX, hy, "SAVED ON");
    drawText(dc, g.susX, hy, "SUSPECTS");
    drawText(dc, g.evX, hy, "EVIDENCE");
    drawText(dc, g.topX, hy, "TOP SUSPECT");
    RECT rule = {g.card.left + 16, g.card.top + 46, g.card.right - 16, g.card.top + 47};
    fillRect(dc, rule, COL_BORDER);

    if (count == 0) {
        SelectObject(dc, g_cardFont);
        SetTextColor(dc, COL_TEXT);
        RECT t1 = {g.card.left, g.card.top + 120, g.card.right, g.card.top + 150};
        drawTextIn(dc, t1, "Nothing here yet", DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        SelectObject(dc, g_font);
        SetTextColor(dc, COL_MUTED);
        RECT t2 = {g.card.left, g.card.top + 156, g.card.right, g.card.top + 186};
        drawTextIn(dc, t2, "Analyze a case in the workspace and it is saved here automatically.", DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        return;
    }

    for (int r = 0; r < g.visibleRows; ++r) {
        int row = g_histScroll + r;
        if (row >= count) break;
        const HistoryEntry* e = g_store->getAt(row);
        if (e == nullptr) break;
        int y = g.rowTop + r * g.rowH;
        int rh = g.rowH - 4;
        if (row == g_histSelected) {
            RECT sel = {g.card.left + 10, y, g.card.right - 10, y + rh};
            fillRound(dc, sel, RGB(239, 246, 255), RGB(191, 219, 254), 12);
            RECT accent = {sel.left + 2, y + 10, sel.left + 6, y + rh - 10};
            fillRect(dc, accent, COL_ACCENT);
        }
        SelectObject(dc, g_smallFont);
        SetTextColor(dc, COL_MUTED);
        RECT idx = {g.idxX, y, g.idxX + 40, y + rh};
        drawTextIn(dc, idx, intToString(row + 1), DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        SelectObject(dc, g_font);
        SetTextColor(dc, COL_TEXT);
        std::string title = e->title;
        if (e->id == 0) title += " (sample)";
        RECT tr = {g.caseX, y, g.caseX + g.caseW, y + rh};
        drawTextIn(dc, tr, title, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

        SelectObject(dc, g_smallFont);
        SetTextColor(dc, COL_MUTED);
        RECT sv = {g.savedX, y, g.savedX + g.savedW, y + rh};
        drawTextIn(dc, sv, e->savedAt, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);

        SelectObject(dc, g_font);
        SetTextColor(dc, COL_TEXT);
        RECT sc = {g.susX, y, g.susX + g.susW, y + rh};
        drawTextIn(dc, sc, intToString(e->suspectCount), DT_LEFT | DT_VCENTER | DT_SINGLELINE);
        RECT ec = {g.evX, y, g.evX + g.evW, y + rh};
        drawTextIn(dc, ec, intToString(e->evidenceCount), DT_LEFT | DT_VCENTER | DT_SINGLELINE);

        RECT dot = {g.topX, y + rh / 2 - 5, g.topX + 10, y + rh / 2 + 5};
        fillRound(dc, dot, scoreColor(e->topScore), scoreColor(e->topScore), 10);
        RECT tp = {g.topX + 18, y, g.topX + g.topW, y + rh};
        drawTextIn(dc, tp, e->topName + "  -  " + intToString(e->topScore) + "/100",
                   DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
    }

    if (count > g.visibleRows) {
        int trackTop = g.rowTop;
        int trackH = g.visibleRows * g.rowH;
        RECT track = {g.card.right - 10, trackTop, g.card.right - 6, trackTop + trackH};
        fillRound(dc, track, RGB(236, 240, 246), RGB(236, 240, 246), 4);
        int thumbH = trackH * g.visibleRows / count;
        if (thumbH < 24) thumbH = 24;
        int maxScroll = count - g.visibleRows;
        int thumbTop = trackTop + (trackH - thumbH) * g_histScroll / (maxScroll > 0 ? maxScroll : 1);
        RECT thumb = {g.card.right - 10, thumbTop, g.card.right - 6, thumbTop + thumbH};
        fillRound(dc, thumb, RGB(148, 163, 184), RGB(148, 163, 184), 4);
    }
}

LRESULT CALLBACK HistoryProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT:
            paintBuffered(hwnd, paintHistory);
            return 0;
        case WM_LBUTTONDOWN:
        case WM_LBUTTONDBLCLK: {
            SetFocus(hwnd);
            RECT rc;
            GetClientRect(hwnd, &rc);
            HistGeo g = histGeometry(rc.right, rc.bottom);
            int x = static_cast<int>(static_cast<short>(LOWORD(lp)));
            int y = static_cast<int>(static_cast<short>(HIWORD(lp)));
            if (y >= g.rowTop && x >= g.card.left && x < g.card.right) {
                int slot = (y - g.rowTop) / g.rowH;
                int row = g_histScroll + slot;
                if (slot < g.visibleRows && row >= 0 && row < g_store->getCount()) {
                    g_histSelected = row;
                    InvalidateRect(hwnd, NULL, FALSE);
                    if (msg == WM_LBUTTONDBLCLK) SendMessageA(g_main, WM_COMMAND, ID_HIST_OPEN, 0);
                }
            }
            return 0;
        }
        case WM_MOUSEWHEEL: {
            RECT rc;
            GetClientRect(hwnd, &rc);
            HistGeo g = histGeometry(rc.right, rc.bottom);
            int delta = static_cast<int>(static_cast<short>(HIWORD(wp)));
            g_histScroll -= (delta > 0 ? 1 : -1) * 2;
            histClamp(g);
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
        case WM_KEYDOWN: {
            RECT rc;
            GetClientRect(hwnd, &rc);
            HistGeo g = histGeometry(rc.right, rc.bottom);
            int count = g_store->getCount();
            if (count == 0) return 0;
            if (wp == VK_UP && g_histSelected > 0) g_histSelected--;
            else if (wp == VK_DOWN && g_histSelected < count - 1) g_histSelected++;
            else if (wp == VK_HOME) g_histSelected = 0;
            else if (wp == VK_END) g_histSelected = count - 1;
            else if (wp == VK_RETURN) { SendMessageA(g_main, WM_COMMAND, ID_HIST_OPEN, 0); return 0; }
            else if (wp == VK_DELETE) { SendMessageA(g_main, WM_COMMAND, ID_HIST_DELETE, 0); return 0; }
            histEnsureVisible(g);
            InvalidateRect(hwnd, NULL, FALSE);
            return 0;
        }
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

LRESULT CALLBACK HomeProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_ERASEBKGND) return 1;
    if (msg == WM_PAINT) {
        paintBuffered(hwnd, paintHome);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

LRESULT CALLBACK HeaderProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_ERASEBKGND) return 1;
    if (msg == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);
        fillRect(dc, rc, COL_WHITE);
        RECT line = {0, rc.bottom - 1, rc.right, rc.bottom};
        fillRect(dc, line, COL_BORDER);
        SetBkMode(dc, TRANSPARENT);

        std::string title = "No case loaded";
        std::string sub = "Load the sample case or create a new one to begin.";
        if (g_caseReady) {
            title = g_case->getCrimeType() + " at " + g_case->getCrimeLocation();
            sub = g_case->getDateTime() + "   |   Suspects: " + intToString(g_case->getSuspectCount()) +
                  "   |   Evidence: " + intToString(g_evidence->getCount());
        }

        const int navReserve = 24 + 96 + 8 + 84 + 20;
        int pillW = 0;
        if (g_caseReady) {
            const char* label = g_analyzed ? "Analysis complete" : "Not analyzed yet";
            SelectObject(dc, g_boldFont);
            SIZE size;
            GetTextExtentPoint32A(dc, label, lstrlenA(label), &size);
            pillW = size.cx + 28;
            RECT pill = {rc.right - navReserve - pillW, 24, rc.right - navReserve, 24 + 28};
            COLORREF bg = g_analyzed ? RGB(220, 252, 231) : RGB(254, 243, 199);
            COLORREF fg = g_analyzed ? RGB(22, 101, 52) : RGB(146, 64, 14);
            fillRound(dc, pill, bg, bg, 28);
            SetTextColor(dc, fg);
            drawTextIn(dc, pill, label, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
        }

        int textRight = rc.right - navReserve - (pillW > 0 ? pillW + 16 : 0);
        SelectObject(dc, g_titleFont);
        SetTextColor(dc, COL_TEXT);
        RECT titleRect = {24, 10, textRight, 44};
        drawTextIn(dc, titleRect, title, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        SelectObject(dc, g_smallFont);
        SetTextColor(dc, COL_MUTED);
        RECT subRect = {24, 46, textRight, 68};
        drawTextIn(dc, subRect, sub, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
        EndPaint(hwnd, &ps);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

LRESULT CALLBACK RankProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_ERASEBKGND) return 1;
    if (msg == WM_PAINT) {
        PAINTSTRUCT ps;
        HDC dc = BeginPaint(hwnd, &ps);
        RECT rc;
        GetClientRect(hwnd, &rc);
        fillRect(dc, rc, COL_BG);
        RECT card = {0, 0, rc.right, rc.bottom};
        fillRound(dc, card, COL_WHITE, COL_BORDER, 16);
        SetBkMode(dc, TRANSPARENT);

        SelectObject(dc, g_cardFont);
        SetTextColor(dc, COL_TEXT);
        drawText(dc, 20, 12, "Suspicion Ranking");

        if (!g_analyzed) {
            SelectObject(dc, g_font);
            SetTextColor(dc, COL_MUTED);
            drawText(dc, 20, 58, "Load a case and click 'Analyze Case' to see the ranking here.");
        } else {
            int barLeft = 250;
            int barMax = rc.right - barLeft - 110;
            if (barMax > 620) barMax = 620;
            if (barMax < 60) barMax = 60;
            for (int i = 0; i < g_resultCount; ++i) {
                int y = 50 + i * 28;
                int score = g_results[i].suspicionScore;
                RECT circle = {20, y, 44, y + 24};
                fillRound(dc, circle, COL_NAVY, COL_NAVY, 24);
                SelectObject(dc, g_boldFont);
                SetTextColor(dc, COL_WHITE);
                drawTextIn(dc, circle, intToString(i + 1), DT_CENTER | DT_VCENTER | DT_SINGLELINE);
                SelectObject(dc, g_font);
                SetTextColor(dc, COL_TEXT);
                RECT nameRect = {56, y, barLeft - 12, y + 24};
                drawTextIn(dc, nameRect, g_results[i].suspectName, DT_LEFT | DT_VCENTER | DT_SINGLELINE | DT_END_ELLIPSIS);
                drawBar(dc, barLeft, y, barMax, score);
                SelectObject(dc, g_scoreFont);
                SetTextColor(dc, COL_TEXT);
                drawText(dc, barLeft + barMax + 14, y + 2, intToString(score) + " / 100");
            }
            SelectObject(dc, g_smallFont);
            SetTextColor(dc, COL_MUTED);
            drawText(dc, 20, rc.bottom - 26, "Rule-based score for ranking only. It does not establish actual guilt.");
        }
        EndPaint(hwnd, &ps);
        return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

LRESULT CALLBACK CardProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_ERASEBKGND:
            return 1;
        case WM_PAINT: {
            PAINTSTRUCT ps;
            HDC dc = BeginPaint(hwnd, &ps);
            RECT rc;
            GetClientRect(hwnd, &rc);
            fillRect(dc, rc, COL_BG);
            fillRound(dc, rc, COL_WHITE, COL_BORDER, 16);
            SetBkMode(dc, TRANSPARENT);
            SelectObject(dc, g_cardFont);
            SetTextColor(dc, COL_TEXT);
            drawText(dc, 20, 12, "Details & Report");
            EndPaint(hwnd, &ps);
            return 0;
        }
        case WM_SIZE:
            if (g_output != NULL) {
                MoveWindow(g_output, 18, 48, LOWORD(lp) - 36, HIWORD(lp) - 48 - 18, TRUE);
            }
            return 0;
        case WM_CTLCOLORSTATIC: {
            HDC dc = reinterpret_cast<HDC>(wp);
            SetTextColor(dc, COL_TEXT);
            SetBkColor(dc, COL_WHITE);
            return reinterpret_cast<LRESULT>(GetStockObject(WHITE_BRUSH));
        }
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

LRESULT CALLBACK ButtonProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    if (msg == WM_MOUSEMOVE && g_hoverWnd != hwnd) {
        HWND previous = g_hoverWnd;
        g_hoverWnd = hwnd;
        if (previous != NULL) InvalidateRect(previous, NULL, FALSE);
        InvalidateRect(hwnd, NULL, FALSE);
        TRACKMOUSEEVENT track;
        ZeroMemory(&track, sizeof(track));
        track.cbSize = sizeof(track);
        track.dwFlags = TME_LEAVE;
        track.hwndTrack = hwnd;
        TrackMouseEvent(&track);
    } else if (msg == WM_MOUSELEAVE) {
        if (g_hoverWnd == hwnd) g_hoverWnd = NULL;
        InvalidateRect(hwnd, NULL, FALSE);
    } else if (msg == WM_SETCURSOR) {
        SetCursor(LoadCursorA(NULL, IDC_HAND));
        return TRUE;
    }
    return CallWindowProcA(g_oldButtonProc, hwnd, msg, wp, lp);
}

enum {
    K_SIDEBAR, K_SIDEBAR_PRIMARY, K_SIDEBAR_DANGER,
    K_HERO_PRIMARY, K_HERO_GHOST, K_NAV, K_PAGE_PRIMARY, K_PAGE_DANGER
};

int buttonKind(int id) {
    switch (id) {
        case ID_ANALYZE:
        case ID_REPORT: return K_SIDEBAR_PRIMARY;
        case ID_EXIT: return K_SIDEBAR_DANGER;
        case ID_HOME_START: return K_HERO_PRIMARY;
        case ID_HOME_HISTORY: return K_HERO_GHOST;
        case ID_NAV_HOME:
        case ID_NAV_HISTORY:
        case ID_NAV_WORK: return K_NAV;
        case ID_HIST_OPEN: return K_PAGE_PRIMARY;
        case ID_HIST_DELETE: return K_PAGE_DANGER;
    }
    return K_SIDEBAR;
}

void drawButton(const DRAWITEMSTRUCT* dis) {
    HDC dc = dis->hDC;
    RECT rc = dis->rcItem;
    int kind = buttonKind(static_cast<int>(dis->CtlID));
    bool hover = (g_hoverWnd == dis->hwndItem);
    bool pressed = (dis->itemState & ODS_SELECTED) != 0;

    COLORREF surface = COL_NAVY;
    if (kind == K_HERO_PRIMARY || kind == K_HERO_GHOST) surface = COL_HERO;
    else if (kind == K_NAV || kind == K_PAGE_PRIMARY || kind == K_PAGE_DANGER) surface = COL_WHITE;
    fillRect(dc, rc, surface);

    COLORREF fill = surface;
    COLORREF border = surface;
    COLORREF text = RGB(203, 213, 232);
    bool centered = false;
    int radius = 12;
    HFONT font = g_font;

    switch (kind) {
        case K_SIDEBAR_PRIMARY:
            fill = pressed ? RGB(37, 99, 235) : (hover ? RGB(96, 165, 250) : COL_ACCENT);
            border = fill;
            text = COL_WHITE;
            break;
        case K_SIDEBAR_DANGER:
            if (hover || pressed) {
                fill = pressed ? RGB(153, 27, 27) : RGB(185, 28, 28);
                border = fill;
                text = COL_WHITE;
            }
            break;
        case K_HERO_PRIMARY:
            fill = pressed ? RGB(37, 99, 235) : (hover ? RGB(96, 165, 250) : COL_ACCENT);
            border = fill;
            text = COL_WHITE;
            centered = true;
            radius = 20;
            font = g_cardFont;
            break;
        case K_HERO_GHOST:
            fill = (hover || pressed) ? RGB(30, 41, 72) : COL_HERO;
            border = RGB(71, 85, 125);
            text = COL_WHITE;
            centered = true;
            radius = 20;
            font = g_cardFont;
            break;
        case K_NAV:
            fill = (hover || pressed) ? RGB(241, 245, 249) : COL_WHITE;
            border = COL_BORDER;
            text = COL_TEXT;
            centered = true;
            radius = 12;
            break;
        case K_PAGE_PRIMARY:
            fill = pressed ? RGB(37, 99, 235) : (hover ? RGB(96, 165, 250) : COL_ACCENT);
            border = fill;
            text = COL_WHITE;
            centered = true;
            break;
        case K_PAGE_DANGER:
            fill = (hover || pressed) ? RGB(254, 226, 226) : COL_WHITE;
            border = RGB(252, 165, 165);
            text = RGB(185, 28, 28);
            centered = true;
            break;
        default:
            if (hover || pressed) {
                fill = pressed ? RGB(58, 76, 118) : RGB(40, 54, 90);
                border = fill;
                text = COL_WHITE;
            }
            break;
    }
    if ((dis->itemState & ODS_FOCUS) && !(dis->itemState & ODS_NOFOCUSRECT)) border = RGB(147, 197, 253);
    if (fill != surface || border != surface) fillRound(dc, rc, fill, border, radius);

    char label[96];
    GetWindowTextA(dis->hwndItem, label, 96);
    SetBkMode(dc, TRANSPARENT);
    SetTextColor(dc, text);
    SelectObject(dc, font);
    RECT tr = rc;
    if (centered) {
        DrawTextA(dc, label, -1, &tr, DT_CENTER | DT_VCENTER | DT_SINGLELINE);
    } else {
        tr.left += 14;
        DrawTextA(dc, label, -1, &tr, DT_LEFT | DT_VCENTER | DT_SINGLELINE);
    }
}

void registerPageWin(int page, HWND w) {
    if (g_pageWinCount[page] < MAX_PAGE_WINS) g_pageWins[page][g_pageWinCount[page]++] = w;
}

HWND makeButton(HWND parent, HINSTANCE inst, int id, const char* text, int page) {
    HWND b = CreateWindowExA(0, "BUTTON", text, WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_OWNERDRAW,
                             0, 0, 10, 10, parent,
                             reinterpret_cast<HMENU>(static_cast<INT_PTR>(id)), inst, NULL);
    WNDPROC old = reinterpret_cast<WNDPROC>(SetWindowLongPtrA(b, GWLP_WNDPROC, reinterpret_cast<LONG_PTR>(ButtonProc)));
    if (g_oldButtonProc == NULL) g_oldButtonProc = old;
    registerPageWin(page, b);
    return b;
}

HWND addSidebarButton(HWND parent, HINSTANCE inst, int id, const char* text, int y) {
    HWND b = makeButton(parent, inst, id, text, PAGE_WORK);
    MoveWindow(b, 14, y, SIDEBAR_W - 28, 28, TRUE);
    return b;
}

void updateHomeButtons() {
    std::string label = "View Case History";
    if (g_store != nullptr && g_store->getCount() > 0) label += " (" + intToString(g_store->getCount()) + ")";
    SetWindowTextA(g_btnHomeHistory, label.c_str());
    InvalidateRect(g_btnHomeHistory, NULL, FALSE);
}

void sendToBottom(HWND w) {
    if (w != NULL) SetWindowPos(w, HWND_BOTTOM, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE);
}

void showPage(int page) {
    g_page = page;
    g_hoverWnd = NULL;
    for (int p = 0; p < 3; ++p) {
        for (int i = 0; i < g_pageWinCount[p]; ++i) {
            ShowWindow(g_pageWins[p][i], p == page ? SW_SHOW : SW_HIDE);
        }
    }
    sendToBottom(g_header);
    sendToBottom(g_rank);
    sendToBottom(g_card);
    sendToBottom(g_home);
    sendToBottom(g_hist);
    if (page == PAGE_HISTORY && g_histSelected < 0 && g_store->getCount() > 0) g_histSelected = 0;
    updateHomeButtons();
    InvalidateRect(g_main, NULL, TRUE);
    if (page == PAGE_HISTORY) {
        histFix();
        SetFocus(g_hist);
    }
}

void onHistoryOpen() {
    const HistoryEntry* e = g_store->getAt(g_histSelected);
    if (e == nullptr) {
        MessageBoxA(g_main, "Select a case in the list first.", "CASE-X", MB_OK | MB_ICONINFORMATION);
        return;
    }
    int id = e->id;
    resetSession();
    if (!deserializeCase(e->body, *g_case, *g_evidence, *g_queue)) {
        resetSession();
        refreshAll();
        MessageBoxA(g_main, "This saved case could not be read.", "CASE-X", MB_OK | MB_ICONWARNING);
        return;
    }
    g_caseReady = true;
    g_caseId = id;
    AnalysisEngine::analyzeCase(*g_case, *g_evidence, g_results, g_resultCount);
    AnalysisEngine::rankSuspects(g_results, g_resultCount);
    g_analyzed = true;
    g_queue->enqueue(InvestigationEvent(g_queue->getCount() + 1, "Opened from History"));
    refreshAll();
    showPage(PAGE_WORK);
    showText("CASE OPENED FROM HISTORY\n\n" + capture([&]() { g_case->displayCaseInfo(); }) +
             "\nThe ranking was recalculated. Click 'Final Report' for the full explanation.\n");
}

void onHistoryDelete() {
    const HistoryEntry* e = g_store->getAt(g_histSelected);
    if (e == nullptr) {
        MessageBoxA(g_main, "Select a case in the list first.", "CASE-X", MB_OK | MB_ICONINFORMATION);
        return;
    }
    int id = e->id;
    std::string question = "Delete this saved case from History?\n\n" + e->title;
    if (MessageBoxA(g_main, question.c_str(), "Confirm delete", MB_YESNO | MB_ICONQUESTION) != IDYES) return;
    g_store->removeById(id);
    if (!g_store->save()) {
        MessageBoxA(g_main, "Could not update the history file.", "CASE-X", MB_OK | MB_ICONWARNING);
    }
    if (id == g_caseId) g_caseId = -1;
    histFix();
    updateHomeButtons();
    InvalidateRect(g_hist, NULL, FALSE);
}

void createControls(HWND hwnd, HINSTANCE inst) {
    g_header = CreateWindowExA(0, HEADER_CLASS, "", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 0, 0, 10, 10, hwnd, NULL, inst, NULL);
    g_rank = CreateWindowExA(0, RANK_CLASS, "", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 0, 0, 10, 10, hwnd, NULL, inst, NULL);
    g_card = CreateWindowExA(0, CARD_CLASS, "", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN, 0, 0, 10, 10, hwnd, NULL, inst, NULL);
    registerPageWin(PAGE_WORK, g_header);
    registerPageWin(PAGE_WORK, g_rank);
    registerPageWin(PAGE_WORK, g_card);

    g_output = CreateWindowExA(0, "EDIT", "",
                               WS_CHILD | WS_VISIBLE | WS_VSCROLL | WS_HSCROLL | ES_MULTILINE |
                               ES_READONLY | ES_AUTOVSCROLL | ES_AUTOHSCROLL,
                               18, 48, 10, 10, g_card, NULL, inst, NULL);
    SendMessageA(g_output, WM_SETFONT, reinterpret_cast<WPARAM>(g_monoFont), TRUE);
    SendMessageA(g_output, EM_SETLIMITTEXT, 0x7FFFFFF, 0);

    int y = 64;
    g_sectionCount = 0;
    g_sections[g_sectionCount].y = y; g_sections[g_sectionCount++].text = "CASE"; y += 22;
    addSidebarButton(hwnd, inst, ID_LOAD, "Load Sample Case", y); y += 31;
    addSidebarButton(hwnd, inst, ID_NEW, "New Case...", y); y += 31;
    addSidebarButton(hwnd, inst, ID_VIEWCASE, "View Case", y); y += 39;

    g_sections[g_sectionCount].y = y; g_sections[g_sectionCount++].text = "SUSPECTS"; y += 22;
    addSidebarButton(hwnd, inst, ID_VIEWSUS, "View Suspects", y); y += 31;
    addSidebarButton(hwnd, inst, ID_SEARCHSUS, "Search Suspect...", y); y += 39;

    g_sections[g_sectionCount].y = y; g_sections[g_sectionCount++].text = "EVIDENCE"; y += 22;
    addSidebarButton(hwnd, inst, ID_ADDEV, "Add Evidence...", y); y += 31;
    addSidebarButton(hwnd, inst, ID_VIEWEV, "View Evidence", y); y += 31;
    addSidebarButton(hwnd, inst, ID_SEARCHEV, "Search Evidence by ID...", y); y += 31;
    addSidebarButton(hwnd, inst, ID_EVSUS, "Evidence of a Suspect...", y); y += 31;
    addSidebarButton(hwnd, inst, ID_DELEV, "Delete Evidence...", y); y += 39;

    g_sections[g_sectionCount].y = y; g_sections[g_sectionCount++].text = "ANALYSIS"; y += 22;
    addSidebarButton(hwnd, inst, ID_ANALYZE, "Analyze Case", y); y += 31;
    addSidebarButton(hwnd, inst, ID_REPORT, "Final Report", y); y += 31;
    addSidebarButton(hwnd, inst, ID_TIMELINE, "Investigation Timeline", y); y += 31;
    addSidebarButton(hwnd, inst, ID_EXIT, "Exit", y); y += 31;
    g_sidebarBottom = y;

    g_btnWorkHome = makeButton(hwnd, inst, ID_NAV_HOME, "Home", PAGE_WORK);
    g_btnWorkHistory = makeButton(hwnd, inst, ID_NAV_HISTORY, "History", PAGE_WORK);

    g_home = CreateWindowExA(0, HOME_CLASS, "", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 0, 0, 10, 10, hwnd, NULL, inst, NULL);
    registerPageWin(PAGE_HOME, g_home);
    g_btnHomeStart = makeButton(hwnd, inst, ID_HOME_START, "Start Investigation", PAGE_HOME);
    g_btnHomeHistory = makeButton(hwnd, inst, ID_HOME_HISTORY, "View Case History", PAGE_HOME);

    g_hist = CreateWindowExA(0, HIST_CLASS, "", WS_CHILD | WS_VISIBLE | WS_CLIPSIBLINGS, 0, 0, 10, 10, hwnd, NULL, inst, NULL);
    registerPageWin(PAGE_HISTORY, g_hist);
    g_btnHistHome = makeButton(hwnd, inst, ID_NAV_HOME, "Home", PAGE_HISTORY);
    g_btnHistWork = makeButton(hwnd, inst, ID_NAV_WORK, "Workspace", PAGE_HISTORY);
    g_btnHistDelete = makeButton(hwnd, inst, ID_HIST_DELETE, "Delete", PAGE_HISTORY);
    g_btnHistOpen = makeButton(hwnd, inst, ID_HIST_OPEN, "Open Case", PAGE_HISTORY);

    showText("CASE-X - Rule-Based Crime Case Analysis and Suspect Ranking System\n\n"
             "Quick start:\n"
             "  1. Click 'Load Sample Case'  (or 'New Case...' to enter your own)\n"
             "  2. Click 'Analyze Case'  (the case is saved to History automatically)\n"
             "  3. Click 'Final Report'\n\n"
             "The score is a rule-based Suspicion Score used only for ranking.\n"
             "It does not establish actual guilt.\n");
    showPage(PAGE_HOME);
}

void layoutControls(int w, int h) {
    int cw = w - SIDEBAR_W;
    if (cw < 300) cw = 300;
    const int margin = 20;
    const int rankH = 188;
    MoveWindow(g_header, SIDEBAR_W, 0, cw, HEADER_H, TRUE);
    MoveWindow(g_rank, SIDEBAR_W + margin, HEADER_H + 20, cw - 2 * margin, rankH, TRUE);
    int top = HEADER_H + 20 + rankH + 16;
    int ch = h - top - margin;
    if (ch < 120) ch = 120;
    MoveWindow(g_card, SIDEBAR_W + margin, top, cw - 2 * margin, ch, TRUE);
    int navRight = SIDEBAR_W + cw - 24;
    MoveWindow(g_btnWorkHistory, navRight - 96, 23, 96, 30, TRUE);
    MoveWindow(g_btnWorkHome, navRight - 96 - 8 - 84, 23, 84, 30, TRUE);

    MoveWindow(g_home, 0, 0, w, h, TRUE);
    HomeGeo hg = homeGeometry(w, h);
    MoveWindow(g_btnHomeStart, hg.pad, hg.btnY, 240, 52, TRUE);
    MoveWindow(g_btnHomeHistory, hg.pad + 256, hg.btnY, 270, 52, TRUE);

    MoveWindow(g_hist, 0, 0, w, h, TRUE);
    int x = w - 24 - 120;
    MoveWindow(g_btnHistOpen, x, 23, 120, 30, TRUE);
    x -= 8 + 84;
    MoveWindow(g_btnHistDelete, x, 23, 84, 30, TRUE);
    x -= 8 + 110;
    MoveWindow(g_btnHistWork, x, 23, 110, 30, TRUE);
    x -= 8 + 84;
    MoveWindow(g_btnHistHome, x, 23, 84, 30, TRUE);

    InvalidateRect(g_header, NULL, FALSE);
    InvalidateRect(g_rank, NULL, FALSE);
    InvalidateRect(g_card, NULL, FALSE);
    InvalidateRect(g_home, NULL, FALSE);
    InvalidateRect(g_hist, NULL, FALSE);
}

void paintBackground(HDC dc, int width, int height) {
    RECT all = {0, 0, width, height};
    fillRect(dc, all, COL_BG);
    if (g_page != PAGE_WORK) return;
    RECT side = {0, 0, SIDEBAR_W, height};
    fillRect(dc, side, COL_NAVY);
    SetBkMode(dc, TRANSPARENT);

    SelectObject(dc, g_titleFont);
    SetTextColor(dc, COL_WHITE);
    drawText(dc, 20, 12, "CASE-X");
    SelectObject(dc, g_smallFont);
    SetTextColor(dc, RGB(148, 163, 184));
    drawText(dc, 21, 40, "Crime Case Analysis");

    SelectObject(dc, g_boldFont);
    SetTextColor(dc, RGB(112, 128, 160));
    for (int i = 0; i < g_sectionCount; ++i) {
        drawText(dc, 20, g_sections[i].y + 2, g_sections[i].text);
    }
    if (height - 30 > g_sidebarBottom + 6) {
        SelectObject(dc, g_smallFont);
        SetTextColor(dc, RGB(112, 128, 160));
        drawText(dc, 20, height - 28, "Team Amon  |  PBL Project");
    }
}

LRESULT CALLBACK MainProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    switch (msg) {
        case WM_CREATE:
            g_main = hwnd;
            createControls(hwnd, reinterpret_cast<CREATESTRUCTA*>(lp)->hInstance);
            return 0;
        case WM_ERASEBKGND: {
            RECT rc;
            GetClientRect(hwnd, &rc);
            paintBackground(reinterpret_cast<HDC>(wp), rc.right, rc.bottom);
            return 1;
        }
        case WM_DRAWITEM:
            drawButton(reinterpret_cast<DRAWITEMSTRUCT*>(lp));
            return TRUE;
        case WM_SIZE:
            if (g_btnHistHome != NULL) {
                layoutControls(LOWORD(lp), HIWORD(lp));
                InvalidateRect(hwnd, NULL, TRUE);
            }
            return 0;
        case WM_GETMINMAXINFO: {
            MINMAXINFO* mm = reinterpret_cast<MINMAXINFO*>(lp);
            mm->ptMinTrackSize.x = 900;
            mm->ptMinTrackSize.y = 700;
            return 0;
        }
        case WM_COMMAND:
            switch (LOWORD(wp)) {
                case ID_LOAD: onLoadSample(); break;
                case ID_NEW: onNewCase(); break;
                case ID_VIEWCASE: onViewCase(); break;
                case ID_VIEWSUS: onViewSuspects(); break;
                case ID_SEARCHSUS: onSearchSuspect(); break;
                case ID_ADDEV: onAddEvidence(); break;
                case ID_VIEWEV: onViewEvidence(); break;
                case ID_SEARCHEV: onSearchEvidence(); break;
                case ID_EVSUS: onEvidenceOfSuspect(); break;
                case ID_DELEV: onDeleteEvidence(); break;
                case ID_ANALYZE: onAnalyze(); break;
                case ID_REPORT: onReport(); break;
                case ID_TIMELINE: onTimeline(); break;
                case ID_EXIT: DestroyWindow(hwnd); break;
                case ID_HOME_START: showPage(PAGE_WORK); break;
                case ID_HOME_HISTORY:
                case ID_NAV_HISTORY: showPage(PAGE_HISTORY); break;
                case ID_NAV_HOME: showPage(PAGE_HOME); break;
                case ID_NAV_WORK: showPage(PAGE_WORK); break;
                case ID_HIST_OPEN: onHistoryOpen(); break;
                case ID_HIST_DELETE: onHistoryDelete(); break;
            }
            return 0;
        case WM_DESTROY:
            PostQuitMessage(0);
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

HFONT makeFont(int height, int weight, const char* face) {
    return CreateFontA(height, 0, 0, 0, weight, FALSE, FALSE, FALSE, DEFAULT_CHARSET, OUT_DEFAULT_PRECIS,
                       CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY, DEFAULT_PITCH | FF_DONTCARE, face);
}

void registerClass(const char* name, WNDPROC proc, HINSTANCE inst, UINT style) {
    WNDCLASSA wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.style = style;
    wc.lpfnWndProc = proc;
    wc.hInstance = inst;
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    wc.lpszClassName = name;
    RegisterClassA(&wc);
}

}

int WINAPI WinMain(HINSTANCE inst, HINSTANCE, LPSTR, int show) {
    g_font = makeFont(-16, FW_NORMAL, "Segoe UI");
    g_boldFont = makeFont(-12, FW_BOLD, "Segoe UI");
    g_titleFont = makeFont(-24, FW_BOLD, "Segoe UI");
    g_smallFont = makeFont(-13, FW_NORMAL, "Segoe UI");
    g_cardFont = makeFont(-17, FW_BOLD, "Segoe UI");
    g_scoreFont = makeFont(-15, FW_BOLD, "Segoe UI");
    g_monoFont = makeFont(-15, FW_NORMAL, "Consolas");
    g_heroFont = makeFont(-46, FW_BOLD, "Segoe UI");
    g_leadFont = makeFont(-18, FW_NORMAL, "Segoe UI");

    g_store = new HistoryStore(exeDirPath("casex_history.dat"));
    g_store->load();
    resetSession();

    registerClass(MAIN_CLASS, MainProc, inst, CS_HREDRAW | CS_VREDRAW);
    registerClass(RANK_CLASS, RankProc, inst, CS_HREDRAW | CS_VREDRAW);
    registerClass(HEADER_CLASS, HeaderProc, inst, CS_HREDRAW | CS_VREDRAW);
    registerClass(CARD_CLASS, CardProc, inst, CS_HREDRAW | CS_VREDRAW);
    registerClass(HOME_CLASS, HomeProc, inst, CS_HREDRAW | CS_VREDRAW);
    registerClass(HIST_CLASS, HistoryProc, inst, CS_DBLCLKS | CS_HREDRAW | CS_VREDRAW);

    HWND hwnd = CreateWindowExA(0, MAIN_CLASS, "CASE-X - Crime Case Analysis System",
                                WS_OVERLAPPEDWINDOW | WS_CLIPCHILDREN, CW_USEDEFAULT, CW_USEDEFAULT, 1140, 740,
                                NULL, NULL, inst, NULL);
    if (hwnd == NULL) return 1;
    ShowWindow(hwnd, show);
    UpdateWindow(hwnd);

    MSG msg;
    while (GetMessageA(&msg, NULL, 0, 0) > 0) {
        TranslateMessage(&msg);
        DispatchMessageA(&msg);
    }

    delete g_case;
    delete g_evidence;
    delete g_queue;
    delete g_store;
    DeleteObject(g_font);
    DeleteObject(g_boldFont);
    DeleteObject(g_titleFont);
    DeleteObject(g_smallFont);
    DeleteObject(g_cardFont);
    DeleteObject(g_scoreFont);
    DeleteObject(g_monoFont);
    DeleteObject(g_heroFont);
    DeleteObject(g_leadFont);
    return static_cast<int>(msg.wParam);
}
