#include "GLView.h"

GLView::GLView(QWidget *parent) :
    QGLWidget(parent)
{
}

void GLView::initializeGL()
{
	qDebug("YGLScreen::initializeGL()");

	_vertex_shader = glCreateShader(GL_VERTEX_SHADER);
	_fragment_shader = glCreateShader(GL_FRAGMENT_SHADER);
	_shader_program = glCreateProgram();
	glAttachShader(_shader_program, _vertex_shader);
	glAttachShader(_shader_program, _fragment_shader);
}

char* GLView::Compile(const QString& src, int shader)
{
	const char* psrc = src.toLatin1().constData();
	glShaderSource(shader, 1, &psrc, NULL);
	glCompileShader(shader);

	int result;
	glGetShaderiv(shader, GL_COMPILE_STATUS, &result);
	if (result != GL_TRUE)
	{
		glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &result);
		char* buf = new char[result];
		glGetShaderInfoLog(shader, result, 0, buf);
		return buf;
	}
	return 0;
}

void GLView::Relink()
{
	glLinkProgram(_shader_program);

	int result;
	glGetProgramiv(_shader_program, GL_LINK_STATUS, &result);
	if (result != GL_TRUE)
	{
		glGetProgramiv(_shader_program, GL_INFO_LOG_LENGTH, &result);
		char* buf = new char[result];
		glGetProgramInfoLog(_shader_program, result, 0, buf);
		qDebug(buf);
		emit LinkProgramError(QString(buf));
		delete[] buf;
		return;
	}

	glUseProgram(_shader_program);
	update();
}

void GLView::UpdateVertexShader(QString shader)
{
	char* error = Compile(shader, _vertex_shader);
	if (error)
	{
		qDebug(error);
		emit VertexShaderError(QString(error));
		delete[] error;
		return;
	}

	Relink();
}

void GLView::UpdateFragmentShader(QString shader)
{
	char* error = Compile(shader, _fragment_shader);
	if (error)
	{
		qDebug(error);
		emit FragmentShaderError(QString(error));
		delete[] error;
		return;
	}

	Relink();
}

void GLView::resizeGL(int width, int height)
{
	qDebug("YGLScreen::resizeGL(%d, %d)", width, height);
	glViewport(0, 0, width, height);
}

void GLView::paintGL()
{
	//qDebug("YGLScreen::paintGL()");
	glBegin(GL_QUADS);
	glVertex2f(-1., -1.);
	glVertex2f(-1, 1.);
	glVertex2f(1., 1.);
	glVertex2f(1., -1.);
	glEnd();
}
