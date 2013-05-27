#include <QMessageBox>
#include "MainWindow.h"
#include "ui_MainWindow.h"
#include "CProgram.h"
#include "GLView.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow),
    program_(0)
{
    ui->setupUi(this);
}

MainWindow::~MainWindow() {
  delete program_;
  delete ui;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
  closeProgram();
  return QMainWindow::closeEvent(e);
}

void MainWindow::showShaderError(QString error) {
  ui->errorLog->setText(error);
}

void MainWindow::on_vertexShader_textChanged() {
  if (program_) program_->updateVertexShader(ui->vertexShader->toPlainText());
}

void MainWindow::on_fragmentShader_textChanged() {
  if (program_) program_->updateFragmentShader(ui->fragmentShader->toPlainText());
}

void MainWindow::on_runBox_toggled(bool checked) {
  emit resumeRendering(checked);
}

void MainWindow::menuOpenNew() {
  if (closeProgram()) {
    delete program_;
    program_ = new CProgram();
    setupProgram();
  }
}

void MainWindow::menuSave() {
  saveProgram();
}

void MainWindow::setupProgram() {
  glview_ = new GLView(program_);
  glview_->resize(640, 360);
  glview_->show();
  connect(this, SIGNAL(resumeRendering(bool)), glview_, SLOT(ResumeRendering(bool)));
  connect(glview_, SIGNAL(VertexShaderError(QString)), SLOT(showShaderError(QString)));
  connect(glview_, SIGNAL(FragmentShaderError(QString)), SLOT(showShaderError(QString)));
  connect(glview_, SIGNAL(LinkProgramError(QString)), SLOT(showShaderError(QString)));

  ui->fragmentShader->setPlainText(program_->getFragmentShader());
  ui->fragmentShader->setEnabled(true);
  ui->vertexShader->setPlainText(program_->getVertexShader());
  ui->vertexShader->setEnabled(true);
  ui->runBox->setEnabled(true);
  glview_->ResumeRendering(ui->runBox->checkState() == Qt::Checked);
}

bool MainWindow::saveProgram() {
  if (program_) {
    QMessageBox box;
    box.setText("TODO SAVE");
    box.exec();
  }
  return true;
}

bool MainWindow::closeProgram() {
  if (!program_ || !glview_) return true;
  ui->fragmentShader->setEnabled(false);
  ui->vertexShader->setEnabled(false);
  ui->runBox->setEnabled(false);
  glview_->close();
  delete glview_; glview_ = 0;
  delete program_; program_ = 0;
  return true;
}
