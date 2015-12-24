#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/System.h"
#include "cinder/ImageIo.h"
#include "gpGpuFrameBuffer.h"
#include "cinder/Utilities.h"
#include "cinder/params/Params.h"

#include "Fluido.h"

using namespace ci;
using namespace ci::app;
using namespace std;
using namespace ds;

class FluidoApp : public App {
  public:
	void setup() override;
	void mouseDown( MouseEvent event ) override;
    void mouseDrag( MouseEvent event ) override;
    void mouseMove( MouseEvent event ) override;
    void mouseUp( MouseEvent event ) override;
    void keyDown( KeyEvent event ) override;
    void keyUp( KeyEvent event ) override;
    
    void init();
    
	void update() override;
	void draw() override;
    
    params::InterfaceGlRef  mParams;
    
    bool            mMouseDown;
    
    vec2            mMousePos, mMouseDir;
    
    float           mPrevTime;
    
    FluidoRef       mFluido;
    
    gl::TextureRef     mTex;
    
    ColorA             mColor;
    float              mRadius;
    float              mForce;
    
    ivec2               mFluidoSize;
    bool                isCtrlDown;
    
    string              mFpsString;
};

void FluidoApp::setup()
{
    isCtrlDown = false;
    mMouseDown = false;
    mMouseDir = vec2(0.0);
    mColor.set(CM_RGB, vec4(1.0,1.0,1.0,1.0));
    mForce = 100.0f;
    mRadius = 10.0f;
    mFluidoSize = ivec2(512,512);
    
    mTex = gl::Texture::create(loadImage(loadAsset("triangle.png")));
    mParams = params::InterfaceGl::create("Fluido", ivec2(300,400));
    
    mFpsString = toString(getAverageFps());
    
    mParams->addParam("FPS: ", &mFpsString);
    
    mParams->addText("Hold down LCTRL to add constant impulse");
    mParams->addSeparator();
    
    mParams->addParam("Fluido Width", &mFluidoSize.x, "min=0 step=1");
    mParams->addParam("Fluido Height", &mFluidoSize.y, "min=0 step=1");
    
    mParams->addButton("Apply to fluid", function<void()>([this](){
        
        init();
        
    }));
    
    mParams->addParam("Impulse Color", &mColor);
    mParams->addParam("Impulse Radius", &mRadius);
    mParams->addParam("Impulse Force", &mForce);
    
    init();
    
}

void FluidoApp::init()
{
    mFluido = Fluido::create(mFluidoSize);
    mFluido->resetObstacles(true);
    
    mTex->setTopDown();
    
    mFluido->registerParams(mParams);
    
    mFluido->addObstacle(mTex);
}

void FluidoApp::mouseDown( MouseEvent event )
{
    mMouseDir = vec2(0.0);
    mMouseDown = true;
    mMousePos = vec2(event.getPos())/ vec2(getWindowSize());
    
    if (isCtrlDown) {
        mFluido->addConstantImpulsePoint({mMousePos*vec2(mFluido->getSize()),mMouseDir,length(mMouseDir), mColor,mRadius,100.0f });
    }
}

void FluidoApp::mouseDrag( MouseEvent event )
{
    const vec2 normalizezPos = vec2(event.getPos())/ vec2(getWindowSize());
    mMouseDir = vec2( normalizezPos - mMousePos );
    mMouseDir *= mForce;
    mMousePos = normalizezPos;
    
}

void FluidoApp::mouseMove( MouseEvent event )
{
    const vec2 normalizezPos = vec2(event.getPos())/ vec2(getWindowSize());
    mMouseDir = vec2( normalizezPos - mMousePos );
    mMouseDir *= mForce;
    mMousePos = normalizezPos;
    
}

void FluidoApp::mouseUp( MouseEvent event )
{
    mMouseDir = vec2(0.0);
    mMouseDown = false;
}


void FluidoApp::update()
{

    float deltaT = (getElapsedSeconds() - mPrevTime);
    mPrevTime = getElapsedSeconds();
    
    //the delta time is in ms.
    mFpsString = toString(1.0f/deltaT);

    mFluido->update(deltaT);
    
    if (mMouseDown) {
        impulsePoint p {mMousePos*vec2(mFluido->getSize()),mMouseDir,length(mMouseDir), mColor,mRadius,100.0f };
        mFluido->addImpulsePoint(p);
    }
}

void FluidoApp::keyDown(KeyEvent event)
{
    switch (event.getCode()) {
        case KeyEvent::KEY_SPACE:
            mFluido->loadShaders();
            break;
        case KeyEvent::KEY_LCTRL:
            isCtrlDown = true;
            break;
        default:
            break;
    }
}

void FluidoApp::keyUp(KeyEvent event)
{
    switch (event.getCode()) {
        case KeyEvent::KEY_LCTRL:
            isCtrlDown = false;
            break;
        default:
            break;
    }
}

void FluidoApp::draw()
{
    gl::clear();
    gl::viewport(getWindowSize());
    gl::setMatricesWindow(getWindowSize());
    
    mFluido->drawDensity(getWindowBounds());
//    mFluido->drawObstacles(getWindowBounds());
//    gl::draw(mTex,getWindowBounds());
    
    if (isCtrlDown) {
        gl::color(mColor);
        const float scaledRadius = mRadius*(float(getWindowSize().x)/float(mFluidoSize.x));
        gl::drawString("Add Constant Impulse", mMousePos*vec2(getWindowSize()) - vec2(0.0, mRadius+10.0f));
        gl::drawStrokedCircle(mMousePos*vec2(getWindowSize()), scaledRadius);
    }
    
    mParams->draw();
    
}

CINDER_APP( FluidoApp, RendererGl, [&](App::Settings *settings) {
    settings->setFullScreen(true);
    settings->setFrameRate(60);
})
