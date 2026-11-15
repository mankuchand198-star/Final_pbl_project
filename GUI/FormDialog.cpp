#include "FormDialog.h"
#include "../Utils.h"

namespace {

const char* FORM_CLASS = "CaseXFormWindow";
bool classRegistered = false;
HBRUSH formBrush = NULL;
const COLORREF FORM_BG = RGB(248, 250, 252);

struct FormState {
    HWND controls[FORM_MAX_FIELDS];
    const FormField* fields;
    int count;
    std::string* values;
    FormValidator validator;
    bool done;
    bool accepted;
};

std::string readControlText(HWND h) {
    int len = GetWindowTextLengthA(h);
    std::string s(static_cast<size_t>(len), '\0');
    if (len > 0) GetWindowTextA(h, &s[0], len + 1);
    return s;
}

bool collectValues(FormState* st, HWND hwnd) {
    for (int i = 0; i < st->count; ++i) {
        if (st->fields[i].type == FORM_COMBO) {
            int idx = static_cast<int>(SendMessageA(st->controls[i], CB_GETCURSEL, 0, 0));
            if (idx < 0) idx = 0;
            st->values[i] = intToString(idx);
        } else {
            st->values[i] = trimSpaces(readControlText(st->controls[i]));
        }
    }
    if (st->validator != nullptr) {
        std::string error;
        if (!st->validator(st->values, st->count, error)) {
            MessageBoxA(hwnd, error.c_str(), "Please check your input", MB_OK | MB_ICONWARNING);
            return false;
        }
    }
    return true;
}

LRESULT CALLBACK FormProc(HWND hwnd, UINT msg, WPARAM wp, LPARAM lp) {
    FormState* st = reinterpret_cast<FormState*>(GetWindowLongPtrA(hwnd, GWLP_USERDATA));
    switch (msg) {
        case WM_CREATE: {
            CREATESTRUCTA* cs = reinterpret_cast<CREATESTRUCTA*>(lp);
            SetWindowLongPtrA(hwnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(cs->lpCreateParams));
            return 0;
        }
        case DM_GETDEFID:
            return MAKELONG(IDOK, DC_HASDEFID);
        case WM_CTLCOLORSTATIC: {
            HDC dc = reinterpret_cast<HDC>(wp);
            SetTextColor(dc, RGB(30, 41, 59));
            SetBkColor(dc, FORM_BG);
            return reinterpret_cast<LRESULT>(formBrush);
        }
        case WM_COMMAND:
            if (st == nullptr) return 0;
            if (LOWORD(wp) == IDOK) {
                if (collectValues(st, hwnd)) {
                    st->accepted = true;
                    st->done = true;
                }
            } else if (LOWORD(wp) == IDCANCEL) {
                st->accepted = false;
                st->done = true;
            }
            return 0;
        case WM_CLOSE:
            if (st != nullptr) {
                st->accepted = false;
                st->done = true;
            }
            return 0;
    }
    return DefWindowProcA(hwnd, msg, wp, lp);
}

void registerFormClass() {
    if (classRegistered) return;
    WNDCLASSA wc;
    ZeroMemory(&wc, sizeof(wc));
    wc.lpfnWndProc = FormProc;
    wc.hInstance = GetModuleHandleA(NULL);
    wc.hCursor = LoadCursorA(NULL, IDC_ARROW);
    formBrush = CreateSolidBrush(FORM_BG);
    wc.hbrBackground = formBrush;
    wc.lpszClassName = FORM_CLASS;
    RegisterClassA(&wc);
    classRegistered = true;
}

void addComboOptions(HWND combo, const std::string& options) {
    std::string current;
    for (size_t i = 0; i <= options.size(); ++i) {
        if (i == options.size() || options[i] == '|') {
            SendMessageA(combo, CB_ADDSTRING, 0, reinterpret_cast<LPARAM>(current.c_str()));
            current = "";
        } else {
            current += options[i];
        }
    }
}

}

bool showForm(HWND owner, const char* title, const FormField* fields, int count,
              std::string* values, FormValidator validator) {
    if (count > FORM_MAX_FIELDS) count = FORM_MAX_FIELDS;
    registerFormClass();

    FormState st;
    st.fields = fields;
    st.count = count;
    st.values = values;
    st.validator = validator;
    st.done = false;
    st.accepted = false;
    for (int i = 0; i < FORM_MAX_FIELDS; ++i) st.controls[i] = NULL;

    const int labelW = 215;
    const int fieldW = 320;
    const int rowH = 34;
    const int pad = 16;
    int clientW = pad + labelW + 8 + fieldW + pad;
    int clientH = pad + count * rowH + 56;

    DWORD style = WS_POPUP | WS_CAPTION | WS_SYSMENU;
    RECT rc = {0, 0, clientW, clientH};
    AdjustWindowRect(&rc, style, FALSE);
    int winW = rc.right - rc.left;
    int winH = rc.bottom - rc.top;

    RECT orc;
    GetWindowRect(owner, &orc);
    int x = orc.left + ((orc.right - orc.left) - winW) / 2;
    int y = orc.top + ((orc.bottom - orc.top) - winH) / 2;
    if (y < 0) y = 0;

    HWND hwnd = CreateWindowExA(WS_EX_DLGMODALFRAME, FORM_CLASS, title, style, x, y, winW, winH,
                                owner, NULL, GetModuleHandleA(NULL), &st);
    if (hwnd == NULL) return false;

    HFONT font = CreateFontA(-16, 0, 0, 0, FW_NORMAL, FALSE, FALSE, FALSE, DEFAULT_CHARSET,
                             OUT_DEFAULT_PRECIS, CLIP_DEFAULT_PRECIS, CLEARTYPE_QUALITY,
                             DEFAULT_PITCH | FF_DONTCARE, "Segoe UI");

    for (int i = 0; i < count; ++i) {
        int top = pad + i * rowH;
        HWND label = CreateWindowExA(0, "STATIC", fields[i].label, WS_CHILD | WS_VISIBLE | SS_LEFT,
                                     pad, top + 4, labelW, 22, hwnd, NULL, GetModuleHandleA(NULL), NULL);
        SendMessageA(label, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        HWND ctl;
        if (fields[i].type == FORM_COMBO) {
            ctl = CreateWindowExA(0, "COMBOBOX", "", WS_CHILD | WS_VISIBLE | WS_TABSTOP | WS_VSCROLL | CBS_DROPDOWNLIST,
                                  pad + labelW + 8, top, fieldW, 220, hwnd, NULL, GetModuleHandleA(NULL), NULL);
            SendMessageA(ctl, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
            addComboOptions(ctl, fields[i].options);
            int sel = 0;
            int parsed = 0;
            if (values[i].size() > 0 && parseInt(values[i], parsed)) sel = parsed;
            SendMessageA(ctl, CB_SETCURSEL, static_cast<WPARAM>(sel), 0);
        } else {
            ctl = CreateWindowExA(WS_EX_CLIENTEDGE, "EDIT", values[i].c_str(),
                                  WS_CHILD | WS_VISIBLE | WS_TABSTOP | ES_AUTOHSCROLL,
                                  pad + labelW + 8, top, fieldW, 26, hwnd, NULL, GetModuleHandleA(NULL), NULL);
            SendMessageA(ctl, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
        }
        st.controls[i] = ctl;
    }

    int btnTop = pad + count * rowH + 8;
    HWND ok = CreateWindowExA(0, "BUTTON", "OK", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_DEFPUSHBUTTON,
                              clientW - pad - 2 * 100 - 10, btnTop, 100, 32, hwnd,
                              reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDOK)), GetModuleHandleA(NULL), NULL);
    HWND cancel = CreateWindowExA(0, "BUTTON", "Cancel", WS_CHILD | WS_VISIBLE | WS_TABSTOP | BS_PUSHBUTTON,
                                  clientW - pad - 100, btnTop, 100, 32, hwnd,
                                  reinterpret_cast<HMENU>(static_cast<INT_PTR>(IDCANCEL)), GetModuleHandleA(NULL), NULL);
    SendMessageA(ok, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);
    SendMessageA(cancel, WM_SETFONT, reinterpret_cast<WPARAM>(font), TRUE);

    EnableWindow(owner, FALSE);
    ShowWindow(hwnd, SW_SHOW);
    if (count > 0) SetFocus(st.controls[0]);

    MSG msg;
    while (!st.done) {
        BOOL got = GetMessageA(&msg, NULL, 0, 0);
        if (got == 0) {
            PostQuitMessage(static_cast<int>(msg.wParam));
            st.accepted = false;
            break;
        }
        if (got == -1) break;
        if (!IsDialogMessageA(hwnd, &msg)) {
            TranslateMessage(&msg);
            DispatchMessageA(&msg);
        }
    }

    EnableWindow(owner, TRUE);
    DestroyWindow(hwnd);
    SetForegroundWindow(owner);
    DeleteObject(font);
    return st.accepted;
}
