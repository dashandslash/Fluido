#version 150


#define M_PI 3.1415926535897932384626433832795

uniform sampler2D Sampler;
//uniform vec3 uColor;
uniform vec2 Scale;

out vec4 FragColor;

in vec2 vTexCoord;

float atan2(in float y, in float x)
{
    return x == 0.0 ? sign(y)*M_PI/2 : atan(y, x);
}

float map(float value, float oldMin, float oldMax, float newMin, float newMax){
    return newMin + (value-oldMin)*(newMax-newMin)/(oldMax-oldMin);
}

vec3 hsv2rgb(vec3 c) {
    vec4 K = vec4(1.0, 2.0 / 3.0, 1.0 / 3.0, 3.0);
    vec3 p = abs(fract(c.xxx + K.xyz) * 6.0 - K.www);
    return c.z * mix(K.xxx, clamp(p - K.xxx, 0.0, 1.0), c.y);
}

void main()
{
    vec4 L = texture(Sampler, vec2(vTexCoord.x, 1.0-vTexCoord.y));
    float angle = atan2(L.x, L.y);
    
    if(angle < 0.0f)
    {
        angle+=2.0f*M_PI;
    }
    
    angle = angle*180.0f/M_PI;
    vec3 color = hsv2rgb( vec3( (angle-90.0)/360.0,1.0,length(L) ) );
    
    FragColor = vec4(color*length(L.xy),1.0);
}