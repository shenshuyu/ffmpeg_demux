#pragma once

struct AVFormatContext;
struct AVOutputFormat;
struct AVPacket;
struct AVStream;

typedef struct MuxParameter
{
	MuxParameter()
	{
		nSrcW = 0;
		nSrcH = 0;
		pFile = 0;
	}

	//image property
	int nSrcW;
	int nSrcH;

	//the file to save for...
	const char *pFile;

	//TODO other parameters...

}MuxParameter;


class FFmpegMuxer
{
public:
	FFmpegMuxer();
	virtual ~FFmpegMuxer();

public:
	virtual bool Init(MuxParameter *param);
	virtual bool PushData(unsigned char *pData, unsigned int nSize, int64_t pts, int64_t dts, bool bKeyFrame);
	virtual bool Finish();
	void SetExtraData(unsigned char *pData, unsigned int nSize);

protected:
	void ClearExtraData();

protected:
	unsigned char *m_pExtraData;
	unsigned int m_extraSize;

	AVFormatContext *m_avfctx;
	AVOutputFormat *m_av_outformat;
	AVPacket *m_outPacket;

};

