#version 300 es
precision lowp float;

in vec2 vTexCoord;
in vec2 vNormalTexCoord;
in vec4 vColor;
in vec2 vFragPos;
out vec4 FragColor;

uniform sampler2D tex;
uniform vec3 lightColor;
uniform vec2 lightSourcePos;
uniform float ambientStrength;
uniform bool enableLight;

void main(){
    vec4 rawColor = texture(tex, vTexCoord) * vColor;
    if(enableLight){
        vec3 normal;
        //object does not have a normal map
        if(vNormalTexCoord.xy == vec2(0,0)){
            normal = vec3(0,0,1);
        }
        else{
            //map [0,1] color to [-1,1] vector
            normal = normalize(texture(tex, vNormalTexCoord).xyz * 2.0f - 1.0f);
        }
        vec3 lightDir = normalize(vec3(lightSourcePos - vFragPos, 1.0f));
        float diffuseFactor = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diffuseFactor * lightColor;
  
        vec3 ambient = lightColor * ambientStrength;
        vec3 pointLight = lightColor * 1.0f/sqrt(distance(vFragPos, lightSourcePos));
        vec3 light = diffuse;

        FragColor = rawColor * vec4(light,1.0f);
    }
    else{
        FragColor = rawColor;
    }
}
