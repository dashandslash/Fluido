#version 150

uniform sampler2D uVelocityTex;
uniform sampler2D uSourceTex;
uniform sampler2D uObstacles;

uniform float uTimeStep;
uniform float uDissipation;

uniform ivec2 ciWindowSize;

in vec2 vTexCoord;
out vec3 FragColor;

void main()
{
    //we need normalized coord
    vec2 st = vTexCoord;

    if (texture(uObstacles, st).r>0.0) {
        return;
    }
    
    vec2 u = texture(uVelocityTex, st).rg;
    vec2 coord = (st - uTimeStep * u);
    FragColor = uDissipation * texture(uSourceTex, coord).rgb ;
}
