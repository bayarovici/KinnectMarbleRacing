#ifndef _C_MANAGED_SHADOW
  #define _C_MANAGED_SHADOW

  #include <irrlicht.h>
	#ifndef _IRR_ODE_KLANG_PLUGIN
		#include <irrklang.h>
	#endif
  #include <IrrOde.h>

using namespace irr;

using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace ode;

#ifndef _IRR_ODE_KLANG_PLUGIN
  using namespace irrklang;
#endif

class CShadowManager;

const int IRR_ODE_KLANG_NODE_ID=MAKE_IRR_ID('i','o','k','n');
const c8 IRR_ODE_KLANG_NODE_NAME[0xFF]="CIrrOdeKlangNode";

/**
 * @class CIrrOdeKlangNode
 * This class brings life to IrrODE scenes by adding sound. The sound will be played depending on various conditions,
 * e.g. on collision with another node
 * @author Christian Keimel / dustbin::games
 */
class CIrrOdeKlangNode : public ISceneNode {
  private:
		ISceneManager *m_pSceneManager;

		stringc m_sFileName;					/**< the file name of the sample to be played */

		f32 m_fMinDistance,						/**< Minimum distance. The sound will be at maximum volume when it gets closer as this */
			  m_fMaxDistance,						/**< Maximum distance. The sound will not be played if it's further away */
				m_fMinVel,								/**< Minimum velocity. This is for velocity sounds. If the object gets faster the sound is played */
				m_fMaxVel,								/**< Maximum velocity. This is for velocity sounds. If the object gets this fast the sound is at max volume */
				m_fMinForce,							/**< Minimum force. This is for collision sounds. Collisions with lighter force don't trigger sound */
				m_fMaxForce,							/**< Maximum force. This is for collision sounds. Collisions with at least this force trigger maximum volume */
				m_fVolume,								/**< The maximum volume */
				m_fMinDamping,							/**< more damping acitvates the sound */
				m_fMaxDamping;							/**< the sound will not get louder if this damping is reached */

		CIrrOdeBody *m_pBody;					/**< the body the sound is attached to */

		bool m_bLinearVelSound,				/**< is it a linear velocity sound? */
			   m_bAngularVelSound,			/**< is it an angular velocity sound? */
				 m_bCollisionSound,				/**< is it a collision sound? */
				 m_bTouching,							/**< is the sound only played if it touches something? For velocity sounds only */
				 m_bAngDamping,				/**< is this an angular damping sound? */
				 m_bPlay;									/**< is the sound played? */

		vector3df m_vOldVelocity,			/**< the old velocity. Used to calculate the force of a collision */
		          m_vBuffer;					/**< buffer for the velocity */

		#ifndef _IRR_ODE_KLANG_PLUGIN
	    ISound *m_pSound,							/**< sound for listener number 1 */
	           *m_pSound2;						/**< sound for listener number 2 */
		  ISoundEngine *m_pSndEngine;		/**< the sound engine */
		#endif

  public:
    CIrrOdeKlangNode(ISceneNode *pParent, ISceneManager *pMgr, s32 iId=-1, const vector3df pos=vector3df(0,0,0), const vector3df rot=vector3df(0,0,0), const vector3df scale=vector3df(1,1,1));
    ~CIrrOdeKlangNode();

    virtual void render() { }

    virtual const aabbox3df &getBoundingBox() const {
      static aabbox3df aBox;
      return aBox;
    }

    virtual ESCENE_NODE_TYPE getType() const;

    virtual void serializeAttributes(IAttributes* out, SAttributeReadWriteOptions* options) const;
    virtual void deserializeAttributes(io::IAttributes* in, io::SAttributeReadWriteOptions* options);

		virtual ISceneNode *clone(ISceneNode* newParent=0, ISceneManager* newManager=0);

		f32 getMinDistance();			/**< get minimum distance */
		f32 getMaxDistance();			/**< get maximum distance */
		f32 getMinVelocity();			/**< get minimum velocity */
		f32 getMaxVelocity();			/**< get maximum velocity */
		f32 getMinForce();				/**< get minimum force */
		f32 getMaxForce();				/**< get maximum force */

		bool isCollisionSound();	  /**< is this a collision sound? */
		bool isLinearVelSound();	  /**< is this a linear velocity sound? */
		bool isAngularVelSound();	  /**< is this an angular velocity sound? */
		bool isAngularDampingSound(); /**< is this an angular damping sound? */
		bool isTouching();				/**< is this sound only played when it touches something? */

		stringc getFileName();		/**< get the sample's filename */

		CIrrOdeBody *getBody();		/**< get the body this sound node is attached to */

		void setMinDistance(f32 f);						/**< set the minimum distance */
		void setMaxDistance(f32 f);						/**< set the maximum distance */
		void setMinVelocity(f32 f);						/**< set the minimum velocity */
		void setMaxVelocity(f32 f);						/**< set the maximum velocity */
		void setMinForce(f32 f);							/**< set the mimum force */
		void setMaxForce(f32 f);							/**< set the maximum force */
		void setMinDamping(f32 f);						/**< set the minimum damping */
		void setMaxDamping(f32 f);						/**< set the maximum damping */
		void setCollisionSound(bool b);				/**< make this sound a collision sound */
		void setAngularDamping(bool b);				/**< make this sound an angular damping sound */
		void setLinearVelSound(bool b);				/**< make this sound a linear velocity sound */
		void setAngularVelSound(bool b);			/**< make this sound an angular velocity sound */
		void setTouching(bool b);							/**< set this sound to be played when the object touches something */
		void setFileName(stringc sFileName);	/**< set the sample file name */
		void stop();													/**< stop playing the sound */

		void setVolume(f32 f);			/**< set the volume */
		void setIsPaused(bool b);		/**< pause or unpause the sound */

		#ifndef _IRR_ODE_KLANG_PLUGIN
  		void setSoundEngine(ISoundEngine *pSndEngine);	/**< set the sound angine */
		#endif

		/**
		 * play the sound
		 */
    void play();

		/**
		 * update the sound's parameters (position, velocity...)
		 */
		void update(ICameraSceneNode *pCam1, ICameraSceneNode *pCam2);
};

/**
 * This class implements the Irrlicht scene node factory for CIrrOdeKlangNodes
 * @author Christian Keimel / dustbin::games
 */
class CIrrOdeKlangNodeFactory : public ISceneNodeFactory {
  protected:
    ISceneManager *m_pManager;

  public:
    CIrrOdeKlangNodeFactory(ISceneManager *pManager);
    virtual ~CIrrOdeKlangNodeFactory();
    virtual ISceneNode *addSceneNode (ESCENE_NODE_TYPE type, ISceneNode *parent=0);
    virtual ISceneNode *addSceneNode (const c8 *typeName, ISceneNode *parent=0);
    virtual u32 getCreatableSceneNodeTypeCount() const;
    virtual ESCENE_NODE_TYPE getCreateableSceneNodeType (u32 idx) const;
    virtual const c8 *getCreateableSceneNodeTypeName(ESCENE_NODE_TYPE type) const;
    virtual const c8 *getCreateableSceneNodeTypeName(u32 idx) const;
};

#endif
