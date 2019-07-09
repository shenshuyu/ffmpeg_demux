#include <iostream>

#include "FFmpegDemuxer.h"
#include "FFmpegMuxer.h"

FFmpegDemuxer* g_ffmpegDemuxer = nullptr;
FFmpegMuxer* g_ffmpegMuxer = nullptr;
unsigned int g_dts = 0;

bool g_isStart = false;


int ReadStreamDataCallBack(AVPacket* pkt) {
    if(!g_isStart){
        MuxParameter parameter;
        parameter.nSrcW = g_ffmpegDemuxer->GetWidth();
        parameter.nSrcH = g_ffmpegDemuxer->GetHeight();
        parameter.pFile = "test.ts";
        g_ffmpegMuxer->Init(&parameter);
        unsigned char* ext_data = nullptr;
        int nsize = 0;
        g_ffmpegDemuxer->GetExtraData(&ext_data, &nsize);
//        g_ffmpegMuxer->SetExtraData(ext_data, nsize);

        g_isStart = true;
    }
   g_dts += 3600;
   g_ffmpegMuxer->PushData(pkt->data, pkt->size, g_dts, g_dts, pkt->flags == AV_PKT_FLAG_KEY);
}

int main(int argc, char** argv){
    g_ffmpegMuxer = new FFmpegMuxer;
    g_ffmpegDemuxer = new FFmpegDemuxer;

    g_ffmpegDemuxer->Init();
    g_ffmpegDemuxer->Start("rtsp://192.168.100.148/live/test01", ReadStreamDataCallBack);


    int number = 0;
    while (number <= 10) {
        av_usleep(1000*1000);
        number++;
    }
    g_ffmpegDemuxer->Stop();
    g_ffmpegMuxer->Finish();
    return 0;
}