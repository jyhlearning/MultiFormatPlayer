#pragma once
#include <windows.h>
#include <GL/gl.h>
#include <GL/glu.h>
#include <QOpenGLShaderProgram>
#include <QOpenGLTexture>
#include "QImage"
#include "QOpenGLWidget"
#include "QOpenGLFunctions"
class MFPOpenGLWidget : public QOpenGLWidget, protected QOpenGLFunctions {
	Q_OBJECT
private:
	QVector<QVector2D> texCoords;
	
	QOpenGLShaderProgram program;

	QOpenGLTexture* texture;

	QVector<QVector3D> vertices;

	float brightness,contrast,saturation;

	void initTextures();              //初始化纹理设置
	void initShaders();
	void scale(float& scaleX, float& scaleY, QOpenGLTexture* texture);
public:
	explicit MFPOpenGLWidget(QWidget* parent = nullptr);
	~MFPOpenGLWidget();
	void initializeGL() override;
	void paintGL() override;
	void resizeGL(int w, int h) override;
	void setImage(const QImage &image);
	void setBrightness(const float v);
	void setContrast(const float v);
	void setSaturation(const float v);
};
