#ifndef STIMULUSPARAMS_H
#define STIMULUSPARAMS_H
#include <QMetaType>
#include <QString>

class StimulusParams
{
public:
    StimulusParams()
        :_dutyCycle(0)
        ,_frequency(0)
        ,_periodCount(0)
        ,_stimulusInterval(0)
        ,_stimulusCount(0)
        ,_deriction(0)
    {
        //初始化全为0
        memset(sendDataTmp, 0, 8);
    }

    //转换刺激参数
    void paramsTrans()
    {
        //第一个字节为占空比
        sendDataTmp[0] = UCHAR(_dutyCycle);
        //第2,3字节为频率
        //高八位存在第2个字节，低八位存在第3个字节
        sendDataTmp[1] = UCHAR(_frequency >> 8);
        sendDataTmp[2] = UCHAR(_frequency);
        //第4,5字节为周期个数
        //高八位存在第2个字节，低八位存在第3个字节
        sendDataTmp[3] = UCHAR(_periodCount >> 8);
        sendDataTmp[4] = UCHAR(_periodCount);
        //第6字节为两次刺激间的间隔时间
        sendDataTmp[5] = UCHAR(_stimulusInterval);
        //第7个字节为刺激次数
        sendDataTmp[6] = UCHAR(_stimulusCount);
        //第8个字节为左右选择控制,0为左，1为右
        sendDataTmp[7] = UCHAR(_direction);
    }

    quint32 _dutyCycle;
    quint32 _frequency;
    quint32 _periodCount;
    quint32 _stimulusInterval;
    quint32 _stimulusCount;
    quint32 _deriction;

    UCHAR sendDataTmp[8];


};

#endif // STIMULUSPARAMS_H
