#ifndef KINECTJOYSTICKGUI_H_
#define KINECTJOYSTICKGUI_H_
#include <irrlicht.h>
using namespace irr;
using namespace core;
using namespace scene;
using namespace gui;

#include <string>
#include <sstream>

template<typename T>
inline std::string stringify(const T& x)
{
    std::ostringstream o;
    if (!(o << x)) return "";
    return o.str();
}

template<typename T>
inline T fromString(char *s)
{
    std::string str = s;
    std::istringstream i(str);
    T x;
    i >> x;
    return x;
}

inline std::string strparty(const char *str1, int n, const char *str2)
{
    std::string str = str1;
    str += stringify(n);
    str += str2;
    return str;
}


class CKinectJoystickGUI : public IGUIElement
{
public:
	 gui::IGUIStaticText *kinectText;
	 CKinectJoystickGUI(IGUIEnvironment * guienv,const core::rect<s32>& rectangle,s32 id=-1,IGUIElement * parent=0,irr::u32 player=1);
	 irr::u32 plyerNo;
     virtual void draw();
	 void update(double,double);
	 double transmissionData;
  private:
	  bool firstData;
	  double firstZ,firstX;
	  double zMov,xMov;
     IGUIEnvironment * gui; //GUI ENV. pointer
     irr::s32 total; //Dimension (X) of the bar, to calculate relative percentage.
     rect<s32> bar; //Dimension of the bar
     rect<s32> position; //Bar
     rect<s32> border; //Border
     rect<s32> tofill; //Percentage indicator
     rect<s32> empty; //"Empty" indicator

     irr::video::SColor fillcolor;
     irr::video::SColor emptycolor;
     irr::video::SColor bordercolor;
     irr::video::IVideoDriver * vdriver;

	 irr::core::recti playerRect; 
	 bool rectInside(irr::core::recti& one,irr::core::recti& two);
	 int vertical;
};
#endif

