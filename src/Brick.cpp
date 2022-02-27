//============================================================================================
// Brick.Cpp - Funktionen für die Verwaltung der baulichen Elemente:
//============================================================================================
#include "StdAfx.h"

extern SB_CColorFX ColorFX;

SLONG ReadLine(BUFFER_V<UBYTE> &Buffer, SLONG BufferStart, char *Line, SLONG LineLength);

//--------------------------------------------------------------------------------------------
// Die Ausmaße einer Brick-Bitmap zurückgeben:
//--------------------------------------------------------------------------------------------
XY BRICK::GetBitmapDimension() const { return (Bitmap[0L].Size); }

//--------------------------------------------------------------------------------------------
// Einen Brick an Stelle X blitten:
//--------------------------------------------------------------------------------------------
void BRICK::BlitAt(SBBM &Offscreen, BOOL Ansatz, const XY &ScreenPos, SLONG Phase) {
    if (ScreenPos.x + Bitmap[0L].Size.x < -2 || ScreenPos.x > RightAirportClip || (bActive == 0)) {
        return;
    }

    // Wenn man es mit dem Maus-Cursor besonders anfasst:
    if (Ansatz != 0) {
        if (NonTrans == 1) {
            Offscreen.BlitFrom(Bitmap[0L], ScreenPos.x - Bitmap[0L].Size.x, ScreenPos.y - Bitmap[0L].Size.y);
        } else {
            Offscreen.BlitFromT(Bitmap[0L], ScreenPos.x - Bitmap[0L].Size.x, ScreenPos.y - Bitmap[0L].Size.y);
        }
    }
    // Normalfall:
    else {
        if (AnimSpeed != 0U) {
            if (Triggered == 0 || (Editor != 0)) {
                if (Phase == -1) {
                    Phase = (Sim.TickerTime / AnimSpeed) % Bitmap.AnzEntries();
                } else if (Phase >= 100) {
                    Phase = (Phase - 100 + (Sim.TickerTime / AnimSpeed)) % Bitmap.AnzEntries();
                }
            }

            if (Triggered == 90) {
                SLONG c = 0;
                SLONG Target = (Sim.TickerTime / AnimSpeed) % WaitSum;

                for (c = 0; c < Bitmap.AnzEntries(); c++) {
                    Target -= WaitTimes[c];
                    if (Target <= 0) {
                        Phase = c;
                        break;
                    }
                }
            }

            if (NonTrans == 1) {
                Offscreen.BlitFrom(Bitmap[Phase], ScreenPos.x, ScreenPos.y);
            } else {
                Offscreen.BlitFromT(Bitmap[Phase], ScreenPos.x, ScreenPos.y);
            }
        } else // Kein Animierter Brick
        {
            if (NonTrans == 1) {
                Offscreen.BlitFrom(Bitmap[0], ScreenPos.x, ScreenPos.y);
            } else {
                Offscreen.BlitFromT(Bitmap[0L], ScreenPos.x, ScreenPos.y);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Einen Brick an Stelle X blitten + skalieren:
//--------------------------------------------------------------------------------------------
void BRICK::BlitAt(SBBM & /*Offscreen*/, BOOL Ansatz, const XY &p1, const XY &p2) {
    // Hier stand überall früher PrimaryBm statt Offscreen...

    if (Ansatz != 0) {
        if (NonTrans == 1) {
            SBBM::BlitFrom(Bitmap[0L], p1, p2);
        } else {
            SBBM::BlitFromT(Bitmap[0L], p1, p2);
        }
    } else {
        if (NonTrans == 1) {
            SBBM::BlitFrom(Bitmap[0L], p1, p2);
        } else {
            SBBM::BlitFrom(Bitmap[0L], p1, p2);
        }
    }
}

//--------------------------------------------------------------------------------------------
// Einen Brick an Stelle X blitten:
//--------------------------------------------------------------------------------------------
void BRICK::BlitAt(SBPRIMARYBM &Offscreen, BOOL Ansatz, const XY &ScreenPos, SLONG Phase) {
    if (ScreenPos.x + Bitmap[0L].Size.x < -2 || ScreenPos.x > 640 || (bActive == 0)) {
        return;
    }

    // Hier stand überall früher PrimaryBm statt Offscreen...

    // Wenn man es mit dem Maus-Cursor besonders anfasst:
    if (Ansatz != 0) {
        if (NonTrans == 1) {
            Offscreen.BlitFrom(Bitmap[0L], ScreenPos.x - Bitmap[0L].Size.x, ScreenPos.y - Bitmap[0L].Size.y);
        } else {
            Offscreen.BlitFromT(Bitmap[0L], ScreenPos.x - Bitmap[0L].Size.x, ScreenPos.y - Bitmap[0L].Size.y);
        }
    }
    // Normalfall:
    else {
        if (AnimSpeed != 0U) {
            if (Triggered == 0 || (Editor != 0)) {
                if (Phase == -1) {
                    Phase = (Sim.TickerTime / AnimSpeed) % Bitmap.AnzEntries();
                } else if (Phase >= 100) {
                    Phase = (Phase - 100 + (Sim.TickerTime / AnimSpeed)) % Bitmap.AnzEntries();
                }
            }

            if (Triggered == 90) {
                SLONG c = 0;
                SLONG Target = (Sim.TickerTime / AnimSpeed) % WaitSum;

                for (c = 0; c < Bitmap.AnzEntries(); c++) {
                    Target -= WaitTimes[c];
                    if (Target <= 0) {
                        Phase = c;
                        break;
                    }
                }
            }

            if (NonTrans == 1) {
                Offscreen.BlitFrom(Bitmap[Phase], ScreenPos.x, ScreenPos.y);
            } else {
                Offscreen.BlitFromT(Bitmap[Phase], ScreenPos.x, ScreenPos.y);
            }
        } else // Kein Animierter Brick
        {
            if (NonTrans == 1) {
                Offscreen.BlitFrom(Bitmap[0], ScreenPos.x, ScreenPos.y);
            } else if (NonTrans == 2 && (Sim.Options.OptionTransparenz != 0)) {
                ColorFX.BlitTrans(Bitmap[0].pBitmap, &Offscreen.PrimaryBm, XY(ScreenPos.x, ScreenPos.y), nullptr, -1);
            } else {
                Offscreen.BlitFromT(Bitmap[0L], ScreenPos.x, ScreenPos.y);
            }
        }
    }
}

//--------------------------------------------------------------------------------------------
// Einen Brick an Stelle X blitten + skalieren:
//--------------------------------------------------------------------------------------------
void BRICK::BlitAt(SBPRIMARYBM & /*Offscreen*/, BOOL Ansatz, const XY &p1, const XY &p2) {
    // Hier stand überall früher PrimaryBm statt Offscreen...

    if (Ansatz != 0) {
        if (NonTrans == 1) {
            SBPRIMARYBM::BlitFrom(Bitmap[0L], p1, p2);
        } else {
            SBPRIMARYBM::BlitFromT(Bitmap[0L], p1, p2);
        }
    } else {
        if (NonTrans == 1) {
            SBPRIMARYBM::BlitFrom(Bitmap[0L], p1, p2);
        } else {
            SBPRIMARYBM::BlitFrom(Bitmap[0L], p1, p2);
        }
    }
}

//--------------------------------------------------------------------------------------------
// Für Editor: Ist Mauscursor auf Glas oder auf echtem Baustein?
//--------------------------------------------------------------------------------------------
BOOL BRICK::IsGlasAt(SLONG x, SLONG y) { return static_cast<BOOL>(Bitmap[0L].GetPixel(x, y) == 0); }

//--------------------------------------------------------------------------------------------
// Ggf. die Bricks an das neue Zeitalter anpassen:
//--------------------------------------------------------------------------------------------
void BRICK::UpdateBrick() {
    BOOL ReloadNecessary = FALSE;
    long c = 0;

    // Falls Bitmap noch nicht vorhanden ==> laden!
    if (Bitmap.AnzEntries() == 0) {
        ReloadNecessary = TRUE;
    }

    if (ReloadNecessary != 0) {
        // Bild muß (neu) geladen werden:
        SLONG AnzPhases = 0;

        // Wie oft kommt diese Periode drin vor ?
        AnzPhases = graphicIDs.AnzEntries();

        // Sind Angaben vorhanden?
        if (AnzPhases > 1) {
            // Speicher für die Animationsphasen bereitstellen:
            Bitmap.ReSize(AnzPhases);

            // Dies wird jetzt wieder als Zähler verwendet:
            AnzPhases = 0;

            // Das richtige Sub-Bild raussuchen:
            for (c = 0; c < graphicIDs.AnzEntries(); c++) {
                // Bild in Brick-Bitmap einbinden:
                Bitmap[AnzPhases++].ReSize(pGLibBrick, graphicIDs[c]);
                /*TECBM(FullFilename (Filename, BrickPath), c).Size);*/
            }
        } else {
            // Speicher für die Animationsphases bereitstellen:
            Bitmap.ReSize(1);

            Bitmap[0L].ReSize(pGLibBrick, graphicIDs[0]);
        }
    }
}

//--------------------------------------------------------------------------------------------
// Berechnet eine intelligentere Position mit Grids & Co.
//--------------------------------------------------------------------------------------------
XY BRICK::GetIntelligentPosition(SLONG x, SLONG y) {
    XY rc;
    XY LocalOffset;
    SLONG BaseOffsetX = 0;

    LocalOffset = XY(0, 0);

    // Die Dinge oben etwas verschieben:
    if (GetBitmapDimension().y - 2 + y < 210 && (GetAsyncKeyState(VK_MENU) == 0)) {
        LocalOffset.x = 22;
        LocalOffset.y = 5;
    }

    if (GetAsyncKeyState(VK_CONTROL) != 0) {
        return (XY(x, y));
    }

    rc.y = (y - (BaseOffset.y + LocalOffset.y) + Bitmap[0].Size.y * 1000) / Grid.y * Grid.y + (BaseOffset.y + LocalOffset.y) - Bitmap[0].Size.y * 1000;

    if (rc.y < MinY) {
        rc.y = MinY;
    }
    if (rc.y > MaxY) {
        rc.y = MaxY;
    }

    BaseOffsetX = (BaseOffset.x + LocalOffset.x) - (rc.y - (BaseOffset.y + LocalOffset.y)) / 2;

    rc.x = (x - BaseOffsetX + Bitmap[0].Size.x * 1000) / Grid.x * Grid.x + BaseOffsetX - Bitmap[0].Size.x * 1000;

    return (rc);
}

//--------------------------------------------------------------------------------------------
// Lädt nachträglich die Tabelle mit den Bricks:
//--------------------------------------------------------------------------------------------
void BRICKS::ReInit(const CString &TabFilename) {
    // CStdioFile    Tab;
    BUFFER_V<char> Line(300);
    char *TimePointer[150];
    long Id = 0;
    long AnzTimePointer = 0;

    // Load Table header:
    auto FileData = *(LoadCompleteFile(FullFilename(TabFilename, ExcelPath)));
    SLONG FileP = 0;

    /*if (!Tab.Open (FullFilename (TabFilename, ExcelPath), CFile::modeRead))
      {
      TeakLibW_Exception (FNL, ExcNever);
      return;
      }*/

    // Die erste Zeile einlesen
    // Tab.ReadString (Line, 300);
    FileP = ReadLine(FileData, FileP, Line.getData(), 300);

    Bricks.ReSize(MAX_BRICKS);

    while (true) {
        // if (!Tab.ReadString (Line.getData(), 300)) break;
        if (FileP >= FileData.AnzEntries()) {
            break;
        }
        FileP = ReadLine(FileData, FileP, Line.getData(), 300);

        TeakStrRemoveEndingCodes(Line.getData(), "\xd\xa\x1a\r");

        // Tabellenzeile hinzufügen:
        Id = atol(strtok(Line.getData(), ";\x8\"")) + 0x10000000;

        // Hinzufügen (darf noch nicht existieren):
        if (IsInAlbum(Id) != 0) {
            TeakLibW_Exception(FNL, ExcNever);
        }

        BRICK brick;
        brick.Filename = strtok(nullptr, ";\x8\"");

        brick.RamPriority = atoi(strtok(nullptr, ";\x8\""));

        brick.NonTrans = static_cast<UBYTE>(atoi(strtok(nullptr, TabSeparator)));
        brick.Triggered = static_cast<UBYTE>(atoi(strtok(nullptr, TabSeparator)));

        brick.Layer = static_cast<UBYTE>(atoi(strtok(nullptr, TabSeparator)));
        brick.AnimSpeed = static_cast<UBYTE>(atoi(strtok(nullptr, TabSeparator)));
        brick.FloorOffset = atoi(strtok(nullptr, TabSeparator));

        brick.BaseOffset.x = atoi(strtok(nullptr, TabSeparator));
        brick.BaseOffset.y = atoi(strtok(nullptr, TabSeparator));

        brick.Grid.x = atoi(strtok(nullptr, TabSeparator));
        brick.Grid.y = atoi(strtok(nullptr, TabSeparator));

        brick.MinY = atoi(strtok(nullptr, TabSeparator));
        brick.MaxY = atoi(strtok(nullptr, TabSeparator));

        brick.ObstacleType = static_cast<UBYTE>(atoi(strtok(nullptr, TabSeparator)));

        brick.WaitSum = 0;
        brick.WaitTimes.ReSize(50);

        for (AnzTimePointer = 0;; AnzTimePointer++) {
            TimePointer[AnzTimePointer] = strtok(nullptr, " ");

            if (TimePointer[AnzTimePointer] == nullptr) {
                break;
            }

            if (strchr(TimePointer[AnzTimePointer], ':') != nullptr) {
                brick.Triggered = 90;
                brick.WaitTimes[AnzTimePointer] = atoi(strchr(TimePointer[AnzTimePointer], ':') + 1);

                *(strchr(TimePointer[AnzTimePointer], ':')) = 0;
            } else {
                brick.WaitTimes[AnzTimePointer] = 1;
            }

            brick.WaitSum += brick.WaitTimes[AnzTimePointer];
        }

        brick.WaitTimes.ReSize(AnzTimePointer);

        brick.graphicIDs.ReSize(AnzTimePointer);

        for (SLONG c = 0; c < AnzTimePointer; c++) {
            brick.graphicIDs[c] = StringToInt64(TimePointer[c]);
            /*brick.graphicIDs[c]=0;

              for (SLONG d=0; d<strlen(TimePointer[c]); d++)
              brick.graphicIDs[c]+=__int64((TimePointer[c])[d])<<(8*d);*/
        }

        push_back(Id, std::move(brick));
    }
}

//--------------------------------------------------------------------------------------------
// Bringt alle Bricks auf den neuesten Stand:
//--------------------------------------------------------------------------------------------
void BRICKS::UpdateBricks() {
    SLONG c = 0;
    SLONG Anz = 0;
    SLONG Bytes = 0;

    // hprintf ("Updating Bricks.");
    for (c = 0; c < Bricks.AnzEntries(); c++) {
        if (IsInAlbum(c) != 0) {
            Bricks[c].UpdateBrick();
        }
    }
#if 0
    for (d=0; d<20; d++)
        for (c=0; c<Bricks.AnzEntries(); c++)
            if (IsInAlbum(c))
                if (Bricks[c].RamPriority==d && bNoVgaRam==FALSE)
                {
                    SBBM tmpBitmap;

                    tmpBitmap.ReSize (Bricks[c].Bitmap[0l].Size);
                    tmpBitmap.BlitFrom (Bricks[c].Bitmap[0l]);
                    Bricks[c].Bitmap[0l].Destroy();

                    DDCAPS ddcaps;

                    ddcaps.dwSize = sizeof (ddcaps);
                    lpDD->GetCaps (&ddcaps, NULL);

                    VidMemFree=ddcaps.dwVidMemFree;

                    if (Bricks[c].Bitmap[0l].ReSize (tmpBitmap.Size, CREATE_VIDMEM)==DD_OK)
                    {
                        Anz++;
                        Bytes+=2*tmpBitmap.Size.x*tmpBitmap.Size.y;

                        ddcaps.dwSize = sizeof (ddcaps);
                        lpDD->GetCaps (&ddcaps, NULL);
                        Bricks[c].Bitmap[0l].BlitFrom (tmpBitmap);
                    }
                    else
                    {
                        Bricks[c].Bitmap[0l].ReSize (tmpBitmap.Size, CREATE_SYSMEM);
                        Bricks[c].Bitmap[0l].BlitFrom (tmpBitmap);

                        hprintf ("%li Bricks (%li Bytes) out-sourced.", Anz, Bytes);
                        hprintf ("Out of Video-RAM. Using normal RAM.");
                        return;
                    }
                }
#endif
    if (bFirstClass == 0) {
        for (c = 0; c < 8; c++) {
            // Hiermit löschen wir die Smacker-Platzhalter an den Gates. Die dienen beim Editieren als optische Hilfe zur Positionierung, aber im Spiel können
            // wir sie nicht gebrauchen
            (*this)[SLONG(0x10000000 + 760 + c)].Bitmap[0].FillWith(0);
            (*this)[SLONG(0x10000000 + 768 + c)].Bitmap[0].FillWith(0);
        }
    }

    hprintf("%li Bricks (%li Bytes) out-sourced.", Anz, Bytes);
}

//--------------------------------------------------------------------------------------------
// Wirft alle Bricks aus dem Speicher:
//--------------------------------------------------------------------------------------------
void BRICKS::Destroy() {
    Bricks.ReSize(0);

    IsInAlbum(SLONG(0x01000000));
}

//--------------------------------------------------------------------------------------------
// Resuariert alle Bricks, die "lost" sind:
//--------------------------------------------------------------------------------------------
void BRICKS::RestoreBricks() {
#if 0
    for (SLONG c=0; c<Bricks.AnzEntries(); c++)
        if (IsInAlbum(c))
        {
            if (Bricks[c].RamPriority<20 && bNoVgaRam==FALSE)
            {
                if (Bricks[c].Bitmap[0l].pBitmap->GetSurface()->IsLost()!=DD_OK)
                {
                    Bricks[c].Bitmap[0l].pBitmap->GetSurface()->Restore ();

                    Bricks[c].Bitmap[0l].Clear ();

                    SBBM TempBm (pGLibBrick, Bricks[c].graphicIDs[0]);

                    Bricks[c].Bitmap[0l].BlitFrom (TempBm);
                }
            }
        }
#endif
}

//============================================================================================
//============================================================================================
//============================================================================================

//--------------------------------------------------------------------------------------------
// Primitiver Konstruktor:
//--------------------------------------------------------------------------------------------
BUILD::BUILD(long BrickId, const XY &ScreenPos, BOOL Ansatz) {
    BUILD::BrickId = BrickId;
    BUILD::ScreenPos = ScreenPos;
    BUILD::Par = 0;

    if (Ansatz != 0) {
        BUILD::ScreenPos -= Bricks[BrickId].GetBitmapDimension();
    }
}

SLONG BUILD::SortIndex1() const { return Bricks[BrickId].Layer; }
SLONG BUILD::SortIndex2() const { return ScreenPos.y + Bricks[BrickId].GetBitmapDimension().y + Bricks[BrickId].FloorOffset; }
SLONG BUILD::SortIndex3() const { return ScreenPos.x; }

//--------------------------------------------------------------------------------------------
// Speichert einen Build-Eintrag:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator<<(TEAKFILE &File, const BUILD &Build) {
    File << Build.BrickId << Build.ScreenPos << Build.Par;
    return (File);
}

//--------------------------------------------------------------------------------------------
// Lädt einen Build-Eintrag:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator>>(TEAKFILE &File, BUILD &Build) {
    File >> Build.BrickId >> Build.ScreenPos >> Build.Par;
    return (File);
}

//--------------------------------------------------------------------------------------------
// Speichert einen Builds-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator<<(TEAKFILE &File, const BUILDS &Builds) {
    File << *((const ALBUM_V<BUILD> *)&Builds);

    return (File);
}

//--------------------------------------------------------------------------------------------
// Lädt einen Builds-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator>>(TEAKFILE &File, BUILDS &Builds) {
    File >> *((ALBUM_V<BUILD> *)&Builds);

    return (File);
}

//--------------------------------------------------------------------------------------------
// Löscht alle Elemente des Flughafens:
//--------------------------------------------------------------------------------------------
void BUILDS::Clear() { ClearAlbum(); }

//--------------------------------------------------------------------------------------------
// Lädt einen Airport-Anordnung:
//--------------------------------------------------------------------------------------------
void BUILDS::Load(SLONG Hall, SLONG Level) {
    CString Filename;
    SLONG Difficulty = Sim.Difficulty;

    if (Difficulty == DIFF_FREEGAME) {
        Difficulty = DIFF_FREEGAMEMAP;
    }

    // Wenn der Flughafen für einen Level nicht existiert, dann Fallback auf den Difficulty-Level davor probieren
    do {
        Filename = FullFilename(HallFilenames[Hall], MiscPath, 100 * Difficulty + Level);
        Difficulty--;
    } while (Difficulty >= 0 && (Editor == 0) && (DoesFileExist(Filename) == 0));

    if (DoesFileExist(Filename) != 0) {
        TEAKFILE File(Filename, TEAKFILE_READ);

        // Etwas tricky: Den geerbeten shifting-operator der ALBUM_V-Klasse aufrufen:
        File >> *((ALBUM_V<BUILD> *)this);
    } else {
        Clear();
#ifdef _DEBUG
        printf("MP: File not found: %s\n", (const char *)Filename);
#endif
    }
}

//--------------------------------------------------------------------------------------------
// Speichert einen Airport-Anordnung:
//--------------------------------------------------------------------------------------------
void BUILDS::Save(SLONG Hall, SLONG Level) const {
    CString Filename;
    SLONG Difficulty = Sim.Difficulty;

    if (Difficulty == DIFF_FREEGAME) {
        Difficulty = DIFF_FREEGAMEMAP;
    }

    if (Level != 0) {
        Filename = FullFilename(HallFilenames[Hall], MiscPath, 100 * Difficulty + Level);

        TEAKFILE File(Filename, TEAKFILE_WRITE);

        // Etwas tricky: Den geerbten shifting-operator der ALBUM_V-Klasse aufrufen:
        File << *((const ALBUM_V<BUILD> *)this);
    }
}

//--------------------------------------------------------------------------------------------
// Sortiert alle Elemente des Flughafens gemäß ihrer Z-Position:
//--------------------------------------------------------------------------------------------
void BUILDS::Sort() {
    ALBUM_V<BUILD>::Sort();
}
