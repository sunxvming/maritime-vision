#include "savehelper.h"

void SaveHelper::pcmToWav(const QString &pcmFile, const QString &wavFile, int sampleRate, int channelCount, bool deleteFile)
{
    //wav音频文件固定头部字节(数据有顺序要求)
    struct WaveFileHeader {
        //RIFF头
        char riffName[4];
        quint32 riffLen;

        //数据类型标识符
        char wavName[4];

        //格式块中的块头
        char fmtName[4];
        quint32 fmtLen;

        //音频编码格式
        quint16 audioFormat;
        //通道数量
        quint16 numChannels;
        //采样率
        quint32 sampleRate;
        //波形数据传输速率
        quint32 bytesPerSecond;
        //数据块对齐单位
        quint16 bytesPerSample;
        //每次采样得到的样本数据位数
        quint16 bitsPerSample;

        //数据块中的块头
        char dataName[4];
        quint32 dataLen;
    };

    WaveFileHeader header;
    qstrcpy(header.riffName, "RIFF");
    qstrcpy(header.wavName, "WAVE");
    qstrcpy(header.fmtName, "fmt ");
    qstrcpy(header.dataName, "data");

    header.fmtLen = 16;
    header.audioFormat = 1;
    header.numChannels = channelCount;
    header.sampleRate = sampleRate;
    header.bytesPerSecond = channelCount * sampleRate;
    header.bytesPerSample = 2;
    header.bitsPerSample = 16;

    QFile filePcm(pcmFile);
    QFile fileWav(wavFile);
    if (!filePcm.open(QIODevice::ReadOnly) || !fileWav.open(QIODevice::WriteOnly)) {
        return;
    }

    //计算对应的长度大小
    int sizeHeader = sizeof(header);
    quint32 sizeData = filePcm.bytesAvailable();
    header.riffLen = (sizeData - 8 + sizeHeader);
    header.dataLen = sizeData;

    //先写入头部信息
    fileWav.write((const char *)&header, sizeHeader);
    //再写入音频数据
    fileWav.write(filePcm.readAll());

    //关闭文件
    filePcm.close();
    fileWav.close();

    //删除文件
    if (deleteFile) {
        QFile(pcmFile).remove();
        qDebug() << TIMEMS << QString("删除文件 -> 文件: %1").arg(pcmFile);
    }
}

int SaveHelper::getSamplingFrequencyIndex(int sampleRate)
{
    int freqIdx = 3;
    if (sampleRate == 96000) {
        freqIdx = 0;
    } else if (sampleRate == 88200) {
        freqIdx = 1;
    } else if (sampleRate == 64000) {
        freqIdx = 2;
    } else if (sampleRate == 48000) {
        freqIdx = 3;
    } else if (sampleRate == 44100) {
        freqIdx = 4;
    } else if (sampleRate == 32000) {
        freqIdx = 5;
    } else if (sampleRate == 24000) {
        freqIdx = 6;
    } else if (sampleRate == 22050) {
        freqIdx = 7;
    } else if (sampleRate == 16000) {
        freqIdx = 8;
    } else if (sampleRate == 12000) {
        freqIdx = 9;
    } else if (sampleRate == 11025) {
        freqIdx = 10;
    } else if (sampleRate == 8000) {
        freqIdx = 11;
    }

    return freqIdx;
}

void SaveHelper::adtsHeader(char *header, int len, int sampleRate, int channelCount, int profile)
{
    //抽取音频命令 ffmpeg -i d:/1.mp4 -vn -y -acodec copy d:/1.aac
    //音频adts头部数据 https://blog.csdn.net/u013113678/article/details/123134860
    int chanCfg = channelCount;
    int freqIdx = getSamplingFrequencyIndex(sampleRate);
    int adtsLen = len + 7;

    //绝大部分音频都是1或者-99未设置(有部分是4表示高压缩率)
    //网上的算法缺少下面这个计算导致部分文件保存的音频文件不正常
    if (profile > 1) {
        freqIdx += (profile - 1);
    }
    profile = 1;

#if 1
    header[0] = (char)0xff;
    header[1] = (char)0xf1;
    header[2] = (char)(((profile) << 6) + (freqIdx << 2) + (chanCfg >> 2));
    header[6] = (char)0xfc;

    header[3] = (char)(((2 & 3) << 6) + (adtsLen >> 11));
    header[4] = (char)((adtsLen & 0x7f8) >> 3);
    header[5] = (char)(((adtsLen & 0x7) << 5) + 0x1f);
#else
    header[0] = 0xff;
    header[1] = 0xf0;
    header[1] |= (0 << 3);
    header[1] |= (0 << 1);
    header[1] |= 1;

    header[2] = (profile) << 6;
    header[2] |= (freqIdx & 0x0f) << 2;
    header[2] |= (0 << 1);
    header[2] |= (chanCfg & 0x04) >> 2;

    header[3] = (chanCfg & 0x03) << 6;
    header[3] |= (0 << 5);
    header[3] |= (0 << 4);
    header[3] |= (0 << 3);
    header[3] |= (0 << 2);
    header[3] |= ((adtsLen & 0x1800) >> 11);

    header[4] = (uint8_t)((adtsLen & 0x7f8) >> 3);
    header[5] = (uint8_t)((adtsLen & 0x7) << 5);
    header[5] |= 0x1f;
    header[6] = 0xfc;
#endif
}
