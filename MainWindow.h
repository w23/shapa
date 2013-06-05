#pragma once
#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class CProgram;
class GLView;
class GLSLSyntaxHighlighter;
class MainWindow : public QMainWindow {
  Q_OBJECT
public:
  explicit MainWindow(QWidget *parent = 0);
  ~MainWindow();

signals:
  void resumeRendering(bool resume);

protected:
  void closeEvent(QCloseEvent *);

public slots:
  void showShaderError(QString error);
  void showErrorMessage(QString error);
  void menuOpenNew();
  void menuOpen();
  void menuSave();

private slots:
  void on_vertexShader_textChanged();
  void on_fragmentShader_textChanged();
  void on_runBox_toggled(bool checked);

private:
  void setupProgram(CProgram *program);
  void openProgram(QString filename);
  bool saveProgram(QString filename);
  bool closeProgram();

private:
  Ui::MainWindow *ui;

  GLSLSyntaxHighlighter *fragmentHighlighter_;
  GLSLSyntaxHighlighter *vertexHighlighter_;

  QString filename_;
  CProgram *program_;
  GLView *glview_;
};
