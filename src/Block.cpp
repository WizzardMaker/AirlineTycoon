//============================================================================================
// Block.cpp : Routinen für den Notizblock der Spieler
//============================================================================================
#include "StdAfx.h"

#include <climits>

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

extern SB_CColorFX ColorFX;

// Daten des aktuellen Savegames beim laden:
extern SLONG SaveVersion;
extern SLONG SaveVersionSub;

extern SB_CFont FontVerySmall;

static __int64 AufTausenderRunden(__int64 val) {
    if (val >= 0) {
        return (val + 500) / 1000;
    }
    return (val - 500) / 1000;
}

static char *CalcMillionen(__int64 val) {
    static char buffer[20];
    snprintf(buffer, sizeof(buffer), "%.1f", DOUBLE(val) / 1e6);
    return buffer;
}

static char *CalcPercentage(__int64 val, __int64 div) {
    static char buffer[20];
    if (div == 0) {
        strncpy(buffer, "---", sizeof(buffer));
    } else {
        snprintf(buffer, sizeof(buffer), "%.1f %%", DOUBLE(val) * 100.0 / DOUBLE(div));
    }
    return buffer;
}

static bool GetColor(SLONG SelectedId, SLONG PlayerNum) {
    if (SelectedId >= 1 && SelectedId <= 5 && Sim.Players.Players[PlayerNum].HasBerater(BERATERTYP_GELD) == 0) {
        return true;
    }
    if (SelectedId >= 6 && SelectedId <= 10 && Sim.Players.Players[PlayerNum].HasBerater(BERATERTYP_INFO) == 0) {
        return true;
    }
    return false;
}

//--------------------------------------------------------------------------------------------
// Blittet den Block an eine bestimmte Stelle:
//--------------------------------------------------------------------------------------------
void BLOCK::BlitAt(SBBM &RoomBm) {
    if (StyleType == 0) {
        RoomBm.BlitFromT(Bitmap, ScreenPos);
        return;
    }

    auto *pLaptop = dynamic_cast<CLaptop *>(Base);

    if (Sim.TimeSlice < AnimationStart) {
        // Fix frozen windows that occasionally got stuck in closing animation
        AnimationStart = Sim.TimeSlice;
    }
    SLONG Phase = (Sim.TimeSlice - AnimationStart) * 2 - 7;

    if (Destructing == 1) {
        Phase = 38 - Phase;
        if (Phase <= -7) {
            Destructing = 2;
            return;
        }
    }

    if (Destructing == 2) {
        return;
    }

    if (Phase < 0) {
        RoomBm.BlitFromT(pLaptop->EckBms[0], ScreenPos.x + 178 / (2 - DoubleBlock), ScreenPos.y);
        RoomBm.BlitFromT(pLaptop->EckBms[1], ScreenPos.x + 178 / (2 - DoubleBlock) + 5, ScreenPos.y);
        return;
    }

    if (Bitmap.pBitmap == nullptr) { // safety net to prevent crashes
        return;
    }

    if (DoubleBlock != 0) {
        if (Phase < 38) {
            SLONG p = min(Phase, 9);

            // 0..10
            CRect SrcRect(198 - p * 198 / 9, 0, 198 + p * 198 / 9, 15);
            CRect DestRect = SrcRect + CPoint(ScreenPos);

            RoomBm.BlitPartFromT(Bitmap, SrcRect.left + ScreenPos.x, SrcRect.top + ScreenPos.y, SrcRect.left, SrcRect.top, SrcRect.right, SrcRect.bottom);

            if (Phase >= 10) {
                SLONG p = min(Phase, 19) - 10;

                SrcRect = CRect(0, 0, 13, p * (Bitmap.Size.y - 19) / 9);
                DestRect = SrcRect + CPoint(ScreenPos);

                RoomBm.BlitPartFromT(Bitmap, SrcRect.left + ScreenPos.x, SrcRect.top + ScreenPos.y, SrcRect.left, SrcRect.top, SrcRect.right, SrcRect.bottom);

                SrcRect = CRect(383, 0, 396, p * (Bitmap.Size.y - 19) / 9);
                DestRect = SrcRect + CPoint(ScreenPos);

                RoomBm.BlitPartFromT(Bitmap, SrcRect.left + ScreenPos.x, SrcRect.top + ScreenPos.y, SrcRect.left, SrcRect.top, SrcRect.right, SrcRect.bottom);
            }

            if (Phase >= 20) {
                SLONG p = min(Phase, 29) - 20;

                SrcRect = CRect(0, 197, p * 198 / 18, 210);
                DestRect = SrcRect + CPoint(ScreenPos);

                RoomBm.BlitPartFromT(Bitmap, SrcRect.left + ScreenPos.x, SrcRect.top + ScreenPos.y, SrcRect.left, SrcRect.top, SrcRect.right, SrcRect.bottom);

                SrcRect = CRect(198 - p * 198 / 18, 197, 178, 210);
                DestRect = SrcRect + CPoint(ScreenPos);

                RoomBm.BlitPartFromT(Bitmap, SrcRect.left + ScreenPos.x, SrcRect.top + ScreenPos.y, SrcRect.left, SrcRect.top, SrcRect.right, SrcRect.bottom);

                SrcRect = CRect(198 - p * 198 / 18, 197, 198 + p * 198 / 18, 210);
                DestRect = SrcRect + CPoint(ScreenPos);

                RoomBm.BlitPartFromT(Bitmap, SrcRect.left + ScreenPos.x, SrcRect.top + ScreenPos.y, SrcRect.left, SrcRect.top, SrcRect.right, SrcRect.bottom);
            }
        }

        if (Phase < 10) {
            RoomBm.BlitFromT(pLaptop->EckBms[0], 178 - Phase * 178 / 9 + ScreenPos.x, ScreenPos.y);
            RoomBm.BlitFromT(pLaptop->EckBms[1], 188 + Phase * 188 / 9 + ScreenPos.x, ScreenPos.y);
        } else if (Phase < 20) {
            RoomBm.BlitFromT(pLaptop->EckBms[2], ScreenPos.x, ScreenPos.y + (Phase - 10) * (Bitmap.Size.y - 16) / 9);
            RoomBm.BlitFromT(pLaptop->EckBms[3], Bitmap.Size.x - 18 + ScreenPos.x, ScreenPos.y + (Phase - 10) * (Bitmap.Size.y - 16) / 9);
        } else if (Phase < 30) {
            RoomBm.BlitFromT(pLaptop->EckBms[2], ScreenPos.x + (Phase - 20) * 82 / 18, ScreenPos.y + Bitmap.Size.y - gNotepadButtonL.Size.y - 2);
            RoomBm.BlitFromT(pLaptop->EckBms[3], 377 + ScreenPos.x - (Phase - 20) * 83 / 18, ScreenPos.y + Bitmap.Size.y - gNotepadButtonL.Size.y - 2);
            RoomBm.BlitFromT(pLaptop->EckBms[3], 178 - (Phase - 20) * 73 / 18 + ScreenPos.x, ScreenPos.y + Bitmap.Size.y - gNotepadButtonL.Size.y - 2);
            RoomBm.BlitFromT(pLaptop->EckBms[2], 188 + (Phase - 20) * 81 / 18 + ScreenPos.x, ScreenPos.y + Bitmap.Size.y - gNotepadButtonL.Size.y - 2);
        }
        if (Phase >= 22 && Phase < 38) {
            ColorFX.BlitTrans(Bitmap.pBitmap, RoomBm.pBitmap, ScreenPos, nullptr, (38 - Phase) / 2);
        }
        if (Phase >= 38) {
            RoomBm.BlitFromT(Bitmap, ScreenPos);
        }
    } else // Single-Block:
    {
        if (Phase < 38) {
            SLONG p = min(Phase, 9);

            // 0..10
            CRect SrcRect(100 - p * 100 / 9, 0, 100 + p * 99 / 9, 15);
            CRect DestRect = SrcRect + CPoint(ScreenPos);

            RoomBm.BlitPartFromT(Bitmap, SrcRect.left + ScreenPos.x, SrcRect.top + ScreenPos.y, SrcRect.left, SrcRect.top, SrcRect.right, SrcRect.bottom);

            if (Phase >= 10) {
                SLONG p = min(Phase, 19) - 10;

                SrcRect = CRect(0, 0, 13, p * (Bitmap.Size.y - 13) / 9);
                DestRect = SrcRect + CPoint(ScreenPos);

                RoomBm.BlitPartFromT(Bitmap, SrcRect.left + ScreenPos.x, SrcRect.top + ScreenPos.y, SrcRect.left, SrcRect.top, SrcRect.right, SrcRect.bottom);

                SrcRect = CRect(196, 0, 199, p * (Bitmap.Size.y - 13) / 9);
                DestRect = SrcRect + CPoint(ScreenPos);

                RoomBm.BlitPartFromT(Bitmap, SrcRect.left + ScreenPos.x, SrcRect.top + ScreenPos.y, SrcRect.left, SrcRect.top, SrcRect.right, SrcRect.bottom);
            }

            if (Phase >= 20) {
                SLONG p = min(Phase, 29) - 20;

                SrcRect = CRect(0, 197, p * 99 / 9, 210);
                DestRect = SrcRect + CPoint(ScreenPos);

                RoomBm.BlitPartFromT(Bitmap, SrcRect.left + ScreenPos.x, SrcRect.top + ScreenPos.y, SrcRect.left, SrcRect.top, SrcRect.right, SrcRect.bottom);

                SrcRect = CRect(199 - p * 100 / 9, 197, 199, 210);
                DestRect = SrcRect + CPoint(ScreenPos);

                RoomBm.BlitPartFromT(Bitmap, SrcRect.left + ScreenPos.x, SrcRect.top + ScreenPos.y, SrcRect.left, SrcRect.top, SrcRect.right, SrcRect.bottom);
            }
        }

        if (Phase < 10) {
            RoomBm.BlitFromT(pLaptop->EckBms[0], 80 - Phase * 80 / 9 + ScreenPos.x, ScreenPos.y);
            RoomBm.BlitFromT(pLaptop->EckBms[1], 90 + Phase * 89 / 9 + ScreenPos.x, ScreenPos.y);
        } else if (Phase < 20) {
            RoomBm.BlitFromT(pLaptop->EckBms[2], ScreenPos.x, ScreenPos.y + (Phase - 10) * (Bitmap.Size.y - 19) / 9);
            RoomBm.BlitFromT(pLaptop->EckBms[3], Bitmap.Size.x - 20 + ScreenPos.x, ScreenPos.y + (Phase - 10) * (Bitmap.Size.y - 19) / 9);
        } else if (Phase < 30) {
            RoomBm.BlitFromT(pLaptop->EckBms[2], ScreenPos.x + (Phase - 20) * 90 / 9, ScreenPos.y + Bitmap.Size.y - pLaptop->EckBms[2].Size.y);
            RoomBm.BlitFromT(pLaptop->EckBms[3], 179 + ScreenPos.x - (Phase - 20) * 89 / 9, ScreenPos.y + Bitmap.Size.y - pLaptop->EckBms[2].Size.y);
        }
        if (Phase >= 22 && Phase < 38) {
            ColorFX.BlitTrans(Bitmap.pBitmap, RoomBm.pBitmap, ScreenPos, nullptr, (38 - Phase) / 2);
        }
        if (Phase >= 38) {
            RoomBm.BlitFromT(Bitmap, ScreenPos);
        }
    }
}

//--------------------------------------------------------------------------------------------
// Eine Lib für den Block laden:
//--------------------------------------------------------------------------------------------
void BLOCK::LoadCityPhotoLib(CITY &city) {
    if ((pGLibPicture != nullptr) && (pGfxMain != nullptr)) {
        Bitmap.Destroy();
        pGfxMain->ReleaseLib(pGLibPicture);
        pGLibPicture = nullptr;
    }

    if (city.PhotoName.GetLength() > 0) {
        try {
            if (StyleType == 0) { // Globus/Filofax
                pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename(city.PhotoName + ".gli", GliPath)), &pGLibPicture, L_LOCMEM);
            } else { // Laptop
                pGfxMain->LoadLib(const_cast<char *>((LPCTSTR)FullFilename(city.PhotoName + ".glj", GliPath)), &pGLibPicture, L_LOCMEM);
            }
        } catch (TeakLibException &e) {
            // Just show no image and remove photo pages
            e.caught();
            city.AnzPhotos = 0;
        }
    }
}

//--------------------------------------------------------------------------------------------
// Einen ToolTip im Fenster anfordern
//--------------------------------------------------------------------------------------------
void BLOCK::SetTip(SLONG TipTypeA, SLONG TipTypeB, SLONG TipId, SLONG TipPar1, SLONG TipPar2, SLONG TipPar3, SLONG TipPar4, SLONG TipPar5, SLONG TipPar6,
                   SLONG TipPar7) {
    Tip = TipTypeA;
    TipB = TipTypeB;

    CurrentTipId = TipId;
    CurrentTipIdPar1 = TipPar1;
    CurrentTipIdPar2 = TipPar2;
    CurrentTipIdPar3 = TipPar3;
    CurrentTipIdPar4 = TipPar4;
    CurrentTipIdPar5 = TipPar5;
    CurrentTipIdPar6 = TipPar6;
    CurrentTipIdPar7 = TipPar7;
}

//--------------------------------------------------------------------------------------------
// Den Block wegen einem Tip neu zeichen?
//--------------------------------------------------------------------------------------------
void BLOCK::UpdateTip(SLONG PlayerNum, BOOL StyleType) {
    if (Tip != TipInUse || TipB != TipInUseB ||
        (((Tip != 0) || (TipB != 0)) &&
         (CurrentTipId != LastTipId || CurrentTipIdPar1 != LastTipIdPar1 || CurrentTipIdPar2 != LastTipIdPar2 || CurrentTipIdPar3 != LastTipIdPar3 ||
          CurrentTipIdPar4 != LastTipIdPar4 || CurrentTipIdPar5 != LastTipIdPar5 || CurrentTipIdPar6 != LastTipIdPar6))) {
        TipInUse = Tip;
        TipInUseB = TipB;

        LastTipId = CurrentTipId;
        LastTipIdPar1 = CurrentTipIdPar1;
        LastTipIdPar2 = CurrentTipIdPar2;
        LastTipIdPar3 = CurrentTipIdPar3;
        LastTipIdPar4 = CurrentTipIdPar4;
        LastTipIdPar5 = CurrentTipIdPar5;
        LastTipIdPar6 = CurrentTipIdPar6;
        LastTipIdPar7 = CurrentTipIdPar7;

        Refresh(PlayerNum, StyleType);
    }
}

//--------------------------------------------------------------------------------------------
// Den Block neu malen:
//--------------------------------------------------------------------------------------------
void BLOCK::Refresh(SLONG PlayerNum, BOOL StyleType) {
    XY TitleArea;   // Hier beginnt der Platz für den Fenstertitel
    XY ClientArea;  // Hier beginnt das Papier
    XY PageArea;    // Hier steht die aktuelle Seite Papier
    XY TitleAreaB;  // Hier beginnt der Platz für den Fenstertitel
    XY ClientAreaB; // Hier beginnt das Papier
    XY PageAreaB;   // Hier steht die aktuelle Seite Papier
    PLAYER &qPlayer = Sim.Players.Players[PlayerNum];

    if (Base == nullptr) {
        return;
    }

    BLOCK::StyleType = StyleType;
    UpdateTip(PlayerNum, StyleType);

    SB_CFont &TitleFont = StyleType != 0 ? FontNormalGreen : FontSmallBlack;

    if (TipInUse == TIP_NONE) {
        // Specials für die Info-Seiten, wenn man keinen Berater hat:
        if (Index == 0 && BlockType == 5) {
            if (SelectedId == 0) {
                AnzPages = 1;
                Page = 0;
            }

            // Tagesbilanz, Wochenbilanz, Gesamtbilanz, Wocheneinnamen der Flugzeuge, Geschäftsbericht
            if ((SelectedId >= 1 && SelectedId <= 5) && (Sim.Players.Players[PlayerNum].HasBerater(BERATERTYP_GELD) == 0)) {
                AnzPages = 1;
                Page = 0;
            }

            // Konkurrenz:
            if ((SelectedId >= 6 && SelectedId <= 10) && (Sim.Players.Players[PlayerNum].HasBerater(BERATERTYP_INFO) == 0)) {
                AnzPages = 1;
                Page = 0;
            }
        }
    }

    if (StyleType == 0) // Globus/Filofax
    {
        auto *pGlobe = dynamic_cast<CGlobe *>(Base);

        TitleArea = XY(48, 57);
        ClientArea = XY(48, 72);
        PageArea = XY(130 + 7, 242);
        TitleAreaB = XY(232 + 48, 57);
        ClientAreaB = XY(232 + 48, 72);
        PageAreaB = XY(232 + 130 + 7, 242);

        // Window zeichnen
        Bitmap.ReSize(pGlobe->Filofax.Size);
        Bitmap.BlitFrom(pGlobe->Filofax);

        // Linker Block: Back/Next/Index/Inhalt
        if (BlockType >= 1 && BlockType <= 5) {
            if (Page > 0) {
                Bitmap.BlitFromT(pGlobe->Back, 40, 237); // Back
            }
            if (Page / PageSize < AnzPages - 1) {
                Bitmap.BlitFromT(pGlobe->Next, 208, 233); // Next
            }

            if (BlockType == 2) {
                Bitmap.BlitFromT(pGlobe->Index1[1], 36, 256); // Plane
            }
            if (BlockType == 1) {
                Bitmap.BlitFromT(pGlobe->Index1[2], 36, 256); // City
            }
            if (BlockType == 5) {
                Bitmap.BlitFromT(pGlobe->Index1[3], 36, 256); // Info
            }

            if (Index != 1) {
                Bitmap.BlitFromT(pGlobe->Inhalt, 0, 178 - 100);
            }

            Bitmap.BlitFromT(pGlobe->FiloTops[0], 32, 2 + 3);
        } else {
            Bitmap.BlitFromT(pGlobe->Index1[0], 36, 256);
        }

        if (BlockType == 2 && (DoubleBlock != 0) && BlockTypeB >= 1 && BlockTypeB <= 6) {
            if (PageB > 0) {
                Bitmap.BlitFromT(pGlobe->Back, 232 + 40, 237); // Back
            }
            if (PageB / PageSizeB < AnzPagesB - 1) {
                Bitmap.BlitFromT(pGlobe->Next, 232 + 208, 233); // Next
            }

            if (BlockTypeB == 3) {
                Bitmap.BlitFromT(pGlobe->IndexA[0], 268, 256); // Aufträge
            }
            if (BlockTypeB == 4) {
                Bitmap.BlitFromT(pGlobe->IndexA[2], 268, 256); // Routen
            }
            if (BlockTypeB == 6) {
                Bitmap.BlitFromT(pGlobe->IndexA[1], 268, 256); // Fracht
            }

            // Schloss bei Routen:
            if (BlockTypeB == 4 && ((qPlayer.SecurityFlags & (1 << 4)) != 0U)) {
                Bitmap.BlitFrom(pGlobe->LockBm, 441, 55);
            }

            if (IndexB != 1) {
                Bitmap.BlitFromT(pGlobe->Inhalt, 232 + 0, 178 - 100);
            }

            Bitmap.BlitFromT(pGlobe->FiloTops[1], 232 + 32 - 3, 2 - 2);
        }

        SB_Hardwarecolor HardwareColor = Bitmap.pBitmap->GetHardwarecolor(0x303030);
        Bitmap.Line(TitleArea + XY(-1, 10), TitleArea + XY(170, 10), FALSE, &HardwareColor, 1);
        Bitmap.Line(TitleAreaB + XY(-1, 10), TitleAreaB + XY(170, 10), FALSE, &HardwareColor, 1);
    } else // Laptop
    {
        auto *pLaptop = dynamic_cast<CLaptop *>(Base);

        TitleArea = XY(41, 7);
        ClientArea = XY(20, 21) + XY(1, 1);
        PageArea = XY(104, 195);
        TitleAreaB = XY(187 + 41, 7);
        ClientAreaB = XY(187 + 20, 21) + XY(-1, 1);
        PageAreaB = XY(291, 195);

        // Window zeichnen
        if (DoubleBlock != 0) {
            Bitmap.ReSize(pLaptop->BigWin.Size);
            Bitmap.BlitFrom(pLaptop->BigWin);
        } else {
            Bitmap.ReSize(pLaptop->Window.Size);
            Bitmap.BlitFrom(pLaptop->Window);
        }

        if (BlockType >= 1 && BlockType <= 6) {
            if (BlockType != 1 || Page - 1 < Cities[SelectedId].AnzTexts || Index == 1) {
                if (TipInUse != TIP_NONE) {
                    Bitmap.BlitFrom(pLaptop->Wallpapers[BlockTypeB - 1], 20, 20);
                } else {
                    // Kein Hintergrund bei Flugplan; das sieht doof aus
                    if ((BlockType != 2 || Page != 0 || Index == 1) && (BlockType != 5 || Index != 0 || SelectedId != 0)) {
                        Bitmap.BlitFrom(pLaptop->Wallpapers[BlockType - 1], 20, 20);
                    }
                }
            }
        }

        if (Index != 1) { // Inhalt Ikone:
            Bitmap.BlitFromT(pLaptop->Inhalt[0], 16, 0);
        }

        // Leere Titelzeile:
        if (BlockType == 1 || (TipInUseB != 0) || (BlockType == 2 && (DoubleBlock != 0) && BlockTypeB >= 1 && BlockTypeB <= 6 && IndexB != 1)) {
            Bitmap.BlitFromT(pLaptop->Switch[6], XY(217, 5));
            Bitmap.BlitFromT(pLaptop->Switch[6], XY(237, 5));
            Bitmap.BlitFromT(pLaptop->Switch[6], XY(257, 5));
            Bitmap.BlitFromT(pLaptop->Switch[6], XY(277, 5));
        }

        if (BlockType == 2 && (DoubleBlock != 0) && BlockTypeB >= 1 && BlockTypeB <= 6 && IndexB != 1) { // Inhalt Ikone rechts
            Bitmap.BlitFromT(pLaptop->Inhalt[0], 187 + 16, 0);
        }

        // Wallpaper:
        if ((DoubleBlock != 0) && BlockTypeB >= 1 && BlockTypeB <= 6 && TipInUseB != TIP_CITY) {
            if (TipInUseB != TIP_NONE) {
                Bitmap.BlitFrom(pLaptop->Wallpapers[BlockType - 1], 205, 20);
            } else if (BlockType == 2) {
                Bitmap.BlitFrom(pLaptop->Wallpapers[BlockTypeB - 1], 205, 20);
            }
        }

        // Schloss bei Routen:
        if (BlockTypeB == 4 && IndexB == 1 && ((qPlayer.SecurityFlags & (1 << 4)) != 0U)) {
            Bitmap.BlitFrom(pLaptop->LockBm, 156 + 185, 4 + 2);
        }

        // Scrollbar:
        if (AnzPages > 1) {
            Bitmap.BlitFromT(pLaptop->ScrollBms[0], 5, 43 + (138 - 43) * Page / PageSize / (AnzPages - 1));
        }
        if (AnzPagesB > 1) {
            Bitmap.BlitFromT(pLaptop->ScrollBms[1], 376, 43 + (138 - 43) * PageB / PageSizeB / (AnzPagesB - 1));
        }
    }

    if (TipInUse == TIP_NONE) {
        // Seitenangegabe:
        if (BlockType != 0) {
            Bitmap.PrintAt(bprintf("%li/%li", Page / PageSize + 1, AnzPages), TitleFont, TEC_FONT_CENTERED, PageArea - XY(30, 0), PageArea + XY(30, 20));
        }

        // Die Linke Seite:
        if (Index == 1) {
            LinkeSeiteIndex(TitleArea, ClientArea);
        } else if (Index == 0) {
            LinkeSeiteInhalt(TitleArea, ClientArea);
        }
    }

switch_again:
    if (TipInUseB == TIP_NONE) {
        // Seitenangegabe rechts:
        if ((BlockTypeB != 0) && (DoubleBlock != 0) && BlockType == 2) {
            Bitmap.PrintAt(bprintf("%li/%li", PageB / PageSizeB + 1, AnzPagesB), TitleFont, TEC_FONT_CENTERED, PageAreaB - XY(30, 0), PageAreaB + XY(30, 20));
        }

        // Die rechte Seite:
        if (IndexB == 1 && (DoubleBlock != 0) && BlockType == 2) {
            RechteSeiteIndex(TitleAreaB, ClientAreaB);
        } else if (IndexB == 0 && (DoubleBlock != 0) && BlockType == 2) {
            if (RechteSeiteInhalt(TitleAreaB, ClientAreaB)) {
                goto switch_again;
            }
        }
    } else if (TipInUseB == TIP_CITY) {
        if (StyleType == 1) // Laptop
        {
            Bitmap.BlitFrom((dynamic_cast<CLaptop *>(Base))->Karte, ClientAreaB + XY(-4, -2));
            if (CurrentTipId != 0xffffffff) {
                DrawCityTipContents(Bitmap, CurrentTipId, TitleAreaB, ClientAreaB + XY(0, -15), ClientAreaB + XY(-18, -20), &TitleFont, &FontSmallBlack);
            }
        } else {
            Bitmap.BlitFrom((dynamic_cast<CGlobe *>(Base))->Karte, ClientAreaB + XY(-9, -11));
            if (CurrentTipId != 0xffffffff) {
                DrawCityTipContents(Bitmap, CurrentTipId, TitleAreaB, ClientAreaB + XY(0, -15), ClientAreaB + XY(-9, -11), &TitleFont, &FontSmallBlack);
            }
        }
    } else if (TipInUseB == TIP_PLANE) {
        // DrawPlaneTipContents (Bitmap, &PlaneTypes[qPlayer.Planes[CurrentTipId].TypeId], &qPlayer.Planes[CurrentTipId], TitleAreaB, ClientAreaB, &TitleFont,
        // &FontSmallBlack, TRUE);
        DrawPlaneTipContents(Bitmap, nullptr, &qPlayer.Planes[CurrentTipId], TitleAreaB, ClientAreaB, &TitleFont, &FontSmallBlack, TRUE);
    } else if (TipInUseB == TIP_AUFTRAG) {
        DrawAuftragTipContents(-1, Bitmap, nullptr, &Sim.Players.Players[PlayerNum].Auftraege[CurrentTipId],
                               CurrentTipIdPar3, // Gate
                               CurrentTipIdPar1, CurrentTipIdPar2, TitleAreaB, ClientAreaB, XY(0, 0), &TitleFont, &FontSmallBlack, CurrentTipIdPar4);
    } else if (TipInUseB == TIP_FRACHT) {
        DrawFrachtTipContents(-1, Bitmap, nullptr, &Sim.Players.Players[PlayerNum].Frachten[CurrentTipId],
                              CurrentTipIdPar3, // t mit diesem Flug transportiert
                              CurrentTipIdPar1, // Costs
                              CurrentTipIdPar5, // Income
                              CurrentTipIdPar2, // Okay
                              TitleAreaB, ClientAreaB, XY(0, 0), &TitleFont, &FontSmallBlack,
                              CurrentTipIdPar4); // Unlocked
    } else if (TipInUseB == TIP_ROUTE) {
        DrawRouteTipContents(Bitmap, PlayerNum, CurrentTipId, CurrentTipIdPar3, CurrentTipIdPar1, CurrentTipIdPar6, CurrentTipIdPar2, CurrentTipIdPar4,
                             CurrentTipIdPar7, TitleAreaB, ClientAreaB, &TitleFont, &FontSmallBlack, CurrentTipIdPar5);
    } else if (TipInUseB == TIP_AUTOFLUG) {
        DrawAutoflugTipContents(Bitmap, CurrentTipId, CurrentTipIdPar1, CurrentTipIdPar2, CurrentTipIdPar3, TitleAreaB, ClientAreaB, &TitleFont,
                                &FontSmallBlack, CurrentTipIdPar4);
    }
}

void BLOCK::LinkeSeiteIndex(XY TitleArea, XY ClientArea) {
    SB_CFont &TitleFont = StyleType != 0 ? FontNormalGreen : FontSmallBlack;
    SLONG c = 0;

    // Inhaltsverzechnis
    switch (BlockType) {
    // Cities:
    case 1:
        Bitmap.PrintAt(StandardTexte.GetS(TOKEN_SCHED, 1400), TitleFont, TEC_FONT_LEFT, TitleArea, Bitmap.Size);

        for (c = Page; c < Page + PageSize && c < Table.AnzRows; c++) {
            if (Table.ValueFlags[0 + c * Table.AnzColums] != 0U) {
                Bitmap.PrintAt(Table.Values[0 + c * Table.AnzColums], FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(0, (c - Page) * 13), Bitmap.Size);
            } else {
                Bitmap.PrintAt(Table.Values[0 + c * Table.AnzColums], FontSmallGrey, TEC_FONT_LEFT, ClientArea + XY(0, (c - Page) * 13), Bitmap.Size);
            }
        }

        break;

        // Planes:
    case 2:
        Bitmap.PrintAt(StandardTexte.GetS(TOKEN_SCHED, 1000), TitleFont, TEC_FONT_LEFT, TitleArea, Bitmap.Size);

        for (c = Page; c < Page + PageSize && c < Table.AnzRows; c++) {
            SB_CFont *f = nullptr;

            if (Table.ValueFlags[0 + c * Table.AnzColums] != 0U) {
                f = &FontSmallRed;
            } else {
                f = &FontSmallBlack;
            }

            Bitmap.PrintAt(Table.Values[0 + c * Table.AnzColums], *f, TEC_FONT_LEFT, ClientArea + XY(0, (c - Page) * 26), Bitmap.Size);
            Bitmap.PrintAt(Table.Values[1 + c * Table.AnzColums], *f, TEC_FONT_RIGHT, ClientArea + XY(145, (c - Page) * 26),
                           ClientArea + XY(175, (c - Page) * 26 + 10));

            Bitmap.PrintAt(Table.Values[3 + c * Table.AnzColums], *f, TEC_FONT_LEFT, ClientArea + XY(0, (c - Page) * 26 + 10), Bitmap.Size);

            // Add additional information to plane list
            PLAYER &qPlayer = Sim.Players.Players[PlayerNum];
            for (SLONG i = 0; i < (SLONG)qPlayer.Planes.AnzEntries(); i++) {
                if (qPlayer.Planes.IsInAlbum(i)) {
                    CPlane &qPlane = qPlayer.Planes[i];

                    // check if plane name is correct, some are mixed up
                    if (qPlane.Name == Table.Values[0 + c * Table.AnzColums]) {

                        XY left = ClientArea + XY(110, (c - Page) * 26);
                        XY right = ClientArea + XY(143, (c - Page) * 26 + 10);

                        if (BlockTypeB == 6) {
                            // Freight
                            Bitmap.PrintAt(Einheiten[EINH_T].bString(qPlane.ptPassagiere / 10), FontSmallGrey, TEC_FONT_RIGHT, left, right);
                        } else if (BlockTypeB == 4) { // Routes
                            Bitmap.PrintAt(bprintf("(%li/%li)", qPlane.MaxPassagiere, qPlane.MaxPassagiereFC), FontSmallGrey, TEC_FONT_RIGHT, left, right);
                        } else {
                            Bitmap.PrintAt(bprintf("(%li)", qPlane.MaxPassagiere + qPlane.MaxPassagiereFC), FontSmallGrey, TEC_FONT_RIGHT, left, right);
                        }
                        Bitmap.PrintAt(Einheiten[EINH_KM].bString(qPlane.ptReichweite), FontSmallGrey, TEC_FONT_LEFT,
                                       ClientArea + XY(123, (c - Page) * 26 + 10), Bitmap.Size);
                        break;
                    }
                }
            }
        }
        break;

        // Infos:
    case 5:
        Bitmap.PrintAt(StandardTexte.GetS(TOKEN_SCHED, 1500), TitleFont, TEC_FONT_LEFT, TitleArea, Bitmap.Size);

        for (c = Page; c < Page + PageSize && c < Table.AnzRows; c++) {
            Bitmap.PrintAt(Table.Values[0 + c * Table.AnzColums], GetColor(c, PlayerNum) ? FontSmallGrey : FontSmallBlack, TEC_FONT_LEFT,
                           ClientArea + XY(0, (c - Page) * 13), Bitmap.Size);
        }
        break;
    default:
        break;
    }
}

void BLOCK::LinkeSeiteInhalt(XY TitleArea, XY ClientArea) {
    SB_CFont &TitleFont = StyleType != 0 ? FontNormalGreen : FontSmallBlack;
    PLAYER &qPlayer = Sim.Players.Players[PlayerNum];
    SLONG c = 0;
    SLONG d = 0;
    SLONG e = 0;
    SLONG i = 0;

    // Inhalt eines speziellen Entities:
    switch (BlockType) {
    // Cities:
    case 1:
        Bitmap.PrintAt(Cities[SelectedId].Name, TitleFont, TEC_FONT_LEFT, TitleArea, Bitmap.Size);

        if (Page == 0) {
            CRentCity &qRentCity = qPlayer.RentCities.RentCities[Cities(SelectedId)];

            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_CITY, 1000), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(0, 1), Bitmap.Size);
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_CITY, 1001), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(0, 14), Bitmap.Size);
            Bitmap.PrintAt((LPCTSTR)Insert1000erDots(Cities[SelectedId].Einwohner), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(85, 1), Bitmap.Size);

            if (qRentCity.Rang == 0) {
                Bitmap.PrintAt(Einheiten[EINH_DM].bString(Cities[SelectedId].BuroRent), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(85, 14), Bitmap.Size);
            } else {
                Bitmap.PrintAt(Einheiten[EINH_DM].bString(qRentCity.Miete), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(85, 14), Bitmap.Size);
                Bitmap.PrintAt(StandardTexte.GetS(TOKEN_CITY, 1002), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(0, 27), Bitmap.Size);
                Bitmap.PrintAt(bprintf("%li", qRentCity.Rang), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(85, 27), Bitmap.Size);
                Bitmap.PrintAt(StandardTexte.GetS(TOKEN_CITY, 1003), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(0, 40), Bitmap.Size);
                Bitmap.PrintAt(bprintf("%li%%", static_cast<SLONG>(qRentCity.Image)), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(85, 40), Bitmap.Size);

                if (Cities.GetIdFromIndex(Cities(SelectedId)) != static_cast<ULONG>(Sim.HomeAirportId)) {
                    Bitmap.PrintAt(StandardTexte.GetS(TOKEN_CITY, 1010), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(0, 66), Bitmap.Size);
                }
            }
        } else if (Page - 1 < Cities[SelectedId].AnzTexts) {
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_CITY, Cities[SelectedId].TextRes + Page - 1), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(0, 1),
                           ClientArea + XY(172, 168));
        } else if (pGLibPicture != nullptr) {
            CString tmp = Cities[SelectedId].PhotoName;
            tmp.SetAt(tmp.GetLength() - 1, char(tmp[tmp.GetLength() - 1] + (Page - Cities[SelectedId].AnzTexts - 1)));

            SBBM Photo(pGLibPicture, StringToInt64(tmp));

            CRect rect(0, 0, 9999, 9999);
            if (Photo.Size.x != 0) {
                RemapColor(Photo.pBitmap, rect, 0x0000, 0x0001);
            }
            Bitmap.BlitFrom(Photo, ClientArea - XY(0, 3) + XY(85, 85) - Photo.Size / SLONG(2));
        }
        break;

        // Paint Planes Details:
    case 2: {
        SB_CFont *f = nullptr;
        CPlanes &qPlanes = qPlayer.Planes;

        // if (qPlanes[SelectedId].AnzBegleiter<PlaneTypes[qPlanes[SelectedId].TypeId].AnzBegleiter ||
        if (qPlanes[SelectedId].AnzBegleiter < qPlanes[SelectedId].ptAnzBegleiter || qPlanes[SelectedId].AnzPiloten < qPlanes[SelectedId].ptAnzPiloten) {
            // qPlanes[SelectedId].AnzPiloten<PlaneTypes[qPlanes[SelectedId].TypeId].AnzPiloten)
            f = &FontSmallRed;
        } else {
            f = &TitleFont;
        }

        Bitmap.PrintAt(qPlanes[SelectedId].Name, *f, TEC_FONT_LEFT, TitleArea, Bitmap.Size);
    }

        if (Page == 0) // Flugplan
        {
            CPlane &qPlane = qPlayer.Planes[SelectedId];
            CFlugplan &Plan = qPlane.Flugplan;

            if (Base->IsLaptop == FALSE) {
                Bitmap.BlitFromT((dynamic_cast<CGlobe *>(Base))->TimeTables[0], ClientArea + XY(21, -2));
            } else {
                Bitmap.BlitFromT(FlugplanBms[0], ClientArea + XY(21, -2));
            }

            if (Base->IsLaptop != 0) {
                Bitmap.BlitFromT(FlugplanBms[40], ClientArea + XY(24, 150));
            } else {
                Bitmap.BlitFromT(FlugplanBms[54], ClientArea + XY(22, 150));
            }

            for (c = 0; c < 7; c++) {
                Bitmap.PrintAt(StandardTexte.GetS(TOKEN_SCHED, 3000 + (c + Sim.Date + Sim.StartWeekday) % 7), FontSmallBlack, TEC_FONT_LEFT,
                               ClientArea + XY(2, 19 + 19 * c), ClientArea + XY(172, 170));
            }

            if (((AtGetAsyncKeyState(ATKEY_SHIFT) / 256) != 0) && ((AtGetAsyncKeyState(ATKEY_CONTROL) / 256) != 0)) {
                Plan.Dump();
            }

            // Flugplan blitten
            for (c = Plan.Flug.AnzEntries() - 1; c >= 0; c--) {
                if (Plan.Flug[c].ObjectType != 0) {
                    SLONG n = 0;

                    for (d = Plan.Flug[c].Startzeit, e = Plan.Flug[c].Startdate;;) {
                        SLONG px = d * 6;
                        SLONG py = 15 + (e - Sim.Date) * 19;

                        if (py >= 15) {
                            if (Plan.Flug[c].ObjectType == 1 || Plan.Flug[c].ObjectType == 2 || Plan.Flug[c].ObjectType == 4) {
                                if (d + 24 * e == Plan.Flug[c].Startzeit + 24 * Plan.Flug[c].Startdate) {
                                    i = 1;
                                } else if (d + 24 * e == Plan.Flug[c].Startzeit + 1 + 24 * Plan.Flug[c].Startdate) {
                                    i = 2;
                                } else if (d + 24 * e == Plan.Flug[c].Landezeit + 24 * Plan.Flug[c].Landedate) {
                                    i = 5;
                                } else if (d + 24 * e == Plan.Flug[c].Landezeit - 1 + 24 * Plan.Flug[c].Landedate) {
                                    i = 4;
                                } else {
                                    i = 3;
                                }

                                if (Plan.Flug[c].ObjectType == 2) {
                                    i += 5;
                                }
                                if (Plan.Flug[c].ObjectType == 4) {
                                    i += 19;
                                }
                            } else {
                                if (d + 24 * e == Plan.Flug[c].Startzeit + 24 * Plan.Flug[c].Startdate) {
                                    i = 11;
                                } else if (d + 24 * e == Plan.Flug[c].Landezeit + 24 * Plan.Flug[c].Landedate) {
                                    i = 15;
                                } else {
                                    i = 13;
                                }

                                if ((d & 1) != 0) {
                                    i++;
                                }
                            }

                            if (e >= Sim.Date && e <= Sim.Date + 6) {
                                Bitmap.BlitFromT(FlugplanBms[i], ClientArea + XY(24 + px, 2 + py));
                                if ((Plan.Flug[c].ObjectType == 2 || Plan.Flug[c].ObjectType == 4) && Plan.Flug[c].Okay > 0) {
                                    if (d == Plan.Flug[c].Startzeit) {
                                        i = 41;
                                    } else if (d == Plan.Flug[c].Landezeit) {
                                        i = 45;
                                    } else {
                                        i = 44;
                                    }
                                    Bitmap.BlitFromT(FlugplanBms[i], ClientArea + XY(24 + px, 2 + py));
                                }

                                if (d > 0 && d == (Plan.Flug[c].Startzeit + 1) % 24 &&
                                    !(Plan.Flug[c].Startdate > Sim.Date || Plan.Flug[c].Startzeit > Sim.GetHour() + 1)) {
                                    Bitmap.BlitFromT(FlugplanBms[17], ClientArea + XY(24 - 8 + px, py + 4));
                                }
                                if (Plan.Flug[c].VonCity == static_cast<ULONG>(Sim.HomeAirportId) && d == Plan.Flug[c].Startzeit &&
                                    (Plan.Flug[c].ObjectType == 1 || Plan.Flug[c].ObjectType == 2) && (Plan.Flug[c].GateWarning != 0U)) {
                                    Bitmap.BlitFromT(FlugplanBms[18], ClientArea + XY(24 + px, py + 4));
                                }
                                if (Plan.Flug[c].VonCity == static_cast<ULONG>(Sim.HomeAirportId) && d > 0 && d == (Plan.Flug[c].Startzeit + 1) % 24 &&
                                    (Plan.Flug[c].ObjectType == 1 || Plan.Flug[c].ObjectType == 2) && (Plan.Flug[c].GateWarning != 0U)) {
                                    Bitmap.BlitFromT(FlugplanBms[18], ClientArea + XY(24 - 6 + px, py + 4));
                                }
                                if (d == Plan.Flug[c].Landezeit && Plan.Flug[c].NachCity == static_cast<ULONG>(Sim.HomeAirportId) &&
                                    (Plan.Flug[c].ObjectType == 1 || Plan.Flug[c].ObjectType == 2) && (Plan.Flug[c].GateWarning != 0U)) {
                                    Bitmap.BlitFromT(FlugplanBms[18], ClientArea + XY(24 + px, py + 4));
                                }
                                if (d == Plan.Flug[c].Landezeit && Plan.Flug[c].ObjectType == 4 && (Plan.Flug[c].GateWarning != 0U)) {
                                    Bitmap.BlitFromT(FlugplanBms[18], ClientArea + XY(24 + px, py + 4));
                                }

                                /*if (d>0 && d==(Plan.Flug[c].Startzeit+1)%24 && Plan.Flug[c].ObjectType==2 && Plan.Flug[c].Okay>0)
                                  Bitmap.BlitFromT (FlugplanBms[19], ClientArea+XY(24-6+px,py+4));*/

                                if (d < 24 && Plan.Flug[c].ObjectType != 0) {
                                    SB_CFont *pFont = nullptr;
                                    XY OffsetA(0, 0);
                                    XY OffsetB(0, 0);

                                    if (Plan.Flug[c].Landezeit + Plan.Flug[c].Landedate * 24 - (Plan.Flug[c].Startzeit + Plan.Flug[c].Startdate * 24) < 4) {
                                        pFont = &FontVerySmall;
                                        OffsetA = XY(0, 2);
                                        OffsetB = XY(2, 0);
                                    } else {
                                        pFont = &FontSmallBlack;
                                    }

                                    if ((Plan.Flug[c].Landezeit + Plan.Flug[c].Landedate * 24) - (Plan.Flug[c].Startzeit + Plan.Flug[c].Startdate * 24) <= 6) {
                                        if ((d > 2 &&
                                             d + 24 * e == Plan.Flug[c].Startzeit + (3 - static_cast<SLONG>(OffsetB.x != 0)) + 24 * Plan.Flug[c].Startdate) ||
                                            ((OffsetB.x != 0) && Plan.Flug[c].Landezeit == d && Plan.Flug[c].Landedate <= e && px >= 12)) {
                                            Bitmap.PrintAt(Cities[Plan.Flug[c].VonCity].Kuerzel, *pFont, TEC_FONT_LEFT,
                                                           ClientArea + OffsetA + XY(max(26, 24 - 18 + 2 + px + static_cast<SLONG>(n == 2) * 6), 3 + py),
                                                           ClientArea + OffsetA + XY(24 + px + static_cast<SLONG>(n == 2) * 6 + 16 + 14, 2 + py + 18));
                                        }

                                        if ((d > 2 && d + 24 * e == Plan.Flug[c].Landezeit + 24 * Plan.Flug[c].Landedate) ||
                                            (px == 12 && (OffsetB.x != 0) && d + 24 * e == Plan.Flug[c].Landezeit + 24 * Plan.Flug[c].Landedate)) {
                                            Bitmap.PrintAt(Cities[Plan.Flug[c].NachCity].Kuerzel, *pFont, TEC_FONT_RIGHT,
                                                           ClientArea + XY(24 - 18 - 14 + px, 4 + py + 8) + OffsetB,
                                                           ClientArea + XY(px + 24 + 8 - 1, py + 8 + 15) + OffsetB);
                                        }
                                    } else {
                                        if ((d > 2 && d + 24 * e == Plan.Flug[c].Startzeit + 4 + 24 * Plan.Flug[c].Startdate) ||
                                            ((OffsetB.x != 0) && Plan.Flug[c].Landezeit == d && Plan.Flug[c].Landedate <= e)) {
                                            Bitmap.PrintAt(Cities[Plan.Flug[c].VonCity].Kuerzel, *pFont, TEC_FONT_LEFT,
                                                           ClientArea + OffsetA + XY(24 - 18 + px + static_cast<SLONG>(n == 2) * 6, 3 + py),
                                                           ClientArea + OffsetA + XY(24 + px + static_cast<SLONG>(n == 2) * 6 + 16 + 14, 2 + py + 18));
                                        }

                                        if ((d > 2 && d + 24 * e == Plan.Flug[c].Landezeit - 1 + 24 * Plan.Flug[c].Landedate) ||
                                            (px == 18 && (OffsetB.x != 0))) {
                                            Bitmap.PrintAt(Cities[Plan.Flug[c].NachCity].Kuerzel, *pFont, TEC_FONT_RIGHT,
                                                           ClientArea + XY(24 - 18 - 14 + px, 4 + py + 8) + OffsetB,
                                                           ClientArea + XY(px + 24 + 8, py + 8 + 15) + OffsetB);
                                        }
                                    }
                                }
                            }
                        }

                        n++;
                        if (Plan.Flug[c].Landezeit == d && Plan.Flug[c].Landedate <= e) {
                            break;
                        }
                        d++;
                        if (d >= 24) {
                            d -= 24;
                            e++;
                        }
                    }
                }
            }

            // Dunklen Bereich markieren:
            if (Bitmap.Size.x > 0) {
                Bitmap.pBitmap->SetClipRect(CRect(ClientArea.x + 24, ClientArea.y + 17, ClientArea.x + 24 + Sim.Time / 10000, ClientArea.y + 17 + 19));
                ColorFX.Apply(4, Bitmap.pBitmap);
                Bitmap.pBitmap->SetClipRect(CRect(0, 0, Bitmap.Size.x, Bitmap.Size.y));
            }

            // Problem markieren:
            if (qPlane.Problem != 0) {
                SLONG Problem = qPlane.Problem;
                SLONG offset = Sim.Time / 10000;
                SLONG day = 0;

                while ((Problem != 0) && (min(Problem * 6, 24 * 6 - offset) - 1) > 0) {
                    SBBM RedBm(min(Problem * 6, 24 * 6 - offset) - 1, 19);
                    RedBm.FillWith((UWORD)RedBm.pBitmap->GetHardwarecolor(0xff0000));

                    CRect rect(0, 0, min(Problem * 6, 24 * 6 - offset) - 1, 19 - 1);
                    ColorFX.BlitTrans(RedBm.pBitmap, Bitmap.pBitmap, XY(ClientArea.x + 24 + offset, ClientArea.y + 17 + day * 19), &rect);

                    Problem -= min(Problem, 24 - offset / 6);
                    offset = 0;
                    day++;
                }
            }
        } else if (Page == 1) // Flugzeug-Details
        {
            CPlane &qPlane = qPlayer.Planes[SelectedId];
            // CPlaneType &qPlaneType = PlaneTypes[qPlane.TypeId];

            // Typ/Hersteller:
            if (!qPlane.ptHersteller.empty()) {
                Bitmap.PrintAt(bprintf("%s %s", (LPCTSTR)qPlane.ptHersteller, (LPCTSTR)qPlane.ptName), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(2, 4),
                               ClientArea + XY(172, 170));
            } else {
                Bitmap.PrintAt(qPlane.ptName, FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(2, 4), ClientArea + XY(172, 170));
            }

            // Passagiere und Reichweite:
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_PLANE, 1000), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(2, 25), ClientArea + XY(172, 170));
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_PLANE, 1017), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(2, 37), ClientArea + XY(172, 170));
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_PLANE, 1001), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(2, 37 + 11), ClientArea + XY(172, 170));
            Bitmap.PrintAt(bprintf("%li (%li)", qPlane.MaxPassagiere, qPlane.MaxPassagiereFC), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(75, 25),
                           ClientArea + XY(172, 170));
            Bitmap.PrintAt(Einheiten[EINH_T].bString(qPlane.ptPassagiere / 10), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(75, 25 + 11),
                           ClientArea + XY(172, 170));
            Bitmap.PrintAt(Einheiten[EINH_KM].bString(qPlane.ptReichweite), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(75, 37 + 11),
                           ClientArea + XY(172, 170));

            // Technische Daten:
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_PLANE, 1002), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(2, 56 - 7 + 11), ClientArea + XY(172, 170));
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_PLANE, 1008), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(2, 82 - 9), ClientArea + XY(172, 170));
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_PLANE, 1009), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(2, 95 - 10), ClientArea + XY(172, 170));
            Bitmap.PrintAt(Einheiten[EINH_KMH].bString(qPlane.ptGeschwindigkeit), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(75, 56 - 7 + 11),
                           ClientArea + XY(172, 170));
            Bitmap.PrintAt(Einheiten[EINH_L].bString(qPlane.ptTankgroesse), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(75, 82 - 9),
                           ClientArea + XY(172, 170));
            Bitmap.PrintAt(Einheiten[EINH_LH].bString(qPlane.ptVerbrauch), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(75, 95 - 10),
                           ClientArea + XY(172, 170));

            // Besatzung:
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_PLANE, 1041), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(2, 69 + 52 - 14), ClientArea + XY(172, 170));
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_PLANE, 1011), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(2, 82 + 52 - 15), ClientArea + XY(172, 170));
            // Bitmap.PrintAt (bprintf (StandardTexte.GetS (TOKEN_PLANE, 1014), qPlane.AnzPiloten, qPlaneType.AnzPiloten),   FontSmallBlack,
            // TEC_FONT_LEFT, ClientArea+XY(75,69+52-14), ClientArea+XY(172,170));
            Bitmap.PrintAt(bprintf(StandardTexte.GetS(TOKEN_PLANE, 1014), qPlane.AnzPiloten, qPlane.ptAnzPiloten), FontSmallBlack, TEC_FONT_LEFT,
                           ClientArea + XY(75, 69 + 52 - 14), ClientArea + XY(172, 170));
            // Bitmap.PrintAt (bprintf (StandardTexte.GetS (TOKEN_PLANE, 1014), qPlane.AnzBegleiter, qPlaneType.AnzBegleiter), FontSmallBlack,
            // TEC_FONT_LEFT, ClientArea+XY(75,82+52-15), ClientArea+XY(172,170));
            Bitmap.PrintAt(bprintf(StandardTexte.GetS(TOKEN_PLANE, 1014), qPlane.AnzBegleiter, qPlane.ptAnzBegleiter), FontSmallBlack, TEC_FONT_LEFT,
                           ClientArea + XY(75, 82 + 52 - 15), ClientArea + XY(172, 170));

            // Baujahr
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_PLANE, 1013), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(2, 82 + 52 - 13 + 15),
                           ClientArea + XY(172, 170));
            Bitmap.PrintAt(bitoa(qPlane.Baujahr), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(75, 82 + 52 - 13 + 15), ClientArea + XY(172, 170));

            // Flugzeug umbenennen:
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_PLANE, 1030), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(2, 103 + 52), ClientArea + XY(172, 170));
        } else if (Page == 2) // Besatzung:
        {
            SLONG c = 0;
            SLONG n = 0;
            CPlane &qPlane = qPlayer.Planes[SelectedId];
            // CPlaneType &qPlaneType = PlaneTypes[qPlane.TypeId];

            //"Besatzung":
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_PLANE, 1040), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(2, 4), ClientArea + XY(172, 170));

            //"Piloten":
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_PLANE, 1041), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(10, 4 + 13), ClientArea + XY(172, 170));

            n = 4 + 13 + 13;
            for (c = 0; c < Workers.Workers.AnzEntries(); c++) {
                if (Workers.Workers[c].Employer == PlayerNum && Workers.Workers[c].Typ == WORKER_PILOT && Workers.Workers[c].PlaneId != -1 &&
                    qPlayer.Planes(Workers.Workers[c].PlaneId) == qPlayer.Planes(SelectedId)) {
                    Bitmap.PrintAt(Workers.Workers[c].Name, FontNormalGrey, TEC_FONT_LEFT, ClientArea + XY(18, n), ClientArea + XY(172, 170));
                    n += 13;
                }
            }
            // for (c=qPlaneType.AnzPiloten-qPlane.AnzPiloten; c>0; c--)
            for (c = qPlane.ptAnzPiloten - qPlane.AnzPiloten; c > 0; c--) {
                Bitmap.PrintAt(StandardTexte.GetS(TOKEN_PLANE, 1045), FontNormalGrey, TEC_FONT_LEFT, ClientArea + XY(18, n), ClientArea + XY(172, 170));
                n += 13;
            }

            //"Flugbegleiter":
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_PLANE, 1042), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(10, n), ClientArea + XY(172, 170));

            n += 13;
            for (c = 0; c < Workers.Workers.AnzEntries(); c++) {
                if (Workers.Workers[c].Employer == PlayerNum && Workers.Workers[c].Typ == WORKER_STEWARDESS && Workers.Workers[c].PlaneId != -1 &&
                    qPlayer.Planes(Workers.Workers[c].PlaneId) == qPlayer.Planes(SelectedId)) {
                    Bitmap.PrintAt(Workers.Workers[c].Name, FontNormalGrey, TEC_FONT_LEFT, ClientArea + XY(18, n), ClientArea + XY(172, 170));
                    n += 13;
                }
            }
            // for (c=qPlaneType.AnzBegleiter-qPlane.AnzBegleiter; c>0; c--)
            for (c = qPlane.ptAnzBegleiter - qPlane.AnzBegleiter; c > 0; c--) {
                Bitmap.PrintAt(StandardTexte.GetS(TOKEN_PLANE, 1045), FontNormalGrey, TEC_FONT_LEFT, ClientArea + XY(18, n), ClientArea + XY(172, 170));
                n += 13;
            }
        } else if (Page == 3) // Statistik:
        {
            CPlane &qPlane = qPlayer.Planes[SelectedId];

            // Headline
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_PLANE, 1050), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(2, 4), ClientArea + XY(172, 170));

            // Passagiere und Reichweite:
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_PLANE, 1051), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(2, 25), ClientArea + XY(172, 170));
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_PLANE, 1052), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(2, 37), ClientArea + XY(172, 170));
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_PLANE, 1053), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(2, 49), ClientArea + XY(172, 170));
            Bitmap.PrintAt(bitoa(qPlane.SummePassagiere), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(75 + 20, 25), ClientArea + XY(172, 170));
            Bitmap.PrintAt(Einheiten[EINH_KM].bString(qPlane.Kilometer), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(75 + 20, 37),
                           ClientArea + XY(172, 170));
            Bitmap.PrintAt(CString(Einheiten[EINH_P].bString(qPlane.Auslastung)) + " / " + CString(Einheiten[EINH_P].bString(qPlane.AuslastungFC)),
                           FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(75 + 20, 49), ClientArea + XY(172, 170));
        } else // Kommentar
        {
            // CPlaneType &qPlaneType = PlaneTypes[qPlayer.Planes[SelectedId].TypeId];
            CPlane &qPlane = qPlayer.Planes[SelectedId];

            // Typ/Hersteller:
            // Bitmap.PrintAt (bprintf ("%s %s", (LPCTSTR)qPlaneType.Hersteller, (LPCTSTR)qPlaneType.Name), FontSmallBlack, TEC_FONT_LEFT,
            // ClientArea+XY(2, 4), ClientArea+XY(172,170));
            Bitmap.PrintAt(bprintf("%s %s", (LPCTSTR)qPlane.ptHersteller, (LPCTSTR)qPlane.ptName), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(2, 4),
                           ClientArea + XY(172, 170));

            // Kommentar:
            // Bitmap.PrintAt (qPlaneType.Kommentar, FontSmallBlack, TEC_FONT_LEFT, ClientArea+XY(2, 30), ClientArea+XY(172,170));
            Bitmap.PrintAt(qPlane.ptKommentar, FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(2, 30), ClientArea + XY(172, 170));
        }
        break;

        // Experten:
    case 5:
        if (SelectedId == 2) {
            Bitmap.PrintAt(CString(StandardTexte.GetS(TOKEN_EXPERT, 2100)) + " " +
                               CString(StandardTexte.GetS(TOKEN_SCHED, 3010 + (Sim.Date + Sim.StartWeekday - 1) % 7)),
                           TitleFont, TEC_FONT_LEFT, TitleArea, Bitmap.Size);
        } else if (SelectedId >= 8 && SelectedId <= 10) {
            SLONG i = SelectedId - 8;
            i += static_cast<SLONG>(PlayerNum <= i);
            auto &qPlayer = Sim.Players.Players[i];
            Bitmap.PrintAt(bprintf(StandardTexte.GetS(TOKEN_EXPERT, 2000 + SelectedId), (LPCTSTR)qPlayer.Abk), TitleFont, TEC_FONT_LEFT, TitleArea,
                           Bitmap.Size);
        } else {
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_EXPERT, 2000 + SelectedId), TitleFont, TEC_FONT_LEFT, TitleArea, Bitmap.Size);
        }

        switch (SelectedId) {
        // Gateauslastung:
        case 0:
            if (Bitmap.pBitmap != nullptr) {
                SLONG x = 0;
                SLONG y = 0;
                SLONG c = 0;
                SB_Hardwarecolor red = Bitmap.pBitmap->GetHardwarecolor(0xff0000);

                CGates &qGates = qPlayer.Gates;

                if (Base->IsLaptop == FALSE) {
                    Bitmap.BlitFromT((dynamic_cast<CGlobe *>(Base))->TimeTables[0], ClientArea + XY(21, -2));
                } else {
                    Bitmap.BlitFromT(FlugplanBms[0], ClientArea + XY(21, -2));
                }

                for (y = 0; y < 7; y++) {
                    for (x = 0; x < 24; x++) {
                        SLONG h = qGates.Auslastung[x + y * 24] * 16 / (qGates.NumRented + 1);

                        if (h != 0) {
                            Bitmap.pBitmap->Line(ClientArea.x + 24 + x * 6, ClientArea.y + 35 + 19 * y, ClientArea.x + 30 + x * 6, ClientArea.y + 35 + 19 * y,
                                                 red);
                            Bitmap.pBitmap->Line(ClientArea.x + 24 + x * 6, ClientArea.y + 35 + 19 * y - h, ClientArea.x + 30 + x * 6,
                                                 ClientArea.y + 35 + 19 * y - h, red);
                            Bitmap.pBitmap->Line(ClientArea.x + 24 + x * 6, ClientArea.y + 35 + 19 * y, ClientArea.x + 24 + x * 6,
                                                 ClientArea.y + 35 + 19 * y - h, red);
                            Bitmap.pBitmap->Line(ClientArea.x + 30 + x * 6, ClientArea.y + 35 + 19 * y, ClientArea.x + 30 + x * 6,
                                                 ClientArea.y + 35 + 19 * y - h, red);

                            if (qGates.Auslastung[x + y * 24] > qGates.NumRented) {
                                for (c = 0; c <= 4; c++) {
                                    Bitmap.Line(ClientArea + XY(25 + c + x * 6, 35 + 19 * y), ClientArea + XY(25 + c + x * 6, 35 + 19 * y - h), red);
                                }
                            }
                        }
                    }

                    Bitmap.PrintAt(StandardTexte.GetS(TOKEN_SCHED, 3000 + (y + Sim.Date + Sim.StartWeekday) % 7), FontSmallBlack, TEC_FONT_LEFT,
                                   ClientArea + XY(2, 19 + 19 * y), ClientArea + XY(172, 170));
                }
            }
            break;

            // Geschäftsbericht
        case 1:
            ZeigeFinanzBericht(ClientArea, Sim.Players.Players[PlayerNum], Sim.Players.Players[PlayerNum].BilanzWoche.Hole(), false, Page);
            break;

            // Tagesbilanz:
        case 2:
            ZeigeTagesBilanz(ClientArea, Sim.Players.Players[PlayerNum], Sim.Players.Players[PlayerNum].BilanzGestern, false, Page);
            break;

            // Wochenbilanz:
        case 3:
            ZeigeTagesBilanz(ClientArea, Sim.Players.Players[PlayerNum], Sim.Players.Players[PlayerNum].BilanzWoche.Hole(), false, Page);
            break;

            // Gesamtbilanz:
        case 4: {
            auto bilanz = Sim.Players.Players[PlayerNum].BilanzGesamt;
            bilanz += Sim.Players.Players[PlayerNum].Bilanz;
            ZeigeTagesBilanz(ClientArea, Sim.Players.Players[PlayerNum], bilanz, false, Page);
        } break;

            // Wocheneinnamen der Flugzeuge:
        case 5:
            if (Sim.Players.Players[PlayerNum].HasBerater(BERATERTYP_GELD) == 0) {
                Bitmap.PrintAt(StandardTexte.GetS(TOKEN_EXPERT, 3000), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(2, 27), ClientArea + XY(172, 170));
            } else {
                if (Page < PageSize) {
                    Bitmap.PrintAt(StandardTexte.GetS(TOKEN_EXPERT, 3009), FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(2, 27), ClientArea + XY(172, 170));
                } else {
                    // Bitmap.PrintAt (StandardTexte.GetS (TOKEN_SCHED, 1000), TitleFont, TEC_FONT_LEFT, TitleArea, Bitmap.Size);

                    for (c = 0; c < PageSize; c++) {
                        auto idx = c + Page - PageSize;
                        if (idx >= Table.AnzRows) {
                            break;
                        }
                        SB_CFont *pFont = nullptr;

                        if (Table.ValueFlags[0 + idx * Table.AnzColums] != 0U) {
                            pFont = &FontSmallRed;
                        } else {
                            pFont = &FontSmallBlack;
                        }

                        for (SLONG x = 170; x >= 0; x -= 4) {
                            if (Bitmap.TryPrintAt(Table.Values[1 + idx * Table.AnzColums], *pFont, TEC_FONT_RIGHT, ClientArea + XY(x, c * 13),
                                                  ClientArea + XY(170, c * 13 + 13)) < 13) {
                                Bitmap.PrintAt(Table.Values[1 + idx * Table.AnzColums], *pFont, TEC_FONT_RIGHT, ClientArea + XY(x, c * 13),
                                               ClientArea + XY(170, c * 13 + 13));
                                if (Bitmap.TryPrintAt(Table.Values[0 + idx * Table.AnzColums], *pFont, TEC_FONT_LEFT, ClientArea + XY(0, c * 13),
                                                      ClientArea + XY(x, c * 13 + 13)) < 13) {
                                    Bitmap.PrintAt(Table.Values[0 + idx * Table.AnzColums], *pFont, TEC_FONT_LEFT, ClientArea + XY(0, c * 13),
                                                   ClientArea + XY(x, c * 13 + 13));
                                } else {
                                    x -= 10;
                                    CString str = Table.Values[0 + idx * Table.AnzColums] + "...";

                                    while (str.GetLength() > 3) {
                                        str.SetAt(str.GetLength() - 3, '.');
                                        str = str.Left(str.GetLength() - 1);

                                        if (Bitmap.TryPrintAt(str, *pFont, TEC_FONT_LEFT, ClientArea + XY(0, c * 13), ClientArea + XY(x, c * 13 + 13)) < 13) {
                                            Bitmap.PrintAt(str, *pFont, TEC_FONT_LEFT, ClientArea + XY(0, c * 13), ClientArea + XY(x, c * 13 + 13));
                                            break;
                                        }
                                    }
                                }

                                break;
                            }
                        }
                    }
                }
            }
            break;

            // Konkurrenz:
        case 6:
            ZeigeInformantenInfos(ClientArea, Page);
            break;

        case 7:
            ZeigeInformantenFinanzBericht(ClientArea, Page);
            break;

        case 8:
            ZeigeInformantenBilanz(ClientArea, 0 + static_cast<SLONG>(PlayerNum <= 0), Page);
            break;

        case 9:
            ZeigeInformantenBilanz(ClientArea, 1 + static_cast<SLONG>(PlayerNum <= 1), Page);
            break;

        case 10:
            ZeigeInformantenBilanz(ClientArea, 2 + static_cast<SLONG>(PlayerNum <= 2), Page);
            break;

        case 11:
            ZeigeKerosinberater(ClientArea, Page);
            break;

        default:
            hprintf("Block.cpp: Default case should not be reached.");
            DebugBreak();
        }
        break;
    default:
        hprintf("Block.cpp: Default case should not be reached.");
        DebugBreak();
    }
}

void BLOCK::RechteSeiteIndex(XY TitleAreaB, XY ClientAreaB) {
    SB_CFont &TitleFont = StyleType != 0 ? FontNormalGreen : FontSmallBlack;
    PLAYER &qPlayer = Sim.Players.Players[PlayerNum];
    SLONG c = 0;

    // Inhaltsverzechnis
    switch (BlockTypeB) {
    // Auftragsliste:
    case 3:
        if (StyleType != 0) {
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_SCHED, 1200), TitleFont, TEC_FONT_LEFT, TitleAreaB + XY(60, 0), Bitmap.Size);
        } else {
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_SCHED, 1200), TitleFont, TEC_FONT_LEFT, TitleAreaB, Bitmap.Size);
        }

        for (c = PageB; c < PageB + PageSizeB && c < TableB.AnzRows; c++) {
            SB_CFont *s = nullptr;

            // Nachschauen, ob der Flug zu lang ist für das Flugzeug:
            if (Index != 1 && TableB.ValueFlags[0 + c * TableB.AnzColums] == 0 && (qPlayer.Auftraege.IsInAlbum(TableB.LineIndex[c]) != 0)) {
                // if (!qPlayer.Auftraege[TableB.LineIndex[c]].FitsInPlane
                // (PlaneTypes[Sim.Players.Players[(SLONG)PlayerNum].Planes[SelectedId].TypeId]))
                if (qPlayer.Auftraege[TableB.LineIndex[c]].FitsInPlane(Sim.Players.Players[PlayerNum].Planes[SelectedId]) == 0) {
                    TableB.ValueFlags[0 + c * TableB.AnzColums] = 2;
                }

                // if
                // (SLONG(qPlayer.Auftraege[TableB.LineIndex[c]].Personen)>PlaneTypes[Sim.Players.Players[(SLONG)PlayerNum].Planes[SelectedId].TypeId].Passagiere)
                if (SLONG(qPlayer.Auftraege[TableB.LineIndex[c]].Personen) > Sim.Players.Players[PlayerNum].Planes[SelectedId].ptPassagiere) {
                    TableB.ValueFlags[0 + c * TableB.AnzColums] = 4;
                }
            }
            // ex:!=0 ex:==1
            if (TableB.ValueFlags[0 + c * TableB.AnzColums] != 0) {
                s = &FontSmallGrey;
            } else {
                s = &FontSmallBlack;
            }

            if (TableB.ValueFlags[0 + c * TableB.AnzColums] == 1 && TableB.ValueFlags[1 + c * TableB.AnzColums] == 0) {
                Bitmap.BlitFromT(gPostItBms[1 + 2], ClientAreaB + XY(1, (c - PageB) * 26)); // Okay==0 ==> roter Rahmen ==> Etwas mit dem Flug stimmt nicht...
            } else {
                Bitmap.BlitFromT(gPostItBms[1], ClientAreaB + XY(1, (c - PageB) * 26));
            }

            Bitmap.BlitFromT(gInfoBms[0], ClientAreaB + XY(3, (c - PageB) * 26 + 9));

            if (Bitmap.TryPrintAt(bprintf("%s - %s", (LPCTSTR)TableB.Values[0 + c * TableB.AnzColums], (LPCTSTR)TableB.Values[1 + c * TableB.AnzColums]), *s,
                                  TEC_FONT_LEFT, ClientAreaB + XY(gPostItBms[0].Size.x + 2, (c - PageB) * 26),
                                  ClientAreaB + XY(XY(142, 170).x, (c - PageB) * 26 + 14)) < 12) {
                Bitmap.PrintAt(bprintf("%s - %s", (LPCTSTR)TableB.Values[0 + c * TableB.AnzColums], (LPCTSTR)TableB.Values[1 + c * TableB.AnzColums]), *s,
                               TEC_FONT_LEFT, ClientAreaB + XY(gPostItBms[0].Size.x + 2, (c - PageB) * 26),
                               ClientAreaB + XY(XY(142, 170).x, (c - PageB) * 26 + 14));
            } else if (Bitmap.TryPrintAt(bprintf("%s - %s", (LPCTSTR)TableB.Values[0 + c * TableB.AnzColums], (LPCTSTR)TableB.Values[3 + c * TableB.AnzColums]),
                                         *s, TEC_FONT_LEFT, ClientAreaB + XY(gPostItBms[0].Size.x + 2, (c - PageB) * 26),
                                         ClientAreaB + XY(XY(142, 170).x, (c - PageB) * 26 + 14)) < 12) {
                Bitmap.PrintAt(bprintf("%s - %s", (LPCTSTR)TableB.Values[0 + c * TableB.AnzColums], (LPCTSTR)TableB.Values[3 + c * TableB.AnzColums]), *s,
                               TEC_FONT_LEFT, ClientAreaB + XY(gPostItBms[0].Size.x + 2, (c - PageB) * 26),
                               ClientAreaB + XY(XY(142, 170).x, (c - PageB) * 26 + 14));
            } else {
                Bitmap.PrintAt(bprintf("%s - %s", (LPCTSTR)TableB.Values[2 + c * TableB.AnzColums], (LPCTSTR)TableB.Values[3 + c * TableB.AnzColums]), *s,
                               TEC_FONT_LEFT, ClientAreaB + XY(gPostItBms[0].Size.x + 2, (c - PageB) * 26),
                               ClientAreaB + XY(XY(142, 170).x, (c - PageB) * 26 + 14));
            }

            Bitmap.PrintAt(TableB.Values[6 + c * TableB.AnzColums], *s, TEC_FONT_RIGHT, ClientAreaB + XY(gPostItBms[0].Size.x + 2, (c - PageB) * 26),
                           ClientAreaB + XY(XY(172, 170).x, (c - PageB) * 26 + 14));

            Bitmap.PrintAt(TableB.Values[4 + c * TableB.AnzColums], *s, TEC_FONT_LEFT, ClientAreaB + XY(gPostItBms[0].Size.x + 2, (c - PageB) * 26 + 10),
                           ClientAreaB + XY(XY(172, 170).x, (c - PageB) * 26 + 13 + 10));
            Bitmap.PrintAt(TableB.Values[5 + c * TableB.AnzColums], *s, TEC_FONT_RIGHT, ClientAreaB + XY(gPostItBms[0].Size.x + 2, (c - PageB) * 26 + 10),
                           ClientAreaB + XY(XY(172, 170).x, (c - PageB) * 26 + 13 + 10));
        }
        break;

        // Routenliste:
    case 4:
        if (StyleType != 0) {
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_SCHED, 1100), TitleFont, TEC_FONT_LEFT, TitleAreaB + XY(60, 0), Bitmap.Size);
        } else {
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_SCHED, 1100), TitleFont, TEC_FONT_LEFT, TitleAreaB, Bitmap.Size);
        }

        for (c = PageB; c < PageB + PageSizeB && c < TableB.AnzRows; c++) {
            SB_CFont *s = nullptr;

            Bitmap.BlitFromT(gPostItBms[0], ClientAreaB + XY(1, (c - PageB) * 26));
            Bitmap.BlitFromT(gInfoBms[0], ClientAreaB + XY(3, (c - PageB) * 26 + 9));

            s = &FontSmallBlack;

            // Nachschauen, ob der Flug zu lang ist für das Flugzeug:
            if (Index != 1) {
                SLONG VonCityId = Routen[TableB.LineIndex[c]].VonCity;
                SLONG NachCityId = Routen[TableB.LineIndex[c]].NachCity;

                // if (Cities.CalcDistance (VonCityId,
                // NachCityId)>PlaneTypes[Sim.Players.Players[(SLONG)PlayerNum].Planes[SelectedId].TypeId].Reichweite*1000)
                if (Cities.CalcDistance(VonCityId, NachCityId) > Sim.Players.Players[PlayerNum].Planes[SelectedId].ptReichweite * 1000) {
                    TableB.ValueFlags[0 + c * TableB.AnzColums] = 2;
                } else {
                    // SLONG Speed = PlaneTypes[Sim.Players.Players[(SLONG)PlayerNum].Planes[SelectedId].TypeId].Geschwindigkeit;
                    SLONG Speed = Sim.Players.Players[PlayerNum].Planes[SelectedId].ptGeschwindigkeit;
                    SLONG Dauer = Cities.CalcFlugdauer(VonCityId, NachCityId, Speed);
                    // SLONG Dauer = (Cities.CalcDistance (VonCityId, NachCityId)/Speed+999)/1000+1+2;

                    if (Dauer >= 24) {
                        TableB.ValueFlags[0 + c * TableB.AnzColums] = 2;
                    }
                }
            }

            if (TableB.ValueFlags[0 + c * TableB.AnzColums] != 0U) {
                s = &FontSmallGrey;
            }

            if (Bitmap.TryPrintAt(bprintf("%s - %s", (LPCTSTR)TableB.Values[0 + c * TableB.AnzColums], (LPCTSTR)TableB.Values[1 + c * TableB.AnzColums]), *s,
                                  TEC_FONT_LEFT, ClientAreaB + XY(gPostItBms[0].Size.x + 2, (c - PageB) * 26),
                                  ClientAreaB + XY(XY(172, 170).x, (c - PageB) * 26 + 14)) < 12) {
                Bitmap.PrintAt(bprintf("%s - %s", (LPCTSTR)TableB.Values[0 + c * TableB.AnzColums], (LPCTSTR)TableB.Values[1 + c * TableB.AnzColums]), *s,
                               TEC_FONT_LEFT, ClientAreaB + XY(gPostItBms[0].Size.x + 2, (c - PageB) * 26),
                               ClientAreaB + XY(XY(172, 170).x, (c - PageB) * 26 + 14));
            } else if (Bitmap.TryPrintAt(bprintf("%s - %s", (LPCTSTR)TableB.Values[0 + c * TableB.AnzColums], (LPCTSTR)TableB.Values[3 + c * TableB.AnzColums]),
                                         *s, TEC_FONT_LEFT, ClientAreaB + XY(gPostItBms[0].Size.x + 2, (c - PageB) * 26),
                                         ClientAreaB + XY(XY(172, 170).x, (c - PageB) * 26 + 14)) < 12) {
                Bitmap.PrintAt(bprintf("%s - %s", (LPCTSTR)TableB.Values[0 + c * TableB.AnzColums], (LPCTSTR)TableB.Values[3 + c * TableB.AnzColums]), *s,
                               TEC_FONT_LEFT, ClientAreaB + XY(gPostItBms[0].Size.x + 2, (c - PageB) * 26),
                               ClientAreaB + XY(XY(172, 170).x, (c - PageB) * 26 + 14));
            } else {
                Bitmap.PrintAt(bprintf("%s - %s", (LPCTSTR)TableB.Values[2 + c * TableB.AnzColums], (LPCTSTR)TableB.Values[3 + c * TableB.AnzColums]), *s,
                               TEC_FONT_LEFT, ClientAreaB + XY(gPostItBms[0].Size.x + 2, (c - PageB) * 26),
                               ClientAreaB + XY(XY(172, 170).x, (c - PageB) * 26 + 14));
            }

            Bitmap.PrintAt(TableB.Values[4 + c * TableB.AnzColums], *s, TEC_FONT_LEFT, ClientAreaB + XY(gPostItBms[0].Size.x + 2, (c - PageB) * 26 + 10),
                           ClientAreaB + XY(XY(172, 170).x, (c - PageB) * 26 + 13 + 10));
        }
        break;

        // Frachtliste:
    case 6:
        if (StyleType != 0) {
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_SCHED, 1300), TitleFont, TEC_FONT_LEFT, TitleAreaB + XY(60, 0), Bitmap.Size);
        } else {
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_SCHED, 1300), TitleFont, TEC_FONT_LEFT, TitleAreaB, Bitmap.Size);
        }

        for (c = PageB; c < PageB + PageSizeB && c < TableB.AnzRows; c++) {
            SB_CFont *s = nullptr;

            // Nachschauen, ob der Flug zu lang ist für das Flugzeug:
            if (Index != 1 && TableB.ValueFlags[0 + c * TableB.AnzColums] == 0 && (qPlayer.Frachten.IsInAlbum(TableB.LineIndex[c]) != 0)) {
                // if (!qPlayer.Frachten[TableB.LineIndex[c]].FitsInPlane (PlaneTypes[Sim.Players.Players[(SLONG)PlayerNum].Planes[SelectedId].TypeId]))
                if (qPlayer.Frachten[TableB.LineIndex[c]].FitsInPlane(Sim.Players.Players[PlayerNum].Planes[SelectedId]) == 0) {
                    TableB.ValueFlags[0 + c * TableB.AnzColums] = 2;
                }
            }
            // ex: ==1
            if (TableB.ValueFlags[0 + c * TableB.AnzColums] != 0) {
                s = &FontSmallGrey;
            } else {
                s = &FontSmallBlack;
            }

            if (TableB.ValueFlags[1 + c * TableB.AnzColums] == 0U) {
                Bitmap.BlitFromT(gPostItBms[5], ClientAreaB + XY(1, (c - PageB) * 26)); // Okay==0 ==> roter Rahmen ==> Etwas mit dem Flug stimmt nicht...
            } else {
                Bitmap.BlitFromT(gPostItBms[4], ClientAreaB + XY(1, (c - PageB) * 26));
            }

            Bitmap.BlitFromT(gInfoBms[0], ClientAreaB + XY(3, (c - PageB) * 26 + 9));

            if (Bitmap.TryPrintAt(bprintf("%s - %s", (LPCTSTR)TableB.Values[0 + c * TableB.AnzColums], (LPCTSTR)TableB.Values[1 + c * TableB.AnzColums]), *s,
                                  TEC_FONT_LEFT, ClientAreaB + XY(gPostItBms[0].Size.x + 2, (c - PageB) * 26),
                                  ClientAreaB + XY(XY(142, 170).x, (c - PageB) * 26 + 14)) < 12) {
                Bitmap.PrintAt(bprintf("%s - %s", (LPCTSTR)TableB.Values[0 + c * TableB.AnzColums], (LPCTSTR)TableB.Values[1 + c * TableB.AnzColums]), *s,
                               TEC_FONT_LEFT, ClientAreaB + XY(gPostItBms[0].Size.x + 2, (c - PageB) * 26),
                               ClientAreaB + XY(XY(142, 170).x, (c - PageB) * 26 + 14));
            } else if (Bitmap.TryPrintAt(bprintf("%s - %s", (LPCTSTR)TableB.Values[0 + c * TableB.AnzColums], (LPCTSTR)TableB.Values[3 + c * TableB.AnzColums]),
                                         *s, TEC_FONT_LEFT, ClientAreaB + XY(gPostItBms[0].Size.x + 2, (c - PageB) * 26),
                                         ClientAreaB + XY(XY(142, 170).x, (c - PageB) * 26 + 14)) < 12) {
                Bitmap.PrintAt(bprintf("%s - %s", (LPCTSTR)TableB.Values[0 + c * TableB.AnzColums], (LPCTSTR)TableB.Values[3 + c * TableB.AnzColums]), *s,
                               TEC_FONT_LEFT, ClientAreaB + XY(gPostItBms[0].Size.x + 2, (c - PageB) * 26),
                               ClientAreaB + XY(XY(142, 170).x, (c - PageB) * 26 + 14));
            } else {
                Bitmap.PrintAt(bprintf("%s - %s", (LPCTSTR)TableB.Values[2 + c * TableB.AnzColums], (LPCTSTR)TableB.Values[3 + c * TableB.AnzColums]), *s,
                               TEC_FONT_LEFT, ClientAreaB + XY(gPostItBms[0].Size.x + 2, (c - PageB) * 26),
                               ClientAreaB + XY(XY(142, 170).x, (c - PageB) * 26 + 14));
            }

            Bitmap.PrintAt(TableB.Values[6 + c * TableB.AnzColums], *s, TEC_FONT_RIGHT, ClientAreaB + XY(gPostItBms[0].Size.x + 2, (c - PageB) * 26),
                           ClientAreaB + XY(XY(172, 170).x, (c - PageB) * 26 + 14));

            Bitmap.PrintAt(TableB.Values[4 + c * TableB.AnzColums], *s, TEC_FONT_LEFT, ClientAreaB + XY(gPostItBms[0].Size.x + 2, (c - PageB) * 26 + 10),
                           ClientAreaB + XY(XY(172, 170).x, (c - PageB) * 26 + 13 + 10));
            Bitmap.PrintAt(TableB.Values[5 + c * TableB.AnzColums], *s, TEC_FONT_RIGHT, ClientAreaB + XY(gPostItBms[0].Size.x + 2, (c - PageB) * 26 + 10),
                           ClientAreaB + XY(XY(172, 170).x, (c - PageB) * 26 + 13 + 10));
        }
        break;
    default:
        hprintf("Block.cpp: Default case should not be reached.");
        DebugBreak();
    }
}

bool BLOCK::RechteSeiteInhalt(XY TitleAreaB, XY ClientAreaB) {
    SB_CFont &TitleFont = StyleType != 0 ? FontNormalGreen : FontSmallBlack;
    PLAYER &qPlayer = Sim.Players.Players[PlayerNum];

    // Details
    switch (BlockTypeB) {
    // Aufträge:
    case 3: {
        if (qPlayer.Auftraege.IsInAlbum(SelectedIdB) == 0) {
            qPlayer.Blocks.RepaintAll = 1;
            IndexB = 1;
            SelectedIdB = PageB = 0;
            UpdatePageSize();
            return true;
        }

        CAuftrag &qAuftrag = qPlayer.Auftraege[SelectedIdB];

        // Headline: LAX - DUS
        Bitmap.PrintAt(bprintf("%s - %s", (LPCTSTR)Cities[qAuftrag.VonCity].Kuerzel, (LPCTSTR)Cities[qAuftrag.NachCity].Kuerzel), TitleFont, TEC_FONT_LEFT,
                       TitleAreaB, Bitmap.Size);

        // Ausführlich von A nach B
        Bitmap.PrintAt(bprintf("%s - %s", (LPCTSTR)Cities[qAuftrag.VonCity].Name, (LPCTSTR)Cities[qAuftrag.NachCity].Name), FontSmallBlack, TEC_FONT_LEFT,
                       ClientAreaB + XY(0, 1), ClientAreaB + XY(172, 168));

        if (PageB == 0) {
            // Personenzahl:
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_AUFTRAG, 1000), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(0, 28), ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(bprintf("%li", qAuftrag.Personen), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(75, 28), ClientAreaB + XY(172, 170));

            // Datum:
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_AUFTRAG, 1003), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(0, 40), ClientAreaB + XY(172, 170));

            if (qAuftrag.Date == qAuftrag.BisDate) {
                Bitmap.PrintAt(StandardTexte.GetS(TOKEN_SCHED, 3010 + (qAuftrag.Date + Sim.StartWeekday) % 7), FontSmallBlack, TEC_FONT_LEFT,
                               ClientAreaB + XY(75, 40), ClientAreaB + XY(172, 170));
            } else {
                Bitmap.PrintAt(CString(StandardTexte.GetS(TOKEN_SCHED, 3009)) + " " +
                                   CString(StandardTexte.GetS(TOKEN_SCHED, 3010 + (qAuftrag.BisDate + Sim.StartWeekday) % 7)),
                               FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(75, 40), ClientAreaB + XY(172, 170));
            }

            // Prämie und Strafe
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_AUFTRAG, 1002), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(0, 66), ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_AUFTRAG, 1001), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(0, 53), ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(bprintf("%s", (LPCTSTR)CString(Einheiten[EINH_DM].bString(qAuftrag.Strafe))), FontSmallBlack, TEC_FONT_LEFT,
                           ClientAreaB + XY(75, 66), ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(bprintf("%s", (LPCTSTR)CString(Einheiten[EINH_DM].bString(qAuftrag.Praemie))), FontSmallBlack, TEC_FONT_LEFT,
                           ClientAreaB + XY(75, 53), ClientAreaB + XY(172, 170));

            // Entfernung:
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_AUFTRAG, 1007), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(0, 92), ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(CString(Einheiten[EINH_KM].bString(Cities.CalcDistance(qAuftrag.VonCity, qAuftrag.NachCity) / 1000)), FontSmallBlack, TEC_FONT_LEFT,
                           ClientAreaB + XY(75, 92), ClientAreaB + XY(172, 170));

            // Wegwerfen:
            if (qAuftrag.InPlan == 0) {
                Bitmap.BlitFrom(FlugplanBms[57], ClientAreaB + XY(0, 118));
                if (qAuftrag.Strafe != 0) {
                    Bitmap.PrintAt(StandardTexte.GetS(TOKEN_AUFTRAG, 2401), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(20, 119),
                                   ClientAreaB + XY(172, 170));
                } else {
                    Bitmap.PrintAt(StandardTexte.GetS(TOKEN_AUFTRAG, 2400), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(20, 119),
                                   ClientAreaB + XY(172, 170));
                }
            }
        } else if (PageB == 1) {
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_AUFTRAG, 2500), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(0, 28), ClientAreaB + XY(172, 170));
        }
    } break;

        // Routen:
    case 4: {
        if (Routen.IsInAlbum(SelectedIdB) == 0) {
            qPlayer.Blocks.RepaintAll = 1;
            IndexB = 1;
            SelectedIdB = PageB = 0;
            UpdatePageSize();
            return true;
        }

        CRoute &qRoute = Routen[SelectedIdB];
        CRentRoute &qRRoute = qPlayer.RentRouten.RentRouten[Routen(SelectedIdB)];

        // Headline: LAX - DUS
        Bitmap.PrintAt(bprintf("%s - %s", (LPCTSTR)Cities[qRoute.VonCity].Kuerzel, (LPCTSTR)Cities[qRoute.NachCity].Kuerzel), TitleFont, TEC_FONT_LEFT,
                       TitleAreaB, Bitmap.Size);

        // Ausführlich von A nach B
        Bitmap.PrintAt(bprintf("%s - %s", (LPCTSTR)Cities[qRoute.VonCity].Name, (LPCTSTR)Cities[qRoute.NachCity].Name), FontSmallBlack, TEC_FONT_LEFT,
                       ClientAreaB + XY(0, 1), ClientAreaB + XY(172, 168));

        if (PageB == 0) {
            // Ticketpreis:
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_ROUTE, 1007), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(2, 27), ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(Einheiten[EINH_DM].bString(qRRoute.Ticketpreis), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(80, 27),
                           ClientAreaB + XY(172, 170));
            Bitmap.PrintAt("+-", FontSmallBlack, TEC_FONT_RIGHT, ClientAreaB + XY(2, 27), ClientAreaB + XY(172, 170));

            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_ROUTE, 1007), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(2, 40), ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(Einheiten[EINH_DM].bString(qRRoute.TicketpreisFC), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(80, 40),
                           ClientAreaB + XY(172, 170));

            // erhöhen oder verringern:
            // Bitmap.PrintAt (StandardTexte.GetS (TOKEN_ROUTE, 1100), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB+XY(2,40), ClientAreaB+XY(172,170));
            // Bitmap.PrintAt (StandardTexte.GetS (TOKEN_ROUTE, 1101), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB+XY(2,53), ClientAreaB+XY(172,170));

            // Ticketpreis von Dumping bis Luxus:
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_ROUTE, 1102), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(2, 66), ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_ROUTE, 1103), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(2, 79), ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_ROUTE, 1104), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(2, 92), ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_ROUTE, 1105), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(2, 105), ClientAreaB + XY(172, 170));

            SLONG Cost = CalculateFlightCost(qRoute.VonCity, qRoute.NachCity, 800, 800, -1) * 3 / 180 * 2;

            Bitmap.PrintAt(Einheiten[EINH_DM].bString(Cost / 2 / 10 * 10), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(80, 66), ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(Einheiten[EINH_DM].bString(Cost / 10 * 10), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(80, 79), ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(Einheiten[EINH_DM].bString(Cost * 2 / 10 * 10), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(80, 92), ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(Einheiten[EINH_DM].bString(Cost * 4 / 10 * 10), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(80, 105),
                           ClientAreaB + XY(172, 170));

            if (Sim.Players.Players[PlayerNum].HasBerater(BERATERTYP_INFO) != 0) {
                for (SLONG c = 0; c < 3; c++) {
                    if (Sim.Players.Players[c + static_cast<SLONG>(PlayerNum <= c)].IsOut == 0) {
                        Bitmap.PrintAt(Sim.Players.Players[c + static_cast<SLONG>(PlayerNum <= c)].AirlineX, FontSmallBlack, TEC_FONT_LEFT,
                                       ClientAreaB + XY(2, 128 + c * 13), ClientAreaB + XY(172, 170));

                        if (Sim.Players.Players[c + static_cast<SLONG>(PlayerNum <= c)].RentRouten.RentRouten[Routen(SelectedIdB)].Rang != 0U) {
                            Bitmap.PrintAt(
                                Einheiten[EINH_DM].bString(
                                    Sim.Players.Players[c + static_cast<SLONG>(PlayerNum <= c)].RentRouten.RentRouten[Routen(SelectedIdB)].Ticketpreis),
                                FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(100, 128 + c * 13), ClientAreaB + XY(172, 170));
                        } else {
                            Bitmap.PrintAt("-", FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(100, 128 + c * 13), ClientAreaB + XY(172, 170));
                        }
                    }
                }
            }
        } else if (PageB == 1) {
            // Entfernung:
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_AUFTRAG, 1007), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(2, 23), ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(CString(Einheiten[EINH_KM].bString(Cities.CalcDistance(qRoute.VonCity, qRoute.NachCity) / 1000)), FontSmallBlack, TEC_FONT_LEFT,
                           ClientAreaB + XY(100, 23), ClientAreaB + XY(172, 170));

            // Image:
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_ROUTE, 1004), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(2, 43), ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(Einheiten[EINH_P].bString(qRRoute.Image), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(100, 43), ClientAreaB + XY(172, 170));
            // Rang:
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_ROUTE, 1011), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(2, 55), ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(bitoa(qRRoute.Rang), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(100, 55), ClientAreaB + XY(172, 170));

            // Bedarf:
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_ROUTE, 1013), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(2, 76 - 2), ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(bitoa(qRoute.Bedarf), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(100, 76 - 2), ClientAreaB + XY(172, 170));
            // Auslastung:
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_ROUTE, 1017), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(2, 86 + 3 + 2), ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_ROUTE, 1018), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(2, 84 + 12 + 3 + 2),
                           ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(CString(Einheiten[EINH_P].bString(qRRoute.Auslastung)) + " / " + CString(Einheiten[EINH_P].bString(qRRoute.AuslastungFC)),
                           FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(100, 92 + 4 + 3 + 2), ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_ROUTE, 1019), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(2, 105 - 1 + 4 + 2 + 2),
                           ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(Einheiten[EINH_P].bString(qRRoute.RoutenAuslastung), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(100, 105 - 1 + 4 + 2 + 2),
                           ClientAreaB + XY(172, 170));

            // Miete:
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_ROUTE, 1006), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(2, 131 - 2), ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(Einheiten[EINH_DM].bString(qRRoute.Miete), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(100, 131 - 2),
                           ClientAreaB + XY(172, 170));
            // Kündigen:
            Bitmap.BlitFrom(FlugplanBms[57], ClientAreaB + XY(2, 144 - 2));
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_ROUTE, 1200), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(22, 148), Bitmap.Size);
        } else if (PageB == 2) {
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_AUFTRAG, 2502), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(0, 28), ClientAreaB + XY(172, 170));
        }
    } break;

        // Frachtaufträge:
    case 6: {
        if (qPlayer.Frachten.IsInAlbum(SelectedIdB) == 0) {
            qPlayer.Blocks.RepaintAll = 1;
            IndexB = 1;
            SelectedIdB = PageB = 0;
            UpdatePageSize();
            return true;
        }

        CFracht &qFracht = qPlayer.Frachten[SelectedIdB];

        // Headline: LAX - DUS
        Bitmap.PrintAt(bprintf("%s - %s", (LPCTSTR)Cities[qFracht.VonCity].Kuerzel, (LPCTSTR)Cities[qFracht.NachCity].Kuerzel), TitleFont, TEC_FONT_LEFT,
                       TitleAreaB, Bitmap.Size);

        // Ausführlich von A nach B
        Bitmap.PrintAt(bprintf("%s - %s", (LPCTSTR)Cities[qFracht.VonCity].Name, (LPCTSTR)Cities[qFracht.NachCity].Name), FontSmallBlack, TEC_FONT_LEFT,
                       ClientAreaB + XY(0, 1), ClientAreaB + XY(172, 168));

        if (PageB == 0) {
            // Frachtinfos:
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_AUFTRAG, 1008), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(0, 28), ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(Einheiten[EINH_T].bString(qFracht.Tons), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(95, 28), ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_AUFTRAG, 1010), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(0, 28 + 9), ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(Einheiten[EINH_T].bString(qFracht.TonsLeft > 0 ? qFracht.TonsLeft : 0), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(95, 28 + 9),
                           ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_AUFTRAG, 1009), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(0, 28 + 18), ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(Einheiten[EINH_T].bString(qFracht.TonsOpen > 0 ? qFracht.TonsOpen : 0), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(95, 28 + 18),
                           ClientAreaB + XY(172, 170));

            // Datum:
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_AUFTRAG, 1003), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(0, 40 + 23), ClientAreaB + XY(172, 170));

            if (qFracht.Date == qFracht.BisDate) {
                Bitmap.PrintAt(StandardTexte.GetS(TOKEN_SCHED, 3010 + (qFracht.Date + Sim.StartWeekday) % 7), FontSmallBlack, TEC_FONT_LEFT,
                               ClientAreaB + XY(75, 40 + 23), ClientAreaB + XY(172, 170));
            } else {
                Bitmap.PrintAt(CString(StandardTexte.GetS(TOKEN_SCHED, 3009)) + " " +
                                   CString(StandardTexte.GetS(TOKEN_SCHED, 3010 + (qFracht.BisDate + Sim.StartWeekday) % 7)),
                               FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(75, 40 + 23), ClientAreaB + XY(172, 170));
            }

            // Prämie und Strafe
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_AUFTRAG, 1002), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(0, 66 + 19), ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_AUFTRAG, 1001), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(0, 53 + 21), ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(bprintf("%s", (LPCTSTR)CString(Einheiten[EINH_DM].bString(qFracht.Strafe))), FontSmallBlack, TEC_FONT_LEFT,
                           ClientAreaB + XY(75, 66 + 19), ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(
                bprintf("%s", (LPCTSTR)CString((qFracht.Praemie == 0) ? StandardTexte.GetS(TOKEN_AUFTRAG, 1011) : Einheiten[EINH_DM].bString(qFracht.Praemie))),
                FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(75, 53 + 21), ClientAreaB + XY(172, 170));

            // Entfernung:
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_AUFTRAG, 1007), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(0, 92 + 17), ClientAreaB + XY(172, 170));
            Bitmap.PrintAt(CString(Einheiten[EINH_KM].bString(Cities.CalcDistance(qFracht.VonCity, qFracht.NachCity) / 1000)), FontSmallBlack, TEC_FONT_LEFT,
                           ClientAreaB + XY(75, 92 + 17), ClientAreaB + XY(172, 170));

            // Wegwerfen:
            if (qPlayer.Frachten[SelectedIdB].InPlan == 0 && qPlayer.Frachten[SelectedIdB].TonsLeft == qPlayer.Frachten[SelectedIdB].TonsOpen) {
                Bitmap.BlitFrom(FlugplanBms[57], ClientAreaB + XY(0, 118 + 17));
                if (qFracht.Strafe != 0) {
                    Bitmap.PrintAt(StandardTexte.GetS(TOKEN_AUFTRAG, 2401), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(20, 119 + 17),
                                   ClientAreaB + XY(172, 170));
                } else {
                    Bitmap.PrintAt(StandardTexte.GetS(TOKEN_AUFTRAG, 2400), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(20, 119 + 17),
                                   ClientAreaB + XY(172, 170));
                }
            }
        } else if (PageB == 1) {
            Bitmap.PrintAt(StandardTexte.GetS(TOKEN_AUFTRAG, 2501), FontSmallBlack, TEC_FONT_LEFT, ClientAreaB + XY(0, 28), ClientAreaB + XY(172, 170));
        }
    } break;
    default:
        hprintf("Block.cpp: Default case should not be reached.");
        DebugBreak();
    }
    return false;
}

//--------------------------------------------------------------------------------------------
// Bringt die Daten (die später durch Refresh angezeigt werden) auf den neusten Stand:
//--------------------------------------------------------------------------------------------
void BLOCK::RefreshData(SLONG PlayerNum) {
    switch (BlockType) {
    // Städte:
    case 1:
        Table.FillWithCities(&Sim.Players.Players[PlayerNum].RentCities);
        break;

        // Flugzeuge:
    case 2:
        // Filter?
        if (BlockTypeB == 3 && IndexB == FALSE && PageB == 1) {
            Table.FillWithPlanes(&Sim.Players.Players[PlayerNum].Planes, FALSE, 2, SelectedIdB);
        } else if (BlockTypeB == 4 && IndexB == FALSE && PageB == 2) {
            // Find vice-versa Route:
            for (SLONG c = 0; c < Routen.AnzEntries(); c++) {
                if ((Routen.IsInAlbum(c) != 0) && Routen[c].VonCity == Routen[SelectedIdB].NachCity && Routen[c].NachCity == Routen[SelectedIdB].VonCity) {
                    Table.FillWithPlanes(&Sim.Players.Players[PlayerNum].Planes, FALSE, 1, SelectedIdB, Routen.GetIdFromIndex(c));
                    break;
                }
            }
        } else if (BlockTypeB == 6 && IndexB == FALSE && PageB == 1) {
            Table.FillWithPlanes(&Sim.Players.Players[PlayerNum].Planes, FALSE, 4, SelectedIdB);
        } else {
            // Kein Filter:
            Table.FillWithPlanes(&Sim.Players.Players[PlayerNum].Planes);
        }
        break;

        // Experten:
    case 5:
        Table.FillWithExperts(PlayerNum);
        if (Index == 0 && SelectedId == 5) {
            Table.FillWithPlanes(&Sim.Players.Players[PlayerNum].Planes, TRUE);
        }
        break;
    default:
        break;
    }

    switch (BlockTypeB) {
    // Aufträge:
    case 3:
        TableB.FillWithAuftraege(&Sim.Players.Players[PlayerNum].Auftraege);
        break;

        // Routen:
    case 4:
        TableB.FillWithRouten(&::Routen, &Sim.Players.Players[PlayerNum].RentRouten /*, &Sim.Players.Players[(SLONG)PlayerNum].RentCities*/);
        break;

        // Frachtaufträge:
    case 6:
        TableB.FillWithFracht(&Sim.Players.Players[PlayerNum].Frachten);
        break;
    default:
        break;
    }

    UpdatePageSize();
}

//--------------------------------------------------------------------------------------------
// Zeigt die Informationen vom Finanzberater / Informant
//--------------------------------------------------------------------------------------------
void BLOCK::PrintLine(XY ClientArea, SLONG rowID, const char *str) const {
    if (str == nullptr) {
        return;
    }
    Bitmap.PrintAt(str, FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(2, rowID * 13), ClientArea + XY(172, 170));
}
void BLOCK::PrintLineAlignRight(XY ClientArea, SLONG rowID, const char *str) const {
    if (str == nullptr) {
        return;
    }
    Bitmap.PrintAt(str, FontSmallBlack, TEC_FONT_RIGHT, ClientArea + XY(2, rowID * 13), ClientArea + XY(172, 170));
}
SLONG BLOCK::PrintLine(XY ClientArea, SLONG rowID, SLONG textID) const {
    if (textID == -1) {
        return 0;
    }
    char *str = StandardTexte.GetS(TOKEN_EXPERT, textID);
    SLONG count = 0;
    for (SLONG i = 1; (str[i - 1] != 0 && str[i] != 0); i++) {
        if (str[i - 1] == ';' && str[i] == ';') {
            ++count;
            str[i - 1] = '\r';
            str[i] = '\n';
        }
    }
    Bitmap.PrintAt(str, FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(2, rowID * 13), ClientArea + XY(172, 170));
    return count;
}
void BLOCK::PrintLineAlignRight(XY ClientArea, SLONG rowID, SLONG textID) const {
    if (textID == -1) {
        return;
    }
    const char *str = StandardTexte.GetS(TOKEN_EXPERT, textID);
    Bitmap.PrintAt(str, FontSmallBlack, TEC_FONT_RIGHT, ClientArea + XY(2, rowID * 13), ClientArea + XY(172, 170));
}
void BLOCK::PrintLineHeading(XY ClientArea, SLONG rowID, SLONG textID) const {
    if (textID == -1) {
        return;
    }
    const char *str = StandardTexte.GetS(TOKEN_EXPERT, textID);
    Bitmap.PrintAt(str, FontSmallBlack, TEC_FONT_LEFT, ClientArea + XY(2, rowID * 13), ClientArea + XY(172, 170));
}
SLONG BLOCK::PrintLineWithValueT(XY ClientArea, SLONG rowID, SLONG textID, __int64 value) {
    SLONG ret = PrintLine(ClientArea, rowID, textID);
    SLONG center = 5 * ret;
    Bitmap.PrintAt(bitoa(AufTausenderRunden(value)), FontSmallBlack, TEC_FONT_RIGHT, ClientArea + XY(2, rowID * 13 + center), ClientArea + XY(172, 170));
    return ret;
}
SLONG BLOCK::PrintLineWithValueMio(XY ClientArea, SLONG rowID, SLONG textID, __int64 value) {
    SLONG ret = PrintLine(ClientArea, rowID, textID);
    SLONG center = 5 * ret;
    Bitmap.PrintAt(CalcMillionen(value), FontSmallBlack, TEC_FONT_RIGHT, ClientArea + XY(2, rowID * 13 + center), ClientArea + XY(172, 170));
    return ret;
}
SLONG BLOCK::PrintLineWithPercentage(XY ClientArea, SLONG rowID, SLONG textID, __int64 value, __int64 div) {
    SLONG ret = PrintLine(ClientArea, rowID, textID);
    SLONG center = 5 * ret;
    Bitmap.PrintAt(CalcPercentage(value, div), FontSmallBlack, TEC_FONT_RIGHT, ClientArea + XY(2, rowID * 13 + center), ClientArea + XY(172, 170));
    return ret;
}
SLONG BLOCK::PrintList(XY ClientArea, const std::vector<std::pair<SLONG, __int64>> &list, SLONG idx) {
    __int64 summe = 0;
    for (const auto &i : list) {
        idx += 1 + PrintLineWithValueT(ClientArea, idx, i.first, i.second);
        summe += i.second;
    }
    ++idx;
    idx += 1 + PrintLineWithValueT(ClientArea, idx, 3604, summe);
    return idx;
}
SLONG BLOCK::PrintList(XY ClientArea, const std::vector<std::tuple<SLONG, __int64, __int64>> &list, SLONG idx) {
    for (const auto &i : list) {
        auto div = std::get<2>(i);
        if (div == 0) {
            idx += 1 + PrintLineWithValueT(ClientArea, idx, std::get<0>(i), std::get<1>(i));
        } else {
            idx += 1 + PrintLineWithPercentage(ClientArea, idx, std::get<0>(i), std::get<1>(i), div);
        }
    }
    return idx;
}

void BLOCK::ZeigeFinanzBericht(XY ClientArea, const PLAYER &player, const CBilanz &ref, bool info, SLONG page) {
    SLONG berater = BERATERTYP_GELD;
    SLONG textKeinBerater = 3000;
    SLONG textBeraterZuSchlecht = 3001;
    if (info) {
        berater = BERATERTYP_INFO;
        textKeinBerater = 3002;
        textBeraterZuSchlecht = 3003;
    }

    if (Sim.Players.Players[PlayerNum].HasBerater(berater) == 0) {
        PrintLine(ClientArea, 2, textKeinBerater);
        return;
    }
    if (page == 0) {
        std::vector<std::tuple<SLONG, __int64, __int64>> tmp = {{3601, ref.GetHaben(), 0}, {3602, ref.GetSoll(), 0}, {3604, ref.GetSumme(), 0}};
        if (!info) {
            PrintLineHeading(ClientArea, 0, 10151);
        }
        SLONG idx = 1;
        idx = PrintList(ClientArea, tmp, idx);
        ++idx;

        if (Sim.Players.Players[PlayerNum].HasBerater(berater) < 80) {
            PrintLine(ClientArea, idx, textBeraterZuSchlecht);
            return;
        }

        std::vector<std::tuple<SLONG, __int64, __int64>> tmp2 = {{10100, ref.GetOpGewinn(), 0},  {-1, ref.GetOpGewinn(), ref.GetHaben()},
                                                                 {10101, ref.GetOpVerlust(), 0}, {-1, ref.GetOpVerlust(), ref.GetSoll()},
                                                                 {10102, ref.GetOpSaldo(), 0},   {-1, ref.GetOpSaldo(), ref.GetSumme()}};
        PrintLineHeading(ClientArea, idx++, 10150);
        idx = PrintList(ClientArea, tmp2, idx);
    } else if (page == 1) {
        if (!info) {
            PrintLineHeading(ClientArea, 0, 10152);
        }
        SLONG idx = 1;
        PrintLineWithValueMio(ClientArea, idx++, 3300, player.Money);
        PrintLineWithValueMio(ClientArea, idx++, 3301, player.Credit);
        ++idx;

        if (Sim.Players.Players[PlayerNum].HasBerater(berater) < 90) {
            PrintLine(ClientArea, idx, textBeraterZuSchlecht);
            return;
        }

        std::vector<std::tuple<SLONG, __int64, __int64>> tmp = {
            {3401, ref.HabenZinsen, 0}, {3402, ref.HabenRendite, 0}, {3501, ref.SollZinsen, 0}, {3502, ref.SollRendite, 0}, {10006, ref.Steuer, 0}};
        PrintLineHeading(ClientArea, idx++, 10153);
        idx = PrintList(ClientArea, tmp, idx);

        ++idx;
        auto summe = ref.HabenZinsen + ref.HabenRendite + ref.SollZinsen + ref.SollRendite + ref.Steuer;
        tmp = {{3604, summe, 0}, {10103, -summe, ref.GetOpGewinn()}};
        idx = PrintList(ClientArea, tmp, idx);
    }
}

void BLOCK::ZeigeTagesBilanz(XY ClientArea, const PLAYER & /*player*/, const CBilanz &ref, bool info, SLONG page) {
    SLONG berater = BERATERTYP_GELD;
    SLONG textKeinBerater = 3000;
    SLONG textBeraterZuSchlecht = 3001;
    if (info) {
        berater = BERATERTYP_INFO;
        textKeinBerater = 3002;
        textBeraterZuSchlecht = 3003;
    }

    if (Sim.Players.Players[PlayerNum].HasBerater(berater) == 0) {
        PrintLine(ClientArea, 2, textKeinBerater);
        return;
    }

    if (!info) {
        PrintLineHeading(ClientArea, 0, 10050 + page);
    }
    if (page == 0) {
        std::vector<std::pair<SLONG, __int64>> tmp = {{3601, ref.GetHaben()}, {3602, ref.GetSoll()}};
        PrintList(ClientArea, tmp, 1);
    } else if (page == 1) {
        std::vector<std::pair<SLONG, __int64>> tmp = {{3403, ref.Tickets},         {3404, ref.Auftraege},      {10006, ref.FrachtAuftraege},
                                                      {10000, ref.KerosinFlug},    {10001, ref.KerosinVorrat}, {10002, ref.Essen},
                                                      {3505, ref.Vertragsstrafen}, {3506, ref.Wartung},        {10003, ref.FlugzeugUmbau}};
        PrintList(ClientArea, tmp, 1);
    } else if (page == 2) {
        if (Sim.Players.Players[PlayerNum].HasBerater(berater) < 30) {
            PrintLine(ClientArea, 2, textBeraterZuSchlecht);
            return;
        }
        std::vector<std::pair<SLONG, __int64>> tmp = {{3504, ref.Personal}, {3507, ref.Gatemiete}, {10034, ref.Citymiete}, {10035, ref.Routenmiete}};
        PrintList(ClientArea, tmp, 1);
    } else if (page == 3) {
        if (Sim.Players.Players[PlayerNum].HasBerater(berater) < 40) {
            PrintLine(ClientArea, 2, textBeraterZuSchlecht);
            return;
        }
        std::vector<std::pair<SLONG, __int64>> tmp = {{10036, ref.HabenZinsen}, {10038, ref.HabenRendite},  {10004, ref.KreditNeu}, {10037, ref.SollZinsen},
                                                      {10039, ref.SollRendite}, {10005, ref.KreditTilgung}, {10006, ref.Steuer}};
        PrintList(ClientArea, tmp, 1);
    } else if (page == 4) {
        if (Sim.Players.Players[PlayerNum].HasBerater(berater) < 50) {
            PrintLine(ClientArea, 2, textBeraterZuSchlecht);
            return;
        }
        std::vector<std::pair<SLONG, __int64>> tmp = {{10020, ref.Aktienverkauf}, {10021, ref.AktienEmission},    {10022, ref.AktienEmissionKompErh},
                                                      {10023, ref.Aktienkauf},    {10024, ref.AktienEmissionFee}, {10025, ref.AktienEmissionKompGez}};
        PrintList(ClientArea, tmp, 1);
    } else if (page == 5) {
        if (Sim.Players.Players[PlayerNum].HasBerater(berater) < 60) {
            PrintLine(ClientArea, 2, textBeraterZuSchlecht);
            return;
        }
        std::vector<std::pair<SLONG, __int64>> tmp = {{10026, ref.FlugzeugVerkauf},  {10027, ref.Takeovers},     {10028, ref.FlugzeugKauf},
                                                      {10029, ref.FlugzeugUpgrades}, {10030, ref.ExpansionCity}, {10031, ref.ExpansionRouten},
                                                      {10032, ref.ExpansionGates},   {10033, ref.ExpansionTanks}};
        PrintList(ClientArea, tmp, 1);
    } else if (page == 6) {
        if (Sim.Players.Players[PlayerNum].HasBerater(berater) < 70) {
            PrintLine(ClientArea, 2, textBeraterZuSchlecht);
            return;
        }
        std::vector<std::pair<SLONG, __int64>> tmp = {
            {10007, ref.SabotageGeklaut}, {10008, ref.SabotageKomp}, {10009, ref.Sabotage}, {10010, ref.SabotageStrafe}, {10011, ref.SabotageSchaden}};
        PrintList(ClientArea, tmp, 1);
    } else if (page == 7) {
        if (Sim.Players.Players[PlayerNum].HasBerater(berater) < 70) {
            PrintLine(ClientArea, 2, textBeraterZuSchlecht);
            return;
        }
        std::vector<std::pair<SLONG, __int64>> tmp = {{10012, ref.BodyguardRabatt}, {10013, ref.GeldErhalten},    {10014, ref.SonstigeEinnahmen},
                                                      {10015, ref.PanneSchaden},    {10016, ref.SecurityKosten},  {10017, ref.WerbeKosten},
                                                      {10018, ref.GeldGeschickt},   {10019, ref.SonstigeAusgaben}};
        PrintList(ClientArea, tmp, 1);
    }
}

void BLOCK::ZeigeInformantenFinanzBericht(XY ClientArea, SLONG page) {
    assert(page >= 0 && page < 6);
    if (Sim.Players.Players[PlayerNum].HasBerater(BERATERTYP_INFO) == 0) {
        PrintLine(ClientArea, 2, 3002);
        return;
    }

    SLONG c = page / 2;
    c += static_cast<SLONG>(c >= PlayerNum);
    if (Sim.Players.Players[c].IsOut != 0) {
        PrintLine(ClientArea, 2, 10200);
        return;
    }
    PrintLine(ClientArea, 0, Sim.Players.Players[c].AirlineX);
    ZeigeFinanzBericht(ClientArea, Sim.Players.Players[c], Sim.Players.Players[c].BilanzWoche.Hole(), true, page % 2);
}

void BLOCK::ZeigeInformantenBilanz(XY ClientArea, SLONG playerId, SLONG page) {
    assert(page >= 0 && page < 8);
    if (Sim.Players.Players[PlayerNum].HasBerater(BERATERTYP_INFO) == 0) {
        PrintLine(ClientArea, 2, 3002);
        return;
    }

    if (Sim.Players.Players[playerId].IsOut != 0) {
        PrintLine(ClientArea, 2, 10200);
        return;
    }
    PrintLine(ClientArea, 0, Sim.Players.Players[playerId].AirlineX);
    ZeigeTagesBilanz(ClientArea, Sim.Players.Players[playerId], Sim.Players.Players[playerId].BilanzWoche.Hole(), true, page);
}

void BLOCK::ZeigeInformantenInfos(XY ClientArea, SLONG /*page*/) const {
    if (Sim.Players.Players[PlayerNum].HasBerater(BERATERTYP_INFO) == 0) {
        PrintLine(ClientArea, 2, 3002);
        return;
    }

    TEAKRAND rnd;
    rnd.SRand(Sim.Date);
    SLONG idx = 1;
    for (SLONG c = 0; c < Sim.Players.Players.AnzEntries(); c++) {
        auto &qPlayer = Sim.Players.Players[c];
        if (qPlayer.IsOut != 0 || c == PlayerNum) {
            continue;
        }

        if ((qPlayer.TargetedPlayer != 0) && rnd.Rand(2) == 0) {
            PrintLine(ClientArea, idx, bprintf(StandardTexte.GetS(TOKEN_EXPERT, 10201), (LPCTSTR)qPlayer.AirlineX));
        } else if ((qPlayer.WantToDoRoutes != 0) && (qPlayer.DoRoutes == 0)) {
            PrintLine(ClientArea, idx, bprintf(StandardTexte.GetS(TOKEN_EXPERT, 10204), (LPCTSTR)qPlayer.AirlineX));
        } else if ((qPlayer.SavesForPlane != 0) && rnd.Rand(2) == 0) {
            PrintLine(ClientArea, idx, bprintf(StandardTexte.GetS(TOKEN_EXPERT, 10203), (LPCTSTR)qPlayer.AirlineX));
        } else if ((qPlayer.OutOfGates != 0) && rnd.Rand(2) == 0) {
            PrintLine(ClientArea, idx, bprintf(StandardTexte.GetS(TOKEN_EXPERT, 10202), (LPCTSTR)qPlayer.AirlineX));
        }
        idx += 3;
    }
}

void BLOCK::KerosinQualiOptimierung(XY ClientArea, SLONG idx, DOUBLE qualiZiel, SLONG txtId, SLONG txtId2, SLONG beraterSchwelle) const {
    const auto &ref = Sim.Players.Players[PlayerNum];
    SLONG menge = 0;
    if (ref.KerosinQuali > qualiZiel) {
        assert(qualiZiel > 0);
        menge = SLONG(std::ceil((ref.TankInhalt * (ref.KerosinQuali - qualiZiel)) / qualiZiel));
    } else {
        assert(qualiZiel < 2);
        menge = SLONG(std::ceil((ref.TankInhalt * (qualiZiel - ref.KerosinQuali)) / (2.0 - qualiZiel)));
    }
    menge = std::min(menge, ref.Tank - ref.TankInhalt);
    if (menge > 0) {
        if (Sim.Players.Players[PlayerNum].HasBerater(BERATERTYP_KEROSIN) < beraterSchwelle) {
            PrintLine(ClientArea, idx, txtId);
        } else {
            PrintLine(ClientArea, idx, bprintf(StandardTexte.GetS(TOKEN_EXPERT, txtId2), menge));
        }
    }
}
void BLOCK::ZeigeKerosinberater(XY ClientArea, SLONG page) {
    const auto &ref = Sim.Players.Players[PlayerNum];
    auto quali = SLONG(std::floor(ref.KerosinQuali * 3.0));

    PrintLineHeading(ClientArea, 0, 10350);

    SLONG idx = 1;
    if (page == 0) {
        PrintLine(ClientArea, idx, 10300);
        PrintLineAlignRight(ClientArea, idx++, bprintf("%li / %li", ref.TankInhalt, ref.Tank));
        PrintLineAlignRight(ClientArea, idx++, CalcPercentage(ref.TankInhalt, ref.Tank));

        PrintLineWithValueMio(ClientArea, idx++, 10303, ref.TankInhalt * ref.TankPreis);

        PrintLine(ClientArea, idx, 10302);
        PrintLineAlignRight(ClientArea, idx++, bitoa(SLONG(std::round(ref.TankPreis))));

        PrintLine(ClientArea, idx, 10301);
        PrintLineAlignRight(ClientArea, idx++, bitoa(Sim.Kerosin));

        if (Sim.Players.Players[PlayerNum].HasBerater(BERATERTYP_KEROSIN) < 30) {
            PrintLine(ClientArea, idx, 3004);
            return;
        }

        if (ref.TankInhalt <= 0) {
            PrintLine(ClientArea, idx, 10304);
            PrintLineAlignRight(ClientArea, idx++, "---");
            PrintLine(ClientArea, idx, 10312);
            PrintLineAlignRight(ClientArea, idx++, "---");
        } else {
            // Qualität im Tank
            PrintLineWithPercentage(ClientArea, idx++, 10304, std::floor((2 - ref.KerosinQuali) * 1000), 1000);
            PrintLineAlignRight(ClientArea, idx++, 10305 + quali);
        }
    } else if (page == 1) {
        const CBilanz stat[3] = {ref.BilanzGestern, ref.BilanzWoche.Hole(), ref.BilanzGesamt};
        for (SLONG i = 0; i < 3; i++) {
            PrintLineWithValueT(ClientArea, idx++, 10314 + i, stat[i].KerosinGespart);

            if (Sim.Players.Players[PlayerNum].HasBerater(BERATERTYP_KEROSIN) < 40 + 10 * i) {
                PrintLine(ClientArea, idx, 3004);
                return;
            }
            ++idx;
        }

        ++idx;
        if (ref.KerosinQuali > 5.0 / 3.0) {
            // Kaufe x Barrel teuer, um Qualität zu erhöhen
            KerosinQualiOptimierung(ClientArea, idx, 5.0 / 3.0, 10317, 10318, 70);
        } else if (ref.KerosinQuali > 4.0 / 3.0) {
            // Kaufe x Barrel teuer, um Qualität zu erhöhen
            KerosinQualiOptimierung(ClientArea, idx, 4.0 / 3.0, 10319, 10320, 80);
        } else if (ref.KerosinQuali < 1.0) {
            // Kaufe x Barrel billig, um Qualität auf reduzieren
            KerosinQualiOptimierung(ClientArea, idx, 1.0, 10321, 10322, 90);
        } else if (ref.KerosinQuali < 4.0 / 3.0) {
            // Kaufe x Barrel billig, um Qualität auf reduzieren
            KerosinQualiOptimierung(ClientArea, idx, 4.0 / 3.0, 10323, 10324, 100);
        }
    }
}

//--------------------------------------------------------------------------------------------
// Geht eine Seite weiter:
//--------------------------------------------------------------------------------------------
void BLOCK::GotoNext() {}

//--------------------------------------------------------------------------------------------
// Geht eine Seite zurück:
//--------------------------------------------------------------------------------------------
void BLOCK::GotoPrevious() {}

//--------------------------------------------------------------------------------------------
// Erneuert den PageSize Eintrag:
//--------------------------------------------------------------------------------------------
void BLOCK::UpdatePageSize() {
    PageSize = 1;
    PageSizeB = 1;
    AnzPages = 1;
    AnzPagesB = 1;

    PLAYER &qPlayer = Sim.Players.Players[PlayerNum];
    if (Index == 1) {
        if (BlockType != 2) {
            PageSize = 13;
        } else {
            PageSize = 6;
        }
        AnzPages = max(0, (Table.AnzRows - 1) / PageSize) + 1;
    } else {
        switch (BlockType) {
        case 1:
            // Städte
            AnzPages = 1 + Cities[SelectedId].AnzTexts + Cities[SelectedId].AnzPhotos;
            break;
        case 2:
            // Aufträge
            AnzPages = 4;
            if (qPlayer.Planes[SelectedId].TypeId != -1) {
                AnzPages += PlaneTypes[qPlayer.Planes[SelectedId].TypeId].AnzPhotos;
            }
            break;
        case 5:
            // Berater
            switch (SelectedId) {
            case 1:
                // Geschäftsbericht
                AnzPages = 2;
                break;
            case 2:
            case 3:
            case 4:
                // Einnahmen Tag / Woche / Gesamt
                AnzPages = 8;
                break;
            case 5:
                // Flugzeugeinnahmen
                PageSize = 13;
                AnzPages = max(0, (Table.AnzRows - 1) / PageSize) + 2;
                break;
            case 7:
                // Informant, Übersicht
                AnzPages = 2 * 3;
                break;
            case 8:
            case 9:
            case 10:
                // Informant, Einnahmen Tag / Woche / Gesamt
                AnzPages = 8;
                break;
            case 11:
                // Kerosinberater
                AnzPages = 2;
                break;
            default:
                AnzPages = 1;
            }

            break;
        default:
            hprintf("Block.cpp: Default case should not be reached.");
            DebugBreak();
        }
    }

    if (BlockTypeB == 3 || BlockTypeB == 4 || BlockTypeB == 6) {
        if (IndexB == 1) {
            PageSizeB = 6;
            AnzPagesB = max(0, (TableB.AnzRows - 1) / PageSizeB) + 1;
        } else if (BlockTypeB == 4) {
            AnzPagesB = 3;
        } else {
            AnzPagesB = 2;
        }
    }
}

//--------------------------------------------------------------------------------------------
// Speichert ein BLOCK-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator<<(TEAKFILE &File, const BLOCK &b) {
    File << b.Table << b.TableB;
    File << b.ScreenPos << b.Destructing;
    File << b.PlayerNum << b.StyleType;
    File << b.BlockType << b.SelectedId << b.Index;
    File << b.BlockTypeB << b.SelectedIdB << b.IndexB;
    File << b.Page << b.AnzPages << b.Tip << b.TipInUse;
    File << b.PageB << b.AnzPagesB << b.TipB << b.TipInUseB;
    File << b.PageSize << b.PageSizeB;

    File << b.Indexes;
    File << b.SelectedIds;
    File << b.Pages;

    if (SaveVersion == 1 && SaveVersionSub >= 11) {
        File << b.DoubleBlock;
    }

    return (File);
}

//--------------------------------------------------------------------------------------------
// Lädt ein BLOCK-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator>>(TEAKFILE &File, BLOCK &b) {
    File >> b.Table >> b.TableB;
    File >> b.ScreenPos >> b.Destructing;
    File >> b.PlayerNum >> b.StyleType;
    File >> b.BlockType >> b.SelectedId >> b.Index;
    File >> b.BlockTypeB >> b.SelectedIdB >> b.IndexB;
    File >> b.Page >> b.AnzPages >> b.Tip >> b.TipInUse;
    File >> b.PageB >> b.AnzPagesB >> b.TipB >> b.TipInUseB;
    File >> b.PageSize >> b.PageSizeB;

    /*if (SaveVersion == 1 && SaveVersionSub < 100) {
        File.Read((UBYTE *)b.Indexes, sizeof(b.Indexes[0]) * 5);
        File.Read(reinterpret_cast<UBYTE *>(b.SelectedIds), sizeof(b.SelectedIds[0]) * 5);
        File.Read(reinterpret_cast<UBYTE *>(b.Pages), sizeof(b.Pages[0]) * 5);

        b.Indexes[5] = 1U;
    } else {*/
    File >> b.Indexes;
    File >> b.SelectedIds;
    File >> b.Pages;
    //}

    if (SaveVersion == 1 && SaveVersionSub >= 11) {
        File >> b.DoubleBlock;
    }

    return (File);
}

//--------------------------------------------------------------------------------------------
// Speichert ein BLOCKS-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator<<(TEAKFILE &File, const BLOCKS &b) {
    File << b.RepaintAll;
    File << *((const ALBUM_V<BLOCK> *)&b);

    return (File);
}

//--------------------------------------------------------------------------------------------
// Lädt ein BLOCKS-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator>>(TEAKFILE &File, BLOCKS &b) {
    File >> b.RepaintAll;
    File >> *((ALBUM_V<BLOCK> *)&b);

    return (File);
}
