#include "StdAfx.h"

#include <cstdio>
#include <stdexcept>
#include <mutex>

#define AT_Error(...) Hdu.HercPrintfMsg(SDL_LOG_PRIORITY_ERROR, "Dbg", __VA_ARGS__)
#define AT_Warn(...) Hdu.HercPrintfMsg(SDL_LOG_PRIORITY_WARN, "Dbg", __VA_ARGS__)
#define AT_Info(...) Hdu.HercPrintfMsg(SDL_LOG_PRIORITY_INFO, "Dbg", __VA_ARGS__)
#define AT_Log(...) AT_Log_I("Dbg", __VA_ARGS__)

const char *ExcAssert = "Assert (called from %s:%li) failed!";
const char *ExcGuardian = "Con/Des guardian %lx failed!";
const char *ExcStrangeMem = "Strange new: %li bytes!";
const char *ExcOutOfMem = "Couldn't allocate %li bytes!";
const char *ExcNotImplemented = "Function not implemented!";
const char *ExcImpossible = "Impossible Event %s occured";

HDU Hdu;
TeakLibException *lastError;

std::mutex logMutex;

HDU::HDU() : Log(nullptr) {
    char *base = SDL_GetBasePath();
    const char *file = "debug.txt";
    BUFFER_V<char> path(strlen(base) + strlen(file) + 1);
    strcpy(path.data(), base);
    strcat(path.data(), file);
    Log = fopen(path.data(), "w");
    // Log = stdout;
    SDL_free(base);

    SDL_LogOutputFunction defaultOut;
    SDL_LogGetOutputFunction(&defaultOut, nullptr);

#ifdef _DEBUG
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
#endif

    auto func = [](void *userdata, int category, SDL_LogPriority priority, const char *message) {
        logMutex.lock();
        char *finalMessage = const_cast<char *>(message);

        bool modified = false;
        if (strstr(message, "||") == nullptr) {
            const unsigned long long size = strlen(message) + strlen("Misc || ") + 1;
            finalMessage = new char[size]{};
            snprintf(finalMessage, size, "Misc || %s", message);
            modified = true;
        }

        const SDL_LogOutputFunction func = reinterpret_cast<SDL_LogOutputFunction>(userdata);
        func(userdata, category, priority, finalMessage);

        if (Hdu.Log) {

            fprintf(Hdu.Log, "%s\n", finalMessage);
            fflush(Hdu.Log);
        }

        if (modified) {
            delete[] finalMessage;
        }

        logMutex.unlock();
    };

    SDL_LogSetOutputFunction(func, reinterpret_cast<void *>(defaultOut));
}

HDU::~HDU() { Close(); }

void HDU::Close() {
    if (Log != nullptr) {
        fclose(Log);
    }
    Log = nullptr;
}

void HDU::HercPrintfMsg(SDL_LogPriority lvl, const char *origin, const char *format, ...) {
    if (Log == nullptr) {
        return;
    }

    if (lvl == SDL_LOG_PRIORITY_ERROR) {
        numErrors = std::min(9999, numErrors + 1);
    } else if (lvl == SDL_LOG_PRIORITY_WARN) {
        numWarnings = std::min(9999, numWarnings + 1);
    }

    std::string prefix{};
    std::string suffix{};
    if (lvl == SDL_LOG_PRIORITY_ERROR) {
        int size = snprintf(nullptr, 0, "\e[1;31m#%d %s || ", numErrors, origin) + 1;
        std::unique_ptr<char[]> buf(new char[size]);
        snprintf(buf.get(), size, "\e[1;31m#%d %s || ", numErrors, origin);
        prefix = {buf.get(), buf.get() + size - 1};

        suffix = "\e[m";
    } else if (lvl == SDL_LOG_PRIORITY_WARN) {
        int size = snprintf(nullptr, 0, "\e[1;33m#%d %s || ", numWarnings, origin) + 1;
        std::unique_ptr<char[]> buf(new char[size]);
        snprintf(buf.get(), size, "\e[1;33m#%d %s || ", numWarnings, origin);
        prefix = {buf.get(), buf.get() + size - 1};

        suffix = "\e[m";
    } else {
        int size = snprintf(nullptr, 0, "\e[1;32m%s || ", origin) + 1;
        std::unique_ptr<char[]> buf(new char[size]);
        snprintf(buf.get(), size, "\e[1;32m%s || ", origin);
        prefix = {buf.get(), buf.get() + size - 1};

        suffix = "\e[m";
    }

    va_list args;
    va_start(args, format);
    SDL_LogMessageV(SDL_LOG_CATEGORY_APPLICATION, lvl, (prefix + format + suffix).c_str(), args);
    va_end(args);
}

void here(char *file, SLONG line) { AT_Info("Here in %s, line %li", file, line); }

SLONG TeakLibW_Exception(const char *file, SLONG line, const char *format, ...) {
    char buffer[128];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    AT_Error("====================================================================");
    AT_Error("Exception in File %s, Line %li:", file, line);
    AT_Error(buffer);
    AT_Error("====================================================================");
    AT_Error("C++ Exception thrown. Program will probably be terminated.");

    delete lastError;

    throw *(lastError = new TeakLibException(buffer));
}

TeakLibException *GetLastException() { return lastError; }
