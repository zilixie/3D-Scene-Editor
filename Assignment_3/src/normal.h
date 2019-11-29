#ifndef NORMAL_H
#define NORMAL_H
#include <Eigen/Core>

Eigen::RowVector3f area_normal(const Eigen::RowVector3f v1, const Eigen::RowVector3f v2, const Eigen::RowVector3f v3);
void per_face_normals(const Eigen::MatrixXf & V, const Eigen::MatrixXi & F, Eigen::MatrixXf & N);
void per_vertex_normals(const Eigen::MatrixXf & V, const Eigen::MatrixXi & F, Eigen::MatrixXf & N);

#endif

