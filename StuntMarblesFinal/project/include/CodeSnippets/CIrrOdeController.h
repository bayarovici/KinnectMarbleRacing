#ifndef _C_IRR_ODE_CONTROLLER
  #define _C_IRR_ODE_CONTROLLER

  #include <IIrrOdeController.h>

/**
 * This is just an implementation of the IIrrOdeController interface that calls the actual functions
 * @see IIrrOdeController
 */
class CIrrOdeController : public IIrrOdeController {
  public:
    virtual ~CIrrOdeController() { }

    virtual void addTorque(CIrrOdeBody *pBody, vector3df vTorque);
    virtual void setAngularDamping(CIrrOdeBody *pBody, f32 fDamping);
    virtual void setPosition(CIrrOdeBody *pBody, vector3df vPos);
    virtual void setLinearVelocity(CIrrOdeBody *pBody, vector3df vVel);
    virtual void setAngularVeclocity(CIrrOdeBody *pBody, vector3df vVel);
};

#endif
