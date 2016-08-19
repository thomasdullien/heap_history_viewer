#ifndef TRANSFORM3D_H
#define TRANSFORM3D_H

#include <QMatrix4x4>
#include <QVector3D>
#include <QQuaternion>

class Transform3D
{
public:
  Transform3D();
  void translate(const QVector3D &dt);
  void scale(const QVector3D &ds);
  void rotate(float angle, const QVector3D &axis);
  void grow(const QVector3D &ds);
  const QMatrix4x4& toMatrix();
private:
  bool dirty_;
  QVector3D translation_;
  QVector3D scale_;
  QQuaternion rotation_;
  QMatrix4x4 world_;
};

#endif // TRANSFORM3D_H
