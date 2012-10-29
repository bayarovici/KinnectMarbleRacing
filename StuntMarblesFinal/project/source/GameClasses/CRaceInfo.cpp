  #include <GameClasses/CRaceInfo.h>

CSectionTime::CSectionTime(u32 iStart) {
  m_iStart=iStart;
  m_iEnd=iStart;
}

void CSectionTime::setEndTime(u32 iEnd) {
  m_iEnd=iEnd;
}

void CSectionTime::setStartTime(u32 iStart) {
  m_iStart=iStart;
}

u32 CSectionTime::getTime() {
  return m_iEnd-m_iStart;
}

u32 CSectionTime::getStartTime() {
  return m_iStart;
}

CLapTime::CLapTime(u32 iTime, u32 iPlayerNo) : CSectionTime(iTime) {
  m_iPlayerNo=iPlayerNo;
}

CLapTime::~CLapTime() {
  for (u32 i=0; i<m_aCheckpoints.size(); i++) delete m_aCheckpoints[i];
  m_aCheckpoints.clear();
}

void CLapTime::addCheckpoint(CSectionTime *pCp) {
  if (m_aCheckpoints.size()>0) m_aCheckpoints[m_aCheckpoints.size()-1]->setEndTime(pCp->getStartTime());
  m_aCheckpoints.push_back(pCp);
}

void CLapTime::endLap(u32 iTime) {
  CSectionTime::setEndTime(iTime);
  if (m_aCheckpoints.size()>0) m_aCheckpoints[m_aCheckpoints.size()-1]->setEndTime(iTime);
  printf("lap ended: time=%i\n",getTime());
}

u32 CLapTime::getCurrentCheckpointNo() {
  return m_aCheckpoints.size();
}

u32 CLapTime::getPlayerNo() {
  return m_iPlayerNo;
}

void CRaceTime::setPositionDone() {
  m_bPositionDone=true;
}

bool CRaceTime::getPositionDone() {
  printf("\tposition done of %i: %s\n",(int)this,m_bPositionDone?"YES":"NO");
  return m_bPositionDone;
}

CRaceTime::CRaceTime(u32 iTime) : CSectionTime(iTime) {
  m_bRaceEnded=false;
  m_pFastestLap=NULL;
  m_bPositionDone=false;
  m_iFinishedPos=99;
  printf("Start Race: %i\n",iTime);
}

CRaceTime::~CRaceTime() {
  for (u32 i=0; i<m_aLaps.size(); i++) delete m_aLaps[i];
  m_aLaps.clear();
}

bool CRaceTime::addLap(CLapTime *pLap) {
  bool bRet=false;
  if (m_aLaps.size()>0 && (m_pFastestLap==NULL || m_pFastestLap->getTime()>m_aLaps[m_aLaps.size()-1]->getTime())) {
    m_pFastestLap=m_aLaps[m_aLaps.size()-1];
    printf("new personal fastest lap: %i\n",m_pFastestLap->getTime());
    bRet=true;
  }
  m_aLaps.push_back(pLap);
  return bRet;
}

void CRaceTime::endRace(u32 iTime) {
  CSectionTime::setEndTime(iTime);
  m_bRaceEnded=true;
}

bool CRaceTime::raceEnded() {
  return m_bRaceEnded;
}

CLapTime *CRaceTime::getCurrentLap() {
  return m_aLaps.size()>0?m_aLaps[m_aLaps.size()-1]:NULL;
}

u32 CRaceTime::getCurrentLapNo() {
  return m_aLaps.size();
}

CLapTime *CRaceTime::getLap(u32 idx) {
  return idx<m_aLaps.size()?m_aLaps[idx]:NULL;
}

CLapTime *CRaceTime::getFastestLap() {
  return m_pFastestLap;
}

void CRaceTime::setStartTime(u32 iTime) {
  m_iStart=iTime;
  m_iEnd=iTime;
}

void CRaceTime::setFinishedPos(u32 i) {
  m_iFinishedPos=i;
}

u32 CRaceTime::getFinishedPos() {
  return m_iFinishedPos;
}

static int iInstances=0;

CRaceInfo::CRaceInfo() {
  m_iLaps=0;
  m_pFastestLap=NULL;
  m_bSplitHorizontal=false;
  m_bRecordReplay=false;
  m_bGhost=false;
  m_sGhostFile[0]='\0';
  iInstances++;
}

CRaceInfo::~CRaceInfo() {
  for (u32 i=0; i<m_aRaceTime.size(); i++) {
    CRaceTime *rt=m_aRaceTime[i];
    delete rt;
  }
  m_aRaceTime.clear();
}

void CRaceInfo::setLaps(u32 iLaps) {
  m_iLaps=iLaps;
}

u32 CRaceInfo::getLaps() {
  return m_iLaps;
}

u32 CRaceInfo::getPlayerNo() {
  return m_aPlayers.size();
}

void CRaceInfo::addPlayerName(stringw sName) {
  m_aPlayers.push_back(stringw(sName));
  m_aRaceTime.push_back(new CRaceTime(0));
  printf("adding player \"%s\"\n",stringc(sName).c_str());
}

stringw CRaceInfo::getPlayerName(u32 iIdx) {
  return m_aPlayers[iIdx];
}

void CRaceInfo::setLevel(stringw sLevel) {
  m_sLevel=stringw(sLevel);
}

stringw CRaceInfo::getLevel() {
  return m_sLevel;
}

CRaceTime *CRaceInfo::getRaceTime(u32 idx) {
  return m_aRaceTime[idx];
}

void CRaceInfo::setFastestLap(CLapTime *pFastestLap) {
  m_pFastestLap=pFastestLap;
}

CLapTime *CRaceInfo::getFastestLap() {
  return m_pFastestLap;
}

array<CRaceTime *> CRaceInfo::getRaceTimeArray() {
  return m_aRaceTime;
}

void CRaceInfo::setSplitHorizontal(bool b) {
  m_bSplitHorizontal=b;
}

void CRaceInfo::setGhost(bool b) {
  m_bGhost=b;
}

bool CRaceInfo::getGhost() {
  return m_bGhost;
}

bool CRaceInfo::getSplitHorizontal() {
  return m_bSplitHorizontal;
}

void CRaceInfo::setHiScoreFile(const wchar_t *sFile) {
  m_sHiScoreFile=stringw(sFile);
}

const wchar_t *CRaceInfo::getHiScoreFile() {
  return m_sHiScoreFile.c_str();
}

bool CRaceInfo::raceFinished() {
  bool bRet=true;
  for (u32 i=0; i<m_aRaceTime.size() && bRet; i++) {
    CRaceTime *pTime=m_aRaceTime[i];
    if (!pTime->raceEnded()) bRet=false;
  }
  return bRet;
}

void CRaceInfo::addFinishedPosition(u32 iPlayer) {
  m_aPositions.push_back(iPlayer);
}

u32 CRaceInfo::getPlayerFinishedPosition(u32 iPlayer) {
  for (u32 i=0; i<m_aPositions.size(); i++) if (m_aPositions[i]==iPlayer) return i;
  return 99;
}

u32 CRaceInfo::getPlayerAtFinishedPosition(u32 iPos) {
  return iPos<m_aPositions.size()?m_aPositions[iPos]:99;
}

CRaceTime *CRaceInfo::getNext(u32 *iPlayer) {
  CRaceTime *pRet=NULL;
  s32 iMaxDone=0,iMaxIdx=-1;

  for (s32 i=0; i<(s32)m_aRaceTime.size(); i++)
    if (!m_aRaceTime[i]->getPositionDone() && m_aRaceTime[i]->raceEnded())
      if ( iMaxIdx==-1 ||
           (s32)m_aRaceTime[i]->getCurrentLapNo()>iMaxDone ||
          (m_aRaceTime[i]->getTime()<m_aRaceTime[iMaxIdx]->getTime())) {
        iMaxDone=m_aRaceTime[i]->getCurrentLapNo();
        iMaxIdx=i;
      }

  if (iMaxIdx!=-1) {
    pRet=m_aRaceTime[iMaxIdx];
    pRet->setPositionDone();
  }
  *iPlayer=iMaxIdx;
  return pRet;
}

void CRaceInfo::calculateResult() {
  u32 iPos;

  for (u32 i=0; i<m_aRaceTime.size(); i++) {
    CRaceTime *pTime=getNext(&iPos);
    if (pTime) pTime->setFinishedPos(i);
    m_aPositions.push_back(iPos);
  }

  printf("\n\n\t\tRace Result\n\t\t***********\n\n");
  for (u32 i=0; i<m_aPositions.size(); i++) {
    printf("\tPosition %i: Player %i\n",i,m_aPositions[i]);
  }
  printf("\n\t\t***********\n\n");
}

void CRaceInfo::setRecordReplay(bool b) {
  m_bRecordReplay=b;
}

bool CRaceInfo::getRecordReplay() {
  return m_bRecordReplay;
}

void CRaceInfo::setGhostFile(const c8 *sFile) {
  strcpy(m_sGhostFile,sFile);
}

const c8 *CRaceInfo::getGhostFile() {
  return m_sGhostFile;
}
