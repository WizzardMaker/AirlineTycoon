#include "StdAfx.h"

#include <cstdio>
#include <stdexcept>

const char *ExcAssert = "Assert (called from %s:%li) failed!";
const char *ExcGuardian = "Con/Des guardian %lx failed!";
const char *ExcStrangeMem = "Strange new: %li bytes!";
const char *ExcOutOfMem = "Couldn't allocate %li bytes!";
const char *ExcNotImplemented = "Function not implemented!";
const char *ExcImpossible = "Impossible Event %s occured";

HDU Hdu;
TeakLibException *lastError;


HDU::HDU() : Log(nullptr) {
    char *base = SDL_GetBasePath();
    const char *file = "debug.txt";
    BUFFER_V<char> path(strlen(base) + strlen(file) + 1);
    strcpy(path.data(), base);
    strcat(path.data(), file);
    Log = fopen(path.data(), "w");
    //Log = stdout;
    SDL_free(base);

    SDL_LogOutputFunction defaultOut;
    SDL_LogGetOutputFunction(&defaultOut, nullptr);

#ifdef _DEBUG
    SDL_LogSetAllPriority(SDL_LOG_PRIORITY_VERBOSE);
#endif

    auto func = [](void* userdata, int category, SDL_LogPriority priority, const char* message) {
        char* finalMessage = const_cast<char*>(message);

        bool modified = false;
        if (strstr(message, "||") == nullptr) {
            const unsigned long long size = strlen(message) + strlen("Misc || ") + 1;
            finalMessage = new char[size]{};
            snprintf(finalMessage, size, "Misc || %s", message);
            modified = true;
        }

        const SDL_LogOutputFunction func = reinterpret_cast<SDL_LogOutputFunction>(userdata);
        func(userdata, category, priority, finalMessage);

        if(Hdu.Log){
            fprintf(Hdu.Log, "%s\n", finalMessage);
            fflush(Hdu.Log);
        }

        if(modified)
            delete[] finalMessage;
    };

    SDL_LogSetOutputFunction(func, reinterpret_cast<void*>(defaultOut));
}

HDU::~HDU()
{
    Close();
}

void HDU::Close() {
    if (Log != nullptr) {
        fclose(Log);
    }
    Log = nullptr;
}

void HDU::HercPrintf(SLONG /*unused*/, const char *format, ...) {
    if (Log == nullptr) {
        return;
    }
    va_list args;
    va_start(args, format);
    //vfprintf(Log, format, args);
    V_AT_Log_I("Herc",format, args);
    va_end(args);
}

void HDU::HercPrintf(const char *format, ...) {
    if (Log == nullptr) {
        return;
    }
    va_list args;
    va_start(args, format);
    //vfprintf(Log, format, args);
    V_AT_Log_I("Herc", format, args);
    va_end(args);
}

void here(char *file, SLONG line) { Hdu.HercPrintf(0, "Here in %s, line %li", file, line); }

SLONG TeakLibW_Exception(const char *file, SLONG line, const char *format, ...) {
    char buffer[128];
    va_list args;
    va_start(args, format);
    vsnprintf(buffer, sizeof(buffer), format, args);
    va_end(args);

    Hdu.HercPrintf(1, "====================================================================");
    Hdu.HercPrintf(0, "Exception in File %s, Line %li:", file, line);
    Hdu.HercPrintf(0, buffer);
    Hdu.HercPrintf(1, "====================================================================");
    Hdu.HercPrintf(0, "C++ Exception thrown. Program will probably be terminated.");

    delete lastError;
    
    throw *(lastError = new TeakLibException(buffer));
}

TeakLibException *GetLastException() { return lastError; }