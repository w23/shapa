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

bool CProgram::loadFromFile(QString filename) {
  QFile file(filename);
  if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
    emit error(QString("Cannot open file %1").arg(filename));
    return false;
  }

  QXmlStreamReader xml(&file);
  QString newFragmentShader, newVertexShader;
  while (!xml.atEnd() && !xml.hasError()) {
    QXmlStreamReader::TokenType token = xml.readNext();
    if (token == QXmlStreamReader::StartElement) {
      if (xml.name() == "version") {
        QXmlStreamAttributes a = xml.attributes();
        if (!a.hasAttribute("gl_major") || a.value("gl_major") != "2" ||
            !a.hasAttribute("gl_minor") || a.value("gl_minor") != "1") {
          emit error(QString("Unsupported shader version"));
          return false;
        }
      } else if (xml.name() == "shader_vertex")
        newVertexShader = xml.readElementText();
      else if (xml.name() == "shader_fragment")
        newFragmentShader = xml.readElementText();
    } else if (token == QXmlStreamReader::EndDocument) break;
  }

  if (xml.hasError()) {
    emit error(QString("There were errors while parsing XML: %1").arg(xml.errorString()));
    return false;
  }

  if (newFragmentShader.isEmpty() || newVertexShader.isEmpty()) {
    emit error(QString("File lacks meaningful content"));
    return false;
  }

  changed_ = false;
  vertexShader_ = newVertexShader;
  fragmentShader_ = newFragmentShader;
  return true;
}

bool CProgram::saveToFile(QString filename) {
  QFile file(filename);
  if (!file.open(QIODevice::WriteOnly)) {
    emit error(QString("Cannot save to file %1").arg(filename));
    return false;
  }

  QXmlStreamWriter xml(&file);
  xml.setAutoFormatting(true);
  xml.writeStartDocument();
  xml.writeStartElement("shapa");
  xml.writeEmptyElement("version");
  xml.writeAttribute("gl_major", "2");
  xml.writeAttribute("gl_minor", "1");
  xml.writeStartElement("shader_vertex");
  xml.writeCDATA(vertexShader_);
  xml.writeEndElement();
  xml.writeStartElement("shader_fragment");
  xml.writeCDATA(fragmentShader_);
  xml.writeEndDocument();
  file.close();
  changed_ = false;
  return true;
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
