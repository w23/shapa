#include <QtGui>
#include "MainWindow.h"
#include "GLView.h"

int main(int argc, char *argv[]) {
  QApplication a(argc, argv);
  MainWindow w;
  w.show();

  /*QGLFormat glfmt;
  glfmt.setVersion(3, 2);
  glfmt.setProfile(QGLFormat::CoreProfile);
  //glfmt.setProfile(QGLFormat::NoProfile);
  QGLFormat::setDefaultFormat(glfmt);*/

  return a.exec();
}
