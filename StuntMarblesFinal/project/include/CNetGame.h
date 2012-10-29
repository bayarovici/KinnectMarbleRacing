#ifndef _C_NET_GAME
  #define _C_NET_GAME

  #include <GameClasses/CTournamentBase.h>
  #include <IrrOdeNet/IPacketHandler.h>
  #include <CConfigFileManager.h>
  #include <CSerializer.h>
  #include <event/IIrrOdeEventListener.h>

using namespace irr;
using namespace ode;

class CIrrOdeNetAdapter;
class CNetPlayer;
class CLevelList;
class IGameLogic;
class CRaceInfo;
class COptions;
class CGame;

enum eNetGameState {
  eSelectState,
  eServer,
  eClient
};

/**
 * @class CNetGame
 * This is the dialog where the user can select whether to connect to a server or to start one
 * @author Christian Keimel / dustbin::games
 */
class CNetGame : public CTournamentBase, public IConnectionHandler, public irr::ode::IIrrOdeEventListener {
  private:
    ITimer *m_pTimer;
    u32 m_iLaps,
        m_iPlayersConnected,
        m_iSelectedTrack;
    eNetGameState m_eState;
    CIrrOdeNetAdapter *m_pNetAdapter;
    wchar_t m_sLevelName[0xFF],
            m_sHiScoreFile[0xFF];
    bool m_bStartRace,
         m_bRecordReplay,
         m_bTournamentInit;

    IGUITab *m_pTab;
    IGUIButton *m_pServer,  /**< "start server" button */
               *m_pClient;  /**< "connect to server" button */

    IGUIStaticText *m_pWait;

    IGUIEditBox *m_pEdServer, /**< "Server IP" edit Box */
                *m_pSrvPort,  /**< "start server" port box */
                *m_pCltPort;  /**< "connect to server" port box */

    stringw m_sServer,    /**< the server IP */
            m_sCltPort,   /**< port number for client to connect */
            m_sSrvPort;   /**< port number for server start */

    IGameLogic *m_pLogic;
    CRaceInfo *m_pInfo;
    CLevelList *m_pLevelList;

    CNetPlayer *m_pServerPlayer;
    array<IGUIStaticText *> m_aPlayerNames;
    bool m_bNetStarted;
    void addPlayerGUI(u32 iNum, const c8 *s);

    virtual void tournamentNameChanged();
    virtual void addTrack(u32 iIdx);
    virtual void delTrack(u32 iIdx);
    virtual void clearList();
    virtual void trackChanged(u32 iIdx, const c8 *sTrack);
    virtual void lapsChanged(u32 iIdx, u16 iLaps);
    virtual bool startTournament();
    virtual void fillTournamentStructure();

    void sendAddTrack(s16 iPeer, u32 iIdx);
    void sendClearList(s16 iPeer);
    void sendTrackChanged(s16 iPeer, u32 iIdx, const c8 *sTrack);
    void sendLapsChanged(s16 iPeer, u32 iIdx, u16 iLaps);

  public:
    CNetGame(IrrlichtDevice *pDevice, CStateMachine *pStateMachine);
    virtual void activate(IState *pPrevious);
    virtual void deactivate(IState *pNext);
    virtual u32 update();

    virtual bool OnEvent (const SEvent &event);

    virtual void readConfig(IXMLReaderUTF8 *pXml);
    virtual void writeConfig(IXMLWriter *pXml);

    virtual void onPeerConnect(u16 iPlayerId);
    virtual void onPeerDisconnect(u16 iPlayerId);

    const wchar_t *getLevelName();
    const wchar_t *getHiScoreFile();

    u32 getLaps();

    //IIrrOdeEventListener virtual methods
    virtual bool onEvent(IIrrOdeEvent *pEvent);
    virtual bool handlesEvent(IIrrOdeEvent *pEvent);
};

#endif
