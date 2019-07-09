//
// Created by youle on 18-10-8.
//

#ifndef FFMPEGMUXER_FFMPEGDEMUXER_H
#define FFMPEGMUXER_FFMPEGDEMUXER_H

#include <iostream>
#include <thread>
#include <string>

extern "C" {
#include <libavformat/avformat.h>
#include <libavcodec/avcodec.h>
#include <libavutil/avutil.h>
#include <libavutil/time.h>
}

typedef int (*ReadStreamDataCB)(AVPacket* pkt);

class FFmpegDemuxer {
public:
    FFmpegDemuxer();
    virtual ~FFmpegDemuxer();

    static int32_t AVInterruptCallBackFun(void *param);

    virtual bool Init();
    virtual bool Start(const char* url, ReadStreamDataCB callback);
    virtual bool Stop();
    void GetExtraData(unsigned char **pData, int* nSize);
    int GetWidth();
    int GetHeight();

private:
    void RreadStreamData();

private:
    static bool is_init_;

    bool is_running_ = false;
    bool quit_block_ = false;
    int video_stream_idx_ = -1;
    ReadStreamDataCB callback_;
    std::string url_;
    AVFormatContext *fmt_ctx_;
    AVStream *video_stream_;
    AVCodecContext *video_dec_ctx_;
    AVBitStreamFilterContext* avbsfc_;
    AVPacket pkt_;

    std::thread* t_;
};


#endif //FFMPEGMUXER_FFMPEGDEMUXER_H
