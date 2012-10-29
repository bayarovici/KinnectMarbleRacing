  #include <irrlicht.h>
  #include <CShadowManager.h>
  #include <CManagedShadow.h>

CShadowManager::CShadowManager() {
  m_pProgress=NULL;
}

CShadowManager *CShadowManager::getSharedInstance() {
  static CShadowManager theManager;
  return &theManager;
}

void CShadowManager::addShadow(CManagedShadow *pNew) {
  m_lShadows.push_back(pNew);
}

void CShadowManager::setLevel(u32 iLevel) {
  if (iLevel!=m_iLevel) {
    m_iLevel=iLevel;
		update();
  }
}

void CShadowManager::update() {
  u32 iCnt=0;
  list<CManagedShadow *>::Iterator it;
  for (it=m_lShadows.begin(); it!=m_lShadows.end(); it++) {
    (*it)->update(m_iLevel);
    if (m_pProgress) {
      iCnt++;
      m_pProgress->progress(iCnt,m_lShadows.getSize());
    }
  }
}

u32 CShadowManager::getLevel() {
  return m_iLevel;
}

void CShadowManager::setProgressCallback(CManagedShadowProgress *pProgress) {
  m_pProgress=pProgress;
}

void CShadowManager::clear() {
  m_lShadows.clear();
}
