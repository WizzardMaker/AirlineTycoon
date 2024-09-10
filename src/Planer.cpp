//============================================================================================
// Planer.cpp : Die Basisklasse für den Planungsbildschirm Laptop/Filofax
//============================================================================================
// Link: "Planer.h"
//============================================================================================
#include "StdAfx.h"
#include "glglobe.h"

#define RDTSC __asm _emit 0x0F __asm _emit 0x31

#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

DOUBLE a = 2.65;
static XY GlobeOffset[] = {XY(141, 16), XY(177, 29)};

extern SLONG FoodCosts[];
extern SB_CFont FontVerySmall;

//--------------------------------------------------------------------------------------------
// Malt den Erdball (Groß oder Klein) an eine Stelle
//--------------------------------------------------------------------------------------------
void PaintGlobe(const TECBM &SourceBm, SBBM *TargetBm, UWORD EarthAlpha, const XY &Offset) {
    TECBMKEYC SrcKey(SourceBm);
    SB_CBitmapKey TgtKey(*TargetBm->pBitmap);

    const UBYTE *s = nullptr; // Source Bitmap Pointer
    UWORD *t = nullptr;       // TargetBitmap Pointer

    UWORD *map = nullptr;
    UBYTE *lightmap = nullptr;

    SLONG cx = 0;
    SLONG cy = 0;

    static SLONG a;
    a = (EarthAlpha >> 7);
#ifdef ENABLE_ASM
    static ULONG _esp;
#endif

    for (cy = 0; cy < GlobeMapper.AnzEntries() - 1; cy++) {
        // oben:
        s = (const_cast<UBYTE *>(SrcKey.Bitmap)) + GlobeMapperY[cy] * SrcKey.lPitch;
        t = (static_cast<UWORD *>(TgtKey.Bitmap)) + (cy + 5 + Offset.y) * (TgtKey.lPitch >> 1) + Offset.x + ((369 - GlobeMapper[cy].AnzEntries()) / 2);

        SLONG anz = GlobeMapper[cy].AnzEntries();

        map = GlobeMapper[cy].getData();
        lightmap = GlobeLight[cy].getData();

#ifndef ENABLE_ASM
        for (cx = anz - 1; cx >= 0; cx--) {
            t[cx] = GlobeMixTab[s[(map[cx] + a) & 511] + (lightmap[cx] << 8)];
        }
#else
        __asm {
            push  ebp
                push  esi
                push  edi

                mov   ecx, anz

                mov   esi, s
                mov   edi, t
                mov   eax, GlobeMixTab.DelPointer
                mov   edx, lightmap
                mov   ebp, map

                sub   ebp, 2
                sub   edi, 2
                dec   edx

                mov   _esp, esp
                mov   esp, eax
                xor   eax, eax

                looping:
                mov   ebx, a
                add    bx, WORD PTR [ebp+ecx*2]
                mov    ah, BYTE PTR [edx+ecx]
                and   ebx, 511
                mov    al, BYTE PTR [esi+ebx]

                mov    bx, WORD PTR [eax*2+esp]
                dec   ecx
                mov   WORD PTR [edi+ecx*2], bx
                jnz   looping

                mov   esp, _esp

                pop   edi
                pop   esi
                pop   ebp
        }
#endif
        // unten:
        if (GlobeMapperY[368 - cy] < SourceBm.Size.y) {
            s = (const_cast<UBYTE *>(SrcKey.Bitmap)) + GlobeMapperY[368 - cy] * SrcKey.lPitch;
            t = (static_cast<UWORD *>(TgtKey.Bitmap)) + ((368 - cy) + 4 + Offset.y) * (TgtKey.lPitch >> 1) + Offset.x +
                ((369 - GlobeMapper[cy].AnzEntries()) / 2);

            anz = GlobeMapper[cy].AnzEntries();

            map = GlobeMapper[cy].getData();
            lightmap = GlobeLight[368 - cy].getData();

#ifndef ENABLE_ASM
            for (cx = anz - 1; cx >= 0; cx--) {
                t[cx] = GlobeMixTab[s[(map[cx] + a) & 511] + (lightmap[cx] << 8)];
            }
#else
            __asm {
                push  ebp
                    push  esi
                    push  edi

                    mov   ecx, anz

                    mov   esi, s
                    mov   edi, t
                    mov   eax, GlobeMixTab.DelPointer
                    mov   edx, lightmap
                    mov   ebp, map

                    sub   ebp, 2
                    sub   edi, 2
                    dec   edx

                    mov   _esp, esp
                    mov   esp, eax
                    xor   eax, eax

                    looping2:
                    mov   ebx, a
                    add    bx, WORD PTR [ebp+ecx*2]
                    mov    ah, BYTE PTR [edx+ecx]
                    and   ebx, 511
                    mov    al, BYTE PTR [esi+ebx]

                    mov    bx, WORD PTR [eax*2+esp]
                    dec   ecx
                    mov   WORD PTR [edi+ecx*2], bx
                    jnz   looping2

                    mov   esp, _esp

                    pop   edi
                    pop   esi
                    pop   ebp
            }
#endif
        }
    }
}

//--------------------------------------------------------------------------------------------
// Konstruktor:
//--------------------------------------------------------------------------------------------
CPlaner::CPlaner(BOOL bHandy, ULONG PlayerNum, UWORD &EarthAlpha, BOOL IsLaptop) : CStdRaum(bHandy, PlayerNum, "", 0), EarthAlpha(EarthAlpha) {
    SLONG c = 0;

    // if (!bHandy) AmbientManager.SetGlobalVolume (50);

    FensterVisible = FALSE;

    for (c = 0; c < Sim.Players.AnzPlayers; c++) {
        if (((Sim.Players.Players[static_cast<SLONG>(PlayerNum)].DisplayPlanes[c] != 0U) && (IsLaptop != 0)) ||
            (c == static_cast<SLONG>(PlayerNum) && (IsLaptop == 0))) {
            Sim.Players.Players[c].Planes.UpdateGlobePos(EarthAlpha);
        }
    }

    // Ist inzwischen ein Blockeintrag ungültig geworden?
    for (c = Sim.Players.Players[static_cast<SLONG>(PlayerNum)].Blocks.AnzEntries() - 1; c >= 0; c--) {
        if (Sim.Players.Players[static_cast<SLONG>(PlayerNum)].Blocks.IsInAlbum(ULONG(c)) != 0) {
            BLOCK &qBlock = Sim.Players.Players[static_cast<SLONG>(PlayerNum)].Blocks[c];

            qBlock.Tip = 0;
            qBlock.TipB = 0;
            qBlock.TipInUse = 0;
            qBlock.TipInUseB = 0;

            // Flugzeugdetail?
            if (qBlock.BlockType == 2 && qBlock.Index != 1) {
                if (Sim.Players.Players[static_cast<SLONG>(PlayerNum)].Planes.IsInAlbum(qBlock.SelectedId) == 0) {
                    qBlock.Index = 1;
                    qBlock.Page = 0;
                }
            }

            // Städtedetail? Zurückschalten:
            if (qBlock.BlockType == 1 && qBlock.Index != 1) {
                qBlock.Index = 1;
                qBlock.Page = 0;
            }

            if (qBlock.DoubleBlock != 0) {
                // Auftragsdetails:
                if (qBlock.BlockTypeB == 3 && qBlock.IndexB != 1) {
                    if (Sim.Players.Players[static_cast<SLONG>(PlayerNum)].Auftraege.IsInAlbum(qBlock.SelectedIdB) == 0) {
                        qBlock.IndexB = 1;
                        qBlock.PageB = 0;
                    }
                }

                // Routendetails:
                if (qBlock.BlockTypeB == 4 && qBlock.IndexB != 1) {
                    if (Routen.IsInAlbum(qBlock.SelectedIdB) == 0) {
                        qBlock.IndexB = 1;
                        qBlock.PageB = 0;
                    } else if (Sim.Players.Players[static_cast<SLONG>(PlayerNum)].RentRouten.RentRouten[Routen(qBlock.SelectedIdB)].Rang == 0) {
                        qBlock.IndexB = 1;
                        qBlock.PageB = 0;
                    }
                }
            }
        }
    }

    gMouseLButtonDownTimer = 0;

    CPlaner::IsLaptop = IsLaptop;
    CPlaner::UsedToRotate = 0;

    GlobeBm.ReSize(381, 381);
    GlobeBm.Clear(0);
}

//--------------------------------------------------------------------------------------------
// Sehr destruktiv!
//--------------------------------------------------------------------------------------------
CPlaner::~CPlaner() { PostItBm.Destroy(); }

//--------------------------------------------------------------------------------------------
// Malt den Erdball (Groß oder Klein) an eine Stelle
//--------------------------------------------------------------------------------------------
void CPlaner::PaintGlobe() { ::PaintGlobe(EarthBm, &GlobeBm, EarthAlpha, XY(0, 0)); }

//--------------------------------------------------------------------------------------------
// malt Dinge auf den Globus drauf:
//--------------------------------------------------------------------------------------------
void CPlaner::PaintGlobeRoutes() {
    SLONG c = 0;
    SLONG d = 0;
    SLONG e = 0;

    for (c = 0; c < Sim.Players.AnzPlayers; c++) {
        if (Sim.Players.Players[c].IsOut != 0) {
            continue;
        }
        if (Sim.Players.Players[PlayerNum].DisplayRoutes[c] == 0U) {
            continue;
        }
        // Die gemieteten Niederlassungen anzeigen:
        for (d = 0; d < Cities.AnzEntries(); d++) {
            if ((IsLaptop == 0) || Sim.Players.Players[c].RentCities.RentCities[d].Rang == 0) {
                continue;
            }

            XY pxy;
            XY tmp = Cities[d].GlobusPosition;

            if (EarthProjectize(tmp, EarthAlpha, &pxy) == 0) {
                SLONG f = 255 - abs(32768 - (SLONG(((tmp.x + 180) * 65536 / 360) - EarthAlpha + 16384 + 65536) & 65535)) / 64;
                if (f <= 0) {
                    f = 0;
                }

                SB_Hardwarecolor color = GlobeBm.pBitmap->GetHardwarecolor((c >= 2) ? f : 0,
                                                                           (c == 1 || c == 3) ? f : 0,
                                                                           (c == 0) ? f : 0);


                GlobeBm.pBitmap->SetPixel(pxy.x - 6, pxy.y, color);
                GlobeBm.pBitmap->SetPixel(pxy.x - 6 - 1, pxy.y, color);
                GlobeBm.pBitmap->SetPixel(pxy.x - 6 + 1, pxy.y, color);
                GlobeBm.pBitmap->SetPixel(pxy.x - 6, pxy.y - 1, color);
                GlobeBm.pBitmap->SetPixel(pxy.x - 6, pxy.y + 1, color);
                GlobeBm.pBitmap->SetPixel(pxy.x - 6 - 1, pxy.y - 1, color);
                GlobeBm.pBitmap->SetPixel(pxy.x - 6 + 1, pxy.y + 1, color);
                GlobeBm.pBitmap->SetPixel(pxy.x - 6 + 1, pxy.y - 1, color);
                GlobeBm.pBitmap->SetPixel(pxy.x - 6 - 1, pxy.y + 1, color);
            }
        }

        // Die gemieteten Routen anzeigen:
        if (2U != Sim.Players.Players[PlayerNum].DisplayRoutes[c]) {
            continue;
        }
        for (d = 0; d < Sim.Players.Players[c].RentRouten.RentRouten.AnzEntries(); d++) {
            if ((IsLaptop == 0) || Sim.Players.Players[c].RentRouten.RentRouten[d].Rang == 0) {
                continue;
            }
            if (Routen[d].VonCity >= Routen[d].NachCity) {
                continue;
            }
            XY von;
            XY nach;
            XY pxy;
            XY lastpxy;
            FXY tmp;

            SLONG n = 0;
            SLONG dist = Cities.CalcDistance(Routen[d].VonCity, Routen[d].NachCity);

            if (dist < 500000) {
                n = 256;
            } else if (dist < 1000000) {
                n = 128;
            } else if (dist < 2000000) {
                n = 64;
            } else if (dist < 5000000) {
                n = 32;
            } else {
                n = 16;
            }

            von = Cities[Routen[d].VonCity].GlobusPosition;
            nach = Cities[Routen[d].NachCity].GlobusPosition;

            for (e = 0; e <= 256; e += n) {
                lastpxy = pxy;

                if (abs(nach.x - von.x) < 180) {
                    tmp.x = FLOAT((von.x * (256 - e) + nach.x * e) / 256.0);
                } else if (nach.x > von.x) {
                    tmp.x = FLOAT(von.x + (-(360 - (nach.x - von.x)) * e) / 256.0);
                } else {
                    tmp.x = FLOAT(von.x + ((360 - (von.x - nach.x)) * e) / 256.0);
                }

                tmp.y = FLOAT((von.y * (256 - e) + nach.y * e) / 256.0);

                if ((EarthProjectize(tmp, EarthAlpha, &pxy) == 0) && e > 0) {
                    SLONG f = 255 - abs(32768 - (SLONG(((tmp.x + 180) * 65536 / 360) - EarthAlpha + 16384 + 65536) & 65535)) / 64;
                    if (f <= 0) {
                        f = 0;
                    }

                    SB_Hardwarecolor color = GlobeBm.pBitmap->GetHardwarecolor((c >= 2) ? f : 0,
                                                                               (c == 1 || c == 3) ? f : 0,
                                                                               (c == 0) ? f : 0);

                    GlobeBm.pBitmap->Line(pxy.x - 6, pxy.y, lastpxy.x - 6, lastpxy.y, color);
                }
            }
        }
    }

    for (c = 0; c < Sim.Players.Players[PlayerNum].Blocks.AnzEntries(); c++) {
        if (IsLaptop == 0 && (FensterVisible == 0)) {
            continue;
        }
        if ((Sim.Players.Players[PlayerNum].Blocks.IsInAlbum(c) == 0)) {
            continue;
        }
        if ((c == 0) != (IsLaptop == 0)) {
            continue;
        }
        if (Sim.Players.Players[PlayerNum].Blocks[c].Index == 1 || Sim.Players.Players[PlayerNum].Blocks[c].BlockType != 2) {
            continue;
        }

        CFlugplan &qPlan = Sim.Players.Players[PlayerNum].Planes[Sim.Players.Players[PlayerNum].Blocks[c].SelectedId].Flugplan;

        for (d = 0; d < qPlan.Flug.AnzEntries(); d++) {
            if (qPlan.Flug[d].ObjectType == 0) {
                continue;
            }

            XY von;
            XY nach;
            XY pxy;
            XY lastpxy;
            FXY tmp;

            SLONG n = 0;
            SLONG dist = Cities.CalcDistance(qPlan.Flug[d].VonCity, qPlan.Flug[d].NachCity);

            if (dist < 500000) {
                n = 256;
            } else if (dist < 1000000) {
                n = 128;
            } else if (dist < 2000000) {
                n = 64;
            } else if (dist < 5000000) {
                n = 32;
            } else {
                n = 16;
            }

            von = Cities[qPlan.Flug[d].VonCity].GlobusPosition;
            nach = Cities[qPlan.Flug[d].NachCity].GlobusPosition;

            for (e = 0; e <= 256; e += n) {
                lastpxy = pxy;

                if (abs(nach.x - von.x) < 180) {
                    tmp.x = FLOAT((von.x * (256 - e) + nach.x * e) / 256.0);
                } else if (nach.x > von.x) {
                    tmp.x = FLOAT(von.x + (-(360 - (nach.x - von.x)) * e) / 256.0);
                } else {
                    tmp.x = FLOAT(von.x + ((360 - (von.x - nach.x)) * e) / 256.0);
                }

                tmp.y = FLOAT((von.y * (255 - e) + nach.y * e) / 256.0);

                if ((EarthProjectize(tmp, EarthAlpha, &pxy) == 0) && e > 0) {
                    SLONG f = 255 - abs(32768 - (SLONG(((tmp.x + 180) * 65536 / 360) - EarthAlpha + 16384 + 65536) & 65535)) / 64;
                    if (f <= 0) {
                        f = 0;
                    }

                    SB_Hardwarecolor color = GlobeBm.pBitmap->GetHardwarecolor((c >= 2) ? f : 0,
                                                                               (c == 1 || c == 3) ? f : 0,
                                                                               (c == 0) ? f : 0);

                    GlobeBm.pBitmap->Line(pxy.x - 6, pxy.y, lastpxy.x - 6, lastpxy.y, color);
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Malt die Post-It Notiz
//--------------------------------------------------------------------------------------------
void CPlaner::PaintPostIt() {
    CString VonCity;
    CString NachCity;
    CString VonCityKuerzel;
    CString NachCityKuerzel;
    SLONG VonCityId = 0;
    SLONG NachCityId = 0;
    SLONG Dauer = 0;
    SLONG Speed = 0;
    SLONG c = 0;
    SLONG i = 0;

    if (CurrentPostItType == 2) // Auftrag
    {
        VonCityId = Sim.Players.Players[PlayerNum].Auftraege[CurrentPostItId].VonCity;
        NachCityId = Sim.Players.Players[PlayerNum].Auftraege[CurrentPostItId].NachCity;
    } else if (CurrentPostItType == 4) // Frachtauftrag
    {
        VonCityId = Sim.Players.Players[PlayerNum].Frachten[CurrentPostItId].VonCity;
        NachCityId = Sim.Players.Players[PlayerNum].Frachten[CurrentPostItId].NachCity;
    } else if (CurrentPostItType == 1) // Route
    {
        VonCityId = Routen[CurrentPostItId].VonCity;
        NachCityId = Routen[CurrentPostItId].NachCity;
    }

    if ((pBlock != nullptr) && (pBlock->Index == 0 || pBlock->Index == 2) && pBlock->BlockType == 2) {
        Speed = Sim.Players.Players[PlayerNum].Planes[pBlock->SelectedId].ptGeschwindigkeit;
        // Speed=PlaneTypes[Sim.Players.Players[(SLONG)PlayerNum].Planes[pBlock->SelectedId].TypeId].Geschwindigkeit;
    } else {
        Speed = 700;
    }

    VonCityKuerzel = Cities[VonCityId].Kuerzel;
    NachCityKuerzel = Cities[NachCityId].Kuerzel;
    VonCity = Cities[VonCityId].Name;
    NachCity = Cities[NachCityId].Name;
    Dauer = Cities.CalcFlugdauer(VonCityId, NachCityId, Speed);
    // Dauer    = (Cities.CalcDistance (VonCityId, NachCityId)/Speed+999)/1000+1+2;

    Dauer++; // Wegen anderer Schleifenart

    /*if (pBlock && pBlock->Index==2 && pBlock && pBlock->BlockType==2)
      {
      PostItBm.ReSize (FlugplanBms[21].Size.x, FlugplanBms[21].Size.y*Dauer);

      for (c=0; c<Dauer; c++)
      {
      SLONG px = 0;
      SLONG py = c*6;

      if (c==0) i=1;
      else if (c==1) i=2;
      else if (c==Dauer-1) i=5;
      else if (c==Dauer-2) i=4;
      else i=3;

      if (CurrentPostItType==2) i+=5;

      PostItBm.BlitFromT (FlugplanBms[20+i], XY(px,py));

      if (Dauer<=6)
      {
      SB_CFont *pFont;
      XY        Offset(0,0);

      if (Dauer-1<5)
      {
      pFont=&FontVerySmall;
      Offset=XY(2,2);
      }
      else pFont=&FontSmallBlack;

      if (c==1)       PostItBm.PrintAt (VonCity, *pFont, TEC_FONT_LEFT, XY(px+2,py-5), XY(px+140,py+15));
      if (c==Dauer-1) PostItBm.PrintAt (NachCity, *pFont, TEC_FONT_LEFT, XY(px+2,py-2)+Offset, XY(px+140,py+15)+Offset);
      }
      else
      {
      if (c==2)       PostItBm.PrintAt (VonCity, FontSmallBlack, TEC_FONT_LEFT, XY(px+2,py-5), XY(px+140,py+15));
      if (c==Dauer-2) PostItBm.PrintAt (NachCity, FontSmallBlack, TEC_FONT_LEFT, XY(px+2,py-2), XY(px+140,py+15));
      }
      }
      }
      else
      { */
    PostItBm.ReSize(FlugplanBms[6].Size.x * Dauer, FlugplanBms[6].Size.y);

    for (c = 0; c < Dauer; c++) {
        SLONG px = c * 6;
        SLONG py = 0;

        if (c == 0) {
            i = 1;
        } else if (c == 1) {
            i = 2;
        } else if (c == Dauer - 1) {
            i = 5;
        } else if (c == Dauer - 2) {
            i = 4;
        } else {
            i = 3;
        }

        if (CurrentPostItType == 2) {
            i += 5;
        }
        if (CurrentPostItType == 4) {
            i += 19;
        }

        PostItBm.BlitFromT(FlugplanBms[i], XY(px, py));

        if (Dauer <= 6) {
            SB_CFont *pFont = nullptr;
            XY OffsetA(0, 0);
            XY OffsetB(0, 0);

            if (Dauer - 1 < 4) {
                pFont = &FontVerySmall;
                OffsetA = XY(0, 2);
                OffsetB = XY(2, 0);
            } else {
                pFont = &FontSmallBlack;
            }

            if (c == 3 || (c == 2 && (OffsetB.x != 0))) {
                PostItBm.PrintAt(VonCityKuerzel, *pFont, TEC_FONT_LEFT, OffsetA + XY(-18 + px + 2 + static_cast<SLONG>(c == 2) * 6, 1 + py),
                                 OffsetA + XY(px + 16 + 14 + static_cast<SLONG>(c == 2) * 6, py + 18));
            }
            if (c == Dauer - 1) {
                PostItBm.PrintAt(NachCityKuerzel, *pFont, TEC_FONT_RIGHT, XY(-18 - 14 + px, 2 + py + 8) + OffsetB, XY(px + 8 - 2, py + 6 + 15) + OffsetB);
            }
        } else {
            if (c == 4) {
                PostItBm.PrintAt(VonCityKuerzel, FontSmallBlack, TEC_FONT_LEFT, XY(-18 + px, 1 + py), XY(px + 16 + 14, py + 18));
            }
            if (c == Dauer - 2) {
                PostItBm.PrintAt(NachCityKuerzel, FontSmallBlack, TEC_FONT_RIGHT, XY(-18 - 14 + px, 2 + py + 8), XY(px + 8, py + 6 + 15));
            }
        }
    }
    /*}*/
}

//--------------------------------------------------------------------------------------------
// Malt Globus, Flugzeuge und CityMarker in den Screen:
//--------------------------------------------------------------------------------------------
void CPlaner::PaintGlobeInScreen(XY TargetPos) {
    PLAYER &qPlayer = Sim.Players.Players[PlayerNum];
    SLONG c = 0;
    SLONG d = 0;

    // Flugzeuge hinter Erde
    if (Sim.Players.Players[PlayerNum].LaptopVirus == 0 || Sim.Players.Players[PlayerNum].GetRoom() != ROOM_LAPTOP) {
        for (d = 0; d < Sim.Players.AnzPlayers; d++) {
            if (Sim.Players.Players[d].IsOut == 0) {
                if (((Sim.Players.Players[PlayerNum].DisplayPlanes[d] != 0U) && (IsLaptop != 0)) || (d == PlayerNum && (IsLaptop == 0))) {
                    PLAYER &qPlayer = Sim.Players.Players[d];

                    for (c = qPlayer.Planes.AnzEntries() - 1; c >= 0; c--) {
                        if (qPlayer.Planes.IsInAlbum(c) != 0)
                        // if ((c==0) == (IsLaptop==0) && FensterVisible)
                        {
                            CPlane &qPlane = qPlayer.Planes[c];

                            if (qPlane.BehindGlobe != 0) {
                                // RoomBm.BlitFromT ((MapPlaneBms[d])[SLONG(qPlane.GlobeAngle)], TargetPos+qPlane.GlobePos-XY(9+6,9)-XY(5,5));
                                RoomBm.BlitFromT((MapPlaneBms[d])[SLONG(qPlane.GlobeAngle)], TargetPos + qPlane.GlobePos - XY(9, 9) - XY(7, 10));
                            }
                        }
                    }
                }
            }
        }
    }

    // Marker hinter der Erde:
    for (c = qPlayer.Blocks.AnzEntries() - 1; c >= 0; c--) {
        if (qPlayer.Blocks.IsInAlbum(c) != 0) {
            if ((c == 0) == (IsLaptop == 0) && (FensterVisible != 0)) {
                BLOCK &qBlock = qPlayer.Blocks[c];
                if (qBlock.Index == 0 && qBlock.BlockType == 1) {
                    auto xx = UWORD(UWORD((Cities[qBlock.SelectedId].GlobusPosition.x + 180) * 65536 / 360) - EarthAlpha + 16384 - 1250);

                    if (!(xx > 16386 && xx < 49152)) {
                        auto py = SLONG(sin((Cities[qBlock.SelectedId].GlobusPosition.y * a) / 240.0 * (3.14159 / 2)) * 185 + 190);
                        auto px = SLONG(sin((xx / 32768.0 - 1) * (3.14159)) * sqrt(184 * 184 - (py - 190) * (py - 190)) + 190);

                        RoomBm.BlitFromT(gCityMarkerBm, TargetPos + XY(px - 18, py - 18));
                    }
                }
            }
        }
    }

    // Erde neu berechnen?
    if (UsedToRotate == 1) {
        UsedToRotate = 0;
        GlobeBm.Clear(0);
        PaintGlobe();
        PaintGlobeRoutes();
    }

    // Die Erde malen:
    RoomBm.BlitFromT(GlobeBm, TargetPos - XY(5, 5));

    // Flugzeuge vor Erde
    if (Sim.Players.Players[PlayerNum].LaptopVirus == 0 || Sim.Players.Players[PlayerNum].GetRoom() != ROOM_LAPTOP) {
        for (d = 0; d < Sim.Players.AnzPlayers; d++) {
            if (Sim.Players.Players[d].IsOut == 0) {
                if (((Sim.Players.Players[PlayerNum].DisplayPlanes[d] != 0U) && (IsLaptop != 0)) || (d == PlayerNum && (IsLaptop == 0))) {
                    PLAYER &qPlayer = Sim.Players.Players[d];

                    for (c = qPlayer.Planes.AnzEntries() - 1; c >= 0; c--) {
                        if (qPlayer.Planes.IsInAlbum(c) != 0)
                        // if ((c==0) == (IsLaptop==0))
                        {
                            CPlane &qPlane = qPlayer.Planes[c];
                            // qPlane.UpdateGlobePos (EarthAlpha);

                            if (qPlane.BehindGlobe == 0) {
                                // RoomBm.BlitFromT ((MapPlaneBms[d])[SLONG(qPlane.GlobeAngle)], TargetPos+qPlane.GlobePos-XY(9+6,9)-XY(5,5));
                                RoomBm.BlitFromT((MapPlaneBms[d])[SLONG(qPlane.GlobeAngle)], TargetPos + qPlane.GlobePos - XY(9, 9) - XY(7, 10));
                            }
                        }
                    }
                }
            }
        }
    }

    // Marker auf der Erde:
    for (c = qPlayer.Blocks.AnzEntries() - 1; c >= 0; c--) {
        if (qPlayer.Blocks.IsInAlbum(c) != 0) {
            if ((c == 0) == (IsLaptop == 0) && (FensterVisible != 0)) {
                BLOCK &qBlock = qPlayer.Blocks[c];
                if (qBlock.Index == 0 && qBlock.BlockType == 1) {
                    auto xx = UWORD(UWORD((Cities[qBlock.SelectedId].GlobusPosition.x + 180) * 65536 / 360) - EarthAlpha + 16384 - 1250);

                    if (xx > 16386 && xx < 49152) {
                        auto py = SLONG(sin((Cities[qBlock.SelectedId].GlobusPosition.y * a) / 240.0 * (3.14159 / 2)) * 185 + 190);
                        auto px = SLONG(sin((xx / 32768.0 - 1) * (3.14159)) * sqrt(184 * 184 - (py - 190) * (py - 190)) + 190);

                        RoomBm.BlitFromT(gCityMarkerBm, TargetPos + XY(px - 18, py - 18));
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Erledigt alles, was regelmäßig abgefragt wird: Tooltips, Verschieben von Flügen...
//--------------------------------------------------------------------------------------------
void CPlaner::DoPollingStuff() {
    PLAYER &qPlayer = Sim.Players.Players[PlayerNum];

    // Geschlossene Blöcke aus dem Array entfernen:
    SLONG c = 0;
    for (c = 0; c < qPlayer.Blocks.AnzEntries(); c++) {
        if ((qPlayer.Blocks.IsInAlbum(c) != 0) && qPlayer.Blocks[c].Destructing == 2) {
            GlobeWindows[qPlayer.Blocks[c].BlockType - 1] = qPlayer.Blocks[c].ScreenPos;
            qPlayer.Blocks -= c;
            if (CurrentBlock == c) {
                CurrentBlock = -1;
            }
            GlobeBm.Clear(0);
            PaintGlobe();
            PaintGlobeRoutes();
        }
    }

    if (MenuIsOpen() != 0) {
        return;
    }

    for (c = qPlayer.Blocks.AnzEntries(); c >= 0; c--) {
        if (qPlayer.Blocks.IsInAlbum(c) != 0) {
            if (qPlayer.Blocks[c].BlockType == 1) {
                if (qPlayer.Blocks[c].Index == 1) {
                    qPlayer.Blocks[c].SetTip(TIP_NONE, TIP_CITY, 0xffffffff);
                } else {
                    qPlayer.Blocks[c].SetTip(TIP_NONE, TIP_CITY, qPlayer.Blocks[c].SelectedId);
                }
            } else {
                qPlayer.Blocks[c].SetTip(TIP_NONE, TIP_NONE, 0);
            }
        }
    }

    // Alle Blöcke bei Bedarf neu zeichnen:
    if (qPlayer.Blocks.RepaintAll != 0) {
        qPlayer.Blocks.RepaintAll = FALSE;

        for (c = qPlayer.Blocks.AnzEntries(); c >= 0; c--) {
            if (qPlayer.Blocks.IsInAlbum(c) != 0) {
                if (qPlayer.Blocks[c].Index == 0 && qPlayer.Blocks[c].BlockType == 4 &&
                    qPlayer.RentRouten.RentRouten[Routen(qPlayer.Blocks[c].SelectedId)].Rang == 0) {
                    qPlayer.Blocks -= c;
                    GlobeBm.Clear(0);
                    PaintGlobe();
                    PaintGlobeRoutes();
                } else {
                    // qPlayer.Blocks[c].RefreshData (PlayerNum);
                    qPlayer.Blocks[c].RefreshData(Sim.localPlayer);
                    qPlayer.Blocks[c].Refresh(PlayerNum, IsLaptop);
                }
            }
        }
    }

    // Drag & Drop regeln...
    if ((DragFlightMode != 0) && CurrentBlock != -1) {
        BLOCK &qBlock = qPlayer.Blocks[DragFlightPar0];

        if ((qPlayer.Buttons & 1) == 0) {
            DragFlightMode = 0;
            qPlayer.Planes[DragFlightPar1].CheckFlugplaene(PlayerNum, FALSE);
            qPlayer.UpdateAuftragsUsage();
            qPlayer.UpdateFrachtauftragsUsage();
            qPlayer.Blocks[CurrentBlock].Refresh(PlayerNum, IsLaptop);
            qPlayer.NetUpdateFlightplan(DragFlightPar1);

            if (pBlock != nullptr) {
                pBlock->RefreshData(PlayerNum);
                pBlock->Refresh(PlayerNum, IsLaptop);
            }
        } else {
            CFlugplan &qPlan = qPlayer.Planes[DragFlightPar1].Flugplan;

            // Bricht die Zeit alles ab?
            if (qPlan.Flug[DragFlightPar2].Startdate == Sim.Date && Sim.GetHour() >= qPlan.Flug[DragFlightPar2].Startzeit) {
                DragFlightMode = 0;
            } else {
                if (qBlock.Index == 0) {
                    SLONG Date = Sim.Date + (ClientPos.y - 17) / 19;
                    SLONG Time = (ClientPos.x - 25) / 6;

                    if (DragFlightMode == 2) {
                        Time -= (qPlan.Flug[DragFlightPar2].Landezeit - qPlan.Flug[DragFlightPar2].Startzeit) +
                                (qPlan.Flug[DragFlightPar2].Landedate - qPlan.Flug[DragFlightPar2].Startdate) * 24;
                        while (Time < 0) {
                            Time += 24;
                            Date--;
                        }
                    }

                    if (Sim.Date == Date) {
                        Limit(SLONG(Sim.GetHour() + 2), Time, SLONG(24));
                    } else {
                        Limit(SLONG(0), Time, SLONG(24));
                    }
                    Limit(Sim.Date, Date, SLONG(Sim.Date + 6));

                    if (Time == 24) {
                        Time = 0;
                        Date++;
                        if (Date > Sim.Date + 6) {
                            Date--;
                            Time = 23;
                        }
                    }

                    SLONG DeltaDate = (Date - qPlan.Flug[DragFlightPar2].Startdate);
                    SLONG DeltaTime = (Time - qPlan.Flug[DragFlightPar2].Startzeit);

                    BUFFER_V<SLONG> DeltaDates(80);
                    BUFFER_V<SLONG> DeltaTimes(80);

                    if ((DeltaDate != 0) || (DeltaTime != 0)) {
                        for (c = DragFlightPar2; c < qPlan.Flug.AnzEntries(); c++) {
                            if (qPlan.Flug[c].ObjectType != 0) {
                                if (c + 1 < qPlan.Flug.AnzEntries() && qPlan.Flug[c + 1].ObjectType != 0) {
                                    DeltaDates[c] = qPlan.Flug[c + 1].Startdate - qPlan.Flug[c].Startdate;
                                    DeltaTimes[c] = qPlan.Flug[c + 1].Startzeit - qPlan.Flug[c].Startzeit;
                                }

                                qPlan.Flug[c].Startdate += DeltaDate;
                                qPlan.Flug[c].Startzeit += DeltaTime;

                                if (qPlan.Flug[c].Startzeit >= 24) {
                                    qPlan.Flug[c].Startzeit -= 24;
                                    qPlan.Flug[c].Startdate++;
                                }
                                if (qPlan.Flug[c].Startzeit < 0) {
                                    qPlan.Flug[c].Startzeit += 24;
                                    qPlan.Flug[c].Startdate--;
                                }

                                // Zahl der Passagiere berechnen:
                                if (qPlan.Flug[c].ObjectType != 3) {
                                    qPlan.Flug[c].FlightChanged();
                                    qPlan.Flug[c].CalcPassengers(PlayerNum, qPlayer.Planes[DragFlightPar1]);
                                    // qPlan.Flug[c].CalcPassengers (qPlayer.Planes[DragFlightPar1].TypeId, PlayerNum, qPlayer.Planes[DragFlightPar1]);
                                    qPlan.Flug[c].PArrived = 0;
                                }

                                if (qPlan.Flug[c].ObjectType == 4) {
                                    qPlan.Flug[c].Okay = 0;
                                }
                            }

                            if ((AtGetAsyncKeyState(ATKEY_SHIFT) & 0xff00) == 0) {
                                break;
                            }
                        }

                        qPlayer.Planes[DragFlightPar1].CheckFlugplaene(PlayerNum, FALSE);

                        if ((AtGetAsyncKeyState(ATKEY_SHIFT) & 0xff00) != 0) {
                            for (c = DragFlightPar2; c < qPlan.Flug.AnzEntries(); c++) {
                                if (qPlan.Flug[c].ObjectType != 0) {
                                    if (c + 1 < qPlan.Flug.AnzEntries() && qPlan.Flug[c + 1].ObjectType != 0) {
                                        if (qPlan.Flug[c + 1].ObjectType == 4) {
                                            qPlan.Flug[c + 1].Okay = 0;
                                        }
                                        qPlan.Flug[c + 1].Startdate = DeltaDates[c] + qPlan.Flug[c].Startdate;
                                        qPlan.Flug[c + 1].Startzeit = DeltaTimes[c] + qPlan.Flug[c].Startzeit;

                                        if (qPlan.Flug[c + 1].Startzeit >= 24) {
                                            qPlan.Flug[c + 1].Startzeit -= 24;
                                            qPlan.Flug[c + 1].Startdate++;
                                        }
                                        if (qPlan.Flug[c + 1].Startzeit < 0) {
                                            qPlan.Flug[c + 1].Startzeit += 24;
                                            qPlan.Flug[c + 1].Startdate--;
                                        }
                                    }
                                }
                            }
                        }

                        qPlan.UpdateNextFlight();
                        qPlan.UpdateNextStart();
                        qPlayer.Planes[DragFlightPar1].CheckFlugplaene(PlayerNum, FALSE);
                        qPlayer.UpdateAuftragsUsage();
                        qPlayer.UpdateFrachtauftragsUsage();
                        qPlayer.NetUpdateFlightplan(DragFlightPar1);
                        qPlayer.Blocks[CurrentBlock].RefreshData(PlayerNum);
                        qPlayer.Blocks[CurrentBlock].Refresh(PlayerNum, IsLaptop);
                    }

                    SetMouseLook(CURSOR_MOVE_H, 0, IsLaptop != 0 ? ROOM_LAPTOP : ROOM_GLOBE, 105);
                } else if (qBlock.Index == 2) {
                    SLONG Date = Sim.Date + qBlock.Page;
                    SLONG Time = (ClientPos.y - 5) / 6;

                    if (DragFlightMode == 2) {
                        Time -= (qPlan.Flug[DragFlightPar2].Landezeit - qPlan.Flug[DragFlightPar2].Startzeit) +
                                (qPlan.Flug[DragFlightPar2].Landedate - qPlan.Flug[DragFlightPar2].Startdate) * 24;
                        while (Time < 0) {
                            Time += 24;
                            Date--;
                        }
                    }

                    if (Sim.Date == Date) {
                        Limit(SLONG(Sim.GetHour() + 2), Time, SLONG(24));
                    } else {
                        Limit(SLONG(0), Time, SLONG(24));
                    }

                    if (Time == 24) {
                        Time = 0;
                        Date++;
                        if (Date > Sim.Date + 6) {
                            Date--;
                            Time = 23;
                        }
                    }

                    SLONG DeltaDate = (Date - qPlan.Flug[DragFlightPar2].Startdate);
                    SLONG DeltaTime = (Time - qPlan.Flug[DragFlightPar2].Startzeit);

                    if ((DeltaDate != 0) || (DeltaTime != 0)) {
                        for (c = DragFlightPar2; c < qPlan.Flug.AnzEntries(); c++) {
                            if (qPlan.Flug[c].ObjectType != 0) {
                                qPlan.Flug[c].Startdate += DeltaDate;
                                qPlan.Flug[c].Startzeit += DeltaTime;

                                if (qPlan.Flug[c].Startzeit >= 24) {
                                    qPlan.Flug[c].Startzeit -= 24;
                                    qPlan.Flug[c].Startdate++;
                                }
                                if (qPlan.Flug[c].Startzeit < 0) {
                                    qPlan.Flug[c].Startzeit += 24;
                                    qPlan.Flug[c].Startdate--;
                                }

                                // Zahl der Passagiere berechnen:
                                if (qPlan.Flug[c].ObjectType != 3) {
                                    qPlan.Flug[c].FlightChanged();
                                    qPlan.Flug[c].CalcPassengers(PlayerNum, qPlayer.Planes[DragFlightPar1]);
                                    // qPlan.Flug[c].CalcPassengers (qPlayer.Planes[DragFlightPar1].TypeId, PlayerNum, qPlayer.Planes[DragFlightPar1]);
                                    qPlan.Flug[c].PArrived = 0;
                                }
                            }
                        }

                        qPlan.UpdateNextFlight();
                        qPlan.UpdateNextStart();
                        qPlayer.Planes[DragFlightPar1].CheckFlugplaene(PlayerNum, FALSE);
                        qPlayer.UpdateAuftragsUsage();
                        qPlayer.UpdateFrachtauftragsUsage();
                        qPlayer.NetUpdateFlightplan(DragFlightPar1);
                        qPlayer.Blocks[CurrentBlock].Refresh(PlayerNum, IsLaptop);
                    }

                    SetMouseLook(CURSOR_MOVE_V, 0, IsLaptop != 0 ? ROOM_LAPTOP : ROOM_GLOBE, 105);
                }
            }
        }
    }

    // Globus zu einem Zielpunkt drehen...
    if (EarthTargetAlpha != EarthAlpha) {
        if (abs(EarthTargetAlpha - EarthAlpha) < 2000 || (UWORD(EarthTargetAlpha - EarthAlpha)) > 65336) {
            EarthAlpha = EarthTargetAlpha;
        } else if (UWORD(EarthTargetAlpha - EarthAlpha) < 0x8000) {
            EarthAlpha += 2000;
        } else {
            EarthAlpha -= 2000;
        }

        for (SLONG c = 0; c < Sim.Players.AnzPlayers; c++) {
            if (((qPlayer.DisplayPlanes[c] != 0U) && (IsLaptop != 0)) || (c == PlayerNum && (IsLaptop == 0))) {
                Sim.Players.Players[c].Planes.UpdateGlobePos(EarthAlpha);
            }
        }

        if (EarthTargetAlpha == EarthAlpha) {
            GlobeBm.Clear(0);
        }
        PaintGlobe();
        if (EarthTargetAlpha == EarthAlpha) {
            PaintGlobeRoutes();
        }
    }

    // Reguläre Plane/City/.. Tips für die Listen
    if ((IsInClientArea != 0) && (pBlock != nullptr) && (pBlock->IsTopWindow != 0) && pBlock->Index == 1) {
        if (pBlock->BlockType != 2 && ClientPos.IfIsWithin(0, 0, 174, 168) && ClientPos.y / 13 + pBlock->Page < pBlock->Table.AnzRows &&
            (qPlayer.Buttons & 1) == 0) {
            SLONG TipType = 0;
            UWORD HighlightColor = ColorOfFontBlack;

            if (pBlock->BlockType == 1 && (pBlock->Table.ValueFlags[(ClientPos.y / 13 + pBlock->Page) * pBlock->Table.AnzColums] == 0U)) {
                HighlightColor = ColorOfFontGrey;
            }

            CheckCursorHighlight(ClientPos, CRect(-1, ClientPos.y / 13 * 13 - 2, 172, ClientPos.y / 13 * 13 + 12), HighlightColor);

            // andere Tips:
            switch (pBlock->BlockType) {
            case 1:
                TipType = TIP_CITY;
                break;
            default:
                break;
            }

            if (TipType != 0) {
                pBlock->SetTip(TIP_NONE, TipType, pBlock->Table.LineIndex[ClientPos.y / 13 + pBlock->Page]);
            }
        } else if (pBlock->BlockType == 2 && ClientPos.IfIsWithin(0, 0, 174, 168) && ClientPos.y / 26 + pBlock->Page < pBlock->Table.AnzRows &&
                   (qPlayer.Buttons & 1) == 0) {
            SLONG TipType = 0;
            UWORD HighlightColor = ColorOfFontBlack;

            if (pBlock->Table.ValueFlags[(ClientPos.y / 26 + pBlock->Page) * pBlock->Table.AnzColums] != 0U) {
                HighlightColor = ColorOfFontRed;
            }

            CheckCursorHighlight(ClientPos, CRect(-1, ClientPos.y / 26 * 26 - 2, 172, ClientPos.y / 26 * 26 + 24), HighlightColor);

            // andere Tips:
            switch (pBlock->BlockType) {
            case 1:
                TipType = TIP_CITY;
                break;
            default:
                break;
            }

            if (TipType != 0) {
                pBlock->SetTip(TIP_NONE, TipType, pBlock->Table.LineIndex[ClientPos.y / 26 + pBlock->Page]);
            }
        }
    }
    if ((pBlock != nullptr) && pBlock->IndexB == 1 && (pBlock->BlockTypeB == 3 || pBlock->BlockTypeB == 4 || pBlock->BlockTypeB == 6) &&
        (IsInClientAreaB != 0) && CurrentPostItType == 0) // s.o.
    {
        if (ClientPosB.IfIsWithin(3, 0, 13, 155) && ClientPosB.y % 26 >= 10 && ClientPosB.y / 26 + pBlock->PageB < pBlock->TableB.AnzRows &&
            (qPlayer.Buttons & 1) == 0) {
            SetMouseLook(CURSOR_HOT, 0, IsLaptop != 0 ? ROOM_LAPTOP : ROOM_GLOBE, 0);
        }
    }
    if ((pBlock != nullptr) && pBlock->IndexB == 1 && pBlock->BlockType == 2 && (IsInClientAreaB != 0) && CurrentPostItType == 0) // s.o.
    {
        if (ClientPosB.IfIsWithin(16, 0, 172 - 5, 155) && ClientPosB.y / 26 + pBlock->PageB < pBlock->TableB.AnzRows && (qPlayer.Buttons & 1) == 0) {
            SLONG TableCursor = pBlock->PageB + ClientPosB.y / 26;
            CString Dummy;

            if (pBlock->TableB.ValueFlags[TableCursor * pBlock->TableB.AnzColums] == 0) {
                CheckCursorHighlight(ClientPosB, CRect(2, ClientPosB.y / 26 * 26 - 3, 172, ClientPosB.y / 26 * 26 + 24), ColorOfFontBlack);
            }
        }
    }

    // Highlight in die Detailsicht einer Stadt?
    if ((pBlock != nullptr) && (IsInClientArea != 0) && pBlock->Index == 0 && pBlock->BlockType == 1 && pBlock->Page == 0) {
        if (ClientPos.IfIsWithin(0, 66, 169, 79)) {
            CheckCursorHighlight(ClientPos, CRect(0, 64, 169, 79), ColorOfFontBlack);
        }
    }

    // Highlight in die Detailsicht eines Auftrags? ("Auftrag wegwerfen")
    if ((pBlock != nullptr) && (IsInClientAreaB != 0) && pBlock->IndexB == 0 && pBlock->BlockTypeB == 3 && pBlock->PageB == 0) {
        if (ClientPosB.IfIsWithin(0, 118, 169, 134)) {
            if (qPlayer.Auftraege[pBlock->SelectedIdB].InPlan == 0) {
                CheckCursorHighlight(ClientPosB, CRect(0, 117, 169, 147), ColorOfFontBlack);
            }
        }
    }

    // Highlight in die Detailsicht eines Frachtauftrags? ("Frachtauftrag wegwerfen")
    if ((pBlock != nullptr) && (IsInClientAreaB != 0) && pBlock->IndexB == 0 && pBlock->BlockTypeB == 6 && pBlock->PageB == 0) {
        if (ClientPosB.IfIsWithin(0, 118 + 17, 169, 134 + 17)) {
            if (qPlayer.Frachten[pBlock->SelectedIdB].InPlan == 0 &&
                qPlayer.Frachten[pBlock->SelectedIdB].TonsOpen == qPlayer.Frachten[pBlock->SelectedIdB].TonsLeft) {
                CheckCursorHighlight(ClientPosB, CRect(0, 117 + 17, 169, 147 + 17), ColorOfFontBlack);
            }
        }
    }

    // Highlight in die Detailsicht eines Flugzeuges (rename plane)?
    if ((pBlock != nullptr) && (IsInClientArea != 0) && pBlock->Index == 0 && pBlock->BlockType == 2 && pBlock->Page == 1) {
        if (ClientPos.IfIsWithin(0, 103 + 52, 169, 121 + 45)) {
            CheckCursorHighlight(ClientPos, CRect(0, 101 + 52, 169, 123 + 41), ColorOfFontBlack);
        }
    }

    // Tip für Flüge im Block (und nicht im Index)
    if ((pBlock != nullptr) && pBlock->Index == 0 && pBlock->BlockType == 2 && pBlock->Page == 0 && CurrentPostItType == 0 && (IsInClientArea != 0)) {
        if (ClientPos.IfIsWithin(22, 150, 58, 164) && (pBlock->IsTopWindow != 0)) // Flugpläne automatisch erweitern
        {
            SetMouseLook(CURSOR_HOT, 3021, IsLaptop != 0 ? ROOM_LAPTOP : ROOM_GLOBE, 150);
        } else if (ClientPos.IfIsWithin(135, 150, 170, 164) && (pBlock->IsTopWindow != 0)) // Flugpläne löschen
        {
            SetMouseLook(CURSOR_HOT, 3022, IsLaptop != 0 ? ROOM_LAPTOP : ROOM_GLOBE, 151);
        }
        if (ClientPos.IfIsWithin(25, 17, 168, 149) && CurrentPostItType == 0) {
            // if ((ClientPos.x-25+1)%6<3)
            if (DragFlightMode == 0) {
                SLONG Date = Sim.Date + (ClientPos.y - 17) / 19;
                SLONG Time = (ClientPos.x - 23) / 6;
                SLONG ActivePlane = pBlock->SelectedId;
                CFlugplan &qPlan = qPlayer.Planes[ActivePlane].Flugplan;

                if (Date != Sim.Date || Time > Sim.GetHour() + 1) {
                    // Drag am Fluganfang:
                    SLONG d = 0;
                    for (d = qPlan.Flug.AnzEntries() - 1; d >= 0; d--) {
                        if (qPlan.Flug[d].ObjectType == 1 || qPlan.Flug[d].ObjectType == 2 || qPlan.Flug[d].ObjectType == 4) {
                            if (qPlan.Flug[d].Startdate == Date && qPlan.Flug[d].Startzeit == Time) {
                                DragFlightPar0 = CurrentBlock;
                                DragFlightPar1 = ActivePlane;
                                DragFlightPar2 = d;
                                SetMouseLook(CURSOR_MOVE_H, 0, IsLaptop != 0 ? ROOM_LAPTOP : ROOM_GLOBE, 105);
                                if ((qPlayer.Buttons & 1) != 0) {
                                    DragFlightMode = 1;
                                }
                                break;
                            }
                        }
                    }

                    // Drag am Flugende:
                    for (d = qPlan.Flug.AnzEntries() - 1; d >= 0; d--) {
                        if (qPlan.Flug[d].ObjectType == 1 || qPlan.Flug[d].ObjectType == 2 || qPlan.Flug[d].ObjectType == 4) {
                            if (qPlan.Flug[d].Landedate == Date && qPlan.Flug[d].Landezeit == Time &&
                                (qPlan.Flug[d].Startdate != Sim.Date || qPlan.Flug[d].Startzeit > Sim.GetHour() + 1)) {
                                DragFlightPar0 = CurrentBlock;
                                DragFlightPar1 = ActivePlane;
                                DragFlightPar2 = d;
                                SetMouseLook(CURSOR_MOVE_H, 0, IsLaptop != 0 ? ROOM_LAPTOP : ROOM_GLOBE, 105);
                                if ((qPlayer.Buttons & 1) != 0) {
                                    DragFlightMode = 2;
                                }
                                break;
                            }
                        }
                    }
                }
            }

            // Routen und Auftragstips im Scheduler anzeigen
            if ((IsInClientArea != 0) && (pBlock->IsTopWindow != 0)) {
                SLONG ActivePlane = pBlock->SelectedId;
                CFlugplan &qPlan = qPlayer.Planes[ActivePlane].Flugplan;
                SLONG Date = Sim.Date + (ClientPos.y - 17) / 19;
                SLONG Time = (ClientPos.x - 25) / 6;
                SLONG d = 0;

                for (d = qPlan.Flug.AnzEntries() - 1; d >= 0; d--) {
                    if (qPlan.Flug[d].ObjectType != 0) {
                        if ((qPlan.Flug[d].Startdate < Date || (qPlan.Flug[d].Startdate == Date && qPlan.Flug[d].Startzeit <= Time)) &&
                            (qPlan.Flug[d].Landedate > Date || (qPlan.Flug[d].Landedate == Date && qPlan.Flug[d].Landezeit >= Time))) {
                            break;
                        }
                    }
                }

                if (d >= 0) {
                    if (qPlan.Flug[d].ObjectType == 2) // Auftrag
                    {
                        SLONG Kosten = CalculateFlightCostRechnerisch(qPlan.Flug[d].VonCity, qPlan.Flug[d].NachCity, qPlayer.Planes[ActivePlane].ptVerbrauch,
                                                                      // PlaneTypes[qPlayer.Planes[ActivePlane].TypeId].Verbrauch,
                                                                      qPlayer.Planes[ActivePlane].ptGeschwindigkeit,
                                                                      // PlaneTypes[qPlayer.Planes[ActivePlane].TypeId].Geschwindigkeit,
                                                                      PlayerNum);

                        Kosten += qPlayer.Auftraege[qPlan.Flug[d].ObjectId].Personen * FoodCosts[qPlayer.Planes[ActivePlane].Essen];

                        pBlock->SetTip(TIP_NONE, TIP_AUFTRAG, qPlan.Flug[d].ObjectId, Kosten, qPlan.Flug[d].Okay, qPlan.Flug[d].Gate,
                                       static_cast<SLONG>((qPlan.Flug[d].Startdate > Sim.Date) ||
                                                          (qPlan.Flug[d].Startdate == Sim.Date && qPlan.Flug[d].Startzeit > Sim.GetHour() + 1)));
                    } else if (qPlan.Flug[d].ObjectType == 4) // Auftrag
                    {
                        SLONG Kosten = CalculateFlightCostRechnerisch(qPlan.Flug[d].VonCity, qPlan.Flug[d].NachCity,
                                                                      // PlaneTypes[qPlayer.Planes[ActivePlane].TypeId].Verbrauch,
                                                                      qPlayer.Planes[ActivePlane].ptVerbrauch,
                                                                      // PlaneTypes[qPlayer.Planes[ActivePlane].TypeId].Geschwindigkeit,
                                                                      qPlayer.Planes[ActivePlane].ptGeschwindigkeit, PlayerNum);

                        pBlock->SetTip(TIP_NONE,                 // TipA
                                       TIP_FRACHT,               // TipB
                                       qPlan.Flug[d].ObjectId,   // TipId ==> Objekt, auf das bezogen wird:
                                       Kosten,                   // TipPar1 ==> Costs
                                       qPlan.Flug[d].Okay,       // TipPar2 ==> Okay
                                       qPlan.Flug[d].Passagiere, // TipPar3 ==> Fracht (T)
                                       static_cast<SLONG>((qPlan.Flug[d].Startdate > Sim.Date) ||
                                                          (qPlan.Flug[d].Startdate == Sim.Date && qPlan.Flug[d].Startzeit > Sim.GetHour() + 1)), // Unlocked
                                       qPlayer.Frachten[qPlan.Flug[d].ObjectId].Praemie * qPlan.Flug[d].Passagiere /
                                           qPlayer.Frachten[qPlan.Flug[d].ObjectId].Tons,
                                       qPlan.Flug[d].ObjectId); // Mit diesem Flug
                    } else if (qPlan.Flug[d].ObjectType == 1)   // Route
                    {
                        SLONG Kosten = CalculateFlightCostRechnerisch(qPlan.Flug[d].VonCity, qPlan.Flug[d].NachCity,
                                                                      // PlaneTypes[qPlayer.Planes[ActivePlane].TypeId].Verbrauch,
                                                                      qPlayer.Planes[ActivePlane].ptVerbrauch,
                                                                      // PlaneTypes[qPlayer.Planes[ActivePlane].TypeId].Geschwindigkeit,
                                                                      qPlayer.Planes[ActivePlane].ptGeschwindigkeit, PlayerNum);

                        if (qPlan.Flug[d].Startdate > Sim.Date || (qPlan.Flug[d].Startdate == Sim.Date && qPlan.Flug[d].Startzeit > Sim.GetHour())) {
                            Kosten += qPlayer.Planes[ActivePlane].ptPassagiere * FoodCosts[qPlayer.Planes[ActivePlane].Essen];
                            // Kosten+=PlaneTypes[qPlayer.Planes[ActivePlane].TypeId].Passagiere*FoodCosts[qPlayer.Planes[ActivePlane].Essen];
                        } else {
                            Kosten += qPlan.Flug[d].Passagiere * FoodCosts[qPlayer.Planes[ActivePlane].Essen];
                        }

                        pBlock->SetTip(TIP_NONE, TIP_ROUTE, qPlan.Flug[d].ObjectId,
                                       (qPlan.Flug[d].Startdate > Sim.Date || (qPlan.Flug[d].Startdate == Sim.Date && qPlan.Flug[d].Startzeit > Sim.GetHour()))
                                           ? 0
                                           : qPlan.Flug[d].Passagiere,
                                       Kosten, qPlan.Flug[d].Gate, qPlan.Flug[d].Ticketpreis,
                                       static_cast<SLONG>((qPlan.Flug[d].Startdate > Sim.Date) ||
                                                          (qPlan.Flug[d].Startdate == Sim.Date && qPlan.Flug[d].Startzeit > Sim.GetHour() + 1)),
                                       (qPlan.Flug[d].Startdate > Sim.Date || (qPlan.Flug[d].Startdate == Sim.Date && qPlan.Flug[d].Startzeit > Sim.GetHour()))
                                           ? 0
                                           : qPlan.Flug[d].PassagiereFC,
                                       qPlan.Flug[d].TicketpreisFC);
                    } else if (qPlan.Flug[d].ObjectType == 3) // Automatik
                    {
                        SLONG Kosten = CalculateFlightCostRechnerisch(qPlan.Flug[d].VonCity, qPlan.Flug[d].NachCity,
                                                                      // PlaneTypes[qPlayer.Planes[ActivePlane].TypeId].Verbrauch,
                                                                      qPlayer.Planes[ActivePlane].ptVerbrauch,
                                                                      // PlaneTypes[qPlayer.Planes[ActivePlane].TypeId].Geschwindigkeit,
                                                                      qPlayer.Planes[ActivePlane].ptGeschwindigkeit, PlayerNum);

                        // pBlock->SetTip (TIP_NONE, TIP_AUTOFLUG, Kosten, PlaneTypes[qPlayer.Planes[ActivePlane].TypeId].Passagiere, qPlan.Flug[d].VonCity,
                        // qPlan.Flug[d].NachCity,
                        // (qPlan.Flug[d].Startdate>Sim.Date)||(qPlan.Flug[d].Startdate==Sim.Date&&qPlan.Flug[d].Startzeit>Sim.GetHour()+1));
                        pBlock->SetTip(TIP_NONE, TIP_AUTOFLUG, Kosten, qPlayer.Planes[ActivePlane].ptPassagiere, qPlan.Flug[d].VonCity, qPlan.Flug[d].NachCity,
                                       static_cast<SLONG>((qPlan.Flug[d].Startdate > Sim.Date) ||
                                                          (qPlan.Flug[d].Startdate == Sim.Date && qPlan.Flug[d].Startzeit > Sim.GetHour() + 1)));
                    }
                }
            }
        }
    }
    // Cursor ist in Tagessicht eines Flugplans
    if ((pBlock != nullptr) && pBlock->Index == 2 && pBlock->BlockType == 2 && CurrentPostItType == 0 && (IsInClientArea != 0)) {
        if (ClientPos.IfIsWithin(8, 152, 127, 161)) {
            SLONG i = (ClientPos.x - 4) / 18;

            SetMouseLook(CURSOR_HOT, 0, IsLaptop != 0 ? ROOM_LAPTOP : ROOM_GLOBE, 104, c, i);
        } else if (ClientPos.IfIsWithin(128, 152, 170, 161)) {
            SetMouseLook(CURSOR_HOT, 0, IsLaptop != 0 ? ROOM_LAPTOP : ROOM_GLOBE, 104, c, 7);
        } else if (ClientPos.IfIsWithin(25, 5, 169, 149) && CurrentPostItType == 0) {
            // if ((ClientPos.y-5-2)%6<3)
            {
                SLONG Date = Sim.Date + pBlock->Page;
                SLONG Time = (ClientPos.y - 6) / 6;
                SLONG ActivePlane = pBlock->SelectedId;
                CFlugplan &qPlan = qPlayer.Planes[ActivePlane].Flugplan;
                SLONG d = 0;

                if (Date != Sim.Date || Time > Sim.GetHour() + 1) {
                    for (d = qPlan.Flug.AnzEntries() - 1; d >= 0; d--) {
                        if (qPlan.Flug[d].ObjectType == 1 || qPlan.Flug[d].ObjectType == 2) {
                            if (qPlan.Flug[d].Startdate == Date && qPlan.Flug[d].Startzeit == Time) {
                                DragFlightPar0 = CurrentBlock;
                                DragFlightPar1 = ActivePlane;
                                DragFlightPar2 = d;
                                SetMouseLook(CURSOR_MOVE_V, 0, IsLaptop != 0 ? ROOM_LAPTOP : ROOM_GLOBE, 105);
                                if ((qPlayer.Buttons & 1) != 0) {
                                    DragFlightMode = 1;
                                }
                                break;
                            }
                        }
                    }

                    // Drag am Flugende:
                    for (d = qPlan.Flug.AnzEntries() - 1; d >= 0; d--) {
                        if (qPlan.Flug[d].ObjectType == 1 || qPlan.Flug[d].ObjectType == 2) {
                            if (qPlan.Flug[d].Startdate == Date && qPlan.Flug[d].Landezeit == Time &&
                                (qPlan.Flug[d].Startdate != Sim.Date || qPlan.Flug[d].Startzeit > Sim.GetHour() + 1)) {
                                DragFlightPar0 = CurrentBlock;
                                DragFlightPar1 = ActivePlane;
                                DragFlightPar2 = d;
                                SetMouseLook(CURSOR_MOVE_V, 0, IsLaptop != 0 ? ROOM_LAPTOP : ROOM_GLOBE, 105);
                                if ((qPlayer.Buttons & 1) != 0) {
                                    DragFlightMode = 2;
                                }
                                break;
                            }
                        }
                    }
                }
            }

            // Routen und Auftragstips im Scheduler anzeigen
            if (MouseLook != CURSOR_MOVE_V && (pBlock->IsTopWindow != 0)) {
                SLONG ActivePlane = pBlock->SelectedId;
                CFlugplan &qPlan = qPlayer.Planes[ActivePlane].Flugplan;
                SLONG Date = Sim.Date + pBlock->Page;
                SLONG Time = (ClientPos.y - 6) / 6;
                SLONG d = 0;

                for (d = qPlan.Flug.AnzEntries() - 1; d >= 0; d--) {
                    if (qPlan.Flug[d].ObjectType != 0) {
                        if ((qPlan.Flug[d].Startdate < Date || (qPlan.Flug[d].Startdate == Date && qPlan.Flug[d].Startzeit <= Time)) &&
                            (qPlan.Flug[d].Landedate > Date || (qPlan.Flug[d].Landedate == Date && qPlan.Flug[d].Landezeit >= Time))) {
                            break;
                        }
                    }
                }

                if (d >= 0) {
                    if (qPlan.Flug[d].ObjectType == 2) // Auftrag
                    {
                        SLONG Kosten = CalculateFlightCostRechnerisch(qPlan.Flug[d].VonCity, qPlan.Flug[d].NachCity,
                                                                      // PlaneTypes[qPlayer.Planes[ActivePlane].TypeId].Verbrauch,
                                                                      qPlayer.Planes[ActivePlane].ptVerbrauch,
                                                                      // PlaneTypes[qPlayer.Planes[ActivePlane].TypeId].Geschwindigkeit,
                                                                      qPlayer.Planes[ActivePlane].ptGeschwindigkeit, PlayerNum);

                        Kosten += qPlayer.Auftraege[qPlan.Flug[d].ObjectId].Personen * FoodCosts[qPlayer.Planes[ActivePlane].Essen];

                        pBlock->SetTip(TIP_NONE, TIP_AUFTRAG, qPlan.Flug[d].ObjectId, Kosten, qPlan.Flug[d].Okay, qPlan.Flug[d].Gate,
                                       static_cast<SLONG>((qPlan.Flug[d].Startdate > Sim.Date) ||
                                                          (qPlan.Flug[d].Startdate == Sim.Date && qPlan.Flug[d].Startzeit > Sim.GetHour() + 1)));
                    } else if (qPlan.Flug[d].ObjectType == 1) // Route
                    {
                        SLONG Kosten = CalculateFlightCostRechnerisch(qPlan.Flug[d].VonCity, qPlan.Flug[d].NachCity,
                                                                      // PlaneTypes[qPlayer.Planes[ActivePlane].TypeId].Verbrauch,
                                                                      qPlayer.Planes[ActivePlane].ptVerbrauch,
                                                                      // PlaneTypes[qPlayer.Planes[ActivePlane].TypeId].Geschwindigkeit,
                                                                      qPlayer.Planes[ActivePlane].ptGeschwindigkeit, PlayerNum);

                        if (qPlan.Flug[d].Startdate > Sim.Date || (qPlan.Flug[d].Startdate == Sim.Date && qPlan.Flug[d].Startzeit > Sim.GetHour())) {
                            // Kosten+=PlaneTypes[qPlayer.Planes[ActivePlane].TypeId].Passagiere*FoodCosts[qPlayer.Planes[ActivePlane].Essen];
                            Kosten += qPlayer.Planes[ActivePlane].ptPassagiere * FoodCosts[qPlayer.Planes[ActivePlane].Essen];
                        } else {
                            Kosten += qPlan.Flug[d].Passagiere * FoodCosts[qPlayer.Planes[ActivePlane].Essen];
                        }

                        pBlock->SetTip(TIP_NONE, TIP_ROUTE, qPlan.Flug[d].ObjectId,
                                       (qPlan.Flug[d].Startdate > Sim.Date || (qPlan.Flug[d].Startdate == Sim.Date && qPlan.Flug[d].Startzeit > Sim.GetHour()))
                                           ? 0
                                           : qPlan.Flug[d].Passagiere,
                                       Kosten, qPlan.Flug[d].Gate, qPlan.Flug[d].Ticketpreis,
                                       static_cast<SLONG>((qPlan.Flug[d].Startdate > Sim.Date) ||
                                                          (qPlan.Flug[d].Startdate == Sim.Date && qPlan.Flug[d].Startzeit > Sim.GetHour() + 1)),
                                       (qPlan.Flug[d].Startdate > Sim.Date || (qPlan.Flug[d].Startdate == Sim.Date && qPlan.Flug[d].Startzeit > Sim.GetHour()))
                                           ? 0
                                           : qPlan.Flug[d].PassagiereFC,
                                       qPlan.Flug[d].TicketpreisFC);
                    } else if (qPlan.Flug[d].ObjectType == 3) // Automatik-Flug
                    {
                        SLONG Kosten = CalculateFlightCostRechnerisch(qPlan.Flug[d].VonCity, qPlan.Flug[d].NachCity,
                                                                      // PlaneTypes[qPlayer.Planes[ActivePlane].TypeId].Verbrauch,
                                                                      qPlayer.Planes[ActivePlane].ptVerbrauch,
                                                                      // PlaneTypes[qPlayer.Planes[ActivePlane].TypeId].Geschwindigkeit,
                                                                      qPlayer.Planes[ActivePlane].ptGeschwindigkeit, PlayerNum);

                        // pBlock->SetTip (TIP_NONE, TIP_AUTOFLUG, Kosten, PlaneTypes[qPlayer.Planes[ActivePlane].TypeId].Passagiere, qPlan.Flug[d].VonCity,
                        // qPlan.Flug[d].NachCity,
                        // (qPlan.Flug[d].Startdate>Sim.Date)||(qPlan.Flug[d].Startdate==Sim.Date&&qPlan.Flug[d].Startzeit>Sim.GetHour()+1));
                        pBlock->SetTip(TIP_NONE, TIP_AUTOFLUG, Kosten, qPlayer.Planes[ActivePlane].ptPassagiere, qPlan.Flug[d].VonCity, qPlan.Flug[d].NachCity,
                                       static_cast<SLONG>((qPlan.Flug[d].Startdate > Sim.Date) ||
                                                          (qPlan.Flug[d].Startdate == Sim.Date && qPlan.Flug[d].Startzeit > Sim.GetHour() + 1)));
                    }
                }
            }
        }
    }
    if ((pBlock != nullptr) && pBlock->IndexB == 0 && pBlock->BlockTypeB == 4 && CurrentPostItType == 0 && (IsInClientAreaB != 0)) {
        if (pBlock->PageB == 0) {
            // Ticketpreis einstellen:
            if (ClientPosB.IfIsWithin(2, 40 + 26, 169, 118)) {
                CheckCursorHighlight(ClientPosB, CRect(0, (ClientPosB.y - 40) / 13 * 13 + 40 - 2, 172, (ClientPosB.y - 40) / 13 * 13 + 40 - 2 + 14),
                                     ColorOfFontBlack);
            }

            CheckCursorHighlight(ClientPosB, CRect(148, 40 - 2, 160, 40 - 2 + 14), ColorOfFontBlack);
            CheckCursorHighlight(ClientPosB, CRect(160, 40 - 2, 172, 40 - 2 + 14), ColorOfFontBlack);
            CheckCursorHighlight(ClientPosB, CRect(148, 40 - 2 - 13, 160, 40 - 2 - 13 + 14), ColorOfFontBlack);
            CheckCursorHighlight(ClientPosB, CRect(160, 40 - 2 - 13, 172, 40 - 2 - 13 + 14), ColorOfFontBlack);

            // Konkurrenzpreise:
            if ((qPlayer.HasBerater(BERATERTYP_INFO) != 0) && ClientPosB.IfIsWithin(2, 128, 166, 166)) {
                SLONG Index = (ClientPosB.y - 128) / 13;

                if (Sim.Players.Players[Index + static_cast<SLONG>(PlayerNum <= Index)].IsOut == 0 &&
                    (Sim.Players.Players[Index + static_cast<SLONG>(PlayerNum <= Index)].RentRouten.RentRouten[Routen(pBlock->SelectedIdB)].Rang != 0U)) {
                    CheckCursorHighlight(ClientPosB, CRect(2, (ClientPosB.y - 128) / 13 * 13 + 128 - 2, 172, (ClientPosB.y - 128) / 13 * 13 + 128 - 2 + 14),
                                         ColorOfFontBlack);
                }
            }
        } else if (pBlock->PageB == 1) {
            if (ClientPosB.IfIsWithin(2, 144, 169, 157)) // Kündigen?
            {
                CheckCursorHighlight(ClientPosB, CRect(0, 142, 172, 156), ColorOfFontBlack);
            }
        }
    }

    // Cursor über Flugzeug?
    if (CurrentBlock == -1 && ((qPlayer.DisplayPlanes[PlayerNum] != 0U) || IsLaptop == FALSE)) {
        for (c = qPlayer.Planes.AnzEntries() - 1; c >= 0; c--) {
            if (qPlayer.Planes.IsInAlbum(c) != 0) {
                XY p = GlobeOffset[IsLaptop] + qPlayer.Planes[c].GlobePos - XY(9 + 6, 9) - XY(5, 5);

                if (gMousePosition.IfIsWithin(p.x, p.y, p.x + 19, p.y + 19)) {
                    SetMouseLook(CURSOR_HOT, c + 1, qPlayer.Planes[c].Name, IsLaptop != 0 ? ROOM_LAPTOP : ROOM_GLOBE, 101, qPlayer.Planes.GetIdFromIndex(c));
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Erledigt den Kram, der unbedingt, *nach* dem Paint gemacht werden muß
//--------------------------------------------------------------------------------------------
void CPlaner::DoPostPaintPollingStuff(XY FlightPlanPos) {
    if (pBlock != nullptr) {
        PLAYER &qPlayer = Sim.Players.Players[PlayerNum];

        FlightPlanPos += pBlock->ScreenPos;

        // Grid, zum markieren der Zieltage (Aufträge):
        if (CurrentPostItType == 2 && pBlock->Index != 1 && (pBlock->Page == 0 || pBlock->Index == 2) && pBlock->BlockType == 2 &&
            (pBlock->IsTopWindow != 0)) // Auftrag
        {
            qPlayer.Auftraege[CurrentPostItId].BlitGridAt(&RoomBm, XY(FlightPlanPos.x, FlightPlanPos.y - static_cast<SLONG>(IsLaptop == 1)),
                                                          static_cast<BOOL>(pBlock->Index == 2), pBlock->Page);
        }

        if (pBlock->IndexB == 1 && pBlock->BlockType == 2 && (pBlock->Page == 0 || pBlock->Index == 2) && (IsInClientAreaB != 0) &&
            CurrentPostItType == 0) // s.o.
        {
            if (ClientPosB.IfIsWithin(16, 0, 172 - 5, 170) && ClientPosB.y / 26 + pBlock->PageB < pBlock->TableB.AnzRows && (qPlayer.Buttons & 1) == 0) {
                SLONG TableCursor = pBlock->PageB + ClientPosB.y / 26;

                if (pBlock->BlockTypeB == 4 || pBlock->TableB.ValueFlags[TableCursor * pBlock->TableB.AnzColums] == 0) {
                    if (pBlock->BlockTypeB == 3 && pBlock->Index != 1 && (pBlock->IsTopWindow != 0)) // Auftrag
                    {
                        qPlayer.Auftraege[pBlock->TableB.LineIndex[TableCursor]].BlitGridAt(
                            &RoomBm, XY(FlightPlanPos.x, FlightPlanPos.y - static_cast<SLONG>(IsLaptop == 1)), static_cast<BOOL>(pBlock->Index == 2),
                            pBlock->Page);
                    }
                } else if (pBlock->TableB.ValueFlags[TableCursor * pBlock->TableB.AnzColums] != 0 && pBlock->Index == 0 && (pBlock->IsTopWindow != 0)) {
                    for (SLONG c = 0; c < 7; c++) {
                        RoomBm.BlitFromT(FlugplanBms[51], FlightPlanPos.x, FlightPlanPos.y - static_cast<SLONG>(IsLaptop == 1) + FlugplanBms[51].Size.y * c);
                    }
                }
            }
        }

        // Grid, zum markieren der Zieltage (Frachtaufträge):
        if (CurrentPostItType == 4 && pBlock->Index != 1 && (pBlock->Page == 0 || pBlock->Index == 2) && pBlock->BlockType == 2 &&
            (pBlock->IsTopWindow != 0)) // Auftrag
        {
            qPlayer.Frachten[CurrentPostItId].BlitGridAt(&RoomBm, XY(FlightPlanPos.x, FlightPlanPos.y - static_cast<SLONG>(IsLaptop == 1)),
                                                         static_cast<BOOL>(pBlock->Index == 2), pBlock->Page);
        }

        if (pBlock->IndexB == 1 && pBlock->BlockType == 2 && (pBlock->Page == 0 || pBlock->Index == 2) && (IsInClientAreaB != 0) &&
            CurrentPostItType == 0) // s.o.
        {
            if (ClientPosB.IfIsWithin(16, 0, 172 - 5, 170) && ClientPosB.y / 26 + pBlock->PageB < pBlock->TableB.AnzRows && (qPlayer.Buttons & 1) == 0) {
                SLONG TableCursor = pBlock->PageB + ClientPosB.y / 26;

                if (pBlock->BlockTypeB == 4 || pBlock->TableB.ValueFlags[TableCursor * pBlock->TableB.AnzColums] == 0) {
                    if (pBlock->BlockTypeB == 6 && pBlock->Index != 1 && (pBlock->IsTopWindow != 0)) // Auftrag
                    {
                        qPlayer.Frachten[pBlock->TableB.LineIndex[TableCursor]].BlitGridAt(
                            &RoomBm, XY(FlightPlanPos.x, FlightPlanPos.y - static_cast<SLONG>(IsLaptop == 1)), static_cast<BOOL>(pBlock->Index == 2),
                            pBlock->Page);
                    }
                } else if (pBlock->TableB.ValueFlags[TableCursor * pBlock->TableB.AnzColums] != 0 && pBlock->Index == 0 && (pBlock->IsTopWindow != 0)) {
                    for (SLONG c = 0; c < 7; c++) {
                        RoomBm.BlitFromT(FlugplanBms[51], FlightPlanPos.x, FlightPlanPos.y - static_cast<SLONG>(IsLaptop == 1) + FlugplanBms[51].Size.y * c);
                    }
                }
            }
        }

        // Grid zum markieren der ungültigen Flugzeuge
        SLONG CheckVonCityId = -1;
        SLONG CheckNachCityId = -1;
        SLONG MinPass = 0;
        if (CurrentPostItType != 0 && pBlock->Index == 1 && pBlock->BlockType == 2 && (pBlock->IsTopWindow != 0)) {
            if (CurrentPostItType == 2) // Auftrag
            {
                CheckVonCityId = qPlayer.Auftraege[CurrentPostItId].VonCity;
                CheckNachCityId = qPlayer.Auftraege[CurrentPostItId].NachCity;
                MinPass = qPlayer.Auftraege[CurrentPostItId].Personen;
            } else if (CurrentPostItType == 1) // Route
            {
                CheckVonCityId = Routen[CurrentPostItId].VonCity;
                CheckNachCityId = Routen[CurrentPostItId].NachCity;
            } else if (CurrentPostItType == 4) // Frachtauftrag
            {
                CheckVonCityId = qPlayer.Frachten[CurrentPostItId].VonCity;
                CheckNachCityId = qPlayer.Frachten[CurrentPostItId].NachCity;
            }
        } else if (pBlock->IndexB == 1 && pBlock->BlockType == 2 && pBlock->Index == 1 && (IsInClientAreaB != 0) && CurrentPostItType == 0) {
            SLONG TableCursor = pBlock->PageB + ClientPosB.y / 26;

            if (TableCursor >= 0 && TableCursor < pBlock->TableB.AnzRows) {
                if (pBlock->BlockTypeB == 3) // Auftrag
                {
                    CheckVonCityId = qPlayer.Auftraege[pBlock->TableB.LineIndex[TableCursor]].VonCity;
                    CheckNachCityId = qPlayer.Auftraege[pBlock->TableB.LineIndex[TableCursor]].NachCity;
                    MinPass = qPlayer.Auftraege[pBlock->TableB.LineIndex[TableCursor]].Personen;
                } else if (pBlock->BlockTypeB == 4) // Route
                {
                    CheckVonCityId = Routen[pBlock->TableB.LineIndex[TableCursor]].VonCity;
                    CheckNachCityId = Routen[pBlock->TableB.LineIndex[TableCursor]].NachCity;
                } else if (pBlock->BlockTypeB == 6) // Aufträge
                {
                    CheckVonCityId = qPlayer.Frachten[pBlock->TableB.LineIndex[TableCursor]].VonCity;
                    CheckNachCityId = qPlayer.Frachten[pBlock->TableB.LineIndex[TableCursor]].NachCity;
                }
            }
        }

        if (CheckVonCityId != -1) {
            for (SLONG c = pBlock->Page; c < pBlock->Page + 6 && c < pBlock->Table.AnzRows; c++) {
                SLONG ActivePlane = pBlock->Table.LineIndex[c];

                CRect HighRect(FlightPlanPos.x - 24, FlightPlanPos.y + c * 26 + 3 - 26, FlightPlanPos.x + 170 - 24, FlightPlanPos.y + c * 26 - 26 + 3 + 23);

                // if (MinPass>PlaneTypes[qPlayer.Planes[ActivePlane].TypeId].Passagiere)
                if (MinPass > qPlayer.Planes[ActivePlane].ptPassagiere) {
                    RemapColor(RoomBm.pBitmap, HighRect, ColorOfFontBlack, ColorOfFontGrey);
                    RemapColor(RoomBm.pBitmap, HighRect, ColorOfFontRed, ColorOfFontGrey);
                }
                // else if (Cities.CalcDistance (CheckVonCityId, CheckNachCityId)>PlaneTypes[qPlayer.Planes[ActivePlane].TypeId].Reichweite*1000)
                else if (Cities.CalcDistance(CheckVonCityId, CheckNachCityId) > qPlayer.Planes[ActivePlane].ptReichweite * 1000) {
                    RemapColor(RoomBm.pBitmap, HighRect, ColorOfFontBlack, ColorOfFontGrey);
                    RemapColor(RoomBm.pBitmap, HighRect, ColorOfFontRed, ColorOfFontGrey);
                } else {
                    // SLONG Speed = PlaneTypes[qPlayer.Planes[ActivePlane].TypeId].Geschwindigkeit;
                    SLONG Speed = qPlayer.Planes[ActivePlane].ptGeschwindigkeit;
                    SLONG Dauer = Cities.CalcFlugdauer(CheckVonCityId, CheckNachCityId, Speed);
                    // SLONG Dauer = (Cities.CalcDistance (CheckVonCityId, CheckNachCityId)/Speed+999)/1000+1+2;

                    if (Dauer >= 24) {
                        RemapColor(RoomBm.pBitmap, HighRect, ColorOfFontBlack, ColorOfFontGrey);
                        RemapColor(RoomBm.pBitmap, HighRect, ColorOfFontRed, ColorOfFontGrey);
                    }
                }
            }
        }

        // Wiederholfunktion für Kerosinpreis erhöhen / verringern
        if ((gMouseLButton != 0) && (IsInClientAreaB != 0) && pBlock->BlockType == 2 && pBlock->IndexB == 0 && pBlock->BlockTypeB == 4 && pBlock->PageB == 0) {
            if (ClientPosB.IfIsWithin(2, 40, 169, 118) && AtGetTime() - gMouseLButtonDownTimer > 800) {
                /*
                CRentRoute *pRRoute = nullptr;
                SLONG SelectedIdB2 = 0;

                for (SLONG c = qPlayer.RentRouten.RentRouten.AnzEntries() - 1; c >= 0; c--) {
                    if (Routen.IsInAlbum(c) != 0) {
                        if (Routen[c].VonCity == Routen[pBlock->SelectedIdB].NachCity && Routen[c].NachCity == Routen[pBlock->SelectedIdB].VonCity) {
                            pRRoute = &qPlayer.RentRouten.RentRouten[c];
                            SelectedIdB2 = c;
                            break;
                        }
                    }
                }

                  SLONG Cost = CalculateFlightCostRechnerisch(Routen[pBlock->SelectedIdB].VonCity, Routen[pBlock->SelectedIdB].NachCity, 800, 800, -1) * 3 / 180
                * 2; CRentRoute &qRRoute = qPlayer.RentRouten.RentRouten[Routen(pBlock->SelectedIdB)]; switch ((ClientPosB.y-40)/13)
                  {
                  case 0:
                  qRRoute.Ticketpreis+=10;
                  pRRoute->Ticketpreis+=10;
                  Limit (SLONG(0), qRRoute.Ticketpreis, SLONG(Cost*16/10*10));
                  qPlayer.UpdateTicketpreise (pBlock->SelectedIdB, qRRoute.Ticketpreis, qRRoute.TicketpreisFC);
                  qPlayer.UpdateTicketpreise (SelectedIdB2, pRRoute->Ticketpreis, pRRoute->TicketpreisFC);
                  qPlayer.NetSynchronizeRoutes();
                  qPlayer.NetRouteUpdateTicketpreise(pBlock->SelectedIdB, qRRoute.Ticketpreis, qRRoute.TicketpreisFC);
                  qPlayer.NetRouteUpdateTicketpreise(SelectedIdB2, pRRoute->Ticketpreis, pRRoute->TicketpreisFC);
                  break;

                  case 1:
                  qRRoute.Ticketpreis-=10;
                  pRRoute->Ticketpreis-=10;
                  Limit (SLONG(0), qRRoute.Ticketpreis, SLONG(Cost*16/10*10));
                  qPlayer.UpdateTicketpreise (pBlock->SelectedIdB, qRRoute.Ticketpreis, qRRoute.TicketpreisFC);
                  qPlayer.UpdateTicketpreise (SelectedIdB2, pRRoute->Ticketpreis, pRRoute->TicketpreisFC);
                  qPlayer.NetSynchronizeRoutes();
                  qPlayer.NetRouteUpdateTicketpreise(pBlock->SelectedIdB, qRRoute.Ticketpreis, qRRoute.TicketpreisFC);
                  qPlayer.NetRouteUpdateTicketpreise(SelectedIdB2, pRRoute->Ticketpreis, pRRoute->TicketpreisFC);
                  break;
                  }*/

                pBlock->RefreshData(PlayerNum);
                pBlock->Refresh(PlayerNum, IsLaptop);

                gMouseLButtonDownTimer = AtGetTime() - 400;
            }
        }

        if (pBlock->IndexB == 0 && CurrentPostItType == 0 && (IsInClientAreaB != 0) && pBlock->PageB == pBlock->AnzPagesB - 2 && (pBlock->IsTopWindow != 0)) {
            XY Off = pBlock->ScreenPos;
            if (IsLaptop == 0) {
                Off += XY(232 + 48, 72);
            } else {
                Off += XY(187 + 20, 21) + XY(-1, 1);
            }

            if (pBlock->BlockTypeB == 4 && ClientPosB.IfIsWithin(2, 144, 169, 157)) // Route Kündigen?
            {
                RoomBm.BlitFrom(FlugplanBms[58], Off.x + 2, Off.y + 144 + 3 - 5);
            }
            if (pBlock->BlockTypeB == 3 && ClientPosB.IfIsWithin(0, 118, 169, 134)) // Auftrag kündigen
            {
                if (qPlayer.Auftraege[pBlock->SelectedIdB].InPlan == 0) {
                    RoomBm.BlitFrom(FlugplanBms[58], Off.x, Off.y + 144 - 26);
                }
            }
            // Highlight in die Detailsicht eines Frachtauftrags? ("Frachtauftrag wegwerfen")
            if (pBlock->BlockTypeB == 6 && qPlayer.Frachten[pBlock->SelectedIdB].InPlan == 0) {
                if (ClientPosB.IfIsWithin(0, 118 + 17, 169, 134 + 17) &&
                    qPlayer.Frachten[pBlock->SelectedIdB].TonsOpen == qPlayer.Frachten[pBlock->SelectedIdB].TonsLeft) {
                    RoomBm.BlitFrom(FlugplanBms[58], Off.x, Off.y + 144 - 26 + 17);
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Erledigt den Kram, wenn in einen Block geklickt wird:
//--------------------------------------------------------------------------------------------
void CPlaner::HandleLButtonDown() {
    PLAYER &qPlayer = Sim.Players.Players[PlayerNum];

    DefaultOnLButtonDown();
    TookUnderCursorWithThisClick = FALSE;

    if (KonstruktorFinished != 2) {
        return;
    }

    gMouseLButtonDownTimer = AtGetTime();

    // in einen Flugzeugblock ==> Wechsel zur Tagesansicht:
    if ((MouseClickArea == ROOM_GLOBE || MouseClickArea == ROOM_LAPTOP) && MouseClickId == 104) {
        if (MouseClickPar2 == 7) {
            pBlock->Index = 0;
            pBlock->Page = 0;
            pBlock->RefreshData(PlayerNum);
            pBlock->Refresh(PlayerNum, IsLaptop);
        }
    }

    // Click in Client Area?
    if ((IsInClientArea != 0) && (pBlock != nullptr)) {
        // in Index Mode?
        if (pBlock->Index == 1) {
            SLONG TableCursor = 0;

            if (pBlock->BlockType == 2) {
                TableCursor = pBlock->Page + ClientPos.y / 26;
            } else {
                TableCursor = pBlock->Page + ClientPos.y / 13;
            }

            if (TableCursor >= 0 && TableCursor < pBlock->Table.AnzRows) {
                switch (pBlock->BlockType) {
                // Städte-Index:
                case 1:
                    pBlock->Index = 0;
                    pBlock->Page = 0;
                    pBlock->SelectedId = pBlock->Table.LineIndex[TableCursor];

                    pBlock->LoadCityPhotoLib(Cities[pBlock->SelectedId]);

                    EarthTargetAlpha = UWORD((Cities[pBlock->SelectedId].GlobusPosition.x + 170) * (3200 / 18) - 16000 + 1300);

                    pBlock->RefreshData(PlayerNum);
                    pBlock->Refresh(PlayerNum, IsLaptop);
                    break;

                    // Flugzeug-Details:
                case 2:
                    // Tutorial und noch zu früh? Dann abbrechen!
                    if ((Sim.IsTutorial != 0) && Sim.Tutorial < 1503) {
                        return;
                    }

                    pBlock->Index = 0;
                    pBlock->Page = 0;
                    pBlock->SelectedId = pBlock->Table.LineIndex[TableCursor];

                    pBlock->RefreshData(PlayerNum);
                    pBlock->Refresh(PlayerNum, IsLaptop);
                    GlobeBm.Clear(0);
                    PaintGlobe();
                    PaintGlobeRoutes();

                    if (CurrentPostItType != 0) {
                        PaintPostIt();
                    }

                    // Tutorium: Spieler betrachtet Flugzeuge:
                    if ((Sim.IsTutorial != 0) && Sim.Tutorial == 1503) {
                        GlowEffects.ReSize(1);
                        GlowBitmapIndices.ReSize(1);
                        GlowEffects[0] = XY(263 + 10, 261 - 2);
                        GlowBitmapIndices[0] = 2;

                        Sim.Tutorial = 1504;
                        qPlayer.Messages.NextMessage();
                        qPlayer.Messages.AddMessage(BERATERTYP_GIRL,
                                                    bprintf(StandardTexte.GetS(TOKEN_TUTORIUM, 1504), (LPCTSTR)qPlayer.Planes[pBlock->SelectedId].Name));
                    }

                    break;

                    // Experten-Details:
                case 5:
                    pBlock->Index = 0;
                    pBlock->Page = 0;
                    pBlock->SelectedId = pBlock->Table.LineIndex[TableCursor];
                    pBlock->RefreshData(PlayerNum);
                    pBlock->Refresh(PlayerNum, IsLaptop);
                    break;
                default:
                    break;
                }
            }
        }
        // Klick in den Wochensicht des Schedulers?
        else if (pBlock->Index == 0 && pBlock->BlockType == 2 && pBlock->Page == 0) {
            SLONG ActivePlane = pBlock->SelectedId;
            CFlugplan &qPlan = qPlayer.Planes[ActivePlane].Flugplan;

            if ((MouseClickArea == ROOM_GLOBE || MouseClickArea == ROOM_LAPTOP) && MouseClickId == 150) // Flugpläne automatisch erweitern
            {
                qPlan.UpdateNextFlight();
                qPlan.UpdateNextStart();
                qPlayer.Planes[pBlock->SelectedId].ExtendFlugplaene(PlayerNum);
                qPlayer.UpdateAuftragsUsage();
                qPlayer.UpdateFrachtauftragsUsage();
                qPlayer.NetUpdateFlightplan(pBlock->SelectedId);
                qPlayer.Planes[pBlock->SelectedId].CheckFlugplaene(PlayerNum, FALSE);
                qPlayer.Blocks[CurrentBlock].RefreshData(PlayerNum);
                qPlayer.Blocks[CurrentBlock].Refresh(PlayerNum, IsLaptop);
            } else if ((MouseClickArea == ROOM_GLOBE || MouseClickArea == ROOM_LAPTOP) && MouseClickId == 151) // Flugplan löschen
            {
                (*qPlayer.LocationWin).MenuStart(MENU_REQUEST, MENU_REQUEST_KILLPLAN, ActivePlane);
            } else if (CurrentPostItType != 0 && ClientPos.IfIsWithin(24, 17, 167, 149)) {
                SLONG tmpObjectType = 0;
                SLONG tmpObjectId = 0;

                // An diese Stelle (links vom Cursor) kommt der Flug hin:
                SLONG Date = Sim.Date + ((ClientPos - PostItBm.Size / SLONG(2) + XY(3, 9)).y - 17) / 19;
                SLONG Time = ((ClientPos - PostItBm.Size / SLONG(2) + XY(3, 9)).x - 24) / 6;

                // Da, wo wir klicken, nehmen wir den Flug raus:
                SLONG ClickDate = Sim.Date + ((ClientPos - XY(0, PostItBm.Size.y / 2) + XY(3, 9)).y - 17) / 19;
                SLONG ClickTime = ((ClientPos - XY(0, PostItBm.Size.y / 2) + XY(3, 9)).x - 24) / 6;

                while (Time < 0) {
                    Time += 24;
                    Date--;
                }
                while (Time > 23) {
                    Time -= 24;
                    Date++;
                }

                // Nachschauen, ob der Flug zu lang ist für das Flugzeug:
                {
                    SLONG VonCityId = 0;
                    SLONG NachCityId = 0;

                    if (CurrentPostItType == 2) // Auftrag
                    {
                        VonCityId = qPlayer.Auftraege[CurrentPostItId].VonCity;
                        NachCityId = qPlayer.Auftraege[CurrentPostItId].NachCity;
                    }
                    if (CurrentPostItType == 4) // Frachtauftrag
                    {
                        VonCityId = qPlayer.Frachten[CurrentPostItId].VonCity;
                        NachCityId = qPlayer.Frachten[CurrentPostItId].NachCity;
                    } else if (CurrentPostItType == 1) // Route
                    {
                        VonCityId = Routen[CurrentPostItId].VonCity;
                        NachCityId = Routen[CurrentPostItId].NachCity;
                    }

                    // if (Cities.CalcDistance (VonCityId, NachCityId)>PlaneTypes[qPlayer.Planes[ActivePlane].TypeId].Reichweite*1000)
                    if (Cities.CalcDistance(VonCityId, NachCityId) > qPlayer.Planes[ActivePlane].ptReichweite * 1000) {
                        qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2310));
                        return;
                    }

                    // SLONG Speed = PlaneTypes[qPlayer.Planes[ActivePlane].TypeId].Geschwindigkeit;
                    SLONG Speed = qPlayer.Planes[ActivePlane].ptGeschwindigkeit;
                    SLONG Dauer = Cities.CalcFlugdauer(VonCityId, NachCityId, Speed);

                    if (Dauer >= 24) {
                        qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2310));
                        return;
                    }
                }

                // Tutorium: Spieler hat Auftrag eingesetzt:
                if ((Sim.IsTutorial != 0) && Sim.Tutorial == 1506) {
                    Sim.Tutorial = 1507;

                    GlowEffects.ReSize(0);

                    qPlayer.Messages.NextMessage();
                    qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_TUTORIUM, 1507));
                    Date = Sim.Date;
                    Time = Sim.GetHour() + 2;
                }

                if (Date == Sim.Date && Time <= Sim.GetHour() + 1) {
                    Time = Sim.GetHour() + 2;
                }

                {
                    if (CurrentPostItType == 2 && qPlayer.Auftraege[CurrentPostItId].InPlan != 0) {
                        if (qPlayer.Auftraege[CurrentPostItId].InPlan == -1) {
                            qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2311), MESSAGE_COMMENT);
                        } else if (qPlayer.Auftraege[CurrentPostItId].InPlan == 1) {
                            qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2312), MESSAGE_COMMENT);
                        }
                    } else {
                        SLONG c = 0;
                        for (c = qPlan.Flug.AnzEntries() - 1; c >= 0; c--) {
                            if (qPlan.Flug[c].ObjectType != 0) {
                                if ((qPlan.Flug[c].Startdate < ClickDate || (qPlan.Flug[c].Startdate == ClickDate && qPlan.Flug[c].Startzeit <= ClickTime)) &&
                                    (qPlan.Flug[c].Landedate > ClickDate || (qPlan.Flug[c].Landedate == ClickDate && qPlan.Flug[c].Landezeit >= ClickTime)) &&
                                    qPlan.Flug[c].ObjectType != 3) {
                                    break;
                                }
                            }
                        }

                        if (c >= 0 &&
                            (qPlan.Flug[c].Startdate > Sim.Date || (qPlan.Flug[c].Startdate == Sim.Date && qPlan.Flug[c].Startzeit > Sim.GetHour() + 1))) {
                            tmpObjectType = qPlan.Flug[c].ObjectType;
                            tmpObjectId = qPlan.Flug[c].ObjectId;
                        } else {
                            c = qPlan.Flug.AnzEntries() - 1;
                        }

                        qPlan.Flug[c] = {};
                        qPlan.Flug[c].Okay = 0;
                        qPlan.Flug[c].Startdate = Date;
                        qPlan.Flug[c].Startzeit = Time;
                        qPlan.Flug[c].ObjectType = CurrentPostItType;
                        qPlan.Flug[c].ObjectId = CurrentPostItId;

                        if (qPlan.Flug[c].ObjectType == 2 && qPlan.Flug[c].ObjectId < 0x100000) {
                            DebugBreak();
                        }

                        if (qPlan.Flug[c].ObjectType == 1) {
                            qPlan.Flug[c].Ticketpreis = qPlayer.RentRouten.RentRouten[Routen(CurrentPostItId)].Ticketpreis;
                            qPlan.Flug[c].TicketpreisFC = qPlayer.RentRouten.RentRouten[Routen(CurrentPostItId)].TicketpreisFC;
                        }

                        // Zahl der Passagiere berechnen:
                        qPlan.Flug[c].FlightChanged();
                        qPlan.Flug[c].CalcPassengers(PlayerNum, qPlayer.Planes[ActivePlane]);
                        // qPlan.Flug[c].CalcPassengers (qPlayer.Planes[ActivePlane].TypeId, PlayerNum, qPlayer.Planes[ActivePlane]);
                        qPlan.Flug[c].PArrived = 0;

                        qPlan.UpdateNextFlight();
                        qPlan.UpdateNextStart();
                        qPlayer.Planes[ActivePlane].CheckFlugplaene(PlayerNum);
                        qPlayer.NetUpdateFlightplan(ActivePlane);
                        qPlayer.UpdateAuftragsUsage();
                        qPlayer.UpdateFrachtauftragsUsage();

                        // Auftrag vom Cursor ersetzen:
                        if ((CurrentPostItType != 1 || ((AtGetAsyncKeyState(ATKEY_SHIFT) & 0xff00) == 0)) &&
                            (CurrentPostItType != 4 || qPlayer.Frachten[CurrentPostItId].TonsOpen <= 0)) {
                            CurrentPostItType = tmpObjectType;
                            CurrentPostItId = tmpObjectId;
                            TookUnderCursorWithThisClick = TRUE;
                        }

                        if (CurrentPostItType == 3) {
                            CurrentPostItType = 0;
                            TookUnderCursorWithThisClick = FALSE;
                        }

                        qPlayer.Blocks[CurrentBlock].RefreshData(PlayerNum);
                        qPlayer.Blocks[CurrentBlock].Refresh(PlayerNum, IsLaptop);
                        if (CurrentPostItType != 0) {
                            PaintPostIt();
                        }
                        GlobeBm.Clear(0);
                        PaintGlobe();
                        PaintGlobeRoutes();
                    }
                }
            }
            // Objekte aus dem Flugplan entnehmen...
            else if (CurrentPostItType == 0 && ClientPos.IfIsWithin(24, 17, 167, 149)) {
                SLONG ActivePlane = pBlock->SelectedId;
                CFlugplan &qPlan = qPlayer.Planes[ActivePlane].Flugplan;
                SLONG Date = Sim.Date + (ClientPos.y - 17) / 19;
                SLONG Time = (ClientPos.x - 25) / 6;
                SLONG c = 0;

                if (Date > Sim.Date || (Date == Sim.Date && Time > Sim.GetHour() + 1)) {
                    for (c = qPlan.Flug.AnzEntries() - 1; c >= 0; c--) {
                        if (qPlan.Flug[c].ObjectType != 0) {
                            if ((qPlan.Flug[c].Startdate < Date || (qPlan.Flug[c].Startdate == Date && qPlan.Flug[c].Startzeit <= Time)) &&
                                (qPlan.Flug[c].Landedate > Date || (qPlan.Flug[c].Landedate == Date && qPlan.Flug[c].Landezeit >= Time)) &&
                                qPlan.Flug[c].ObjectType != 3) {
                                break;
                            }
                        }
                    }

                    if (c >= 0 && qPlan.Flug[c].ObjectType != 3) {
                        if (qPlan.Flug[c].Startdate > Sim.Date || (qPlan.Flug[c].Startdate == Sim.Date && qPlan.Flug[c].Startzeit > Sim.GetHour() + 1)) {
                            CurrentPostItType = qPlan.Flug[c].ObjectType;
                            CurrentPostItId = qPlan.Flug[c].ObjectId;
                            if (CurrentPostItType != 0) {
                                TookUnderCursorWithThisClick = TRUE;
                            }

                            if (qPlan.Flug[c].ObjectType != 1 || ((AtGetAsyncKeyState(ATKEY_SHIFT) & 0xff00) == 0)) {
                                qPlan.Flug[c].ObjectType = 0;
                            }

                            qPlan.UpdateNextFlight();
                            qPlan.UpdateNextStart();
                            qPlayer.Planes[ActivePlane].CheckFlugplaene(PlayerNum);
                            qPlayer.UpdateAuftragsUsage();
                            qPlayer.UpdateFrachtauftragsUsage();
                            qPlayer.NetUpdateFlightplan(ActivePlane);
                            qPlayer.Blocks[CurrentBlock].RefreshData(PlayerNum);
                            qPlayer.Blocks[CurrentBlock].Refresh(PlayerNum, IsLaptop);
                            if (CurrentPostItType != 0) {
                                PaintPostIt();
                            }
                            GlobeBm.Clear(0);
                            PaintGlobe();
                            PaintGlobeRoutes();
                        }
                    }
                }
            }
        }
        // Klick in die Detailsicht eines Flugzeuges?
        else if (pBlock->Index == 0 && pBlock->BlockType == 2 && pBlock->Page == 1) {
            if (ClientPos.IfIsWithin(0, 103 + 52, 169, 121 + 45)) {
                MenuStart(MENU_RENAMEPLANE, pBlock->SelectedId);
            }
        }
        // Klick in die Tagessicht des Schedulers?
        else if (pBlock->Index == 2 && pBlock->BlockType == 2) {
            if (ClientPos.IfIsWithin(25, 5, 169, 149)) {
                if (CurrentPostItType != 0) {
                    SLONG ActivePlane = pBlock->SelectedId;
                    CFlugplan &qPlan = qPlayer.Planes[ActivePlane].Flugplan;
                    SLONG tmpObjectType = 0;
                    SLONG tmpObjectId = 0;
                    SLONG Date = Sim.Date + pBlock->Page;
                    SLONG Time = (ClientPos.y - 5) / 6;

                    if (Date == Sim.Date && Time <= Sim.GetHour() + 1) {
                        Time = Sim.GetHour() + 2;
                    }

                    // Nachschauen, ob der Flug zu lang ist für das Flugzeug:
                    {
                        SLONG VonCityId = 0;
                        SLONG NachCityId = 0;

                        if (CurrentPostItType == 2) // Auftrag
                        {
                            VonCityId = qPlayer.Auftraege[CurrentPostItId].VonCity;
                            NachCityId = qPlayer.Auftraege[CurrentPostItId].NachCity;
                        } else if (CurrentPostItType == 1) // Route
                        {
                            VonCityId = Routen[CurrentPostItId].VonCity;
                            NachCityId = Routen[CurrentPostItId].NachCity;
                        } else if (CurrentPostItType == 4) // Frachtauftrag
                        {
                            VonCityId = qPlayer.Frachten[CurrentPostItId].VonCity;
                            NachCityId = qPlayer.Frachten[CurrentPostItId].NachCity;
                        }

                        // if (Cities.CalcDistance (VonCityId, NachCityId)>PlaneTypes[qPlayer.Planes[ActivePlane].TypeId].Reichweite*1000)
                        if (Cities.CalcDistance(VonCityId, NachCityId) > qPlayer.Planes[ActivePlane].ptReichweite * 1000) {
                            qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2310));
                            return;
                        }

                        // SLONG Speed = PlaneTypes[qPlayer.Planes[ActivePlane].TypeId].Geschwindigkeit;
                        SLONG Speed = qPlayer.Planes[ActivePlane].ptGeschwindigkeit;
                        SLONG Dauer = Cities.CalcFlugdauer(VonCityId, NachCityId, Speed);
                        // SLONG Dauer = (Cities.CalcDistance (VonCityId, NachCityId)/Speed+999)/1000+1+2;

                        if (Dauer >= 24) {
                            qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2310));
                            return;
                        }
                    }

                    {
                        if (CurrentPostItType == 2 && qPlayer.Auftraege[CurrentPostItId].InPlan != 0) {
                            if (qPlayer.Auftraege[CurrentPostItId].InPlan == -1) {
                                qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2311), MESSAGE_COMMENT);
                            } else if (qPlayer.Auftraege[CurrentPostItId].InPlan == 1) {
                                qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2312), MESSAGE_COMMENT);
                            }
                        } else {
                            SLONG c = 0;
                            for (c = qPlan.Flug.AnzEntries() - 1; c >= 0; c--) {
                                if (qPlan.Flug[c].ObjectType != 0) {
                                    if ((qPlan.Flug[c].Startdate < Date || (qPlan.Flug[c].Startdate == Date && qPlan.Flug[c].Startzeit <= Time)) &&
                                        (qPlan.Flug[c].Landedate > Date || (qPlan.Flug[c].Landedate == Date && qPlan.Flug[c].Landezeit >= Time)) &&
                                        qPlan.Flug[c].ObjectType != 3) {
                                        break;
                                    }
                                }
                            }

                            {
                                tmpObjectType = qPlan.Flug[c].ObjectType;
                                tmpObjectId = qPlan.Flug[c].ObjectId;
                            };

                            qPlan.Flug[c] = {};
                            qPlan.Flug[c].Okay = 0;
                            qPlan.Flug[c].Startdate = Date;
                            qPlan.Flug[c].Startzeit = Time;
                            qPlan.Flug[c].ObjectType = CurrentPostItType;
                            qPlan.Flug[c].ObjectId = CurrentPostItId;

                            if (qPlan.Flug[c].ObjectType == 2 && qPlan.Flug[c].ObjectId < 0x100000) {
                                DebugBreak();
                            }

                            if (qPlan.Flug[c].ObjectType == 1) {
                                qPlan.Flug[c].Ticketpreis = qPlayer.RentRouten.RentRouten[Routen(CurrentPostItId)].Ticketpreis;
                                qPlan.Flug[c].TicketpreisFC = qPlayer.RentRouten.RentRouten[Routen(CurrentPostItId)].TicketpreisFC;
                            }

                            // Zahl der Passagiere berechnen:
                            qPlan.Flug[c].FlightChanged();
                            qPlan.Flug[c].CalcPassengers(PlayerNum, qPlayer.Planes[ActivePlane]);
                            // qPlan.Flug[c].CalcPassengers (qPlayer.Planes[ActivePlane].TypeId, PlayerNum, qPlayer.Planes[ActivePlane]);
                            qPlan.Flug[c].PArrived = 0;

                            qPlan.UpdateNextFlight();
                            qPlan.UpdateNextStart();
                            qPlayer.NetUpdateFlightplan(ActivePlane);
                            qPlayer.Planes[ActivePlane].CheckFlugplaene(PlayerNum);
                            qPlayer.UpdateAuftragsUsage();
                            qPlayer.UpdateFrachtauftragsUsage();
                            qPlayer.NetUpdateFlightplan(ActivePlane);

                            // Auftrag vom Cursor ersetzen:
                            CurrentPostItType = tmpObjectType;
                            CurrentPostItId = tmpObjectId;

                            if (CurrentPostItType == 3) {
                                CurrentPostItType = 0;
                            }

                            qPlayer.Blocks[CurrentBlock].RefreshData(PlayerNum);
                            qPlayer.Blocks[CurrentBlock].Refresh(PlayerNum, IsLaptop);
                            if (CurrentPostItType != 0) {
                                PaintPostIt();
                            }
                            GlobeBm.Clear(0);
                            PaintGlobe();
                            PaintGlobeRoutes();
                        }
                    }
                }
                // Objekte aus dem Flugplan entnehmen...
                else if (CurrentPostItType == 0) {
                    SLONG ActivePlane = pBlock->SelectedId;
                    CFlugplan &qPlan = qPlayer.Planes[ActivePlane].Flugplan;
                    SLONG Date = Sim.Date + pBlock->Page;
                    SLONG Time = (ClientPos.y - 5) / 6;
                    SLONG c = 0;

                    if (Date > Sim.Date || (Date == Sim.Date && Time > Sim.GetHour() + 1)) {
                        for (c = qPlan.Flug.AnzEntries() - 1; c >= 0; c--) {
                            if (qPlan.Flug[c].ObjectType != 0) {
                                if ((qPlan.Flug[c].Startdate < Date || (qPlan.Flug[c].Startdate == Date && qPlan.Flug[c].Startzeit <= Time)) &&
                                    (qPlan.Flug[c].Landedate > Date || (qPlan.Flug[c].Landedate == Date && qPlan.Flug[c].Landezeit >= Time)) &&
                                    qPlan.Flug[c].ObjectType != 3) {
                                    break;
                                }
                            }
                        }

                        if (c >= 0 && qPlan.Flug[c].ObjectType != 3) {
                            if (qPlan.Flug[c].Startdate > Sim.Date || (qPlan.Flug[c].Startdate == Sim.Date && qPlan.Flug[c].Startzeit > Sim.GetHour() + 1)) {
                                CurrentPostItType = qPlan.Flug[c].ObjectType;
                                CurrentPostItId = qPlan.Flug[c].ObjectId;
                                qPlan.Flug[c].ObjectType = 0;

                                qPlan.UpdateNextFlight();
                                qPlan.UpdateNextStart();
                                qPlayer.Planes[ActivePlane].CheckFlugplaene(PlayerNum);
                                qPlayer.UpdateAuftragsUsage();
                                qPlayer.UpdateFrachtauftragsUsage();
                                qPlayer.NetUpdateFlightplan(ActivePlane);
                                qPlayer.Blocks[CurrentBlock].RefreshData(PlayerNum);
                                qPlayer.Blocks[CurrentBlock].Refresh(PlayerNum, IsLaptop);
                                if (CurrentPostItType != 0) {
                                    PaintPostIt();
                                }
                                GlobeBm.Clear(0);
                                PaintGlobe();
                                PaintGlobeRoutes();
                            }
                        }
                    }
                }
            }
        }
        // Klick in die Detailsicht einer Stadt?
        else if (pBlock->Index == 0 && pBlock->BlockType == 1 && pBlock->Page == 0) {
            if (ClientPos.IfIsWithin(0, 66, 169, 79)) {
                if (Cities.GetIdFromIndex(Cities(pBlock->SelectedId)) != static_cast<ULONG>(Sim.HomeAirportId)) {
                    MenuStart(MENU_REQUEST, MENU_REQUEST_KILLCITY, Cities(pBlock->SelectedId));
                }
            }
        }
    }
    // Click in Client Area B?
    else if ((pBlock != nullptr) && (IsInClientAreaB != 0) && pBlock->BlockType == 2) {
        // in Index Mode?
        if (pBlock->IndexB == 1) {
            SLONG TableCursor = pBlock->PageB + ClientPosB.y / 26;

            if (TableCursor >= 0 && TableCursor < pBlock->TableB.AnzRows) {
                switch (pBlock->BlockTypeB) {
                // Auftrags-Details:
                case 3:
                    if (ClientPosB.IfIsWithin(3, 0, 15, 155) && ClientPosB.y % 26 >= 10 && ClientPosB.y / 26 + pBlock->PageB < pBlock->TableB.AnzRows) {
                        // Tutorium: Spieler soll Auftrag nehmen:
                        if ((Sim.IsTutorial != 0) && Sim.Tutorial == 1505) {
                            qPlayer.Messages.NextMessage();
                            qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_TUTORIUM, 1510));
                        } else {
                            pBlock->IndexB = 0;
                            pBlock->PageB = 0;
                            pBlock->SelectedIdB = pBlock->TableB.LineIndex[TableCursor];

                            pBlock->AnzPagesB = 2;

                            pBlock->RefreshData(PlayerNum);
                            pBlock->Refresh(PlayerNum, IsLaptop);
                        }
                    } else if (ClientPosB.IfIsWithin(0, 0, 172 - 5, 155)) {
                        CString PlaneName;

                        if (pBlock->TableB.ValueFlags[TableCursor * pBlock->TableB.AnzColums] == 2) {
                            qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2310));
                            break;
                        }
                        if (pBlock->TableB.ValueFlags[TableCursor * pBlock->TableB.AnzColums] == 1) {
                            qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2312));
                            break;
                        }
                        if (pBlock->TableB.ValueFlags[TableCursor * pBlock->TableB.AnzColums] == 4) {
                            qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2316));
                            break;
                        }

                        if (qPlayer.IsAuftragInUse(pBlock->TableB.LineIndex[TableCursor], &PlaneName) == 0) {
                            CurrentPostItType = 2;
                            CurrentPostItId = pBlock->TableB.LineIndex[TableCursor];
                            TookUnderCursorWithThisClick = TRUE;
                            PaintPostIt();

                            // Tutorium: Spieler nimmt Auftrag:
                            if ((Sim.IsTutorial != 0) && Sim.Tutorial == 1505) {
                                Sim.Tutorial = 1506;

                                GlowEffects.ReSize(1);
                                GlowBitmapIndices.ReSize(1);
                                GlowEffects[0] = XY(26 + 20, 52 + 5);
                                GlowBitmapIndices[0] = 4;

                                qPlayer.Messages.NextMessage();
                                qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_TUTORIUM, 1506));
                            }
                        } else {
                            qPlayer.Messages.AddMessage(BERATERTYP_GIRL, bprintf(StandardTexte.GetS(TOKEN_ADVICE, 2300), (LPCTSTR)PlaneName));
                        }
                    }
                    break;

                    // Routen-Details:
                case 4:
                    if (ClientPosB.IfIsWithin(3, 0, 13, 155) && ClientPosB.y % 26 >= 10 && ClientPosB.y / 26 + pBlock->PageB < pBlock->TableB.AnzRows) {
                        pBlock->IndexB = 0;
                        pBlock->PageB = 0;
                        pBlock->SelectedIdB = pBlock->TableB.LineIndex[TableCursor];

                        pBlock->RefreshData(PlayerNum);

                        pBlock->AnzPagesB = 3;
                        pBlock->Refresh(PlayerNum, IsLaptop);
                    } else if (ClientPosB.IfIsWithin(0, 0, 172 - 5, 155)) {
                        // Man kann Route nur verwenden, wenn man auch die Niederlassungen hat:
                        if (pBlock->TableB.ValueFlags[TableCursor * pBlock->TableB.AnzColums] == 2) {
                            qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2310));
                            break;
                        }
                        if (pBlock->TableB.ValueFlags[TableCursor * pBlock->TableB.AnzColums] == 3) {
                            qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2314));
                            break;
                        }
                        if (pBlock->TableB.ValueFlags[TableCursor * pBlock->TableB.AnzColums] == 4) {
                            qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2301));
                            break;
                        }

                        CurrentPostItType = 1;
                        CurrentPostItId = pBlock->TableB.LineIndex[TableCursor];
                        TookUnderCursorWithThisClick = TRUE;
                        PaintPostIt();
                    }
                    break;

                    // Frachtauftrags-Details:
                case 6:
                    if (ClientPosB.IfIsWithin(3, 0, 15, 155) && ClientPosB.y % 26 >= 10 && ClientPosB.y / 26 + pBlock->PageB < pBlock->TableB.AnzRows) {
                        // Tutorium: Spieler soll Auftrag nehmen:
                        if ((Sim.IsTutorial != 0) && Sim.Tutorial == 1505) {
                            qPlayer.Messages.NextMessage();
                            qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_TUTORIUM, 1510));
                        } else {
                            pBlock->IndexB = 0;
                            pBlock->PageB = 0;
                            pBlock->SelectedIdB = pBlock->TableB.LineIndex[TableCursor];

                            pBlock->AnzPagesB = 2;

                            pBlock->RefreshData(PlayerNum);
                            pBlock->Refresh(PlayerNum, IsLaptop);
                        }
                    } else if (ClientPosB.IfIsWithin(0, 0, 172 - 5, 155)) {
                        CString PlaneName;

                        if (pBlock->TableB.ValueFlags[TableCursor * pBlock->TableB.AnzColums] == 2) {
                            qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2310));
                            break;
                        }
                        if (pBlock->TableB.ValueFlags[TableCursor * pBlock->TableB.AnzColums] == 1) {
                            qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2330));
                            break;
                        }

                        //==>+<==
                        // if (!Sim.Players.Players[PlayerNum].IsAuftragInUse (pBlock->TableB.LineIndex[TableCursor], &PlaneName))
                        {
                            CurrentPostItType = 4;
                            CurrentPostItId = pBlock->TableB.LineIndex[TableCursor];
                            TookUnderCursorWithThisClick = TRUE;
                            PaintPostIt();
                        }
                        /*else
                          {
                          qPlayer.Messages.AddMessage (BERATERTYP_GIRL, bprintf (StandardTexte.GetS (TOKEN_ADVICE, 2300), (LPCTSTR)PlaneName));
                          } */
                    }
                    break;
                default:
                    break;
                }
            }
        }
        // Klick in die Route?
        else if (pBlock->IndexB == 0 && pBlock->BlockTypeB == 4 && pBlock->PageB == 0) {
            if (ClientPosB.IfIsWithin(2, 40 - 13 - 2, 169, 118)) {
                PLAYER &qPlayer = Sim.Players.Players[PlayerNum];
                CRentRoute &qRRoute = qPlayer.RentRouten.RentRouten[Routen(pBlock->SelectedIdB)];
                CRentRoute *pRRoute = nullptr;
                SLONG SelectedIdB2 = 0;
                SLONG Cost =
                    CalculateFlightCostRechnerisch(Routen[pBlock->SelectedIdB].VonCity, Routen[pBlock->SelectedIdB].NachCity, 800, 800, -1) * 3 / 180 * 2;

                for (SLONG c = qPlayer.RentRouten.RentRouten.AnzEntries() - 1;; c--) {
                    if (Routen.IsInAlbum(c) != 0) {
                        if (Routen[c].VonCity == Routen[pBlock->SelectedIdB].NachCity && Routen[c].NachCity == Routen[pBlock->SelectedIdB].VonCity) {
                            pRRoute = &qPlayer.RentRouten.RentRouten[c];
                            SelectedIdB2 = c;
                            break;
                        }
                    }
                }

                SLONG ticketPreis = qRRoute.Ticketpreis;
                SLONG ticketPreisFC = qRRoute.TicketpreisFC;
                switch ((ClientPosB.y - 40) / 13) {
                case 2:
                    ticketPreis = Cost / 2 / 10 * 10;
                    ticketPreisFC = ticketPreis * 2;
                    break;
                case 3:
                    ticketPreis = Cost / 10 * 10;
                    ticketPreisFC = ticketPreis * 2;
                    break;
                case 4:
                    ticketPreis = Cost * 2 / 10 * 10;
                    ticketPreisFC = ticketPreis * 2;
                    break;
                case 5:
                    ticketPreis = Cost * 4 / 10 * 10;
                    ticketPreisFC = ticketPreis * 2;
                    break;
                default:
                    break;
                }

                if (ClientPosB.IfIsWithin(148, 40 - 2 - 13, 160, 40 - 2 - 13 + 14)) {
                    if (AtGetAsyncKeyState(ATKEY_CONTROL) / 256 != 0) {
                        ticketPreis += 100 * Sim.Options.OptionTicketPriceIncrement;
                        ticketPreisFC += 200 * Sim.Options.OptionTicketPriceIncrement;
                    } else if (AtGetAsyncKeyState(ATKEY_SHIFT) / 256 != 0) {
                        ticketPreis += 10 * Sim.Options.OptionTicketPriceIncrement;
                        ticketPreisFC += 20 * Sim.Options.OptionTicketPriceIncrement;
                    } else {
                        ticketPreis += Sim.Options.OptionTicketPriceIncrement;
                        ticketPreisFC += 2 * Sim.Options.OptionTicketPriceIncrement;
                    }
                }
                if (ClientPosB.IfIsWithin(160, 40 - 2 - 13, 172, 40 - 2 - 13 + 14)) {
                    if (AtGetAsyncKeyState(ATKEY_CONTROL) / 256 != 0) {
                        ticketPreis -= 100 * Sim.Options.OptionTicketPriceIncrement;
                        ticketPreisFC -= 200 * Sim.Options.OptionTicketPriceIncrement;
                    } else if (AtGetAsyncKeyState(ATKEY_SHIFT) / 256 != 0) {
                        ticketPreis -= 10 * Sim.Options.OptionTicketPriceIncrement;
                        ticketPreisFC -= 20 * Sim.Options.OptionTicketPriceIncrement;
                    } else {
                        ticketPreis -= Sim.Options.OptionTicketPriceIncrement;
                        ticketPreisFC -= 2 * Sim.Options.OptionTicketPriceIncrement;
                    }
                }
                /*if ((ClientPosB.y-27)/13==0)
                  {
                  case  0: ticketPreis+=10; break;
                  case  1: ticketPreis-=10; break;
                  }

                  case 10: ticketPreisFC+=10; break;
                  case 11: ticketPreisFC-=10; break;*/

                qRRoute.Ticketpreis = ticketPreis;
                qRRoute.TicketpreisFC = ticketPreisFC;

                Limit(SLONG(0), qRRoute.Ticketpreis, SLONG(Cost * 16 / 10 * 10));
                Limit(SLONG(0), qRRoute.TicketpreisFC, SLONG(Cost * 16 / 10 * 10 * 3));
                pRRoute->Ticketpreis = qRRoute.Ticketpreis;

                qPlayer.UpdateTicketpreise(pBlock->SelectedIdB, qRRoute.Ticketpreis, qRRoute.TicketpreisFC);
                qPlayer.UpdateTicketpreise(SelectedIdB2, pRRoute->Ticketpreis, pRRoute->TicketpreisFC);
                PLAYER::NetSynchronizeRoutes();
                qPlayer.NetRouteUpdateTicketpreise(pBlock->SelectedIdB, qRRoute.Ticketpreis, qRRoute.TicketpreisFC);
                qPlayer.NetRouteUpdateTicketpreise(SelectedIdB2, pRRoute->Ticketpreis, pRRoute->TicketpreisFC);

                pBlock->RefreshData(PlayerNum);
                pBlock->Refresh(PlayerNum, IsLaptop);
            }

            // Konkurrenzpreise:
            if ((qPlayer.HasBerater(BERATERTYP_INFO) != 0) && ClientPosB.IfIsWithin(2, 128, 166, 167)) {
                CRentRoute &qRRoute = qPlayer.RentRouten.RentRouten[Routen(pBlock->SelectedIdB)];
                SLONG Index = (ClientPosB.y - 128) / 13;

                if (Sim.Players.Players[Index + static_cast<SLONG>(PlayerNum <= Index)].IsOut == 0 &&
                    (Sim.Players.Players[Index + static_cast<SLONG>(PlayerNum <= Index)].RentRouten.RentRouten[Routen(pBlock->SelectedIdB)].Rang != 0U)) {
                    qRRoute.Ticketpreis =
                        Sim.Players.Players[Index + static_cast<SLONG>(PlayerNum <= Index)].RentRouten.RentRouten[Routen(pBlock->SelectedIdB)].Ticketpreis;
                    qRRoute.TicketpreisFC =
                        Sim.Players.Players[Index + static_cast<SLONG>(PlayerNum <= Index)].RentRouten.RentRouten[Routen(pBlock->SelectedIdB)].TicketpreisFC;

                    qPlayer.UpdateTicketpreise(pBlock->SelectedIdB, qRRoute.Ticketpreis, qRRoute.TicketpreisFC);
                    PLAYER::NetSynchronizeRoutes();
                    qPlayer.NetRouteUpdateTicketpreise(pBlock->SelectedIdB, qRRoute.Ticketpreis, qRRoute.TicketpreisFC);

                    pBlock->RefreshData(PlayerNum);
                    pBlock->Refresh(PlayerNum, IsLaptop);
                }
            }
        }
        // Klick in die Detailsicht einer Route?
        else if (pBlock->IndexB == 0 && pBlock->BlockTypeB == 4 && pBlock->PageB == 1) {
            if (ClientPosB.IfIsWithin(2, 144, 169, 157)) {
                SLONG d = 0;
                SLONG RouteB = -1;

                for (d = 0; d < Routen.AnzEntries(); d++) {
                    if ((Routen.IsInAlbum(d) != 0) && Routen[d].VonCity == Routen[pBlock->SelectedIdB].NachCity &&
                        Routen[d].NachCity == Routen[pBlock->SelectedIdB].VonCity) {
                        RouteB = Routen(d);
                        break;
                    }
                }

#ifdef _DEBUG
                if (RouteB == -1)
                    DebugBreak();
#endif

                for (SLONG c = qPlayer.Planes.AnzEntries() - 1; c >= 0; c--) {
                    if (qPlayer.Planes.IsInAlbum(c) != 0) {
                        for (SLONG d = qPlayer.Planes[c].Flugplan.Flug.AnzEntries() - 1; d >= 0; d--) {
                            if (qPlayer.Planes[c].Flugplan.Flug[d].ObjectType == 1) {
                                if (Routen(qPlayer.Planes[c].Flugplan.Flug[d].ObjectId) == Routen(pBlock->SelectedIdB) ||
                                    SLONG(Routen(qPlayer.Planes[c].Flugplan.Flug[d].ObjectId)) == RouteB) {
                                    qPlayer.Messages.AddMessage(BERATERTYP_GIRL,
                                                                bprintf(StandardTexte.GetS(TOKEN_ADVICE, 2315), (LPCTSTR)qPlayer.Planes[c].Name));
                                    return;
                                }
                            }
                        }
                    }
                }

                MenuStart(MENU_REQUEST, MENU_REQUEST_KILLROUTE, Routen(pBlock->SelectedIdB), RouteB);
            }
        }
        // Highlight in die Detailsicht eines Auftrags?
        else if (pBlock->IndexB == 0 && pBlock->BlockTypeB == 3 && pBlock->PageB == 0) {
            if (ClientPosB.IfIsWithin(0, 118, 169, 134)) {
                if (qPlayer.Auftraege[pBlock->SelectedIdB].InPlan == 0) {
                    if (qPlayer.Auftraege[pBlock->SelectedIdB].Strafe != 0) {
                        MenuStart(MENU_REQUEST, MENU_REQUEST_KILLAUFTRAG, qPlayer.Auftraege(pBlock->SelectedIdB));
                    } else {
                        MenuStart(MENU_REQUEST, MENU_REQUEST_KILLAUFTRAG0, qPlayer.Auftraege(pBlock->SelectedIdB));
                    }
                }
            }
        }
        // Highlight in die Detailsicht eines Frachtauftrags?
        else if (pBlock->IndexB == 0 && pBlock->BlockTypeB == 6 && pBlock->PageB == 0) {
            if (ClientPosB.IfIsWithin(00, 118 + 17, 169, 134 + 17)) {
                if (qPlayer.Frachten[pBlock->SelectedIdB].InPlan == 0 &&
                    qPlayer.Frachten[pBlock->SelectedIdB].TonsOpen == qPlayer.Frachten[pBlock->SelectedIdB].TonsLeft) {
                    if (qPlayer.Frachten[pBlock->SelectedIdB].Strafe != 0) {
                        MenuStart(MENU_REQUEST, MENU_REQUEST_KILLFAUFTRAG, qPlayer.Frachten(pBlock->SelectedIdB));
                    } else {
                        MenuStart(MENU_REQUEST, MENU_REQUEST_KILLFAUFTRAG0, qPlayer.Frachten(pBlock->SelectedIdB));
                    }
                }
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Erledigt den Kram, wenn in einen Block geklickt wird:
//--------------------------------------------------------------------------------------------
void CPlaner::HandleRButtonDown() {
    DefaultOnRButtonDown();

    if (KonstruktorFinished != 2) {
        return;
    }

    if (CurrentPostItType != 0 && (Sim.Players.Players[PlayerNum].LaptopBattery >= 0 || IsLaptop == FALSE)) {
        // Auftrag vom Cursor wegwerfen
        CurrentPostItType = 0;
        CurrentPostItId = -1;
    } else {
        if ((Sim.IsTutorial != 0) && Sim.Tutorial <= 1506) {
            return;
        }
        if (MenuIsOpen() != 0) {
            MenuRightClick(gMousePosition);
        } else {
            if (CurrentBlock != -1) {
                if (pBlock->Index != 1 && (IsInClientArea != 0)) {
                    if (IsLaptop == 0) {
                        gMovePaper.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
                    }
                    ButtonIndex();
                } else if (pBlock->IndexB != 1 && (IsInClientAreaB != 0)) {
                    if (IsLaptop == 0) {
                        gMovePaper.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
                    }
                    ButtonIndexB();
                } else {
                    if (IsLaptop != 0) {
                        // Block schließen:
                        if (pBlock->Destructing == 0) {
                            pBlock->Destructing = 1;
                            pBlock->AnimationStart = Sim.TimeSlice;
                        }
                    } else {
                        FensterVisible = FALSE;
                        if (IsLaptop == 0) {
                            gMovePaper.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
                        }

                        GlobeBm.Clear(0);
                        PaintGlobe();
                        PaintGlobeRoutes();
                    }
                }
            } else if ((IsDialogOpen() == 0) && gMousePosition.y < 440) { // Raum verlassen:
                Sim.Players.Players[PlayerNum].LeaveRoom();
            }

            CStdRaum::OnRButtonDown(0, gMousePosition);
        }
    }
}

//--------------------------------------------------------------------------------------------
// Erledigt den Kram, wenn der Cursor losgelassen wird:
//--------------------------------------------------------------------------------------------
void CPlaner::HandleLButtonUp() {
    // Stop dragging...
    CurrentDragId = -1;

    PLAYER &qPlayer = Sim.Players.Players[PlayerNum];

    gMouseLButtonDownTimer = 0;

    if ((gMouseClickPosition - gMousePosition).abs() > 10 && (TookUnderCursorWithThisClick != 0) && (CurrentPostItType != 0) && (Sim.IsTutorial == 0)) {
        if (pBlock != nullptr) {
            CFlugplan &qPlan = Sim.Players.Players[PlayerNum].Planes[pBlock->SelectedId].Flugplan;

            if (pBlock->Index == 0 && pBlock->BlockType == 2 && pBlock->Page == 0) {
                if ((IsInClientArea != 0) && ClientPos.IfIsWithin(24, 17, 167, 149)) {
                    SLONG tmpObjectType = 0;
                    SLONG tmpObjectId = -1;

                    // An diese Stelle (links vom Cursor) kommt der Flug hin:
                    SLONG Date = Sim.Date + ((ClientPos - PostItBm.Size / SLONG(2) + XY(3, 9)).y - 17) / 19;
                    SLONG Time = ((ClientPos - PostItBm.Size / SLONG(2) + XY(3, 9)).x - 24) / 6;

                    // Da, wo wir klicken, nehmen wir den Flug raus:
                    SLONG ClickDate = Sim.Date + ((ClientPos - XY(0, PostItBm.Size.y / 2) + XY(3, 9)).y - 17) / 19;
                    SLONG ClickTime = ((ClientPos - XY(0, PostItBm.Size.y / 2) + XY(3, 9)).x - 24) / 6;

                    while (Time < 0) {
                        Time += 24;
                        Date--;
                    }
                    while (Time > 23) {
                        Time -= 24;
                        Date++;
                    }

                    // Nachschauen, ob der Flug zu lang ist für das Flugzeug:
                    {
                        SLONG VonCityId = 0;
                        SLONG NachCityId = 0;

                        if (CurrentPostItType == 2) // Auftrag
                        {
                            VonCityId = qPlayer.Auftraege[CurrentPostItId].VonCity;
                            NachCityId = qPlayer.Auftraege[CurrentPostItId].NachCity;
                        } else if (CurrentPostItType == 1) // Route
                        {
                            VonCityId = Routen[CurrentPostItId].VonCity;
                            NachCityId = Routen[CurrentPostItId].NachCity;
                        } else if (CurrentPostItType == 4) // Frachtauftrag
                        {
                            VonCityId = qPlayer.Frachten[CurrentPostItId].VonCity;
                            NachCityId = qPlayer.Frachten[CurrentPostItId].NachCity;
                        }

                        // if (Cities.CalcDistance (VonCityId, NachCityId)>PlaneTypes[qPlayer.Planes[pBlock->SelectedId].TypeId].Reichweite*1000)
                        if (Cities.CalcDistance(VonCityId, NachCityId) > qPlayer.Planes[pBlock->SelectedId].ptReichweite * 1000) {
                            qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2310));
                            return;
                        }

                        // SLONG Speed = PlaneTypes[qPlayer.Planes[pBlock->SelectedId].TypeId].Geschwindigkeit;
                        SLONG Speed = qPlayer.Planes[pBlock->SelectedId].ptGeschwindigkeit;
                        SLONG Dauer = Cities.CalcFlugdauer(VonCityId, NachCityId, Speed);

                        if (Dauer >= 24) {
                            qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2310));
                            return;
                        }
                    }

                    if (Date == Sim.Date && Time <= Sim.GetHour() + 1) {
                        Time = Sim.GetHour() + 2;
                    }

                    {
                        if (CurrentPostItType == 2 && qPlayer.Auftraege[CurrentPostItId].InPlan != 0) {
                            if (qPlayer.Auftraege[CurrentPostItId].InPlan == -1) {
                                qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2311), MESSAGE_COMMENT);
                            } else if (qPlayer.Auftraege[CurrentPostItId].InPlan == 1) {
                                qPlayer.Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2312), MESSAGE_COMMENT);
                            }
                        } else {
                            SLONG c = 0;

                            for (c = qPlan.Flug.AnzEntries() - 1; c >= 0; c--) {
                                if (qPlan.Flug[c].ObjectType != 0) {
                                    if ((qPlan.Flug[c].Startdate < ClickDate ||
                                         (qPlan.Flug[c].Startdate == ClickDate && qPlan.Flug[c].Startzeit <= ClickTime)) &&
                                        (qPlan.Flug[c].Landedate > ClickDate ||
                                         (qPlan.Flug[c].Landedate == ClickDate && qPlan.Flug[c].Landezeit >= ClickTime)) &&
                                        qPlan.Flug[c].ObjectType != 3) {
                                        break;
                                    }
                                }
                            }

                            if (c >= 0 &&
                                (qPlan.Flug[c].Startdate > Sim.Date || (qPlan.Flug[c].Startdate == Sim.Date && qPlan.Flug[c].Startzeit > Sim.GetHour() + 1))) {
                                tmpObjectType = qPlan.Flug[c].ObjectType;
                                tmpObjectId = qPlan.Flug[c].ObjectId;
                            } else {
                                c = qPlan.Flug.AnzEntries() - 1;
                            }

                            qPlan.Flug[c] = {};
                            qPlan.Flug[c].Okay = 0;
                            qPlan.Flug[c].Startdate = Date;
                            qPlan.Flug[c].Startzeit = Time;
                            qPlan.Flug[c].ObjectType = CurrentPostItType;
                            qPlan.Flug[c].ObjectId = CurrentPostItId;

                            if (qPlan.Flug[c].ObjectType == 2 && qPlan.Flug[c].ObjectId < 0x100000) {
                                DebugBreak();
                            }

                            if (qPlan.Flug[c].ObjectType == 1) {
                                qPlan.Flug[c].Ticketpreis = qPlayer.RentRouten.RentRouten[Routen(CurrentPostItId)].Ticketpreis;
                                qPlan.Flug[c].TicketpreisFC = qPlayer.RentRouten.RentRouten[Routen(CurrentPostItId)].TicketpreisFC;
                            }

                            // Zahl der Passagiere berechnen:
                            qPlan.Flug[c].FlightChanged();
                            // qPlan.Flug[c].CalcPassengers (qPlayer.Planes[pBlock->SelectedId].TypeId, PlayerNum, qPlayer.Planes[pBlock->SelectedId]);
                            qPlan.Flug[c].CalcPassengers(PlayerNum, qPlayer.Planes[pBlock->SelectedId]);
                            qPlan.Flug[c].PArrived = 0;

                            qPlan.UpdateNextFlight();
                            qPlan.UpdateNextStart();
                            qPlayer.Planes[pBlock->SelectedId].CheckFlugplaene(PlayerNum);
                            qPlayer.UpdateAuftragsUsage();
                            qPlayer.UpdateFrachtauftragsUsage();
                            qPlayer.NetUpdateFlightplan(pBlock->SelectedId);

                            // Auftrag vom Cursor ersetzen:
                            if (CurrentPostItType != 1 || ((AtGetAsyncKeyState(ATKEY_SHIFT) & 0xff00) == 0)) {
                                CurrentPostItType = tmpObjectType;
                                CurrentPostItId = tmpObjectId;
                            }

                            if (CurrentPostItType == 3) {
                                CurrentPostItType = 0;
                            }

                            qPlayer.Blocks[CurrentBlock].RefreshData(PlayerNum);
                            qPlayer.Blocks[CurrentBlock].Refresh(PlayerNum, IsLaptop);
                            if (CurrentPostItType != 0) {
                                PaintPostIt();
                            }
                            GlobeBm.Clear(0);
                            PaintGlobe();
                            PaintGlobeRoutes();
                        }
                    }

                    TookUnderCursorWithThisClick = FALSE;
                } else {
                    CurrentPostItType = 0;
                }
            }
        } else {
            CurrentPostItType = 0;
        }
    }
}

//--------------------------------------------------------------------------------------------
// Erledigt den Kram, wenn der Cursor losgelassen wird:
//--------------------------------------------------------------------------------------------
void CPlaner::HandleRButtonUp() {
    DefaultOnRButtonUp();

    if (KonstruktorFinished != 2) {
        return;
    }

    CStdRaum::OnRButtonUp(0, gMousePosition);
}

//--------------------------------------------------------------------------------------------
// Nächste Seite...
//--------------------------------------------------------------------------------------------
void CPlaner::ButtonNext() {
    if (pBlock != nullptr) {
        if (pBlock->Page / pBlock->PageSize + 1 >= pBlock->AnzPages) {
            return;
        }

        if (IsLaptop == 0) {
            gMovePaper.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
        }

        pBlock->Page += pBlock->PageSize;

        pBlock->Refresh(PlayerNum, IsLaptop);
    }
}

//--------------------------------------------------------------------------------------------
// Nächste Seite für Block B...
//--------------------------------------------------------------------------------------------
void CPlaner::ButtonNextB() {
    if (pBlock != nullptr) {
        if (pBlock->PageB / pBlock->PageSizeB + 1 >= pBlock->AnzPagesB) {
            return;
        }

        if (IsLaptop == 0) {
            gMovePaper.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
        }

        pBlock->PageB += pBlock->PageSizeB;

        pBlock->RefreshData(PlayerNum);
        pBlock->Refresh(PlayerNum, IsLaptop);
    }
}

//--------------------------------------------------------------------------------------------
// Vorherige Seite...
//--------------------------------------------------------------------------------------------
void CPlaner::ButtonPrev() {
    if (pBlock != nullptr) {
        if (pBlock->Page <= 0) {
            return;
        }

        if (IsLaptop == 0) {
            gMovePaper.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
        }

        pBlock->Page -= pBlock->PageSize;

        if (pBlock->Page < 0) {
            pBlock->Page = 0;
        }

        pBlock->Refresh(PlayerNum, IsLaptop);
    }
}

//--------------------------------------------------------------------------------------------
// Vorherige Seite für Block B...
//--------------------------------------------------------------------------------------------
void CPlaner::ButtonPrevB() {
    if (pBlock != nullptr) {
        if (pBlock->PageB <= 0) {
            return;
        }

        if (IsLaptop == 0) {
            gMovePaper.Play(DSBPLAY_NOSTOP, Sim.Options.OptionEffekte * 100 / 7);
        }

        pBlock->PageB -= pBlock->PageSizeB;

        if (pBlock->PageB < 0) {
            pBlock->PageB = 0;
        }

        pBlock->RefreshData(PlayerNum);
        pBlock->Refresh(PlayerNum, IsLaptop);
    }
}

//--------------------------------------------------------------------------------------------
// Block schließen...
//--------------------------------------------------------------------------------------------
void CPlaner::ButtonClose() const {
    if (pBlock != nullptr) {
        pBlock->Destructing = 1;
        pBlock->AnimationStart = Sim.TimeSlice;
    }
}

//--------------------------------------------------------------------------------------------
// Zurück zum Index...
//--------------------------------------------------------------------------------------------
void CPlaner::ButtonIndex() {
    if (pBlock != nullptr) {
        SLONG PostSelected = pBlock->SelectedId;

        pBlock->SelectedId = 0;
        pBlock->Index = TRUE;
        pBlock->Page = 0;
        pBlock->RefreshData(PlayerNum);
        pBlock->Refresh(PlayerNum, IsLaptop);

        for (SLONG c = 0; c < pBlock->Table.AnzRows; c++) {
            if (pBlock->Table.LineIndex[c] == PostSelected) {
                if (pBlock->BlockType == 2) {
                    pBlock->Page = c / 6 * 6;
                } else {
                    pBlock->Page = c / 13 * 13;
                }

                if (pBlock->Page < 0) {
                    pBlock->Page = 0;
                }
                if (pBlock->AnzPages <= 1) {
                    pBlock->Page = 0;
                }

                pBlock->Refresh(PlayerNum, IsLaptop);
            }
        }

        GlobeBm.Clear(0);
        PaintGlobe();
        PaintGlobeRoutes();
    }
}

//--------------------------------------------------------------------------------------------
// Zurück zum Index für den rechten Block...
//--------------------------------------------------------------------------------------------
void CPlaner::ButtonIndexB() {
    if (pBlock != nullptr) {
        SLONG PostSelected = pBlock->SelectedIdB;

        pBlock->SelectedIdB = 0;
        pBlock->IndexB = TRUE;
        pBlock->PageB = 0;
        pBlock->AnzPagesB = max(0, (pBlock->TableB.AnzRows - 1) / 6) + 1;
        pBlock->RefreshData(PlayerNum);
        pBlock->Refresh(PlayerNum, IsLaptop);

        for (SLONG c = 0; c < pBlock->TableB.AnzRows; c++) {
            if (pBlock->TableB.LineIndex[c] == PostSelected) {
                if (IsLaptop != 0) {
                    pBlock->PageB = c - 2;
                } else {
                    pBlock->PageB = c / 6 * 6;
                }

                if (pBlock->PageB < 0) {
                    pBlock->PageB = 0;
                }
                if (pBlock->AnzPagesB <= 1) {
                    pBlock->PageB = 0;
                }

                if (bFullscreen != 0) {
                    // if (IsLaptop) SDL_WarpMouseGlobal(pBlock->ScreenPos.x+232+48+48+15-40, pBlock->ScreenPos.y+72+13-50+26*(c-pBlock->Page));
                    // else          SDL_WarpMouseGlobal(pBlock->ScreenPos.x+232+104+48+90-40-48-21, pBlock->ScreenPos.y+195+13-50-80+26*(c-pBlock->Page));
                }

                pBlock->Refresh(PlayerNum, IsLaptop);
            }
        }

        GlobeBm.Clear(0);
        PaintGlobe();
        PaintGlobeRoutes();
    }
}

//--------------------------------------------------------------------------------------------
// Dreht den Globus; Parameter 1000 oder -1000
//--------------------------------------------------------------------------------------------
void CPlaner::TurnGlobe(SLONG Angle) {
    if (UsedToRotate == 0) {
        GlobeBm.Clear(0);
    }
    EarthAlpha += UWORD(Angle);
    EarthTargetAlpha = EarthAlpha;

    for (SLONG c = 0; c < Sim.Players.AnzPlayers; c++) {
        if (Sim.Players.Players[c].IsOut == 0) {
            if (((Sim.Players.Players[PlayerNum].DisplayPlanes[c] != 0U) && (IsLaptop != 0)) || (c == PlayerNum && (IsLaptop == 0))) {
                Sim.Players.Players[c].Planes.UpdateGlobePos(EarthAlpha);
            }
        }
    }

    PaintGlobe();
    PaintGlobeRoutes();

    UsedToRotate = 2;
}

//--------------------------------------------------------------------------------------------
// Geht mit einem Doppelklick um:
//--------------------------------------------------------------------------------------------
void CPlaner::HandleLButtonDouble() {
    PLAYER &qPlayer = Sim.Players.Players[PlayerNum];

    if (Sim.IsTutorial != 0) {
        return;
    }

    // Click in Client Area B?
    if ((pBlock != nullptr) && (IsInClientAreaB != 0) && pBlock->BlockType == 2 && CurrentPostItType != 0) {
        if (pBlock->Index == 0 && pBlock->BlockType == 2 && pBlock->Page == 0) {
            // in Index Mode?
            if (pBlock->IndexB == 1) {
                SLONG TableCursor = pBlock->PageB + ClientPosB.y / 26;
                CFlugplan &qPlan = Sim.Players.Players[PlayerNum].Planes[pBlock->SelectedId].Flugplan;

                if (TableCursor >= 0 && TableCursor < pBlock->TableB.AnzRows) {
                    switch (pBlock->BlockTypeB) {
                    // Auftrags-Details:
                    case 3:
                        if (!(ClientPosB.IfIsWithin(3, 0, 15, 155) && ClientPosB.y % 26 >= 10 && ClientPosB.y / 26 + pBlock->PageB < pBlock->TableB.AnzRows)) {
                            if (ClientPosB.IfIsWithin(0, 0, 172 - 5, 155)) {
                                CString PlaneName;

                                if (pBlock->TableB.ValueFlags[TableCursor * pBlock->TableB.AnzColums] == 2) {
                                    break;
                                }
                                if (pBlock->TableB.ValueFlags[TableCursor * pBlock->TableB.AnzColums] == 1) {
                                    break;
                                }
                                if (pBlock->TableB.ValueFlags[TableCursor * pBlock->TableB.AnzColums] == 4) {
                                    break;
                                }

                                if (Sim.Players.Players[PlayerNum].IsAuftragInUse(pBlock->TableB.LineIndex[TableCursor], &PlaneName) == 0) {
                                    goto add;
                                }
                            }
                        }
                        break;

                        // Routen-Details:
                    case 4:
                        if (ClientPosB.IfIsWithin(3, 0, 13, 155) && ClientPosB.y % 26 >= 10 && ClientPosB.y / 26 + pBlock->PageB < pBlock->TableB.AnzRows) {
                            pBlock->IndexB = 0;
                            pBlock->PageB = 0;
                            pBlock->SelectedIdB = pBlock->TableB.LineIndex[TableCursor];

                            pBlock->RefreshData(PlayerNum);

                            pBlock->AnzPagesB = 3;
                            pBlock->Refresh(PlayerNum, IsLaptop);
                        } else if (ClientPosB.IfIsWithin(0, 0, 172 - 5, 155)) {
                            if (pBlock->TableB.ValueFlags[TableCursor * pBlock->TableB.AnzColums] == 2) {
                                break;
                            }
                            if (pBlock->TableB.ValueFlags[TableCursor * pBlock->TableB.AnzColums] == 3) {
                                Sim.Players.Players[PlayerNum].Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2314));
                            } else if (pBlock->TableB.ValueFlags[TableCursor * pBlock->TableB.AnzColums] == 4) {
                                Sim.Players.Players[PlayerNum].Messages.AddMessage(BERATERTYP_GIRL, StandardTexte.GetS(TOKEN_ADVICE, 2301));
                            } else {
                            add:
                                SLONG c = 0;
                                for (c = qPlan.Flug.AnzEntries() - 1; c >= 0; c--) {
                                    if (qPlan.Flug[c].ObjectType != 0) {
                                        break;
                                    }
                                }

                                c++;

                                if (c < qPlan.Flug.AnzEntries()) {
                                    qPlan.Flug[c] = {};
                                    qPlan.Flug[c].Okay = 0;

                                    if (c > 0) {
                                        qPlan.Flug[c].Startdate = qPlan.Flug[c - 1].Landedate + static_cast<SLONG>(qPlan.Flug[c - 1].Landezeit == 23);
                                        qPlan.Flug[c].Startzeit = (qPlan.Flug[c - 1].Landezeit + 1) % 24;

                                        if (qPlan.Flug[c].Startdate < Sim.Date ||
                                            (qPlan.Flug[c].Startdate == Sim.Date && qPlan.Flug[c].Startzeit < Sim.GetHour() + 2)) {
                                            qPlan.Flug[c].Startdate = Sim.Date;
                                            qPlan.Flug[c].Startzeit = Sim.GetHour() + 2;
                                        }
                                    } else {
                                        qPlan.Flug[c].Startdate = Sim.Date;
                                        qPlan.Flug[c].Startzeit = Sim.GetHour() + 2;
                                    }

                                    qPlan.Flug[c].ObjectType = CurrentPostItType;
                                    qPlan.Flug[c].ObjectId = CurrentPostItId;

                                    if (qPlan.Flug[c].ObjectType == 2 && qPlan.Flug[c].ObjectId < 0x100000) {
                                        DebugBreak();
                                    }

                                    if (qPlan.Flug[c].ObjectType == 1) {
                                        qPlan.Flug[c].Ticketpreis = Sim.Players.Players[PlayerNum].RentRouten.RentRouten[Routen(CurrentPostItId)].Ticketpreis;
                                        qPlan.Flug[c].TicketpreisFC =
                                            Sim.Players.Players[PlayerNum].RentRouten.RentRouten[Routen(CurrentPostItId)].TicketpreisFC;
                                    }

                                    // Zahl der Passagiere berechnen:
                                    qPlan.Flug[c].FlightChanged();
                                    // qPlan.Flug[c].CalcPassengers (qPlayer.Planes[pBlock->SelectedId].TypeId, PlayerNum, qPlayer.Planes[pBlock->SelectedId]);
                                    qPlan.Flug[c].CalcPassengers(PlayerNum, qPlayer.Planes[pBlock->SelectedId]);
                                    qPlan.Flug[c].PArrived = 0;

                                    qPlan.UpdateNextFlight();
                                    qPlan.UpdateNextStart();
                                    qPlayer.Planes[pBlock->SelectedId].CheckFlugplaene(PlayerNum);
                                    qPlayer.UpdateAuftragsUsage();
                                    qPlayer.UpdateFrachtauftragsUsage();
                                    qPlayer.NetUpdateFlightplan(pBlock->SelectedId);

                                    pBlock->RefreshData(PlayerNum);
                                    pBlock->Refresh(PlayerNum, IsLaptop);
                                    GlobeBm.Clear(0);
                                    PaintGlobe();
                                    PaintGlobeRoutes();
                                }

                                CurrentPostItType = 0;
                                CurrentPostItId = -1;
                            }
                        }
                        break;
                    default:
                        break;
                    }
                }
            }
        }
    }
}
