#version 430 core

in vec3 vColor;
out vec4 fColor;

layout(location = 34) uniform float screenToTexWidth;
layout(location = 35) uniform float screenToTexHeight;
layout(location = 33) uniform sampler2D uDiffuseAndSpecularTexture;
layout(location = 18) uniform int numLevels;
layout(location = 40) uniform vec3 edgeColor;
layout(location = 42) uniform vec3 noEdgeColor;
layout(location = 41) uniform vec2 offset;
layout(location = 10) uniform vec3 tintColor;
layout(location = 11) uniform float tintIntensity;

subroutine void RenderPassType();
subroutine uniform RenderPassType renderPass;

layout(location = 36) uniform int radius;

float intensity(in vec4 color)
{
	return sqrt((color.x*color.x) + (color.y*color.y) + (color.z*color.z));
}

vec4 getColorAt(vec2 xy)
{
	ivec2 pixelPos = ivec2(xy.x, xy.y);
	vec4 diffSpec = texelFetch(uDiffuseAndSpecularTexture, pixelPos, 0);
	return diffSpec; 
}

vec3 simple_edge_detection(in float step, in vec2 center)
{
	float center_intensity = intensity(getColorAt(center));
	int darker_count = 0;
	float max_intensity = center_intensity;
	for (int i = -radius; i <= radius; i++)
	{
		for (int j = -radius; j <= radius; j++)
		{
			vec2 current_location = center + vec2(i*step, j*step);
			float current_intensity = intensity(getColorAt(current_location));
			if (current_intensity < center_intensity) { darker_count++; } 
			if (current_intensity > max_intensity)
			{
				max_intensity = current_intensity;
			}
		}
	}

	if ((max_intensity - center_intensity) < 0.01*radius)
	{
		if (darker_count / (radius*radius) < (1 - (1 / radius)))
		{
			return getColorAt(center);
		}
	}
	return edgeColor;
}

subroutine(RenderPassType)
void DoEdges()
{
	float step = 1.0;
	fColor.xyz = simple_edge_detection(step, vec2((gl_FragCoord.x + offset.x) * screenToTexWidth, (gl_FragCoord.y + offset.y) * screenToTexHeight));
	fColor.a = 1.0;
}

subroutine(RenderPassType)
void DoObjectOnly()
{
	fColor.xyz = getColorAt(vec2((gl_FragCoord.x + offset.x) * screenToTexWidth, (gl_FragCoord.y + offset.y) * screenToTexHeight)).rgb;
	fColor.a = 1.0;
}

vec3 PC()
{
	vec3 color = vColor;
	color = (1.0f - tintIntensity) * color + tintIntensity * vec4(tintColor, 1.0f);
	return color;
}

subroutine(RenderPassType)
void Shade()
{
	vec3 positionEye;
	vec3 normalNormEye;

	vec3 light = PC();
	fColor = vec4(light, 1.0f);
}

void main()
{
	renderPass();
}