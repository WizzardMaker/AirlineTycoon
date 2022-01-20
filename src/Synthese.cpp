//============================================================================================
// Synthese.cpp : Die Sprachsynthese!
//============================================================================================
#include "StdAfx.h"
#include "Synthese.h"

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

// Zum debuggen:
static const char FileId[] = "Synt";

// Die letzten beiden Sprecher:
static SBFX TalkerFx[2];
static BUFFER<SLONG> TalkerFxVon[2];
static BUFFER<SLONG> TalkerFxBis[2];
static CString TalkerId[2] = {"", ""};
static SLONG TalkerLastLoaded = 0;

//--------------------------------------------------------------------------------------------
// Rahmenfunktion:
//--------------------------------------------------------------------------------------------
void SynthesizeNumber(SBFX *TargetFx, const CString &Talker, SLONG Number, BOOL Waehrung) {
    SLONG LocalVon[100];
    SLONG LocalBis[100];
    SLONG c = 0;
    SLONG TalkerIndex = 0;
    SLONG ElementsInFusion = 0;

    if ((Sim.Options.OptionRoundNumber != 0) && (Number >= 1000 || Number <= -1000)) {
        SLONG f = 1;

        while (Number > 100 || Number < -100) {
            Number /= 10;
            f *= 10;
        }

        Number *= f;
    }

    // Den richtigen Talker-Index raussuchen:
    for (c = 0; c < 2; c++) {
        if (Talker == TalkerId[c]) {
            break;
        }
    }
    if (c == 2) {
        TalkerId[TalkerLastLoaded] = Talker;

        TalkerFx[TalkerLastLoaded].ReInit(Talker + "numbers.raw", const_cast<char *>((LPCTSTR)MiscPath));
        TalkerFx[TalkerLastLoaded].Tokenize(TalkerFxVon[TalkerLastLoaded], TalkerFxBis[TalkerLastLoaded]);

        // NumberFx.Tokenize (TalkerFx[TalkerLastLoaded]);

        c = TalkerLastLoaded;
        TalkerLastLoaded ^= 1;
    }
    TalkerIndex = c;

    if (Number == 0) {
        LocalVon[ElementsInFusion] = (TalkerFxVon[TalkerIndex])[SAY_D_NULL];
        LocalBis[ElementsInFusion] = (TalkerFxBis[TalkerIndex])[SAY_D_NULL];
        ElementsInFusion++;
        Number = -Number;
    } else {
        // Negativ?
        if (Number < 0) {
            LocalVon[ElementsInFusion] = (TalkerFxVon[TalkerIndex])[SAY_D_MINUS];
            LocalBis[ElementsInFusion] = (TalkerFxBis[TalkerIndex])[SAY_D_MINUS];
            ElementsInFusion++;
            Number = -Number;
        }

        while (Number > 0) {
            SLONG n = 0;

            if (Number >= 1000000) {
                n = Number / 1000000;
            } else if (Number >= 1000) {
                n = Number / 1000;
            } else {
                n = Number;
            }

            if (n >= 100) {
                //?-huntert...
                LocalVon[ElementsInFusion] = (TalkerFxVon[TalkerIndex])[SAY_D_1_ + (n / 100) - 1];
                LocalBis[ElementsInFusion] = (TalkerFxBis[TalkerIndex])[SAY_D_1_ + (n / 100) - 1];
                ElementsInFusion++;

                // if (Number>=1000 || n%100!=0 || Waehrung)
                if ((ElementsInFusion != 0) || Number >= 1000 || (Waehrung != 0)) {
                    LocalVon[ElementsInFusion] = (TalkerFxVon[TalkerIndex])[SAY_D_100_];
                    LocalBis[ElementsInFusion] = (TalkerFxBis[TalkerIndex])[SAY_D_100_];
                    ElementsInFusion++;
                } else {
                    LocalVon[ElementsInFusion] = (TalkerFxVon[TalkerIndex])[SAY_D_100];
                    LocalBis[ElementsInFusion] = (TalkerFxBis[TalkerIndex])[SAY_D_100];
                    ElementsInFusion++;
                }

                n -= (n / 100 * 100);
            }

            if (n > 0) {
                // 1-20...
                if (n <= 20) {
                    if (n == 1 && (Number >= 1000000 || (Number < 100 && (Waehrung != 0)))) {
                        LocalVon[ElementsInFusion] = (TalkerFxVon[TalkerIndex])[SAY_D_Eine];
                        LocalBis[ElementsInFusion] = (TalkerFxBis[TalkerIndex])[SAY_D_Eine];
                        ElementsInFusion++;
                    } else if ((ElementsInFusion != 0) || Number >= 1000 || (Waehrung != 0)) {
                        LocalVon[ElementsInFusion] = (TalkerFxVon[TalkerIndex])[SAY_D_1_ + n - 1];
                        LocalBis[ElementsInFusion] = (TalkerFxBis[TalkerIndex])[SAY_D_1_ + n - 1];
                        ElementsInFusion++;
                    } else {
                        LocalVon[ElementsInFusion] = (TalkerFxVon[TalkerIndex])[SAY_D_1 + n - 1];
                        LocalBis[ElementsInFusion] = (TalkerFxBis[TalkerIndex])[SAY_D_1 + n - 1];
                        ElementsInFusion++;
                    }
                }
                // 30-90...
                else if (n % 10 == 0) {
                    if ((ElementsInFusion != 0) || Number >= 1000 || (Waehrung != 0)) {
                        LocalVon[ElementsInFusion] = (TalkerFxVon[TalkerIndex])[SAY_D_20_ + n / 10 - 2];
                        LocalBis[ElementsInFusion] = (TalkerFxBis[TalkerIndex])[SAY_D_20_ + n / 10 - 2];
                        ElementsInFusion++;
                    } else {
                        LocalVon[ElementsInFusion] = (TalkerFxVon[TalkerIndex])[SAY_D_20 + n / 10 - 2];
                        LocalBis[ElementsInFusion] = (TalkerFxBis[TalkerIndex])[SAY_D_20 + n / 10 - 2];
                        ElementsInFusion++;
                    }
                }
                // 31, 72, ...
                else {
                    // einund...
                    LocalVon[ElementsInFusion] = (TalkerFxVon[TalkerIndex])[SAY_D_1UND_ + (n % 10) - 1];
                    LocalBis[ElementsInFusion] = (TalkerFxBis[TalkerIndex])[SAY_D_1UND_ + (n % 10) - 1];
                    ElementsInFusion++;

                    if ((ElementsInFusion != 0) || Number >= 1000 || (Waehrung != 0)) {
                        LocalVon[ElementsInFusion] = (TalkerFxVon[TalkerIndex])[SAY_D_20_ + n / 10 - 2];
                        LocalBis[ElementsInFusion] = (TalkerFxBis[TalkerIndex])[SAY_D_20_ + n / 10 - 2];
                        ElementsInFusion++;
                    } else {
                        LocalVon[ElementsInFusion] = (TalkerFxVon[TalkerIndex])[SAY_D_20 + n / 10 - 2];
                        LocalBis[ElementsInFusion] = (TalkerFxBis[TalkerIndex])[SAY_D_20 + n / 10 - 2];
                        ElementsInFusion++;
                    }
                }
            }

            if (Number >= 1000000) {
                Number = Number - Number / 1000000 * 1000000;

                if (Number > 0 || (Waehrung != 0)) {
                    LocalVon[ElementsInFusion] = (TalkerFxVon[TalkerIndex])[SAY_D_1000000_];
                    LocalBis[ElementsInFusion] = (TalkerFxBis[TalkerIndex])[SAY_D_1000000_];
                    ElementsInFusion++;
                } else {
                    LocalVon[ElementsInFusion] = (TalkerFxVon[TalkerIndex])[SAY_D_1000000];
                    LocalBis[ElementsInFusion] = (TalkerFxBis[TalkerIndex])[SAY_D_1000000];
                    ElementsInFusion++;
                }
            } else if (Number >= 1000) {
                Number = Number - Number / 1000 * 1000;

                if (Number > 0 || (Waehrung != 0)) {
                    LocalVon[ElementsInFusion] = (TalkerFxVon[TalkerIndex])[SAY_D_1000_];
                    LocalBis[ElementsInFusion] = (TalkerFxBis[TalkerIndex])[SAY_D_1000_];
                    ElementsInFusion++;
                } else {
                    LocalVon[ElementsInFusion] = (TalkerFxVon[TalkerIndex])[SAY_D_1000];
                    LocalBis[ElementsInFusion] = (TalkerFxBis[TalkerIndex])[SAY_D_1000];
                    ElementsInFusion++;
                }
            } else {
                Number = 0;
            }
        }
    }

    // Währung:
    if (Waehrung != 0) {
        LocalVon[ElementsInFusion] = (TalkerFxVon[TalkerIndex])[SAY_D_MARK];
        LocalBis[ElementsInFusion] = (TalkerFxBis[TalkerIndex])[SAY_D_MARK];
        ElementsInFusion++;
    }

    TargetFx->Fusion(&TalkerFx[TalkerIndex], LocalVon, LocalBis, ElementsInFusion);
    // TargetFx->Fusion ((const SBFX**)Elements, ElementsInFusion);
}

//--------------------------------------------------------------------------------------------
// Wirft alle Waves wieder raus:
//--------------------------------------------------------------------------------------------
void FlushTalkers() {
    TalkerId[0].Empty();
    TalkerId[1].Empty();

    TalkerFx[0].Destroy();
    TalkerFx[1].Destroy();
    TalkerFxVon[0].ReSize(0);
    TalkerFxVon[1].ReSize(0);
    TalkerFxBis[0].ReSize(0);
    TalkerFxBis[1].ReSize(0);
}
