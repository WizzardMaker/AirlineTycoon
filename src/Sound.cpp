//============================================================================================
// Sound.Cpp - Management von Midi und Wave Routinen
//============================================================================================
#include "StdAfx.h"
#include "Synthese.h"
#include <filesystem>

#define AT_Log(...) AT_Log_I("Sound", __VA_ARGS__)

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// static ULONG LastMidiPosition = 0;
static CString LastMidiFilename;
static SLONG AudioMode = 0;

extern FILE *pSoundLogFile;
extern SLONG SoundLogFileStartTime;

SLONG CUnrepeatedRandom::Rand(SLONG Min, SLONG Max) {
    while (true) {
        SLONG r = Random.Rand(Min, Max);

        if (r == Last[0] || r == Last[1] || r == Last[2]) {
            continue;
        }

        Last[0] = Last[1];
        Last[1] = Last[2];
        Last[2] = r;

        return (r);
    }
}

CUnrepeatedRandom MidiRandom;

//--------------------------------------------------------------------------------------------
// Spielt einen Soundeffekt auf dem Universal Soundkanal:
//--------------------------------------------------------------------------------------------
void PlayUniversalFx(const CString &Filename, SLONG Volume) {
    if (Volume != 0) {
        gUniversalFx.Stop();
        gUniversalFx.ReInit(Filename);
        gUniversalFx.Play(DSBPLAY_NOSTOP, Volume * 100 / 7);
    }
}

//--------------------------------------------------------------------------------------------
// Entfernt einen SubString wie "[[arab3200.wav]]" aus einem String:
//--------------------------------------------------------------------------------------------
CString RemoveSpeechFilename(CString String) {
    if (String.GetLength() == 0) {
        return (String);
    }

    while (true) {
        char *pstart = const_cast<char *>((LPCTSTR)String);

        char *p = strstr(pstart, "[[");
        if (p == nullptr) {
            break;
        }

        char *pp = strstr(p, "]]");
        if (pp == nullptr) {
            break;
        }

        String = String.Left(p - pstart) + String.Mid(pp - pstart + 2);

        if (p == pstart && String[0] == ' ') {
            String = String.Mid(1);
        }
    }

    while (true) {
        char *pstart = const_cast<char *>((LPCTSTR)String);

        char *p = strstr(pstart, "  ");
        if (p == nullptr) {
            break;
        }

        String = String.Left(p - pstart) + String.Mid(p - pstart + 1);
    }

    while (String.GetLength() > 0 && String[0] == ' ') {
        String = String.Mid(1);
    }

    return (String);
}

//--------------------------------------------------------------------------------------------
// Sucht einen SubString wie "[[arab3200.wav]]" aus einem String:
//--------------------------------------------------------------------------------------------
CString GetSpeechFilename(const CString &String, SLONG Index, CString *pTextFollows) {
    char *pstart = const_cast<char *>((LPCTSTR)String);

    while (Index != 0) {
        pstart = strstr(pstart + 1, "[[");
        if (pstart == nullptr) {
            return ("");
        }
        Index--;
    }

    char *p = strstr(pstart, "[[");
    if (p == nullptr) {
        return ("");
    }

    char *pp = strstr(p, "]]");
    if (pp == nullptr) {
        return ("");
    }

    if (pTextFollows != nullptr) {
        (*pTextFollows) = String.Mid(pp - const_cast<char *>((LPCTSTR)String) + 2);
    }

    return (String.Mid(p - const_cast<char *>((LPCTSTR)String) + 2, pp - p - 2));
}

//--------------------------------------------------------------------------------------------
// Erzeugt ein (zusammengesetztes) Wave, was dem Text entspricht:
//--------------------------------------------------------------------------------------------
BOOL CreateSpeechSBFX(const CString &String, SBFX *pFx, SLONG PlayerNum, BOOL *bAnyMissing) {
    CString str;
    CString path;

    static bool useOgg = false;
    static bool checked = false;

    if (!checked) {
        checked = true;

        useOgg = DoesFileExist(FullFilename("aa/100.ogg", VoicePath)); // test dummy file to check for ogg files
        AT_Log("Decided to use %s for voice files", useOgg ? ".ogg" : ".raw");
    }

    CString suffix;
    if (useOgg) {
        suffix = ".ogg";
    } else {
        suffix = ".raw";
    }

    char sep = std::filesystem::path::preferred_separator;
    CString TextFollows;
    BUFFER_V<SBFX *> Effects(50);
    SLONG c = 0;
    SLONG m = 0;
    SLONG n = 0;
    BOOL UndoWait = FALSE;

    if (gpSSE == nullptr || !gpSSE->IsSoundEnabled()) {
        return (0);
    }

    for (n = 0; n < 50; n++) {
        Effects[n] = nullptr;
    }

    for (m = n = 0; n < 50; n++) {
        if (Effects[m] == nullptr) {
            Effects[m] = new SBFX;
        }

        str = GetSpeechFilename(String, n, &TextFollows);
        if (str.GetLength() == 0) {
            break;
        }
        if ((str[0] == 'p' || str[0] == 'P') && str[1] == '1') {
            str.SetAt(1, char('1' + PlayerNum));
        }

        if (strcmp(str, "*") == 0) {
            while (TextFollows[0] == 32) {
                TextFollows = TextFollows.Mid(1);
            }

            // Check for airline:
            for (c = 0; c < 4; c++) {
                if (strnicmp(TextFollows, Sim.Players.Players[c].AirlineX, Sim.Players.Players[c].AirlineX.GetLength()) == 0) {
                    str = path + sep + "name" + bitoa(c + 1);
                    Effects[m++]->ReInit(str.ToLower() + suffix, const_cast<char *>((LPCTSTR)VoicePath));

                    CString tmp = CString(":") + bprintf("%06i", AtGetTime() - SoundLogFileStartTime) + " playing " + str + suffix + "\xd\xa";
                    if (pSoundLogFile != nullptr) {
                        fwrite(tmp, 1, strlen(tmp), pSoundLogFile);
                    }
                }
            }

            // Check for city:
            if (strcmp(str, "*") == 0) {
                for (c = 0; c < Cities.AnzEntries(); c++) {
                    if (strnicmp(TextFollows, Cities[c].Name, Cities[c].Name.GetLength()) == 0) {
                        str = path + sep + Cities[c].KuerzelReal;
                        Effects[m++]->ReInit(str.ToLower() + suffix, const_cast<char *>((LPCTSTR)VoicePath));

                        CString tmp = CString(":") + bprintf("%06i", AtGetTime() - SoundLogFileStartTime) + " playing " + str + suffix + "\xd\xa";
                        if (pSoundLogFile != nullptr) {
                            fwrite(tmp, 1, strlen(tmp), pSoundLogFile);
                        }
                    }
                }
            }

            // Check for plane:
            if (strcmp(str, "*") == 0) {
                for (c = PlaneTypes.AnzEntries() - 1; c >= 0; c--) {
                    if (PlaneTypes.IsInAlbum(c) != 0) {
                        if (strnicmp(TextFollows, PlaneTypes[c].Name, PlaneTypes[c].Name.GetLength()) == 0) {
                            str = path + sep + bprintf("pl%lib", PlaneTypes.GetIdFromIndex(c) - 0x10000000);
                            Effects[m++]->ReInit(str.ToLower() + suffix, const_cast<char *>((LPCTSTR)VoicePath));

                            CString tmp = CString(":") + bprintf("%06i", AtGetTime() - SoundLogFileStartTime) + " playing " + str + suffix + "\xd\xa";
                            if (pSoundLogFile != nullptr) {
                                fwrite(tmp, 1, strlen(tmp), pSoundLogFile);
                            }
                            break;
                        }
                        if (strnicmp(TextFollows, PlaneTypes[c].Hersteller, PlaneTypes[c].Hersteller.GetLength()) == 0) {
                            str = path + sep + bprintf("pl%lih", PlaneTypes.GetIdFromIndex(c) - 0x10000000);
                            Effects[m++]->ReInit(str.ToLower() + suffix, const_cast<char *>((LPCTSTR)VoicePath));

                            CString tmp = CString(":") + bprintf("%06i", AtGetTime() - SoundLogFileStartTime) + " playing " + str + suffix + "\xd\xa";
                            if (pSoundLogFile != nullptr) {
                                fwrite(tmp, 1, strlen(tmp), pSoundLogFile);
                            }
                            break;
                        }
                    }
                }
            }

            // Check for number:
            if (strcmp(str, "*") == 0) {
                if (TextFollows[0] == '-' || (TextFollows[0] >= '0' && TextFollows[0] <= '9') || strnicmp(TextFollows, "DM", 2) == 0) {
                    UndoWait = TRUE;
                    pCursor->SetImage(gCursorSandBm.pBitmap);
                    SLONG _x = gMousePosition.x;
                    SLONG _y = gMousePosition.y;
                    FrameWnd->TranslatePointToScreenSpace(_x, _y);
                    pCursor->MoveImage(_x - 16, _y - 16);
                    FrameWnd->Invalidate();
                    MessagePump();

#if 0
                    BOOL DM = FALSE;

                    if (strnicmp(TextFollows, "DM", 2) == 0) {
                        DM = TRUE;
                        TextFollows = TextFollows.Mid(2);
                        while (TextFollows[0] == 32) {
                            TextFollows = TextFollows.Mid(1);
                        }
                    }

                    char *p = const_cast<char *>((LPCTSTR)TextFollows);
                    SLONG Number = 0;
                    SLONG Mult = 1;

                    while (p[0] == '.' || p[0] == '-' || (p[0] >= '0' && p[0] <= '9')) {
                        if (p[0] >= '0' && p[0] <= '9') {
                            Number = Number * 10 + (p[0] - '0');
                        }
                        if (p[0] == '-') {
                            Mult = -1;
                        }

                        p++;
                    }

                    while (p[0] == 32) {
                        p++;
                    }

                    if (strnicmp(p, "DM", 2) == 0) {
                        DM = TRUE;
                    }

                    SynthesizeNumber (Effects[m++], path+"\\", Number*Mult, DM);
#endif
                }
            }
        } else {
            char *p = strchr(const_cast<char *>((LPCTSTR)str), '\\');

            if (p != nullptr) {
                path = str.Left(p - const_cast<char *>((LPCTSTR)str));

                if (path.GetLength() != str.GetLength() - 1) {
                    str.Replace('\\', sep);
                    Effects[m++]->ReInit(str.ToLower() + suffix, const_cast<char *>((LPCTSTR)VoicePath));

                    CString tmp = CString(":") + bprintf("%06i", AtGetTime() - SoundLogFileStartTime) + " playing " + str + suffix + "\xd\xa";
                    if (pSoundLogFile != nullptr) {
                        fwrite(tmp, 1, strlen(tmp), pSoundLogFile);
                    }
                }
            } else {
                Effects[m++]->ReInit(str.ToLower() + suffix, const_cast<char *>((LPCTSTR)VoicePath));

                CString tmp = CString(":") + bprintf("%06i", AtGetTime() - SoundLogFileStartTime) + " playing " + str + suffix + "\xd\xa";
                if (pSoundLogFile != nullptr) {
                    fwrite(tmp, 1, strlen(tmp), pSoundLogFile);
                }
            }

            if (m > 0) {
                if (Effects[m - 1]->pFX->GetByteLength() == 0) {
                    if (bAnyMissing != nullptr) {
                        *bAnyMissing = 1;
                    }
                    Effects[m - 1]->ReInit("none.raw");
                    hprintf("No voice for: %s", LPCTSTR(String));
                }
            }
        }
    }

    pFx->Fusion(Effects, m);

    if ((UndoWait != 0) && MouseWait == 0) {
        pCursor->SetImage(gCursorBm.pBitmap);
        SLONG _x = gMousePosition.x;
        SLONG _y = gMousePosition.y;
        FrameWnd->TranslatePointToScreenSpace(_x, _y);
        pCursor->MoveImage(_x, _y);
    }

    for (c = 0; c < 50; c++) {
        if (Effects[c] != nullptr) {
            delete Effects[c];
        }
    }

    return static_cast<BOOL>(m > 0);
}

//--------------------------------------------------------------------------------------------
// Für eine Voice hinzu:
//--------------------------------------------------------------------------------------------
void CVoiceScheduler::AddVoice(const CString &str) {
    Voices.ReSize(Voices.AnzEntries() + 1);

    Voices[Voices.AnzEntries() - 1] = str;

    if (AnzEntries() == 1) {
        CurrentVoice.ReInit(str + ".raw");
        CurrentVoice.Play(0, Sim.Options.OptionDurchsagen * 100 / 7 * AmbientManager.GlobalVolume / 100 * AmbientManager.GlobalVolume / 100);
    }
}

//--------------------------------------------------------------------------------------------
// Löscht alle Voices:
//--------------------------------------------------------------------------------------------
void CVoiceScheduler::Clear() {
    if (AnzEntries() != 0) {
        CurrentVoice.Stop();
        CurrentVoice.Destroy();
        Voices.ReSize(0);
    }
}

//--------------------------------------------------------------------------------------------
// Gibt die Anzahl der Voices im Buffer zurück:
//--------------------------------------------------------------------------------------------
SLONG CVoiceScheduler::AnzEntries() { return (Voices.AnzEntries()); }

//--------------------------------------------------------------------------------------------
// Gibt TRUE zurück, wenn gerade eine Voice gespielt wird:
//--------------------------------------------------------------------------------------------
BOOL CVoiceScheduler::IsVoicePlaying() {
    if (gpSSE == nullptr || !gpSSE->IsSoundEnabled()) {
        return (0);
    }

    if (AnzEntries() > 0 && (CurrentVoice.pFX != nullptr)) {
        dword status = 0;
        CurrentVoice.pFX->GetStatus(&status);
        return static_cast<BOOL>((status & DSBSTATUS_PLAYING) != 0);
    }
    return (FALSE);
}

//--------------------------------------------------------------------------------------------
//Überwacht die Voices:
//--------------------------------------------------------------------------------------------
void CVoiceScheduler::Pump() {
    if (AnzEntries() > 0 && (IsVoicePlaying() == 0)) {
        SLONG c = 0;

        for (c = 0; c < Voices.AnzEntries() - 1; c++) {
            Voices[c] = Voices[c + 1];
        }

        Voices.ReSize(Voices.AnzEntries() - 1);

        if (Voices.AnzEntries() > 0) {
            CurrentVoice.ReInit(Voices[0] + ".raw");
            CurrentVoice.Play(0, Sim.Options.OptionDurchsagen * 100 / 7 * AmbientManager.GlobalVolume / 100 * AmbientManager.GlobalVolume / 100);
        } else {
            CurrentVoice.Destroy();
        }
    }
}

//--------------------------------------------------------------------------------------------
// Spielt die Fanfare ab:
//--------------------------------------------------------------------------------------------
void PlayFanfare() { PlayUniversalFx("taeterae.raw", Sim.Options.OptionEffekte); }

//--------------------------------------------------------------------------------------------
// Konvertiert einen Prozentwert in Dezibel
//--------------------------------------------------------------------------------------------
SLONG Prozent2Dezibel(SLONG Prozent) {
    FLOAT rc = 0;

    if (Prozent < 0) {
        Prozent = 0;
    }
    if (Prozent > 100) {
        Prozent = 100;
    }

    // Quadratische Gleichung 2. Grades:
    // rc = FLOAT(-(Prozent-100)*(Prozent-100)*(Prozent-100)*(Prozent-100)/10000*(Prozent-100)*(Prozent-100)/10000);
    rc = (Prozent * 128) / 100;

    return (SLONG(rc));
}

//--------------------------------------------------------------------------------------------
// Skala von 0-8
//--------------------------------------------------------------------------------------------
void SetMidiVolume(SLONG volume) {

    SSE::SetMusicVolume(volume);
    /*SLONG       midiVolume;

    //CDebugEntryExit ("SetMidiVolume");

    if (gpSSE && IsMidiAvailable())
    {
    MIXERBOUNDS	mbMusic;
    gpSSE->GetMixerVolume("MIDI", &midiVolume, &mbMusic);
    if (mbMusic.dwSteps<1) mbMusic.dwSteps=1;

    SLONG StepSize=(mbMusic.lMaximum-mbMusic.lMinimum+1)/mbMusic.dwSteps;
    if (StepSize<1) StepSize=1;

    midiVolume=mbMusic.lMinimum+volume*(mbMusic.lMaximum-mbMusic.lMinimum)/7/StepSize*StepSize;
    if (midiVolume > mbMusic.lMaximum)
    midiVolume = mbMusic.lMaximum;

    gpSSE->SetMixerVolume("MIDI", midiVolume);
    } */
}

//--------------------------------------------------------------------------------------------
// Skala von 0-8
//--------------------------------------------------------------------------------------------
void SetWaveVolume(SLONG volume) {
    SSE::SetSoundVolume(volume);
    /*SLONG       waveVolume;

    //CDebugEntryExit ("SetMidiVolume");

    if (gpSSE)
    {
    MIXERBOUNDS	mbMusic;
    gpSSE->GetMixerVolume("WAVE", &waveVolume, &mbMusic);
    if (mbMusic.dwSteps<1) mbMusic.dwSteps=1;

    SLONG StepSize=(mbMusic.lMaximum-mbMusic.lMinimum+1)/mbMusic.dwSteps;
    if (StepSize<1) StepSize=1;

    waveVolume=mbMusic.lMinimum+volume*(mbMusic.lMaximum-mbMusic.lMinimum)/7/StepSize*StepSize;
    if (waveVolume > mbMusic.lMaximum)
    waveVolume = mbMusic.lMaximum;

    gpSSE->SetMixerVolume("WAVE", waveVolume);
    }*/
}

//--------------------------------------------------------------------------------------------
// Spielt das nächste Midi:
//--------------------------------------------------------------------------------------------
void NextMidi() {
    static BOOL WasHere = 0;

    if ((Sim.Options.OptionMusik != 0) && Sim.Options.OptionMusicType != 0) {
        if (WasHere != 0) {
            switch ((Sim.Options.OptionLoopMusik == 0) ? MidiRandom.Rand(0, 8) : (Sim.Options.OptionLoopMusik - 1)) {
                /*case 0:  PlayMidi ("swing.mid");    break;
                  case 1:  PlayMidi ("reggae.mid");   break;
                  case 2:  PlayMidi ("dream1g.mid");  break;
                  case 3:  PlayMidi ("indust.mid");   break;
                  case 4:  PlayMidi ("funk.mid");     break;
                  case 5:  PlayMidi ("shuffle.mid");  break; */

            case 0:
                PlayMidi("lating.mid");
                break;
            case 1:
                PlayMidi("reggaeg.mid");
                break;
            case 2:
                PlayMidi("shuffleg.mid");
                break;
            case 3:
                PlayMidi("at2.mid");
                break;
            case 4:
                PlayMidi("funky2.mid");
                break;
            case 5:
                PlayMidi("karibik.mid");
                break;
            case 6:
                PlayMidi("reag1.mid");
                break;
            case 7:
                PlayMidi("shuffle2.mid");
                break;
            case 8:
                PlayMidi("swingin2.mid");
                break;

            default:
                PlayMidi("funky2.mid");
            }
        } else {
            PlayMidi("funky2.mid");
        }
    }

    WasHere = TRUE;
}

//--------------------------------------------------------------------------------------------
// Gibt zurück, ob der Rechner Midi-fähig ist:
//--------------------------------------------------------------------------------------------
BOOL IsMidiAvailable() {
#if 0
    BUFFER_V<char> str(500);

    //CDebugEntryExit ("IsMidiAvailable");

    if (midiOutGetNumDevs()<=0) return (FALSE);

    if (mciSendString ("sysinfo sequencer quantity", str, 500 ,0)) return (FALSE);

    if (atoi(str)>0) return (TRUE);
    else return (FALSE);
#else
    return (TRUE);
#endif
}

//--------------------------------------------------------------------------------------------
// Spielt das angegebne Midi-Files:
//--------------------------------------------------------------------------------------------
void PlayMidi(const CString &Filename) {
    // CDebugEntryExit ("PlayMidi");

    if ((IsMidiAvailable() != 0) || AudioMode == 2) {
        if (gpMidi == nullptr) {
            return;
        }

        gpMidi->Stop();
        gpMidi->Load(FullFilename(Filename, SoundPath));
        gpMidi->Play();

        SSE::SetMusicCallback(NextMidi);
    }
}

//--------------------------------------------------------------------------------------------
// Spielt das angegebne Midi-Files ab der Stelle x:
//--------------------------------------------------------------------------------------------
/*void PlayMidiFrom (const CString &Filename, SLONG StartPosition)
  {
//CDebugEntryExit ("PlayMidiFrom");

CString tmpFilename = FullFilename (Filename, SoundPath);
BUFFER_V<char> ShortFilename (strlen(tmpFilename)+2);

GetShortPathName (tmpFilename, ShortFilename, strlen(tmpFilename)+1);

//Killing an old midi song:
mciSendString ("stop TeakMidi", 0, 0, 0);
mciSendString ("close TeakMidi", 0, 0, 0);

//Starting a new one:
if (!mciSendString (bprintf ("open sequencer!\"%s\" alias TeakMidi", (const char*)ShortFilename), 0, 0 ,0))
{
mciSendString (bprintf ("play TeakMidi from %li notify", StartPosition), 0, 0, FrameWnd->m_hWnd);

LastMidiPosition = StartPosition;
LastMidiFilename = Filename;
}
}*/

//--------------------------------------------------------------------------------------------
// Bricht das Spielen des aktuellen Midi-Files ab:
//--------------------------------------------------------------------------------------------
void StopMidi() {
    // CDebugEntryExit ("StopMidi");

    if (IsMidiAvailable() != 0) {
        SSE::SetMusicCallback(nullptr);
        if (gpMidi != nullptr) {
            gpMidi->Stop();
        }
    }
}

//--------------------------------------------------------------------------------------------
// Hält das Abspielen eines Midi-Files an; kann (muß aber nicht) fortgesetzt werden:
//--------------------------------------------------------------------------------------------
void PauseMidi() {
    // CDebugEntryExit ("PauseMidi");

    if (IsMidiAvailable() != 0) {
        if (gpMidi != nullptr) {
            gpMidi->Pause();
        }
    }
}

//--------------------------------------------------------------------------------------------
// Setzt das abspielen eines angehaltenen Midifiles fort:
//--------------------------------------------------------------------------------------------
void ResumeMidi() {
    // CDebugEntryExit ("ResumeMidi");

    if ((IsMidiAvailable() != 0) && (Sim.Options.OptionMusik != 0) && Sim.Options.OptionMusicType != 0) {
        if (gpMidi != nullptr) {
            gpMidi->Resume();
        }
    }
}

//--------------------------------------------------------------------------------------------
// Ein Soundeffekt:
//--------------------------------------------------------------------------------------------
void SBFX::Destroy() {
    if (pFX != nullptr) {
        pFX->Release();
        pFX = nullptr;
    }
}

void SBFX::Fusion(BUFFER_V<SBFX *>& Fx, SLONG NumFx) {
    std::vector<FX *> Elements;
    Elements.resize(NumFx);

    Destroy();

    for (SLONG c = 0; c < NumFx; c++) {
        Elements[c] = Fx[c]->pFX;
    }

    if (gpSSE != nullptr) {
        gpSSE->CreateFX(&pFX);
        pFX->Fusion(Elements);
    }
}

void SBFX::Fusion(const SBFX *Fx, const SLONG *Von, const SLONG *Bis, SLONG NumFx) {
    Destroy();

    if (gpSSE != nullptr) {
        gpSSE->CreateFX(&pFX);
        pFX->Fusion(Fx->pFX, const_cast<SLONG *>(Von), const_cast<SLONG *>(Bis), NumFx);
    }
}

void SBFX::Tokenize(BUFFER_V<SBFX> &Effects) const {
    SLONG c = 0;
    SLONG Anzahl = 0;
    FX **ppFx = pFX->Tokenize(0x80007FFF80007FFF, Anzahl);

    Effects.ReSize(0);
    Effects.ReSize(Anzahl);
    for (c = 0; c < Anzahl; c++) {
        Effects[c].pFX = ppFx[c];
    }

    delete ppFx;
}

void SBFX::Tokenize(BUFFER_V<SLONG> &Von, BUFFER_V<SLONG> &Bis) const {
    SLONG Anzahl = 0;

    Von.ReSize(100);
    Bis.ReSize(100);

    pFX->Tokenize(0x80007FFF80007FFF, static_cast<SLONG *>(Von.data()), static_cast<SLONG *>(Bis.data()), Anzahl);

    Von.ReSize(Anzahl);
    Bis.ReSize(Anzahl);
}

void SBFX::ReInit(const CString &Filename, char *Path) {
    CString localPath;

    Destroy();

    if (Path == nullptr) {
        localPath = SoundPath;
    } else {
        localPath = Path;
    }

    if (gpSSE != nullptr) {
        gpSSE->CreateFX(&pFX);
        pFX->Load(const_cast<char *>((LPCTSTR)FullFilename(Filename, localPath)));

        SBFX::Filename = FullFilename(Filename, localPath);
    }
}

void SBFX::Play(dword dwFlags) const {
    if ((pFX != nullptr) && (Sim.Options.OptionDigiSound != 0)) {
        pFX->Play(dwFlags);
    }

    if (pSoundLogFile != nullptr) {
        fwrite(Filename, 1, strlen(Filename), pSoundLogFile);
    }
}

void SBFX::Play(dword dwFlags, SLONG PercentVolume) const {
    if ((pFX != nullptr) && (Sim.Options.OptionDigiSound != 0)) {
        pFX->Play(dwFlags);
        pFX->SetVolume(Prozent2Dezibel(PercentVolume));
    }

    CString Tmp = CString(":") + bprintf("%06i", AtGetTime() - SoundLogFileStartTime) + " playing " + Filename + "\xd\xa";
    if ((pSoundLogFile != nullptr) && Filename.GetLength() > 1) {
        fwrite(Tmp, 1, strlen(Tmp), pSoundLogFile);
    }
}

void SBFX::Stop() const {
    if (pFX != nullptr) {
        pFX->Stop();
    }
}

void SBFX::SetVolume(SLONG volume) const {
    if (pFX != nullptr) {
        pFX->SetVolume(volume);
    }
}

//--------------------------------------------------------------------------------------------
// Der Ambiente-Manager: Einen Soundeffekt initialisieren
//--------------------------------------------------------------------------------------------
void CAmbienteManager::SetFx(SLONG FxId, const CString &Soundeffekt) {
    if (FxId >= AmbientFx.AnzEntries()) {
        return;
    }

    AmbientFx[FxId].Volume = 0;
    AmbientFx[FxId].CurrentVolume = 0;
    AmbientFx[FxId].Soundeffekt.ReInit(Soundeffekt);
}

//--------------------------------------------------------------------------------------------
// Die globale Laufstärke setzen
//--------------------------------------------------------------------------------------------
void CAmbienteManager::SetGlobalVolume(SLONG Volume) {
    GlobalVolume = Volume;

    gUniversalPlaneFx.SetVolume(Prozent2Dezibel(Sim.Options.OptionPlaneVolume * 100 / 7 * AmbientManager.GlobalVolume / 100));

    RecalcVolumes();
}

//--------------------------------------------------------------------------------------------
// Die Lautstärke für einen Effekt setzen:
//--------------------------------------------------------------------------------------------
void CAmbienteManager::SetVolume(SLONG FxId, SLONG Volume) {
    SLONG NewVolume = Volume * GlobalVolume / 100;

    if (FxId >= AmbientFx.AnzEntries()) {
        return;
    }

    if (NewVolume > 100) {
        NewVolume = 100;
    }
    if (NewVolume < 0) {
        NewVolume = 0;
    }
    if (Volume > 100) {
        Volume = 100;
    }
    if (Volume < 0) {
        Volume = 0;
    }

    AmbientFx[FxId].Volume = Volume;

    if (Sim.Options.OptionAmbiente == 0) {
        NewVolume = 0;
    } else {
        NewVolume = NewVolume * Sim.Options.OptionAmbiente / 8;
    }

    if (NewVolume != AmbientFx[FxId].CurrentVolume) {
        // Nötigenfalls überhaupt erst einmal starten:
        if (AmbientFx[FxId].CurrentVolume == 0) {
            AmbientFx[FxId].Soundeffekt.Play(DSBPLAY_NOSTOP | DSBPLAY_LOOPING);
        }

        AmbientFx[FxId].CurrentVolume = NewVolume;

        AmbientFx[FxId].Soundeffekt.SetVolume(Prozent2Dezibel(NewVolume));

        // Nötigenfalls ganz anhalten:
        if (AmbientFx[FxId].CurrentVolume == 0) {
            AmbientFx[FxId].Soundeffekt.Stop();
        }
    }
}

//--------------------------------------------------------------------------------------------
// Die Lautstärke neu berechnen:
//--------------------------------------------------------------------------------------------
void CAmbienteManager::RecalcVolumes() {
    for (SLONG c = 0; c < AmbientFx.AnzEntries(); c++) {
        if (c == AMBIENT_JET_OUTSIDE || c == AMBIENT_JET_FIELD) {
            SetVolume(c, Prozent2Dezibel(Sim.Options.OptionPlaneVolume * 100 / 7 * AmbientFx[c].Volume / 100));
        } else {
            SetVolume(c, AmbientFx[c].Volume);
        }
    }
}

//--------------------------------------------------------------------------------------------
// Schaltet in den Pause-Modus:
//--------------------------------------------------------------------------------------------
void CAmbienteManager::Pause() const {
    for (SLONG c = 0; c < AmbientFx.AnzEntries(); c++) {
        if (AmbientFx[c].Soundeffekt.pFX != nullptr) {
            if (AmbientFx[c].CurrentVolume == 0) {
                AmbientFx[c].Soundeffekt.pFX->Pause();
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Setzt das Ambiente fort:
//--------------------------------------------------------------------------------------------
void CAmbienteManager::Resume() {
    for (SLONG c = 0; c < AmbientFx.AnzEntries(); c++) {
        if (AmbientFx[c].Soundeffekt.pFX != nullptr) {
            if (AmbientFx[c].CurrentVolume > 0) {
                SetVolume(c, AmbientFx[c].Volume);
                AmbientFx[c].Soundeffekt.pFX->Resume();
            }
        }
    }
}
#if 0
void CloseMixer(HMIXER* phMixer)
{
    if (*phMixer)
    {
        mixerClose(*phMixer);
        *phMixer = 0;
    }
}
#endif
#define SND_TYPE unsigned char

SND_TYPE Tokens[] = {64, 135, 192, 2, 254, 100};

BOOL Near(SND_TYPE a, SND_TYPE b);

BOOL Near(SND_TYPE a, SND_TYPE b) {
    if (abs(SLONG(a) - SLONG(b)) < 3) {
        return (TRUE);
    }
    if (b == 128) {
        return (FALSE);
    }
    if ((abs(SLONG(a) - SLONG(b)) - 3) * 256 / (abs(SLONG(a) - 128) + abs(SLONG(b) - 128)) < 40) {
        return (TRUE);
    }

    return (FALSE);
}

void CompressWave(BUFFER_V<SND_TYPE> &Input, BUFFER_V<SND_TYPE> &Output);

void CompressWave(BUFFER_V<SND_TYPE> &Input, BUFFER_V<SND_TYPE> &Output) {
    SLONG c = 0;
    SLONG d = 0;
    SLONG e = 0;
    SLONG o = 0;
    SLONG t = 0; // toleranz

    for (c = Input.AnzEntries() - 1; c >= 0; c--) {
        for (d = 0; d < sizeof(Tokens); d++) {
            if (Input[c] == Tokens[d]) {
                Input[c]++;
            }
        }
    }

    Output.ReSize(Input.AnzEntries());

    o = 0;
    for (c = 0; c < Input.AnzEntries(); c++) {
        if (abs(Input[c] - 128) > 7) {
            for (d = 20; c > d && d < 275 + 255; d++) {
                // if (abs(SLONG(Input[c])-SLONG(Input[c-d]))<t && abs(SLONG(Input[c+1])-SLONG(Input[c-d+1]))<t && abs(SLONG(Input[c+2])-SLONG(Input[c-d+2]))<t)
                if ((Near(Input[c], Input[c - d]) != 0) && (Near(Input[c + 1], Input[c - d + 1]) != 0) && (Near(Input[c + 2], Input[c - d + 2]) != 0)) {
                    BOOL Equal = TRUE;

                    for (e = 1; e < d; e += 2) {
                        if ((Near(Input[c + e], Input[c - d + e]) == 0) && (Near(Input[c + e], Input[c - d + e + 1]) == 0) &&
                            (Near(Input[c + e], Input[c - d + e - 1]) == 0)) {
                            Equal = FALSE;
                            break;
                        }
                    }

                    if (Equal != 0) {
                        for (e = 2; e < d; e += 2) {
                            if ((Near(Input[c + e], Input[c - d + e]) == 0) && (Near(Input[c + e], Input[c - d + e + 1]) == 0) &&
                                (Near(Input[c + e], Input[c - d + e - 1]) == 0)) {
                                Equal = FALSE;
                                break;
                            }
                        }
                    }

                    if (Equal != 0) {
                        if (d - 20 >= 256) {
                            Output[o++] = Tokens[5];
                            Output[o++] = static_cast<SND_TYPE>(d - 20 - 256);
                        } else {
                            Output[o++] = Tokens[4];
                            Output[o++] = static_cast<SND_TYPE>(d - 20);
                        }

                        c += (d - 1);
                        break;
                    }
                }
            }
            if (d != 275 + 255 && c > d) {
                continue;
            }
        }

        if (abs(Input[c] - 128) < 4) {
            t = 3;
        } else {
            t = 2;
        }
        if (abs(SLONG(Input[c]) - SLONG(Input[c + 1])) < t && abs(SLONG(Input[c]) - SLONG(Input[c + 2])) < t &&
            abs(SLONG(Input[c]) - SLONG(Input[c + 3])) < t) {
            for (d = 0; d < 255 && c + d < Input.AnzEntries(); d++) {
                if (!(abs(SLONG(Input[c]) - SLONG(Input[c + d])) < t)) {
                    break;
                }
            }

            if (d == 3) {
                Output[o++] = Tokens[2];
                Output[o++] = Input[c];
            } else if (d == 4) {
                Output[o++] = Tokens[1];
                Output[o++] = Input[c];
            } else {
                if (Input[c] == 127 || Input[c] == 128 || Input[c] == 129) {
                    Output[o++] = Tokens[3];
                    Output[o++] = static_cast<SND_TYPE>(d);
                } else {
                    Output[o++] = Tokens[0];
                    Output[o++] = Input[c];
                    Output[o++] = static_cast<SND_TYPE>(d);
                }
            }

            c += (d - 1);
        } else {
            Output[o++] = Input[c];
            // if (Output[o-1]==Tokens[0]) DebugBreak();
        }
    }

    Output.ReSize(o);
}

void DecompressWave(BUFFER_V<SND_TYPE> &Input, BUFFER_V<SND_TYPE> &Output);

void DecompressWave(BUFFER_V<SND_TYPE> &Input, BUFFER_V<SND_TYPE> &Output) {
    SLONG c = 0;
    SLONG d = 0;
    SLONG o = 0;

    o = 0;
    for (c = 0; c < Input.AnzEntries(); c++) {
        if (Input[c] == Tokens[0]) {
            for (d = 0; d < Input[c + 2]; d++) {
                Output[o++] = Input[c + 1];
            }

            c += 2;
        } else if (Input[c] == Tokens[1]) {
            for (d = 0; d < 4; d++) {
                Output[o++] = Input[c + 1];
            }

            c++;
        } else if (Input[c] == Tokens[2]) {
            for (d = 0; d < 3; d++) {
                Output[o++] = Input[c + 1];
            }

            c++;
        } else if (Input[c] == Tokens[3]) {
            for (d = 0; d < Input[c + 1]; d++) {
                Output[o++] = 127 + rand() % 2;
            }

            c++;
        } else if (Input[c] == Tokens[4]) {
            for (d = 0; d < Input[c + 1] + 20; d++) {
                Output[o] = Output[o - Input[c + 1] - 20];
                o++;
            }

            c++;
        } else if (Input[c] == Tokens[5]) {
            for (d = 0; d < Input[c + 1] + 20 + 256; d++) {
                Output[o] = Output[o - Input[c + 1] - 20];
                o++;
            }

            c++;
        } else {
            Output[o++] = Input[c];
        }
    }
}
