#version 150


uniform sampler2D Sampler;
uniform vec3 uColor;
uniform vec2 Scale;

out vec4 FragColor;

in vec2 vTexCoord;

void main()
{
    vec4 L = texture(Sampler, vec2(vTexCoord.x, 1.0-vTexCoord.y));
    FragColor = vec4(L.rgb,1.0);
}