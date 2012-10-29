#ifndef _C_OPTIONS
  #define _C_OPTIONS

  #include <IState.h>
  #include <CConfigFileManager.h>

class CSettings;
class CGame;

class COptions : public IState, public IEventReceiver, public IConfigFileReader, public IConfigFileWriter {
  private:
    ITimer *m_pTimer;
    bool m_bQuit,
         m_bGhostRec,
         m_bVideoScreen,
         m_bNetBook,
         m_bNetgame,
         m_bServerNetbook;
    u32 m_iVideoFPS,
        m_iVideoSize;

    IGUIEditBox *m_pPlayerNames[2];
    stringw m_sPlayerNames[2];

    CSettings *m_pSettings;
    CGame *m_pGame;
    IGUITabControl *m_pTabCtrl;

    IGUICheckBox *m_pGhostRec,
                 *m_pVideoScreen,
                 *m_pNetBook;
    IGUIComboBox *m_pVideoFPS,
                 *m_pVideoSize;

    ISoundEngine *m_pSndEngine;

    void addPlayerTab(IGUIElement *pParent, u32 iNum);

  public:
    COptions(IrrlichtDevice *pDevice, CStateMachine *pStateMachine);
    virtual void activate(IState *pPrevious);
    virtual void deactivate(IState *pNext);
    virtual u32 update();

    virtual bool OnEvent (const SEvent &event);

    void setGame(CGame *pGame);

		u32 getShadowLevel();
		const wchar_t *getPlayerName(u32 i);

		bool recordGhost() { return m_bGhostRec; }
		bool netbook() { return m_bNetgame?m_bServerNetbook:m_bNetBook; }

		void setServerNetbook(bool b) { m_bNetgame=true; m_bServerNetbook=b; }
		void resetNetgame() { m_bNetgame=false; }

    virtual void writeConfig(IXMLWriter *pXml);
    virtual void readConfig(IXMLReaderUTF8 *pXml);
};

#endif
