#version 330

flat in vec3 startPos;
in vec3 vertPos;
uniform vec4 fixedColor;         // parameter: grid color as rgba-value
uniform vec2  resolution;        // parameter: resolution of viewport (width, height)
uniform float dashSize;          // parameter: dash line size
uniform float gapSize;           // parameter: gap size

out vec4 finalColor;


void main()
{
	vec2  dir  = (vertPos.xy-startPos.xy) * resolution/2.0;
	float dist = length(dir);

	if (fract(dist / (dashSize + gapSize)) > dashSize/(dashSize + gapSize))
		discard;

	finalColor = fixedColor;
}
