#pragma once
#define GL_GLEXT_PROTOTYPES
#include <QtOpenGL>

class CProgram;
class GLView : public QGLWidget {
  Q_OBJECT
public:
  explicit GLView(CProgram *program, QWidget *parent = 0);
  //! \todo destuctor and gl delete stuff

protected:
  void initializeGL();
  void resizeGL(int width, int height);
  void paintGL();

public slots:
  void UpdateVertexShader(QString shader);
  void UpdateFragmentShader(QString shader);
  void ResumeRendering(bool resume);

signals:
  void VertexShaderError(QString error);
  void FragmentShaderError(QString error);
  void LinkProgramError(QString error);

private:
  void Relink();

private:
  CProgram *program_;
  QElapsedTimer _etimer;
  QTimer _timer;
  bool _active;
  QOpenGLBuffer *quadGeometry_;
  QOpenGLShader *vertexShader_, *fragmentShader_;
  QOpenGLShaderProgram *shaderProgram_;
  static const float s_quad_geometry[8];
};
