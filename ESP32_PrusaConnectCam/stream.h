/**
   @file stream.h

   @brief stream library

   Inspiration is from git repository https://gist.github.com/me-no-dev/d34fba51a8f059ac559bf62002e61aa3

   @author Miroslav Pivovarsky
   Contact: miroslav.pivovarsky@gmail.com

   @bug: no know bug
*/

#ifndef _STREAM_H_
#define _STREAM_H_

#include "ESPAsyncWebSrv.h"

#include "Arduino.h"
#include "mcu_cfg.h"
#include "var.h"
#include "log.h"
#include "camera.h"

typedef struct {
  camera_fb_t *fb;  ///< pointer to frame buffer
  size_t index;     ///< index of frame
} camera_frame_t;   ///< camera frame structure

class AsyncBufferResponse : public AsyncAbstractResponse {
private:
  uint8_t *_buf;    ///< pointer to frame buffer
  size_t _len;      ///< length of frame buffer
  size_t _index;    ///< index of frame 

public:
  AsyncBufferResponse(uint8_t *, size_t, const char *);
  ~AsyncBufferResponse();
  bool _sourceValid() const;
  virtual size_t _fillBuffer(uint8_t *, size_t) override;
  size_t _content(uint8_t *, size_t, size_t);
};

class AsyncFrameResponse : public AsyncAbstractResponse {
private:
  camera_fb_t *fb;    ///< pointer to frame buffer
  size_t _index;      ///< index of frame 

public:
  AsyncFrameResponse(camera_fb_t *, const char *);
  ~AsyncFrameResponse();
  bool _sourceValid() const;
  virtual size_t _fillBuffer(uint8_t *, size_t) override;
  size_t _content(uint8_t *, size_t, size_t);
};

class AsyncJpegStreamResponse : public AsyncAbstractResponse {
private:
  camera_frame_t _frame;      ///< camera frame
  size_t _index;              ///< index of frame
  uint64_t lastAsyncRequest;  ///< last async request
  Camera *camera;             ///< pointer to camera
  Logs *log;                  ///< pointer to logs
  camera_fb_t _dframe;        ///< camera frame

public:
  AsyncJpegStreamResponse(Camera *, Logs *);
  ~AsyncJpegStreamResponse();
  bool _sourceValid() const;
  virtual size_t _fillBuffer(uint8_t *, size_t ) override;
  size_t _content(uint8_t *, size_t , size_t );
};

#endif
/* EOF */