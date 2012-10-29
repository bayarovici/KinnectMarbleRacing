#ifndef _C_LEVEL_SELECT_GUI
  #define _C_LEVEL_SELECT_GUI

  #include <irrlicht.h>
  #include <irrklang.h>

  #include <GameClasses/CLevelList.h>
  #include <CFollowPathAnimator.h>

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace irrklang;

class CLevelSelectGUI {
  private:
    ISceneManager *m_pPreviewManager;
    IrrlichtDevice *m_pDevice;
    IGUIEnvironment *m_pGuienv;
    IVideoDriver *m_pDriver;
    ISceneManager *m_pSmgr;

    ISoundEngine *m_pSndEngine;
    ITimer *m_pTimer;

    rect<s32> m_cViewPort;
    IGUIListBox *m_pLevels;
    IGUITab *m_pTab;
    IGUIImage *m_pPreview,
              *m_pNoPreview;
    CLevelList *m_pLevelList;

    s32 m_iHiScoreItem;
    CFollowPathAnimator *m_pPreviewAnimator;
    bool m_bOdeInitialized,
         m_bNetbook;

    u32 m_iSelectedLevel;
  public:
    CLevelSelectGUI(IrrlichtDevice *pDevice, ISoundEngine *pSndEngine, ISceneManager *pPreviewManager, bool bNetbook);
    ~CLevelSelectGUI();

    IGUITab *getTab();
    bool update();
    bool isMultiplayer();

    void setSelectedLevel(s32 idx);
    s32 getSelectedLevel();

    const wchar_t *getHiScoreFile();
    const wchar_t *getLevelName();
    const wchar_t *getLevelFile();
    const wchar_t *getLevelInfo();
    const wchar_t *getLevelNotebookFile();

    void drawAll();
};

#endif
