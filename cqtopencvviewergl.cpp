#include "cqtopencvviewergl.h"

#include <QOpenGLFunctions_3_0>
#include <opencv2/opencv.hpp>

CQtOpenCVViewerGl::CQtOpenCVViewerGl(QWidget *parent) :
	QOpenGLWidget(parent),
	framePtr(NULL)
{
    mBgColor = QColor::fromRgb(150, 150, 150);
}

CQtOpenCVViewerGl::~CQtOpenCVViewerGl()
{
	makeCurrent();
	glDisable(GL_TEXTURE_2D);
	glDeleteTextures(1, &texID);
	
}

void CQtOpenCVViewerGl::getGLErr()
{
	for (GLenum err; (err = glGetError()) != GL_NO_ERROR;)
	{
		qDebug() << "gl error" << err;
	}
}

void CQtOpenCVViewerGl::clearScene()
{
	showImage(cv::Mat());
}

void CQtOpenCVViewerGl::initializeGL()
{
    initializeOpenGLFunctions();

    r = ((float)mBgColor.darker().red())/255.0f;
    g = ((float)mBgColor.darker().green())/255.0f;
    b = ((float)mBgColor.darker().blue())/255.0f;

	glClearColor(r, g, b, 1.0f);
		
	glDisable(GL_DEPTH_TEST);
	glEnable(GL_TEXTURE_2D);
	glGenTextures(1, &texID);
	glBindTexture(GL_TEXTURE_2D, texID);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texWidth);
	glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texHeight);
	
	
	getGLErr();
	
}

void CQtOpenCVViewerGl::resizeGL(int width, int height)
{

    glViewport(0, 0, (GLint)width, (GLint)height);

    glMatrixMode(GL_PROJECTION);
    glLoadIdentity();

    glOrtho(0, width, height, 0, 0, 1);

    glMatrixMode(GL_MODELVIEW);

    recalculatePosition();

    emit imageSizeChanged(mRenderWidth, mRenderHeight);

    updateScene();
}

void CQtOpenCVViewerGl::updateScene()
{
	if (this->isVisible())
	{
		update();
	}
}

void CQtOpenCVViewerGl::paintGL()
{
	glEnable(GL_TEXTURE_2D);
	glBindTexture(GL_TEXTURE_2D, texID);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glDisable(GL_DEPTH_TEST);
	glLoadIdentity();

	if (!renderFrame.empty())
	{
		if (texWidth == 0 || texHeight == 0)
		{
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, mFrameWidth, mFrameHeight, 0, GL_BGR, GL_UNSIGNED_BYTE, NULL);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_WIDTH, &texWidth);
			glGetTexLevelParameteriv(GL_TEXTURE_2D, 0, GL_TEXTURE_HEIGHT, &texHeight);
			getGLErr();
		}
		else
		{
			recalculatePosition();
			//use glTexImage2D to draw qimage
			glBindTexture(GL_TEXTURE_2D, texID);
			glTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, mFrameWidth, mFrameHeight,
				GL_BGR, GL_UNSIGNED_BYTE, framePtr);
			//start draw
			glBegin(GL_QUADS);
			glTexCoord2f(0, 0); glVertex2f(mRenderPosX, mRenderPosY);
			glTexCoord2f(1, 0); glVertex2f(mRenderPosX + mRenderWidth, mRenderPosY);
			glTexCoord2f(1, 1); glVertex2f(mRenderPosX + mRenderWidth, mRenderPosY + mRenderHeight);
			glTexCoord2f(0, 1); glVertex2f(mRenderPosX, mRenderPosY + mRenderHeight);
			glEnd();
			glFlush();
			//end
			//get opengl error info
			getGLErr();
		}
	}
}

void CQtOpenCVViewerGl::recalculatePosition()
{
	QSize frameSize(mFrameWidth, mFrameHeight);
	auto scaled = frameSize.scaled(this->size(), Qt::KeepAspectRatio);

    mRenderHeight = scaled.height();
	mRenderWidth = scaled.width();

    mRenderPosX = floor((this->size().width() - mRenderWidth) / 2);
    mRenderPosY = floor((this->size().height() - mRenderHeight) / 2);

}

bool CQtOpenCVViewerGl::showImage(const cv::Mat &frame)
{

	//recalculatePosition();
	//sharedvariable::mutex.lock();
	renderFrame = frame.clone();
	//sharedvariable::mutex.unlock();

	mFrameWidth = renderFrame.cols;
	mFrameHeight = renderFrame.rows;
	framePtr = renderFrame.data;

	updateScene();

	return true;
}