#version 150

uniform sampler2D uTex;
uniform vec3 uColor;

uniform ivec2 ciWindowSize;

out vec4 FragColor;

in vec2 vTexCoord;

void main()
{
    vec2 st = vTexCoord;
    
    vec4 L = texture(uTex, vec2(st.x, 1.0-st.y));
    FragColor = vec4(L.rgb,L.a);
}