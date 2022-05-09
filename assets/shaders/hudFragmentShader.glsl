#version 300 es
precision mediump float;
in vec2 vTexCoord;
in vec4 vColor;
out vec4 FragColor;
uniform sampler2D tex;
void main(){
	FragColor = texture(tex, vTexCoord) * vColor;
}