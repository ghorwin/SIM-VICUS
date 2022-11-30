#version 330

flat in vec3 startPos;
in vec3 vertPos;
uniform vec4 fixedColor;         // parameter: grid color as rgba-value

out vec4 finalColor;

uniform vec2  resolution;
uniform float dashSize;
uniform float gapSize;

void main()
{
	vec2  dir  = (vertPos.xy-startPos.xy) * resolution/2.0;
	float dist = length(dir);

	if (fract(dist / (dashSize + gapSize)) > dashSize/(dashSize + gapSize))
		discard;

	finalColor = fixedColor;
}
