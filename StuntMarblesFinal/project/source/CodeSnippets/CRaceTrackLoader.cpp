  #include <CRaceTrackLoader.h>

CRaceTrackLoader::CRaceTrackLoader(IrrlichtDevice *pDevice) {
  m_pDevice=pDevice;
  m_pGenerator=new CRaceTrackGenerator(pDevice);
}

CRaceTrackLoader::~CRaceTrackLoader() {
  delete m_pGenerator;
}

scene::IAnimatedMesh *CRaceTrackLoader::createMesh(io::IReadFile *file) {
  m_pGenerator->loadProject(file);
  while (m_pGenerator->step()) printf("*");
  return m_pGenerator->getAnimatedMesh();
}

#ifdef _IRREDIT_PLUGIN
bool CRaceTrackLoader::isALoadableFileExtension (const c8 *filename) const {
  const c8 *s=strrchr(filename,'.');
  return s!=NULL && strcmp(s,".racetrack")==0;
}
#else
bool CRaceTrackLoader::isALoadableFileExtension (const io::path &filename) const {
  core::stringc sExt=filename.subString((u32)filename.findLast('.'),10);
  return sExt==".racetrack";
}
#endif

