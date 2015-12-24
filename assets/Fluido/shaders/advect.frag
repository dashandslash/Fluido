#version 150


uniform sampler2D uVelocityTex;
uniform sampler2D uSourceTex;
uniform sampler2D uObstacles;

uniform float uTimeStep;
uniform float uDissipation;

in vec2 vTexCoord;
out vec3 FragColor;

void main()
{
    //we need normalized coord
    ivec2 st = ivec2(gl_FragCoord.xy);

    if (texelFetch(uObstacles, st,0).r>0.0) {
        return;
    }
    
    vec2 u = texelFetch(uVelocityTex, st,0).rg;
    vec2 coord = (vTexCoord - uTimeStep * u);
    FragColor = uDissipation * texture(uSourceTex, coord).rgb ;
}
