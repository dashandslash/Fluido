#version 410

uniform sampler2D uTexture;

uniform float uMaxValue;

uniform float uClampForce;

in vec2 			vTexCoord;
out vec4 			FragColor;

void main(){
    vec2 st = vTexCoord;
    
    vec4 color = texture(uTexture, st);
    
    float l = length(color.xyz);
    if (l > uMaxValue) {
        float dinges = (l - uMaxValue) * uClampForce;
        color.xyz = normalize(color.xyz) * (l - dinges);
        
        
    }
    
    FragColor = color ;
}