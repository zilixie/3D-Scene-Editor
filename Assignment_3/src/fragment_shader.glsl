#version 150 core

out vec4 outColor;
in vec3 f_color;
uniform bool click;

void main()
{
	vec3 blue = vec3(0.05, 0.49, 0.82);
	vec3 red = vec3(0.75,0.2,0.18);
	
	if (click) {
		outColor = vec4(blue, 1.0);
	}
	else {
		outColor = vec4(f_color, 1.0);
	}
}
