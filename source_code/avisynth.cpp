
#include <windows.h>
#include "DibHelper.h"
#include <assert.h>

typedef struct {
  PAVIFILE file;
  AVISynthStream *streams;
  int nb_streams;
  int next_stream;
} AVISynthContext;

// make them globals for now...
AVISynthContext *avs;
AVISynthStream *stream;
WAVEFORMATEX wvfmt;
BITMAPINFO savedVideoFormat;

#define av_log(a,b,c,d) LocalOutput(c,d)

void *av_mallocz(size_t size) {
	void *a = malloc(size);
	memset(a, 0, size);
	return a;
}

int avisynth_read_header(){
  avs = (AVISynthContext *) av_mallocz(sizeof(AVISynthContext));
  HRESULT res;
  AVIFILEINFO info;
  DWORD id;


  AVIFileInit();

  
  res = AVIFileOpen(&avs->file, read_config_filepath(), OF_READ|OF_SHARE_DENY_WRITE, NULL);
  if (res != S_OK)
    {
      av_log(s, AV_LOG_ERROR, "AVIFileOpen failed with error %ld", res);
      AVIFileExit();
      return -1;
    }

  res = AVIFileInfo(avs->file, &info, sizeof(info));
  if (res != S_OK)
    {
      av_log(s, AV_LOG_ERROR, "AVIFileInfo failed with error %ld", res);
      AVIFileExit();
      return -1;
    }

  avs->streams = (AVISynthStream *) av_mallocz(info.dwStreams * sizeof(AVISynthStream));
  assert(info.dwStreams == 1);
  for (id=0; id<info.dwStreams; id++)
    {
      stream = &avs->streams[id];
      stream->read = 0;
      if (AVIFileGetStream(avs->file, &stream->handle, 0, id) == S_OK)
        {
          if (AVIStreamInfo(stream->handle, &stream->info, sizeof(stream->info)) == S_OK)
            {
              if (stream->info.fccType == streamtypeAUDIO)
                {

			      assert(false); // don't do audio yet
                  LONG struct_size = sizeof(WAVEFORMATEX);
                  if (AVIStreamReadFormat(stream->handle, 0, &wvfmt, &struct_size) != S_OK)
                    continue;

				  /* audio:
                  st = avformat_new_stream(s, NULL);
                  st->id = id;
                  st->codec->codec_type = AVMEDIA_TYPE_AUDIO;

                  st->codec->block_align = wvfmt.nBlockAlign;
                  st->codec->channels = wvfmt.nChannels;
                  st->codec->sample_rate = wvfmt.nSamplesPerSec;
                  st->codec->bit_rate = wvfmt.nAvgBytesPerSec * 8;
                  st->codec->bits_per_coded_sample = wvfmt.wBitsPerSample;


                  st->codec->codec_tag = wvfmt.wFormatTag;
                  st->codec->codec_id = ff_wav_codec_get_id(wvfmt.wFormatTag, st->codec->bits_per_coded_sample);
				  */
                  stream->chunck_samples = wvfmt.nSamplesPerSec * (__int64)info.dwScale / (__int64) info.dwRate;
                  stream->chunck_size = stream->chunck_samples * wvfmt.nChannels * wvfmt.wBitsPerSample / 8;
                }
              else if (stream->info.fccType == streamtypeVIDEO)
                {

                  LONG struct_size = sizeof(BITMAPINFO);

                  stream->chunck_size = stream->info.dwSampleSize;
                  stream->chunck_samples = 1;

                  if (AVIStreamReadFormat(stream->handle, 0, &savedVideoFormat, &struct_size) != S_OK)
                    continue;

				  /*

				  stream->info.dwRate is numerator
				  stream->info.dwScale is denominator [?]
				  savedVideoFormat.bmiHeader.biWidth

				  */
                  /*st = avformat_new_stream(s, NULL);
                  st->id = id;
                  st->codec->codec_type = AVMEDIA_TYPE_VIDEO;
                  st->r_frame_rate.num = stream->info.dwRate;
                  st->r_frame_rate.den = stream->info.dwScale;

                  st->codec->width = savedVideoFormat.bmiHeader.biWidth;
                  st->codec->height = savedVideoFormat.bmiHeader.biHeight;

                  st->codec->bits_per_coded_sample = savedVideoFormat.bmiHeader.biBitCount;
                  st->codec->bit_rate = (uint64_t)stream->info.dwSampleSize * (uint64_t)stream->info.dwRate * 8 / (uint64_t)stream->info.dwScale;
                  st->codec->codec_tag = savedVideoFormat.bmiHeader.biCompression;
                  st->codec->codec_id = ff_codec_get_id(ff_codec_bmp_tags, savedVideoFormat.bmiHeader.biCompression);
                  if (st->codec->codec_id == CODEC_ID_RAWVIDEO && savedVideoFormat.bmiHeader.biCompression== BI_RGB) {
                    st->codec->extradata = av_malloc(9 + FF_INPUT_BUFFER_PADDING_SIZE);
                    if (st->codec->extradata) {
                      st->codec->extradata_size = 9;
                      memcpy(st->codec->extradata, "BottomUp", 9);
                    }
                  }


                  st->duration = stream->info.dwLength;
				  */
                }
              else
                {
                  AVIStreamRelease(stream->handle);
                  continue;
                }

              avs->nb_streams++;

             // st->codec->stream_codec_tag = stream->info.fccHandler;

              //avpriv_set_pts_info(st, 64, info.dwScale, info.dwRate);
              //st->start_time = stream->info.dwStart; // wow what is the dshow equivalent? hmm...
            }
        }
    }

  return 0;
}

int avisynth_read_packet(BYTE *pData, LONG pDataSize)
{
  HRESULT res;
  AVISynthStream *stream;
  int stream_id = avs->next_stream;
  assert(stream_id == 0);
  LONG read_size;

  // handle interleaving manually...
  stream = &avs->streams[stream_id];

  // read is the pts? that always increases...
  //if (stream->read >= stream->info.dwLength)
  //  return -1;

  // chunck_size is an avisynth thing. whoa!
  if(pDataSize < stream->chunck_size)
	 assert(false);

  // guess we have our own concept of stream_index, maybe?
  // pkt->stream_index = stream_id;
  // is this like a pts integer or something?
  __int64 pts = avs->streams[stream_id].read / avs->streams[stream_id].chunck_samples;

  // I think this will block until data is available, or 0 for EOF [?]
  res = AVIStreamRead(stream->handle, stream->read, stream->chunck_samples, pData, stream->chunck_size, &read_size, NULL);

  __int64 size = read_size;
  
  assert(pDataSize >= read_size);

  stream->read += stream->chunck_samples;

  // I guess with avi you're just supposed to read one stream, then the next, forever? huh?

  /*
  // prepare for the next stream to read
  do {
    avs->next_stream = (avs->next_stream+1) % avs->nb_streams;
  } while (avs->next_stream != stream_id && s->streams[avs->next_stream]->discard >= AVDISCARD_ALL);
  */
  return (res == S_OK) ? read_size : -1;
}

void avisynth_read_close()
{

  int i;

  for (i=0;i<avs->nb_streams;i++)
    {
      AVIStreamRelease(avs->streams[i].handle);
    }

  free(avs->streams);
  AVIFileRelease(avs->file);
  AVIFileExit();
  free(avs);
}

// unused, for now...
int avisynth_read_seek(int stream_index, DWORD pts, int flags)
{

  int stream_id;

  for (stream_id = 0; stream_id < avs->nb_streams; stream_id++)
    {
      avs->streams[stream_id].read = pts * avs->streams[stream_id].chunck_samples;
    }

  return 0;
}