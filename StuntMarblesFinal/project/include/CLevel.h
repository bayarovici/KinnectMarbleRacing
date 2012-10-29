#ifndef _C_LEVEL
  #define _C_LEVEL

  #include <IState.h>
  #include <CConfigFileManager.h>

class CLevelList;
class CLevelSelectGUI;
class IGameLogic;
class COptions;
class CRaceInfo;
class CGame;

class CLevel : public IState, public IEventReceiver, public IConfigFileReader, public IConfigFileWriter {
  private:
    class CGhostInfo {
      public:
        c8 m_sPlayer[0xFF];
        u32 m_iSteps;
    };

		bool m_bOdeInitialized,
		     m_bHorizontalSplit,
		     m_bRecordReplay,
		     m_bPlayGhost;

    IGUICheckBox *m_pSelectNOP[2],
                 *m_pSplitHorizontal,
                 *m_pRecordReplay,
                 *m_pPlayGhost;
    IGUITab *m_pSplitTab,
            *m_pGhostTab;
    IGUIWindow *m_pGhostSelect;
    IGUIListBox *m_pGhostFile;
		IGUIButton *m_pStart,
		           *m_pGhostOk,
		           *m_pGhostCancel;
		IGUIComboBox *m_pLaps;
		IGUIStaticText *m_pLevelInfo,
                   *m_pHiScore,
                   *m_pGhostPlayer,
                   *m_pGhostTime;

		wchar_t m_sLevelName[1024],
		        m_sHiScoreFile[1024];
		u32 m_iPlayerNo;
		s32 m_iLaps,
		    m_iSelect,
		    m_iSelectedLevel;

    IGameLogic *m_pLogic;
    ISoundEngine *m_pSndEngine;
    CLevelList *m_pLevels;
    CLevelSelectGUI *m_pLevelSelect;
    COptions *m_pOptions;
    CRaceInfo *m_pInfo;
    CGame *m_pRace;
    array<CGhostInfo *> m_lGhostFiles;

    bool updateLevel();
    void initGame();

    void activateGhostSelect();

  public:
    CLevel(IrrlichtDevice *pDevice, CStateMachine *pStateMachine);
    virtual ~CLevel();
    virtual void activate(IState *pPrevious);
    virtual void deactivate(IState *pNext);
    virtual u32 update();

    virtual bool OnEvent (const SEvent &event);

		const wchar_t *getLevelName();
		const wchar_t *getHiScoreFile();
		u32 getPlayerNo();
		s32 getLaps();

		bool splitHorizontal();
		bool playGhost();

    virtual void writeConfig(IXMLWriter *pXml);
    virtual void readConfig(IXMLReaderUTF8 *pXml);

    void setOptions(COptions *pOptions);

    void setRace(CGame *pRace);
};

#endif
