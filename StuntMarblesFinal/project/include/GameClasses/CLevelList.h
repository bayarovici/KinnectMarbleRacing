#ifndef _C_LEVEL_LIST
  #define _C_LEVEL_LIST

  #include <irrlicht.h>

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;

class CLevelList {
  public:
    class CLevelInfo {
      public:
        CLevelInfo(const c8 *sLevel, IrrlichtDevice *pDevice);

        stringw m_sName,
                m_sInfo,
                m_sFile,
                m_sNetbookFile,
                m_sHiScoreFile;
        s32 m_iPos;
    };

    array<CLevelInfo *> m_aLevels;

    CLevelList(IrrlichtDevice *pDevice);
    ~CLevelList();
};

#endif
