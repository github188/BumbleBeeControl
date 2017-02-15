#ifndef CQTOPENCVVIEWERGL_H
#define CQTOPENCVVIEWERGL_H

#include <QOpenGLWidget>
#include <QOpenGLFunctions_3_0>
#include <opencv2/core/core.hpp>
#include <qDebug>

//#include "sharedvariable.h"


class CQtOpenCVViewerGl : public QOpenGLWidget, protected QOpenGLFunctions_3_0
{
	Q_OBJECT
public:
	explicit CQtOpenCVViewerGl(QWidget *parent = 0);
	~CQtOpenCVViewerGl();

signals:
	void    imageSizeChanged( int outW, int outH ); // Used to resize the image outside the widget

public slots:
	bool	showImage(const cv::Mat &frame);
	void	clearScene();

protected:
	void 	initializeGL();							// OpenGL initialization
	void 	paintGL();								// OpenGL Rendering
	void 	resizeGL(int width, int height);        // Widget Resize Event

	void    updateScene();
	void	getGLErr();

private:

	//QOpenGLTexture *texture;

	QColor  mBgColor;								// Background color

	cv::Mat renderFrame;							//renderFrame mat file

	float r, g, b;
	unsigned char* framePtr;

	unsigned int mFrameWidth;
	unsigned int mFrameHeight;
	unsigned int mRenderWidth;
	unsigned int mRenderHeight;
	unsigned int mRenderPosX;
	unsigned int mRenderPosY;
	
	GLuint texID;
	GLint texWidth, texHeight;

	void recalculatePosition();
};

#endif // CQTOPENCVVIEWERGL_H
