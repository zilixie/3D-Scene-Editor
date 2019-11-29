#include "normal.h"
#include <Eigen/Geometry>

Eigen::RowVector3f area_normal(const Eigen::RowVector3f v1, const Eigen::RowVector3f v2, const Eigen::RowVector3f v3) {
	Eigen::RowVector3f e1 = v1-v2;
	Eigen::RowVector3f e2 = v3-v2;
	Eigen::RowVector3f e3 = v3-v1;

	float cos = (e1.dot(e2))/(e1.norm()*e2.norm());
	float sin = sqrt(1-pow(cos,2));
	float area = (0.5)*e1.norm()*e2.norm()*sin;
	Eigen::RowVector3f normal = e1.cross(e2);
	normal = (-1) * area * normal.normalized();

	return normal;
}

void per_face_normals(const Eigen::MatrixXf & V, const Eigen::MatrixXi & F, Eigen::MatrixXf & N) {
	N = Eigen::MatrixXf::Zero(F.rows(),3);
	for (int i=0; i<F.rows(); i++) {
		Eigen::RowVector3f v1 = V.row(F(i,0));
		Eigen::RowVector3f v2 = V.row(F(i,1));
		Eigen::RowVector3f v3 = V.row(F(i,2));
		Eigen::RowVector3f n = area_normal(v1,v2,v3);
		N.row(i) = n.normalized();
	}
}

void per_vertex_normals(const Eigen::MatrixXf & V, const Eigen::MatrixXi & F, Eigen::MatrixXf & N) {
	N = Eigen::MatrixXf::Zero(V.rows(),3);
	for (int i=0; i<V.rows(); i++) {
		Eigen::RowVector3f nv (0,0,0);
		for (int j=0; j<F.rows(); j++) {
			if ((F(j,0) == i) || (F(j,1) == i) || (F(j,2) == i)) {
				nv += area_normal(V.row(F(j,0)),V.row(F(j,1)),V.row(F(j,2)));
			}
		}
		N.row(i) = nv.normalized();
	}
}
