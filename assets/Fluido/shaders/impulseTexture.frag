#version 150

uniform sampler2D uTexture;
uniform sampler2D uImpulseTex;

in vec2 vTexCoord;

out vec4 FragColor;

void main()
{

    vec4 baseTex = texture(uTexture, vTexCoord);
    
    FragColor = mix(baseTex, texture(uImpulseTex, vTexCoord), length(texture(uImpulseTex, vTexCoord).rg));
    
//    FragColor.rgb = max(FragColor.rgb,vec3(-1.0));
    
    //FragColor.a = 1.0;

    
//    float l = length(FragColor.xyz);
//    if (l > 0.58150) {
//        float dinges = (l - 0.58150) * 1.0;
//        FragColor.xyz = normalize(FragColor.xyz) * (l - dinges);
//    }
//    FragColor = color ;


}

