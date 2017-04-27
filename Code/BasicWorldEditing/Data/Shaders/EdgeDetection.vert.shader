#version 430 core

layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexColor;

out vec3 vColor;

layout(location = 13) uniform mat4 modelToWorld;
layout(location = 16) uniform mat4 worldToView;
layout(location = 17) uniform mat4 projection;

void main()
{
	vColor = vertexColor;
	
	// calculate position of vertex
	gl_Position = projection * worldToView * modelToWorld * vec4(vertexPosition, 1.0f);
}
