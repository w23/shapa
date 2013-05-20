#include "GLView.h"

const float GLView::s_quad_geometry[8] = {
    1.f, -1.f, 1.f, 1.f, -1.f, -1.f, -1.f, 1.f
};

GLView::GLView(QWidget *parent) :
	QGLWidget(parent),
    _timer(this),
    _active(false),
    quadGeometry_(0), vertexShader_(0), fragmentShader_(0)
{
	connect(&_timer, SIGNAL(timeout()),SLOT(update()));
}

void GLView::initializeGL()
{
	qDebug("YGLScreen::initializeGL()");

    GLuint vao;
    glGenVertexArrays(1, &vao);
    glBindVertexArray(vao);

    quadGeometry_ = new QOpenGLBuffer(QOpenGLBuffer::VertexBuffer);
    vertexShader_ = new QOpenGLShader(QOpenGLShader::Vertex);
    fragmentShader_ = new QOpenGLShader(QOpenGLShader::Fragment);
    shaderProgram_ = new QOpenGLShaderProgram();
    shaderProgram_->addShader(vertexShader_);
    shaderProgram_->addShader(fragmentShader_);

    quadGeometry_->setUsagePattern(QOpenGLBuffer::StaticDraw);
    quadGeometry_->create();
    quadGeometry_->bind();
    quadGeometry_->allocate(s_quad_geometry, sizeof(s_quad_geometry));
}

void GLView::Relink() {
    if (!shaderProgram_->link()) {
        emit LinkProgramError(shaderProgram_->log());
        return;
    }
    emit LinkProgramError(QString("K"));
    shaderProgram_->bind();
    shaderProgram_->setAttributeBuffer("vertex", GL_FLOAT, 0, 2);
    shaderProgram_->enableAttributeArray("vertex");
    ResumeRendering(_active);
}

void GLView::UpdateVertexShader(QString shader) {
    _timer.stop();
    if (!vertexShader_->compileSourceCode(shader)) {
        emit VertexShaderError(vertexShader_->log());
        return;
    }
    emit VertexShaderError(QString("OK"));
	Relink();
}

void GLView::UpdateFragmentShader(QString shader) {
    _timer.stop();
    if (!fragmentShader_->compileSourceCode(shader)) {
        emit FragmentShaderError(fragmentShader_->log());
        return;
    }
    emit FragmentShaderError(QString("OK"));
    Relink();
}

void GLView::resizeGL(int width, int height)
{
	qDebug("YGLScreen::resizeGL(%d, %d)", width, height);
	glViewport(0, 0, width, height);
}

void GLView::paintGL()
{
    float t = 1.f + _etimer.elapsed() / 1000.f;
    shaderProgram_->setUniformValue(shaderProgram_->uniformLocation("g_time"), t);
    glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
}

void GLView::ResumeRendering(bool resume)
{
    _active = resume;

    if (_active)
    {
        _timer.start(1000/30);
        _etimer.restart();
        update();
    }
    else
        _timer.stop();
}
