#include "Editor.h"

Vector3f Editor::color_compute(int color_bit, Vector3f n, Vector3f l) {
	Vector3f color;
	if (color_bit == 0) {color = Vector3f(191,51,46);}
	if (color_bit == 1) {color = Vector3f(255,99,71);}
	if (color_bit == 2) {color = Vector3f(255,165,0);}
	if (color_bit == 3) {color = Vector3f(240,230,140);}
	if (color_bit == 4) {color = Vector3f(144,238,144);}
	if (color_bit == 5) {color = Vector3f(102,205,170);}
	if (color_bit == 6) {color = Vector3f(32,178,170);}
	if (color_bit == 7) {color = Vector3f(135,206,250);}
	if (color_bit == 8) {color = Vector3f(100,149,237);}
	if (color_bit == 9) {color = Vector3f(65,105,225);}
	if (color_bit == 10) {color = Vector3f(106,90,205);}
	if (color_bit == 11) {color = Vector3f(123,104,238);}
	if (color_bit == 12) {color = Vector3f(186,85,211);}
	if (color_bit == 13) {color = Vector3f(255,182,193);}
	if (color_bit == 14) {color = Vector3f(240,128,128);}

	Vector3f kd = color;
	Vector3f ka = kd/10.0;
	Vector3f ks (0.1,0.1,0.1);
	Vector3f I(1,1,1);
	Vector3f v = l;
	float p = 100;
	
	float factor = max(n.dot(l), (float)0.0);
	Vector3f diffuse = factor * (I.array() * kd.array()).matrix();
	Vector3f h = (v + l).normalized();
	factor = pow(max(n.dot(h), (float)0.0), p);
	Vector3f specular = factor * (I.array() * ks.array()).matrix();
	Vector3f ambient = ka;
	return ambient + diffuse + specular;
}

std::string Editor::color_to_hex(Vector3f color) {
	std::string result;
	for (int i=0; i < 3; i++) {
		int c_int = (int)color(i);
		std::stringstream sstream;
		sstream << std::hex << c_int;
		std::string hex_color = sstream.str();
		if (hex_color.length() == 1) {
			hex_color = "0" + hex_color;
		} else if (hex_color.length() > 2) {
			hex_color = "ff";
		} result += hex_color;
	}
	return result;
}

void Editor::screenshot(void) {
	char filename[100];
	sprintf(filename, "snap%d.svg", snap_num);

	char buff[1000];
	snprintf(buff, sizeof(buff), 
		"<svg xmlns='http://www.w3.org/2000/svg' version='1.1' width='%f' height='%f'>"
		"<g transform='matrix(1 0 0 -1 0 %f)'>"
		"<rect x='0' y='0' width='%f' height='%f' fill='white'/>\n",width, height, height, width, height);
	std::string input = buff;

	Matrix4f proj;
	if (camera_type == 0) { proj = persp;
	} else { proj = orth; }

	std::vector<std::vector<float>> order;
	for (int j = 0; j < objects.size(); j++) {
		int num_tri = obj_index[objects[j]];
		int t = 0;
		for (int k = 0; k < objects[j]; k++) {
			t += obj_index[k]; // count total triangles before head
		}
		for (int k = 0; k < num_tri; k++) {
			int i = t*3 + k*3;
			Vector4f v1(V.col(i)(0), V.col(i)(1), V.col(i)(2),1);
			Vector4f v2(V.col(i+1)(0), V.col(i+1)(1), V.col(i+1)(2),1);
			Vector4f v3(V.col(i+2)(0), V.col(i+2)(1), V.col(i+2)(2),1);

			v1 = model.block(0, j*4, 4, 4) * v1;
			v2 = model.block(0, j*4, 4, 4) * v2;
			v3 = model.block(0, j*4, 4, 4) * v3;
			Vector3f c ((v1(0)+v2(0)+v3(0))/3, (v1(1)+v2(1)+v3(1))/3, (v1(2)+v2(2)+v3(2))/3);

			float d = (cam_pos - c).norm();
			std::vector<float> tri_index;
			tri_index.push_back(d);
			tri_index.push_back((float)j);
			tri_index.push_back((float)i);
			order.push_back(tri_index);
		}
	}
	std::sort(order.begin(), order.end(),
		[](const std::vector<float>& a, const std::vector<float>& b) { return a[0] > b[0]; });

	for (int k = 0; k< order.size(); k++) {
		int i = (int)order[k][2];
		int j = (int)order[k][1];
		Vector4f v1(V.col(i)(0), V.col(i)(1), V.col(i)(2),1);
		Vector4f v2(V.col(i+1)(0), V.col(i+1)(1), V.col(i+1)(2),1);
		Vector4f v3(V.col(i+2)(0), V.col(i+2)(1), V.col(i+2)(2),1);
		v1 = model.block(0, j*4, 4, 4) * v1;
		v2 = model.block(0, j*4, 4, 4) * v2;
		v3 = model.block(0, j*4, 4, 4) * v3;
		v1 = view * proj * camera * v1; v1 = v1/v1(3);
		v2 = view * proj * camera * v2; v2 = v2/v2(3);
		v3 = view * proj * camera * v3; v3 = v3/v3(3);

		Vector2f v1_2f((v1(0) + 1) * width/2, (v1(1) + 1) * height/2);
		Vector2f v2_2f((v2(0) + 1) * width/2, (v2(1) + 1) * height/2);
		Vector2f v3_2f((v3(0) + 1) * width/2, (v3(1) + 1) * height/2);
		Vector4f face_normal (N(i/3,0), N(i/3,1), N(i/3,2), 0);
		face_normal = model.block(0, j*4, 4, 4) * face_normal;
		Vector3f fn_3f (face_normal(0),face_normal(1),face_normal(2));
		fn_3f = fn_3f.normalized();

		memset(buff, 0, sizeof(buff));
		snprintf(buff, sizeof(buff), 
			"<polygon points='%f,%f %f,%f %f,%f' style='fill:#%s; stroke:black; stroke-width:0.3' />\n", 
			v1_2f(0),v1_2f(1), v2_2f(0),v2_2f(1), v3_2f(0),v3_2f(1), color_to_hex(color_compute(color_bits[j], fn_3f, cam_pos/radius)).c_str());
		std::string tri_str = buff;
		input += tri_str;
	}
	input += "</g></svg>";
	std::ofstream file(filename);
	file << input;
	file.close();
	snap_num ++;
}

