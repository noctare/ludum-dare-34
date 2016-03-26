#version 130

uniform mat4 uni_Model;
uniform mat4 uni_View;
uniform mat4 uni_Projection;
uniform vec4 uni_Color;

in vec4 in_Position;
in vec4 in_Color;
in vec3 in_Tangent;
in vec3 in_Normal;
in vec2 in_TexCoords;

out vec4 ex_Color;
out vec2 ex_TexCoords;

void main() {
	gl_Position = uni_Projection * uni_View * uni_Model * in_Position;
	ex_Color = in_Color * uni_Color;
	ex_TexCoords = in_TexCoords;
}