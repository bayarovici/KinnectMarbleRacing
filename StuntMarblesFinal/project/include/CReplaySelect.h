#ifndef _C_REPLAY_SELECT
  #define _C_REPLAY_SELECT

  #include <IState.h>
  #include <irrklang.h>
  #include <CConfigFileManager.h>

  #include <IrrOdeNet/CIrrOdeRePlayer.h>

class CStateMachine;
class CLevelList;

class CReplaySelect : public IState, public IEventReceiver, public IConfigFileReader, public IConfigFileWriter, public IMessageHandler {
  protected:
    u32 m_iRet,
        m_iSteps;

    ISoundEngine *m_pSndEngine;
    CIrrOdeRePlayer *m_pPlayer;
    CLevelList *m_pLevelList;
    stringw m_sReplay,
            m_sLevelFile;;

    array<stringw> m_aPlayers;

    IGUITab *m_pTab;
    IGUIListBox *m_pReplayList;
    IGUIButton *m_pStart,
               *m_pCancel;
    IGUIStaticText *m_pLevel,
                   *m_pTime,
                   *m_pPlayers[4],
                   *m_pPlr[4];

    void updateReplayInfo();

  public:
    CReplaySelect(IrrlichtDevice *pDevice, CStateMachine *pStateMachine);
    virtual ~CReplaySelect();
    virtual void activate(IState *pPrevious);
    virtual void deactivate(IState *pNext);
    virtual u32 update();

    virtual bool OnEvent (const SEvent &event);

    virtual void writeConfig(IXMLWriter *pXml);
    virtual void readConfig(IXMLReaderUTF8 *pXml);

    virtual void handleMessage(IIrrOdeEvent *pMessage);

    const wchar_t *getReplayName();
};

#endif
