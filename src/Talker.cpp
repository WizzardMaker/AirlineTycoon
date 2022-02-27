//============================================================================================
// Talker.cpp : Die Sprechenden Personen
//============================================================================================
#include "StdAfx.h"

//============================================================================================
//CTalkers::
//============================================================================================
//Konstruktor:
//============================================================================================
CTalkers::CTalkers () : Talkers (TALKER_ANZ)
{
    Init ();
}

//--------------------------------------------------------------------------------------------
//Initialisiert
//--------------------------------------------------------------------------------------------
void CTalkers::Init ()
{
    SLONG c = 0;

    for (c=0; c<Talkers.AnzEntries(); c++)
    {
        Talkers[c].State=Talkers[c].NumRef=0;
        Talkers[c].OwnNumber=c;
        Talkers[c].Locking=0;
    }
}

//--------------------------------------------------------------------------------------------
//Macht die Animationen der Leute:
//--------------------------------------------------------------------------------------------
void CTalkers::Pump ()
{
    SLONG c = 0;

    for (c=0; c<Talkers.AnzEntries(); c++) {
        if (Talkers[c].NumRef != 0)
        {
            if (Talkers[c].State==1 || Talkers[c].State==3)
            {
                Talkers[c].Phase = (Talkers[c].Phase+1)%2;
            }
            else if (Talkers[c].State==2 || Talkers[c].State==4)
            {
                Talkers[c].Phase++;
                if (Talkers[c].Phase==4 && Talkers[c].State==4) { Talkers[c].State=0;
}
                if (Talkers[c].Phase==4 && Talkers[c].State==2) { Talkers[c].State=3;
}
            }
        }
}
}

//============================================================================================
//CTalker::
//============================================================================================
//Ein Gespräch per Telefon oder direkt beginnen
//============================================================================================
void CTalker::StartDialog (BOOL Medium)
{
    if (Medium != 0)
    {
        //Telefon:
        State=2;
        Phase=0;
    }
    else
    {
        State=1;
    }
}

//--------------------------------------------------------------------------------------------
//Ein Gespräch beenden und ggf. auflegen
//--------------------------------------------------------------------------------------------
void CTalker::StopDialog ()
{
    if (State==1) {
        State=0;
    } else
    {
        State=4;
        Phase=0;
    }
}

//--------------------------------------------------------------------------------------------
//Anfangen zu sprechen:
//--------------------------------------------------------------------------------------------
void CTalker::StartTalking ()
{
    Talking = TRUE;
}

//--------------------------------------------------------------------------------------------
//Auf den Gesprächsparter warten
//--------------------------------------------------------------------------------------------
void CTalker::StopTalking ()
{
    Talking = FALSE;
}

//--------------------------------------------------------------------------------------------
//Ist ein bestimmter Charaketer in einem Dialog?
//--------------------------------------------------------------------------------------------
BOOL CTalker::IsBusy () const
{
    return static_cast<BOOL>((State!=0 && State!=4) || (Locking != 0));
}

//--------------------------------------------------------------------------------------------
//Ist ein bestimmter Charaketer in einem Dialog?
//--------------------------------------------------------------------------------------------
BOOL CTalker::IsTalking () const
{
    return static_cast<BOOL>(State!=0);
}

//--------------------------------------------------------------------------------------------
//Blittet die animierte Figur an eine bestimmte Stelle
//--------------------------------------------------------------------------------------------
void CTalker::BlitAt (SBBM &/*Offscreen*/, XY /*Pos*/)
{
    //Offscreen.BlitFromT (TalkerBm, Pos);
}

//--------------------------------------------------------------------------------------------
//Besagt, das die Person verwendet wird:
//--------------------------------------------------------------------------------------------
void CTalker::IncreaseReference ()
{
    NumRef++;

    /*#if (TalkerBm.Size.x==0)
      TalkerBm.ReSize (FullFilename (bprintf ("talker%i.lbm", OwnNumber), ClanPath), SYSRAMBM);*/
}

//--------------------------------------------------------------------------------------------
//Besagt, das Person nicht mehr gebraucht wird:
//--------------------------------------------------------------------------------------------
void CTalker::DecreaseReference ()
{
    if (NumRef>0) { NumRef--;
}

    /*if (NumRef==0)
      TalkerBms.Destroy();*/
}

//--------------------------------------------------------------------------------------------
//Besagt, das die Person gesperrt wird:
//--------------------------------------------------------------------------------------------
void CTalker::IncreaseLocking ()
{
    Locking++;
}

//--------------------------------------------------------------------------------------------
//Besagt, das Person nicht mehr gesperrt wird:
//--------------------------------------------------------------------------------------------
void CTalker::DecreaseLocking ()
{
    if (Locking>0) { Locking--;
}
}

//--------------------------------------------------------------------------------------------
//Speichert ein Talker-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator << (TEAKFILE &File, const CTalker &Talker)
{
    File << Talker.OwnNumber << Talker.State  << Talker.Talking;
    File << Talker.Phase     << Talker.NumRef << Talker.Locking;
    File << Talker.StatePar;

    return (File);
}

//--------------------------------------------------------------------------------------------
//Lädt ein Talker-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator >> (TEAKFILE &File, CTalker &Talker)
{
    File >> Talker.OwnNumber >> Talker.State  >> Talker.Talking;
    File >> Talker.Phase     >> Talker.NumRef >> Talker.Locking;
    File >> Talker.StatePar;

    return (File);
}

//--------------------------------------------------------------------------------------------
//Speichert ein Talkers-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator << (TEAKFILE &File, const CTalkers &Talkers)
{
    File << Talkers.Talkers;

    return (File);
}

//--------------------------------------------------------------------------------------------
//Lädt ein Talkers-Objekt:
//--------------------------------------------------------------------------------------------
TEAKFILE &operator >> (TEAKFILE &File, CTalkers &Talkers)
{
    File >> Talkers.Talkers;

    return (File);
}
