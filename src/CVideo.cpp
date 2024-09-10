//============================================================================================
// CVideo.cpp : Ein Smacker Video
//============================================================================================
#include "StdAfx.h"
#include "CVideo.h"
#include <cmath>
#include <smacker.h>
#ifdef _DEBUG
#define new DEBUG_NEW
#undef THIS_FILE
static char THIS_FILE[] = __FILE__;
#endif

//--------------------------------------------------------------------------------------------
// ULONG PlayerNum
//--------------------------------------------------------------------------------------------
CVideo::CVideo(const CString &SmackName) : CStdRaum(FALSE, 0, "", 0) {
    RoomBm.ReSize(640, 480);
    RoomBm.FillWith(0);
    PrimaryBm.BlitFrom(RoomBm);

    FrameNum = 0;
    FrameNext = 0;

    StopMidi();
    SSE::DisableDS();

    gMouseStartup = TRUE;

    pSmack = smk_open_file(FullFilename(SmackName, IntroPath), SMK_MODE_MEMORY);
    if (pSmack == nullptr) {
        return;
    }

    smk_enable_video(pSmack, 1U);

    smk_info_video(pSmack, &Width, &Height, &Scale);
    if (Scale != SMK_FLAG_Y_NONE) {
        Height *= 2;
    }

    unsigned char tracks = 0;
    unsigned char channels[7];
    unsigned char depth[7];
    unsigned long rate[7];
    smk_enable_audio(pSmack, 0, 1U);
    smk_info_audio(pSmack, &tracks, channels, depth, rate);

    SDL_AudioSpec desired{};
    desired.freq = rate[0];
    desired.format = SDL_AUDIO_MASK_SIGNED | (depth[0] & SDL_AUDIO_MASK_BITSIZE);
    desired.channels = channels[0];
    desired.samples = 11760;
    desired.callback = nullptr;
    desired.userdata = nullptr;
    audioDevice = SDL_OpenAudioDevice(nullptr, 0, &desired, nullptr, 0);
    if (audioDevice == 0U) {
        hprintf(SDL_GetError());
    }

    Bitmap.ReSize(XY(Width, Height), CREATE_SYSMEM | CREATE_INDEXED);

    State = smk_first(pSmack);
    DoFrame(TRUE);
    DoAudio();

    PrimaryBm.SetVSync(FALSE);
    SDL_PauseAudioDevice(audioDevice, 0);
}

//--------------------------------------------------------------------------------------------
// CVideo-Fenster zerstören:
//--------------------------------------------------------------------------------------------
CVideo::~CVideo() {
    if ((pRoomLib != nullptr) && (pGfxMain != nullptr)) {
        pGfxMain->ReleaseLib(pRoomLib);
        pRoomLib = nullptr;
    }

    if (audioDevice != 0U) {
        SDL_CloseAudioDevice(audioDevice);
    }
    audioDevice = 0;

    if (pSmack != nullptr) {
        smk_close(pSmack);
    }
    pSmack = nullptr;

    gMouseStartup = FALSE;
    pCursor->SetImage(gCursorBm.pBitmap);

    PrimaryBm.SetVSync(TRUE);

    if (Sim.Options.OptionEnableDigi != 0) {
        gpSSE->EnableDS();
    }
    if (Sim.Options.OptionMusicType != 0) {
        NextMidi();
    }
    SetMidiVolume(Sim.Options.OptionMusik);
}

void CVideo::DoFrame(BOOL isFirst) {
    // Take the next frame:
    if (!isFirst) {
        State = smk_next(pSmack);
    }

    // Copy video frame with line-doubling if needed
    const SB_CBitmapKey Key(*Bitmap.pBitmap);
    const unsigned char *pVideo = smk_get_video(pSmack);
    const SLONG scale_mode = Scale == SMK_FLAG_Y_NONE ? 1 : 2;
    for (ULONG y = 0; y < Height; y++) {
        memcpy(static_cast<BYTE *>(Key.Bitmap) + (y * Key.lPitch), pVideo + ((y / scale_mode) * Key.lPitch), Key.lPitch);
    }

    CalculatePalettemapper(smk_get_palette(pSmack), Bitmap.pBitmap->GetPixelFormat()->palette);

    const DWORD now = AtGetTime();
    DOUBLE usf = 0;
    smk_info_all(pSmack, nullptr, nullptr, &usf);
    FrameNext = now + static_cast<DWORD>(usf / 1000.0);
}

void CVideo::DoAudio() const { SDL_QueueAudio(audioDevice, smk_get_audio(pSmack, 0), smk_get_audio_size(pSmack, 0)); }

/////////////////////////////////////////////////////////////////////////////
// CVideo message handlers
//--------------------------------------------------------------------------------------------
// void CVideo::OnPaint():
//--------------------------------------------------------------------------------------------
void CVideo::OnPaint() {
    if (pSmack == nullptr) {
        return;
    }

    if (FrameNum++ < 2) {
        PrimaryBm.BlitFrom(RoomBm);
    }

    // Die Standard Paint-Sachen kann der Basisraum erledigen
    CStdRaum::OnPaint();

    if (AtGetTime() >= FrameNext && State == SMK_MORE) {
        DoFrame();
        DoAudio();
    }

    PrimaryBm.BlitFrom(Bitmap, 320 - Width / 2, 240 - Height / 2);

    if (State != SMK_MORE) {
        OnVideoDone();
    }
}

//--------------------------------------------------------------------------------------------
// void CVideo::OnLButtonDown(UINT nFlags, CPoint point)
//--------------------------------------------------------------------------------------------
void CVideo::OnLButtonDown(UINT /*nFlags*/, CPoint /*point*/) { OnVideoCancel(); }

//--------------------------------------------------------------------------------------------
// void CVideo::OnRButtonDown(UINT nFlags, CPoint point):
//--------------------------------------------------------------------------------------------
void CVideo::OnRButtonDown(UINT /*nFlags*/, CPoint /*point*/) {
    DefaultOnRButtonDown();
    OnVideoCancel();
}

//--------------------------------------------------------------------------------------------
// BOOL CStdRaum::OnSetCursor(CWnd* pWnd, UINT nHitTest, UINT message) : AG:
//--------------------------------------------------------------------------------------------
BOOL CVideo::OnSetCursor(void *pWnd, UINT nHitTest, UINT message) { return (FrameWnd->OnSetCursor(pWnd, nHitTest, message)); }

//--------------------------------------------------------------------------------------------
// void CStdRaum::OnMouseMove(UINT nFlags, CPoint point): AG:
//--------------------------------------------------------------------------------------------
void CVideo::OnMouseMove(UINT nFlags, CPoint point) { FrameWnd->OnMouseMove(nFlags, point); }

//--------------------------------------------------------------------------------------------
// void CVideo::OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags)
//--------------------------------------------------------------------------------------------
void CVideo::OnKeyDown(UINT nChar, UINT /*nRepCnt*/, UINT /*nFlags*/) { OnVideoCancel(); }
