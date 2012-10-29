#ifndef _C_TOURNAMENT_SELECT
  #define _C_TOURNAMENT_SELECT

  #include <IState.h>
  #include <CConfigFileManager.h>
  #include <GameClasses/CTournamentBase.h>

class COptions;
class CLevelList;
class CRaceInfo;
class CTournamentInfo;
class CTournamentRace;

class CTournament : public CTournamentBase {
  protected:
    u32 m_iHSplit;

    IGUIComboBox *m_pPlayerNo,
                 *m_pHSplit;

    virtual bool startTournament();
    virtual void fillTournamentStructure();

    virtual void tournamentNameChanged() { }
    virtual void addTrack(u32 iIdx) { }
    virtual void delTrack(u32 iIdx) { }
    virtual void clearList() { }
    virtual void trackChanged(u32 iIdx, const c8 *sTrack) { }
    virtual void lapsChanged(u32 iIdx, u16 iLaps) { }

  public:
    CTournament(IrrlichtDevice *pDevice, CStateMachine *pStateMachine);
    virtual ~CTournament();

    virtual void activate(IState *pPrevious);
    virtual void deactivate(IState *pNext);

    virtual bool OnEvent (const SEvent &event);

    virtual void writeConfig(IXMLWriter *pXml);
    virtual void readConfig(IXMLReaderUTF8 *pXml);
};

#endif
