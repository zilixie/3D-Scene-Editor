#version 150 core
#define PI 3.1415926538

in vec4 position;
in vec3 vertex_normal;
out vec3 f_color;

uniform mat4 model;					//the model matrix defined for the current vertex.
uniform mat4 anim_model;			//the model matrix defined for the current vertex.  (The object is in animation mode)
uniform mat4 control_point_model;	//is the model matrix defined for the control point of bezier curve
uniform mat4 view;					//the viewing matrix that change vertex from world coordinate system to camera coordinate system.
uniform mat4 camera;				//the matrix defined for camera transformation.
uniform mat4 proj;					//the matrix defined for projection (perspective or orthogonal).
uniform float time;					//a float value of the time since the start of the program
uniform bool draw_frame;			//the shader will draw frames when this variable is set true.
uniform bool draw_bezier_curve;		//the shader will draw bezier curves for animation when this variable is set true.
uniform int color_bit;				//the color_bit ranges from 0 to 14, different value represents different color.
uniform int normal_type;			//the type of normal the application is using for rendering color (vertex normal or face normal).
uniform int start_animation;		//whether or not the animation start button was pressed.
uniform vec3 face_normal;			//face normal.
uniform vec3 light_d;				//light direction.

vec3 blinn_phong(vec3 ka, vec3 kd, vec3 ks, float p, vec3 n, vec3 v, vec3 l) {
	vec3 I = vec3(1,1,1);
	float factor = max(n.x * l.x + n.y * l.y + n.z * l.z, 0);
	vec3 diffuse = factor * I * kd;

	vec3 h = normalize(v + l);
	factor = pow(max((n.x * h.x + n.y * h.y + n.z * h.z), 0), p);
	vec3 specular = factor * I * ks;

	vec3 ambient = ka;
	return ambient + diffuse + specular;
}

void main()
{
	float theta = PI * time;
	float c = cos(0.5 * theta);
	float s = sin(0.5 * theta);

	vec3 color = vec3(0.0,0.0,0.0);
	if (!draw_frame) {
		if (color_bit == 0) {color = vec3(191,51,46)/255.0;}
		if (color_bit == 1) {color = vec3(255,99,71)/255.0;}
		if (color_bit == 2) {color = vec3(255,165,0)/255.0;}
		if (color_bit == 3) {color = vec3(240,230,140)/255.0;}
		if (color_bit == 4) {color = vec3(144,238,144)/255.0;}
		if (color_bit == 5) {color = vec3(102,205,170)/255.0;}
		if (color_bit == 6) {color = vec3(32,178,170)/255.0;}
		if (color_bit == 7) {color = vec3(135,206,250)/255.0;}
		if (color_bit == 8) {color = vec3(100,149,237)/255.0;}
		if (color_bit == 9) {color = vec3(65,105,225)/255.0;}
		if (color_bit == 10) {color = vec3(106,90,205)/255.0;}
		if (color_bit == 11) {color = vec3(123,104,238)/255.0;}
		if (color_bit == 12) {color = vec3(186,85,211)/255.0;}
		if (color_bit == 13) {color = vec3(255,182,193)/255.0;}
		if (color_bit == 14) {color = vec3(240,128,128)/255.0;}
	}

	if (draw_bezier_curve) {
		gl_Position = view * proj * camera * control_point_model * vec4(position[0], position[1], position[2], 1.0);
	}
	else if (start_animation == 1 && !draw_bezier_curve) {
		gl_Position = view * proj * camera * anim_model * vec4(position[0], position[1], position[2], 1.0);
	}
	else {
		gl_Position = view * proj * camera * model * vec4(position[0], position[1], position[2], 1.0);
	}

	vec3 kd = color;
	vec3 ka = kd/10.0;
	vec3 ks = vec3(0.1,0.1,0.1);
	float p = 100;

	if (normal_type == 2) {
		f_color = blinn_phong(ka, kd, ks, p, normalize(face_normal), light_d, light_d);
	} else if (normal_type == 3) {
		vec4 vertex_normal_4d = model * vec4(vertex_normal[0], vertex_normal[1], vertex_normal[2], 0.0);
		vec3 vertex_normal_3d = vec3(vertex_normal_4d[0], vertex_normal_4d[1], vertex_normal_4d[2]);
		f_color = blinn_phong(ka, kd, ks, p, normalize(vertex_normal_3d), light_d, light_d);
	} else {
		f_color = color;
	}


}