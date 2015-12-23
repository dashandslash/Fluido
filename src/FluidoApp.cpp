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
    void mouseUp( MouseEvent event ) override;
    void keyDown( KeyEvent event ) override;
    
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
};

void FluidoApp::setup()
{
    mMouseDown = false;
    mMouseDir = vec2(0.0);
    mColor.set(CM_RGB, vec4(1.0,1.0,1.0,1.0));
    mForce = 100.0f;
    mRadius = 10.0f;
    mFluido = Fluido::create(512, 512);
    mFluido->resetObstacles(true);
    
    mTex = gl::Texture::create(loadImage(loadAsset("triangle.png")));
    mParams = params::InterfaceGl::create("Fluido", ivec2(200,400));
    

    mParams->addParam("Impulse Color", &mColor);
    mParams->addParam("Impulse Radius", &mRadius);
    mParams->addParam("Impulse Force", &mForce);
    
    
    mTex->setTopDown();
    

    mFluido->registerParams(mParams);
    
    mFluido->addObstacle(mTex);
}

void FluidoApp::mouseDown( MouseEvent event )
{
    mMouseDir = vec2(0.0);
    mMouseDown = true;
    mMousePos = vec2(event.getPos())/ vec2(getWindowSize());
}

void FluidoApp::mouseDrag( MouseEvent event )
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
    gl::drawString("FPS: " + toString(getAverageFps()), vec2(20));
    mParams->draw();
    
}

CINDER_APP( FluidoApp, RendererGl, [&](App::Settings *settings) {
    settings->setFullScreen(true);
    settings->setFrameRate(60);
})
