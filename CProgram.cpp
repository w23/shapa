#include <QFile>
#include <QXmlStreamWriter>
#include <QXmlStreamReader>
#include "CProgram.h"

const QString CProgram::s_defaultVertexShader_ = QString(
  "attribute vec2 vertex;\n"
  "varying vec2 p;\n"
  "void main() {\n"
    "gl_Position = vec4(vertex, 0., 1.);\n"
    "p = vertex.xy;\n"
  "}\n");

const QString CProgram::s_defaultFragmentShader_ = QString(
  "varying vec2 p;\n"
  "void main() {\n"
    "gl_FragColor = vec4(p, 0., 0.);\n"
  "}\n");

CProgram::CProgram(QObject *parent) : QObject(parent)
  , changed_(false)
  , vertexShader_(s_defaultVertexShader_)
  , fragmentShader_(s_defaultFragmentShader_)
{}

void CProgram::loadFromFile(QString filename) {
  //! \todo
  abort();
  changed_ = false;
}

void CProgram::saveToFile(QString filename) {
  QFile file(filename);
  if (!file.open(QIODevice::WriteOnly)) {
    emit error(QString("Cannot save to file %1").arg(filename));
    return;
  }

  QXmlStreamWriter xml(&file);
  xml.setAutoFormatting(true);
  xml.writeStartDocument();
  xml.writeStartElement("shader_vertex");
  xml.writeCDATA(vertexShader_);
  xml.writeEndElement();
  xml.writeStartElement("shader_fragment");
  xml.writeCDATA(fragmentShader_);
  xml.writeEndElement();
  xml.writeEndDocument();
  file.close();
  changed_ = false;
}

void CProgram::updateVertexShader(QString source) {
  changed_ = true;
  vertexShader_ = source;
  emit vertexShaderChanged(vertexShader_);
}

void CProgram::updateFragmentShader(QString source) {
  changed_ = true;
  fragmentShader_ = source;
  emit fragmentShaderChanged(fragmentShader_);
}
