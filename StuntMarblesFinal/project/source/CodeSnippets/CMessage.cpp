  #include <CSerializer.h>
  #include <CMessage.h>

CMessage::CMessage(CSerializer *pBuffer) {
  m_iBufferSize=pBuffer->getBufferSize();
  memcpy(m_pBuffer,pBuffer->getBuffer(),pBuffer->getBufferSize());
}

CMessage::CMessage(const char *pBuffer, u32 iBufferSize) {
  m_iBufferSize=iBufferSize;
  memcpy(m_pBuffer,pBuffer,m_iBufferSize);
}

CMessage::CMessage(CMessage *pMessage) {
  m_iBufferSize=pMessage->getBufferSize();
  memcpy(m_pBuffer,pMessage->getBuffer(),m_iBufferSize);
}

void CMessage::setBuffer(CSerializer *pBuffer) {
  m_iBufferSize=pBuffer->getBufferSize();
  memcpy(m_pBuffer,pBuffer->getBuffer(),m_iBufferSize);
}

void CMessage::save(IWriteFile *f) {
  if (f) {
    f->write(&m_iBufferSize,sizeof(u32));
    f->write(m_pBuffer,m_iBufferSize);
  }
}

void CMessage::save(FILE *f) {
  if (f) {
    fwrite(&m_iBufferSize,sizeof(u32),1,f);
    fwrite(m_pBuffer,m_iBufferSize,1,f);
  }
}

c8 *CMessage::getBuffer() {
  return m_pBuffer;
}

u32 CMessage::getBufferSize() {
  return m_iBufferSize;
}


