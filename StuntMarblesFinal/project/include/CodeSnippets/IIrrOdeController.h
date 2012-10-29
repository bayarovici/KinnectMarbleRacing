#ifndef _I_IRR_ODE_CONTROLLER
  #define _I_IRR_ODE_CONTROLLER

  #include <irrlicht.h>
  #include <CIrrOdeBody.h>

using namespace irr;
using namespace core;
using namespace scene;
using namespace video;
using namespace io;
using namespace gui;
using namespace ode;

/**
 * This is an interface that is used to hide the IrrOde function calls. There are two implementations:
 * The CIrrOdeController that calls the functions directly and the CIrrOdeNetController that send the
 * commands over the network
 * @see CIrrOdeController
 * @see CIrrOdeNetController
 */
class IIrrOdeController {
  public:
    virtual ~IIrrOdeController() { }
		
		/**
		 * Add a torque to a body
		 * @param pBody the body to apply the torque to
		 * @param vTorque the torque to apply
		 */
    virtual void addTorque(CIrrOdeBody *pBody, vector3df vTorque)=0;
		
		/**
		 * Set the angular damping of a body
		 * @param pBody the body to modify
		 * @param fDamping the new damping parameter
		 */
    virtual void setAngularDamping(CIrrOdeBody *pBody, f32 fDamping)=0;
		
		/**
		 * Set the position of a body
		 * @param pBody the body to modify
		 * @param vPos the new position of the body
		 */
    virtual void setPosition(CIrrOdeBody *pBody, vector3df vPos)=0;
		
		/**
		 * Set the linear velocity of a body
		 * @param pBody the body to modify
		 * @param vVel the new linear velocity
		 */
    virtual void setLinearVelocity(CIrrOdeBody *pBody, vector3df vVel)=0;
		
		/**
		 * Set the angular velocity of a body
		 * @param pBody the body to modify
		 * @param vVel the new angular velocity
		 */
    virtual void setAngularVeclocity(CIrrOdeBody *pBody, vector3df vVel)=0;
};

#endif
