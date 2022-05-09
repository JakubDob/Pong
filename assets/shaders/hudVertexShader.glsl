#version 300 es
precision mediump float;
layout(location = 0) in vec2 aPos;
layout(location = 1) in vec2 aTexCoord;
layout(location = 2) in vec2 aNormalTexCoord;
layout(location = 3) in vec4 aColor;
layout(location = 4) in float aDepth;

layout (std140) uniform Transforms {
	mat4 projection; //hud,text
	mat4 projectionView; //rest
};

out vec2 vTexCoord;
out vec4 vColor;
void main(){
	gl_Position = projection * vec4(aPos, 0, 1);
	vColor = aColor;
	vTexCoord = aTexCoord;
}