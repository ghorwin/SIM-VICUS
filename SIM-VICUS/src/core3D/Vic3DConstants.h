#ifndef VIC3DConstantsH
#define VIC3DConstantsH

namespace Vic3D {

/*! Definition of shader program IDs. */
enum Shaders {
	/*! grid.vert:
		in vec2 position  (x,y)
		uniform mat4 worldToView

		grid.frag:
		uniform vec3 gridColor;                // parameter: grid color as rgb triple
		uniform vec3 backColor;                // parameter: background color as rgb triple
	*/
	SHADER_GRID,
	/*! VertexNormalColor.vert:
		layout(location = 0) in vec3 position; // input:  attribute with index '0' with 3 elements per vertex (coordinates)
		layout(location = 1) in vec3 normal;   // input:  attribute with index '1' with 3 elements per vertex (normal)
		layout(location = 2) in vec4 color;    // input:  attribute with index '2' with 4 elements (=rgba) per vertex
		uniform mat4 worldToView;              // parameter: the world-to-view matrix

		phong_lighting.frag:
		uniform vec3 lightPos;     // parameter: light position as vec3 (world coords)
		uniform vec3 lightColor;   // parameter: light color as rgb
		uniform vec3 viewPos;      // parameter: view position as vec3 (world coords)
	*/
	SHADER_OPAQUE_GEOMETRY,
	/*! lines.vert:
		layout(location = 0) in vec3 position; // input:  attribute with index '0' with 3 floats (x, y, z coords) per vertex

		uniform mat4 worldToView;              // parameter: world to view transformation matrix
		uniform mat4 modelToWorld;             // parameter: model to world transformation matrix

		lines.frag:
	*/
	SHADER_LINES,
	SHADER_COORDINATE_SYSTEM,
	SHADER_TRANSPARENT_GEOMETRY,
	NUM_SHADER_PROGRAMS
};

} // namespace Vic3D

#endif // VIC3DConstantsH
