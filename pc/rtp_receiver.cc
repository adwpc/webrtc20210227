/*
 *  Copyright 2015 The WebRTC project authors. All Rights Reserved.
 *
 *  Use of this source code is governed by a BSD-style license
 *  that can be found in the LICENSE file in the root of the source
 *  tree. An additional intellectual property rights grant can be found
 *  in the file PATENTS.  All contributing project authors may
 *  be found in the AUTHORS file in the root of the source tree.
 */

#include "pc/rtp_receiver.h"

#include <stddef.h>

#include <utility>
#include <vector>

#include "api/media_stream_proxy.h"
#include "pc/media_stream.h"
#include "rtc_base/location.h"

namespace webrtc {

// This function is only expected to be called on the signalling thread.
int RtpReceiverInternal::GenerateUniqueId() {
  static int g_unique_id = 0;

  return ++g_unique_id;
}

std::vector<rtc::scoped_refptr<MediaStreamInterface>>
RtpReceiverInternal::CreateStreamsFromIds(std::vector<std::string> stream_ids) {
  std::vector<rtc::scoped_refptr<MediaStreamInterface>> streams(
      stream_ids.size());
  for (size_t i = 0; i < stream_ids.size(); ++i) {
    streams[i] = MediaStreamProxy::Create(
        rtc::Thread::Current(), MediaStream::Create(std::move(stream_ids[i])));
  }
  return streams;
}

// Attempt to attach the frame decryptor to the current media channel on the
// correct worker thread only if both the media channel exists and a ssrc has
// been allocated to the stream.
void RtpReceiverInternal::MaybeAttachFrameDecryptorToMediaChannel(
    const absl::optional<uint32_t>& ssrc,
    rtc::Thread* worker_thread,
    rtc::scoped_refptr<webrtc::FrameDecryptorInterface> frame_decryptor,
    cricket::MediaChannel* media_channel,
    bool stopped) {
  if (media_channel && frame_decryptor && ssrc.has_value() && !stopped) {
    worker_thread->Invoke<void>(RTC_FROM_HERE, [&] {
      media_channel->SetFrameDecryptor(*ssrc, frame_decryptor);
    });
  }
}

}  // namespace webrtc
