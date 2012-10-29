#ifndef _C_MESSAGE
  #define _C_MESSAGE

  #include <irrlicht.h>
  #include <CSerializer.h>


enum eReplayMessages {
  eMessageApplication=0,
  eMessageLoadLevel,
  eMessageWorldStep,
  eMessageMoveBody,
  eMessageRemoveBody,
  eMessageControl=32,
  eMessageUserMsg=128
};

using namespace irr;

using namespace core;
using namespace io;
using namespace ode;

/**
 * @class CMessage
 * This class is used by the CMarbles2Recorder. With this class all changes to the world are saved.
 * @see CMarbles2Recorder
 * @see CIrrOdeRePlayer
 * @see CSerializer
 * @author Christian Keimel / dustbin::games
 */
class CMessage {
  protected:
    c8 m_pBuffer[4096];		/**< the message */
    u32 m_iBufferSize;		/**< the message's buffer size */

  public:
	  /**
		 * Create a new message with the message code and the stacked buffer
		 * @param pBuffer the buffer that holds the message
		 */
    CMessage(CSerializer *pBuffer);

    CMessage(CMessage *pMessage);

		/**
		 * create a new message with the message code and the buffer
		 * @param pBuffer the message buffer
		 * @param iBufferSize the size of the message in the buffer
		 */
    CMessage(const char *pBuffer, u32 iBufferSize);

    void setBuffer(CSerializer *pBuffer);

		/**
		 * Save this message
		 * @param f file to save message to
		 */
    void save(IWriteFile *f);

		/**
		 * Save this message
		 * @param f file to save message to
		 */
    void save(FILE *f);

		/**
		 * Get the buffer of the message
		 * @return the buffer of the message
		 */
    c8 *getBuffer();

		/**
		 * Return the size of the message's buffer
		 * @return the size of the message's buffer
		 */
    u32 getBufferSize();
};

#endif
