#include <KinectJoystickGUI.h>
#include  <iostream>
#include <sstream>
CKinectJoystickGUI::CKinectJoystickGUI(IGUIEnvironment * guienv,const core::rect<s32>& rectangle,s32 id,IGUIElement * parent,irr::u32 playerNo) : IGUIElement(EGUIET_ELEMENT,guienv,parent,id,rectangle){
	this->plyerNo = playerNo;
	//1100+80+30,10+80+30, 1100+80+30+20,10+80+30+20
	playerRect= irr::core::recti(1210, 120, 1230, 140); 

	vertical=0;  
	total = rectangle.LowerRightCorner.X - rectangle.UpperLeftCorner.X;
	gui = guienv;
	bar = rectangle;

	if(parent == 0)
		guienv->getRootGUIElement()->addChild(this); //Ensure that draw method is called
	vdriver = this->gui->getVideoDriver();
	fillcolor.set(255,255,255,255);
	emptycolor.set(255,0,0,0);
	border = bar;

	kinectText=NULL;

	firstData=true;
	//this->setProgress(0);
}

void CKinectJoystickGUI::update(double zMovement,double xMovement){
	//ONLY FOR TESTING
	//core::stringw str =stringify(transmissionData).c_str(); // SPAGHETTI
	//gui::IGUIFont *font = this->gui->getFont("data/blinkfont.xml");    
	//core::dimension2d<s32> dim(128,128);
	//core::rect<s32> rec;

//	rec = core::rect<s32>(120,140-dim.Height/2, 90+dim.Width,65+dim.Height/2); 
	//if(kinectText)
		//kinectText->remove();
	//kinectText = this->gui->addStaticText(str.c_str(), rec, false,false);
	// text->setOverrideFont(font);
	//kinectText->setOverrideColor(video::SColor(255,255,77,0));
	//kinectText->setOverrideFont(font);
	
	//zMov values from -50 to 300
	//xMov values from -200 to 200
	xMov=xMovement;
	zMov=zMovement;
	//80+20

	if(zMov>100)
		zMov=100;
	else
		//-80-20
		if(zMov<-100)
			zMov=-100;
	if(xMov>100)
		xMov=100;
	else
		if(xMov<-100)
			xMov=-100;
}
void CKinectJoystickGUI::draw(){
	if(this->IsVisible == false)
		return;
	// Draw player rectangle
	if(plyerNo==1){
		for(int i=0;i<3;i++)
			for(int j=0;j<3;j++){
				irr::core::recti outlineRect = irr::core::recti(300+i*80,10+j*80, 300+80+i*80,10+80+j*80);
				if(!playerRect.isRectCollided(outlineRect))
					vdriver->draw2DRectangleOutline(outlineRect, video::SColor(255,255,0,0));
				else
					vdriver->draw2DRectangle( video::SColor(255,255,0,0),outlineRect);

			}
			playerRect= irr::core::recti(300+80+30+xMov, 10+80+30-zMov, 300+80+30+20+xMov,10+80+30+20-zMov); 
			vdriver->draw2DRectangle(irr::video::SColor(255, 255, 128, 64),playerRect);
	}

	if(plyerNo==2){ // second player
		//std::cout <<"DRAW SECOND";
		for(int i=0;i<3;i++)
			for(int j=0;j<3;j++){
				irr::core::recti outlineRect = irr::core::recti(1100+i*80,10+j*80, 1100+80+i*80,10+80+j*80);
				if(!playerRect.isRectCollided(outlineRect))
					vdriver->draw2DRectangleOutline(outlineRect, video::SColor(255,255,0,0));
				else
					vdriver->draw2DRectangle( video::SColor(255,140,0,0),outlineRect);

			}
			playerRect= irr::core::recti(1100+80+30+xMov, 10+80+30-zMov, 1100+80+30+20+xMov,10+80+30+20-zMov); 
			vdriver->draw2DRectangle(irr::video::SColor(255, 255, 128, 64),playerRect);
	}
}

bool CKinectJoystickGUI::rectInside(irr::core::recti& one,irr::core::recti& two){
	return true;
}