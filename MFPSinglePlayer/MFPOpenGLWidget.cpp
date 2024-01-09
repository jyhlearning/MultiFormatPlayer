#include "MFPOpenGLWidget.h"

void MFPOpenGLWidget::initTextures() {
	// 加载 Avengers.jpg 图片
	texture = new QOpenGLTexture(QOpenGLTexture::Target2D);
	// 设置最近的过滤模式，以缩小纹理  
	texture->setMinificationFilter(QOpenGLTexture::LinearMipMapLinear); //滤波
	// 设置双线性过滤模式，以放大纹理
	texture->setMagnificationFilter(QOpenGLTexture::Linear);
	// 重复使用纹理坐标    
	// 纹理坐标(1.1, 1.2)与(0.1, 0.2)相同
	texture->setWrapMode(QOpenGLTexture::ClampToBorder);
	//分配储存空间
	texture->allocateStorage();
}

void MFPOpenGLWidget::initShaders() {
	//纹理坐标   
	texCoords.append(QVector2D(0, 1)); //左上
	texCoords.append(QVector2D(1, 1)); //右上    
	texCoords.append(QVector2D(0, 0)); //左下
	texCoords.append(QVector2D(1, 0)); //右下   
	//顶点坐标
	vertices.append(QVector3D(-1, -1, 1)); //左下   
	vertices.append(QVector3D(1, -1, 1)); //右下
	vertices.append(QVector3D(-1, 1, 1)); //左上    
	vertices.append(QVector3D(1, 1, 1)); //右上
	QOpenGLShader* vshader = new QOpenGLShader(QOpenGLShader::Vertex, this);
	const char* vsrc =
		"attribute vec4 vertex;\n"
		"attribute vec2 texCoord;\n"
		"varying vec2 texc;\n"
		"void main(void)\n"
		"{\n"
		"    gl_Position = vertex;\n"
		"    texc = texCoord;\n"
		"}\n";
	vshader->compileSourceCode(vsrc); //编译顶点着色器代码
	QOpenGLShader* fshader = new QOpenGLShader(QOpenGLShader::Fragment, this);
	const char* fsrc =
		"uniform sampler2D texture;\n"
		"varying vec2 texc;\n"
		"uniform float brightness;\n"
		"uniform float contrast;\n"
		"uniform float saturation;\n"
		"void main(void)\n"
		"{\n"
		"    vec4 textureColor = texture2D(texture, texc);\n"
		"	 const mediump vec3 luminanceWeighting = vec3(0.2125, 0.7154, 0.0721);\n"
		"	 vec3 greyScaleColor = vec3(dot(textureColor.rgb, luminanceWeighting));"
		"	 vec3 color = textureColor.rgb + vec3(brightness);\n"
		"    vec3 color1 = (color - vec3(0.5)) * contrast + vec3(0.5);\n"
		"	 vec3 color2 = vec3(dot(color1.rgb, luminanceWeighting));\n"
		"    gl_FragColor = vec4(mix(color2, color1.rgb, saturation), textureColor.w);;\n"
		"}\n";
	fshader->compileSourceCode(fsrc); //编译纹理着色器代码
	program.addShader(vshader); //添加顶点着色器    
	program.addShader(fshader); //添加纹理碎片着色器
	program.bindAttributeLocation("vertex", 0); //绑定顶点属性位置    
	program.bindAttributeLocation("texCoord", 1); //绑定纹理属性位置

	// 链接着色器管道  
	if (!program.link())
		close();
	// 绑定着色器管道
	if (!program.bind())
		close();
	setBrightness(0);
	setContrast(1);
	setSaturation(1);
}

void MFPOpenGLWidget::scale(float& scaleX, float& scaleY, QOpenGLTexture* texture) {
	const float W = width(), H = height(), w = texture->width(), h = texture->height();
	if (W / H > w / h) {
		scaleY = 0;
		scaleX = 0.5 - w * H / (2 * W * h);
	}
	else {
		scaleX = 0;
		scaleY = 0.5 - W * h / (2 * w * H);
	}
}

MFPOpenGLWidget::MFPOpenGLWidget(QWidget* parent): QOpenGLWidget(parent) {
	saturation = 1.0;
	contrast = 1.0;
	brightness = 0.0;
}

MFPOpenGLWidget::~MFPOpenGLWidget() { delete texture; }

void MFPOpenGLWidget::initializeGL() {
	initializeOpenGLFunctions(); //初始化OPenGL功能函数
	glClearColor(0, 0, 0, 0); //设置背景为黑色    
	glEnable(GL_TEXTURE_2D); //设置纹理2D功能可用
	initTextures(); //初始化纹理设置
	initShaders(); //初始化shaders
}

void MFPOpenGLWidget::paintGL() {
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT); //清除屏幕缓存和深度缓冲    
	// 计算纹理坐标的比例
	float scaleX = 0, scaleY = 0;
	scale(scaleX, scaleY, texture);
	QMatrix4x4 matrix;
	matrix.translate(0.0, 0.0, -5.0); // 矩阵变换

	// 重新设置纹理坐标
	texCoords.clear();
	texCoords.append(QVector2D(-scaleX, 1 + scaleY)); //左上
	texCoords.append(QVector2D(1 + scaleX, 1 + scaleY)); //右上    
	texCoords.append(QVector2D(-scaleX, -scaleY)); //左下
	texCoords.append(QVector2D(1 + scaleX, -scaleY)); //右下   
	program.enableAttributeArray(0);
	program.enableAttributeArray(1);
	program.setAttributeArray(0, vertices.constData());
	program.setAttributeArray(1, texCoords.constData());

	program.setUniformValue("texture", 0);
	program.setUniformValue("brightness", brightness);
	program.setUniformValue("contrast", contrast);
	program.setUniformValue("saturation", saturation);

	texture->setWrapMode(QOpenGLTexture::ClampToBorder);
	texture->bind(); //绑定纹理
	glDrawArrays(GL_TRIANGLE_STRIP, 0, 4); //绘制纹理
	texture->release(); //释放绑定的纹理
	texture->destroy(); //消耗底层的纹理对象
	texture->create(); //重新创建纹理对象
}

void MFPOpenGLWidget::resizeGL(int w, int h) {
	glViewport(0, 0, w, h);
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();

	// 设置正交投影矩阵
	glOrtho(0, w, 0, h, -1, 1);

	glMatrixMode(GL_MODELVIEW);
}

void MFPOpenGLWidget::setImage(const QImage& image) {
	texture->setData(image); //设置纹理图像
	//设置纹理细节
	texture->setLevelofDetailBias(-1); //值越小，图像越清晰
	texture->setSize(image.width(), image.height());

	//更新图像
	update();
}

void MFPOpenGLWidget::setContrast(const float v) { contrast = v; } //[0,2]

void MFPOpenGLWidget::setSaturation(const float v) { saturation = v; } //[0,2]

void MFPOpenGLWidget::setBrightness(const float v) { brightness = v; } //[-1,1]
