#ifndef _C_RACE_TRACK_LOADER
  #define _C_RACE_TRACK_LOADER

  #include <irrlicht.h>
  #include <CRaceTrackGenerator.h>

using namespace irr;

class CRaceTrackLoader : public scene::IMeshLoader {
  protected:
    CRaceTrackGenerator *m_pGenerator;
    IrrlichtDevice *m_pDevice;

  public:
    CRaceTrackLoader(IrrlichtDevice *pDevice);

    virtual ~CRaceTrackLoader();

    virtual scene::IAnimatedMesh *createMesh(io::IReadFile *file);

    #ifdef _IRREDIT_PLUGIN
      virtual bool isALoadableFileExtension (const c8 *filename) const;
    #else
      virtual bool isALoadableFileExtension (const io::path &filename) const;
    #endif
};

#endif
