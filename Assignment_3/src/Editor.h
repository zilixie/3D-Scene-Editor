#ifndef EDITOR_H
#define EDITOR_H

#include "Helpers.h"

#ifdef __APPLE__
#define GL_SILENCE_DEPRECATION
#include <GLFW/glfw3.h> // GLFW is necessary to handle the OpenGL context
#else
#include <GLFW/glfw3.h> // GLFW is necessary to handle the OpenGL context
#endif

#include <Eigen/Core>
#include <Eigen/Dense>
#include <chrono>
#include <fstream>
#include <string>
#include <iostream>
#include <algorithm>
#include <initializer_list>
#include <math.h>
#include <cmath>
#include "read_off.h"
#include "normal.h"

#define TRANSLATION_MODE 1
#define DELETE_MODE 2
#define ANIMATION_MODE 3
#define QUIT_MODE 7

using namespace std;
using namespace Eigen;

class Editor {
	public:
		int mode;										// Current mode
		int vertex_count;								// Numbers of vertices in V
		int normal_type;								// type of normal for shading
		bool obj_clicked;								// Whether there is aobject being clicked
		int ith_obj;									// which object is selected (for translate and animation)
		float width, height;							// width and height of screen
		int camera_type;								// perspective or orthogonal
		int bezier_step;								// from 0 to 4
		int closest_control_point;						// cloest control point from 0 to 3 (use for edit bezier curve)
		std::vector<int> obj_index; 					// num of triangles for each type of obj

		// each object has the following properties
		std::vector<int> objects;						// vector of index to obj_index
		std::vector<int> color_bits;					// vector of color_bit
		std::vector<Eigen::MatrixXf> control_points;	// control point matrix (use for animation)
		Eigen::MatrixXf V, N, VN;						// vertices, face normals, vertex normals

		Eigen::MatrixXf model, anim_model;				// model matrix
		Eigen::MatrixXf translation, rotation, scaling;	// translation matrix, rotation matrix, scaling matrix

		// for view control
		int pause;
		float alpha;
		float alpha_shift, beta_shift;
		Eigen::Matrix4f view;							// view transformation matrix
		Eigen::Matrix4f camera;							// camera transformation matrix
		Eigen::Matrix4f persp, orth;					// projection
		float radius;									// distance from camera to origin
		Vector3f cam_pos;								// current camera position
		Vector2f p0, p1; 								// previous and current cursor position

		// for animation
		int start_animation;
		int snap_num;

		void init(void);
		void switch_mode(int m);
		void insert_mesh(std::string filename, float scale);
		void add_obj(int obj_id);
		bool click_on_obj(Eigen::Vector2f world_coord_2d);
		void find_closest_control_point(void);
		void camera_move(int dir, int clock);

		void rotate_by(double degree, int clockwise, int direction);
		void scale_by(double percentage, int up);
		void delete_at(int del_ith_obj);
		float bezier_curve(float A, float B, float C, float D, float t);
		void change_color(int up);
		void toggle_render_mode(int right);
		void screenshot(void);
		std::string color_to_hex(Vector3f color);
		Eigen::Vector3f color_compute(int color_bit, Vector3f n, Vector3f l);
		Eigen::Vector3f pixel_to_world_coord(Eigen::Vector2f pixel, int width, int height);
};

inline void Editor::switch_mode(int m){
	if ((bezier_step != 4 && bezier_step != 0) && mode == ANIMATION_MODE) {
		control_points[ith_obj] = MatrixXf::Zero(3, 4);
		bezier_step = 4;
	}
	obj_clicked = false;
	ith_obj = -1;
	closest_control_point = -1;

	mode = m;
	V.conservativeResize(3, vertex_count);
	start_animation = 0;

	if (m == TRANSLATION_MODE) { std::cout << "Translation mode is on." << std::endl; }
	else if (m == DELETE_MODE) { std::cout << "Deletion mode is on." << std::endl; }
	else if (m == ANIMATION_MODE) { std::cout << "Animation mode is on." << std::endl; }
	else if (m == QUIT_MODE) { std::cout << "Quit Application." << std::endl; }
}

inline void Editor::init(void) {
	mode = TRANSLATION_MODE;
	normal_type = 1;
	pause = 0;
	start_animation = 0;
	snap_num = 0;
	radius = 9.0;
	cam_pos = Vector3f(9.0,0,0);

	obj_clicked = false;
	ith_obj = -1;
	vertex_count = 0;
	camera_type = 0;
	alpha_shift = 0;
	beta_shift = 0;
	bezier_step = 0;
	closest_control_point = -1;

	view = MatrixXf::Identity(4, 4);
	model = MatrixXf::Identity(4, 4);
	anim_model = MatrixXf::Identity(4, 4);
	translation = MatrixXf::Identity(4, 4);
	rotation = MatrixXf::Identity(4, 4);
	scaling = MatrixXf::Identity(4, 4);

	p0 = Vector2f(0,0);
	p1 = Vector2f(0,0);
	V.resize(3,36);

	insert_mesh("../data/cube.off", 1);
	insert_mesh("../data/bunny.off", 10);
	insert_mesh("../data/bumpy_cube.off", 0.2);
	insert_mesh("../data/sphere.off", 1);
	add_obj(0);
}

inline void Editor::add_obj(int obj_id) {
	if (mode != TRANSLATION_MODE) {
		switch_mode(TRANSLATION_MODE);
	}
	objects.push_back(obj_id);
	color_bits.push_back(0);
	control_points.push_back(MatrixXf::Zero(3, 4));

	int obj_count = (int)objects.size();
	model.conservativeResize(4, obj_count * 4);
	anim_model.conservativeResize(4, obj_count * 4);
	translation.conservativeResize(4, obj_count * 4);
	rotation.conservativeResize(4, obj_count * 4);
	scaling.conservativeResize(4, obj_count * 4);

	model.topRightCorner(4,4).setIdentity();
	anim_model.topRightCorner(4,4).setIdentity();
	translation.topRightCorner(4,4).setIdentity();
	rotation.topRightCorner(4,4).setIdentity();
	scaling.topRightCorner(4,4).setIdentity();
}

inline void Editor::insert_mesh(std::string filename, float scale) {
	MatrixXf mesh_V;
	MatrixXi mesh_F;
	read_off(filename,mesh_V,mesh_F);
	
	V.conservativeResize(3, vertex_count + mesh_F.rows() * 3);
	for (int i=0; i<mesh_F.rows(); i++) {
		for (int j=0; j<3; j++) {
			int r = mesh_F(i,j);
			V.col(vertex_count + i * 3 + j) << scale*mesh_V(r,0), scale*mesh_V(r,1), scale*mesh_V(r,2);
		}
	}
	obj_index.push_back(mesh_F.rows());

	int obj_count = (int)obj_index.size();
	int i = 0;
	for (int j = 0; j < obj_count-1; j++) {
		i += obj_index[j]; // count total triangles
	}
	float total_area = 0.0;
	Eigen::Vector3f mesh_bc (0.0,0.0,0.0);
	for (int k = 0; k < obj_index[obj_count-1]; k++) {
		Eigen::Vector3f v1(V(0,i*3+k*3), V(1,i*3+k*3), V(2,i*3+k*3));
		Eigen::Vector3f v2(V(0,i*3+k*3+1), V(1,i*3+k*3+1), V(2,i*3+k*3+1));
		Eigen::Vector3f v3(V(0,i*3+k*3+2), V(1,i*3+k*3+2), V(2,i*3+k*3+2));

		Vector3f bc ((v1(0)+v2(0)+v3(0))/3, (v1(1)+v2(1)+v3(1))/3, (v1(2)+v2(2)+v3(2))/3);
		float area = 0.5 * ((v2-v1).cross(v3-v1)).norm();
		mesh_bc += (area * bc);
		total_area += area;
	}
	mesh_bc = mesh_bc/total_area;
	for (int i = vertex_count; i < vertex_count + mesh_F.rows() * 3; i++) {
		V.col(i) << V(0,i) - mesh_bc(0), V(1,i) - mesh_bc(1), V(2,i) - mesh_bc(2); 
	}

	Eigen::MatrixXf new_fn;
	per_face_normals(mesh_V, mesh_F, new_fn);

	N.conservativeResize(vertex_count/3 + mesh_F.rows(), 3);
	for (int i=vertex_count/3; i < vertex_count/3 + mesh_F.rows(); i++){
		N.row(i) = new_fn.row(i - vertex_count/3);
	}

	Eigen::MatrixXf mesh_vn;
	per_vertex_normals(mesh_V, mesh_F, mesh_vn);

	Eigen::MatrixXf new_vn (mesh_F.rows()*3, 3);
	for (int i=0; i<mesh_F.rows(); i++) {
		for (int j=0; j<3; j++) {
			int r = mesh_F(i,j);
			new_vn.row(i * 3 + j) = mesh_vn.row(r);
		}
	}

	VN.conservativeResize(vertex_count + mesh_F.rows()*3, 3);
	for (int i=vertex_count; i < vertex_count + mesh_F.rows()*3; i++){
		VN.row(i) = new_vn.row(i - vertex_count);
	}
	vertex_count += (mesh_F.rows() * 3);
}

inline bool Editor::click_on_obj(Eigen::Vector2f cursor_pos) {
	for (int i = 0; i < objects.size(); i ++) {
		int num_tri = obj_index[objects[i]];

		int start = 0;
		for (int j = 0; j < objects[i]; j++) { start += obj_index[j] * 3; }
		for (int j = start; j < start + num_tri * 3; j += 3) {
			Eigen::Vector4f v1(V(0,j), V(1,j), V(2,j), 1);
			Eigen::Vector4f v2(V(0,j+1), V(1,j+1), V(2,j+1), 1);
			Eigen::Vector4f v3(V(0,j+2), V(1,j+2), V(2,j+2), 1);
			if (camera_type == 0) {
				v1 = view * persp * camera * model.block(0, i * 4, 4, 4) * v1;
				v2 = view * persp * camera * model.block(0, i * 4, 4, 4) * v2;
				v3 = view * persp * camera * model.block(0, i * 4, 4, 4) * v3;
			} else {
				v1 = view * orth * camera * model.block(0, i * 4, 4, 4) * v1;
				v2 = view * orth * camera * model.block(0, i * 4, 4, 4) * v2;
				v3 = view * orth * camera * model.block(0, i * 4, 4, 4) * v3;
			}
			Vector2f screen_pos_v1 ((v1(0)/v1(3)+1)* width/2.0, (v1(1)/v1(3)+1)* height/2.0);
			Vector2f screen_pos_v2 ((v2(0)/v2(3)+1)* width/2.0, (v2(1)/v2(3)+1)* height/2.0);
			Vector2f screen_pos_v3 ((v3(0)/v3(3)+1)* width/2.0, (v3(1)/v3(3)+1)* height/2.0);

			MatrixXf m(3,2);
			m << (RowVector2f)(screen_pos_v1), (RowVector2f)(screen_pos_v2), (RowVector2f)(screen_pos_v3);
			m.transposeInPlace();

			Matrix3f A;
			Vector3f b;
			A << m, 1, 1, 1;
			b << cursor_pos(0), cursor_pos(1), 1;
			Vector3f x = A.colPivHouseholderQr().solve(b);
			if (x(0) > 0 && x(1) > 0 && x(2) > 0) {
				ith_obj = i;
				obj_clicked = true;
				return true;
			} else { obj_clicked = false; }
		}
	}
	return false;
}

inline Eigen::Vector3f Editor::pixel_to_world_coord(Eigen::Vector2f pixel, int width, int height) {
	Eigen::Vector4f canonical_coord((pixel(0)/width)*2-1, (pixel(1)/height)*2-1, 0, 1);
	Eigen::Vector4f shift_h;
	if (camera_type == 0) { shift_h = camera.inverse() * persp.inverse() * view.inverse() * canonical_coord; } 
	else { shift_h = camera.inverse() * orth.inverse() * view.inverse() * canonical_coord; }
	Eigen::Vector3f shift (shift_h(0)/shift_h(3), shift_h(1)/shift_h(3), shift_h(2)/shift_h(3));
	return shift;
}

inline void Editor::camera_move(int dir, int clock) {
	Vector3f shift(0,0,0);
	Vector3f new_cam_pos(0,0,0);
	if (dir == 1) { shift = Vector3f(0.3, 0, 0); }
	if (dir == 2) { shift = Vector3f(0, 0.3, 0); }
	if (dir == 3) { shift = Vector3f(0, 0, 0.3); }
	if (clock == 0) {
		new_cam_pos = cam_pos + shift;
	} else { new_cam_pos = cam_pos - shift; }

	radius = new_cam_pos.norm();
	float new_beta = asin(new_cam_pos(1)/radius);
	if (!isnan(new_beta)) {beta_shift = new_beta;}
	if (dir != 2) {
		float new_alpha = acos(new_cam_pos(0)/(radius * cos(new_beta)));
		if (new_cam_pos(2) < 0) { new_alpha = -new_alpha; }
		if (!isnan(new_alpha)) {alpha_shift = new_alpha;}
	}
}

inline void Editor::delete_at(int del_ith_obj) {
	objects.erase(objects.begin() + del_ith_obj);
	color_bits.erase(color_bits.begin() + del_ith_obj);
	control_points.erase(control_points.begin() + del_ith_obj);

	int obj_count = (int)objects.size();
	for (int j = del_ith_obj * 4; j < obj_count * 4; j++) {
		model.col(j) = model.col(j + 4);
		translation.col(j) = translation.col(j + 4);
		rotation.col(j) = rotation.col(j + 4);
		scaling.col(j) = scaling.col(j + 4);
		anim_model.col(j) = anim_model.col(j + 4);
	}
	model.conservativeResize(4, obj_count * 4);
	translation.conservativeResize(4, obj_count * 4);
	rotation.conservativeResize(4, obj_count * 4);
	scaling.conservativeResize(4, obj_count * 4);
	anim_model.conservativeResize(4, obj_count * 4);
}

inline void Editor::rotate_by(double degree, int clockwise, int direction) {
	if (mode == TRANSLATION_MODE && ith_obj != -1) {
		double theta;
		if (clockwise == 0) {theta = (-1) * degree * (M_PI / 180);} 
		else {theta = degree * (M_PI / 180);}

		Matrix4f r = MatrixXf::Identity(4, 4);
		if (direction == 0) { //z
			r.topLeftCorner(2,2) << cos(theta), -sin(theta), 
									sin(theta), cos(theta);
		} else if (direction == 1) { //x
			r.topLeftCorner(3,3) << 1,0,0,
									0, cos(theta), -sin(theta),
									0, sin(theta), cos(theta);
		} else if (direction == 2) { //y
			r.topLeftCorner(3,3) << cos(theta), 0,sin(theta),
									0, 1, 0,
									-sin(theta), 0, cos(theta);
		}
		rotation.block(0, ith_obj * 4, 4, 4) = r  * rotation.block(0, ith_obj * 4, 4, 4);
	}
}

inline void Editor::scale_by(double percentage, int up) {
	if (mode == TRANSLATION_MODE && ith_obj != -1) {
		Matrix4f s = MatrixXf::Identity(4, 4);
		if (up) { s.topLeftCorner(3,3) << (1 - percentage), 0, 0, 
											0, (1 - percentage), 0,
											0, 0, (1 - percentage);
		} else { s.topLeftCorner(3,3) << (1 + percentage), 0, 0, 
											0, (1 + percentage), 0,
											0, 0, (1 + percentage);
		}
		scaling.block(0, ith_obj * 4, 4, 4) = s * scaling.block(0, ith_obj * 4, 4, 4);
	}
}

inline void Editor::find_closest_control_point(void) {
	closest_control_point = -1;
	double dist = 100.0;
	for (int i=0; i<4; i++) {
		Eigen::Vector4f v(V(0,vertex_count+i), V(1,vertex_count+i), V(2,vertex_count+i), 1);
		if (camera_type == 0) { v = view * persp * camera * v; } 
		else { v = view * orth * camera * v; }
		Vector2f screen_pos ((v(0)/v(3)+1)* width/2.0, (v(1)/v(3)+1)* height/2.0);
		double d = (p1 - screen_pos).norm();
		if (d < dist) {
			dist = d;
			closest_control_point = i;
		}
	}
}

inline float Editor::bezier_curve(float A, float B, float C, float D, float t) {
	float AB = A*(1-t) + B*t;
	float BC = B*(1-t) + C*t;
	float CD = C*(1-t) + D*t;
	float ABC = AB*(1-t) + BC*t;
	float BCD = BC*(1-t) + CD*t;
	return ABC*(1-t) + BCD*t;
}

inline void Editor::change_color(int up) {
	if (up) { color_bits[ith_obj] ++; } 
	else { color_bits[ith_obj] --; }
	if (color_bits[ith_obj] == 15) { color_bits[ith_obj] = 0; }
	if (color_bits[ith_obj] == -1) { color_bits[ith_obj] = 14; }
}

inline void Editor::toggle_render_mode(int right) {
	if (right) { normal_type ++; } 
	else { normal_type --; }
	if (normal_type == 4) { normal_type = 0; }
	if (normal_type == -1) { normal_type = 3; }
}

#endif

