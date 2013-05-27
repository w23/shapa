#pragma once
#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class CProgram;
class GLView;
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
  void menuOpenNew();
  void menuSave();

private slots:
  void on_vertexShader_textChanged();
  void on_fragmentShader_textChanged();
  void on_runBox_toggled(bool checked);

private:
  void setupProgram();
  bool saveProgram();
  bool closeProgram();

private:
  Ui::MainWindow *ui;

  QString file_;
  CProgram *program_;
  GLView *glview_;
};
