#ifndef _C_TOURNAMENT_BASE
  #define _C_TOURNAMENT_BASE

  #include <IState.h>
  #include <CConfigFileManager.h>

class COptions;
class CLevelList;
class CRaceInfo;
class CTournamentInfo;
class CTournamentRace;

class CTournamentBase : public IState, public IEventReceiver, public IConfigFileReader, public IConfigFileWriter {
  protected:
    class CTournamentRaceGUI {
      protected:
        IGUITab *m_pTab;
        IGUIComboBox *m_pTrack,
                     *m_pLaps;
        IGUIStaticText *m_pNum;
        IGUIButton *m_pDel;
        stringw m_sName;
        u32 m_iLaps,
            m_iId;

      public:
        CTournamentRaceGUI(IrrlichtDevice *pDevice, u32 iId, IGUIElement *pParent, bool bCanRemoveRace);
        ~CTournamentRaceGUI();

        const wchar_t *getTrack();
        u32 getLaps();

        void setId(u32 iId);
        u32 getId();

        void setTrack(const wchar_t *sTrack);
        void setLaps(u32 iLaps);
    };

    u32 m_iSelect,
        m_iPlayerNo;
    bool m_bRecordReplay,
         m_bNetClient;

    list<CTournamentRaceGUI *> m_lRaces;

    CTournamentRace *m_pRace;
    ISoundEngine *m_pSndEngine;
    CLevelList *m_pLevels;
    IGUITab *m_pTab,
            *m_pRaceTab,
            *m_pRaceListButtons;

    IGUIButton *m_pAddRace,
               *m_pClearList,
               *m_pStart,
               *m_pCancel,
               *m_pSave,
               *m_pLoad;

    IGUIEditBox *m_pName;
    IGUICheckBox *m_pRecordReplay;
    IGUIComboBox *m_pTournaments;
    CTournamentInfo *m_pTournament;
    COptions *m_pOptions;

    virtual void fillTournamentList();
    virtual void fillTournamentStructure()=0;
    virtual void fillTournamentGUI();
    virtual bool startTournament();

    virtual void tournamentNameChanged()=0;
    virtual void addTrack(u32 iIdx)=0;
    virtual void delTrack(u32 iIdx)=0;
    virtual void clearList()=0;
    virtual void trackChanged(u32 iIdx, const c8 *sTrack)=0;
    virtual void lapsChanged(u32 iIdx, u16 iLaps)=0;

  public:
    CTournamentBase(IrrlichtDevice *pDevice, CStateMachine *pStateMachine);
    virtual ~CTournamentBase();

    virtual void activate(IState *pPrevious);
    virtual void deactivate(IState *pNext);
    virtual u32 update();

    virtual bool OnEvent (const SEvent &event);

    virtual void writeConfig(IXMLWriter *pXml);
    virtual void readConfig(IXMLReaderUTF8 *pXml);

    void setRace(CTournamentRace *pRace);
    void setOptions(COptions *pOptions);
};

#endif
