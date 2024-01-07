﻿#pragma once
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

	void initTextures();              //初始化纹理设置
	void initShaders();
public:
	explicit MFPOpenGLWidget(QWidget* parent = nullptr);
	~MFPOpenGLWidget();
	void initializeGL() override;
	void paintGL() override;
	void resizeGL(int w, int h) override;
	void setImage(const QImage &image);
};