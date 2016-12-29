#ifndef TRACKANDANALYSIS_H
#define TRACKANDANALYSIS_H

#include <opencv2/opencv.hpp>
#include <opencv2/imgproc/imgproc.hpp>
#include <cmath>
#include <stdio.h>

#include "configs.h"

using namespace std;

class Trackandanalysis
{
public:
    Trackandanalysis();
    void init(cv::Mat &frame);
    void image_prepare();
    void calc_moment();
    void calc_centroid();
    void calc_major_principal_axis_angle();
    cv::Point get_centroid();
    cv::Moments get_moments();
    double get_major_pricipal_axis_angle();

private:
    static const int MIN_PIXEL_FOR_DETECTION = 50;
    static const int THRESH_BG = 30;
    static const int EXTRA_PIXEL = 40;
    static const int MAX_X = 1000, MAX_Y = 1000;
    double Pi;

    cv::Mat bgImg,bgImgROI,curImg,curImgROI,bgBlank;
    cv::Rect ROI;
    cv::Moments moment;//矩
    cv::Point centroid;//质心
    double mAxisTheta;//主轴角度(与X轴正方向）
};

#endif // TRACKANDANALYSIS_H
