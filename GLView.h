#ifndef GLVIEW_H
#define GLVIEW_H

#define GL_GLEXT_PROTOTYPES
#include <QtOpenGL>

class GLView : public QGLWidget
{
    Q_OBJECT
public:
    explicit GLView(QWidget *parent = 0);
	//! \todo destuctor and gl delete stuff

protected:
	void initializeGL();
	void resizeGL(int width, int height);
	void paintGL();

public slots:
	void UpdateVertexShader(QString shader);
	void UpdateFragmentShader(QString shader);

private:
	char* Compile(const QString& src, int shader);
	void Relink();

signals:
	void VertexShaderError(QString error);
	void FragmentShaderError(QString error);
	void LinkProgramError(QString error);

private:
	int _vertex_shader, _fragment_shader;
	int _shader_program;
};

#endif // GLVIEW_H
