#pragma once
#include <QObject>

class CProgram : public QObject {
  Q_OBJECT
public:
  explicit CProgram(QObject *parent = 0);

  bool hasChanged() const { return changed_; }
  const QString& getVertexShader() const { return vertexShader_; }
  const QString& getFragmentShader() const { return fragmentShader_; }

signals:
  void error(QString message);
  void fragmentShaderChanged(QString source);
  void vertexShaderChanged(QString source);

public slots:
  void loadFromFile(QString filename);
  void saveToFile(QString filename);
  void updateVertexShader(QString source);
  void updateFragmentShader(QString source);
    
private:
  bool changed_;
  QString vertexShader_;
  QString fragmentShader_;

  static const QString s_defaultVertexShader_;
  static const QString s_defaultFragmentShader_;
};
