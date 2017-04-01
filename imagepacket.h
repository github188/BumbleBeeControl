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
        , stimulusParam({0,0,0,0,0,0})
		//pixFormat("")
	{
	}

	ImagePacket(const ImagePacket &src)
	{
		image = src.image.clone();
		seqNumber = src.seqNumber;
		timeStamp = src.timeStamp;
        stimulusParam = src.stimulusParam;
	}

	~ImagePacket()
	{
	}

	//stimulus parameters structure
	cv::Mat image;
    quint32 seqNumber;
	//timestamp double is msec
    double timeStamp;
    Configs::ParaConfig stimulusParam;
};
Q_DECLARE_METATYPE(ImagePacket);
