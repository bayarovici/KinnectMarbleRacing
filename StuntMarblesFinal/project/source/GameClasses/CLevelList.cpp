  #include <GameClasses/CLevelList.h>

CLevelList::CLevelInfo::CLevelInfo(const c8 *sLevel, IrrlichtDevice *pDevice) {
  IFileSystem *pFs=pDevice->getFileSystem();
  const c8 *sDir=pFs->getWorkingDirectory().c_str();
  c8 s[0xFF],
     sFile[0xFF];

  sprintf(sFile,"%s/data/levels/%s/level.xml",sDir,sLevel);
  sprintf(s,"%s/data/levels/%s/info.xml",sDir,sLevel);

  m_sFile=stringw(sFile);

  sprintf(sFile,"%s/data/levels/%s/netbook.xml",sDir,sLevel);
  m_sNetbookFile=stringw(sFile);

  sprintf(sFile,"%s/data/hiScore/%s.xml",sDir,sLevel);
  m_sHiScoreFile=stringw(sFile);

  if (pFs->existFile(s)) {
    IXMLReaderUTF8 *pXml=pFs->createXMLReaderUTF8(s);
    u32 iState=0;
    if (pXml) {
      while (pXml->read()) {
        switch (pXml->getNodeType()) {
          case EXN_ELEMENT:
            if (!strcmp(pXml->getNodeName(),"name")) iState=1;
            if (!strcmp(pXml->getNodeName(),"info")) iState=2;
            if (!strcmp(pXml->getNodeName(),"pos" )) iState=3;
            break;

          case EXN_ELEMENT_END:
            iState=0;
            break;

          case EXN_TEXT:
            switch (iState) {
              case 1: m_sName=stringw(pXml->getNodeData()).c_str(); break;
              case 2: m_sInfo=stringw(pXml->getNodeData()).c_str(); break;
              case 3: m_iPos=atoi(pXml->getNodeData()); break;
            }
            break;

          default:
            break;
        }
      }
      pXml->drop();
    }
  }
  else {
    m_iPos=10000;
    m_sName=stringw(sLevel);
  }
}

CLevelList::CLevelList(IrrlichtDevice *pDevice) {
  IFileSystem *pFs=pDevice->getFileSystem();
  stringc sCwd=pFs->getWorkingDirectory();
  c8 sDir[0xFF];
  sprintf(sDir,"%s/data/levels",sCwd.c_str());
  pFs->changeWorkingDirectoryTo(sDir);
  IFileList *pFileList=pFs->createFileList();
	pFs->changeWorkingDirectoryTo(sCwd.c_str());

	for (u32 i=0; i<pFileList->getFileCount(); i++) {
		c8 sFileXml[0xFF];
		sprintf(sFileXml,"data/levels/%s/level.xml",pFileList->getFileName(i).c_str());
		if (strcmp(pFileList->getFileName(i).c_str(),"." ) && pFs->existFile(sFileXml) &&
        strcmp(pFileList->getFileName(i).c_str(),"..") && pFileList->isDirectory(i)) {
      CLevelInfo *pInfo=new CLevelInfo(pFileList->getFileName(i).c_str(),pDevice);
      bool bDone=false;
      for (u32 j=0; j<m_aLevels.size(); j++) {
        if (pInfo->m_iPos<m_aLevels[j]->m_iPos) {
          m_aLevels.insert(pInfo,j);
          bDone=true;
          break;
        }
      }

      if (!bDone) {
        m_aLevels.push_back(pInfo);
      }
		}
	}
	pFileList->drop();
}

CLevelList::~CLevelList() {
  for (u32 i=0; i<m_aLevels.size(); i++) delete m_aLevels[i];
  m_aLevels.clear();
}
