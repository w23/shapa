#include <QtGui/QApplication>
#include "MainWindow.h"
#include "GLView.h"

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

	MainWindow w;
    w.show();

	GLView gl;
	gl.resize(800,600);
	gl.show();

	gl.connect(&w, SIGNAL(willClose()), SLOT(close()));
	gl.connect(&w, SIGNAL(vertexShader(QString)), SLOT(UpdateVertexShader(QString)));
	gl.connect(&w, SIGNAL(fragmentShader(QString)), SLOT(UpdateFragmentShader(QString)));
	w.connect(&gl, SIGNAL(VertexShaderError(QString)), SLOT(showError(QString)));
	w.connect(&gl, SIGNAL(FragmentShaderError(QString)), SLOT(showError(QString)));
	w.connect(&gl, SIGNAL(LinkProgramError(QString)), SLOT(showError(QString)));
	w.emitState();

    return a.exec();
}
