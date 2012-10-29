#ifndef _C_TOURNAMENT_INFO
  #define _C_TOURNAMENT_INFO

  #include <irrlicht.h>

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

class CRaceInfo;

/**
 * @class CTournamentRaceInfo
 * This class holds all information about a race in a tournament
 * @author Christian Keimel / dustbin::games
 */
class CTournamentRaceInfo {
  protected:
    wchar_t m_sTrack[0xFF];			/**< the track's level name */
    u32 m_iLaps;								/**< the number of laps */
    bool m_bRaceFinished;				/**< is the race finished? */

  public:
    CTournamentRaceInfo(const wchar_t *sTrack, u32 iLaps);		/**< constructor with level name and lap numbers */
		
    wchar_t *getTrack();	/**< get the track's level name */
    u32 getLaps();				/**< get the number of laps */

    void setRaceFinished(bool b);		/**< set the race finished flag */
    bool raceFinished();						/**< get the race finished flag */
};

/**
 * @class CTournamentInfo
 * This class stores all information about a tournament
 * @author Christian Keimel / dustbin::games
 */
class CTournamentInfo {
  protected:
    u32 m_iPlayerNo;													/**< the number of players */
    wchar_t m_sName[0xFF];										/**< the name of the tournament */
    array<CTournamentRaceInfo *> m_aRaces;		/**< the races of the tournament */
    array<CRaceInfo *> m_aRaceInfo;						/**< the raceinfos of the tournament races */
    array<stringw> m_aPlayerNames;						/**< the player names */
    bool m_bHSplit,														/**< horizontal split? */
         m_bRecordReplay;											/**< record replay? */

  public:
    CTournamentInfo(const wchar_t *sName);
    ~CTournamentInfo();

    u32 getRaceCount();													/**< get the number of races */
    void addRace(CTournamentRaceInfo *pRace);		/**< add a race to the tournament */
    CTournamentRaceInfo *getRace(u32 iIdx);			/**< get a specific race */
    wchar_t *getName();													/**< get the name of the tournament */
    void addRaceInfo(CRaceInfo *pInfo);					/**< add raceinfo of a tournament race */
    CRaceInfo *getRaceInfo(u32 iIdx);						/**< get race info of a tournament race */
    void setPlayerNo(u32 i);										/**< set the number of players */
    u32 getPlayerNo();													/**< get the number of players */
    void setSplitHorizontal(bool b);						/**< set the split horizontal flag */
    bool getSplitHorizontal();									/**< get the split horizontal flag */
    void addPlayerName(const wchar_t *sName);		/**< add a player name */
    const wchar_t *getPlayerName(u32 iIdx);			/**< get a player name */
    void setRecordReplay(bool b);								/**< set the record replay flag */
    bool getRecordReplay();											/**< get the record replay flag */
};

#endif
