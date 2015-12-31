//
//  Fluido.cpp
//  Fluido
//  The MIT License (MIT)
//  Copyright (c) 2015 Luca Lolli.
//  Created by luca lolli on 21/12/2015.
//
//

#include <stdio.h>
#include "Fluido.h"
#include "cinder/System.h"

using namespace ci;
using namespace ci::app;
using namespace std;

namespace ds {
    Fluido::Fluido(int width, int height)
    {
        Fluido(ivec2(width,height));
    }
    
    Fluido::Fluido(ivec2 size):mSize(size)
    {
        mImpulseColorNew = false;
        mImpulseVelocityNew = false;
        mPrevTime = getElapsedSeconds();
        mDissipation = 0.995;
        mTimeStep = 1.0;
        mSigma = 0.05f;
        mKappa = 0.05f;
        mAmbientTemp = 0.0f;
        mInversBeta = 0.2485f;
        mCellSize = 1.25f;
        mNumIterations = 40;
        mGravity = vec3(0.0);
        mGradientScale = 1.125f;
        mVorticityForce = 0.0f;
        mMaxDensity = 0.0f;
        mMaxTemperature = 0.0f;
        mMaxVelocity = 0.0f;
        mClampingForce = 0.5f;
        
        initBuffers();
        
        mImpulsePoints = new ConcurrentCircularBuffer<impulsePoint>(100);
        
    }
    
    Fluido::~Fluido()
    {
        
        mParamRef->removeParam("dissipation");
        mParamRef->removeParam("timeStep");
        mParamRef->removeParam("Sigma");
        mParamRef->removeParam("Kappa");
        mParamRef->removeParam("Ambient Temp");
        mParamRef->removeParam("InversBeta");
        mParamRef->removeParam("Cell Size");
        mParamRef->removeParam("Num Jacobi Iterations");
        mParamRef->removeParam("Gravity X Y");
        mParamRef->removeParam("Gradient Scale");
        mParamRef->removeParam("Vorticity Force");
        
        mImpulsePoints->cancel();
    }
    
    void Fluido::initBuffers()
    {
        mVelocityBuffer = gpGpuFrameBuffer::create(mSize, GL_RG32F);
        mDensityBuffr = gpGpuFrameBuffer::create(mSize, GL_RGBA32F);
        mPressureBuffer = gpGpuFrameBuffer::create(mSize, GL_R32F);
        mTemperatureBuffer = gpGpuFrameBuffer::create(mSize, GL_RGBA32F);
        
        gl::Texture::Format TextureFormat;
        TextureFormat.setMagFilter( GL_LINEAR );
        TextureFormat.setMinFilter( GL_LINEAR );
        TextureFormat.setWrap(GL_REPEAT, GL_REPEAT);
        TextureFormat.setInternalFormat( GL_RGBA32F );
        
        gl::Fbo::Format format;
        
        format.setColorTextureFormat(TextureFormat);
        
        mVorticitySecondPassFbo = gl::Fbo::create(mSize.x, mSize.y, format);
        mVorticityFirstPassFbo = gl::Fbo::create(mSize.x, mSize.y, format);
        mDivergenceFbo = gl::Fbo::create(mSize.x, mSize.y, format);
        mHiResObstaclesFbo = gl::Fbo::create(mSize.x, mSize.y, format);
        mObstaclesFbo = gl::Fbo::create(mSize.x, mSize.y, format);
        
        clear(mObstaclesFbo, ColorA(0.0,0.0,0.0,0.0));
        clear(mDivergenceFbo, ColorA(0.0,0.0,0.0,0.0));
        clear(mHiResObstaclesFbo, ColorA(0.0,0.0,0.0,0.0));
        
        loadShaders();
        
        resetObstacles();
    }
    
    void Fluido::resetObstacles(bool isContained)
    {
        gl::ScopedFramebuffer fbo(mObstaclesFbo);
        gl::clear(ColorA(0.0, 0.0, 0.0, 0.0));
        if (isContained) {
            gl::color(ColorA(1.0,0.0,0.0,1.0));
            gl::drawStrokedRect( Rectf(vec2(0.0), mSize),5.0);
            gl::color(ColorA(1.0,1.0,1.0,1.0));
        }
    }
    
    void Fluido::addImpulsePoint(impulsePoint point)
    {
        mImpulsePoints->tryPushFront(point);
    }
    
    void Fluido::addConstantImpulsePoint(impulsePoint point)
    {
        mConstantImpulsePoints.push_back(point);
    }
    
    void Fluido::loadShaders()
    {
        try {
            
            advectShader = gl::GlslProg::GlslProg::create(gl::GlslProg::Format().vertex(loadAsset("Fluido/shaders/passThru.vert"))
                                                          .fragment( loadAsset("Fluido/shaders/advect.frag") ));
            jacobiShader = gl::GlslProg::GlslProg::create(gl::GlslProg::Format().vertex(loadAsset("Fluido/shaders/passThru.vert"))
                                                          .fragment( loadAsset("Fluido/shaders/jacobi.frag") ));
            buoyancyShader = gl::GlslProg::GlslProg::create(gl::GlslProg::Format().vertex(loadAsset("Fluido/shaders/passThru.vert"))
                                                            .fragment( loadAsset("Fluido/shaders/buoyancy.frag") ));
            divergenceShader = gl::GlslProg::GlslProg::create(gl::GlslProg::Format().vertex(loadAsset("Fluido/shaders/passThru.vert"))
                                                              .fragment( loadAsset("Fluido/shaders/divergence.frag") ));
            subGradientShader = gl::GlslProg::GlslProg::create(gl::GlslProg::Format().vertex(loadAsset("Fluido/shaders/passThru.vert"))
                                                               .fragment( loadAsset("Fluido/shaders/subtractGradient.frag") ));
            impulseShader = gl::GlslProg::GlslProg::create(gl::GlslProg::Format().vertex(loadAsset("Fluido/shaders/passThru.vert"))
                                                           .fragment( loadAsset("Fluido/shaders/impulse.frag") ));
            
            visualizeShader = gl::GlslProg::GlslProg::create(gl::GlslProg::Format().vertex(loadAsset("Fluido/shaders/passThru.vert"))
                                                             .fragment( loadAsset("Fluido/shaders/visualize.frag") ));
            velVisualizerShader = gl::GlslProg::create(gl::GlslProg::Format().vertex(loadAsset("Fluido/shaders/passThru.vert")).fragment(loadAsset("Fluido/shaders/visualizeVelocity.frag")));
            
            mImpulseTextureShader = gl::GlslProg::GlslProg::create(gl::GlslProg::Format().vertex(loadAsset("Fluido/shaders/passThru.vert"))
                                                                   .fragment( loadAsset("Fluido/shaders/impulseTexture.frag") ));
            
            vorticityFirstShader = gl::GlslProg::GlslProg::create(gl::GlslProg::Format().vertex(loadAsset("Fluido/shaders/passThru.vert"))
                                                                  .fragment( loadAsset("Fluido/shaders/vorticityFirst.frag") ));
            
            vorticitySecondShader = gl::GlslProg::GlslProg::create(gl::GlslProg::Format().vertex(loadAsset("Fluido/shaders/passThru.vert"))
                                                                  .fragment( loadAsset("Fluido/shaders/vorticitySecond.frag") ));
            
            addForceShader = gl::GlslProg::GlslProg::create(gl::GlslProg::Format().vertex(loadAsset("Fluido/shaders/passThru.vert"))
                                                            .fragment( loadAsset("Fluido/shaders/addTexture.frag") ));
            
            clampTextureShader = gl::GlslProg::GlslProg::create(gl::GlslProg::Format().vertex(loadAsset("Fluido/shaders/passThru.vert"))
                                                                .fragment( loadAsset("Fluido/shaders/clampTexture.frag") ));
            
        } catch ( gl::GlslProgCompileExc ex ) {
            console() << "GLSL Error: " << ex.what() << endl;
            
        } catch ( gl::GlslNullProgramExc ex ) {
            console() << "GLSL Error: " << ex.what() << endl;
            ci::app::App::get()->quit();
        }
        catch( std::exception &exc ) {
            console() << "Uncaught exception, type: " << System::demangleTypeName( typeid( exc ).name() ) << ", what : " << exc.what();
            ci::app::App::get()->quit();
        }
        catch ( ... ) {
            console() << "Unknown GLSL Error" << endl;
            ci::app::App::get()->quit();
        }
    }
    
    void Fluido::registerParams(const params::InterfaceGlRef &params)
    {
        mParamRef = params;
        
        mParamRef->addParam("dissipation", &mDissipation);
        mParamRef->addParam("timeStep", &mTimeStep);
        mParamRef->addParam("Sigma", &mSigma);
        mParamRef->addParam("Kappa", &mKappa);
        mParamRef->addParam("Ambient Temp", &mAmbientTemp);
        mParamRef->addParam("InversBeta", &mInversBeta, "step=0.0005 min=0.0");
        mParamRef->addParam("Cell Size", &mCellSize, "step=0.001 min=0.0");
        mParamRef->addParam("Num Jacobi Iterations", &mNumIterations, "step=1 min=0 max=300");
        mParamRef->addParam("Gravity X Y", &mGravity);
        mParamRef->addParam("Gradient Scale", &mGradientScale, "step=0.01");
        mParamRef->addParam("Vorticity Force", &mVorticityForce, "step=0.1 max=200.0 min=0.0 ");
        mParamRef->addParam("Max Velocity", &mMaxVelocity, "step=0.1 max=200.0 min=0.0 ");
        mParamRef->addParam("Max Density", &mMaxDensity, "step=0.1 max=200.0 min=0.0 ");
        mParamRef->addParam("Max Temperature", &mMaxTemperature, "step=0.1 max=200.0 min=0.0 ");
        mParamRef->addParam("Clamping Force", &mClampingForce, "step=0.05 max=200.0 min=0.0 ");
        
    }
    
    void Fluido::clear(const gl::FboRef &fbo, ColorA color)
    {
        gl::ScopedFramebuffer   scopedfbo(fbo);
        gl::viewport(fbo->getSize());
        gl::setMatricesWindow(fbo->getSize());
        gl::clear(color);
        
    }
    
    void Fluido::update(float deltaT)
    {
        gl::ScopedBlend alpha(false);
        float currTime = getElapsedSeconds();
        mDeltaT = deltaT==-1.0f?(currTime-mPrevTime):deltaT;
        mDeltaT *= mTimeStep;
        
        mPrevTime = currTime;
        
        if (mMaxVelocity>0.0) {
            clampTexture(mVelocityBuffer, mMaxVelocity, mClampingForce);
        }
        if (mMaxDensity>0.0) {
            clampTexture(mDensityBuffr, mMaxDensity, mClampingForce);
        }
        if (mMaxTemperature>0.0) {
            clampTexture(mTemperatureBuffer, mMaxTemperature, mClampingForce);
        }

        if (mVorticityForce>0.0f) {
            vorticityFirst(mVorticityFirstPassFbo, mVelocityBuffer->getTexture(), mObstaclesFbo->getColorTexture());
            
            vorticitySecond(mVorticitySecondPassFbo, mVorticityFirstPassFbo->getColorTexture(), mDeltaT, 1.0, mCellSize);
            
            addForce(mVelocityBuffer,mVorticitySecondPassFbo->getColorTexture(),mVorticityForce);
        }
        
        Advect(mVelocityBuffer, mVelocityBuffer->getTexture(), mObstaclesFbo->getColorTexture(), mDissipation, mDeltaT);
        
        Advect(mTemperatureBuffer, mVelocityBuffer->getTexture(), mObstaclesFbo->getColorTexture(), mDissipation, mDeltaT);
        
        Advect(mDensityBuffr, mVelocityBuffer->getTexture(), mObstaclesFbo->getColorTexture(),mDissipation, mDeltaT);
        
        Buoyancy(mVelocityBuffer, mTemperatureBuffer->getTexture(), mDensityBuffr->getTexture(), mAmbientTemp, mDeltaT, mKappa, mSigma, mGravity);
        
        while( mImpulsePoints->isNotEmpty()) {
            impulsePoint p;
            mImpulsePoints->tryPopBack(&p);
            
            if(p.addDens){
                injectImpulse(mTemperatureBuffer->getBuffer(), p.position, p.radius, p.color);
            }
            
            if (p.addTemp) {
                injectImpulse(mDensityBuffr->getBuffer(), p.position, p.radius, p.color);
            }
            
            if (p.magnitude>0.0 && p.addVel)
            {
                injectImpulse(mVelocityBuffer->getBuffer(), p.position, p.radius, vec4(p.direction,1.0,1.0));
            }
        }
        
        for (auto p : mConstantImpulsePoints) {
            
            injectImpulse(mTemperatureBuffer->getBuffer(), p.position, p.radius, p.color);
            injectImpulse(mDensityBuffr->getBuffer(), p.position, p.radius, p.color);
            
            if (p.magnitude>0.0)
            {
                injectImpulse(mVelocityBuffer->getBuffer(), p.position, p.radius, vec4(p.direction,1.0,1.0));
            }
        }
        
        if (mImpulseColorNew) {
            injectTexture(mTemperatureBuffer, mImpulseColorTex);
            injectTexture(mDensityBuffr, mImpulseColorTex);
            mImpulseColorNew = false;
        }
        if (mImpulseVelocityNew) {
            injectTexture(mVelocityBuffer, mImpulseVelocityTex);
            mImpulseVelocityNew = false;
        }
        
        ComputeDivergence(mDivergenceFbo, mVelocityBuffer->getTexture(), mObstaclesFbo->getColorTexture(), mCellSize);
        
        clear(mPressureBuffer->getBuffer());
        
        for (int i = 0; i < mNumIterations; ++i) {
            Jacobi(mPressureBuffer, mDivergenceFbo->getColorTexture(), mObstaclesFbo->getColorTexture(), mCellSize, mInversBeta);
        }
        
        subtractGradient(mVelocityBuffer, mPressureBuffer->getTexture(), mObstaclesFbo->getColorTexture(), mCellSize);
    }
    
    void Fluido::clampTexture(const gpGpuFrameBufferRef &buffer, float maxValue, float clampingForce)
    {
        gl::ScopedFramebuffer fbo( buffer->getBuffer() );
        gl::ScopedViewport viewPort(buffer->getSize());
        gl::setMatricesWindow(buffer->getSize());
        buffer->drawBuffer();
        
        gl::ScopedTextureBind tex0( buffer->getTexture(), 0 );
        
        gl::ScopedGlslProg shader(clampTextureShader);
        
        clampTextureShader->uniform("uTexture", 0);
        
        clampTextureShader->uniform("uMaxValue", maxValue);
        
        clampTextureShader->uniform("uClampForce", clampingForce);
        
        gl::drawSolidRect(Rectf(0.0,0.0,mSize.x,mSize.y));
        
        buffer->swap();
    }
    
    void Fluido::addForce(const gpGpuFrameBufferRef &buffer, const gl::TextureRef &texture, float force)
    {
        
        gl::ScopedFramebuffer fbo( buffer->getBuffer() );
        gl::ScopedViewport viewPort(buffer->getSize());
        gl::setMatricesWindow(buffer->getSize());
        buffer->drawBuffer();
        
        gl::ScopedTextureBind tex0( buffer->getTexture(), 0 );
        gl::ScopedTextureBind tex1( texture, 1 );
        
        gl::ScopedGlslProg shader(addForceShader);
        
        addForceShader->uniform("uBackbuffer", 0);
        
        addForceShader->uniform("uAddTexture", 1);
        
        addForceShader->uniform("uForce", force);
        
        gl::drawSolidRect(Rectf(0.0,0.0,mSize.x,mSize.y));
        
        buffer->swap();
    }
    
    void Fluido::vorticityFirst(const gl::FboRef &buffer, const gl::TextureRef& velocityTexture, const gl::TextureRef& obstaclesTexture){
        
        gl::ScopedFramebuffer fbo( buffer );
        gl::ScopedViewport viewPort(buffer->getSize());
        gl::setMatricesWindow(buffer->getSize());
        
        gl::ScopedTextureBind tex1( velocityTexture, 0 );
        gl::ScopedTextureBind tex2( obstaclesTexture, 1 );
        
        gl::ScopedGlslProg shader(vorticityFirstShader);
        
        vorticityFirstShader->uniform("uVelocity", 0);
        vorticityFirstShader->uniform("uObstacle", 1);
        
        gl::drawSolidRect(Rectf(0.0,0.0,mSize.x,mSize.y));
        
    }
    
    void Fluido::vorticitySecond(const gl::FboRef &buffer, const gl::TextureRef& vorticityTexture, float deltaT, float scale, float cellSize)
    {
        
        gl::ScopedFramebuffer fbo( buffer );
        gl::ScopedViewport viewPort(buffer->getSize());
        gl::setMatricesWindow(buffer->getSize());
        
        gl::ScopedTextureBind tex0( vorticityTexture, 0 );
        
        gl::ScopedGlslProg shader(vorticitySecondShader);
        
        vorticitySecondShader->uniform("uVorticity", 0);
        
        vorticitySecondShader->uniform("uHalfInverseCellSize", float(0.5f / cellSize));
        
        vorticitySecondShader->uniform("uDeltaT", deltaT);
        vorticitySecondShader->uniform("uConfinementScale", 1.0f);
        
        gl::drawSolidRect(Rectf(0.0,0.0,mSize.x,mSize.y));
        
    }
    
    void Fluido::Advect(const gpGpuFrameBufferRef &buffer, const gl::TextureRef &velocity, const gl::TextureRef &obstacles, float dissipation, float timeStep)
    {
        
        gl::ScopedFramebuffer   fbo(buffer->getBuffer());
        gl::viewport(buffer->getSize());
        gl::setMatricesWindow(buffer->getSize());
        buffer->drawBuffer();
        
        gl::ScopedGlslProg prog(advectShader);
        
        gl::ScopedTextureBind tex0(velocity,0);
        gl::ScopedTextureBind tex1(buffer->getTexture(),1);
        gl::ScopedTextureBind tex2(obstacles,2);
        
        advectShader->uniform("uVelocityTex", 0);
        advectShader->uniform("uSourceTex", 1);
        advectShader->uniform("uObstacles", 2);
        
        advectShader->uniform("uDissipation", dissipation);
        advectShader->uniform("uTimeStep", timeStep);
        
        gl::color(Color::white());
        gl::drawSolidRect(Rectf(vec2(0.0,0.0), buffer->getSize()));
        buffer->swap();
    }
    
    void Fluido::Buoyancy(const gpGpuFrameBufferRef &buffer, const gl::TextureRef &temperature, const gl::TextureRef &density, float ambientTemperature, float timeStep, float kappa, float sigma, vec3 gravity)
    {
        
        gl::ScopedFramebuffer   fbo(buffer->getBuffer());
        gl::viewport(buffer->getSize());
        gl::setMatricesWindow(buffer->getSize());
        buffer->drawBuffer();
        
        gl::ScopedGlslProg prog(buoyancyShader);
        
        gl::ScopedTextureBind tex0(buffer->getTexture(),0);
        gl::ScopedTextureBind tex1(temperature,1);
        gl::ScopedTextureBind tex2(density,2);
        
        buoyancyShader->uniform("uVelocity", 0);
        buoyancyShader->uniform("uTemperature", 1);
        buoyancyShader->uniform("uDensity", 2);
        buoyancyShader->uniform("uGravity", gravity);
        
        buoyancyShader->uniform("uKappa", kappa);
        buoyancyShader->uniform("uSigma", sigma);
        buoyancyShader->uniform("uTimeStep", timeStep);
        buoyancyShader->uniform("uAmbientTemperature", ambientTemperature);
        
        gl::color(Color::white());
        gl::drawSolidRect(Rectf(vec2(0.0,0.0), buffer->getSize()));
        buffer->swap();
    }
    
    void Fluido::ComputeDivergence(const gl::FboRef &destFbo, const gl::TextureRef &velocity, const gl::TextureRef &obstacles, float cellSize)
    {
        
        gl::ScopedFramebuffer   fbo(destFbo);
        gl::viewport(destFbo->getSize());
        gl::setMatricesWindow(destFbo->getSize());
        
        gl::ScopedGlslProg prog(divergenceShader);
        gl::ScopedTextureBind tex1(velocity,0);
        gl::ScopedTextureBind tex2(obstacles,1);
        
        divergenceShader->uniform("uVelocity", 0);
        divergenceShader->uniform("uObstacles", 1);
        divergenceShader->uniform("uHalfInverseCellSize", 0.5f/cellSize);
        
        gl::color(Color::white());
        gl::drawSolidRect(Rectf(vec2(0.0,0.0), destFbo->getSize()));
        
    }
    
    void Fluido::Jacobi(const gpGpuFrameBufferRef &buffer, const gl::TextureRef &divergence, const gl::TextureRef &obstacles, float cellSize, float inverseBeta)
    {
        
        gl::ScopedFramebuffer   fbo(buffer->getBuffer());
        gl::viewport(buffer->getSize());
        gl::setMatricesWindow(buffer->getSize());
        
        buffer->drawBuffer();
        
        gl::ScopedGlslProg prog(jacobiShader);
        gl::ScopedTextureBind tex0(buffer->getTexture(),0);
        gl::ScopedTextureBind tex1(divergence,1);
        gl::ScopedTextureBind tex2(obstacles,2);
        
        jacobiShader->uniform("uPressureTex", 0);
        jacobiShader->uniform("uDivergenceTex", 1);
        jacobiShader->uniform("uObstacles", 2);
        
        jacobiShader->uniform("uAlpha", -cellSize*cellSize);
        jacobiShader->uniform("uInverseBeta", inverseBeta);
        
        gl::color(Color::white());
        gl::drawSolidRect(Rectf(vec2(0.0,0.0), buffer->getSize()));
        buffer->swap();
        
    }
    
    void Fluido::subtractGradient(const gpGpuFrameBufferRef &buffer, const gl::TextureRef &pressure, const gl::TextureRef &obstacles, float cellSize)
    {
        
        gl::ScopedFramebuffer   fbo(buffer->getBuffer());
        gl::viewport(buffer->getSize());
        gl::setMatricesWindow(buffer->getSize());
        buffer->drawBuffer();
        
        gl::ScopedGlslProg prog(subGradientShader);
        gl::ScopedTextureBind tex0(buffer->getTexture(),0);
        gl::ScopedTextureBind tex1(pressure,1);
        gl::ScopedTextureBind tex2(obstacles,2);
        
        subGradientShader->uniform("uVelocityTex", 0);
        subGradientShader->uniform("uPressureTex", 1);
        
        subGradientShader->uniform("uObstacles", 2);
        
        subGradientShader->uniform("uGradientScale", mGradientScale/cellSize );
        
        gl::color(Color::white());
        gl::drawSolidRect(Rectf(vec2(0.0,0.0), buffer->getSize()));
        buffer->swap();
    }
    
    void Fluido::injectImpulse(const gl::FboRef &destFbo, vec2 position, float radius, vec4 color)
    {
        
        gl::ScopedBlend alpha(true);
        
        gl::ScopedFramebuffer   fbo(destFbo);
        gl::viewport(destFbo->getSize());
        gl::setMatricesWindow(destFbo->getSize());
        
        gl::ScopedGlslProg prog(impulseShader);
        
        impulseShader->uniform("uPoint", position*vec2(mSize));
        impulseShader->uniform("uRadius", radius);
        impulseShader->uniform("uColor", color);
        gl::color(Color::white());
        
        gl::drawSolidRect(Rectf(vec2(0.0,0.0), destFbo->getSize()));
        
    }
    
    void Fluido::injectTexture(const gpGpuFrameBufferRef &buffer, const gl::TextureRef &texture)
    {
        
//        gl::ScopedBlendAdditive alpha;
        gl::ScopedFramebuffer   fbo(buffer->getBuffer());
        gl::viewport(buffer->getSize());
        gl::setMatricesWindow(buffer->getSize());
        
        gl::ScopedGlslProg prog(mImpulseTextureShader);
        
        gl::ScopedTextureBind tex0(buffer->getTexture(),0);
        gl::ScopedTextureBind tex1(texture,1);
        
        mImpulseTextureShader->uniform("uTexture", 0);
        mImpulseTextureShader->uniform("uImpulseTex", 1);
        
        gl::color(Color::white());
        
        gl::drawSolidRect(Rectf(vec2(0.0,0.0), buffer->getSize()));
        
//        gl::draw(texture,Rectf(vec2(0.0,0.0), buffer->getSize()));

    }
    
    
    void Fluido::addImpulseTexture(const gl::TextureRef &colorTexture)
    {
        mImpulseColorNew = true;
        mImpulseColorTex = colorTexture;
    }
    
    void Fluido::addImpulseTexture(const gl::TextureRef &colorTexture, const gl::TextureRef &velocityTexture)
    {
        mImpulseColorNew = true;
        mImpulseVelocityNew = true;
        mImpulseColorTex = colorTexture;
        mImpulseVelocityTex = velocityTexture;
    }
    
    ivec2 Fluido::getSize()
    {
        return mSize;
    }
    
    void Fluido::addObstacle(const gl::TextureRef &obstacle)
    {
        gl::ScopedBlendAdditive blendAdd;
        gl::ScopedFramebuffer fbo(mObstaclesFbo);
        gl::ScopedViewport viewport(mObstaclesFbo->getSize());
        gl::setMatricesWindow(mObstaclesFbo->getSize());
        gl::color(1.0,1.0,1.0,1.0);
        gl::draw(obstacle, mObstaclesFbo->getBounds());
    }
    
    void Fluido::drawDensity(vec2 size)
    {
        drawDensity(Rectf(vec2(0.0f), size));
    }
    
    void Fluido::drawVelocity(vec2 size)
    {
        drawVelocity(Rectf(vec2(0.0f), size));
    }
    
    void Fluido::drawTemperature(vec2 size)
    {
        drawTemperature(Rectf(vec2(0.0f), size));
    }
    
    void Fluido::drawPressure(vec2 size)
    {
        drawPressure(Rectf(vec2(0.0f), size));
    }
    
    void Fluido::drawObstacles(vec2 size)
    {
        drawObstacles(Rectf(vec2(0.0f), size));
    }
    
    void Fluido::drawDensity(Rectf bounds)
    {
        gl::ScopedTextureBind tex0(mDensityBuffr->getTexture());
        gl::ScopedGlslProg shader(visualizeShader);
        visualizeShader->uniform("uTex", 0);
        gl::drawSolidRect(bounds);
    }
    
    void Fluido::drawVelocity(Rectf bounds)
    {
        gl::ScopedTextureBind tex0(mVelocityBuffer->getTexture());
        gl::ScopedGlslProg shader(velVisualizerShader);
        velVisualizerShader->uniform("uTex", 0);
        gl::drawSolidRect(bounds);
    }
    
    void Fluido::drawTemperature(Rectf bounds)
    {
        gl::ScopedTextureBind tex0(mTemperatureBuffer->getTexture());
        gl::ScopedGlslProg shader(visualizeShader);
        visualizeShader->uniform("uTex", 0);
        gl::drawSolidRect(bounds);
    }
    
    void Fluido::drawPressure(Rectf bounds)
    {
        gl::ScopedTextureBind tex0(mPressureBuffer->getTexture());
        gl::ScopedGlslProg shader(visualizeShader);
        visualizeShader->uniform("uTex", 0);
        gl::drawSolidRect(bounds);
    }
    
    void Fluido::drawObstacles(Rectf bounds)
    {
        gl::ScopedTextureBind tex0(mObstaclesFbo->getColorTexture());
        gl::ScopedGlslProg shader(visualizeShader);
        visualizeShader->uniform("uTex", 0);
        gl::drawSolidRect(bounds);
    }
}
