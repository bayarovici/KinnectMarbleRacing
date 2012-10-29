#ifndef _C_RACE_TRACK_GENERATOR
  #define _C_RACE_TRACK_GENERATOR

  #include "irrlicht.h"

using namespace irr;

class CRaceTrackGenerator {
  public:
    class CReferencePoint {
      public:
        CReferencePoint(core::vector3df v, f32 fBefore, f32 fAfter, u32 iSteps) {
          m_vPoint=v;
          m_fCutBefore=fBefore;
          m_fCutAfter=fAfter;
          m_iSteps=iSteps;
        }

        core::vector3df m_vPoint;
        f32 m_fCutBefore,
            m_fCutAfter;
        u32 m_iSteps;
    };

  protected:
    void renderRect(core::rect<f32> r, video::SColor cCol);

    void renderState0();
    void renderState1();
    void renderState2();
    void renderState3();
    void renderState4();
    void renderState5();
    void renderState6();
    void renderState7();

    void calculateCurve(core::array<core::vector3df> &aParts, core::vector3df *points, u32 iSteps);

    void calculateStraight(core::array<core::vector3df> &aParts, core::vector3df *points, f32 fSize);

    //*********************
    //* Protected Members *
    //*********************
    core::array<CReferencePoint *> m_aPoints;
    core::array<core::vector3df> m_aCutPoints,
                                 m_aCurved,
                                 m_aOutlineL,
                                 m_aOutlineR,
                                 m_aWallVerts;
    core::array<core::line2df> m_aFlatRaceTrack;
    core::array<core::rect<f32> > m_aGroundRects;
    core::aabbox3df m_cBox;
    scene::IMeshBuffer *m_pRoadBuffer,
                       *m_pWallBuffer,
                       *m_pGroundBuffer;

    core::stringc m_cWorkingDir;

    IrrlichtDevice *m_pDevice;
    video::IVideoDriver *m_pDriver;

    u32 m_iState;
    f32 m_fRoadWidth;
    video::SMaterial m_cMat;

  public:
    CRaceTrackGenerator(IrrlichtDevice *pDevice);

    ~CRaceTrackGenerator();

    void addPoint(core::vector3df v, f32 fCutBefore, f32 fCutAfter, u32 iSteps);

    void cutLines();

    void createCurves();

    void createRoadOutline();

    void createRoadBuffer();

    void createWalls();

    void createWallBuffer();

    void createGroundPlane();

    void render();

    bool step();

    void reset();

    u32 getNumOfPoints();

    CReferencePoint *getPointData(u32 i);

    void removePoint(u32 i);

    u32 getState();

    void clearPoints();

    void addReferencePoint(CReferencePoint *p);

    f32 getRoadWidth();

    void setRoadWidth(f32 f);

    void saveMeshToFile(core::stringc sFile);

    scene::IAnimatedMesh *getAnimatedMesh();

    void saveProject(const char *sFile);
    void loadProject(io::IReadFile *sFile);
    void fillVector(core::vector3df &v, const char *s);
};

#endif
