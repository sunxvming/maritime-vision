#ifndef SAVEHELPER_H
#define SAVEHELPER_H

#include "saveinclude.h"

class SaveHelper
{
public:
    //pcm文件转wav文件
    static void pcmToWav(const QString &pcmFile, const QString &wavFile, int sampleRate, int channelCount, bool deleteFile = true);
    //aac文件采样率下标
    static int getSamplingFrequencyIndex(int sampleRate);
    //aac文件添加adts头
    static void adtsHeader(char *header, int len, int sampleRate, int channelCount, int profile);
};

#endif // SAVEHELPER_H
