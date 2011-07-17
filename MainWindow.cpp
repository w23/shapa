#include "MainWindow.h"
#include "ui_MainWindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::emitState()
{
	emit vertexShader(ui->vertexShader->toPlainText());
	emit fragmentShader(ui->fragmentShader->toPlainText());
}

void MainWindow::closeEvent(QCloseEvent *e)
{
	//! \todo check for unsaved changes
	emit willClose();
	return QMainWindow::closeEvent(e);
}

void MainWindow::showError(QString error)
{
	ui->errorLog->setText(error);
}

void MainWindow::on_vertexShader_textChanged()
{
	emit vertexShader(ui->vertexShader->toPlainText());
}

void MainWindow::on_fragmentShader_textChanged()
{
	emit fragmentShader(ui->fragmentShader->toPlainText());
}
