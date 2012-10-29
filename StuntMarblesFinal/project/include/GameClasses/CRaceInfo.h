#ifndef _C_TIME_INFO
  #define _C_TIME_INFO

  #include <irrlicht.h>

using namespace irr;
using namespace core;

/**
 * @class CSectionTime
 * This class is used to store the time of a section
 * @author Christian Keimel / dustbin::games
 */
class CSectionTime {
  protected:
    u32 m_iStart,		/**< start time of the section */
        m_iEnd;			/**< end time of the section */

  public:
    CSectionTime(u32 iStart);				/**< create a new section time object */
    void setEndTime(u32 iEnd);			/**< end a section */
    u32 getTime();									/**< get the time of a finished section ("0" if the section was not finished */
    u32 getStartTime();							/**< get the section's start time */
    void setStartTime(u32 iStart);	/**< set the section's start time */
};

/**
 * @class CLapTime
 * This class modifies the CSectionTime class to make it store lap times
 * @author Christian Keimel / dustbin::games
 * @see CSectionTime
 */
class CLapTime : public CSectionTime {
  protected:
    array<CSectionTime *> m_aCheckpoints;			/**< the section times of a lap */
    u32 m_iPlayerNo;													/**< the index of the player */

  public:
    CLapTime(u32 iTime, u32 iPlayerNo);				/**< initialize a laptime object */
    ~CLapTime();
    void addCheckpoint(CSectionTime *pCp);		/**< add a section time */
    void endLap(u32 iTime);										/**< end the lap */

    u32 getCurrentCheckpointNo();							/**< get the number of the next checkpoint */
    u32 getPlayerNo();												/**< get the index of the player */
};

/**
 * @class CRaceTime
 * This class modifies the CSectionTime class to make it store race times
 * @author Christian Keimel / dustbin::games
 * @see CSectionTime
 */
class CRaceTime : public CSectionTime {
  protected:
    array<CLapTime *> m_aLaps;			/**< the laps of the race */
    bool m_bRaceEnded,							/**< is the race finished? */
         m_bPositionDone;           /**< this flag marks players when the final position has been calculated */
    u32 m_iFinishedPos;             /**< the final position of the player */
    CLapTime *m_pFastestLap;				/**< the fastest lap of the race */

  public:
    CRaceTime(u32 iTime);						/**< start the race */
    ~CRaceTime();

    bool addLap(CLapTime *pLap);		/**< add a lap to the race */
    void endRace(u32 iTime);				/**< end the race */
    void setStartTime(u32 iTime);		/**< set the start time of the race */

    void setFinishedPos(u32 i);     /**< set the final position of the player */
    u32 getFinishedPos();           /**< get the final position of the player */

    CLapTime *getCurrentLap();			/**< get the current lap */
    u32 getCurrentLapNo();					/**< get the number of the current lap */
    bool raceEnded();								/**< was the race ended? */

    void setPositionDone();
    bool getPositionDone();

    CLapTime *getLap(u32 idx);			/**< get a specific lap */
    CLapTime *getFastestLap();			/**< get the fastest lap of the racetime (i.e. the fastest lap of a specific player) */
};

/**
 * @class CRaceInfo
 * This class stores all data necessary to do a race
 * @author Christian Keimel / dustbin::games
 * @see CRaceTime
 * @see CLapTime
 * @see CSectionTime
 */
class CRaceInfo {
  private:
    u32 m_iLaps;											/**< the number of laps of the race */
    stringw m_sLevel,									/**< the track file of the race */
		        m_sHiScoreFile;						/**< the hiScore file of the racetrack */
    bool m_bSplitHorizontal,					/**< horizontal splitscreen? */
         m_bRecordReplay,							/**< record a replay? */
         m_bGhost;                    /**< add a ghost player? */
    c8 m_sGhostFile[0xFF];            /**< the ghost player file */

    array<stringw> m_aPlayers;				/**< the players */
    array<CRaceTime *> m_aRaceTime;		/**< the racetimes of the players */
    array<u32> m_aPositions;					/**< the positions of the players */

    CLapTime *m_pFastestLap;					/**< the fastest lap of the race */

    CRaceTime *getNext(u32 *iPlayer);
  public:
    CRaceInfo();
    ~CRaceInfo();

    void setLaps(u32 iLaps);	/**< set the number of laps */
    u32 getLaps();						/**< get the number of laps */

    void addPlayerName(stringw sName);		/**< add a player */
    stringw getPlayerName(u32 iIdx);			/**< get a specific player */
    u32 getPlayerNo();										/**< get the number of players */

    void setLevel(stringw sLevel);	/**< set the track's level name */
    stringw getLevel();							/**< get the track's level name */

    CRaceTime *getRaceTime(u32 idx);		/**< get the race time of a player */

    void setFastestLap(CLapTime *pFastestLap);	/**< set the fastest lap */
    CLapTime *getFastestLap();									/**< get the fastest lap */
    array<CRaceTime *> getRaceTimeArray();			/**< get an array of the racetimes of the players */
    void setSplitHorizontal(bool b);						/**< set the split horizontal flag */
    bool getSplitHorizontal();									/**< get the split horizontal flag */
    void setGhost(bool b);                      /**< set a ghost player */
    bool getGhost();                            /**< add a ghost player? */
    void setRecordReplay(bool b);								/**< set the record replay flag */
    bool getRecordReplay();											/**< get the record replay flag */
    void setHiScoreFile(const wchar_t *sFile);	/**< set the name of the HiScore file */
    const wchar_t *getHiScoreFile();						/**< get then ame of the HiScore file */
    bool raceFinished();												/**< is the race finished? */
    void calculateResult();											/**< calculate the race result */
    void addFinishedPosition(u32 iPlayer);			/**< add a player to the final standings list */
    u32 getPlayerFinishedPosition(u32 iPlayer);	/**< get the position of a player */
    u32 getPlayerAtFinishedPosition(u32 iPos);  /**< get the playerno at a specific final standings position */
    void setGhostFile(const c8 *sFile);         /**< set the ghost player file */
    const c8 *getGhostFile();                   /**< get the ghost player file */
};
#endif
