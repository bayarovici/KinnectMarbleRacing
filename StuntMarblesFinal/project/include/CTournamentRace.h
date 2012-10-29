#ifndef _C_TOURNAMENT_RACE
  #define _C_TOURNAMENT_RACE

  #include <IState.h>
  #include <CConfigFileManager.h>
  #include <IrrOdeNet/IPacketHandler.h>
  #include <CSerializer.h>
  #include <event/IIrrOdeEventListener.h>

class CTournamentRaceInfo;
class CIrrOdeNetAdapter;
class CTournamentInfo;
class IGameLogic;
class COptions;
class CGame;

using namespace irr;
using namespace ode;

class CRaceInfoGUI {
  protected:
    IGUIStaticText *m_pInfo;

  public:
    CRaceInfoGUI(IrrlichtDevice *pDevice, u32 iRace, CTournamentRaceInfo *pRace, IGUIElement *pParent);
    ~CRaceInfoGUI();

    void setState(u32 iState);
};

class CTournamentRace : public IState, public IEventReceiver, public IConnectionHandler, public IIrrOdeEventListener {
  protected:
    u32 m_iSelect,
        m_iReturnState;
    ISoundEngine *m_pSndEngine;

    ITimer *m_pTimer;
    IGUITab *m_pTab,
            *m_pRaces,
            *m_pStandings;
    IGUITable *m_pTable;
    IGUITabControl *m_pPages;
    CTournamentInfo *m_pTrnInfo;

    array<CRaceInfoGUI *> m_aRaces;
    u32 m_iNextRace,
        m_iState;
    CGame *m_pRace;
    IGameLogic *m_pLogic;
    CIrrOdeNetAdapter *m_pNetAdapter;
    COptions *m_pOptions;

    bool startRace();

  public:
    CTournamentRace(IrrlichtDevice *pDevice, CStateMachine *pStateMachine);
    virtual ~CTournamentRace();

    virtual void activate(IState *pPrevious);
    virtual void deactivate(IState *pNext);
    virtual u32 update();

    virtual bool OnEvent (const SEvent &event);

    void setRace(CGame *pRace);

    void setTournamentInfo(CTournamentInfo *pInfo);
    void setNetAdapter(CIrrOdeNetAdapter *pAdapter) { m_pNetAdapter=pAdapter; }

    virtual void onPeerConnect(u16 iPlayerId);
    virtual void onPeerDisconnect(u16 iPlayerId);

    //IIrrOdeEventListener virtual methods
    virtual bool onEvent(IIrrOdeEvent *pEvent);
    virtual bool handlesEvent(IIrrOdeEvent *pEvent);

    void setOptions(COptions *pOptions) { m_pOptions=pOptions; }
};

#endif
