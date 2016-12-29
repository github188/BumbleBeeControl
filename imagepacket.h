#include <QMetaType>
#include <QString>

#include <opencv2/opencv.hpp>
#include "configs.h"
#pragma once

class ImagePacket
{
public:

	ImagePacket()
		: image(cv::Mat())
		, seqNumber(0)
		, stimulusPara({0,0,0,0})
		//pixFormat("")
	{
	}

	ImagePacket(const ImagePacket &src)
	{
		image = src.image.clone();
		seqNumber = src.seqNumber;
		timeStamp = src.timeStamp;
		stimulusPara = src.stimulusPara;
	}

	~ImagePacket()
	{
	}

	//stimulus parameters structure
	cv::Mat image;
    quint32 seqNumber;
	//timestamp double is msec
    double timeStamp;
	Configs::ParaConfig stimulusPara;
};
Q_DECLARE_METATYPE(ImagePacket);
