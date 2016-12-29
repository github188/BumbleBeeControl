#include "Trackandanalysis.h"
#include <QDebug>

Trackandanalysis::Trackandanalysis()
{

}

void Trackandanalysis::init(cv::Mat &frame)
{
    if(global::status.frameId == 1)
    {

        Pi = acos(-1.0);
        //bgImg用于背景差分
        bgImg = frame.clone();
        curImg = frame;
        cv::cvtColor(bgImg,bgImg,CV_BGR2GRAY);
        //生成全黑背景图
        bgBlank = cv::Mat::zeros(bgImg.rows,bgImg.cols,bgImg.type());
        //初始化ROI为全图范围
        ROI = cv::Rect(0, 0, MAX_X, MAX_Y);
    }
    else
    {
        return;
    }
}
void Trackandanalysis::image_prepare()
{
    //读取最新的画面
    cv::Mat img = curImg.clone();
    cv::cvtColor(img, img, CV_BGR2GRAY);
    cv::GaussianBlur(img, img, cv::Size(5, 5), 0.5);
    cv::dilate(img, img, 10);
    cv::erode(img, img, cv::Mat());
    curImgROI = img(ROI);
    bgImgROI = bgImg(ROI);

    cv::Point roi_tl,roi_br;

    int nc,nr;
    int x_sum, y_sum,x_min,x_max,y_min,y_max,x,y;
    int roi_height, roi_width;
    int point_count;

    nc = ROI.width;
    nr = ROI.height;
    roi_tl = ROI.tl();
    roi_br = ROI.br();
    x_sum = 0;
    y_sum = 0;
    x_min = roi_br.x;
    x_max = roi_tl.x;
    y_min = roi_br.y;
    y_max = roi_tl.y;
    point_count = 0;

    //bg=background,cur=current

    curImgROI -= bgImgROI;
    //二值化
    cv::threshold(curImgROI,curImgROI,THRESH_BG,255,cv::THRESH_BINARY);
    cv::erode(curImgROI, curImgROI, cv::Mat());
    //拷贝到黑色背景上
    curImgROI.copyTo(bgBlank(ROI));
    //确定新的ROI范围
    if(curImgROI.isContinuous())
    {
        nc = nc * nr;
        nr = 1;
    }
    for (int j = 0; j < nr; ++j)
    {
        uchar * cur_data = curImgROI.ptr<uchar>(j);
        for (int i = 0; i < nc; ++i)
        {
            if (cur_data[i] > 0)
            {
                if(nr != 1)
                {
                    x = i + roi_tl.x;
                    y = j + roi_tl.y;
                }
                else
                {
                    x = i % ROI.height + roi_tl.x;
                    y = i / ROI.height + roi_tl.y;
                }

                x_min = x_min < x ? x_min : x;
                x_max = x_max > x ? x_max : x;
                y_min = y_min < y ? y_min : y;
                y_max = y_max > y ? y_max : y;
            }
        }
    }
    //如果面积小于给定的最小检测面积(通过包含点数来计算），则忽略该部分重新全图检测。
    if (moment.m00 < MIN_PIXEL_FOR_DETECTION)
    {
        ROI = cv::Rect(0, 0, MAX_X, MAX_Y);
        printf("Reset ROI\n");
        return;
    }
    else
    {
        //ROI想四周扩张EXTRA_PIXEL范围
        x_min -= EXTRA_PIXEL;
        y_min -= EXTRA_PIXEL;
        x_max += EXTRA_PIXEL;
        y_max += EXTRA_PIXEL;
        //ensure the rectangle tl and br point in the frame
        x_min = x_min > 0 ? x_min : 0;
        y_min = y_min > 0 ? y_min : 0;
        x_max = x_max < MAX_X ? x_max : MAX_X;
        y_max = y_max < MAX_Y ? y_max : MAX_Y;

        //create new ROI
        roi_width = x_max - x_min;
        roi_height = y_max - y_min;
        ROI = cv::Rect(x_min, y_min, roi_width, roi_height);
    }
}

void Trackandanalysis::calc_moment()
{
    image_prepare();
    //calculate moments
    moment = moments(bgBlank,true);
}

cv::Moments Trackandanalysis::get_moments()
{
    return moment;
}

void Trackandanalysis::calc_centroid()
{
    calc_moment();
    //center point of the rat
    int x_cm = moment.m10/moment.m00;
    int y_cm = moment.m01/moment.m00;
    centroid = cv::Point(x_cm, y_cm);
}

cv::Point Trackandanalysis::get_centroid()
{
    return centroid;
}


/*
    Return the major principal axis angle of the object in rad
*/
void Trackandanalysis::calc_major_principal_axis_angle()
{
    calc_moment();

    long numerator, denominator;
    double theta;
    numerator = moment.mu11;
    denominator = moment.mu20 - moment.mu02;
    /*
        the angle theta is calculated as follow
        |	numerator	|	denominator	|		theta		|
        |		0		|		-		|		Pi/2		|
        |		+		|		-		|		Pi/2+Phi	|
        |		+		|		0		|		Pi/4		|
        |		+		|		+		|		Phi			|
        |		0		|		0		|		0			|
        |		-		|		+		|		Phi			|
        |		-		|		0		|		-Pi/4		|
        |		-		|		-		|		Phi-Pi/2	|

        The Phi = 0.5 * atan(2 * numerator / denominator)
        */
    if (denominator == 0)
    {
        if (numerator == 0)
        {
            theta = 0;

        }
        else if (numerator > 0)
        {
            theta = Pi / 4;
        }
        else
        {
            theta = -1 * Pi / 4;
        }
    }
    else if (denominator >0)
    {
        theta = 0.5 * atan(2 * numerator / denominator);

    }
    else
    {
        if (numerator == 0)
        {
            theta = Pi / 2;
        }
        else if (numerator >0)
        {
            theta = Pi / 2 + 0.5 * atan(2 * numerator / denominator);
        }
        else
        {
            theta = 0.5 * atan(2 * numerator / denominator) - Pi / 2;
        }
    }
    mAxisTheta = theta;
}

double Trackandanalysis::get_major_pricipal_axis_angle()
{
    return mAxisTheta;
}
