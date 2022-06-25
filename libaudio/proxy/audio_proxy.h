/*
 * Copyright (C) 2019 The Android Open Source Project
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 */

#ifndef AUDIO_PROXY_H
#define AUDIO_PROXY_H

#include <system/audio.h>
#include <hardware/hardware.h>
#include <hardware/audio.h>
#include <hardware/audio_alsaops.h>

#include <audio_utils/resampler.h>

#include "alsa_device_profile.h"
#include "alsa_device_proxy.h"
#include "alsa_logging.h"

#include "audio_streams.h"
#include "audio_usages.h"
#include "audio_devices.h"
#include "audio_offload.h"

#include "audio_pcm.h"
#include "audio_mixer.h"
#include "audio_abox.h"
#include "audio_streamconfig.h"
#include "audio_board_info.h"

struct call_param {
    uint32_t NSEC        : 1;
    uint32_t ext_vol     : 1;
    uint32_t apcall_app  : 3;
    uint32_t special     : 3;
    uint32_t device      : 7;
    uint32_t sample_rate : 4;
    uint32_t mic_num     : 3;
    uint32_t channel     : 5;
    uint32_t call_type   : 5;
};

/* Data Structure for Audio Proxy */
struct audio_proxy_stream
{
    audio_stream_type stream_type;
    audio_usage       stream_usage;

    // Real configuration for PCM/Compress Device
    int sound_card;
    int sound_device;

    struct pcm *pcm;
    struct pcm_config pcmconfig;

    int nonblock_flag;
    int ready_new_metadata;

    // Common
    unsigned int            requested_sample_rate;
    audio_channel_mask_t    requested_channel_mask;
    audio_format_t          requested_format;

    float vol_left, vol_right;

    uint64_t frames; /* total frames written, not cleared when entering standby */


    // Channel Conversion & Resample for Recording
    bool   need_monoconversion;
    bool   need_resampling;
    bool   need_update_pcm_config;

    int16_t* actual_read_buf;
    int      actual_read_status;
    size_t   actual_read_buf_size;
    size_t   read_buf_frames;

    void *   proc_buf_out;
    int      proc_buf_size;

    // Resampler
    struct resampler_itfe *             resampler;
    struct resampler_buffer_provider    buf_provider;

    int cpcall_rec_skipcnt;
    bool callrec_err_detect;
};

struct audio_proxy
{
    // Audio Path Routing
    struct mixer *mixer;
    struct audio_route *aroute;
    char *xml_path;

    // Mixer Update Thread
    pthread_rwlock_t mixer_update_lock;
    pthread_t        mixer_update_thread;

    audio_usage   active_playback_ausage;
    device_type   active_playback_device;
    modifier_type active_playback_modifier;

    audio_usage   active_capture_ausage;
    device_type   active_capture_device;
    modifier_type active_capture_modifier;

    // Primary Output Stream Proxy
    struct audio_proxy_stream *primary_out;

    /* Device Configuration */
    int num_earpiece;
    int num_speaker;
    int num_proximity;

    /* BuiltIn MIC Characteristics Map */
    int num_mic;
    struct audio_microphone_characteristic_t mic_info[AUDIO_MICROPHONE_MAX_COUNT];

    // PCM Devices for Audio Path(Loopback / ERAP)
    bool support_out_loopback;
    struct pcm *out_loopback;
    struct pcm *erap_in;

    /* Speaker AMP Configuration */
    bool support_spkamp;
    struct pcm *spkamp_reference;
    struct pcm *spkamp_playback;

    /*  Bluetooth Configuration */
    bool bt_internal;
    bool bt_external;

    bool support_btsco;
    struct pcm *btsco_playback;
    struct pcm *btsco_erap_pcminfo[BTSCO_MAX_ERAP_IDX];
    unsigned int btsco_erap_flag[BTSCO_MAX_ERAP_IDX];

    /* FM Radio Configuration */
    bool fm_internal;
    bool fm_external;

    // PCM Devices for FM Radio
    struct pcm *fm_playback;     // FM PCM Playback from A-Box
    struct pcm *fm_capture;      // FM PCM Capture to A-Box

    /* PCM Devices for Voice Call */
    struct pcm *call_rx;    // CP to Output Devices
    struct pcm *call_tx;    // Input Devices to CP

    // Call State
    bool call_state;
    bool vcall_bandchange;
    struct call_param call_param_idx;

    // Audio Mode
    int audio_mode;

    /* Call Screen Loopback Configuration */
        struct pcm *callscreen_loopback;
};


#define AUDIO_PARAMETER_DEVICE_CARD   "card"
#define AUDIO_PARAMETER_DEVICE_DEVICE "device"

#define MIXER_UPDATE_TIMEOUT    5  // 5 seconds

// Definition for MMAP Stream
#define MMAP_PERIOD_SIZE (DEFAULT_MEDIA_SAMPLING_RATE/1000)
#define MMAP_PERIOD_COUNT_MIN 32
#define MMAP_PERIOD_COUNT_MAX 512
#define MMAP_PERIOD_COUNT_DEFAULT (MMAP_PERIOD_COUNT_MAX)

#endif /* AUDIO_PROXY_H */
