  #include <IrrOdeNet/CEventServerMessage.h>
  #include <IrrOdeNet/NetMessages.h>

CEventServerMessage::CEventServerMessage(u16 iCode) {
  m_iCode=iCode;
}

CEventServerMessage::CEventServerMessage(CSerializer *pData) {
  pData->resetBufferPos();
  u16 iType=pData->getU16();
  printf("\t\tdeserializing server message | %i\n",iType);
  if (iType==eNetServerMessage) {
    m_iCode=pData->getU16();
    printf("\t\tcode=%i\n",m_iCode);

    if (m_iCode==260)
      printf("xyz");

    switch (m_iCode) {
      case eNetSetPlayerName:
      case eNetSetRaceTrack:
        m_iData1=pData->getU16();
        pData->getString(m_sData2);
        break;

      case eMessagePlayerDisconnected:
      case eNetDelRace:
      case eNetAddRace:
      case eNetLoadLevel:
      case eNetClientPlayerId:
        m_iData1=pData->getU16();
        break;

      case eNetTournamentName:
        pData->getString(m_sData2);
        m_iData1=pData->getU16();
        break;

      case eNetSetRaceLaps:
        m_iData1=pData->getU16();
        m_iData3=pData->getU16();
        break;
    }
  }
}

CEventServerMessage::CEventServerMessage(irr::ode::IIrrOdeEvent *pEvt) {
  if (pEvt->getType()==eNetServerMessage) {
    CEventServerMessage *p=(CEventServerMessage *)pEvt;
    m_iCode=p->getCode();
    m_iData1=p->getData1();
    m_iData3=p->getData3();
    strcpy(m_sData2,p->getData2());
  }
}

CSerializer *CEventServerMessage::serialize() {
  if (m_pSerializer==NULL) {
    m_pSerializer=new CSerializer();
    m_pSerializer->addU16(eNetServerMessage);
    m_pSerializer->addU16(m_iCode);

    switch (m_iCode) {
      case eNetSetPlayerName:
      case eNetSetRaceTrack:
        m_pSerializer->addU16(m_iData1);
        m_pSerializer->addString(m_sData2);
        break;

      case eMessagePlayerDisconnected:
      case eNetDelRace:
      case eNetAddRace:
      case eNetLoadLevel:
      case eNetClientPlayerId:
        m_pSerializer->addU16(m_iData1);
        break;

      case eNetTournamentName:
        m_pSerializer->addString(m_sData2);
        m_pSerializer->addU16(m_iData1);
        break;

      case eNetSetRaceLaps:
        m_pSerializer->addU16(m_iData1);
        m_pSerializer->addU16(m_iData3);
        break;
    }
  }

  return m_pSerializer;
}

const c8 *CEventServerMessage::toString() {
  sprintf(m_sString,"CEventServerMessage");

  switch(m_iCode) {
    case eNetSetPlayerName         : sprintf(m_sString,"%s (eNetSetPlayerName)"         ,m_sString); break;
    case eNetSetRaceTrack          : sprintf(m_sString,"%s (eNetSetRaceTrack)"          ,m_sString); break;
    case eMessagePlayerDisconnected: sprintf(m_sString,"%s (eMessagePlayerDisconnected)",m_sString); break;
    case eNetDelRace               : sprintf(m_sString,"%s (eNetDelRace)"               ,m_sString); break;
    case eNetAddRace               : sprintf(m_sString,"%s (eNetAddRace)"               ,m_sString); break;
    case eNetLoadLevel             : sprintf(m_sString,"%s (eNetLoadLevel)"             ,m_sString); break;
    case eNetClientPlayerId        : sprintf(m_sString,"%s (eNetClientPlayerId)"        ,m_sString); break;
    case eNetTournamentName        : sprintf(m_sString,"%s (eNetTournamentName)"        ,m_sString); break;
    case eNetSetRaceLaps           : sprintf(m_sString,"%s (eNetSetRaceLaps)"           ,m_sString); break;
    case eNetQueryPlayerName       : sprintf(m_sString,"%s (eNetQueryPlayerName)"       ,m_sString); break;
    case eNetClearList             : sprintf(m_sString,"%s (eNetClearList)"             ,m_sString); break;
    case eNetStartTournament       : sprintf(m_sString,"%s (eNetStartTournament)"       ,m_sString); break;
    case eNetBackToStart           : sprintf(m_sString,"%s (eNetBackToStart)"           ,m_sString); break;
    case eNetGameRunning           : sprintf(m_sString,"%s (eNetGameRunning)"           ,m_sString); break;
    case eNetStartRace             : sprintf(m_sString,"%s (eNetStartRace)"             ,m_sString); break;
    case eMessageServerFull        : sprintf(m_sString,"%s (eMessageServerFull)"        ,m_sString); break;
  }

  switch (m_iCode) {
    case eNetSetPlayerName:
    case eNetSetRaceTrack:
      sprintf(m_sString,"%s: iData1=%i, sData2=\"%s\"",m_sString,m_iData1,m_sData2);
      break;

    case eMessagePlayerDisconnected:
    case eNetDelRace:
    case eNetAddRace:
    case eNetLoadLevel:
    case eNetClientPlayerId:
      sprintf(m_sString,"%s: iData1=%i",m_sString,m_iData1);
      break;

    case eNetTournamentName:
      sprintf(m_sString,"%s: sData2=\"%s\"",m_sString,m_sData2);
      break;

    case eNetSetRaceLaps:
      sprintf(m_sString,"%s: iData1=%i, iData3=%i",m_sString,m_iData1,m_iData3);
      break;
  }
  return m_sString;
}
