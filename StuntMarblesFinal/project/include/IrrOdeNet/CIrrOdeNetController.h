#ifndef _C_IRR_ODE_NET_CONTROLLER
  #define _C_IRR_ODE_NET_CONTROLLER

  #include <IIrrOdeController.h>
  #include <CSerializer.h>

class CIrrOdeNetAdapter;

class CIrrOdeNetController : public IIrrOdeController {
  protected:
    CIrrOdeNetAdapter *m_pNetAdapter;

  public:
    virtual ~CIrrOdeNetController() { m_pNetAdapter=NULL; }

    virtual void addTorque(CIrrOdeBody *pBody, vector3df vTorque);
    virtual void setAngularDamping(CIrrOdeBody *pBody, f32 fDamping);
    virtual void setPosition(CIrrOdeBody *pBody, vector3df vPos);
    virtual void setLinearVelocity(CIrrOdeBody *pBody, vector3df vVel);
    virtual void setAngularVeclocity(CIrrOdeBody *pBody, vector3df vVel);

    void setNetAdapter(CIrrOdeNetAdapter *pAdapter) { m_pNetAdapter=pAdapter; }
};

#endif

