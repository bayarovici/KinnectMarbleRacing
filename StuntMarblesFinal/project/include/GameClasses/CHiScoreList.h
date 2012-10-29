#ifndef _C_HI_SCORE_LIST
  #define _C_HI_SCORE_LIST

  #include <irrlicht.h>

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

/**
 * @class CHiScoreList
 * This is a class that manages and displays the highscore table
 */
class CHiScoreList {
  private:
    class CHiScoreEntry {
      public:
        wchar_t sName[0xFF];
        u32 iTime;
    };

    /**
     * @class CHiScoreList::_ENTRY
     * This class holds the entries of the highscore table
     */
    list<CHiScoreEntry *> m_lList;  /**< list of highscore entries */
    IrrlichtDevice *m_pDevice;      /**< irrlicht device */
    s32 m_iRet;                     /**< the return value */
    c8 m_sFileName[1024];

  public:
    /**
     * The constructor
     * @param pDevice Irrlicht device for the XML reader and writer
     * @param sFileName the Hiscore list filename to be read
     * @param sLevelName the name of the level
     */
    CHiScoreList(IrrlichtDevice *pDevice, const c8 *sFileName);

    /**
     * The destructor
     */
    ~CHiScoreList();

    s32 insert(u32 iTime, const wchar_t *sName);
    u32 getPositionOf(u32 iTime, const wchar_t *sName);

    /**
     * @brief show the highscore table
     * This function is used to show the highscore table
     * @param aRaceInfo an array of race information to be inserted into the hiscore table
     */
    void show(IGUIStaticText *pListField, IGUIEnvironment *pGuiEnv, const wchar_t *sLevelName);

    /**
     * @brief save the highscore table
     * This method saves the highscore table
     */
    void save();
};

#endif

