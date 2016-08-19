#include "transform3d.h"

Transform3D::Transform3D() : dirty_(true), scale_(1.0, 1.0, 1.0) {
}

void Transform3D::translate(const QVector3D &dt) {
  dirty_ = true;
  translation_ += dt;
}

void Transform3D::scale(const QVector3D &ds) {
  dirty_ = true;
  scale_ *= ds;
}

void Transform3D::rotate(float angle, const QVector3D &axis) {
  dirty_ = true;
  rotation_ *= QQuaternion::fromAxisAndAngle(axis, angle) * rotation_;
}

void Transform3D::grow(const QVector3D &ds) {
  dirty_ = true;
  scale_ += ds;
}

const QMatrix4x4& Transform3D::toMatrix() {
  if (dirty_) {
    world_.setToIdentity();
    world_.translate(translation_);
    world_.rotate(rotation_);
    world_.scale(scale_);
  }
  return world_;
}
