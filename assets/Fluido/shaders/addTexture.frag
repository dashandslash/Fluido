#version 410

uniform sampler2D 	uBackbuffer;
uniform sampler2D 	uAddTexture;
uniform float 		uForce;

in vec2				vTexCoord;
out vec4			FragColor;

void main(){
    vec2 st = vTexCoord;
    
    vec4 color = texture(uBackbuffer, st) + texture(uAddTexture, st) * uForce;
    FragColor.xy = color.xy;
}
