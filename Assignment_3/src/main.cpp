// This example is heavily based on the tutorial at https://open.gl
// OpenGL Helpers to reduce the clutter
#include "Helpers.h"
using Clock = std::chrono::high_resolution_clock;
using TimePoint = std::chrono::time_point<Clock>;

#include "Editor.h"
// Global Variables
VertexBufferObject VBO; // VertexBufferObject wrapper
VertexBufferObject VBO_n; // VertexBufferObject wrapper
Editor e;

// Callback Functions
void framebuffer_size_callback(GLFWwindow* window, int width, int height) { glViewport(0, 0, width, height); }
void mouse_scroll_callback(GLFWwindow* window, double xoffset, double yoffset) { e.radius -= 0.2 * yoffset; }

void mouse_move_callback(GLFWwindow* window, double xpos, double ypos) {
	// Get the size of the window.
	int width, height;
	glfwGetCursorPos(window, &xpos, &ypos);
	glfwGetWindowSize(window, &width, &height);
	Eigen::Vector2f pixel(xpos, height-1-ypos); // pixel position
	// Track the mouse positions
	if (e.p0 != Vector2f(0,0) && e.p1 != Vector2f(0,0)) {
		e.p0 = e.p1;
		e.p1 = pixel;
	} else { e.p0 = e.p1 = pixel; }

	if (e.mode == TRANSLATION_MODE && e.ith_obj != -1 && e.obj_clicked) {
		Eigen::Vector3f world_p0 = e.pixel_to_world_coord(e.p0, width, height);
		Eigen::Vector3f world_p1 = e.pixel_to_world_coord(e.p1, width, height);
		if (e.camera_type == 0) {
			e.translation(0, e.ith_obj * 4 + 3) += (world_p1(0) - world_p0(0)) * 7*e.radius/54;
			e.translation(1, e.ith_obj * 4 + 3) += (world_p1(1) - world_p0(1)) * 7*e.radius/54;
			e.translation(2, e.ith_obj * 4 + 3) += (world_p1(2) - world_p0(2)) * 7*e.radius/54;
		} else {
			e.translation(0, e.ith_obj * 4 + 3) += (world_p1(0) - world_p0(0));
			e.translation(1, e.ith_obj * 4 + 3) += (world_p1(1) - world_p0(1));
			e.translation(2, e.ith_obj * 4 + 3) += (world_p1(2) - world_p0(2));
		}
	}
	if (e.mode == ANIMATION_MODE && e.closest_control_point != -1) {
		Eigen::Vector3f world_p0 = e.pixel_to_world_coord(e.p0, width, height);
		Eigen::Vector3f world_p1 = e.pixel_to_world_coord(e.p1, width, height);
		Vector3f shift (world_p1(0)-world_p0(0), world_p1(1)-world_p0(1), world_p1(2)-world_p0(2));
		if (e.camera_type == 0) {shift = shift * 7*e.radius/54; }
		Eigen::Matrix4f t_m;
		t_m << 1, 0, 0, shift(0),
			   0, 1, 0, shift(1),
			   0, 0, 1, shift(2),
			   0, 0, 0, 1;
		Vector3f v = (Vector3f) e.control_points[e.ith_obj].col(e.closest_control_point);
		Vector4f stored_v = t_m * Vector4f(v(0), v(1), v(2), 1);
		e.control_points[e.ith_obj].col(e.closest_control_point) << stored_v(0)/stored_v(3), stored_v(1)/stored_v(3), stored_v(2)/stored_v(3);
		e.V.col(e.vertex_count + e.closest_control_point) << stored_v(0), stored_v(1), stored_v(2);
	}
}

void mouse_click_callback(GLFWwindow* window, int button, int action, int mods) {
	if (e.mode == TRANSLATION_MODE) {	
		if (action == GLFW_RELEASE) { e.obj_clicked = false; } 
		else if (!e.click_on_obj(e.p1)) { e.ith_obj = -1; }
	} else if (e.mode == DELETE_MODE) {
		if (e.click_on_obj(e.p1) && action == GLFW_PRESS) { // down edge: delete triangle
			e.delete_at(e.ith_obj);
			e.ith_obj = -1;
			e.obj_clicked = false;
		}
		if (action == GLFW_RELEASE) { e.ith_obj = -1; }
	} else if (e.mode == ANIMATION_MODE) {
		if (action == GLFW_PRESS && e.bezier_step < 4) {
			if (e.ith_obj == -1) { e.click_on_obj(e.p1); } 
			else {
				Eigen::Vector3f world_p0 = e.pixel_to_world_coord(Vector2f(e.width/2, e.height/2), e.width, e.height);    		
				Eigen::Vector3f world_p1 = e.pixel_to_world_coord(e.p1, e.width, e.height);
				Vector3f v = (Vector3f) e.control_points[e.ith_obj].col(e.bezier_step);
				if (v == Vector3f(0,0,0)) {
					Vector3f shift (world_p1(0)-world_p0(0), world_p1(1)-world_p0(1), world_p1(2)-world_p0(2));
					if (e.camera_type == 0) {shift = shift * 7*e.radius/54;}
					Eigen::Matrix4f t_m;
					t_m << 1, 0, 0, shift(0),
						   0, 1, 0, shift(1),
						   0, 0, 1, shift(2),
						   0, 0, 0, 1;
					Eigen::Matrix4f r_m = e.rotation.block(0, e.ith_obj * 4, 4, 4);
					Eigen::Matrix4f s_m = e.scaling.block(0, e.ith_obj * 4, 4, 4);
					Vector4f stored_v = t_m * r_m * s_m * Vector4f(0.0,0.0,0.0,1);

					e.control_points[e.ith_obj].col(e.bezier_step) << stored_v(0)/stored_v(3), stored_v(1)/stored_v(3), stored_v(2)/stored_v(3);
					e.V.conservativeResize(3, e.vertex_count + e.bezier_step + 1);
					e.V.col(e.vertex_count + e.bezier_step) << stored_v(0), stored_v(1), stored_v(2);
					e.bezier_step ++;
				}
			}
		} else if (action == GLFW_PRESS && e.bezier_step == 4) {
			bool click = e.click_on_obj(e.p1);
			if (!click) { e.find_closest_control_point(); } 
			else {
				e.closest_control_point = -1;
				Vector3f v = (Vector3f) e.control_points[e.ith_obj].col(3);
				if (v != Vector3f(0,0,0)) {
					e.V.conservativeResize(3, e.vertex_count + 4);
					for (int k =0; k < 4; k++) { e.V.col(e.vertex_count + k) = e.control_points[e.ith_obj].col(k); }
				} else { e.bezier_step = 0; }
			} 
		}
		if (action == GLFW_RELEASE) { e.closest_control_point = -1; }
    }
	VBO.update(e.V); // Upload the change to the GPU
	VBO_n.update(e.VN.transpose());
}

void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods) {
	if (key == GLFW_KEY_O && action == GLFW_RELEASE) { e.switch_mode(TRANSLATION_MODE); }
	if (key == GLFW_KEY_P && action == GLFW_RELEASE) { e.switch_mode(DELETE_MODE); }
	if (key == GLFW_KEY_U && action == GLFW_RELEASE) { e.switch_mode(ANIMATION_MODE); }
	if (key == GLFW_KEY_Q && action == GLFW_RELEASE) { e.switch_mode(QUIT_MODE); }
	if (key == 49 && action == GLFW_RELEASE) { e.add_obj(0); }
	if (key == 50 && action == GLFW_RELEASE) { e.add_obj(1); }
	if (key == 51 && action == GLFW_RELEASE) { e.add_obj(2); }
	if (key == 52 && action == GLFW_RELEASE) { e.add_obj(3); }
	if (key == 53 && action == GLFW_RELEASE) { e.add_obj(4); }
	if (key == GLFW_KEY_COMMA && action == GLFW_RELEASE && e.ith_obj != -1) { e.change_color(1); }
	if (key == GLFW_KEY_PERIOD && action == GLFW_RELEASE && e.ith_obj != -1) { e.change_color(0); }
	if (key == GLFW_KEY_LEFT_BRACKET  && action == GLFW_RELEASE) { e.toggle_render_mode(1); }
	if (key == GLFW_KEY_RIGHT_BRACKET  && action == GLFW_RELEASE) { e.toggle_render_mode(0); }
	if (key == GLFW_KEY_SLASH && action == GLFW_RELEASE) { e.camera_type = abs(e.camera_type-1); }
	if (key == GLFW_KEY_G) { e.rotate_by(2,1,0); }
	if (key == GLFW_KEY_H) { e.rotate_by(2,1,1); }
	if (key == GLFW_KEY_J) { e.rotate_by(2,1,2); }
	if (key == GLFW_KEY_B) { e.rotate_by(2,0,0); }
	if (key == GLFW_KEY_N) { e.rotate_by(2,0,1); }
	if (key == GLFW_KEY_M) { e.rotate_by(2,0,2); }
	if (key == GLFW_KEY_K) { e.scale_by(0.05,1); }
	if (key == GLFW_KEY_L) { e.scale_by(0.05,0); }
	if (key == GLFW_KEY_LEFT) { e.alpha_shift -= M_PI/90.0; }
	if (key == GLFW_KEY_RIGHT) { e.alpha_shift += M_PI/90.0; }
	if (key == GLFW_KEY_UP) { e.beta_shift -= M_PI/90.0; }
	if (key == GLFW_KEY_DOWN) { e.beta_shift += M_PI/90.0; }
	if (key == GLFW_KEY_MINUS) {e.view.topLeftCorner(2,2) = e.view.topLeftCorner(2,2) * 0.97;}
	if (key == GLFW_KEY_EQUAL) {e.view.topLeftCorner(2,2) = e.view.topLeftCorner(2,2) * 1.03;}
	if (key == GLFW_KEY_SPACE && action == GLFW_RELEASE) { e.pause = abs(e.pause-1); if (e.pause == 1) {e.alpha_shift = e.alpha;} }
	if (key == GLFW_KEY_Z && action == GLFW_RELEASE && e.mode == ANIMATION_MODE) { e.start_animation = abs(e.start_animation-1); }
	if (key == GLFW_KEY_X && action == GLFW_RELEASE) { e.screenshot(); std::cout << "svg will be save to build folder" << std::endl; }
	if (key == GLFW_KEY_W) { e.camera_move(2, 0); }
	if (key == GLFW_KEY_S) { e.camera_move(2, 1); }
	if (key == GLFW_KEY_E) { e.camera_move(1, 0); }
	if (key == GLFW_KEY_D) { e.camera_move(1, 1); }
	if (key == GLFW_KEY_R) { e.camera_move(3, 0); }
	if (key == GLFW_KEY_F) { e.camera_move(3, 1); }

	VBO.update(e.V); // Upload the change to the GPU
	VBO_n.update(e.VN.transpose());
}

// Main
int main(void) {
	GLFWwindow* window;
	if (!glfwInit()) { return -1; }     // Initialize the library
	glfwWindowHint(GLFW_SAMPLES, 8);    // Activate supersampling
	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);    // Ensure that we get at least a 3.2 context
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
	// On apple we have to load a core profile with forward compatibility
	#ifdef __APPLE__
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, GL_TRUE);
		glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GL_TRUE);
	#endif
	// Create a windowed mode window and its OpenGL context
	window = glfwCreateWindow(640, 480, "Assignment 3", NULL, NULL);
	if (!window) {
		glfwTerminate();
		return -1;
	}
	glfwMakeContextCurrent(window); // Make the window's context current

	#ifndef __APPLE__
		glewExperimental = true;
		GLenum err = glewInit();
		if(GLEW_OK != err) {
		/* Problem: glewInit failed, something is seriously wrong. */
		fprintf(stderr, "Error: %s\n", glewGetErrorString(err));
		}
		glGetError(); // pull and savely ignonre unhandled errors like GL_INVALID_ENUM
		fprintf(stdout, "Status: Using GLEW %s\n", glewGetString(GLEW_VERSION));
	#endif

	int major, minor, rev;
	major = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MAJOR);
	minor = glfwGetWindowAttrib(window, GLFW_CONTEXT_VERSION_MINOR);
	rev = glfwGetWindowAttrib(window, GLFW_CONTEXT_REVISION);
	printf("OpenGL version recieved: %d.%d.%d\n", major, minor, rev);
	printf("Supported OpenGL is %s\n", (const char*)glGetString(GL_VERSION));
	printf("Supported GLSL is %s\n", (const char*)glGetString(GL_SHADING_LANGUAGE_VERSION));

	e.init();
								// Initialize the VAO
	VertexArrayObject VAO;		// A Vertex Array Object (or VAO) is an object that describes how the vertex
	VAO.init();					// attributes are stored in a Vertex Buffer Object (or VBO). This means that
	VAO.bind();					// the VAO is not the actual object storing the vertex data,
								// but the descriptor of the vertex data.
	VBO.init();           // Initialize the VBO with the vertices data
	VBO.update(e.V);      // A VBO is a data container that lives in the GPU memory
	VBO_n.init();
	VBO_n.update(e.VN.transpose());
	Program program;   	// Initialize the OpenGL Program. A program controls the OpenGL pipeline and it must contains
						// at least a vertex shader and a fragment shader to be valid
	program.init("../src/vertex_shader.glsl","../src/fragment_shader.glsl","outColor"); // Compile the two shaders and upload the binary to the GPU
	program.bind();          // Note that we have to explicitly specify that the output "slot" called outColor
	                         // is the one that we want in the fragment buffer (and thus on screen)
	glfwSetKeyCallback(window, key_callback);                 // Register the keyboard callback
	glfwSetMouseButtonCallback(window, mouse_click_callback); // Register the mouse callback
	glfwSetCursorPosCallback(window, mouse_move_callback);    // Register the cursor move callback
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback); // Update viewport
	glfwSetScrollCallback(window, mouse_scroll_callback);
	glEnable(GL_DEPTH_TEST);
	glPointSize(10.0f);

	float n = 4.0, f = 100.0, l = -1.0, r = 1.0, t = 1.0, b = -1.0;
	e.persp << 2*n/(r-l), 0,           (r+l)/(r-l),      0,
			   0,         2*n/(t-b),   (t+b)/(t-b),      0,
			   0,         0,           (-1)*(f+n)/(f-n), (-1)*(2*f*n)/(f-n),
			   0,         0,           -1,               0;
	e.orth << 2/(r-l),   0,       0,        -(r+l)/(r-l),
			  0,         2/(t-b), 0,        -(t+b)/(t-b),
			  0,         0,       -2/(f-n), -(f+n)/(f-n),
			  0,         0,       0,        1; 

	TimePoint t_start = std::chrono::high_resolution_clock::now();
	TimePoint t_now = std::chrono::high_resolution_clock::now();
	TimePoint new_t_start = TimePoint();
	//float total_pause_time = 0.0;
	//float cur_pause_time = 0.0;
	float time = 0.0;
	float glob_time = 0.0;
	float alpha = 0.0, beta = 0.0;

	while (!glfwWindowShouldClose(window) && e.mode != QUIT_MODE) {
		VAO.bind();    // Bind your VAO (not necessary if you have only one)
		program.init("../src/vertex_shader.glsl","../src/fragment_shader.glsl","outColor");
		program.bind();
		// The following line connects the VBO we defined above with the position "slot" in the vertex shader
		program.bindVertexAttribArray("position",VBO); // The vertex shader wants the position of the vertices as an input.
		program.bindVertexAttribArray("vertex_normal",VBO_n);

		int width, height;
		glfwGetWindowSize(window, &width, &height);
		e.width = float(width);
		e.height = float(height);
		float aspect_ratio = float(height)/float(width);
		if (e.view(0,0)/e.view(1,1) != aspect_ratio) { e.view(0,0) = aspect_ratio * e.view(1,1); }

		// Clear the framebuffer
		glClearColor(1.0f, 1.0f, 1.0f, 0.4f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		glUniform1i(program.uniform("normal_type"), e.normal_type);
		glUniform1i(program.uniform("start_animation"), e.start_animation);
		glUniformMatrix4fv(program.uniform("view"), 1, GL_FALSE, e.view.data());
		//glUniform1i(program.uniform("animated"), e.mode == ANIMATION_MODE);

		t_now = std::chrono::high_resolution_clock::now();
		glob_time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - t_start).count();

		if (e.pause) {
			alpha = e.alpha_shift;
			new_t_start = std::chrono::high_resolution_clock::now();
		} 
		else {
			time = std::chrono::duration_cast<std::chrono::duration<float>>(t_now - new_t_start).count();
			alpha = e.alpha_shift + 0.3 * time;
		}
		beta = e.beta_shift;

		e.cam_pos = e.radius * Vector3f(cos(alpha)*cos(beta), sin(beta), sin(alpha)*cos(beta));

		Eigen::Matrix4f camera_d, camera_pos;
		camera_d << cos(M_PI/2.0-alpha),   0, -sin(M_PI/2.0-alpha), 0,
					-cos(alpha)*sin(beta), cos(beta),  -sin(alpha)*sin(beta),   0,
					cos(alpha)*cos(beta),  sin(beta),  sin(alpha)*cos(beta),    0,
					0,                     0, 0,                      1;
		camera_pos << 1, 0, 0, -e.cam_pos(0),
				 	  0, 1, 0, -e.cam_pos(1),
					  0, 0, 1, -e.cam_pos(2),
				 	  0, 0, 0, 1;
		e.camera = camera_d * camera_pos;
		e.alpha = alpha;

		glUniform1f(program.uniform("time"), time);
		glUniformMatrix4fv(program.uniform("camera"), 1, GL_FALSE, e.camera.data());
		if (e.camera_type == 0) { glUniformMatrix4fv(program.uniform("proj"), 1, GL_FALSE, e.persp.data()); } 
		else { glUniformMatrix4fv(program.uniform("proj"), 1, GL_FALSE, e.orth.data()); }
		Vector3f light_d = e.cam_pos.normalized();
		glUniform3f(program.uniform("light_d"), light_d(0), light_d(1),light_d(2));

		if (e.mode == TRANSLATION_MODE && e.ith_obj != -1) {
			Eigen::Matrix4f t_m = e.translation.block(0, e.ith_obj * 4, 4, 4);
			Eigen::Matrix4f r_m = e.rotation.block(0, e.ith_obj * 4, 4, 4);
			Eigen::Matrix4f s_m = e.scaling.block(0, e.ith_obj * 4, 4, 4);
			e.model.block(0, e.ith_obj * 4, 4, 4) = t_m * r_m * s_m;
		}
		if (e.start_animation) {
			for (int i = 0; i < e.objects.size(); i++) {
				Eigen::MatrixXf control_matrix = e.control_points[i];
				Vector3f v = (Vector3f) control_matrix.col(3);

				if (v != Vector3f(0.0, 0.0, 0.0)) {
					float normal_time = remainder(glob_time, 10.0);
					if (normal_time < 0) { normal_time += 10.0; }
					if (normal_time > 5.0) { normal_time = 10.0 - normal_time; }
					float bezier_x = e.bezier_curve(control_matrix(0,0), control_matrix(0,1), control_matrix(0,2), control_matrix(0,3), normal_time/5.0);
					float bezier_y = e.bezier_curve(control_matrix(1,0), control_matrix(1,1), control_matrix(1,2), control_matrix(1,3), normal_time/5.0);
					float bezier_z = e.bezier_curve(control_matrix(2,0), control_matrix(2,1), control_matrix(2,2), control_matrix(2,3), normal_time/5.0);
					Eigen::Matrix4f t_m;
					t_m << 1, 0, 0, bezier_x,
						   0, 1, 0, bezier_y,
						   0, 0, 1, bezier_z,
						   0, 0, 0, 1;
					Eigen::Matrix4f r_m = e.rotation.block(0, i * 4, 4, 4);
					Eigen::Matrix4f s_m = e.scaling.block(0, i * 4, 4, 4);
					e.anim_model.block(0, i * 4, 4, 4) = t_m * r_m * s_m;
				} else { e.anim_model.block(0, i * 4, 4, 4) = e.model.block(0, i * 4, 4, 4); }
			}
		}
		// Draw triangles
		for (int j = 0; j < e.objects.size(); j++) {
			glUniform1i(program.uniform("color_bit"), e.color_bits[j]);
			glUniformMatrix4fv(program.uniform("model"), 1, GL_FALSE, &e.model(0, j * 4));
			glUniformMatrix4fv(program.uniform("anim_model"), 1, GL_FALSE, &e.anim_model(0, j * 4));
			int num_tri = e.obj_index[e.objects[j]];
			int t = 0;
			for (int k = 0; k < e.objects[j]; k++) { t += e.obj_index[k]; } // count total triangles before head
			for (int k = 0; k < num_tri; k++) {
				if (e.normal_type != 0) {
					glUniform1i(program.uniform("draw_frame"), 0);
					Vector4f face_normal (e.N(t+k,0), e.N(t+k,1), e.N(t+k,2), 0);
					face_normal = e.model.block(0, j * 4, 4, 4) * face_normal;

					glUniform3f(program.uniform("face_normal"), face_normal(0), face_normal(1), face_normal(2));
					glDrawArrays(GL_TRIANGLES, t*3 + k*3, 3);
				}
				if (e.normal_type != 3) {
					glUniform1i(program.uniform("draw_frame"), 1);
					glDrawArrays(GL_LINE_LOOP, t*3 + k*3, 3);
				}
			}
		}
		glUniform1i(program.uniform("draw_bezier_curve"), 1);
		if (e.V.cols() > e.vertex_count) {
			if (e.ith_obj != -1) {
				int k = 0;
				Eigen::MatrixXf control_matrix = e.control_points[e.ith_obj];
				for (k = 0; k < 4; k++) {
					Vector3f v = (Vector3f) control_matrix.col(k);
					if (v != Vector3f(0.0, 0.0, 0.0)) {
						Eigen::Matrix4f t_m;
						t_m << 1, 0, 0, v(0),
							   0, 1, 0, v(1),
							   0, 0, 1, v(2),
							   0, 0, 0, 1;
						Eigen::Matrix4f r_m = e.rotation.block(0, e.ith_obj * 4, 4, 4);
						Eigen::Matrix4f s_m = e.scaling.block(0, e.ith_obj * 4, 4, 4);
						Eigen::Matrix4f control_point_model = t_m * r_m * s_m;

						Eigen::Matrix4f identity = MatrixXf::Identity(4, 4);

						glUniformMatrix4fv(program.uniform("control_point_model"), 1, GL_FALSE, identity.data());
						glDrawArrays(GL_POINTS, e.vertex_count + k, 1);
					} else { break; }
				}
				glDrawArrays(GL_LINE_STRIP, e.vertex_count, k);
				if ((Vector3f) control_matrix.col(3) != Vector3f(0.0, 0.0, 0.0)) {
					e.V.conservativeResize(3, e.vertex_count + 4 + 100);
					for (int j=0; j < 100; j ++) {
						float t = float(j)/100;
						float bezier_x = e.bezier_curve(control_matrix(0,0), control_matrix(0,1), control_matrix(0,2), control_matrix(0,3), t);
						float bezier_y = e.bezier_curve(control_matrix(1,0), control_matrix(1,1), control_matrix(1,2), control_matrix(1,3), t);
						float bezier_z = e.bezier_curve(control_matrix(2,0), control_matrix(2,1), control_matrix(2,2), control_matrix(2,3), t);
						e.V.col(e.vertex_count + 4 + j) << bezier_x, bezier_y, bezier_z;
					}
					VBO.update(e.V);
					glDrawArrays(GL_LINE_STRIP, e.vertex_count + 4, 100);
				}
			}
		}
		glUniform1i(program.uniform("draw_bezier_curve"), 0);
		glfwSwapBuffers(window); // Swap front and back buffers
		glfwPollEvents(); // Poll for and process events
	}
	// Deallocate opengl memory
	program.free();
	VAO.free();
	VBO.free();
	VBO_n.free();
	// Deallocate glfw internals
	glfwTerminate();
	return 0;
}
