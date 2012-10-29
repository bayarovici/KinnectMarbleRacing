  #include <CRaceTrackGenerator.h>

#define _COLOR_RED video::SColor(0xFF,0xFF,0,0)
#define _COLOR_BLUE video::SColor(0xFF,0,0,0xFF)
#define _COLOR_GREEN video::SColor(0xFF,0,0xFF,0)
#define _COLOR_YELLOW video::SColor(0xFF,0xFF,0xFF,0)
#define _COLOR_WHITE video::SColor(0xFF,0xFF,0xFF,0xFF)
#define _COLOR_BLACK video::SColor(0xFF,0,0,0)

void CRaceTrackGenerator::renderRect(core::rect<f32> r, video::SColor cCol) {
  core::vector3df v[]={
    core::vector3df(r.LowerRightCorner.X,0.0f,r.LowerRightCorner.Y),
    core::vector3df(r.LowerRightCorner.X,0.0f,r.UpperLeftCorner .Y),
    core::vector3df(r.UpperLeftCorner .X,0.0f,r.UpperLeftCorner .Y),
    core::vector3df(r.UpperLeftCorner .X,0.0f,r.LowerRightCorner.Y)
  };

  for (u32 i=0; i<4; i++) {
    u32 iNext=i<3?i+1:2;
    m_pDriver->draw3DLine(v[i],v[iNext],cCol);
  }
}

void CRaceTrackGenerator::renderState0() {
  if (m_aPoints.size()<3) return;

  bool b=false;
  for (u32 i=0; i<m_aPoints.size(); i++) {
    u32 iNext=i<m_aPoints.size()-1?i+1:0;
    m_pDriver->setMaterial(m_cMat);
    m_pDriver->draw3DLine(m_aPoints[i]->m_vPoint,m_aPoints[iNext]->m_vPoint,b?_COLOR_WHITE:_COLOR_RED);
    b=!b;
  }

  core::vector3df p1=m_aPoints[0]->m_vPoint-core::vector3df(-10,-10,-10),
                  p2=m_aPoints[0]->m_vPoint+core::vector3df( 10, 10, 10);

  core::aabbox3df box=core::aabbox3df(p1,p2);
  m_pDriver->draw3DBox(box,_COLOR_GREEN);
}

void CRaceTrackGenerator::renderState1() {
  u32 iCol=0;
  for (u32 i=0; i<m_aCutPoints.size(); i++) {
    iCol++;
    if (iCol>1) iCol=0;
    video::SColor cCol;

    switch (iCol) {
      case 0: cCol=_COLOR_BLACK; break;
      case 1: cCol=_COLOR_RED; break;
    }

    u32 iNext=i<m_aCutPoints.size()-1?i+1:0;
    m_pDriver->setMaterial(m_cMat);
    m_pDriver->draw3DLine(m_aCutPoints[i],m_aCutPoints[iNext],cCol);
  }
}

void CRaceTrackGenerator::renderState2() {
  u32 iCol=0;
  for (u32 i=0; i<m_aCurved.size(); i++) {
    iCol++;
    if (iCol>1) iCol=0;
    video::SColor cCol;

    switch (iCol) {
      case 0: cCol=_COLOR_BLACK; break;
      case 1: cCol=_COLOR_RED; break;
    }

    u32 iNext=i<m_aCurved.size()-1?i+1:0;
    m_pDriver->setMaterial(m_cMat);
    m_pDriver->draw3DLine(m_aCurved[i],m_aCurved[iNext],cCol);
  }
}

void CRaceTrackGenerator::renderState3() {
  u32 iCol=0;
  for (u32 i=0; i<m_aOutlineL.size(); i++) {
    iCol++;
    if (iCol>1) iCol=0;
    video::SColor cCol;

    switch (iCol) {
      case 0: cCol=_COLOR_BLACK; break;
      case 1: cCol=_COLOR_RED; break;
    }

    u32 iNext=i<m_aOutlineL.size()-1?i+1:0;
    m_pDriver->setMaterial(m_cMat);
    m_pDriver->draw3DLine(m_aOutlineL[i],m_aOutlineL[iNext],cCol);
  }

  iCol=0;
  for (u32 i=0; i<m_aOutlineR.size(); i++) {
    iCol++;
    if (iCol>1) iCol=0;
    video::SColor cCol;

    switch (iCol) {
      case 0: cCol=_COLOR_BLACK; break;
      case 1: cCol=_COLOR_RED; break;
    }

    u32 iNext=i<m_aOutlineR.size()-1?i+1:0;
    m_pDriver->setMaterial(m_cMat);
    m_pDriver->draw3DLine(m_aOutlineR[i],m_aOutlineR[iNext],cCol);
  }
}

void CRaceTrackGenerator::renderState4() {
  video::SMaterial cMat;
  cMat.setTexture(0,m_pDriver->getTexture(m_cWorkingDir+"/data/road.png"));
  cMat.Lighting=false;

  m_pDriver->setMaterial(cMat);
  m_pDriver->drawMeshBuffer(m_pRoadBuffer);
}

void CRaceTrackGenerator::renderState5() {
  renderState4();

  for (u32 i=0; i<m_aWallVerts.size(); i+=4) {
    u32 idx[]={ 0,1,  1,2,  2,0,  1,2,  2,3,  3,1 };

    for (u32 j=0; j<12; j+=2) {
      m_pDriver->setMaterial(m_cMat);

      m_pDriver->draw3DLine(m_aWallVerts[i+idx[j]],m_aWallVerts[i+idx[j+1]],_COLOR_BLACK);
    }
  }
}

void CRaceTrackGenerator::renderState6() {
  renderState4();

  video::SMaterial cMat;
  cMat.setTexture(0,m_pDriver->getTexture(m_cWorkingDir+"/data/wall.png"));
  cMat.Lighting=false;

  m_pDriver->setMaterial(cMat);
  m_pDriver->drawMeshBuffer(m_pWallBuffer);
  m_pDriver->draw3DBox(m_cBox,_COLOR_RED);
}

void CRaceTrackGenerator::calculateCurve(core::array<core::vector3df> &aParts, core::vector3df *points, u32 iSteps) {
  for (u32 i=0; i<iSteps; i++) {
    f32 t=((f32)i/(f32)iSteps);
    core::vector3df fResult;
    fResult=((1-t)*(1-t)*points[0])+(2*t*(1-t)*points[1])+(t*t*points[2]);
    aParts.push_back(fResult);
  }
}

void CRaceTrackGenerator::renderState7() {
  renderState6();
  video::SMaterial cMatGround;
  cMatGround.setTexture(0,m_pDriver->getTexture(m_cWorkingDir+"/data/ground.png"));
  cMatGround.Lighting=false;

  m_pDriver->setMaterial(cMatGround);
  m_pDriver->drawMeshBuffer(m_pGroundBuffer);
}

void CRaceTrackGenerator::calculateStraight(core::array<core::vector3df> &aParts, core::vector3df *points, f32 fSize) {
  core::vector3df cDir=(points[0]-points[1]).normalize(),cPos=points[0];
  f32 fLen=(points[0]-points[1]).getLength(),fPos=0.0f;
  while (fPos<fLen) {
    cPos-=fSize*cDir;
    fPos+=fSize;
    if (fPos<fLen) aParts.push_back(cPos);
  }
}

CRaceTrackGenerator::CRaceTrackGenerator(IrrlichtDevice *pDevice) {
  m_pDevice=pDevice;
  m_pDriver=pDevice->getVideoDriver();
  m_iState=0;
  m_cMat.Lighting=false;
  m_pRoadBuffer=NULL;
  m_pWallBuffer=NULL;
  m_pGroundBuffer=NULL;

  m_cWorkingDir=pDevice->getFileSystem()->getWorkingDirectory();
  m_fRoadWidth=30.0f;
}

CRaceTrackGenerator::~CRaceTrackGenerator() {
  for (u32 i=0; i<m_aPoints.size(); i++) delete m_aPoints[i];
}

void CRaceTrackGenerator::addPoint(core::vector3df v, f32 fCutBefore, f32 fCutAfter, u32 iSteps) {
  m_aPoints.push_back(new CReferencePoint(v,fCutBefore,fCutAfter,iSteps));
}

void CRaceTrackGenerator::cutLines() {
  if (m_iState!=0) return;

  m_aCutPoints.clear();
  for (u32 i=0; i<m_aPoints.size(); i++) {
    u32 iNext=i<m_aPoints.size()-1?i+1:0;
    core::vector3df v1=m_aPoints[i]->m_vPoint,v2=m_aPoints[iNext]->m_vPoint,dir=v2-v1;
    f32 fLen1=dir.getLength()*      m_aPoints[i    ]->m_fCutAfter  ,
        fLen2=dir.getLength()*(1.0f-m_aPoints[iNext]->m_fCutBefore);

    dir.normalize();

    m_aCutPoints.push_back(v1);
    m_aCutPoints.push_back(v1+fLen1*dir);
    m_aCutPoints.push_back(v1+fLen2*dir);
  }
  m_iState=1;
}

void CRaceTrackGenerator::createCurves() {
  if (m_iState!=1) return;

  m_aCurved.clear();
  for (u32 i=0; i<m_aCutPoints.size(); i+=3) {
    u32 iPrev=i>0?i-1:m_aCutPoints.size()-1,iNext=i<m_aCutPoints.size()-1?i+1:0;
    core::vector3df p[]={ m_aCutPoints[iPrev], m_aCutPoints[i], m_aCutPoints[iNext] };
    calculateCurve(m_aCurved,p,m_aPoints[i/3]->m_iSteps);

    u32 iNext2=i+2;
    if (iNext2>m_aCutPoints.size()-1) iNext2-=m_aCutPoints.size()-1;

    core::vector3df s[]={ m_aCutPoints[iNext], m_aCutPoints[iNext2] };
    calculateStraight(m_aCurved,s,m_fRoadWidth);
  }

  m_iState=2;
}

void CRaceTrackGenerator::createRoadOutline() {
  if (m_iState!=2) return;

  m_aOutlineL.clear();
  m_aOutlineR.clear();

  for (u32 i=0; i<m_aCurved.size(); i++) {
    u32 iNext=i<m_aCurved.size()-1?i+1:0;
    core::vector3df v1=m_aCurved[i],v2=m_aCurved[iNext],
                    vNormal=((v2-v1).crossProduct(core::vector3df(0,1,0)));

    vNormal.normalize();
    m_aOutlineL.push_back(v1+m_fRoadWidth*vNormal);
    m_aOutlineR.push_back(v1-m_fRoadWidth*vNormal);
  }

  for (u32 i=0; i<m_aOutlineL.size() && i<m_aOutlineR.size(); i++) {
    if (m_aOutlineL[i].Y!=m_aOutlineR[i].Y) {
      f32 newY=(m_aOutlineL[i].Y+m_aOutlineR[i].Y)/2.0f;
      m_aOutlineL[i].Y=newY;
      m_aOutlineR[i].Y=newY;
    }
  }

  m_iState=3;
}

void CRaceTrackGenerator::createRoadBuffer() {
  if (m_iState!=3) return;

  if (m_pRoadBuffer) delete m_pRoadBuffer;
  core::array<video::S3DVertex> aVertices;
  core::array<u16> aIndices;
  u32 iCol=0;

  f32 fTexCoord=0.0f,fXText=2*m_fRoadWidth,fXTexPos=0.0f;

  for (u32 i=0; i<m_aOutlineL.size() && i<m_aOutlineR.size(); i++) {
    u32 iNext1=i<m_aOutlineL.size()-1?i+1:0,iNext2=i<m_aOutlineR.size()-1?i+1:0;

    core::vector3df v[]={ m_aOutlineL[i], m_aOutlineR[i], m_aOutlineL[iNext1], m_aOutlineR[iNext2] };

    f32 fLen1=(m_aOutlineL[i]-m_aOutlineL[iNext1]).getLength(),
        fLen2=(m_aOutlineR[i]-m_aOutlineR[iNext1]).getLength();

    video::S3DVertex vtx[4];

    vtx[0].TCoords.Y=fTexCoord; vtx[0].TCoords.X=1.0f;
    vtx[1].TCoords.Y=fTexCoord; vtx[1].TCoords.X=0.0f;

    fXTexPos+=fLen1>fLen2?fLen1:fLen2;

    fTexCoord=fXTexPos/fXText;

    vtx[2].TCoords.Y=fTexCoord; vtx[2].TCoords.X=1.0f;
    vtx[3].TCoords.Y=fTexCoord; vtx[3].TCoords.X=0.0f;

    u32 idx=aVertices.size();
    for (u32 j=0; j<4; j++) {
      u32 ix=j<3?j+1:0,ip=j>0?j-1:3;
      vtx[j].Pos=v[j];
      if (j==0 || j==1)
        vtx[j].Normal=core::vector3df(v[j]-v[ip]).crossProduct(v[j]-v[ix]);
      else
        vtx[j].Normal=core::vector3df(v[j]-v[ix]).crossProduct(v[j]-v[ip]);

      vtx[j].Color=_COLOR_WHITE;
      aVertices.push_back(vtx[j]);

      iCol++;
      if (iCol>5) iCol=0;
    }

    aIndices.push_back(idx+1);
    aIndices.push_back(idx  );
    aIndices.push_back(idx+2);

    aIndices.push_back(idx+1);
    aIndices.push_back(idx+2);
    aIndices.push_back(idx+3);
  }

  for (u32 i=0; i<aVertices.size(); i++) {
    core::vector3df v1=aVertices[i].Pos;
    bool b=false;
    for (u32 j=i+1; j<aVertices.size() && !b; j++) {
      if (aVertices[j].Pos==v1) {
        core::vector3df vNorm=(aVertices[i].Normal+aVertices[j].Normal)/2;
        aVertices[i].Normal=vNorm;
        aVertices[j].Normal=vNorm;
        b=true;
      }
    }
  }

  m_pRoadBuffer=new scene::SMeshBuffer();
  m_pRoadBuffer->append(aVertices.const_pointer(),aVertices.size(),aIndices.const_pointer(),aIndices.size());
  m_pRoadBuffer->recalculateBoundingBox();
  m_cBox.addInternalBox(m_pRoadBuffer->getBoundingBox());

  m_iState=4;
}

void CRaceTrackGenerator::createWalls() {
  if (m_iState!=4) return;

  m_aWallVerts.clear();

  for (u32 i=0; i<m_aOutlineL.size(); i++) {
    u32 iNext=i<m_aOutlineL.size()-1?i+1:0;

    core::vector3df v1=m_aOutlineL[i],v2=m_aOutlineL[iNext];

    m_aWallVerts.push_back(v1);
    m_aWallVerts.push_back(v2);
    m_aWallVerts.push_back(core::vector3df(v1.X,0.0f,v1.Z));
    m_aWallVerts.push_back(core::vector3df(v2.X,0.0f,v2.Z));
  }

  for (u32 i=0; i<m_aOutlineR.size(); i++) {
    u32 iNext=i<m_aOutlineR.size()-1?i+1:0;

    core::vector3df v1=m_aOutlineR[i],v2=m_aOutlineR[iNext];

    m_aWallVerts.push_back(core::vector3df(v1.X,0.0f,v1.Z));
    m_aWallVerts.push_back(core::vector3df(v2.X,0.0f,v2.Z));
    m_aWallVerts.push_back(v1);
    m_aWallVerts.push_back(v2);
  }

  m_iState=5;
}

void CRaceTrackGenerator::createWallBuffer() {
  if (m_iState!=5) return;

  if (m_pWallBuffer!=NULL) delete m_pWallBuffer;

  core::array<video::S3DVertex> aVerts;
  core::array<u16> aIndices;

  f32 fTexPosX=0.0f,fTexStep=40.0f,fStepX;

  for (u32 i=0; i<m_aWallVerts.size(); i+=4) {
    u32 idx=aVerts.size();

    video::S3DVertex v[4];

    v[0].Pos=m_aWallVerts[i  ]; v[0].Color=_COLOR_WHITE;
    v[1].Pos=m_aWallVerts[i+1]; v[1].Color=_COLOR_WHITE;
    v[2].Pos=m_aWallVerts[i+2]; v[2].Color=_COLOR_WHITE;
    v[3].Pos=m_aWallVerts[i+3]; v[3].Color=_COLOR_WHITE;

    f32 f1=v[0].Pos.Y,f3=v[2].Pos.Y,
        f2=v[1].Pos.Y,f4=v[3].Pos.Y;

    fStepX=(v[2].Pos-v[3].Pos).getLength()/fTexStep;
    if (f3!=0.0f) fStepX=-fStepX;

    v[0].TCoords.X=fTexPosX+fStepX; v[0].TCoords.Y=-f1/fTexStep;
    v[1].TCoords.X=fTexPosX       ; v[1].TCoords.Y=-f2/fTexStep;
    v[2].TCoords.X=fTexPosX+fStepX; v[2].TCoords.Y=-f3/fTexStep;;
    v[3].TCoords.X=fTexPosX       ; v[3].TCoords.Y=-f4/fTexStep;;

    fTexPosX+=fStepX;

    for (u32 j=0; j<4; j++) aVerts.push_back(v[j]);

    aIndices.push_back(idx+1);
    aIndices.push_back(idx);
    aIndices.push_back(idx+2);

    aIndices.push_back(idx+1);
    aIndices.push_back(idx+2);
    aIndices.push_back(idx+3);
  }

  m_pWallBuffer=new scene::SMeshBuffer();
  m_pWallBuffer->append(aVerts.const_pointer(),aVerts.size(),aIndices.const_pointer(),aIndices.size());
  m_pWallBuffer->recalculateBoundingBox();
  m_cBox.addInternalBox(m_pWallBuffer->getBoundingBox());

  m_iState=6;
}

void CRaceTrackGenerator::createGroundPlane() {
  if (m_iState!=6) return;

  if (m_pGroundBuffer!=NULL) delete m_pGroundBuffer;

  core::aabbox3df cBox=m_pRoadBuffer->getBoundingBox();
  f32 fMinX=10000,fMinZ=10000,fMaxX=-10000,fMaxZ=-10000;
  core::vector3df vEdges[8];
  cBox.getEdges(vEdges);
  fMinX=vEdges[0].X; fMaxX=vEdges[7].X;
  fMinZ=vEdges[0].Z; fMaxZ=vEdges[7].Z;

  f32 fStep=50.0f;
  u32 iX=(u32)((fMaxX-fMinX+(4*fStep))/fStep)-2,iZ=(u32)((fMaxZ-fMinZ+4*fStep)/fStep)-2;

  fMaxX=fMinX+iX*fStep;
  fMaxZ=fMinZ+iZ*fStep;

  u32 iXMinOffset=4,iXMaxOffset=2,
      iZMinOffset=4,iZMaxOffset=2;

  iX+=iXMinOffset+iXMaxOffset; fMinX-=iXMinOffset*fStep; fMaxX+=iZMaxOffset*fStep;
  iZ+=iZMinOffset+iZMaxOffset; fMinZ-=iZMinOffset*fStep; fMaxZ+=iZMaxOffset*fStep;

  m_pGroundBuffer=new scene::SMeshBuffer();

  core::array<video::S3DVertex> aVerts;
  core::array<u16> aIdx;

  for (u32 z=0; z<=iZ; z++)
    for (u32 x=0; x<iX; x++) {
      video::S3DVertex s[4];

      s[0].Pos=core::vector3df(fMinX+fStep* x   ,0.0f,fMinZ+fStep* z   );
      s[1].Pos=core::vector3df(fMinX+fStep*(x+1),0.0f,fMinZ+fStep* z   );
      s[2].Pos=core::vector3df(fMinX+fStep*(x+1),0.0f,fMinZ+fStep*(z+1));
      s[3].Pos=core::vector3df(fMinX+fStep* x   ,0.0f,fMinZ+fStep*(z+1));

      s[0].TCoords.X=0; s[0].TCoords.Y=0;
      s[1].TCoords.X=1; s[1].TCoords.Y=0;
      s[2].TCoords.X=1; s[2].TCoords.Y=1;
      s[3].TCoords.X=0; s[3].TCoords.Y=1;

      s[0].Color=_COLOR_WHITE;
      s[1].Color=_COLOR_WHITE;
      s[2].Color=_COLOR_WHITE;
      s[3].Color=_COLOR_WHITE;

      s[0].Normal=core::vector3df(0,1,0);
      s[1].Normal=core::vector3df(0,1,0);
      s[2].Normal=core::vector3df(0,1,0);
      s[3].Normal=core::vector3df(0,1,0);

      u16 iIdx=aVerts.size();

      aVerts.push_back(s[0]);
      aVerts.push_back(s[1]);
      aVerts.push_back(s[2]);
      aVerts.push_back(s[3]);

      aIdx.push_back(iIdx);
      aIdx.push_back(iIdx+2);
      aIdx.push_back(iIdx+1);

      aIdx.push_back(iIdx);
      aIdx.push_back(iIdx+3);
      aIdx.push_back(iIdx+2);
    }

  m_pGroundBuffer->append(aVerts.const_pointer(),aVerts.size(),aIdx.const_pointer(),aIdx.size());
  m_pGroundBuffer->recalculateBoundingBox();
  m_cBox.addInternalBox(m_pGroundBuffer->getBoundingBox());
  m_iState=7;
}

void CRaceTrackGenerator::render() {
  switch (m_iState) {
    case 0: renderState0(); break;
    case 1: renderState1(); break;
    case 2: renderState2(); break;
    case 3: renderState3(); break;
    case 4: renderState4(); break;
    case 5: renderState5(); break;
    case 6: renderState6(); break;
    case 7: renderState7(); break;
  }
}

bool CRaceTrackGenerator::step() {
  if (m_aPoints.size()<3) return false;

  switch (m_iState) {
    case 0: cutLines(); break;
    case 1: createCurves(); break;
    case 2: createRoadOutline(); break;
    case 3: createRoadBuffer(); break;
    case 4: createWalls(); break;
    case 5: createWallBuffer(); break;
    case 6: createGroundPlane(); break;
    default: return false;
  }
  return true;
}

void CRaceTrackGenerator::reset() {
  m_iState=0;
}

u32 CRaceTrackGenerator::getNumOfPoints() {
  return m_aPoints.size();
}

CRaceTrackGenerator::CReferencePoint *CRaceTrackGenerator::getPointData(u32 i) {
  if (i>=m_aPoints.size()) return NULL;
  return m_aPoints[i];
}

void CRaceTrackGenerator::removePoint(u32 i) {
  if (m_iState!=0 || i>=m_aPoints.size()) return;
  m_aPoints.erase(i);
}

u32 CRaceTrackGenerator::getState() {
  return m_iState;
}

void CRaceTrackGenerator::clearPoints() {
  m_iState=0;
  m_aPoints.clear();
}

void CRaceTrackGenerator::addReferencePoint(CReferencePoint *p) {
  m_aPoints.push_back(p);
}

f32 CRaceTrackGenerator::getRoadWidth() { return m_fRoadWidth; }
void CRaceTrackGenerator::setRoadWidth(f32 f) {
  m_fRoadWidth=f;
}

void CRaceTrackGenerator::saveMeshToFile(core::stringc sFile) {
  scene::SMesh *pMesh=new scene::SMesh();
  if (m_pRoadBuffer!=NULL) pMesh->addMeshBuffer(m_pRoadBuffer);
  if (m_pWallBuffer!=NULL) pMesh->addMeshBuffer(m_pWallBuffer);
  if (m_pGroundBuffer!=NULL) pMesh->addMeshBuffer(m_pGroundBuffer);

  scene::IMeshWriter *pWriter=m_pDevice->getSceneManager()->createMeshWriter(scene::EMWT_STL);
  if (pWriter) {
    #ifdef _IRREDIT_PLUGIN
      io::IWriteFile *pFile=m_pDevice->getFileSystem()->createAndWriteFile(sFile.c_str(),false);
    #else
      io::IWriteFile *pFile=m_pDevice->getFileSystem()->createAndWriteFile(sFile,false);
    #endif
    pWriter->writeMesh(pFile,pMesh);
  }

  pMesh->drop();
}

scene::IAnimatedMesh *CRaceTrackGenerator::getAnimatedMesh() {
  scene::SMesh *pMesh=new scene::SMesh();

  if (m_pRoadBuffer!=NULL) pMesh->addMeshBuffer(m_pRoadBuffer);
  if (m_pWallBuffer!=NULL) pMesh->addMeshBuffer(m_pWallBuffer);
  if (m_pGroundBuffer!=NULL) pMesh->addMeshBuffer(m_pGroundBuffer);

  pMesh->recalculateBoundingBox();

  scene::SAnimatedMesh *pRet=new irr::scene::SAnimatedMesh(pMesh);
  pRet->recalculateBoundingBox();
  return pRet;
}

void CRaceTrackGenerator::loadProject(io::IReadFile *sFile) {
  io::IXMLReader *p=m_pDevice->getFileSystem()->createXMLReader(sFile);
  if (p) {
    clearPoints();
    while (p->read()) {
      if (p->getNodeType()==io::EXN_ELEMENT) {
        if (!wcscmp(p->getNodeName(),L"RaceTrack")) {
          f32 f=p->getAttributeValueAsFloat(L"roadwidth");
          setRoadWidth(f);
        }

        if (!wcscmp(p->getNodeName(),L"referencepoint")) {
          core::stringc s=core::stringc(p->getAttributeValue(L"point"));
          f32 f1=p->getAttributeValueAsFloat(L"before"),f2=p->getAttributeValueAsFloat(L"after");
          u32 i=p->getAttributeValueAsInt(L"steps");

          core::vector3df v;
          fillVector(v,s.c_str());
          addPoint(v,f1,f2,i);
        }
      }
    }
  }
}

void CRaceTrackGenerator::saveProject(const char *sFile) {
  io::IXMLWriter *p=m_pDevice->getFileSystem()->createXMLWriter(sFile);
  if (p) {
    wchar_t s[0xFF];
    swprintf(s,0xFF,L"%f",getRoadWidth());
    while (wcschr(s,L',')) *wcschr(s,L',')=L'.';

    p->writeXMLHeader();
    p->writeLineBreak();
    p->writeElement(L"RaceTrack",false,L"roadwidth",s);
    p->writeLineBreak();

    for (u32 i=0; i<getNumOfPoints(); i++) {
      CRaceTrackGenerator::CReferencePoint *point=getPointData(i);
      if (point) {
        wchar_t s1[0xFF],s2[0xFF],s3[0xFF],s4[0xFF];
        swprintf(s1,0xFF,L" %f| %f| %f",point->m_vPoint.X,point->m_vPoint.Y,point->m_vPoint.Z);

        while (wcschr(s1,L',')) *wcschr(s1,L',')=L'.';
        while (wcschr(s1,L'|')) *wcschr(s1,L'|')=L',';

        swprintf(s2,0xFF,L"%f",point->m_fCutBefore); while (wcschr(s2,L',')) *wcschr(s2,L',')=L'.';
        swprintf(s3,0xFF,L"%f",point->m_fCutAfter ); while (wcschr(s3,L',')) *wcschr(s3,L',')=L'.';
        swprintf(s4,0xFF,L"%i",point->m_iSteps);

        p->writeElement(L"referencepoint",true,L"point",s1,L"before",s2,L"after",s3,L"steps",s4);
        p->writeLineBreak();
      }
    }

    p->writeClosingTag(L"RaceTrack");
    p->writeLineBreak();
  }
}


void CRaceTrackGenerator::fillVector(core::vector3df &v, const char *s) {
  const char *sp[3];

  sp[0]=s;
  sp[1]=strchr(sp[0],',');
  if (sp[1]!=NULL) {
    sp[1]++;
    sp[2]=strchr(sp[1],',');
    if (sp[2]!=NULL) sp[2]++;
  }
  else sp[2]=NULL;

  if (sp[0]!=NULL) v.X=atof(sp[0]);
  if (sp[1]!=NULL) v.Y=atof(sp[1]);
  if (sp[2]!=NULL) v.Z=atof(sp[2]);
}
