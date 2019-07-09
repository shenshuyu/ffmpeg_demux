//
// Created by youle on 18-10-8.
//

#include <sstream>
#include "FFmpegDemuxer.h"

int32_t FFmpegDemuxer::AVInterruptCallBackFun(void *param)
{
    FFmpegDemuxer* pFFmpeg = static_cast<FFmpegDemuxer*>(param);
    if(pFFmpeg->quit_block_){
        //通知FFMpeg可以从阻塞工作线程中释放操作
        printf("AVInterruptCallBackFun\n");
        return 1;
    }
    //通知FFMpeg继续阻塞工作
    return 0;
}

FFmpegDemuxer::FFmpegDemuxer() {}

FFmpegDemuxer::~FFmpegDemuxer() {}

bool FFmpegDemuxer::Init() {
    av_register_all();
    int ret = avformat_network_init();
    if (ret < 0)
        return ret;
}

bool FFmpegDemuxer::Start(const char* url, ReadStreamDataCB callback) {
    int32_t ret = -1;
    AVDictionary *opts = nullptr;

    if (0 == strncmp(url, "rtsp://", 7)) {
        av_dict_set(&opts, "rtsp_transport", "tcp", 0);
        av_dict_set(&opts, "stimeout", std::to_string(10).data(), 0);
        av_dict_set(&opts, "buffer_size", "1024000", 0); //设置缓存大小，1080p可将值调大
        av_dict_set_int(&opts, "max_delay", 30, 0);
        //av_dict_set(&opts, "protocol_whitelist", "file,crypto,udp,rtp,sdp", 0); //设置缓存大小，1080p可将值调大
    }


    this->fmt_ctx_ = avformat_alloc_context();
    this->fmt_ctx_->interrupt_callback.callback = AVInterruptCallBackFun;
    this->fmt_ctx_->interrupt_callback.opaque = this;
    this->quit_block_ = false;
    this->url_ = url;
    this->callback_ = callback;

    std::stringstream ss;
    ret = avformat_open_input(&fmt_ctx_, this->url_.data(), 0, &opts);
    if (ret < 0) {
        char errmsg[1024] = {0};
        av_strerror(ret, errmsg, 1023);
        ss << "[FFMPEG] Open error! ret=" << ret << " url=" << url << " libavformat errmsg=" << errmsg;
        std::cout << ss.str() << std::endl;
        return -1;
    }

    ret = avformat_find_stream_info(fmt_ctx_, nullptr);
    if (ret < 0) {
        char errmsg[1024] = {0};
        av_strerror(ret, errmsg, 1023);
        ss << "[FFMPEG] find stream error! ret=" << ret << " url=" << url << " libavformat errmsg=" << errmsg;
        std::cout << ss.str() << std::endl;
        return -2;
    }

    ret = av_find_best_stream(this->fmt_ctx_, AVMEDIA_TYPE_VIDEO, -1, -1, 0, 0);
    if (ret < 0) {
        char errmsg[1024] = {0};
        av_strerror(ret, errmsg, 1023);
        ss << "[FFMPEG] find stream error! ret=" << ret << " url=" << url << " libavformat errmsg=" << errmsg;
        std::cout << ss.str() << std::endl;
        return -2;
    }
    else {
        this->video_stream_idx_ = ret;
        this->video_stream_ = this->fmt_ctx_->streams[this->video_stream_idx_];
    }

    video_stream_ = fmt_ctx_->streams[video_stream_idx_];
    this->video_dec_ctx_ = this->video_stream_->codec;

    av_dump_format(this->fmt_ctx_, 0, this->url_.data(), 0);

//    if (strcmp(this->fmt_ctx_->iformat->name, "h264") == 0 ||
//        strcmp(this->fmt_ctx_->iformat->name, "hevc") == 0 ||
//        strcmp(this->fmt_ctx_->iformat->name, "mpeg") == 0) {
//    } else {
//    }

    if(nullptr == this->video_dec_ctx_){
        ss << "[FFMPEG] video decode ctx is nullptr" << " url=" << url;
        std::cout << ss.str() << std::endl;
        return -2;
    }

    av_init_packet(&this->pkt_);
    this->pkt_.data = 0;
    this->pkt_.size = 0;

    if(nullptr != opts) {
        av_dict_free(&opts);
    }


    t_ = new std::thread(std::bind(&FFmpegDemuxer::RreadStreamData, this));

    ret = 0;
    return ret;

}

bool FFmpegDemuxer::Stop() {
    this->is_running_ = false;
    if(t_->joinable()){
        t_->join();
        delete t_;
        t_ = nullptr;
    }
    avformat_close_input(&fmt_ctx_);
}

void FFmpegDemuxer::RreadStreamData() {
    char errmsg[1024] = {0};
    this->is_running_ = true;
    while (this->is_running_) {
        int32_t ret = av_read_frame(this->fmt_ctx_, &this->pkt_);
        if (ret == AVERROR(EAGAIN)) {
            av_usleep(10000);
            continue;
        }
        if (ret < 0) {
            av_strerror(ret, errmsg, 1023);
            printf("url: %s is read error: %s\n", this->url_.data(), errmsg);
            this->is_running_ = false;
            break;
        }

        if (this->video_stream_idx_ != this->pkt_.stream_index) {
            av_packet_unref(&this->pkt_);
            continue;
        }
        this->callback_(&this->pkt_);
        av_packet_unref(&this->pkt_);
    }
    return;
}

void FFmpegDemuxer::GetExtraData(unsigned char **pData, int* nSize) {
    *pData = this->video_stream_->codecpar->extradata;
    *nSize = this->video_stream_->codecpar->extradata_size;
    return;
}

int FFmpegDemuxer::GetWidth() {
   return this->video_stream_->codecpar->width;
}

int FFmpegDemuxer::GetHeight() {
    return this->video_stream_->codecpar->height;

}

