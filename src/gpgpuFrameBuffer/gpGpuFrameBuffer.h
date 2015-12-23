//
//  gpGpuFrameBuffer.h
//  dsFluid
//
//  Created by luca lolli on 16/12/2015.
//
//

#ifndef gpGpuFrameBuffer_h
#define gpGpuFrameBuffer_h


#include "cinder/cinder.h"
#include "cinder/gl/gl.h"

using namespace ci;
using namespace ci::app;
using namespace std;

typedef shared_ptr<class gpGpuFrameBuffer> gpGpuFrameBufferRef;

class gpGpuFrameBuffer{
    
public:
    static gpGpuFrameBufferRef create(int width, int height, GLint colorFormat = GL_RGBA32F)
    {
        return gpGpuFrameBufferRef( new gpGpuFrameBuffer{width, height, colorFormat});
    }
    
    static gpGpuFrameBufferRef create(ivec2 size, GLint colorFormat = GL_RGBA32F)
    {
        return gpGpuFrameBufferRef( new gpGpuFrameBuffer{size.x, size.y, colorFormat});
    }

    gpGpuFrameBuffer(int width, int height, GLint colorFormat = GL_RGBA32F);
    
    gpGpuFrameBuffer(ivec2 size, GLint colorFormat = GL_RGBA32F );
    ~gpGpuFrameBuffer();
    
    void initFbo();
    
    void clear();
    
    gl::FboRef getBuffer();
    
    GLint getBufferLocation();
    
    GLint getTextureLocation();
    
    void drawBuffer();
    
    void bindTexture(int textureUnit = 0);
    
    void unbindTexture();
    
    void bindBuffer();
    
    void unbindBuffer(bool toSwap = false);
    
    gl::TextureRef getTexture();
    
    void swap();
    
    void draw();
    
    ivec2 getSize();
    
private:
    GLint                   mColorFormat;
    int                     mWidth, mHeight;
    
    gl::Fbo::Format         mFboFormat;
    gl::Texture::Format     mTextureFormat;
    
    GLenum                  mReadIndex, mWriteIndex;
    
    gl::FboRef              mFbo;
    
    
};

#endif /* gpGpuFrameBuffer_h */
