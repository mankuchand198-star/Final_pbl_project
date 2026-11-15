#ifndef FORMDIALOG_H
#define FORMDIALOG_H

#include <windows.h>
#include <string>

const int FORM_TEXT = 0;
const int FORM_COMBO = 1;
const int FORM_MAX_FIELDS = 12;

struct FormField {
    const char* label;
    int type;
    std::string options;
};

typedef bool (*FormValidator)(const std::string* values, int count, std::string& error);

bool showForm(HWND owner, const char* title, const FormField* fields, int count,
              std::string* values, FormValidator validator);

#endif
