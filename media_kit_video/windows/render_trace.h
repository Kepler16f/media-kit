// PiliPlus4WOA render trace (D3D11 renderer).
//
// Pure counters, no behaviour change. Written to
// %TEMP%\piliplus_render_trace.log, one line every >= 2s while any counter
// changes. Purpose: tell apart "mpv stopped producing frames" from "the
// engine stopped pulling the texture", which is exactly the difference
// between a decode stall and a presentation stall.
//
//   frame   frames mpv actually rendered (MPV_RENDER_UPDATE_FRAME seen)
//   render  VideoOutput::Render() calls
//   commit  mailbox ProducerCommit() calls (frame published to the consumer)
//   cb      Flutter texture callback invocations (engine pulling the picture)
//   mark    MarkTextureFrameAvailable calls
//   swap    mpv_render_context_report_swap() calls
//
// Reading it:
//   frame grows, cb does not  -> the engine stopped pulling the texture
//   frame does not grow        -> mpv stopped producing frames (decode/timing)
//   commit does not grow       -> the mailbox stopped publishing frames

#ifndef PiliPlus4WOA_RENDER_TRACE_H_
#define PiliPlus4WOA_RENDER_TRACE_H_

#ifdef _WIN32

#include <Windows.h>

#include <cstdio>
#include <string>

// MSVC treats C4996 as an error (fopen is unsafe); suppress it here.
#pragma warning(push)
#pragma warning(disable : 4996)

struct PiliRenderTrace {
  long long frame = 0;
  long long render = 0;
  long long commit = 0;
  long long cb = 0;
  long long mark = 0;
  long long swap = 0;

  long long l_frame = -1;
  long long l_render = -1;
  long long l_commit = -1;
  long long l_cb = -1;
  long long l_mark = -1;
  long long l_swap = -1;

  DWORD last_tick = 0;
  bool started = false;
  std::string path;

  void tick() {
    const DWORD now = ::GetTickCount();
    if (!started) {
      started = true;
      last_tick = now;
      char buf[MAX_PATH] = {0};
      const DWORD n = ::GetTempPathA(MAX_PATH, buf);
      path = std::string(n > 0 && n < MAX_PATH ? buf : ".") +
             "\\piliplus_render_trace.log";
      return;
    }
    if (now - last_tick < 2000) {
      return;
    }
    last_tick = now;
    if (frame == l_frame && render == l_render && commit == l_commit &&
        cb == l_cb && mark == l_mark && swap == l_swap) {
      return;
    }
    l_frame = frame;
    l_render = render;
    l_commit = commit;
    l_cb = cb;
    l_mark = mark;
    l_swap = swap;
    FILE* f = nullptr;
    if (::fopen_s(&f, path.c_str(), "a") == 0 && f != nullptr) {
      std::fprintf(f,
                   "t=%lu frame=%lld render=%lld commit=%lld cb=%lld "
                   "mark=%lld swap=%lld\n",
                   static_cast<unsigned long>(now), frame, render, commit, cb,
                   mark, swap);
      std::fclose(f);
    }
  }
};

extern PiliRenderTrace g_piliTrace;

#pragma warning(pop)

#endif  // _WIN32

#endif  // PiliPlus4WOA_RENDER_TRACE_H_
