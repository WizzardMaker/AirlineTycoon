#include "StdAfx.h"

const char *ExcOpen = "Can't open %s!";
const char *ExcRead = "Can't read %s!";
const char *ExcWrite = "Can't write %s!";
const char *ExcSeek = "Can't seek %s at %li!";

TEAKFILE::TEAKFILE()
    : Ctx(nullptr), Path(nullptr), MemPointer(0), MemBufferUsed(0)

{}

TEAKFILE::TEAKFILE(char const *path, SLONG mode) : Ctx(nullptr), Path(nullptr), MemPointer(0), MemBufferUsed(0) { Open(path, mode); }

TEAKFILE::~TEAKFILE() { Close(); }

void TEAKFILE::ReadLine(char *buffer, SLONG size) const {
    int i = 0;
    for (i = 0; i < size && (IsEof() == 0); i++) {
        char c = SDL_ReadU8(Ctx);

        if (c == '\r') {
            continue;
        }

        if (c == '\n') {
            break;
        }

        buffer[i] = c;
    }

    if (i < size) {
        buffer[i] = '\0';
    }
}

int TEAKFILE::IsEof() const { return static_cast<int>(SDL_RWtell(Ctx) >= SDL_RWsize(Ctx)); }

void TEAKFILE::Close() {
    if (Ctx != nullptr) {
        SDL_RWclose(Ctx);
    }
    Ctx = nullptr;
    if (Path != nullptr) {
        SDL_free(Path);
    }
    Path = nullptr;
}

SLONG TEAKFILE::GetFileLength() const { return static_cast<SLONG>(SDL_RWsize(Ctx)); }

SLONG TEAKFILE::GetPosition() const { return static_cast<SLONG>(SDL_RWtell(Ctx)); }

void TEAKFILE::Open(char const *path, SLONG mode) {
    Ctx = SDL_RWFromFile(path, mode == TEAKFILE_WRITE ? "wb" : "rb");
    if (Ctx == nullptr) {
        TeakLibW_Exception(nullptr, 0, ExcOpen, Path);
    }

    Path = SDL_strdup(path);
}

int TEAKFILE::IsOpen() const { return static_cast<int>(Ctx != nullptr); }

void TEAKFILE::Read(unsigned char *buffer, SLONG size) {
    if (MemBuffer.AnzEntries() > 0) {
        SLONG anz = 0;
        if (size >= MemBufferUsed - MemPointer) {
            anz = MemBufferUsed - MemPointer;
        } else {
            anz = size;
        }
        memcpy(buffer, MemPointer + MemBuffer, anz);
        MemPointer += size;
    } else {
        if (SDL_RWread(Ctx, buffer, 1, size) != size) {
            TeakLibW_Exception(nullptr, 0, ExcRead, Path);
        }
    }
}

void TEAKFILE::Write(const unsigned char *buffer, SLONG size) {
    if (MemBuffer.AnzEntries() > 0) {
        if (MemPointer + size > MemBuffer.AnzEntries()) {
            SLONG slack = MemBuffer.AnzEntries() / 10;
            MemBuffer.ReSize(slack + size + MemPointer);
        }
        memcpy(MemBuffer + MemPointer, buffer, size);
        MemPointer += size;
        MemBufferUsed += size;
    } else {
        if (SDL_RWwrite(Ctx, buffer, 1, size) != size) {
            TeakLibW_Exception(nullptr, 0, ExcWrite, Path);
        }
    }
}

void TEAKFILE::ReadTrap(SLONG trap) const {
    if (SDL_ReadLE32(Ctx) != trap) {
        DebugBreak();
    }
}

void TEAKFILE::WriteTrap(SLONG trap) const { SDL_WriteLE32(Ctx, trap); }

void TEAKFILE::SetPosition(SLONG pos) const {
    if (SDL_RWseek(Ctx, pos, RW_SEEK_SET) < 0) {
        TeakLibW_Exception(nullptr, 0, ExcSeek, Path, pos);
    }
}

void TEAKFILE::Announce(SLONG size) { MemBuffer.ReSize(size); }

CRLEReader::CRLEReader(const char *path) : Ctx(nullptr), SeqLength(0), SeqUsed(0), IsSeq(false), Sequence(), IsRLE(false), Size(0), Key(0), Path(path) {
    Ctx = SDL_RWFromFile(path, "rb");
    if (Ctx != nullptr) {
        char str[6];
        SDL_RWread(Ctx, str, sizeof(str), 1);
        if (strcmp(str, "xtRLE") == 0) {
            IsRLE = true;
            int version = SDL_ReadLE32(Ctx);
            if (version >= 0x102) {
                Key = 0xA5;
            }
            if (version >= 0x101) {
                Size = SDL_ReadLE32(Ctx);
            }
        } else {
            Size = static_cast<SLONG>(SDL_RWsize(Ctx));
            SDL_RWseek(Ctx, 0, RW_SEEK_SET);
        }
    }
}

CRLEReader::~CRLEReader() { Close(); }

bool CRLEReader::Close() {
    if (Ctx == nullptr) {
        return false;
    }
    return SDL_RWclose(Ctx) == 0;
}

void CRLEReader::SaveAsPlainText() {
    if (Ctx == nullptr) {
        return;
    }

    BUFFER_V<BYTE> buffer(GetSize());
    if (Read(buffer.getData(), buffer.AnzEntries(), true)) {
        char fn[255];
        snprintf(fn, 255, "%s.txt", Path);
        hprintf("Write to %s", fn);

        // TEAKFILE file(fn, TEAKFILE_WRITE);
        FILE *fp = fopen(fn, "w");
        for (int i = 0; i < buffer.AnzEntries(); ++i) {
            fputc(buffer[i], fp);
        }
        fclose(fp);
    }
}

bool CRLEReader::Buffer(void *buffer, SLONG size) { return SDL_RWread(Ctx, buffer, size, 1) > 0; }

bool CRLEReader::NextSeq() {
    char buf = 0;
    if (!Buffer(&buf, 1)) {
        return false;
    }
    SeqLength = buf;

    if ((SeqLength & 0x80) != 0) {
        SeqLength &= 0x7FU;
        SeqUsed = 0;
        IsSeq = true;
        if (!Buffer(Sequence, SeqLength)) {
            return false;
        }
        for (int i = 0; i < SeqLength; i++) {
            Sequence[i] ^= Key;
        }
    } else {
        IsSeq = false;
        if (!Buffer(Sequence, 1)) {
            return false;
        }
    }
    return true;
}

bool CRLEReader::Read(BYTE *buffer, SLONG size, bool decode) {
    if (!decode || !IsRLE) {
        return Buffer(buffer, size);
    }

    for (SLONG i = 0; i < size; i++) {
        if ((SeqLength == 0) && !NextSeq()) {
            return false;
        }

        if (IsSeq) {
            buffer[i] = Sequence[SeqUsed++];
            if (SeqUsed == SeqLength) {
                SeqLength = 0;
            }
        } else {
            buffer[i] = Sequence[0];
            SeqLength--;
        }
    }
    return true;
}

int DoesFileExist(char const *path) {
    SDL_RWops *ctx = SDL_RWFromFile(path, "rb");
    if (ctx != nullptr) {
        SDL_RWclose(ctx);
        return 1;
    }
#ifdef _DEBUG
    hprintf("TeakFile.cpp: File not found: %s", path);
#endif
    return 0;
}

BUFFER_V<BYTE> LoadCompleteFile(char const *path) {
    CRLEReader reader(path);
    BUFFER_V<BYTE> buffer(reader.GetSize());
    if (!reader.Read(buffer.getData(), buffer.AnzEntries(), true)) {
        return buffer;
    }

    if (reader.getIsRLE()) {
        CRLEReader konverter(path);
        konverter.SaveAsPlainText();
    }

    return buffer;
}
