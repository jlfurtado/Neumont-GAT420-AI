#version 430 core

in vec2 UV;
in vec3 fPos;
out vec4 fColor;

layout(location = 18) uniform sampler2D textureSampler;
layout(location = 10) uniform vec3 tintColor;
layout(location = 11) uniform float tintIntensity;

void main()
{
	fColor = texture(textureSampler, UV);
	fColor = vec4((1.0f - tintIntensity) * vec3(fColor) + tintIntensity * tintColor, 1.0f);

}