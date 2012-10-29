#ifndef _C_GHOST_CLASSES
  #define _C_GHOST_CLASSES

  #include <irrlicht.h>
  #include <CSerializer.h>

class CGhostState {
  protected:
    CSerializer *m_pSerializer;

    vector3df m_vPosition,
              m_vRotation;

  public:
    CGhostState(vector3df vPos, vector3df vRot) { m_vPosition=vPos; m_vRotation=vRot; m_pSerializer=NULL; }
    CGhostState(irr::ode::CSerializer *pSerializer) {
      m_pSerializer=NULL;
      pSerializer->resetBufferPos();
      pSerializer->getVector3df(m_vPosition);
      pSerializer->getVector3df(m_vRotation);
    }

    ~CGhostState() { if (m_pSerializer) delete m_pSerializer; }

    vector3df getPosition() { return m_vPosition; }
    vector3df getRotation() { return m_vRotation; }

    CSerializer *serialize() {
      if (m_pSerializer==NULL) {
        m_pSerializer=new irr::ode::CSerializer();
        m_pSerializer->addVector3df(m_vPosition);
        m_pSerializer->addVector3df(m_vRotation);
      }
      return m_pSerializer;
    }
};

class CGhostLaps {
  public:
    u32 m_iLapNo;
    list<CGhostState *> m_lStates;

    CGhostLaps(u32 iLapNo) { m_iLapNo=iLapNo; }
    ~CGhostLaps() {
      while (m_lStates.getSize()>0) {
        list<CGhostState *>::Iterator it=m_lStates.begin();
        CGhostState *p=*it;
        m_lStates.erase(it);
        delete p;
      }
    }
};

class CGhostHeader {
  protected:
    CSerializer *m_pSerializer;
    c8 m_sPlayer[0xFF],m_sIdent[0xFF],m_sLevel[0xFF];

  public:
    CGhostHeader(const c8 *sPlayer, const c8 *sLevel) {
      strcpy(m_sPlayer,sPlayer);
      strcpy(m_sIdent,"StuntMarbleRacersGhostFile");
      strcpy(m_sLevel,sLevel);

      c8 *s=strrchr(m_sLevel,'/');
      if (s) {
        *s='\0';
        s=strrchr(m_sLevel,'/');
        if (s) strcpy(m_sLevel,s+1);
      }
      m_pSerializer=NULL;
    }

    ~CGhostHeader() { if (m_pSerializer) delete m_pSerializer; }

    CGhostHeader(irr::ode::CSerializer *pData) {
      pData->resetBufferPos();
      pData->getString(m_sIdent);

      if (strcmp(m_sIdent,"StuntMarbleRacersGhostFile")) {
        printf("invalid ghost file!\n");
        return;
      }

      pData->getString(m_sPlayer);
      pData->getString(m_sLevel );
      m_pSerializer=NULL;
    }

    CSerializer *serialize() {
      if (m_pSerializer==NULL) {
        m_pSerializer=new irr::ode::CSerializer();
        m_pSerializer->addString(m_sIdent);
        m_pSerializer->addString(m_sPlayer);
        m_pSerializer->addString(m_sLevel);
      }
      return m_pSerializer;
    }

    bool isValid() { return strcmp(m_sIdent,"StuntMarbleRacersGhostFile")==0; }

    const c8 *getLevel() { return m_sLevel; }
    const c8 *getPlayer() { return m_sPlayer; }
    const c8 *getIdent() { return m_sIdent; }
};

#endif
