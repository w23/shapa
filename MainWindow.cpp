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
    QFont monospace("Monospace", 8);
    monospace.setStyleHint(QFont::Monospace);
    monospace.setFixedPitch(true);
    ui->fragmentShader->setFont(monospace);
    ui->vertexShader->setFont(monospace);
    ui->errorLog->setFont(monospace);

    QFontMetrics metric(monospace);
    const int tab_width = metric.width("  ");
    ui->fragmentShader->setTabStopWidth(tab_width);
    ui->vertexShader->setTabStopWidth(tab_width);

    if (QApplication::arguments().size() > 1)
      openProgram(QApplication::arguments().at(1));
}

MainWindow::~MainWindow() {
  delete program_;
  delete ui;
}

void MainWindow::closeEvent(QCloseEvent *e)
{
  if (!closeProgram()) e->ignore();
  else QMainWindow::closeEvent(e);
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
    filename_ = QString();
    setupProgram(new CProgram());
  }
}

void MainWindow::menuOpen() {
  if (closeProgram()) {
    QFileDialog dialog(this, "Open shader");
    dialog.setFileMode(QFileDialog::ExistingFile);
    dialog.setNameFilter("XML Shaders (*.xml)");
    if (!dialog.exec()) return;
    openProgram(dialog.selectedFiles().first());
  }
}

void MainWindow::menuSave() {
  saveProgram(filename_);
}

void MainWindow::openProgram(QString filename) {
  CProgram *program = new CProgram();
  connect(program, SIGNAL(error(QString)),
          this, SLOT(showErrorMessage(QString)));
  if (!program->loadFromFile(filename)) {
    delete program;
    return;
  }
  filename_ = filename;
  setupProgram(program);
}

void MainWindow::setupProgram(CProgram *program) {
  glview_ = new GLView(program);
  glview_->resize(640, 360);
  glview_->show();

  ui->fragmentShader->setPlainText(program->getFragmentShader());
  ui->fragmentShader->setEnabled(true);
  ui->vertexShader->setPlainText(program->getVertexShader());
  ui->vertexShader->setEnabled(true);
  ui->runBox->setEnabled(true);

  connect(this, SIGNAL(resumeRendering(bool)), glview_, SLOT(ResumeRendering(bool)));
  connect(glview_, SIGNAL(VertexShaderError(QString)), SLOT(showShaderError(QString)));
  connect(glview_, SIGNAL(FragmentShaderError(QString)), SLOT(showShaderError(QString)));
  connect(glview_, SIGNAL(LinkProgramError(QString)), SLOT(showShaderError(QString)));

  program_ = program;
  glview_->ResumeRendering(ui->runBox->checkState() == Qt::Checked);
}

bool MainWindow::saveProgram(QString filename) {
  if (!program_) return true;
  if (filename.isEmpty())
    filename = QFileDialog::getSaveFileName(this, "Save shader", QString(),
                                         "XML Shaders (*.xml)");
  if (program_->saveToFile(filename)) {
    filename_ = filename;
    return true;
  }
  return false;
}

bool MainWindow::closeProgram() {
  if (!program_ || !glview_) return true;

  if (program_->hasChanged()) {
    switch (QMessageBox::question(this, "Save shader?",
                                  "You have some changes here, dude!",
                                  QMessageBox::StandardButtons(QMessageBox::Yes
                                                               | QMessageBox::No
                                                               | QMessageBox::Cancel),
                                  QMessageBox::Cancel)) {
    case QMessageBox::Cancel:
      return false;
    case QMessageBox::Yes:
      saveProgram(filename_);
    default: {}
    }
  }

  ui->fragmentShader->setEnabled(false);
  ui->vertexShader->setEnabled(false);
  ui->runBox->setEnabled(false);
  glview_->close();
  delete glview_; glview_ = 0;
  delete program_; program_ = 0;
  return true;
}

void MainWindow::showErrorMessage(QString error) {
  QMessageBox::critical(this, "Error!11", error);
}
