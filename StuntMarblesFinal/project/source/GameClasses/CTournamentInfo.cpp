  #include <GameClasses/CTournamentInfo.h>
  #include <GameClasses/CRaceInfo.h>

CTournamentRaceInfo::CTournamentRaceInfo(const wchar_t *sTrack, u32 iLaps) {
  wcscpy(m_sTrack,sTrack);
  m_iLaps=iLaps;
  m_bRaceFinished=false;
}

wchar_t *CTournamentRaceInfo::getTrack() {
  return m_sTrack;
}

u32 CTournamentRaceInfo::getLaps() {
  return m_iLaps;
}

void CTournamentRaceInfo::setRaceFinished(bool b) {
  m_bRaceFinished=b;
}

bool CTournamentRaceInfo::raceFinished() {
  return m_bRaceFinished;
}

CTournamentInfo::CTournamentInfo(const wchar_t *sName) {
  wcscpy(m_sName,sName);
  m_bRecordReplay=false;
}

CTournamentInfo::~CTournamentInfo() {
  for (u32 i=0; i<m_aRaces.size(); i++) delete m_aRaces[i];
  m_aRaces.clear();
  for (u32 i=0; i<m_aRaceInfo.size(); i++) delete m_aRaceInfo[i];
  m_aRaceInfo.clear();
  m_aPlayerNames.clear();
}

u32 CTournamentInfo::getRaceCount() {
  return m_aRaces.size();
}

void CTournamentInfo::addRace(CTournamentRaceInfo *pRace) {
  if (pRace) m_aRaces.push_back(pRace);
}

CTournamentRaceInfo *CTournamentInfo::getRace(u32 iIdx) {
  return iIdx<m_aRaces.size()?m_aRaces[iIdx]:NULL;
}

wchar_t *CTournamentInfo::getName() {
  return m_sName;
}

void CTournamentInfo::addRaceInfo(CRaceInfo *pInfo) {
  m_aRaceInfo.push_back(pInfo);
}

CRaceInfo *CTournamentInfo::getRaceInfo(u32 iIdx) {
  return iIdx<m_aRaceInfo.size()?m_aRaceInfo[iIdx]:NULL;
}

void CTournamentInfo::setPlayerNo(u32 i) {
  printf("\t#### CTournamentInfo::setPlayerNo: %i\n",i);
  if (i==3)
    printf("Hmpf!\n");
  m_iPlayerNo=i;
}

u32 CTournamentInfo::getPlayerNo() {
  return m_iPlayerNo;
}

void CTournamentInfo::setSplitHorizontal(bool b) {
  m_bHSplit=b;
}

bool CTournamentInfo::getSplitHorizontal() {
  return m_bHSplit || m_iPlayerNo==1;
}

void CTournamentInfo::addPlayerName(const wchar_t *sName) {
  wchar_t *s;
  s=(wchar_t *)malloc((wcslen(sName)+1)*sizeof(wchar_t));
  wcscpy(s,sName);
  m_aPlayerNames.push_back(stringw(s));
}

const wchar_t *CTournamentInfo::getPlayerName(u32 iIdx) {
  return iIdx<m_aPlayerNames.size()?m_aPlayerNames[iIdx].c_str():NULL;
}

void CTournamentInfo::setRecordReplay(bool b) {
  m_bRecordReplay=b;
}

bool CTournamentInfo::getRecordReplay() {
  return m_bRecordReplay;
}
