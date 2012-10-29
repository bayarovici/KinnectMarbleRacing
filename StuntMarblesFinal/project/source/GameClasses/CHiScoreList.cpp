  #include <GameClasses/CHiScoreList.h>

/**
 * The constructor
 * @param Irrlicht device for the XML reader and writer
 */
CHiScoreList::CHiScoreList(IrrlichtDevice *pDevice, const c8 *sFileName) {
  m_pDevice=pDevice;
  IXMLReaderUTF8 *pXml=m_pDevice->getFileSystem()->createXMLReaderUTF8(sFileName);
  strcpy(m_sFileName,sFileName);
  u32 iCnt=0;

  //If the XML was opened successfully we read the information from the file
  if (pXml) {
    while (pXml->read() && iCnt<10) {
      if (!strcmp(pXml->getNodeName(),"score")) {
        CHiScoreEntry *theEntry=new CHiScoreEntry();
        theEntry->iTime=atoi(pXml->getAttributeValue("time"));
        wcscpy(theEntry->sName,stringw(pXml->getAttributeValue("name")).c_str());
        m_lList.push_back(theEntry);
        iCnt++;
      }
    }
    pXml->drop();
  }
}

/**
 * The destructor
 */
CHiScoreList::~CHiScoreList() {
  list<CHiScoreEntry *>::Iterator i;
  for (i=m_lList.begin(); i!=m_lList.end(); i++) delete(*i);
  m_lList.clear();
}

u32 CHiScoreList::getPositionOf(u32 iTime, const wchar_t *sName) {
  list<CHiScoreEntry *>::Iterator i;
  u32 iPos=1;

  for (i=m_lList.begin(); i!=m_lList.end() && iPos<=10; i++) {
    if ((*i)->iTime==iTime && !wcscmp((*i)->sName,sName)) return iPos;
    iPos++;
  }
  return 0;
}

s32 CHiScoreList::insert(u32 iTime, const wchar_t *sName) {
  s32 iPos=0;
  list<CHiScoreEntry *>::Iterator i;
  bool bDone=false;

  //search for the position of the new entry
  for (i=m_lList.begin(); i!=m_lList.end() && iPos<10; i++) {
    if ((*i)->iTime>iTime) {
      CHiScoreEntry *pEntry=new CHiScoreEntry();
      pEntry->iTime=iTime;
      wcscpy(pEntry->sName,sName);
      m_lList.insert_before(i,pEntry);
      bDone=true;
      break;
    }
    iPos++;
  }

  //if no position was found and iPos is zero we have an empty highscore list, so we add the entry as first place
  if (iPos==0 && !bDone) {
    CHiScoreEntry *pEntry=new CHiScoreEntry();
    pEntry->iTime=iTime;
    wcscpy(pEntry->sName,sName);
    m_lList.push_front(pEntry);
    return bDone;
  }
  else
    //if no position was found and iPos is less than 10 we have a list with less than 10 entries so we
    //add the new entry at the end of the list
    if (iPos<10 && !bDone) {
      CHiScoreEntry *pEntry=new CHiScoreEntry();
      pEntry->iTime=iTime;
      wcscpy(pEntry->sName,sName);
      m_lList.push_back(pEntry);
    }
  return bDone?iPos:-1;
}

/**
 * This function is used to show the highscore table
 * @param a new time value to be entered to the list. If this param is 0 the highscore list will just be shown, no entry will be added
 */
void CHiScoreList::show(IGUIStaticText *pListField, IGUIEnvironment *pGuiEnv,const wchar_t *sLevelName) {
  dimension2du screenSize=m_pDevice->getVideoDriver()->getScreenSize();
  m_iRet=0;
  u32 iPos=0;
  list<CHiScoreEntry *>::Iterator i;
  //create a window to display the table
  rect<s32> winRect(screenSize.Width/2-249,130,screenSize.Width/2+248,screenSize.Height-25);

  iPos=1;
  position2di pos=position2di(10,10);
  wchar_t s[0xFFFF];
  swprintf(s,0xFFFF,L"\"%s\"\n\n",sLevelName);
  for (i=m_lList.begin(); i!=m_lList.end() && iPos<=10; i++) {
    swprintf(s,0xFFFF,L"%s%2i: %s (%.2f)\n",s,iPos++,(*i)->sName,((f32)(*i)->iTime)/1000.0f);
  }
  pListField->setText(s);
}

/**
 * This method saves the highscore table
 */
void CHiScoreList::save() {
  IXMLWriter *pXml=m_pDevice->getFileSystem()->createXMLWriter(m_sFileName);
  if (pXml) {
    pXml->writeXMLHeader();
    pXml->writeElement(L"HiScore",false);
    pXml->writeLineBreak();
    u32 iCnt=0;
    list<CHiScoreEntry *>::Iterator i;
    //Save the list of highscore entries, but only 10 entries will be saved
    for (i=m_lList.begin(); i!=m_lList.end() && iCnt<10; i++) {
      iCnt++;
      wchar_t s[0xFF];
      swprintf(s,0xFE,L"%i",(*i)->iTime);
      pXml->writeElement(L"score",true,L"time",s,L"name",(*i)->sName);
      pXml->writeLineBreak();
    }
    pXml->writeClosingTag(L"HiScore");
    pXml->drop();
  }
}
