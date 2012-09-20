#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

namespace Ui {
    class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

	void emitState();

signals:
	void willClose();
	void vertexShader(QString vs);
	void fragmentShader(QString fs);
    void resumeRendering(bool resume);

protected:
	void closeEvent(QCloseEvent *);

public slots:
	void showError(QString error);

private slots:
	void on_vertexShader_textChanged();

	void on_fragmentShader_textChanged();

    void on_checkBox_toggled(bool checked);

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
