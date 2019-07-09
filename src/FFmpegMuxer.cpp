#include <string.h>
#include <stdint.h>
#include "FFmpegMuxer.h"

#ifdef __cplusplus
extern "C" {
#endif//#ifdef __cplusplus

#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavutil/opt.h"

#ifdef __cplusplus
}
#endif//#ifdef __cplusplus

FFmpegMuxer::FFmpegMuxer():
		m_pExtraData(0),
		m_extraSize(0),
		m_avfctx(0),
		m_av_outformat(0),
		m_outPacket(0)
{
}

FFmpegMuxer::~FFmpegMuxer()
{
	ClearExtraData();

	if (m_avfctx)
	{
		if (m_avfctx->pb)
		{
			avio_close(m_avfctx->pb);
			m_avfctx->pb = NULL;
		}

		if (m_avfctx->metadata)
		{
			av_dict_free(&m_avfctx->metadata);
			m_avfctx->metadata = NULL;
		}

		av_free(m_avfctx);
		m_avfctx = NULL;
	}
}

bool FFmpegMuxer::Init(MuxParameter *param)
{
	m_avfctx = avformat_alloc_context();
	if (m_avfctx == 0)
	{
		return false;
	}

	m_av_outformat = av_guess_format(0, param->pFile, 0);

	if (m_av_outformat == 0)
	{
		return false;
	}

	m_avfctx->oformat = m_av_outformat;
	AVStream* av = avformat_new_stream(m_avfctx, NULL);
	av->codecpar->bits_per_coded_sample = 0;
	av->codecpar->codec_id = AV_CODEC_ID_H264;
	av->codecpar->codec_tag = av_codec_get_tag(m_avfctx->oformat->codec_tag, AV_CODEC_ID_H264);
	av->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;
	av->codecpar->extradata_size = m_extraSize;
	av->codecpar->extradata = (unsigned char*)av_malloc(av->codec->extradata_size);
	memcpy(av->codecpar->extradata , m_pExtraData , m_extraSize);
	av->codecpar->format = AV_PIX_FMT_YUV420P;
	av->codecpar->height = param->nSrcH;
	av->codecpar->width = param->nSrcW;
	av->codecpar->bit_rate = 5000000;
	av->codec->time_base.den = 25;
	av->codec->time_base.num = 1;
	av->id = 0;

	if (m_av_outformat->flags & AVFMT_GLOBALHEADER)
	{
		av->codec->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
	}

	memcpy(m_avfctx->filename, param->pFile, strlen(param->pFile));
	if (avio_open2(&m_avfctx->pb, m_avfctx->filename, AVIO_FLAG_WRITE, NULL, NULL) < 0)
	{
		return false;
	}

	AVDictionary * av_opts = NULL;
	av_dict_set(&av_opts, "movflags", "faststart", 0);
	int ret = avformat_write_header(m_avfctx, &av_opts);
	av_dict_free(&av_opts);
	if (ret < 0)
	{
		return false;
	}

	m_outPacket = av_packet_alloc();

	return true;
}

bool FFmpegMuxer::PushData(unsigned char *pData, unsigned int nSize, int64_t pts, int64_t dts, bool bKeyFrame)
{
	av_init_packet(m_outPacket);
	m_outPacket->stream_index = 0;
	m_outPacket->size = nSize;
	m_outPacket->data = pData;
	m_outPacket->pts = pts;
	m_outPacket->dts = dts;

	if (bKeyFrame)
	{
		m_outPacket->flags = AV_PKT_FLAG_KEY;
	}

	int nRet = av_write_frame(m_avfctx , m_outPacket);

	return true;
}

bool FFmpegMuxer::Finish()
{
	av_write_trailer(m_avfctx);
	if (m_outPacket)
	{
		av_packet_free(&m_outPacket);
		m_outPacket = 0;
	}
	return true;
}

void FFmpegMuxer::SetExtraData(unsigned char *pData, unsigned int nSize)
{
	if (pData == 0 || nSize == 0)
	{
		return;
	}

	ClearExtraData();

	m_pExtraData = new unsigned char[nSize];
	m_extraSize = nSize;
	memcpy(m_pExtraData, pData, nSize);
}

void FFmpegMuxer::ClearExtraData()
{
	if (m_pExtraData)
	{
		delete[] m_pExtraData;
		m_pExtraData = 0;
		m_extraSize = 0;
	}
}

