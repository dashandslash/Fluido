#include "cinder/app/App.h"
#include "cinder/app/RendererGl.h"
#include "cinder/gl/gl.h"
#include "cinder/System.h"
#include "cinder/ImageIo.h"
#include "gpGpuFrameBuffer.h"
#include "cinder/Utilities.h"
#include "cinder/params/Params.h"

#include "cinder/Capture.h"

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
    
    gl::TextureRef     mTex, mTex1;
    
    ColorA             mColor;
    float              mRadius;
    float              mForce;
    
    ivec2               mFluidoSize;
    bool                isCtrlDown;
    
    string              mFpsString;
    
    int                 mDrawingMode;
    
    bool                mDrawObstacles;
    
    gl::FboRef          mImpulseVelocityTex;
    gl::FboRef          mImpulseColorTex;
    
    CaptureRef          mCapture;
    gl::TextureRef      mCaptureTex;
};

void FluidoApp::setup()
{
    mDrawObstacles = false;
    isCtrlDown = false;
    mMouseDown = false;
    mMouseDir = vec2(0.0);
    mColor.set(CM_RGB, vec4(1.0,1.0,1.0,1.0));
    mForce = 100.0f;
    mRadius = 10.0f;
    mFluidoSize = ivec2(512,512);
    
    gl::Texture::Format TextureFormat;
    TextureFormat.setMagFilter( GL_LINEAR );
    TextureFormat.setMinFilter( GL_LINEAR );
    TextureFormat.setWrap(GL_REPEAT, GL_REPEAT);
    TextureFormat.setInternalFormat( GL_RGBA32F );
    
    gl::Fbo::Format format;
    
    format.setColorTextureFormat(TextureFormat);
    
    mImpulseColorTex = gl::Fbo::create(mFluidoSize.x, mFluidoSize.y, format);
    mImpulseVelocityTex = gl::Fbo::create(mFluidoSize.x, mFluidoSize.y, format);
    
    try {
        mCapture = Capture::create( 640, 480 );
        mCapture->start();
    }
    catch( ci::Exception &exc ) {
        console()<< "Failed to init capture " << endl;;
    }
    
    {
        gl::ScopedFramebuffer fbo(mImpulseVelocityTex);
        gl::clear(ColorA(0.0,0.0,0.0,0.0));
        gl::ScopedViewport viewport(mFluidoSize);
        gl::setMatricesWindow(mFluidoSize);
        gl::color(-3.0, 0.0, 0.0);
        gl::drawSolidRect(Rectf(100.0f,50.0f,150.0f,150));
        
        
    }
    {
        gl::ScopedFramebuffer fbo(mImpulseColorTex);
        gl::clear(ColorA(0.0,0.0,0.0,0.0));
        gl::ScopedViewport viewport(mFluidoSize);
        gl::setMatricesWindow(mFluidoSize);
        gl::color(1.0, 1.0, 1.0);
        gl::drawSolidRect(Rectf(100.0f,50.0f,150.0f,150));
        
        
    }
    
    mTex = gl::Texture::create(loadImage(loadAsset("triangle.png")));
    mTex1 = gl::Texture::create(loadImage(loadAsset("tex1.png")));
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
    mDrawingMode = 1;
    std::vector<std::string> mode;
    mode.push_back( "Velocity as Angle" );
    mode.push_back( "Density" );
    mode.push_back( "Temperature" );
    mode.push_back( "Pressure" );
    
    mParams->addParam( "Draw Mode: ", mode, &mDrawingMode );
    mParams->addParam("Draw Obstacles", &mDrawObstacles);
    
    init();
    
}

void FluidoApp::init()
{
    mFluido = Fluido::create(mFluidoSize);
    mFluido->resetObstacles(true);
    
    mTex->setTopDown();
    
    mFluido->registerParams(mParams);
    
//    mFluido->addObstacle(mTex);
}

void FluidoApp::mouseDown( MouseEvent event )
{
    mMouseDir = vec2(0.0);
    mMouseDown = true;
    mMousePos = vec2(event.getPos())/ vec2(getWindowSize());
    
    if (isCtrlDown) {
        //Mouse Position has to be normalized from 0 to 1
        mFluido->addConstantImpulsePoint({mMousePos,mMouseDir,length(mMouseDir), mColor,mRadius,100.0f, true, true, true });
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
    
//    if (mCapture->checkNewFrame())
    {
        
        if( mCapture && mCapture->checkNewFrame() ) {
            if( ! mCaptureTex ) {
                // Capture images come back as top-down, and it's more efficient to keep them that way
                mCaptureTex = gl::Texture::create( *mCapture->getSurface(), gl::Texture::Format().loadTopDown() );
            }
            else {
                mCaptureTex->update( *mCapture->getSurface() );

            }
            
        }
        {
            gl::ScopedFramebuffer fbo(mImpulseVelocityTex);
            gl::clear(ColorA(0.0,0.0,0.0,0.0));
            gl::ScopedViewport viewport(mFluidoSize);
            gl::setMatricesWindow(mFluidoSize);
            gl::color(0.0, 0.0, 0.0);
            gl::draw(mCaptureTex,Rectf(vec2(0.0,mFluidoSize.y),vec2(mFluidoSize.x,0.0)));
            //            cam =  ( mCapture->getSurface() );
        }
        {
            gl::ScopedFramebuffer fbo(mImpulseColorTex);
            gl::clear(ColorA(0.0,0.0,0.0,0.0));
            gl::ScopedViewport viewport(mFluidoSize);
            gl::setMatricesWindow(mFluidoSize);
            gl::color(1.0, 1.0, 1.0,1.0);
            gl::draw(mCaptureTex,Rectf(vec2(mFluidoSize),vec2(0.0,0.0)));
            
        }
        
    }
//    mFluido->addImpulseTexture(mImpulseColorTex->getColorTexture());//, mImpulseVelocityTex->getColorTexture());

    float deltaT = (getElapsedSeconds() - mPrevTime);
    mPrevTime = getElapsedSeconds();
    
    //the delta time is in ms.
    mFpsString = toString(1.0f/deltaT);

    mFluido->update(deltaT);
    
    if (mMouseDown) {
        //Mouse Position has to be normalized from 0 to 1
        impulsePoint p {mMousePos,mMouseDir,length(mMouseDir), mColor,mRadius,100.0f, true, true, true };
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
        case KeyEvent::KEY_z:
//            mFluido->addImpulseTexture(mImpulseColorTex->getColorTexture(), mImpulseVelocityTex->getColorTexture());
            mFluido->addImpulseTexture(mImpulseColorTex->getColorTexture());//, mImpulseVelocityTex->getColorTexture());
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
    gl::clear(ColorA(0.0,0.0,0.0,0.0));
    gl::viewport(getWindowSize());
    gl::setMatricesWindow(getWindowSize());
    gl::ScopedBlendAdditive blend;

    switch (mDrawingMode) {
        case 0:
            mFluido->drawVelocity(getWindowBounds());
            break;
        case 1:
            mFluido->drawDensity(getWindowBounds());
            break;
        case 2:
            mFluido->drawTemperature(getWindowBounds());
            break;
        case 3:
            mFluido->drawPressure(getWindowBounds());
            break;
        default:
            break;
    }
    if (mDrawObstacles) {
        
        mFluido->drawObstacles(getWindowBounds());
        
    }
    
    if (isCtrlDown) {
        gl::color(mColor);
        const float scaledRadius = mRadius*(float(getWindowSize().x)/float(mFluidoSize.x));
        gl::drawString("Add Constant Impulse", mMousePos*vec2(getWindowSize()) - vec2(0.0, mRadius+10.0f));
        gl::drawStrokedCircle(mMousePos*vec2(getWindowSize()), scaledRadius);
    }
    

    gl::color(1.0, 1.0, 1.0,1.0);
//    gl::draw(mCaptureTex, Rectf(vec2(getWindowWidth(),0.0), vec2(0.0,getWindowHeight())));
    
    mParams->draw();

}

CINDER_APP( FluidoApp, RendererGl, [&](App::Settings *settings) {
    settings->setFullScreen(true);
    settings->setFrameRate(60);
    settings->setMultiTouchEnabled();
})
