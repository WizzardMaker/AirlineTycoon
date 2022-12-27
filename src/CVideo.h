#pragma once

class CVideo : public CStdRaum {
    // Construction
  public:
    CVideo(const CString &SmackName);

    // Attributes
  protected:
    smk pSmack;
    char State;
    SBBM Bitmap;

    SLONG FrameNum;
    DWORD FrameNext;
    unsigned long Width{};
    unsigned long Height{};
    unsigned char Scale{};

    void DoFrame(BOOL isFirst = FALSE);
    void DoAudio() const;

  public:
    virtual ~CVideo();

    virtual void OnVideoDone()   {}
    virtual void OnVideoCancel() {}

    // Generated message map functions
  public:
    //{{AFX_MSG(COutro)
    void OnLButtonDown(UINT nFlags, CPoint point) override;
    virtual void OnPaint();
    void OnRButtonDown(UINT nFlags, CPoint point) override;
    BOOL OnSetCursor(void *pWnd, UINT nHitTest, UINT message) override;
    void OnMouseMove(UINT nFlags, CPoint point) override;
    void OnKeyDown(UINT nChar, UINT nRepCnt, UINT nFlags) override;
    //}}AFX_MSG
    // DECLARE_MESSAGE_MAP()

  private:
    SDL_AudioDeviceID audioDevice;
};
