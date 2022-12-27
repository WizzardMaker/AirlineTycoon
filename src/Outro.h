// Outro.cpp : Der Render-Outro

/////////////////////////////////////////////////////////////////////////////
// COutro window

class COutro : public CVideo {
    // Construction
  public:
    COutro(const CString &SmackName) : CVideo(SmackName) { }

    void OnVideoDone() override { Sim.Gamestate = GAMESTATE_BOOT; }
    void OnVideoCancel() override { Sim.Gamestate = GAMESTATE_BOOT; }
};

/////////////////////////////////////////////////////////////////////////////
