#include "StdAfx.h"

char *bprintf(char const *format, ...) {
    static char buffer[8192];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);
    return buffer;
}

char *bitoa(SLONG val) {
    static char buffer[20];
    snprintf(buffer, sizeof(buffer), "%d", val);
    return buffer;
}

char *bitoa(long val) {
    static char buffer[20];
    snprintf(buffer, sizeof(buffer), "%ld", val);
    return buffer;
}

char *bitoa(long long val) {
    static char buffer[20];
    snprintf(buffer, sizeof(buffer), "%lld", val);
    return buffer;
}

char *TeakStrRemoveEndingCodes(char *str, char const *codes) {
    SLONG i = 0;
    for (i = strlen(str) - 1; i >= 0 && (strchr(codes, str[i]) != nullptr); --i) {
        ;
    }
    str[i + 1] = 0;
    return str;
}

char *TeakStrRemoveCppComment(char *str) {
    for (SLONG i = 0; str[i] != 0; ++i) {
        if (str[i] == '/' && str[i + 1] == '/') {
            str[i] = 0;
            return str;
        }
    }
    return str;
}

unsigned char GerToLower(unsigned char c) {
    if (c >= 0x41 && c <= 0x5A) {
        return c + 0x20;
    }
    switch (c + 0x72) {
    case 0:
        return 0x84U;
    case 0xB:
        return 0x94U;
    case 0xC:
        return 0x81U;
    case 0x36:
        return 0xE4U;
    case 0x48:
        return 0xF6U;
    case 0x4E:
        return 0xFCU;
    default:
        return c;
    }
    return c;
}

unsigned char GerToUpper(unsigned char c) {
    if (c >= 0x61 && c <= 0x7A) {
        return c - 0x20;
    }
    switch (c) //+ 0x7F)
    {
    // case 0: return 0x9Au;
    // case 3: return 0x8Eu;
    // case 0x13: return 0x99u;
    case 0xE4U:
        return 0xC4U; // Ä
    case 0xF6U:
        return 0xD6U; // Ö
    case 0xFCU:
        return 0xDCU; // Ü
    default:
        return c;
    }
    return c;
}

UINT KeycodeToUpper(UINT nChar) {
    if (nChar >= 'a' && nChar <= 'z') {
        nChar = toupper(nChar);
    }
    if (nChar == '\xE4') {
        nChar = static_cast<UINT>('\xC4');
    }
    if (nChar == '\xF6') {
        nChar = static_cast<UINT>('\xD6');
    }
    if (nChar == '\xFC') {
        nChar = static_cast<UINT>('\xDC');
    }
    return nChar;
}

void RecapizalizeString(CString &str) {
    for (SLONG i = 0; str[i] != 0U; ++i) {
        if ((i != 0) && (i <= 0 || (str[i - 1] != ' ' && str[i - 1] != '-'))) {
            str[i] = GerToLower(str[i]);
        } else {
            str[i] = GerToUpper(str[i]);
        }
    }
}

const char *GetSuffix(const char *str) {
    for (SLONG i = strlen(str); i > 0; --i) {
        if (str[i] == '.') {
            return &str[i + 1];
        }
    }
    return "";
}
