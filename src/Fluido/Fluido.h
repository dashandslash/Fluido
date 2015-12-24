//
//  Fluido.h
//  Fluido
//  The MIT License (MIT)
//  Copyright (c) 2015 Luca Lolli.
//  Created by luca lolli on 21/12/2015.
//
//

#ifndef Fluido_h
#define Fluido_h

#include "cinder/Cinder.h"
#include "cinder/gl/gl.h"
#include "gpGpuFrameBuffer.h"
#include "cinder/params/Params.h"
#include "cinder/ConcurrentCircularBuffer.h"

using namespace ci;
using namespace ci::app;
using namespace std;

namespace ds {
    typedef shared_ptr<class Fluido> FluidoRef;
    
    struct impulsePoint {
        
        vec2    position;
        vec2    direction;
        float   magnitude;
        
        ColorA  color;
        float   radius;
        float   force;
        
    };

    class Fluido{
    public:
        static FluidoRef create(int width, int height){
            return FluidoRef( new Fluido{ ivec2(width,height) });
        }
        static FluidoRef create(ivec2 size){
            return FluidoRef( new Fluido{ size });
        }
        
        Fluido(int width, int height);
        
        Fluido(ivec2 size);
        
        void resetObstacles(bool isContained = false);
        
        void loadShaders();
        //! Register the parameters to the cinder InterfaceGL
        void registerParams(const params::InterfaceGlRef &params);
        //! Update the simulation, the Default value is the internal dT.
        void update(float deltaT = -1.0f);
        //! Draw a temporary circle into the Density and Temperature buffer, if magnetude > 0 draw also into the velocity texture with the corresponding direction value.
        void addImpulsePoint(impulsePoint point);
        //! Draw a circle every frame into the Density and Temperature buffer, if magnetude > 0 draw also into the velocity texture with the corresponding direction value.
        void addConstantImpulsePoint(impulsePoint point);
        
        void addObstacle(const gl::TextureRef &obstacle);
        
        ivec2 getSize();
        
        void drawVelocity(vec2 size);
        
        void drawDensity(vec2 size);
        
        void drawObstacles(vec2 size);
        
        void drawTemperature(vec2 size);
        
        void drawPressure(vec2 size);
        
        void drawVelocity(Rectf bounds);
        
        void drawDensity(Rectf bounds);
        
        void drawObstacles(Rectf bounds);
        
        void drawTemperature(Rectf bounds);
        
        void drawPressure(Rectf bounds);
        
        ~Fluido();
        
    private:
        
        void initBuffers();
        
        void clear(const gl::FboRef &fbo, ColorA color = ColorA(0.0,0.0,0.0,1.0));
        
        void Advect(const gpGpuFrameBufferRef &buffer, const gl::TextureRef &velocity, const gl::TextureRef &obstacles, float dissipation, float timeStep);
        
        void Buoyancy(const gpGpuFrameBufferRef &buffer, const gl::TextureRef &temperature, const gl::TextureRef &density, float AmbientTemperature, float TimeStep, float Kappa, float Sigma, vec3 gravity = vec3(0.0));
        
        void ComputeDivergence(const gl::FboRef &fbo, const gl::TextureRef &velocity, const gl::TextureRef &obstacles, float cellSize);
        
        void Jacobi(const gpGpuFrameBufferRef &buffer, const gl::TextureRef &divergence, const gl::TextureRef &obstacles, float cellsize, float inverseBeta);
        
        void subtractGradient(const gpGpuFrameBufferRef &buffer, const gl::TextureRef &pressure, const gl::TextureRef &obstacles, float gradientScale);
        
        void injectImpulse(const gl::FboRef &fbo, vec2 position, float radius, vec4 color);

        ivec2               mSize;
        
        gl::GlslProgRef     jacobiShader,
                            buoyancyShader,
                            divergenceShader,
                            advectShader,
                            subGradientShader,
                            impulseShader,
                            visualizeShader,
                            velVisualizerShader;
        
        
        gpGpuFrameBufferRef mVelocityBuffer,
                            mDensityBuffr,
                            mPressureBuffer,
                            mTemperatureBuffer;
        
        gl::FboRef          mDivergenceFbo,
                            mObstaclesFbo,
                            mHiResObstaclesFbo;
        
        gl::FboRef          mFbo;
        
        float               mDissipation,
                            mTimeStep,
                            mSigma,
                            mKappa,
                            mAmbientTemp,
                            mInversBeta,
                            mCellSize;
        
        int                 mNumIterations;
        
        float               mDeltaT;
        
        float               mPrevTime;
        
        vec3                mGravity;
        
        ConcurrentCircularBuffer<impulsePoint>	*mImpulsePoints;
        vector<impulsePoint>	mConstantImpulsePoints;
        
        params::InterfaceGlRef  mParamRef;
    };
}
#endif /* Fluido_h */
