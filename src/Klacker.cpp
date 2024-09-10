//============================================================================================
// Klacker.cpp : Routinen für die Basisfunktionen der Klackertafel:
//============================================================================================
#include "StdAfx.h"

//--------------------------------------------------------------------------------------------
// Initialisiert ein paar Dinge:
//--------------------------------------------------------------------------------------------
KLACKER::KLACKER() {
    // Klacker-Tafel initialisieren:
    Clear();
    Haben.fill(0);

    pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename("klacker.gli", RoomPath)), &pGLib, L_LOCMEM);
    Cursors.ReSize(pGLib, "KL_K1", 8); // 8 neue Chars für die Polen........................... //3 neue Chars füt SL //3 Neue für ES
    KlackerBms.ReSize(
        pGLib, "KL_SP KL_A KL_B KL_C KL_D KL_E KL_F KL_G KL_H KL_I KL_J KL_K KL_L KL_M KL_N KL_O KL_P KL_Q KL_R KL_S KL_T KL_U KL_V KL_W KL_X KL_Y KL_Z KL_AE "
               "KL_OE KL_UE KL_0 KL_1 KL_2 KL_3 KL_4 KL_5 KL_6 KL_7 KL_8 KL_9 KL_PKT KL_AUSR KL_DPKT KL_SEMI KL_KOMMA KL_FRAGE KL_KLA KL_KLZ KL_MINUS KL_BULL "
               "KL_GLCH KL_MAL KL_KREUZ KL_MORE KL_LESS KL_PL KL_193 KL_200 KL_207 KL_201 KL_204 KL_205 KL_210 KL_211 KL_216 KL_138 KL_141 KL_218 KL_217 "
               "KL_221 KL_142 KL_165 KL_198 KL_202 KL_163 KL_209 KL_140 KL_143 KL_175 KL_188 KL_192 KL_197 KL_165B KL_195 KL_128 KL_HUMAN KL_COMPI "
               "KD_SP KD_A KD_B KD_C KD_D KD_E KD_F KD_G KD_H KD_I KD_J KD_K KD_L KD_M KD_N KD_O KD_P KD_Q KD_R KD_S KD_T KD_U KD_V KD_W KD_X KD_Y KD_Z KD_AE "
               "KD_OE KD_UE KD_0 KD_1 KD_2 KD_3 KD_4 KD_5 KD_6 KD_7 KD_8 KD_9 KD_PKT KD_AUSR KD_DPKT KD_SEMI KD_KOMMA KD_FRAGE KD_KLA KD_KLZ KD_MINUS KD_BULL "
               "KD_GLCH KD_MAL KD_KREUZ KD_MORE KD_LESS KL_PL KD_193 KD_200 KD_207 KD_201 KD_204 KD_205 KD_210 KD_211 KD_216 KD_138 KD_141 KD_218 KD_217 "
               "KD_221 KD_142 KD_165 KD_198 KD_202 KD_163 KD_209 KD_140 KD_143 KD_175 KD_188 KD_192 KD_197 KD_165B KD_195 KD_128");

    KlackerFx[0].ReInit("klack0.raw");
    KlackerFx[1].ReInit("klack1.raw");
    KlackerFx[2].ReInit("klack2.raw");
}

//--------------------------------------------------------------------------------------------
// Der Destruktor::
//--------------------------------------------------------------------------------------------
KLACKER::~KLACKER() {
    Cursors.Destroy();
    KlackerBms.Destroy();

    if ((pGLib != nullptr) && (pGfxMain != nullptr)) {
        pGfxMain->ReleaseLib(pGLib);
        pGLib = nullptr;
    }
}

//--------------------------------------------------------------------------------------------
// Löscht alle Inhalte:
//--------------------------------------------------------------------------------------------
void KLACKER::Clear() {
    Soll.fill(0);

    for (bool &c : LineDisabled) {
        c = false;
    }
}

//--------------------------------------------------------------------------------------------
// Blättert alle Zeichen die es nötig haben einen Zeile weiter:
//--------------------------------------------------------------------------------------------
BOOL KLACKER::Klack() {
    SLONG c = 0;
    SLONG d = 0;
    SLONG Anz = 0;
    SLONG OrgAnz = 0;
    SLONG l = strlen(KlackerFntDef);
    BOOL rc = FALSE;

    SLONG AnzKlack = 0;

    for (c = 0; c < 16 * 24; c++) {
        if (Soll[c] != Haben[c]) {
            rc = TRUE;

            AnzKlack++;
            OrgAnz = Anz = (Soll[c] + l - Haben[c]) % l;

            if (Anz > 20) {
                Anz = 9;
            } else if (Anz > 10) {
                Anz = 7;
            } else if (Anz > 6) {
                Anz = 4;
            } else if (Anz > 3) {
                Anz = 2;
            } else {
                Anz = 1;
            }

            for (d = 0; d < Anz; d++) {
                Haben[c]++;
                if (KlackerFntDef[Haben[c]] == 0) {
                    Haben[c] = 0;
                }

                if (Haben[c] == 27 && Soll[c] < 27) {
                    Haben[c] = 0;
                } else if (Haben[c] == 30 && Soll[c] < 30) {
                    Haben[c] = 0;
                } else if (Haben[c] == 40 && Soll[c] < 40) {
                    Haben[c] = 0;
                }
                if (Haben[c] >= l - 2) {
                    Haben[c] = 0;
                }
            }

            if (OrgAnz < (Soll[c] + l - Haben[c]) % l) {
                Haben[c] = Soll[c];
            }
        }
    }

    if (AnzKlack > 0) {
        KlackerFx[0].Play(DSBPLAY_FIRE, Sim.Options.OptionEffekte * 100 / 7);

        if (AnzKlack > 20) {
            KlackerFx[1].Play(DSBPLAY_FIRE, Sim.Options.OptionEffekte * 100 / 7);
            if (AnzKlack > 50) {
                KlackerFx[2].Play(DSBPLAY_FIRE, Sim.Options.OptionEffekte * 100 / 7);
            }
        }
    }

    return (rc);
}

//--------------------------------------------------------------------------------------------
// Gibt TRUE zurück, wenn nichts mehr zu klackern ist:
//--------------------------------------------------------------------------------------------
BOOL KLACKER::IsFinished() const { return static_cast<BOOL>(Soll == Haben); }

//--------------------------------------------------------------------------------------------
// Bringt alle Felder sofort auf den Soll-Stand:
//--------------------------------------------------------------------------------------------
void KLACKER::Warp() {
    SLONG c = 0;

    for (c = 0; c < 16 * 24; c++) {
        Haben[c] = Soll[c];
    }
}

//--------------------------------------------------------------------------------------------
// Schreibt einen Text-String in die Soll-Bitmap:
//--------------------------------------------------------------------------------------------
void KLACKER::PrintAt(SLONG x, SLONG y, const char *Text) {
    SLONG c = 0;

    for (c = 0; c + x < 24 && (Text[c] != 0); c++) {
        char ch = Text[c];

        ch = GerToUpper(ch);
        if (gLanguage != LANGUAGE_1 && gLanguage != LANGUAGE_E) {
            if (ch == '\xC9' || ch == '\xC8' || ch == '\xCA' || ch == '\xE9' || ch == '\xE8' || ch == '\xEA') {
                ch = 'E';
            }
            if (ch == '\xE1' || ch == '\xC1' || ch == '\xE0' || ch == '\xC0' || ch == '\xE3') {
                ch = 'A';
            }
            if (ch == '\xED' || ch == '\xCD' || ch == '\xEC' || ch == '\xCC') {
                ch = 'I';
            }
            if (ch == '\xFA' || ch == '\xDA') {
                ch = 'U';
            }
            if (ch == '\xF3' || ch == '\xD3') {
                ch = 'O';
            }
            if (ch == '\xE7') {
                ch = 'C';
            }
        }
        if (gLanguage == LANGUAGE_1) {
            if (ch == static_cast<char>(225)) {
                ch = static_cast<char>(193);
            }
            if (ch == static_cast<char>(232)) {
                ch = static_cast<char>(200);
            }
            if (ch == static_cast<char>(239)) {
                ch = static_cast<char>(207);
            }
            if (ch == static_cast<char>(233)) {
                ch = static_cast<char>(201);
            }
            if (ch == static_cast<char>(236)) {
                ch = static_cast<char>(204);
            }
            if (ch == static_cast<char>(237)) {
                ch = static_cast<char>(205);
            }
            if (ch == static_cast<char>(242)) {
                ch = static_cast<char>(210);
            }
            if (ch == static_cast<char>(243)) {
                ch = static_cast<char>(211);
            }
            if (ch == static_cast<char>(248)) {
                ch = static_cast<char>(216);
            }
            if (ch == static_cast<char>(154)) {
                ch = static_cast<char>(138);
            }
            if (ch == static_cast<char>(157)) {
                ch = static_cast<char>(141);
            }
            if (ch == static_cast<char>(250)) {
                ch = static_cast<char>(218);
            }
            if (ch == static_cast<char>(249)) {
                ch = static_cast<char>(217);
            }
            if (ch == static_cast<char>(253)) {
                ch = static_cast<char>(221);
            }
            if (ch == static_cast<char>(158)) {
                ch = static_cast<char>(142);
            }
        }
        if (gLanguage == LANGUAGE_E) // E=Polnisch! Krank!
        {
            if (ch == static_cast<char>(185)) {
                ch = static_cast<char>(165);
            }
            if (ch == static_cast<char>(230)) {
                ch = static_cast<char>(198);
            }
            if (ch == static_cast<char>(234)) {
                ch = static_cast<char>(202);
            }
            if (ch == static_cast<char>(179)) {
                ch = static_cast<char>(163);
            }
            if (ch == static_cast<char>(241)) {
                ch = static_cast<char>(209);
            }
            if (ch == static_cast<char>(243)) {
                ch = static_cast<char>(211);
            }
            if (ch == static_cast<char>(156)) {
                ch = static_cast<char>(140);
            }
            if (ch == static_cast<char>(159)) {
                ch = static_cast<char>(143);
            }
            if (ch == static_cast<char>(191)) {
                ch = static_cast<char>(175);
            }
        }
        if (gLanguage == LANGUAGE_S || gLanguage == LANGUAGE_O) {
            if (ch == static_cast<char>(164)) {
                ch = static_cast<char>(165);
            }
            if (ch == static_cast<char>(227)) {
                ch = static_cast<char>(195);
            }
            if (ch == static_cast<char>(165)) {
                ch = static_cast<char>(166); // Fix, weil 165 schon von Tschechen o. Polen belegt ist
            }
        }

        const auto *p = strchr(KlackerFntDef, ch);

        if (p == nullptr) {
            Soll[c + x + y * 24] = 0;
        } else {
            Soll[c + x + y * 24] = UBYTE(p - KlackerFntDef);
        }
    }
}

//--------------------------------------------------------------------------------------------
// Druckt eine Volume-Anzeige wohin:
//--------------------------------------------------------------------------------------------
void KLACKER::PrintVolumeAt(SLONG x, SLONG y, SLONG Maximum, SLONG Current) {
    SLONG c = 0;
    const char *p = nullptr;

    for (c = 0; c + x < 24 && c < Maximum; c++) {
        p = strchr(KlackerFntDef, '-');

        if (p == nullptr) {
            Soll[c + x + y * 24] = 0;
        } else {
            Soll[c + x + y * 24] = UBYTE(p - KlackerFntDef);
        }
    }

    if (x + Current < 24) {
        p = strchr(KlackerFntDef, '|');

        if (p == nullptr) {
            Soll[x + Current + y * 24] = 0;
        } else {
            Soll[x + Current + y * 24] = UBYTE(p - KlackerFntDef);
        }
    }
}

//--------------------------------------------------------------------------------------------
// Entfernt alles Flugzeuge:
//--------------------------------------------------------------------------------------------
void CKlackerPlanes::Reset() {
    KlackerPlanes.ReSize(10);

    for (SLONG c = 0; c < KlackerPlanes.AnzEntries(); c++) {
        KlackerPlanes[c].Size = -1;
    }

    TimeSinceStart = 0;
}

//--------------------------------------------------------------------------------------------
// Bewegt die Flugzeuge und macht neue
//--------------------------------------------------------------------------------------------
void CKlackerPlanes::Pump(XY AvoidPoint) {
    TimeSinceStart++;

    for (SLONG c = 0; c < KlackerPlanes.AnzEntries(); c++) {
        if (KlackerPlanes[c].Size != -1) {
            KlackerPlanes[c].ScreenPos.x += KlackerPlanes[c].Dir;

            if (KlackerPlanes[c].Dir > 0 && KlackerPlanes[c].ScreenPos.x > 640) {
                KlackerPlanes[c].Size = -1;
            }
            if (KlackerPlanes[c].Dir < 0 && KlackerPlanes[c].ScreenPos.x < -190 - KlackerPlanes[c].Size * 35) {
                KlackerPlanes[c].Size = -1;
            }

            if (AvoidPoint.y > KlackerPlanes[c].ScreenPos.y - 100 && AvoidPoint.y < KlackerPlanes[c].ScreenPos.y + 20) {
                if (AvoidPoint.y > KlackerPlanes[c].ScreenPos.y - 50) {
                    KlackerPlanes[c].ScreenPos.y -= 4;
                }
                if (AvoidPoint.y < KlackerPlanes[c].ScreenPos.y - 50) {
                    KlackerPlanes[c].ScreenPos.y += 4;
                }
            }
        }
    }

    if ((TimeSinceStart > 600 && TimeSinceStart % 50 == 0) || (TimeSinceStart > 800 && TimeSinceStart % 44 == 0) ||
        (TimeSinceStart > 1100 && TimeSinceStart % 47 == 0)) {
        for (SLONG c = 0; c < KlackerPlanes.AnzEntries(); c++) {
            if (KlackerPlanes[c].Size == -1) {
                KlackerPlanes[c].Dir = rand() % 2 * 20 - 10;
                KlackerPlanes[c].Logo = rand() % 4;
                KlackerPlanes[c].Size = min(20, rand() % (((TimeSinceStart - 580) / 300) + 1));
                KlackerPlanes[c].ScreenPos.y = rand() % 400 + 80;

                if (KlackerPlanes[c].Dir < 0) {
                    KlackerPlanes[c].ScreenPos.x = 640;
                } else {
                    KlackerPlanes[c].ScreenPos.x = -190 - KlackerPlanes[c].Size * 35;
                }
                break;
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Blittet die Flugzeuge auf den Screen
//--------------------------------------------------------------------------------------------
void CKlackerPlanes::PostPaint(SBBM &PrimaryBm) const {
    for (SLONG c = 0; c < KlackerPlanes.AnzEntries(); c++) {
        if (KlackerPlanes[c].Size != -1) {
            SLONG x = KlackerPlanes[c].ScreenPos.x;

            if (KlackerPlanes[c].Dir < 0) {
                // Bug:
                PrimaryBm.BlitFromT(gUniversalPlaneBms[30], x, KlackerPlanes[c].ScreenPos.y - gUniversalPlaneBms[9 + 0].Size.y);
                if (KlackerPlanes[c].Logo > 0) {
                    PrimaryBm.BlitFromT(gUniversalPlaneBms[9 + KlackerPlanes[c].Logo * 5 + 0], x,
                                        KlackerPlanes[c].ScreenPos.y - gUniversalPlaneBms[9 + 0].Size.y);
                }
                x += gUniversalPlaneBms[9 + KlackerPlanes[c].Logo * 5 + 0].Size.x;

                // Körper:
                for (SLONG d = 0; d < KlackerPlanes[c].Size; d++) {
                    PrimaryBm.BlitFromT(gUniversalPlaneBms[31], x, KlackerPlanes[c].ScreenPos.y - gUniversalPlaneBms[9 + 1].Size.y);
                    if (KlackerPlanes[c].Logo != 0) {
                        PrimaryBm.BlitFromT(gUniversalPlaneBms[9 + KlackerPlanes[c].Logo * 5 + 1], x,
                                            KlackerPlanes[c].ScreenPos.y - gUniversalPlaneBms[9 + 1].Size.y);
                    }
                    x += gUniversalPlaneBms[9 + KlackerPlanes[c].Logo * 5 + 1].Size.x;
                }

                // Heck & Triebwerk:
                PrimaryBm.BlitFromT(gUniversalPlaneBms[4],
                                    x - gUniversalPlaneBms[9 + 1].Size.x * KlackerPlanes[c].Size / 2 - gUniversalPlaneBms[4].Size.x / 2 + 20 + 40,
                                    KlackerPlanes[c].ScreenPos.y - 20 - 35 - 63);
                PrimaryBm.BlitFromT(gUniversalPlaneBms[29], x, KlackerPlanes[c].ScreenPos.y - gUniversalPlaneBms[9 + 2].Size.y + 19);
                if (KlackerPlanes[c].Logo != 0) {
                    PrimaryBm.BlitFromT(gUniversalPlaneBms[9 + KlackerPlanes[c].Logo * 5 + 2], x,
                                        KlackerPlanes[c].ScreenPos.y - gUniversalPlaneBms[9 + 2].Size.y + 19);
                }
                PrimaryBm.BlitFromT(gUniversalPlaneBms[6],
                                    x - gUniversalPlaneBms[9 + 1].Size.x * KlackerPlanes[c].Size / 2 - gUniversalPlaneBms[4].Size.x / 2 + 40,
                                    KlackerPlanes[c].ScreenPos.y - 20 - 28);
            } else {
                // Triebwerke:
                SLONG xx = x;
                PrimaryBm.BlitFromT(gUniversalPlaneBms[32],
                                    x - 15 + gUniversalPlaneBms[9 + KlackerPlanes[c].Logo * 5 + 1].Size.x * KlackerPlanes[c].Size / 2 - 32,
                                    KlackerPlanes[c].ScreenPos.y - 20 - 25 - 74);

                // Heck:
                PrimaryBm.BlitFromT(gUniversalPlaneBms[9 + 4], x, KlackerPlanes[c].ScreenPos.y - gUniversalPlaneBms[9 + 4].Size.y + 19 - 50);
                PrimaryBm.BlitFromT(gUniversalPlaneBms[9 + KlackerPlanes[c].Logo * 5 + 4], x,
                                    KlackerPlanes[c].ScreenPos.y - gUniversalPlaneBms[9 + 4].Size.y + 19 - 50);
                x += gUniversalPlaneBms[9 + KlackerPlanes[c].Logo * 5 + 4].Size.x;

                // Körper:
                for (SLONG d = 0; d < KlackerPlanes[c].Size; d++) {
                    PrimaryBm.BlitFromT(gUniversalPlaneBms[31], x, KlackerPlanes[c].ScreenPos.y - gUniversalPlaneBms[9 + 1].Size.y);
                    if (KlackerPlanes[c].Logo != 0) {
                        PrimaryBm.BlitFromT(gUniversalPlaneBms[9 + KlackerPlanes[c].Logo * 5 + 1], x,
                                            KlackerPlanes[c].ScreenPos.y - gUniversalPlaneBms[9 + 1].Size.y);
                    }
                    x += gUniversalPlaneBms[9 + KlackerPlanes[c].Logo * 5 + 1].Size.x;
                }

                // Bug:
                PrimaryBm.BlitFromT(gUniversalPlaneBms[9 + 3], x, KlackerPlanes[c].ScreenPos.y - gUniversalPlaneBms[9 + 3].Size.y);
                PrimaryBm.BlitFromT(gUniversalPlaneBms[9 + KlackerPlanes[c].Logo * 5 + 3], x, KlackerPlanes[c].ScreenPos.y - gUniversalPlaneBms[9 + 3].Size.y);

                PrimaryBm.BlitFromT(gUniversalPlaneBms[5], xx + gUniversalPlaneBms[9 + KlackerPlanes[c].Logo * 5 + 1].Size.x * KlackerPlanes[c].Size / 2 - 20,
                                    KlackerPlanes[c].ScreenPos.y - 20 - 26);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Blittet die Flugzeuge auf den Screen
//--------------------------------------------------------------------------------------------
void CKlackerPlanes::PostPaint(SBPRIMARYBM & /*PrimaryBm*/) {}
