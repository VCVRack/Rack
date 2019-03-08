// Audio playback, recording and mixing. Public domain. See "unlicense" statement at the end of this file.
// dr_audio - v0.0 (unversioned) - Release Date TBD
//
// David Reid - mackron@gmail.com

// !!!!! THIS IS WORK IN PROGRESS !!!!!

// USAGE
//
// dr_audio is a single-file library. To use it, do something like the following in one .c file.
//   #define DR_AUDIO_IMPLEMENTATION
//   #include "dr_audio.h"
//
// You can then #include this file in other parts of the program as you would with any other header file.
//
// dr_audio supports loading and decoding of WAV, FLAC and Vorbis streams via dr_wav, dr_flac and stb_vorbis respectively. To enable
// these all you need to do is #include "dr_audio.h" _after_ #include "dr_wav.h", #include "dr_flac.h" and #include "stb_vorbis.c" in
// the implementation file, like so:
//
//   #define DR_WAV_IMPLEMENTATION
//   #include "dr_wav.h"
//
//   #define DR_FLAC_IMPLEMENTATION
//   #include "dr_flac.h"
//
//   #define STB_VORBIS_IMPLEMENTATION
//   #include "stb_vorbis.c"
//
//   #define DR_AUDIO_IMPLEMENTATION
//   #include "dr_audio.h"
//
// dr_wav, dr_flac and stb_vorbis are entirely optional, and dr_audio will automatically detect the ones that are available without
// any additional intervention on your part.
//
//
// dr_audio has a layered API with different levels of flexibility vs simplicity. An example of the high level API follows:
//
//   dra_device* pDevice;
//   dra_result result = dra_device_create(NULL, dra_device_type_playback, &pDevice);
//   if (result != DRA_RESULT_SUCCESS) {
//       return -1;
//   }
//
//   dra_voice* pVoice;
//   result = dra_voice_create_from_file(pDevice, "my_song.flac", &pVoice);
//   if (result != DRA_RESULT_SUCCESS) {
//       return -1;
//   }
//
//   dra_voice_play(pVoice, DR_FALSE);
//
//   ...
//
//   dra_voice_delete(pVoice);
//   dra_device_delete(pDevice);
//
//
// An example of the low level API:
//
//   dra_context context;
//   dra_result result = dra_context_init(&context);  // Initializes the backend (DirectSound/ALSA)
//   if (result != DRA_RESULT_SUCCESS) {
//       return -1;
//   }
//
//   unsigned int deviceID = 0;                 // Default device
//   unsigned int channels = 2;                 // Stereo
//   unsigned int sampleRate = 48000;
//   unsigned int latencyInMilliseconds = 0;    // 0 will default to DR_AUDIO_DEFAULT_LATENCY.
//   dra_device device;
//   result = dra_device_init_ex(&context, dra_device_type_playback, deviceID, channels, sampleRate, latencyInMilliseconds, &device);
//   if (result != DRA_RESULT_SUCCESS) {
//       return -1;
//   }
//
//   dra_voice* pVoice;
//   dra_result result = dra_voice_create(pDevice, dra_format_f32, channels, sampleRate, voiceBufferSizeInBytes, pVoiceSampleData, &pVoice);
//   if (result != DRA_RESULT_SUCCESS) {
//       return -1;
//   }
//
//   ...
//
//   // Sometime later you may need to update the data inside the voice's internal buffer... It's your job to handle
//   // synchronization - have fun! Hint: use playback events at fixed locations to know when a region of the buffer
//   // is available for updating.
//   float* pVoiceData = (float*)dra_voice_get_buffer_ptr_by_sample(pVoice, sampleOffset);
//   if (pVoiceData == NULL) {
//       return -1;
//   }
//
//   memcpy(pVoiceData, pNewVoiceData, sizeof(float) * samplesToCopy);
//
//   ...
//
//   dra_voice_delete(pVoice);
//   dra_device_uninit(pDevice);
//   dra_context_uninit(pContext);
//
// In the above example the voice and device are configured to use the same number of channels and sample rate, however they are
// allowed to differ, in which case dr_audio will automatically convert the data. Note that sample rate conversion is currently
// very low quality.
//
// To handle streaming buffers, you can attach a callback that's fired when a voice's playback position reaches a certain point.
// Usually you would set this to the middle and end of the buffer, filling the previous half with new data. Use the
// dra_voice_add_playback_event() API for this.
//
// The playback position of a voice can be retrieved and set with dra_voice_get_playback_position() and dra_voice_set_playback_position()
// respctively. The playback is specified in samples. dra_voice_get_playback_position() will always return a value which is a multiple
// of the channel count. dra_voice_set_playback_position() will round the specified sample index to a multiple of the channel count.
//
//
// dr_audio has support for submixing which basically allows you to control volume (and in the future, effects) for groups of sounds
// which would typically be organized into categories. An abvious example would be in games where you may want to have separate volume
// controls for music, voices, special effects, etc. To do submixing, all you need to do is create a mixer. There is a master mixer
// associated with every device, and all newly created mixers are a child of the master mixer, by default:
//
//   dra_mixer* pMusicMixer;
//   dra_result result = dra_mixer_create(pDevice, &pMusicMixer);
//   if (result != DRA_RESULT_SUCCESS) {
//      return -1;
//   }
//
//   // At this point pMusicMixer is a child of the device's master mixer. To change the hierarchy, just do something like this:
//   dra_mixer_attach_submixer(pSomeOtherMixer, pMusicMixer);
//
//   // A voice is attached to the master mixer by default, but you can attach it to a different mixer like this:
//   dra_mixer_attach_voice(pMusicMixer, pMyMusicVoice);
//
//   // Control the volume of the mixer...
//   dra_mixer_set_volume(pMusicMixer, 0.5f);   // <-- The volume is linear, so this is half volume.
//
//
//
// dr_audio includes an abstraction for audio decoding. Built-in support is included for WAV, FLAC and Vorbis streams:
//
//   dra_decoder decoder;
//   if (dra_decoder_open_file(&decoder, filePath) != DRA_RESULT_SUCCESS) {
//       return -1;
//   }
//
//   dr_uint64 samplesRead = dra_decoder_read_f32(&decoder, samplesToRead, pSamples);
//   update_my_voice_data(pVoice, pSamples, samplesRead);
//
//   dra_decoder_close(&decoder);
//
// Decoders can be opened/initialized from files, a block of memory, or application-defined callbacks.
//
//
//
// OPTIONS
// #define these options before including this file.
//
// #define DR_AUDIO_NO_DSOUND
//   Disables the DirectSound backend. Note that this is the only backend for the Windows platform.
//
// #define DR_AUDIO_NO_ALSA
//   Disables the ALSA backend. Note that this is the only backend for the Linux platform.
//
// #define DR_AUDIO_NO_STDIO
//   Disables any functions that use stdio, such as dra_sound_create_from_file().


#ifndef dr_audio2_h
#define dr_audio2_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stddef.h>

#ifndef DR_SIZED_TYPES_DEFINED
#define DR_SIZED_TYPES_DEFINED
#if defined(_MSC_VER) && _MSC_VER < 1600
typedef   signed char    dr_int8;
typedef unsigned char    dr_uint8;
typedef   signed short   dr_int16;
typedef unsigned short   dr_uint16;
typedef   signed int     dr_int32;
typedef unsigned int     dr_uint32;
typedef   signed __int64 dr_int64;
typedef unsigned __int64 dr_uint64;
#else
#include <stdint.h>
typedef int8_t           dr_int8;
typedef uint8_t          dr_uint8;
typedef int16_t          dr_int16;
typedef uint16_t         dr_uint16;
typedef int32_t          dr_int32;
typedef uint32_t         dr_uint32;
typedef int64_t          dr_int64;
typedef uint64_t         dr_uint64;
#endif
typedef dr_int8          dr_bool8;
typedef dr_int32         dr_bool32;
#define DR_TRUE          1
#define DR_FALSE         0
#endif

#ifndef DR_AUDIO_MAX_CHANNEL_COUNT
#define DR_AUDIO_MAX_CHANNEL_COUNT      16
#endif

#ifndef DR_AUDIO_MAX_EVENT_COUNT
#define DR_AUDIO_MAX_EVENT_COUNT        16
#endif

#define DR_AUDIO_EVENT_ID_STOP  0xFFFFFFFFFFFFFFFFULL
#define DR_AUDIO_EVENT_ID_PLAY  0xFFFFFFFFFFFFFFFEULL

typedef int dra_result;
#define DRA_RESULT_SUCCESS              0
#define DRA_RESULT_UNKNOWN_ERROR        -1
#define DRA_RESULT_INVALID_ARGS         -2
#define DRA_RESULT_OUT_OF_MEMORY        -3
#define DRA_RESULT_FAILED_TO_OPEN_FILE  -4
#define DRA_RESULT_NO_BACKEND           -1024
#define DRA_RESULT_NO_BACKEND_DEVICE    -1025
#define DRA_RESULT_NO_DECODER           -1026
#define DRA_FAILED(result)              ((result) != 0)
#define DRA_SUCCEEDED(result)           ((result) == 0)

#define DRA_MIXER_FLAG_PAUSED           (1 << 0)

typedef enum
{
    dra_device_type_playback = 0,
    dra_device_type_capture
} dra_device_type;

typedef enum
{
    dra_format_u8 = 0,
    dra_format_s16,
    dra_format_s24,
    dra_format_s32,
    dra_format_f32,
    dra_format_default = dra_format_f32
} dra_format;

typedef enum
{
    dra_src_algorithm_none,
    dra_src_algorithm_linear,
} dra_src_algorithm;

// dra_thread_event_type is used internally for thread management.
typedef enum
{
    dra_thread_event_type_none,
    dra_thread_event_type_terminate,
    dra_thread_event_type_play
} dra_thread_event_type;

typedef struct dra_backend dra_backend;
typedef struct dra_backend_device dra_backend_device;

typedef struct dra_context dra_context;
typedef struct dra_device dra_device;
typedef struct dra_mixer dra_mixer;
typedef struct dra_voice dra_voice;

typedef void* dra_thread;
typedef void* dra_mutex;
typedef void* dra_semaphore;

typedef void (* dra_event_proc)            (dr_uint64 eventID, void* pUserData);
typedef void (* dra_samples_processed_proc)(dra_device* pDevice, const size_t sampleCount, const float* pSamples, void* pUserData);

typedef struct
{
    dr_uint64 id;
    void* pUserData;
    dr_uint64 sampleIndex;
    dra_event_proc proc;
    dra_voice* pVoice;
    dr_bool32 hasBeenSignaled;
} dra__event;

typedef struct
{
    size_t firstEvent;
    size_t eventCount;
    size_t eventBufferSize;
    dra__event* pEvents;
    dra_mutex lock;
} dra__event_queue;

struct dra_context
{
    dra_backend* pBackend;
};

struct dra_device
{
    // The context that created and owns this device.
    dra_context* pContext;

    // The backend device. This is used to connect the cross-platform front-end with the backend.
    dra_backend_device* pBackendDevice;

    // The main mutex for handling thread-safety.
    dra_mutex mutex;

    // The main thread. For playback devices, this is the thread that waits for fragments to finish processing an then
    // mixes and pushes new audio data to the hardware buffer for playback.
    dra_thread thread;

    // The semaphore used by the main thread to determine whether or not an event is waiting to be processed.
    dra_semaphore threadEventSem;

    // The event type of the most recent event.
    dra_thread_event_type nextThreadEventType;

    // TODO: Make these booleans flags.

    // Whether or not the device owns the context. This basically just means whether or not the device was created with a null
    // context and needs to delete the context itself when the device is deleted.
    dr_bool32 ownsContext;

    // Whether or not the device is being closed. This is used by the thread to determine if it needs to terminate. When
    // dra_device_close() is called, this flag will be set and threadEventSem released. The thread will then see this as it's
    // signal to terminate.
    dr_bool32 isClosed;

    // Whether or not the device is currently playing. When at least one voice is playing, this will be DR_TRUE. When there
    // are no voices playing, this will be set to DR_FALSE and the background thread will sit dormant until another voice
    // starts playing or the device is closed.
    dr_bool32 isPlaying;

    // Whether or not the device should stop on the next fragment. This is used for stopping playback of devices that
    // have no voice's playing.
    dr_bool32 stopOnNextFragment;


    // The master mixer. This is the one and only top-level mixer.
    dra_mixer* pMasterMixer;

    // The number of voices currently being played. This is used to determine whether or not the device should be placed
    // into a dormant state when nothing is being played.
    size_t playingVoicesCount;


    // When a playback event is scheduled it is added to this queue. Events are not posted straight away, but are instead
    // placed in a queue for processing later at specific times to ensure the event is posted AFTER the device has actually
    // played the sample the event is set for.
    dra__event_queue eventQueue;

    // The function to call when a segment of samples has been processed. This is only really needed for capture devices,
    // but can also be used to keep track of all of the audio data that's passed through a playback device. This is called
    // as the read pointer moves passed each segment and again for the leftover partial segment that'll occur when the
    // device is stopped.
    dra_samples_processed_proc onSamplesProcessed;
    void* pUserDataForOnSamplesProcessed;


    // The number of channels being used by the device.
    unsigned int channels;

    // The sample rate in seconds.
    unsigned int sampleRate;
};

struct dra_mixer
{
    // The device that created and owns this mixer.
    dra_device* pDevice;

    // Whether or not the mixer is paused.
    dr_uint32 flags;


    // The parent mixer.
    dra_mixer* pParentMixer;

    // The first child mixer.
    dra_mixer* pFirstChildMixer;

    // The last child mixer.
    dra_mixer* pLastChildMixer;

    // The next sibling mixer.
    dra_mixer* pNextSiblingMixer;

    // The previous sibling mixer.
    dra_mixer* pPrevSiblingMixer;


    // The first voice attached to the mixer.
    dra_voice* pFirstVoice;

    // The last voice attached to the mixer.
    dra_voice* pLastVoice;


    // The volume of the buffer as a linear scale. A value of 0.5 means the sound is at half volume. There is no hard
    // limit on the volume, however going beyond 1 may introduce clipping.
    float linearVolume;


    // Mixers output the results of the final mix into a buffer referred to as the staging buffer. A parent mixer will
    // use the staging buffer when it mixes the results of it's submixers. This is an offset of pData.
    float* pStagingBuffer;

    // A pointer to the buffer containing the sample data of the buffer currently being mixed, as floating point values.
    // This is an offset of pData.
    float* pNextSamplesToMix;


    // The sample data for pStagingBuffer and pNextSamplesToMix.
    float pData[1];
};

struct dra_voice
{
    // The device that created and owns this voice.
    dra_device* pDevice;

    // The mixer the voice is attached to. Should never be null. The mixer doesn't "own" the voice - the voice
    // is simply attached to it.
    dra_mixer* pMixer;


    // The next voice in the linked list of voices attached to the mixer.
    dra_voice* pNextVoice;

    // The previous voice in the linked list of voices attached to the mixer.
    dra_voice* pPrevVoice;


    // The format of the audio data contained within this voice.
    dra_format format;

    // The number of channels.
    unsigned int channels;

    // The sample rate in seconds.
    unsigned int sampleRate;


    // The volume of the voice as a linear scale. A value of 0.5 means the sound is at half volume. There is no hard
    // limit on the volume, however going beyond 1 may introduce clipping.
    float linearVolume;



    // Whether or not the voice is currently playing.
    dr_bool32 isPlaying;

    // Whether or not the voice is currently looping. Whether or not the voice is looping is determined by the last
    // call to dra_voice_play().
    dr_bool32 isLooping;


    // The total number of frames in the voice.
    dr_uint64 frameCount;

    // The current read position, in frames. An important detail with this is that it's based on the sample rate of the
    // device, not the voice.
    dr_uint64 currentReadPos;


    // A buffer for storing converted frames. This is used by dra_voice__next_frame(). As frames are converted to
    // floats, that are placed into this buffer.
    float convertedFrame[DR_AUDIO_MAX_CHANNEL_COUNT];


    // Data for sample rate conversion. Different SRC algorithms will use different data, which will be stored in their
    // own structure.
    struct
    {
        // The sample rate conversion algorithm to use.
        dra_src_algorithm algorithm;

        union
        {
            struct
            {
                dr_uint64 prevFrameIndex;
                float prevFrame[DR_AUDIO_MAX_CHANNEL_COUNT];
                float nextFrame[DR_AUDIO_MAX_CHANNEL_COUNT];
            } linear;
        } data;
    } src;


    // The number of playback notification events. This does not include the stop and play events.
    size_t playbackEventCount;

    // A pointer to the list of playback events.
    dra__event playbackEvents[DR_AUDIO_MAX_EVENT_COUNT];

    // The event to call when the voice has stopped playing, either naturally or explicitly with dra_voice_stop().
    dra__event stopEvent;

    // The event to call when the voice starts playing.
    dra__event playEvent;


    // Application defined user data.
    void* pUserData;


    // The size of the buffer in bytes.
    size_t sizeInBytes;

    // The actual buffer containing the raw audio data in it's native format. At mix time the data will be converted
    // to floats.
    dr_uint8 pData[1];
};


// dra_context_init()
dra_result dra_context_init(dra_context* pContext);
void dra_context_uninit(dra_context* pContext);

// Helper for allocating and initializing a context. If you want to do your own memory management or just want to put the context
// object on the stack, just use dra_context_init() instead.
dra_result dra_context_create(dra_context** ppContext);
void dra_context_delete(dra_context* pContext);


// dra_device_init_ex()
//
// If deviceID is 0 the default device for the given type is used.
// format can be dra_format_default which is dra_format_s32.
// If channels is set to 0, defaults 2 channels (stereo).
// If sampleRate is set to 0, defaults to 48000.
// If latency is 0, defaults to 50 milliseconds. See notes about latency above.
//
// Concerning the DirectSound backend (From MSDN):
//   Note that if your application is playing sounds as well as capturing them, capture buffer creation can fail when
//   the format of the capture buffer is not the same as that of the primary buffer. The reason is that some cards have
//   only a single clock and cannot support capture and playback at two different frequencies.
//
// This means you will need to keep the channels and sample rate consistent across playback and capture devices when
// using the DirectSound backend.
dra_result dra_device_init_ex(dra_context* pContext, dra_device_type type, unsigned int deviceID, unsigned int channels, unsigned int sampleRate, unsigned int latencyInMilliseconds, dra_device* pDevice);
dra_result dra_device_init(dra_context* pContext, dra_device_type type, dra_device* pDevice);
void dra_device_uninit(dra_device* pDevice);

// Helper for allocating and initializing a device object.
dra_result dra_device_create_ex(dra_context* pContext, dra_device_type type, unsigned int deviceID, unsigned int channels, unsigned int sampleRate, unsigned int latencyInMilliseconds, dra_device** ppDevice);
dra_result dra_device_create(dra_context* pContext, dra_device_type type, dra_device** ppDevice);
void dra_device_delete(dra_device* pDevice);

// Starts a capture device.
//
// Do not call this on a playback device - this is managed by dr_audio. This will fail for playback devices.
dra_result dra_device_start(dra_device* pDevice);

// Stops a capture device.
//
// Do not call this on a playback device - this is managed by dr_audio. This will fail for playback devices.
dra_result dra_device_stop(dra_device* pDevice);

// Sets the function to call when a segment of samples have been processed by the device (either captured
// or played back). Use this to keep track of the audio data that's passed to a playback device or from a
// capture device.
void dra_device_set_samples_processed_callback(dra_device* pDevice, dra_samples_processed_proc proc, void* pUserData);




// dra_mixer_create()
dra_result dra_mixer_create(dra_device* pDevice, dra_mixer** ppMixer);

// dra_mixer_delete()
//
// Deleting a mixer will detach any attached voices and sub-mixers and attach them to the master mixer. It is
// up to the application to manage the allocation of sub-mixers and voices. Typically you'll want to delete
// child mixers and voices before deleting a mixer.
void dra_mixer_delete(dra_mixer* pMixer);

// dra_mixer_attach_submixer()
void dra_mixer_attach_submixer(dra_mixer* pMixer, dra_mixer* pSubmixer);

// dra_mixer_detach_submixer()
void dra_mixer_detach_submixer(dra_mixer* pMixer, dra_mixer* pSubmixer);

// dra_mixer_detach_all_submixers()
void dra_mixer_detach_all_submixers(dra_mixer* pMixer);

// dra_mixer_attach_voice()
void dra_mixer_attach_voice(dra_mixer* pMixer, dra_voice* pVoice);

// dra_mixer_detach_voice()
void dra_mixer_detach_voice(dra_mixer* pMixer, dra_voice* pVoice);

// dra_mixer_detach_all_voices()
void dra_mixer_detach_all_voices(dra_mixer* pMixer);

// dra_voice_set_volume()
void dra_mixer_set_volume(dra_mixer* pMixer, float linearVolume);

// dra_voice_get_volume()
float dra_mixer_get_volume(dra_mixer* pMixer);

// Mixes the next number of frameCount and moves the playback position appropriately.
//
// pMixer     [in] The mixer.
// frameCount [in] The number of frames to mix.
//
// Returns the number of frames actually mixed.
//
// The return value is used to determine whether or not there's anything left to mix in the future. When there are
// no samples left to mix, the device can be put into a dormant state to prevent unnecessary processing.
//
// Mixed samples will be placed in pMixer->pStagingBuffer.
size_t dra_mixer_mix_next_frames(dra_mixer* pMixer, size_t frameCount);


// Non-recursively counts the number of voices that are attached to the given mixer.
size_t dra_mixer_count_attached_voices(dra_mixer* pMixer);

// Recursively counts the number of voices that are attached to the given mixer.
size_t dra_mixer_count_attached_voices_recursive(dra_mixer* pMixer);

// Non-recursively gathers all of the voices that are currently attached to the given mixer.
size_t dra_mixer_gather_attached_voices(dra_mixer* pMixer, dra_voice** ppVoicesOut);

// Recursively gathers all of the voices that are currently attached to the given mixer.
size_t dra_mixer_gather_attached_voices_recursive(dra_mixer* pMixer, dra_voice** ppVoicesOut);


// Marks the given mixer as paused.
void dra_mixer_pause(dra_mixer* pMixer);

// Unmarks the given mixer as paused.
void dra_mixer_resume(dra_mixer* pMixer);

// Determines whether or not the given mixer is paused.
dr_bool32 dra_mixer_is_paused(dra_mixer* pMixer);


// dra_voice_create()
dra_result dra_voice_create(dra_device* pDevice, dra_format format, unsigned int channels, unsigned int sampleRate, size_t sizeInBytes, const void* pInitialData, dra_voice** ppVoice);
dra_result dra_voice_create_compatible(dra_device* pDevice, size_t sizeInBytes, const void* pInitialData, dra_voice** ppVoice);

// dra_voice_delete()
void dra_voice_delete(dra_voice* pVoice);

// dra_voice_play()
//
// If the mixer the voice is attached to is not playing, the voice will be marked as playing, but won't actually play anything until
// the mixer is started again.
void dra_voice_play(dra_voice* pVoice, dr_bool32 loop);

// dra_voice_stop()
void dra_voice_stop(dra_voice* pVoice);

// dra_voice_is_playing()
dr_bool32 dra_voice_is_playing(dra_voice* pVoice);

// dra_voice_is_looping()
dr_bool32 dra_voice_is_looping(dra_voice* pVoice);


// dra_voice_set_volume()
void dra_voice_set_volume(dra_voice* pVoice, float linearVolume);

// dra_voice_get_volume()
float dra_voice_get_volume(dra_voice* pVoice);


void dra_voice_set_on_stop(dra_voice* pVoice, dra_event_proc proc, void* pUserData);
void dra_voice_set_on_play(dra_voice* pVoice, dra_event_proc proc, void* pUserData);
dr_bool32 dra_voice_add_playback_event(dra_voice* pVoice, dr_uint64 sampleIndex, dr_uint64 eventID, dra_event_proc proc, void* pUserData);
void dra_voice_remove_playback_event(dra_voice* pVoice, dr_uint64 eventID);

// dra_voice_get_playback_position()
dr_uint64 dra_voice_get_playback_position(dra_voice* pVoice);

// dra_voice_set_playback_position()
void dra_voice_set_playback_position(dra_voice* pVoice, dr_uint64 sampleIndex);


// dra_voice_get_buffer_ptr_by_sample()
void* dra_voice_get_buffer_ptr_by_sample(dra_voice* pVoice, dr_uint64 sample);

// dra_voice_write_silence()
void dra_voice_write_silence(dra_voice* pVoice, dr_uint64 sampleOffset, dr_uint64 sampleCount);


//// Other APIs ////

// Frees memory that was allocated internally by dr_audio.
void dra_free(void* p);

// Retrieves the number of bits per sample based on the given format.
unsigned int dra_get_bits_per_sample_by_format(dra_format format);

// Retrieves the number of bytes per sample based on the given format.
unsigned int dra_get_bytes_per_sample_by_format(dra_format format);


//// Decoder APIs ////

typedef enum
{
    dra_seek_origin_start,
    dra_seek_origin_current
} dra_seek_origin;

typedef size_t   (* dra_decoder_on_read_proc) (void* pUserData, void* pDataOut, size_t bytesToRead);
typedef dr_bool32 (* dra_decoder_on_seek_proc) (void* pUserData, int offset, dra_seek_origin origin);

typedef void     (* dra_decoder_on_delete_proc)       (void* pBackendDecoder);
typedef dr_uint64 (* dra_decoder_on_read_samples_proc) (void* pBackendDecoder, dr_uint64 samplesToRead, float* pSamplesOut);
typedef dr_bool32 (* dra_decoder_on_seek_samples_proc) (void* pBackendDecoder, dr_uint64 sample);

typedef struct
{
    const dr_uint8* data;
    size_t dataSize;
    size_t currentReadPos;
} dra__memory_stream;

typedef struct
{
    unsigned int channels;
    unsigned int sampleRate;
    dr_uint64 totalSampleCount;  // <-- Can be 0.

    dra_decoder_on_read_proc onRead;
    dra_decoder_on_seek_proc onSeek;
    void* pUserData;

    void* pBackendDecoder;
    dra_decoder_on_delete_proc onDelete;
    dra_decoder_on_read_samples_proc onReadSamples;
    dra_decoder_on_seek_samples_proc onSeekSamples;

    // A hack to enable decoding from memory without mallocing the user data.
    dra__memory_stream memoryStream;
} dra_decoder;

// dra_decoder_open()
dra_result dra_decoder_open(dra_decoder* pDecoder, dra_decoder_on_read_proc onRead, dra_decoder_on_seek_proc onSeek, void* pUserData);
float* dra_decoder_open_and_decode_f32(dra_decoder_on_read_proc onRead, dra_decoder_on_seek_proc onSeek, void* pUserData, unsigned int* channels, unsigned int* sampleRate, dr_uint64* totalSampleCount);

dra_result dra_decoder_open_memory(dra_decoder* pDecoder, const void* pData, size_t dataSize);
float* dra_decoder_open_and_decode_memory_f32(const void* pData, size_t dataSize, unsigned int* channels, unsigned int* sampleRate, dr_uint64* totalSampleCount);

#ifndef DR_AUDIO_NO_STDIO
dra_result dra_decoder_open_file(dra_decoder* pDecoder, const char* filePath);
float* dra_decoder_open_and_decode_file_f32(const char* filePath, unsigned int* channels, unsigned int* sampleRate, dr_uint64* totalSampleCount);
#endif

// dra_decoder_close()
void dra_decoder_close(dra_decoder* pDecoder);

// dra_decoder_read_f32()
dr_uint64 dra_decoder_read_f32(dra_decoder* pDecoder, dr_uint64 samplesToRead, float* pSamplesOut);

// dra_decoder_seek_to_sample()
dr_bool32 dra_decoder_seek_to_sample(dra_decoder* pDecoder, dr_uint64 sample);


//// High Level Helper APIs ////

#ifndef DR_AUDIO_NO_STDIO
// Creates a voice from a file.
dra_result dra_voice_create_from_file(dra_device* pDevice, const char* filePath, dra_voice** ppVoice);
#endif


//// High Level World API ////
//
// This section is for the sound world APIs. These are high-level APIs that sit directly on top of the main API.
typedef struct dra_sound_world dra_sound_world;
typedef struct dra_sound dra_sound;
typedef struct dra_sound_desc dra_sound_desc;

typedef void     (* dra_sound_on_delete_proc) (dra_sound* pSound);
typedef dr_uint64 (* dra_sound_on_read_proc)   (dra_sound* pSound, dr_uint64 samplesToRead, void* pSamplesOut);
typedef dr_bool32 (* dra_sound_on_seek_proc)   (dra_sound* pSound, dr_uint64 sample);

struct dra_sound_desc
{
    // The format of the sound.
    dra_format format;

    // The number of channels in the audio data.
    unsigned int channels;

    // The sample rate of the audio data.
    unsigned int sampleRate;


    // The size of the audio data in bytes. If this is 0 it is assumed the data will be streamed.
    size_t dataSize;

    // A pointer to the audio data. If this is null it is assumed the audio data is streamed.
    void* pData;


    // A pointer to the function to call when the sound object is deleted. This is used to give the application an
    // opportunity to do any clean up, such as closing decoders or whatnot.
    dra_sound_on_delete_proc onDelete;

    // A pointer to the function to call when dr_audio needs to request a chunk of audio data. This is only used when
    // streaming data.
    dra_sound_on_read_proc onRead;

    // A pointer to the function to call when dr_audio needs to seek the audio data. This is only used when streaming
    // data.
    dra_sound_on_seek_proc onSeek;

    // A pointer to some application defined user data that can be associated with the sound.
    void* pUserData;
};

struct dra_sound_world
{
    // The playback device.
    dra_device* pPlaybackDevice;

    // Whether or not the world owns the playback device. When this is set to DR_TRUE, it will be deleted when the world is deleted.
    dr_bool32 ownsPlaybackDevice;
};

struct dra_sound
{
    // The world that owns this sound.
    dra_sound_world* pWorld;

    // The voice object for emitting audio out of the device.
    dra_voice* pVoice;

    // The descriptor of the sound that was used to initialize the sound.
    dra_sound_desc desc;

    // Whether or not the sound is looping.
    dr_bool32 isLooping;

    // Whether or not the sound should be stopped at the end of the chunk that's currently playing.
    dr_bool32 stopOnNextChunk;

    // Application defined user data.
    void* pUserData;
};

// dra_sound_world_create()
//
// The playback device can be null, in which case a default one will be created.
dra_sound_world* dra_sound_world_create(dra_device* pPlaybackDevice);

// dra_sound_world_delete()
//
// This will delete every sound this world owns.
void dra_sound_world_delete(dra_sound_world* pWorld);

// dra_sound_world_play_inline()
//
// pMixer [in, optional] The mixer to attach the sound to. Can null, in which case it's attached to the master mixer.
void dra_sound_world_play_inline(dra_sound_world* pWorld, dra_sound_desc* pDesc, dra_mixer* pMixer);

// Plays an inlined sound in 3D space.
//
// This is a placeholder function. 3D position is not yet implemented.
void dra_sound_world_play_inline_3f(dra_sound_world* pWorld, dra_sound_desc* pDesc, dra_mixer* pMixer, float xPos, float yPos, float zPos);

// Stops playing every sound.
//
// This will stop all voices that are attached to the world's playback deviecs, including those that are not attached to a dra_sound object.
void dra_sound_world_stop_all_sounds(dra_sound_world* pWorld);


// Sets the position of the listener for 3D effects.
//
// This is placeholder.
void dra_sound_world_set_listener_position(dra_sound_world* pWorld, float xPos, float yPos, float zPos);

// Sets the orientation of the listener for 3D effects.
//
// This is placeholder.
void dra_sound_world_set_listener_orientation(dra_sound_world* pWorld, float xForward, float yForward, float zForward, float xUp, float yUp, float zUp);




// dra_sound_create()
//
// The datails in "desc" can be accessed from the returned object directly.
//
// This uses the pUserData member of the internal voice. Do not overwrite this. Instead, use the pUserData member of
// the returned dra_sound object.
dra_sound* dra_sound_create(dra_sound_world* pWorld, dra_sound_desc* pDesc);

#ifndef DR_AUDIO_NO_STDIO
// dra_sound_create_from_file()
//
// This will hold a handle to the file for the life of the sound.
dra_sound* dra_sound_create_from_file(dra_sound_world* pWorld, const char* filePath);
#endif

// dra_sound_delete()
void dra_sound_delete(dra_sound* pSound);


// dra_sound_play()
void dra_sound_play(dra_sound* pSound, dr_bool32 loop);

// dra_sound_stop()
void dra_sound_stop(dra_sound* pSound);


// Attaches the given sound to the given mixer.
//
// Setting pMixer to null will detach the sound from the mixer it is currently attached to and attach it
// to the master mixer.
void dra_sound_attach_to_mixer(dra_sound* pSound, dra_mixer* pMixer);


// dra_sound_set_on_stop()
void dra_sound_set_on_stop(dra_sound* pSound, dra_event_proc proc, void* pUserData);

// dra_sound_set_on_play()
void dra_sound_set_on_play(dra_sound* pSound, dra_event_proc proc, void* pUserData);


#ifdef __cplusplus
}
#endif
#endif  //dr_audio2_h



///////////////////////////////////////////////////////////////////////////////
//
// IMPLEMENTATION
//
///////////////////////////////////////////////////////////////////////////////
#ifdef DR_AUDIO_IMPLEMENTATION
#include <stdlib.h>
#include <math.h>
#include <assert.h>
#include <stdio.h>  // For good old printf debugging. Delete later.

#ifdef _MSC_VER
#define DR_AUDIO_INLINE static __forceinline
#else
#define DR_AUDIO_INLINE static inline
#endif

#define DR_AUDIO_DEFAULT_CHANNEL_COUNT  2
#define DR_AUDIO_DEFAULT_SAMPLE_RATE    48000
#define DR_AUDIO_DEFAULT_LATENCY        100     // Milliseconds. TODO: Test this with very low values. DirectSound appears to not signal the fragment events when it's too small. With values of about 20 it sounds crackly.
#define DR_AUDIO_DEFAULT_FRAGMENT_COUNT 3       // The hardware buffer is divided up into latency-sized blocks. This controls that number. Must be at least 2.

#define DR_AUDIO_BACKEND_TYPE_NULL      0
#define DR_AUDIO_BACKEND_TYPE_DSOUND    1
#define DR_AUDIO_BACKEND_TYPE_ALSA      2

#ifdef dr_wav_h
#define DR_AUDIO_HAS_WAV
    #ifndef DR_WAV_NO_STDIO
    #define DR_AUDIO_HAS_WAV_STDIO
    #endif
#endif
#ifdef dr_flac_h
#define DR_AUDIO_HAS_FLAC
    #ifndef DR_FLAC_NO_STDIO
    #define DR_AUDIO_HAS_FLAC_STDIO
    #endif
#endif
#ifdef STB_VORBIS_INCLUDE_STB_VORBIS_H
#define DR_AUDIO_HAS_VORBIS
    #ifndef STB_VORBIS_NO_STDIO
    #define DR_AUDIO_HAS_VORBIS_STDIO
    #endif
#endif

#if defined(DR_AUDIO_HAS_WAV)    || \
    defined(DR_AUDIO_HAS_FLAC)   || \
    defined(DR_AUDIO_HAS_VORBIS)
#define DR_AUDIO_HAS_EXTERNAL_DECODER
#endif


// Thanks to good old Bit Twiddling Hacks for this one: http://graphics.stanford.edu/~seander/bithacks.html#RoundUpPowerOf2
DR_AUDIO_INLINE unsigned int dra_next_power_of_2(unsigned int x)
{
    x--;
    x |= x >> 1;
    x |= x >> 2;
    x |= x >> 4;
    x |= x >> 8;
    x |= x >> 16;
    x++;

    return x;
}

DR_AUDIO_INLINE unsigned int dra_prev_power_of_2(unsigned int x)
{
    return dra_next_power_of_2(x) >> 1;
}


DR_AUDIO_INLINE float dra_mixf(float x, float y, float a)
{
    return x*(1-a) + y*a;
}


///////////////////////////////////////////////////////////////////////////////
//
// Platform Specific
//
///////////////////////////////////////////////////////////////////////////////

// Every backend struct should begin with these properties.
struct dra_backend
{
#define DR_AUDIO_BASE_BACKEND_ATTRIBS \
    unsigned int type; \

    DR_AUDIO_BASE_BACKEND_ATTRIBS
};

// Every backend device struct should begin with these properties.
struct dra_backend_device
{
#define DR_AUDIO_BASE_BACKEND_DEVICE_ATTRIBS \
    dra_backend* pBackend; \
    dra_device_type type; \
    unsigned int channels; \
    unsigned int sampleRate; \
    unsigned int fragmentCount; \
    unsigned int samplesPerFragment; \

    DR_AUDIO_BASE_BACKEND_DEVICE_ATTRIBS
};




// Compile-time platform detection and #includes.
#ifdef _WIN32
#include <windows.h>

//// Threading (Win32) ////
typedef DWORD (* dra_thread_entry_proc)(LPVOID pData);

dra_thread dra_thread_create(dra_thread_entry_proc entryProc, void* pData)
{
    return (dra_thread)CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)entryProc, pData, 0, NULL);
}

void dra_thread_delete(dra_thread thread)
{
    CloseHandle((HANDLE)thread);
}

void dra_thread_wait(dra_thread thread)
{
    WaitForSingleObject((HANDLE)thread, INFINITE);
}


dra_mutex dra_mutex_create()
{
    return (dra_mutex)CreateEventA(NULL, FALSE, TRUE, NULL);
}

void dra_mutex_delete(dra_mutex mutex)
{
    CloseHandle((HANDLE)mutex);
}

void dra_mutex_lock(dra_mutex mutex)
{
    WaitForSingleObject((HANDLE)mutex, INFINITE);
}

void dra_mutex_unlock(dra_mutex mutex)
{
    SetEvent((HANDLE)mutex);
}


dra_semaphore dra_semaphore_create(int initialValue)
{
    return (void*)CreateSemaphoreA(NULL, initialValue, LONG_MAX, NULL);
}

void dra_semaphore_delete(dra_semaphore semaphore)
{
    CloseHandle((HANDLE)semaphore);
}

dr_bool32 dra_semaphore_wait(dra_semaphore semaphore)
{
    return WaitForSingleObject((HANDLE)semaphore, INFINITE) == WAIT_OBJECT_0;
}

dr_bool32 dra_semaphore_release(dra_semaphore semaphore)
{
    return ReleaseSemaphore((HANDLE)semaphore, 1, NULL) != 0;
}



//// DirectSound ////
#ifndef DR_AUDIO_NO_DSOUND
#define DR_AUDIO_ENABLE_DSOUND
#include <dsound.h>
#include <mmreg.h>  // WAVEFORMATEX

GUID DR_AUDIO_GUID_NULL = {0x00000000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};

static GUID _g_draGUID_IID_DirectSoundNotify           = {0xb0210783, 0x89cd, 0x11d0, {0xaf, 0x08, 0x00, 0xa0, 0xc9, 0x25, 0xcd, 0x16}};
static GUID _g_draGUID_IID_IDirectSoundCaptureBuffer8  = {0x00990df4, 0x0dbb, 0x4872, {0x83, 0x3e, 0x6d, 0x30, 0x3e, 0x80, 0xae, 0xb6}};
static GUID _g_draGUID_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT = {0x00000003, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};

#ifdef __cplusplus
static GUID g_draGUID_IID_DirectSoundNotify            = _g_draGUID_IID_DirectSoundNotify;
static GUID g_draGUID_IID_IDirectSoundCaptureBuffer8   = _g_draGUID_IID_IDirectSoundCaptureBuffer8;
//static GUID g_draGUID_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT  = _g_draGUID_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
#else
static GUID* g_draGUID_IID_DirectSoundNotify           = &_g_draGUID_IID_DirectSoundNotify;
static GUID* g_draGUID_IID_IDirectSoundCaptureBuffer8  = &_g_draGUID_IID_IDirectSoundCaptureBuffer8;
//static GUID* g_draGUID_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT = &_g_draGUID_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
#endif

typedef HRESULT (WINAPI * pDirectSoundCreate8Proc)(LPCGUID pcGuidDevice, LPDIRECTSOUND8 *ppDS8, LPUNKNOWN pUnkOuter);
typedef HRESULT (WINAPI * pDirectSoundEnumerateAProc)(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext);
typedef HRESULT (WINAPI * pDirectSoundCaptureCreate8Proc)(LPCGUID pcGuidDevice, LPDIRECTSOUNDCAPTURE8 *ppDSC8, LPUNKNOWN pUnkOuter);
typedef HRESULT (WINAPI * pDirectSoundCaptureEnumerateAProc)(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext);

typedef struct
{
    DR_AUDIO_BASE_BACKEND_ATTRIBS

    // A handle to the dsound DLL for doing run-time linking.
    HMODULE hDSoundDLL;

    pDirectSoundCreate8Proc pDirectSoundCreate8;
    pDirectSoundEnumerateAProc pDirectSoundEnumerateA;
    pDirectSoundCaptureCreate8Proc pDirectSoundCaptureCreate8;
    pDirectSoundCaptureEnumerateAProc pDirectSoundCaptureEnumerateA;
} dra_backend_dsound;

typedef struct
{
    DR_AUDIO_BASE_BACKEND_DEVICE_ATTRIBS


    // The main device object for use with DirectSound.
    LPDIRECTSOUND8 pDS;

    // The DirectSound "primary buffer". It's basically just representing the connection between us and the hardware device.
    LPDIRECTSOUNDBUFFER pDSPrimaryBuffer;

    // The DirectSound "secondary buffer". This is where the actual audio data will be written to by dr_audio when it's time
    // to play back some audio through the speakers. This represents the hardware buffer.
    LPDIRECTSOUNDBUFFER pDSSecondaryBuffer;


    // The main capture device object for use with DirectSound. This is only used by capture devices and is created by DirectSoundCaptureCreate8().
    LPDIRECTSOUNDCAPTURE8 pDSCapture;

    // The capture buffer. This is where captured audio data will be placed. This is only used by capture devices.
    LPDIRECTSOUNDCAPTUREBUFFER8 pDSCaptureBuffer;


    // The notification object used by DirectSound to notify dr_audio that it's ready for the next fragment of audio data.
    LPDIRECTSOUNDNOTIFY pDSNotify;

    // Notification events for each fragment.
    HANDLE pNotifyEvents[DR_AUDIO_DEFAULT_FRAGMENT_COUNT];

    // The event the main playback thread will wait on to determine whether or not the playback loop should terminate.
    HANDLE hStopEvent;

    // The index of the fragment that is currently being played.
    unsigned int currentFragmentIndex;

    // The address of the mapped fragment. This is set with IDirectSoundBuffer8::Lock() and passed to IDriectSoundBuffer8::Unlock().
    void* pLockPtr;

    // The size of the locked buffer. This is set with IDirectSoundBuffer8::Lock() and passed to IDriectSoundBuffer8::Unlock().
    DWORD lockSize;

} dra_backend_device_dsound;

typedef struct
{
    unsigned int deviceID;
    unsigned int counter;
    const GUID* pGuid;
} dra_dsound__device_enum_data;

static BOOL CALLBACK dra_dsound__get_device_guid_by_id__callback(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext)
{
    (void)lpcstrDescription;
    (void)lpcstrModule;

    dra_dsound__device_enum_data* pData = (dra_dsound__device_enum_data*)lpContext;
    assert(pData != NULL);

    if (pData->counter == pData->deviceID) {
        pData->pGuid = lpGuid;
        return DR_FALSE;
    }

    pData->counter += 1;
    return DR_TRUE;
}

const GUID* dra_dsound__get_playback_device_guid_by_id(dra_backend* pBackend, unsigned int deviceID)
{
    // From MSDN:
    //
    // The first device enumerated is always called the Primary Sound Driver, and the lpGUID parameter of the callback is
    // NULL. This device represents the preferred output device set by the user in Control Panel.
    if (deviceID == 0) {
        return NULL;
    }

    dra_backend_dsound* pBackendDS = (dra_backend_dsound*)pBackend;
    if (pBackendDS == NULL) {
        return NULL;
    }

    // The device ID is treated as the device index. The actual ID for use by DirectSound is a GUID. We use DirectSoundEnumerateA()
    // iterate over each device. This function is usually only going to be used during initialization time so it won't be a performance
    // issue to not cache these.
    dra_dsound__device_enum_data data = {0};
    data.deviceID = deviceID;
    pBackendDS->pDirectSoundEnumerateA(dra_dsound__get_device_guid_by_id__callback, &data);

    return data.pGuid;
}

const GUID* dra_dsound__get_capture_device_guid_by_id(dra_backend* pBackend, unsigned int deviceID)
{
    // From MSDN:
    //
    // The first device enumerated is always called the Primary Sound Driver, and the lpGUID parameter of the callback is
    // NULL. This device represents the preferred output device set by the user in Control Panel.
    if (deviceID == 0) {
        return NULL;
    }

    dra_backend_dsound* pBackendDS = (dra_backend_dsound*)pBackend;
    if (pBackendDS == NULL) {
        return NULL;
    }

    // The device ID is treated as the device index. The actual ID for use by DirectSound is a GUID. We use DirectSoundEnumerateA()
    // iterate over each device. This function is usually only going to be used during initialization time so it won't be a performance
    // issue to not cache these.
    dra_dsound__device_enum_data data = {0};
    data.deviceID = deviceID;
    pBackendDS->pDirectSoundCaptureEnumerateA(dra_dsound__get_device_guid_by_id__callback, &data);

    return data.pGuid;
}

dra_backend* dra_backend_create_dsound()
{
    dra_backend_dsound* pBackendDS = (dra_backend_dsound*)calloc(1, sizeof(*pBackendDS));   // <-- Note the calloc() - makes it easier to handle the on_error goto.
    if (pBackendDS == NULL) {
        return NULL;
    }

    pBackendDS->type = DR_AUDIO_BACKEND_TYPE_DSOUND;

    pBackendDS->hDSoundDLL = LoadLibraryW(L"dsound.dll");
    if (pBackendDS->hDSoundDLL == NULL) {
        goto on_error;
    }

    pBackendDS->pDirectSoundCreate8 = (pDirectSoundCreate8Proc)GetProcAddress(pBackendDS->hDSoundDLL, "DirectSoundCreate8");
    if (pBackendDS->pDirectSoundCreate8 == NULL){
        goto on_error;
    }

    pBackendDS->pDirectSoundEnumerateA = (pDirectSoundEnumerateAProc)GetProcAddress(pBackendDS->hDSoundDLL, "DirectSoundEnumerateA");
    if (pBackendDS->pDirectSoundEnumerateA == NULL){
        goto on_error;
    }

    pBackendDS->pDirectSoundCaptureCreate8 = (pDirectSoundCaptureCreate8Proc)GetProcAddress(pBackendDS->hDSoundDLL, "DirectSoundCaptureCreate8");
    if (pBackendDS->pDirectSoundCaptureCreate8 == NULL){
        goto on_error;
    }

    pBackendDS->pDirectSoundCaptureEnumerateA = (pDirectSoundCaptureEnumerateAProc)GetProcAddress(pBackendDS->hDSoundDLL, "DirectSoundCaptureEnumerateA");
    if (pBackendDS->pDirectSoundCaptureEnumerateA == NULL){
        goto on_error;
    }


    return (dra_backend*)pBackendDS;

on_error:
    if (pBackendDS != NULL) {
        if (pBackendDS->hDSoundDLL != NULL) {
            FreeLibrary(pBackendDS->hDSoundDLL);
        }

        free(pBackendDS);
    }

    return NULL;
}

void dra_backend_delete_dsound(dra_backend* pBackend)
{
    dra_backend_dsound* pBackendDS = (dra_backend_dsound*)pBackend;
    if (pBackendDS == NULL) {
        return;
    }

    if (pBackendDS->hDSoundDLL != NULL) {
        FreeLibrary(pBackendDS->hDSoundDLL);
    }

    free(pBackendDS);
}

void dra_backend_device_close_dsound(dra_backend_device* pDevice)
{
    dra_backend_device_dsound* pDeviceDS = (dra_backend_device_dsound*)pDevice;
    if (pDeviceDS == NULL) {
        return;
    }

    if (pDeviceDS->pDSNotify) IDirectSoundNotify_Release(pDeviceDS->pDSNotify);

    if (pDevice->type == dra_device_type_playback) {
        if (pDeviceDS->pDSSecondaryBuffer) IDirectSoundBuffer_Release(pDeviceDS->pDSSecondaryBuffer);
        if (pDeviceDS->pDSPrimaryBuffer) IDirectSoundBuffer_Release(pDeviceDS->pDSPrimaryBuffer);
        if (pDeviceDS->pDS) IDirectSound_Release(pDeviceDS->pDS);
    } else {
        if (pDeviceDS->pDSCaptureBuffer) IDirectSoundCaptureBuffer_Release(pDeviceDS->pDSCaptureBuffer);
        if (pDeviceDS->pDSCapture) IDirectSoundCapture_Release(pDeviceDS->pDSCapture);
    }


    for (int i = 0; i < DR_AUDIO_DEFAULT_FRAGMENT_COUNT; ++i) {
        CloseHandle(pDeviceDS->pNotifyEvents[i]);
    }

    if (pDeviceDS->hStopEvent != NULL) {
        CloseHandle(pDeviceDS->hStopEvent);
    }

    free(pDeviceDS);
}

dra_backend_device* dra_backend_device_open_playback_dsound(dra_backend* pBackend, unsigned int deviceID, unsigned int channels, unsigned int sampleRate, unsigned int latencyInMilliseconds)
{
    // These are declared at the top to stop compilations errors on GCC about goto statements skipping over variable initialization.
    HRESULT hr;
    WAVEFORMATEXTENSIBLE* actualFormat;
    unsigned int sampleRateInMilliseconds;
    unsigned int proposedFramesPerFragment;
    unsigned int framesPerFragment;
    size_t fragmentSize;
    size_t hardwareBufferSize;

    dra_backend_dsound* pBackendDS = (dra_backend_dsound*)pBackend;
    if (pBackendDS == NULL) {
        return NULL;
    }

    dra_backend_device_dsound* pDeviceDS = (dra_backend_device_dsound*)calloc(1, sizeof(*pDeviceDS));
    if (pDeviceDS == NULL) {
        goto on_error;
    }

    if (channels == 0) {
        channels = DR_AUDIO_DEFAULT_CHANNEL_COUNT;
    }

    pDeviceDS->pBackend = pBackend;
    pDeviceDS->type = dra_device_type_playback;
    pDeviceDS->channels = channels;
    pDeviceDS->sampleRate = sampleRate;

    hr = pBackendDS->pDirectSoundCreate8(dra_dsound__get_playback_device_guid_by_id(pBackend, deviceID), &pDeviceDS->pDS, NULL);
    if (FAILED(hr)) {
        goto on_error;
    }

    // The cooperative level must be set before doing anything else.
    hr = IDirectSound_SetCooperativeLevel(pDeviceDS->pDS, GetForegroundWindow(), DSSCL_PRIORITY);
    if (FAILED(hr)) {
        goto on_error;
    }


    // The primary buffer is basically just the connection to the hardware.
    DSBUFFERDESC descDSPrimary;
    memset(&descDSPrimary, 0, sizeof(DSBUFFERDESC));
    descDSPrimary.dwSize  = sizeof(DSBUFFERDESC);
    descDSPrimary.dwFlags = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME;

    hr = IDirectSound_CreateSoundBuffer(pDeviceDS->pDS, &descDSPrimary, &pDeviceDS->pDSPrimaryBuffer, NULL);
    if (FAILED(hr)) {
        goto on_error;
    }


    // If the channel count is 0 then we need to use the default. From MSDN:
    //
    // The method succeeds even if the hardware does not support the requested format; DirectSound sets the buffer to the closest
    // supported format. To determine whether this has happened, an application can call the GetFormat method for the primary buffer
    // and compare the result with the format that was requested with the SetFormat method.
    WAVEFORMATEXTENSIBLE wf;
    memset(&wf, 0, sizeof(wf));
    wf.Format.cbSize               = sizeof(wf);
    wf.Format.wFormatTag           = WAVE_FORMAT_EXTENSIBLE;
    wf.Format.nChannels            = (WORD)channels;
    wf.Format.nSamplesPerSec       = (DWORD)sampleRate;
    wf.Format.wBitsPerSample       = sizeof(float)*8;
    wf.Format.nBlockAlign          = (wf.Format.nChannels * wf.Format.wBitsPerSample) / 8;
    wf.Format.nAvgBytesPerSec      = wf.Format.nBlockAlign * wf.Format.nSamplesPerSec;
    wf.Samples.wValidBitsPerSample = wf.Format.wBitsPerSample;
    wf.dwChannelMask               = 0;
    wf.SubFormat                   = _g_draGUID_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
    if (channels > 2) {
        wf.dwChannelMask = ~(((DWORD)-1) << channels);
    }

    hr = IDirectSoundBuffer_SetFormat(pDeviceDS->pDSPrimaryBuffer, (WAVEFORMATEX*)&wf);
    if (FAILED(hr)) {
        goto on_error;
    }


    // Get the ACTUAL properties of the buffer. This is silly API design...
    DWORD requiredSize;
    hr = IDirectSoundBuffer_GetFormat(pDeviceDS->pDSPrimaryBuffer, NULL, 0, &requiredSize);
    if (FAILED(hr)) {
        goto on_error;
    }

    char rawdata[1024];
    actualFormat = (WAVEFORMATEXTENSIBLE*)rawdata;
    hr = IDirectSoundBuffer_GetFormat(pDeviceDS->pDSPrimaryBuffer, (WAVEFORMATEX*)actualFormat, requiredSize, NULL);
    if (FAILED(hr)) {
        goto on_error;
    }

    pDeviceDS->channels = actualFormat->Format.nChannels;
    pDeviceDS->sampleRate = actualFormat->Format.nSamplesPerSec;

    // DirectSound always has the same number of fragments.
    pDeviceDS->fragmentCount = DR_AUDIO_DEFAULT_FRAGMENT_COUNT;


    // The secondary buffer is the buffer where the real audio data will be written to and used by the hardware device. It's
    // size is based on the latency, sample rate and channels.
    //
    // The format of the secondary buffer should exactly match the primary buffer as to avoid unnecessary data conversions.
    sampleRateInMilliseconds = pDeviceDS->sampleRate / 1000;
    if (sampleRateInMilliseconds == 0) {
        sampleRateInMilliseconds = 1;
    }

    // The size of a fragment is sized such that the number of frames contained within it is a multiple of 2. The reason for
    // this is to keep it consistent with the ALSA backend.
    proposedFramesPerFragment = sampleRateInMilliseconds * latencyInMilliseconds;
    framesPerFragment = dra_prev_power_of_2(proposedFramesPerFragment);
    if (framesPerFragment == 0) {
        framesPerFragment = 2;
    }

    pDeviceDS->samplesPerFragment = framesPerFragment * pDeviceDS->channels;

    fragmentSize = pDeviceDS->samplesPerFragment * sizeof(float);
    hardwareBufferSize = fragmentSize * pDeviceDS->fragmentCount;
    assert(hardwareBufferSize > 0);     // <-- If you've triggered this is means you've got something set to 0. You haven't been setting that latency to 0 have you?! That's not allowed!

    // Meaning of dwFlags (from MSDN):
    //
    // DSBCAPS_CTRLPOSITIONNOTIFY
    //   The buffer has position notification capability.
    //
    // DSBCAPS_GLOBALFOCUS
    //   With this flag set, an application using DirectSound can continue to play its buffers if the user switches focus to
    //   another application, even if the new application uses DirectSound.
    //
    // DSBCAPS_GETCURRENTPOSITION2
    //   In the first version of DirectSound, the play cursor was significantly ahead of the actual playing sound on emulated
    //   sound cards; it was directly behind the write cursor. Now, if the DSBCAPS_GETCURRENTPOSITION2 flag is specified, the
    //   application can get a more accurate play cursor.
    DSBUFFERDESC descDS;
    memset(&descDS, 0, sizeof(DSBUFFERDESC));
    descDS.dwSize = sizeof(DSBUFFERDESC);
    descDS.dwFlags = DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GLOBALFOCUS | DSBCAPS_GETCURRENTPOSITION2;
    descDS.dwBufferBytes = (DWORD)hardwareBufferSize;
    descDS.lpwfxFormat = (WAVEFORMATEX*)&wf;
    hr = IDirectSound_CreateSoundBuffer(pDeviceDS->pDS, &descDS, &pDeviceDS->pDSSecondaryBuffer, NULL);
    if (FAILED(hr)) {
        goto on_error;
    }


    // As DirectSound is playing back the hardware buffer it needs to notify dr_audio when it's ready for new data. This is done
    // through a notification object which we retrieve from the secondary buffer.
    hr = IDirectSoundBuffer8_QueryInterface(pDeviceDS->pDSSecondaryBuffer, g_draGUID_IID_DirectSoundNotify, (void**)&pDeviceDS->pDSNotify);
    if (FAILED(hr)) {
        goto on_error;
    }

    DSBPOSITIONNOTIFY notifyPoints[DR_AUDIO_DEFAULT_FRAGMENT_COUNT];  // One notification event for each fragment.
    for (int i = 0; i < DR_AUDIO_DEFAULT_FRAGMENT_COUNT; ++i)
    {
        pDeviceDS->pNotifyEvents[i] = CreateEventA(NULL, FALSE, FALSE, NULL);
        if (pDeviceDS->pNotifyEvents[i] == NULL) {
            goto on_error;
        }

        notifyPoints[i].dwOffset = (DWORD)(i * fragmentSize);    // <-- This is in bytes.
        notifyPoints[i].hEventNotify = pDeviceDS->pNotifyEvents[i];
    }

    hr = IDirectSoundNotify_SetNotificationPositions(pDeviceDS->pDSNotify, DR_AUDIO_DEFAULT_FRAGMENT_COUNT, notifyPoints);
    if (FAILED(hr)) {
        goto on_error;
    }



    // The termination event is used to determine when the playback thread should be terminated. The playback thread
    // will wait on this event in addition to the notification events in it's main loop.
    pDeviceDS->hStopEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
    if (pDeviceDS->hStopEvent == NULL) {
        goto on_error;
    }

    return (dra_backend_device*)pDeviceDS;

on_error:
    dra_backend_device_close_dsound((dra_backend_device*)pDeviceDS);
    return NULL;
}

dra_backend_device* dra_backend_device_open_capture_dsound(dra_backend* pBackend, unsigned int deviceID, unsigned int channels, unsigned int sampleRate, unsigned int latencyInMilliseconds)
{
    (void)latencyInMilliseconds;

    HRESULT hr;
    unsigned int sampleRateInMilliseconds;
    unsigned int proposedFramesPerFragment;
    unsigned int framesPerFragment;
    size_t fragmentSize;
    size_t hardwareBufferSize;

    dra_backend_dsound* pBackendDS = (dra_backend_dsound*)pBackend;
    if (pBackendDS == NULL) {
        return NULL;
    }

    dra_backend_device_dsound* pDeviceDS = (dra_backend_device_dsound*)calloc(1, sizeof(*pDeviceDS));
    if (pDeviceDS == NULL) {
        goto on_error;
    }

    if (channels == 0) {
        channels = DR_AUDIO_DEFAULT_CHANNEL_COUNT;
    }

    pDeviceDS->pBackend = pBackend;
    pDeviceDS->type = dra_device_type_capture;
    pDeviceDS->channels = channels;
    pDeviceDS->sampleRate = sampleRate;

    hr = pBackendDS->pDirectSoundCaptureCreate8(dra_dsound__get_capture_device_guid_by_id(pBackend, deviceID), &pDeviceDS->pDSCapture, NULL);
    if (FAILED(hr)) {
        goto on_error;
    }

    pDeviceDS->fragmentCount = DR_AUDIO_DEFAULT_FRAGMENT_COUNT;

    // The secondary buffer is the buffer where the real audio data will be written to and used by the hardware device. It's
    // size is based on the latency, sample rate and channels.
    //
    // The format of the secondary buffer should exactly match the primary buffer as to avoid unnecessary data conversions.
    sampleRateInMilliseconds = pDeviceDS->sampleRate / 1000;
    if (sampleRateInMilliseconds == 0) {
        sampleRateInMilliseconds = 1;
    }

    // The size of a fragment is sized such that the number of frames contained within it is a multiple of 2. The reason for
    // this is to keep it consistent with the ALSA backend.
    proposedFramesPerFragment = sampleRateInMilliseconds * latencyInMilliseconds;
    framesPerFragment = dra_prev_power_of_2(proposedFramesPerFragment);
    if (framesPerFragment == 0) {
        framesPerFragment = 2;
    }

    pDeviceDS->samplesPerFragment = framesPerFragment * pDeviceDS->channels;

    fragmentSize = pDeviceDS->samplesPerFragment * sizeof(float);
    hardwareBufferSize = fragmentSize * pDeviceDS->fragmentCount;
    assert(hardwareBufferSize > 0);     // <-- If you've triggered this is means you've got something set to 0. You haven't been setting that latency to 0 have you?! That's not allowed!


    WAVEFORMATEXTENSIBLE wf;
    memset(&wf, 0, sizeof(wf));
    wf.Format.cbSize               = sizeof(wf);
    wf.Format.wFormatTag           = WAVE_FORMAT_EXTENSIBLE;
    wf.Format.nChannels            = (WORD)channels;
    wf.Format.nSamplesPerSec       = (DWORD)sampleRate;
    wf.Format.wBitsPerSample       = sizeof(float)*8;
    wf.Format.nBlockAlign          = (wf.Format.nChannels * wf.Format.wBitsPerSample) / 8;
    wf.Format.nAvgBytesPerSec      = wf.Format.nBlockAlign * wf.Format.nSamplesPerSec;
    wf.Samples.wValidBitsPerSample = wf.Format.wBitsPerSample;
    wf.dwChannelMask               = 0;
    wf.SubFormat                   = _g_draGUID_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT;
    if (channels > 2) {
        wf.dwChannelMask = ~(((DWORD)-1) << channels);
    }

    DSCBUFFERDESC descDS;
    memset(&descDS, 0, sizeof(descDS));
    descDS.dwSize = sizeof(descDS);
    descDS.dwFlags = 0;
    descDS.dwBufferBytes = (DWORD)hardwareBufferSize;
    descDS.lpwfxFormat = (WAVEFORMATEX*)&wf;

    LPDIRECTSOUNDCAPTUREBUFFER pDSCB_Temp;
    hr = IDirectSoundCapture_CreateCaptureBuffer(pDeviceDS->pDSCapture, &descDS, &pDSCB_Temp, NULL);
    if (FAILED(hr)) {
        goto on_error;
    }

    hr = IDirectSoundCapture_QueryInterface(pDSCB_Temp, g_draGUID_IID_IDirectSoundCaptureBuffer8, (LPVOID*)&pDeviceDS->pDSCaptureBuffer);
    IDirectSoundCaptureBuffer_Release(pDSCB_Temp);
    if (FAILED(hr)) {
        goto on_error;  // Failed to retrieve the DirectSoundCaptureBuffer8 interface.
    }


    // As DirectSound is playing back the hardware buffer it needs to notify dr_audio when it's ready for new data. This is done
    // through a notification object which we retrieve from the secondary buffer.
    hr = IDirectSoundCaptureBuffer8_QueryInterface(pDeviceDS->pDSCaptureBuffer, g_draGUID_IID_DirectSoundNotify, (void**)&pDeviceDS->pDSNotify);
    if (FAILED(hr)) {
        goto on_error;
    }

    DSBPOSITIONNOTIFY notifyPoints[DR_AUDIO_DEFAULT_FRAGMENT_COUNT];  // One notification event for each fragment.
    for (int i = 0; i < DR_AUDIO_DEFAULT_FRAGMENT_COUNT; ++i)
    {
        pDeviceDS->pNotifyEvents[i] = CreateEventA(NULL, FALSE, FALSE, NULL);
        if (pDeviceDS->pNotifyEvents[i] == NULL) {
            goto on_error;
        }

        notifyPoints[i].dwOffset = (DWORD)(((i+1) * fragmentSize) % hardwareBufferSize);    // <-- This is in bytes.
        notifyPoints[i].hEventNotify = pDeviceDS->pNotifyEvents[i];
    }

    hr = IDirectSoundNotify_SetNotificationPositions(pDeviceDS->pDSNotify, DR_AUDIO_DEFAULT_FRAGMENT_COUNT, notifyPoints);
    if (FAILED(hr)) {
        goto on_error;
    }


    // The termination event is used to determine when the capture thread should be terminated. This thread
    // will wait on this event in addition to the notification events in it's main loop.
    pDeviceDS->hStopEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
    if (pDeviceDS->hStopEvent == NULL) {
        goto on_error;
    }

    return (dra_backend_device*)pDeviceDS;

on_error:
    dra_backend_device_close_dsound((dra_backend_device*)pDeviceDS);
    return NULL;
}

dra_backend_device* dra_backend_device_open_dsound(dra_backend* pBackend, dra_device_type type, unsigned int deviceID, unsigned int channels, unsigned int sampleRate, unsigned int latencyInMilliseconds)
{
    if (type == dra_device_type_playback) {
        return dra_backend_device_open_playback_dsound(pBackend, deviceID, channels, sampleRate, latencyInMilliseconds);
    } else {
        return dra_backend_device_open_capture_dsound(pBackend, deviceID, channels, sampleRate, latencyInMilliseconds);
    }
}

void dra_backend_device_play(dra_backend_device* pDevice)
{
    dra_backend_device_dsound* pDeviceDS = (dra_backend_device_dsound*)pDevice;
    if (pDeviceDS == NULL) {
        return;
    }

    if (pDevice->type == dra_device_type_playback) {
        IDirectSoundBuffer_Play(pDeviceDS->pDSSecondaryBuffer, 0, 0, DSBPLAY_LOOPING);
    } else {
        IDirectSoundCaptureBuffer8_Start(pDeviceDS->pDSCaptureBuffer, DSCBSTART_LOOPING);
    }
}

void dra_backend_device_stop(dra_backend_device* pDevice)
{
    dra_backend_device_dsound* pDeviceDS = (dra_backend_device_dsound*)pDevice;
    if (pDeviceDS == NULL) {
        return;
    }

    if (pDevice->type == dra_device_type_playback) {
        // Don't do anything if the buffer is not already playing.
        DWORD status;
        IDirectSoundBuffer_GetStatus(pDeviceDS->pDSSecondaryBuffer, &status);
        if ((status & DSBSTATUS_PLAYING) == 0) {
            return; // The buffer is already stopped.
        }

        // Stop the playback straight away to ensure output to the hardware device is stopped as soon as possible.
        IDirectSoundBuffer_Stop(pDeviceDS->pDSSecondaryBuffer);
        IDirectSoundBuffer_SetCurrentPosition(pDeviceDS->pDSSecondaryBuffer, 0);
    } else {
        // Don't do anything if the buffer is not already playing.
        DWORD status;
        IDirectSoundCaptureBuffer8_GetStatus(pDeviceDS->pDSCaptureBuffer, &status);
        if ((status & DSBSTATUS_PLAYING) == 0) {
            return; // The buffer is already stopped.
        }

        // Stop capture straight away to ensure output to the hardware device is stopped as soon as possible.
        //IDirectSoundCaptureBuffer8_Stop(pDeviceDS->pDSCaptureBuffer); // <-- There's actually a typo in my version of dsound.h which trigger's a compilation error here. The call below is safe, albeit slightly less intuitive.
        IDirectSoundCaptureBuffer_Stop(pDeviceDS->pDSCaptureBuffer);
    }

    // Now we just need to make dra_backend_device_play() return which in the case of DirectSound we do by
    // simply signaling the stop event.
    SetEvent(pDeviceDS->hStopEvent);
}

dr_bool32 dra_backend_device_wait(dra_backend_device* pDevice)   // <-- Returns DR_TRUE if the function has returned because it needs more data; DR_FALSE if the device has been stopped or an error has occured.
{
    dra_backend_device_dsound* pDeviceDS = (dra_backend_device_dsound*)pDevice;
    if (pDeviceDS == NULL) {
        return DR_FALSE;
    }

    unsigned int eventCount = DR_AUDIO_DEFAULT_FRAGMENT_COUNT + 1;
    HANDLE eventHandles[DR_AUDIO_DEFAULT_FRAGMENT_COUNT + 1];   // +1 for the stop event.
    memcpy(eventHandles, pDeviceDS->pNotifyEvents, sizeof(HANDLE) * DR_AUDIO_DEFAULT_FRAGMENT_COUNT);
    eventHandles[DR_AUDIO_DEFAULT_FRAGMENT_COUNT] = pDeviceDS->hStopEvent;

    DWORD rc = WaitForMultipleObjects(DR_AUDIO_DEFAULT_FRAGMENT_COUNT + 1, eventHandles, FALSE, INFINITE);
    if (rc >= WAIT_OBJECT_0 && rc < eventCount)
    {
        unsigned int eventIndex = rc - WAIT_OBJECT_0;
        HANDLE hEvent = eventHandles[eventIndex];

        // Has the device been stopped? If so, need to return DR_FALSE.
        if (hEvent == pDeviceDS->hStopEvent) {
            return DR_FALSE;
        }

        // If we get here it means the event that's been signaled represents a fragment.
        pDeviceDS->currentFragmentIndex = eventIndex;
        return DR_TRUE;
    }

    return DR_FALSE;
}

void* dra_backend_device_map_next_fragment(dra_backend_device* pDevice, size_t* pSamplesInFragmentOut)
{
    assert(pSamplesInFragmentOut != NULL);

    dra_backend_device_dsound* pDeviceDS = (dra_backend_device_dsound*)pDevice;
    if (pDeviceDS == NULL) {
        return NULL;
    }

    if (pDeviceDS->pLockPtr != NULL) {
        return NULL;    // A fragment is already mapped. Can only have a single fragment mapped at a time.
    }

    if (pDevice->type == dra_device_type_playback) {
        // If the device is not currently playing, we just return the first fragment. Otherwise we return the fragment that's sitting just past the
        // one that's currently playing.
        DWORD dwOffset = 0;
        DWORD dwBytes = pDeviceDS->samplesPerFragment * sizeof(float);

        DWORD status;
        IDirectSoundBuffer_GetStatus(pDeviceDS->pDSSecondaryBuffer, &status);
        if ((status & DSBSTATUS_PLAYING) != 0) {
            dwOffset = (((pDeviceDS->currentFragmentIndex + 1) % pDeviceDS->fragmentCount) * pDeviceDS->samplesPerFragment) * sizeof(float);
        }

        HRESULT hr = IDirectSoundBuffer_Lock(pDeviceDS->pDSSecondaryBuffer, dwOffset, dwBytes, &pDeviceDS->pLockPtr, &pDeviceDS->lockSize, NULL, NULL, 0);
        if (FAILED(hr)) {
            return NULL;
        }
    } else {
        DWORD dwOffset = (pDeviceDS->currentFragmentIndex * pDeviceDS->samplesPerFragment) * sizeof(float);
        DWORD dwBytes = pDeviceDS->samplesPerFragment * sizeof(float);

        HRESULT hr = IDirectSoundCaptureBuffer8_Lock(pDeviceDS->pDSCaptureBuffer, dwOffset, dwBytes, &pDeviceDS->pLockPtr, &pDeviceDS->lockSize, NULL, NULL, 0);
        if (FAILED(hr)) {
            return NULL;
        }
    }

    *pSamplesInFragmentOut = pDeviceDS->samplesPerFragment;
    return pDeviceDS->pLockPtr;
}

void dra_backend_device_unmap_next_fragment(dra_backend_device* pDevice)
{
    dra_backend_device_dsound* pDeviceDS = (dra_backend_device_dsound*)pDevice;
    if (pDeviceDS == NULL) {
        return;
    }

    if (pDeviceDS->pLockPtr == NULL) {
        return;     // Nothing is mapped.
    }

    if (pDevice->type == dra_device_type_playback) {
        IDirectSoundBuffer_Unlock(pDeviceDS->pDSSecondaryBuffer, pDeviceDS->pLockPtr, pDeviceDS->lockSize, NULL, 0);
    } else {
        IDirectSoundCaptureBuffer8_Unlock(pDeviceDS->pDSCaptureBuffer, pDeviceDS->pLockPtr, pDeviceDS->lockSize, NULL, 0);
    }

    pDeviceDS->pLockPtr = NULL;
    pDeviceDS->lockSize = 0;
}
#endif  // DR_AUDIO_NO_SOUND
#endif  // _WIN32

#ifdef __linux__
#include <unistd.h>
#include <sys/syscall.h>
#include <sys/types.h>
#include <pthread.h>
#include <fcntl.h>
#include <semaphore.h>

//// Threading (POSIX) ////
typedef void* (* dra_thread_entry_proc)(void* pData);

dra_thread dra_thread_create(dra_thread_entry_proc entryProc, void* pData)
{
    pthread_t thread;
    if (pthread_create(&thread, NULL, entryProc, pData) != 0) {
        return NULL;
    }

    return (dra_thread)thread;
}

void dra_thread_delete(dra_thread thread)
{
    (void)thread;
}

void dra_thread_wait(dra_thread thread)
{
    pthread_join((pthread_t)thread, NULL);
}


dra_mutex dra_mutex_create()
{
    // The pthread_mutex_t object is not a void* castable handle type. Just create it on the heap and be done with it.
    pthread_mutex_t* mutex = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
    if (pthread_mutex_init(mutex, NULL) != 0) {
        free(mutex);
        mutex = NULL;
    }

    return mutex;
}

void dra_mutex_delete(dra_mutex mutex)
{
    pthread_mutex_destroy((pthread_mutex_t*)mutex);
}

void dra_mutex_lock(dra_mutex mutex)
{
    pthread_mutex_lock((pthread_mutex_t*)mutex);
}

void dra_mutex_unlock(dra_mutex mutex)
{
    pthread_mutex_unlock((pthread_mutex_t*)mutex);
}


dra_semaphore dra_semaphore_create(int initialValue)
{
    sem_t* semaphore = (sem_t*)malloc(sizeof(sem_t));
    if (sem_init(semaphore, 0, (unsigned int)initialValue) == -1) {
        free(semaphore);
        semaphore = NULL;
    }

    return (dra_semaphore)semaphore;
}

void dra_semaphore_delete(dra_semaphore semaphore)
{
    sem_close((sem_t*)semaphore);
}

dr_bool32 dra_semaphore_wait(dra_semaphore semaphore)
{
    return sem_wait((sem_t*)semaphore) != -1;
}

dr_bool32 dra_semaphore_release(dra_semaphore semaphore)
{
    return sem_post((sem_t*)semaphore) != -1;
}


//// ALSA ////

#ifndef DR_AUDIO_NO_ALSA
#define DR_AUDIO_ENABLE_ALSA
#include <alsa/asoundlib.h>

typedef struct
{
    DR_AUDIO_BASE_BACKEND_ATTRIBS

    int unused;
} dra_backend_alsa;

typedef struct
{
    DR_AUDIO_BASE_BACKEND_DEVICE_ATTRIBS

    // The ALSA device handle.
    snd_pcm_t* deviceALSA;

    // Whether or not the device is currently playing.
    dr_bool32 isPlaying;

    // Whether or not the intermediary buffer is mapped.
    dr_bool32 isBufferMapped;

    // The intermediary buffer where audio data is written before being submitted to the device.
    float* pIntermediaryBuffer;
} dra_backend_device_alsa;


static dr_bool32 dra_alsa__get_device_name_by_id(dra_backend* pBackend, unsigned int deviceID, char* deviceNameOut)
{
    assert(pBackend != NULL);
    assert(deviceNameOut != NULL);

    deviceNameOut[0] = '\0';    // Safety.

    if (deviceID == 0) {
        strcpy(deviceNameOut, "default");
        return DR_TRUE;
    }


    unsigned int iDevice = 0;

    char** deviceHints;
    if (snd_device_name_hint(-1, "pcm", (void***)&deviceHints) < 0) {
        //printf("Failed to iterate devices.");
        return -1;
    }

    char** nextDeviceHint = deviceHints;
    while (*nextDeviceHint != NULL && iDevice < deviceID) {
        nextDeviceHint += 1;
        iDevice += 1;
    }

    dr_bool32 result = DR_FALSE;
    if (iDevice == deviceID) {
        strcpy(deviceNameOut, snd_device_name_get_hint(*nextDeviceHint, "NAME"));
        result = DR_TRUE;
    }

    snd_device_name_free_hint((void**)deviceHints);


    return result;
}


dra_backend* dra_backend_create_alsa()
{
    dra_backend_alsa* pBackendALSA = (dra_backend_alsa*)calloc(1, sizeof(*pBackendALSA));   // <-- Note the calloc() - makes it easier to handle the on_error goto.
    if (pBackendALSA == NULL) {
        return NULL;
    }

    pBackendALSA->type = DR_AUDIO_BACKEND_TYPE_ALSA;


    return (dra_backend*)pBackendALSA;

#if 0
on_error:
    if (pBackendALSA != NULL) {
        free(pBackendALSA);
    }

    return NULL;
#endif
}

void dra_backend_delete_alsa(dra_backend* pBackend)
{
    dra_backend_alsa* pBackendALSA = (dra_backend_alsa*)pBackend;
    if (pBackendALSA == NULL) {
        return;
    }

    free(pBackend);
}


void dra_backend_device_close_alsa(dra_backend_device* pDevice)
{
    dra_backend_device_alsa* pDeviceALSA = (dra_backend_device_alsa*)pDevice;
    if (pDeviceALSA == NULL) {
        return;
    }

    if (pDeviceALSA->deviceALSA != NULL) {
        snd_pcm_close(pDeviceALSA->deviceALSA);
    }

    free(pDeviceALSA->pIntermediaryBuffer);
    free(pDeviceALSA);
}

dra_backend_device* dra_backend_device_open_playback_alsa(dra_backend* pBackend, unsigned int deviceID, unsigned int channels, unsigned int sampleRate, unsigned int latencyInMilliseconds)
{
    unsigned int periods;
    int dir;
    size_t sampleRateInMilliseconds;
    unsigned int proposedFramesPerFragment;
    unsigned int framesPerFragment;
    snd_pcm_sw_params_t* pSWParams;

    dra_backend_alsa* pBackendALSA = (dra_backend_alsa*)pBackend;
    if (pBackendALSA == NULL) {
        return NULL;
    }

    snd_pcm_hw_params_t* pHWParams = NULL;

    dra_backend_device_alsa* pDeviceALSA = (dra_backend_device_alsa*)calloc(1, sizeof(*pDeviceALSA));
    if (pDeviceALSA == NULL) {
        goto on_error;
    }

    pDeviceALSA->pBackend = pBackend;
    pDeviceALSA->type = dra_device_type_playback;
    pDeviceALSA->channels = channels;
    pDeviceALSA->sampleRate = sampleRate;

    char deviceName[1024];
    if (!dra_alsa__get_device_name_by_id(pBackend, deviceID, deviceName)) {     // <-- This will return "default" if deviceID is 0.
        goto on_error;
    }

    if (snd_pcm_open(&pDeviceALSA->deviceALSA, deviceName, SND_PCM_STREAM_PLAYBACK, 0) < 0) {
        goto on_error;
    }


    if (snd_pcm_hw_params_malloc(&pHWParams) < 0) {
        goto on_error;
    }

    if (snd_pcm_hw_params_any(pDeviceALSA->deviceALSA, pHWParams) < 0) {
        goto on_error;
    }

    if (snd_pcm_hw_params_set_access(pDeviceALSA->deviceALSA, pHWParams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
        goto on_error;
    }

    if (snd_pcm_hw_params_set_format(pDeviceALSA->deviceALSA, pHWParams, SND_PCM_FORMAT_FLOAT_LE) < 0) {
        goto on_error;
    }


    if (snd_pcm_hw_params_set_rate_near(pDeviceALSA->deviceALSA, pHWParams, &sampleRate, 0) < 0) {
        goto on_error;
    }

    if (snd_pcm_hw_params_set_channels_near(pDeviceALSA->deviceALSA, pHWParams, &channels) < 0) {
        goto on_error;
    }

    pDeviceALSA->sampleRate = sampleRate;
    pDeviceALSA->channels = channels;

    periods = DR_AUDIO_DEFAULT_FRAGMENT_COUNT;
    dir = 1;
    if (snd_pcm_hw_params_set_periods_near(pDeviceALSA->deviceALSA, pHWParams, &periods, &dir) < 0) {
        //printf("Failed to set periods.\n");
        goto on_error;
    }

    pDeviceALSA->fragmentCount = periods;

    //printf("Periods: %d | Direction: %d\n", periods, dir);


    sampleRateInMilliseconds = pDeviceALSA->sampleRate / 1000;
    if (sampleRateInMilliseconds == 0) {
        sampleRateInMilliseconds = 1;
    }


    // According to the ALSA documentation, the value passed to snd_pcm_sw_params_set_avail_min() must be a power
    // of 2 on some hardware. The value passed to this function is the size in frames of a fragment. Thus, to be
    // as robust as possible the size of the hardware buffer should be sized based on the size of a closest power-
    // of-two fragment.
    //
    // To calculate the size of a fragment, the first step is to determine the initial proposed size. From that
    // it is dropped to the previous power of two. The reason for this is that, based on admittedly very basic
    // testing, ALSA seems to have good latency characteristics, and less latency is always preferable.
    proposedFramesPerFragment = sampleRateInMilliseconds * latencyInMilliseconds;
    framesPerFragment = dra_prev_power_of_2(proposedFramesPerFragment);
    if (framesPerFragment == 0) {
        framesPerFragment = 2;
    }

    pDeviceALSA->samplesPerFragment = framesPerFragment * pDeviceALSA->channels;

    if (snd_pcm_hw_params_set_buffer_size(pDeviceALSA->deviceALSA, pHWParams, framesPerFragment * pDeviceALSA->fragmentCount) < 0) {
        //printf("Failed to set buffer size.\n");
        goto on_error;
    }


    if (snd_pcm_hw_params(pDeviceALSA->deviceALSA, pHWParams) < 0) {
        goto on_error;
    }

    snd_pcm_hw_params_free(pHWParams);



    // Software params. There needs to be at least fragmentSize bytes in the hardware buffer before playing it, and there needs
    // be fragmentSize bytes available after every wait.
    pSWParams = NULL;
    if (snd_pcm_sw_params_malloc(&pSWParams) < 0) {
        goto on_error;
    }

    if (snd_pcm_sw_params_current(pDeviceALSA->deviceALSA, pSWParams) != 0) {
        goto on_error;
    }

    if (snd_pcm_sw_params_set_start_threshold(pDeviceALSA->deviceALSA, pSWParams, framesPerFragment) != 0) {
        goto on_error;
    }
    if (snd_pcm_sw_params_set_avail_min(pDeviceALSA->deviceALSA, pSWParams, framesPerFragment) != 0) {
        goto on_error;
    }

    if (snd_pcm_sw_params(pDeviceALSA->deviceALSA, pSWParams) != 0) {
        goto on_error;
    }
    snd_pcm_sw_params_free(pSWParams);


    // The intermediary buffer that will be used for mapping/unmapping.
    pDeviceALSA->isBufferMapped = DR_FALSE;
    pDeviceALSA->pIntermediaryBuffer = (float*)malloc(pDeviceALSA->samplesPerFragment * sizeof(float));
    if (pDeviceALSA->pIntermediaryBuffer == NULL) {
        goto on_error;
    }

    return (dra_backend_device*)pDeviceALSA;

on_error:
    if (pHWParams) {
        snd_pcm_hw_params_free(pHWParams);
    }

    if (pDeviceALSA != NULL) {
        if (pDeviceALSA->deviceALSA != NULL) {
            snd_pcm_close(pDeviceALSA->deviceALSA);
        }

        free(pDeviceALSA->pIntermediaryBuffer);
        free(pDeviceALSA);
    }

    return NULL;
}

dra_backend_device* dra_backend_device_open_capture_alsa(dra_backend* pBackend, unsigned int deviceID, unsigned int channels, unsigned int sampleRate, unsigned int latencyInMilliseconds)
{
    unsigned int periods;
    int dir;
    size_t sampleRateInMilliseconds;
    unsigned int proposedFramesPerFragment;
    unsigned int framesPerFragment;
    snd_pcm_sw_params_t* pSWParams;

    dra_backend_alsa* pBackendALSA = (dra_backend_alsa*)pBackend;
    if (pBackendALSA == NULL) {
        return NULL;
    }

    snd_pcm_hw_params_t* pHWParams = NULL;

    dra_backend_device_alsa* pDeviceALSA = (dra_backend_device_alsa*)calloc(1, sizeof(*pDeviceALSA));
    if (pDeviceALSA == NULL) {
        goto on_error;
    }

    pDeviceALSA->pBackend = pBackend;
    pDeviceALSA->type = dra_device_type_capture;
    pDeviceALSA->channels = channels;
    pDeviceALSA->sampleRate = sampleRate;

    char deviceName[1024];
    if (!dra_alsa__get_device_name_by_id(pBackend, deviceID, deviceName)) {     // <-- This will return "default" if deviceID is 0.
        goto on_error;
    }

    if (snd_pcm_open(&pDeviceALSA->deviceALSA, deviceName, SND_PCM_STREAM_CAPTURE, 0) < 0) {
        goto on_error;
    }


    if (snd_pcm_hw_params_malloc(&pHWParams) < 0) {
        goto on_error;
    }

    if (snd_pcm_hw_params_any(pDeviceALSA->deviceALSA, pHWParams) < 0) {
        goto on_error;
    }

    if (snd_pcm_hw_params_set_access(pDeviceALSA->deviceALSA, pHWParams, SND_PCM_ACCESS_RW_INTERLEAVED) < 0) {
        goto on_error;
    }

    if (snd_pcm_hw_params_set_format(pDeviceALSA->deviceALSA, pHWParams, SND_PCM_FORMAT_FLOAT_LE) < 0) {
        goto on_error;
    }


    if (snd_pcm_hw_params_set_rate_near(pDeviceALSA->deviceALSA, pHWParams, &sampleRate, 0) < 0) {
        goto on_error;
    }

    if (snd_pcm_hw_params_set_channels_near(pDeviceALSA->deviceALSA, pHWParams, &channels) < 0) {
        goto on_error;
    }

    pDeviceALSA->sampleRate = sampleRate;
    pDeviceALSA->channels = channels;

    periods = DR_AUDIO_DEFAULT_FRAGMENT_COUNT;
    dir = 1;
    if (snd_pcm_hw_params_set_periods_near(pDeviceALSA->deviceALSA, pHWParams, &periods, &dir) < 0) {
        //printf("Failed to set periods.\n");
        goto on_error;
    }

    pDeviceALSA->fragmentCount = periods;

    //printf("Periods: %d | Direction: %d\n", periods, dir);


    sampleRateInMilliseconds = pDeviceALSA->sampleRate / 1000;
    if (sampleRateInMilliseconds == 0) {
        sampleRateInMilliseconds = 1;
    }


    // According to the ALSA documentation, the value passed to snd_pcm_sw_params_set_avail_min() must be a power
    // of 2 on some hardware. The value passed to this function is the size in frames of a fragment. Thus, to be
    // as robust as possible the size of the hardware buffer should be sized based on the size of a closest power-
    // of-two fragment.
    //
    // To calculate the size of a fragment, the first step is to determine the initial proposed size. From that
    // it is dropped to the previous power of two. The reason for this is that, based on admittedly very basic
    // testing, ALSA seems to have good latency characteristics, and less latency is always preferable.
    proposedFramesPerFragment = sampleRateInMilliseconds * latencyInMilliseconds;
    framesPerFragment = dra_prev_power_of_2(proposedFramesPerFragment);
    if (framesPerFragment == 0) {
        framesPerFragment = 2;
    }

    pDeviceALSA->samplesPerFragment = framesPerFragment * pDeviceALSA->channels;

    if (snd_pcm_hw_params_set_buffer_size(pDeviceALSA->deviceALSA, pHWParams, framesPerFragment * pDeviceALSA->fragmentCount) < 0) {
        //printf("Failed to set buffer size.\n");
        goto on_error;
    }


    if (snd_pcm_hw_params(pDeviceALSA->deviceALSA, pHWParams) < 0) {
        goto on_error;
    }

    snd_pcm_hw_params_free(pHWParams);



    // Software params. There needs to be at least fragmentSize bytes in the hardware buffer before playing it, and there needs
    // be fragmentSize bytes available after every wait.
    pSWParams = NULL;
    if (snd_pcm_sw_params_malloc(&pSWParams) < 0) {
        goto on_error;
    }

    if (snd_pcm_sw_params_current(pDeviceALSA->deviceALSA, pSWParams) != 0) {
        goto on_error;
    }

    if (snd_pcm_sw_params_set_start_threshold(pDeviceALSA->deviceALSA, pSWParams, framesPerFragment) != 0) {
        goto on_error;
    }
    if (snd_pcm_sw_params_set_avail_min(pDeviceALSA->deviceALSA, pSWParams, framesPerFragment) != 0) {
        goto on_error;
    }

    if (snd_pcm_sw_params(pDeviceALSA->deviceALSA, pSWParams) != 0) {
        goto on_error;
    }
    snd_pcm_sw_params_free(pSWParams);

    // The intermediary buffer that will be used for mapping/unmapping.
    pDeviceALSA->isBufferMapped = DR_FALSE;
    pDeviceALSA->pIntermediaryBuffer = (float*)malloc(pDeviceALSA->samplesPerFragment * sizeof(float));
    if (pDeviceALSA->pIntermediaryBuffer == NULL) {
        goto on_error;
    }

    return (dra_backend_device*)pDeviceALSA;

on_error:
    dra_backend_device_close_alsa((dra_backend_device*)pDeviceALSA);
    return NULL;
}

dra_backend_device* dra_backend_device_open_alsa(dra_backend* pBackend, dra_device_type type, unsigned int deviceID, unsigned int channels, unsigned int sampleRate, unsigned int latencyInMilliseconds)
{
    if (type == dra_device_type_playback) {
        return dra_backend_device_open_playback_alsa(pBackend, deviceID, channels, sampleRate, latencyInMilliseconds);
    } else {
        return dra_backend_device_open_capture_alsa(pBackend, deviceID, channels, sampleRate, latencyInMilliseconds);
    }
}


void dra_backend_device_play(dra_backend_device* pDevice)
{
    dra_backend_device_alsa* pDeviceALSA = (dra_backend_device_alsa*)pDevice;
    if (pDeviceALSA == NULL) {
        return;
    }

    snd_pcm_prepare(pDeviceALSA->deviceALSA);
    pDeviceALSA->isPlaying = DR_TRUE;
}

void dra_backend_device_stop(dra_backend_device* pDevice)
{
    dra_backend_device_alsa* pDeviceALSA = (dra_backend_device_alsa*)pDevice;
    if (pDeviceALSA == NULL) {
        return;
    }

    snd_pcm_drop(pDeviceALSA->deviceALSA);
    pDeviceALSA->isPlaying = DR_FALSE;
}

dr_bool32 dra_backend_device_wait(dra_backend_device* pDevice)
{
    dra_backend_device_alsa* pDeviceALSA = (dra_backend_device_alsa*)pDevice;
    if (pDeviceALSA == NULL) {
        return DR_FALSE;
    }

    if (!pDeviceALSA->isPlaying) {
        return DR_FALSE;
    }

    if (pDevice->type == dra_device_type_playback) {
        int result = snd_pcm_wait(pDeviceALSA->deviceALSA, -1);
        if (result > 0) {
            return DR_TRUE;
        }

        if (result == -EPIPE) {
            // xrun. Prepare the device again and just return DR_TRUE.
            snd_pcm_prepare(pDeviceALSA->deviceALSA);
            return DR_TRUE;
        }
    } else {
        snd_pcm_uframes_t frameCount = pDeviceALSA->samplesPerFragment / pDeviceALSA->channels;
        snd_pcm_sframes_t framesRead = snd_pcm_readi(pDeviceALSA->deviceALSA, pDeviceALSA->pIntermediaryBuffer, frameCount);
        if (framesRead > 0) {
            return DR_TRUE;
        }

        if (framesRead == -EPIPE) {
            // xrun. Prepare the device again and just return DR_TRUE.
            snd_pcm_prepare(pDeviceALSA->deviceALSA);
            return DR_TRUE;
        }
    }

    return DR_FALSE;
}

void* dra_backend_device_map_next_fragment(dra_backend_device* pDevice, size_t* pSamplesInFragmentOut)
{
    assert(pSamplesInFragmentOut != NULL);

    dra_backend_device_alsa* pDeviceALSA = (dra_backend_device_alsa*)pDevice;
    if (pDeviceALSA == NULL) {
        return NULL;
    }

    if (pDeviceALSA->isBufferMapped) {
        return NULL;    // A fragment is already mapped. Can only have a single fragment mapped at a time.
    }

    //if (pDeviceALSA->type == dra_device_type_capture) {
    //    snd_pcm_readi(pDeviceALSA->deviceALSA, pDeviceALSA->pIntermediaryBuffer, pDeviceALSA->samplesPerFragment / pDeviceALSA->channels);
    //}

    *pSamplesInFragmentOut = pDeviceALSA->samplesPerFragment;
    return pDeviceALSA->pIntermediaryBuffer;
}

void dra_backend_device_unmap_next_fragment(dra_backend_device* pDevice)
{
    dra_backend_device_alsa* pDeviceALSA = (dra_backend_device_alsa*)pDevice;
    if (pDeviceALSA == NULL) {
        return;
    }

    if (pDeviceALSA->isBufferMapped) {
        return;    // Nothing is mapped.
    }

    // Unammping is when the data is written to the device.
    if (pDeviceALSA->type == dra_device_type_playback) {
        snd_pcm_writei(pDeviceALSA->deviceALSA, pDeviceALSA->pIntermediaryBuffer, pDeviceALSA->samplesPerFragment / pDeviceALSA->channels);
    }
}
#endif  // DR_AUDIO_NO_ALSA
#endif  // __linux__


void dra_thread_wait_and_delete(dra_thread thread)
{
    dra_thread_wait(thread);
    dra_thread_delete(thread);
}


dra_backend* dra_backend_create()
{
    dra_backend* pBackend = NULL;

#ifdef DR_AUDIO_ENABLE_DSOUND
    pBackend = dra_backend_create_dsound();
    if (pBackend != NULL) {
        return pBackend;
    }
#endif

#ifdef DR_AUDIO_ENABLE_ALSA
    pBackend = dra_backend_create_alsa();
    if (pBackend != NULL) {
        return pBackend;
    }
#endif

    // If we get here it means we couldn't find a backend. Default to a NULL backend? Returning NULL makes it clearer that an error occured.
    return NULL;
}

void dra_backend_delete(dra_backend* pBackend)
{
    if (pBackend == NULL) {
        return;
    }

#ifdef DR_AUDIO_ENABLE_DSOUND
    if (pBackend->type == DR_AUDIO_BACKEND_TYPE_DSOUND) {
        dra_backend_delete_dsound(pBackend);
        return;
    }
#endif

#ifdef DR_AUDIO_ENABLE_ALSA
    if (pBackend->type == DR_AUDIO_BACKEND_TYPE_ALSA) {
        dra_backend_delete_alsa(pBackend);
        return;
    }
#endif

    // Should never get here. If this assert is triggered it means you haven't plugged in the API in the list above.
    assert(DR_FALSE);
}


dra_backend_device* dra_backend_device_open(dra_backend* pBackend, dra_device_type type, unsigned int deviceID, unsigned int channels, unsigned int sampleRate, unsigned int latencyInMilliseconds)
{
    if (pBackend == NULL) {
        return NULL;
    }

#ifdef DR_AUDIO_ENABLE_DSOUND
    if (pBackend->type == DR_AUDIO_BACKEND_TYPE_DSOUND) {
        return dra_backend_device_open_dsound(pBackend, type, deviceID, channels, sampleRate, latencyInMilliseconds);
    }
#endif

#ifdef DR_AUDIO_ENABLE_ALSA
    if (pBackend->type == DR_AUDIO_BACKEND_TYPE_ALSA) {
        return dra_backend_device_open_alsa(pBackend, type, deviceID, channels, sampleRate, latencyInMilliseconds);
    }
#endif


    // Should never get here. If this assert is triggered it means you haven't plugged in the API in the list above.
    assert(DR_FALSE);
    return NULL;
}

void dra_backend_device_close(dra_backend_device* pDevice)
{
    if (pDevice == NULL) {
        return;
    }

    assert(pDevice->pBackend != NULL);

#ifdef DR_AUDIO_ENABLE_DSOUND
    if (pDevice->pBackend->type == DR_AUDIO_BACKEND_TYPE_DSOUND) {
        dra_backend_device_close_dsound(pDevice);
        return;
    }
#endif

#ifdef DR_AUDIO_ENABLE_ALSA
    if (pDevice->pBackend->type == DR_AUDIO_BACKEND_TYPE_ALSA) {
        dra_backend_device_close_alsa(pDevice);
        return;
    }
#endif
}



///////////////////////////////////////////////////////////////////////////////
//
// Cross Platform
//
///////////////////////////////////////////////////////////////////////////////

// Reads the next frame.
//
// Frames are retrieved with respect to the device the voice is attached to. What this basically means is
// that any data conversions will be done within this function.
//
// The return value is a pointer to the voice containing the converted samples, always as floating point.
//
// If the voice is in the same format as the device (floating point, same sample rate and channels), then
// this function will be on a fast path and will return almost immediately with a pointer that points to
// the voice's actual data without any data conversion.
//
// If an error occurs, null is returned. Null will be returned if the end of the voice's buffer is reached
// and it's non-looping. This will not return NULL if the voice is looping - it will just loop back to the
// start as one would expect.
//
// This function is not thread safe, but can be called from multiple threads if you do your own
// synchronization. Just keep in mind that the return value may point to the voice's actual internal data.
float* dra_voice__next_frame(dra_voice* pVoice);

// dra_voice__next_frames()
size_t dra_voice__next_frames(dra_voice* pVoice, size_t frameCount, float* pSamplesOut);


dra_result dra_context_init(dra_context* pContext)
{
    if (pContext == NULL) return DRA_RESULT_INVALID_ARGS;
    memset(pContext, 0, sizeof(*pContext));

    // We need a backend first.
    pContext->pBackend = dra_backend_create();
    if (pContext->pBackend == NULL) {
        return DRA_RESULT_NO_BACKEND;    // Failed to create a backend.
    }

    return DRA_RESULT_SUCCESS;
}

void dra_context_uninit(dra_context* pContext)
{
    if (pContext == NULL || pContext->pBackend == NULL) return;
    dra_backend_delete(pContext->pBackend);
}

dra_result dra_context_create(dra_context** ppContext)
{
    if (ppContext == NULL) return DRA_RESULT_INVALID_ARGS;
    *ppContext = NULL;

    dra_context* pContext = (dra_context*)malloc(sizeof(*pContext));
    if (pContext == NULL) {
        return DRA_RESULT_OUT_OF_MEMORY;
    }

     dra_result result = dra_context_init(pContext);
     if (result != DRA_RESULT_SUCCESS) {
         free(pContext);
         return result;
     }

     *ppContext = pContext;
     return DRA_RESULT_SUCCESS;
}

void dra_context_delete(dra_context* pContext)
{
    if (pContext == NULL) return;

    dra_context_uninit(pContext);
    free(pContext);
}


void dra_event_queue__schedule_event(dra__event_queue* pQueue, dra__event* pEvent)
{
    if (pQueue == NULL || pEvent == NULL) {
        return;
    }

    dra_mutex_lock(pQueue->lock);
    {
        if (pQueue->eventCount == pQueue->eventBufferSize)
        {
            // Ran out of room. Resize.
            size_t newEventBufferSize = (pQueue->eventBufferSize == 0) ? 16 : pQueue->eventBufferSize*2;
            dra__event* pNewEvents = (dra__event*)malloc(newEventBufferSize * sizeof(*pNewEvents));
            if (pNewEvents == NULL) {
                return;
            }

            for (size_t i = 0; i < pQueue->eventCount; ++i) {
                pQueue->pEvents[i] = pQueue->pEvents[(pQueue->firstEvent + i) % pQueue->eventBufferSize];
            }

            pQueue->firstEvent = 0;
            pQueue->eventBufferSize = newEventBufferSize;
            pQueue->pEvents = pNewEvents;
        }

        assert(pQueue->eventCount < pQueue->eventBufferSize);

        pQueue->pEvents[(pQueue->firstEvent + pQueue->eventCount) % pQueue->eventBufferSize] = *pEvent;
        pQueue->eventCount += 1;
    }
    dra_mutex_unlock(pQueue->lock);
}

void dra_event_queue__cancel_events_of_voice(dra__event_queue* pQueue, dra_voice* pVoice)
{
    if (pQueue == NULL || pVoice == NULL) {
        return;
    }

    dra_mutex_lock(pQueue->lock);
    {
        // We don't actually remove anything from the queue, but instead zero out the event's data.
        for (size_t i = 0; i < pQueue->eventCount; ++i) {
            dra__event* pEvent = &pQueue->pEvents[(pQueue->firstEvent + i) % pQueue->eventBufferSize];
            if (pEvent->pVoice == pVoice) {
                pEvent->pVoice = NULL;
                pEvent->proc = NULL;
            }
        }
    }
    dra_mutex_unlock(pQueue->lock);
}

dr_bool32 dra_event_queue__next_event(dra__event_queue* pQueue, dra__event* pEventOut)
{
    if (pQueue == NULL || pEventOut == NULL) {
        return DR_FALSE;
    }

    dr_bool32 result = DR_FALSE;
    dra_mutex_lock(pQueue->lock);
    {
        if (pQueue->eventCount > 0) {
            *pEventOut = pQueue->pEvents[pQueue->firstEvent];
            pQueue->firstEvent = (pQueue->firstEvent + 1) % pQueue->eventBufferSize;
            pQueue->eventCount -= 1;
            result = DR_TRUE;
        }
    }
    dra_mutex_unlock(pQueue->lock);

    return result;
}

void dra_event_queue__post_events(dra__event_queue* pQueue)
{
    if (pQueue == NULL) {
        return;
    }

    dra__event nextEvent;
    while (dra_event_queue__next_event(pQueue, &nextEvent)) {
        if (nextEvent.proc) {
            nextEvent.proc(nextEvent.id, nextEvent.pUserData);
        }
    }
}


void dra_device__post_event(dra_device* pDevice, dra_thread_event_type type)
{
    assert(pDevice != NULL);

    pDevice->nextThreadEventType = type;
    dra_semaphore_release(pDevice->threadEventSem);
}

void dra_device__lock(dra_device* pDevice)
{
    assert(pDevice != NULL);
    dra_mutex_lock(pDevice->mutex);
}

void dra_device__unlock(dra_device* pDevice)
{
    assert(pDevice != NULL);
    dra_mutex_unlock(pDevice->mutex);
}

dr_bool32 dra_device__is_playing_nolock(dra_device* pDevice)
{
    assert(pDevice != NULL);
    return pDevice->isPlaying;
}

dr_bool32 dra_device__mix_next_fragment(dra_device* pDevice)
{
    assert(pDevice != NULL);

    size_t samplesInFragment;
    void* pSampleData = dra_backend_device_map_next_fragment(pDevice->pBackendDevice, &samplesInFragment);
    if (pSampleData == NULL) {
        dra_backend_device_stop(pDevice->pBackendDevice);
        return DR_FALSE;
    }

    size_t framesInFragment = samplesInFragment / pDevice->channels;
    size_t framesMixed = dra_mixer_mix_next_frames(pDevice->pMasterMixer, framesInFragment);

    memcpy(pSampleData, pDevice->pMasterMixer->pStagingBuffer, (size_t)samplesInFragment * sizeof(float));

    if (pDevice->onSamplesProcessed) {
        pDevice->onSamplesProcessed(pDevice, framesMixed * pDevice->channels, (const float*)pSampleData, pDevice->pUserDataForOnSamplesProcessed);
    }

    dra_backend_device_unmap_next_fragment(pDevice->pBackendDevice);

    if (framesMixed == 0) {
        pDevice->stopOnNextFragment = DR_TRUE;
    }

    //printf("Mixed next fragment into %p\n", pSampleData);
    return DR_TRUE;
}

void dra_device__play(dra_device* pDevice)
{
    assert(pDevice != NULL);

    dra_device__lock(pDevice);
    {
        // Don't do anything if the device is already playing.
        if (!dra_device__is_playing_nolock(pDevice))
        {
            assert(pDevice->pBackendDevice->type == dra_device_type_capture || pDevice->playingVoicesCount > 0);

            dra_device__post_event(pDevice, dra_thread_event_type_play);
            pDevice->isPlaying = DR_TRUE;
            pDevice->stopOnNextFragment = DR_FALSE;
        }
    }
    dra_device__unlock(pDevice);
}

void dra_device__stop(dra_device* pDevice)
{
    assert(pDevice != NULL);

    dra_device__lock(pDevice);
    {
        // Don't do anything if the device is already stopped.
        if (dra_device__is_playing_nolock(pDevice))
        {
            //assert(pDevice->playingVoicesCount == 0);

            dra_backend_device_stop(pDevice->pBackendDevice);
            pDevice->isPlaying = DR_FALSE;
        }
    }
    dra_device__unlock(pDevice);
}

void dra_device__voice_playback_count_inc(dra_device* pDevice)
{
    assert(pDevice != NULL);

    dra_device__lock(pDevice);
    {
        pDevice->playingVoicesCount += 1;
        pDevice->stopOnNextFragment  = DR_FALSE;
    }
    dra_device__unlock(pDevice);
}

void dra_device__voice_playback_count_dec(dra_device* pDevice)
{
    dra_device__lock(pDevice);
    {
        pDevice->playingVoicesCount -= 1;
    }
    dra_device__unlock(pDevice);
}

// The entry point signature is slightly different depending on whether or not we're using Win32 or POSIX threads.
#ifdef _WIN32
DWORD dra_device__thread_proc(LPVOID pData)
#else
void* dra_device__thread_proc(void* pData)
#endif
{
    dra_device* pDevice = (dra_device*)pData;
    assert(pDevice != NULL);

    // The thread is always open for the life of the device. The loop below will only terminate when a terminate message is received.
    for (;;)
    {
        // Wait for an event...
        dra_semaphore_wait(pDevice->threadEventSem);

        if (pDevice->nextThreadEventType == dra_thread_event_type_terminate) {
            //printf("Terminated!\n");
            break;
        }

        if (pDevice->nextThreadEventType == dra_thread_event_type_play)
        {
            if (pDevice->pBackendDevice->type == dra_device_type_playback) {
                // The backend device needs to start playing, but we first need to ensure it has an initial chunk of data available.
                dra_device__mix_next_fragment(pDevice);
            }

            // Start playing the backend device only after the initial fragment has been mixed, and only if it's a playback device.
            dra_backend_device_play(pDevice->pBackendDevice);

            // There could be "play" events needing to be posted.
            dra_event_queue__post_events(&pDevice->eventQueue);


            // Wait for the device to request more data...
            while (dra_backend_device_wait(pDevice->pBackendDevice)) {
                dra_event_queue__post_events(&pDevice->eventQueue);

                if (pDevice->stopOnNextFragment) {
                    dra_device__stop(pDevice);  // <-- Don't break from the loop here. Instead have dra_backend_device_wait() return naturally from the stop notification.
                } else {
                    if (pDevice->pBackendDevice->type == dra_device_type_playback) {
                        dra_device__mix_next_fragment(pDevice);
                    } else {
                        size_t sampleCount;
                        void* pSampleData = dra_backend_device_map_next_fragment(pDevice->pBackendDevice, &sampleCount);
                        if (pSampleData != NULL) {
                            if (pDevice->onSamplesProcessed) {
                                pDevice->onSamplesProcessed(pDevice, sampleCount, (const float*)pSampleData, pDevice->pUserDataForOnSamplesProcessed);
                            }

                            dra_backend_device_unmap_next_fragment(pDevice->pBackendDevice);
                        }
                    }
                }
            }

            // There could be some events needing to be posted.
            dra_event_queue__post_events(&pDevice->eventQueue);
            //printf("Stopped!\n");

            // Don't fall through.
            continue;
        }
    }

    return 0;
}

dra_result dra_device_init_ex(dra_context* pContext, dra_device_type type, unsigned int deviceID, unsigned int channels, unsigned int sampleRate, unsigned int latencyInMilliseconds, dra_device* pDevice)
{
    if (pDevice == NULL) return DRA_RESULT_INVALID_ARGS;

    dr_bool32 ownsContext = DR_FALSE;
    if (pContext == NULL) {
        pContext = (dra_context*)malloc(sizeof(*pContext));
        if (pContext == NULL) {
            return DRA_RESULT_OUT_OF_MEMORY;
        }

        dra_result result = dra_context_init(pContext);
        if (result != DRA_RESULT_SUCCESS) {
            return result;
        }

        ownsContext = DR_TRUE;
    }

    if (sampleRate == 0) sampleRate = DR_AUDIO_DEFAULT_SAMPLE_RATE;
    if (latencyInMilliseconds == 0) latencyInMilliseconds = DR_AUDIO_DEFAULT_LATENCY;


    dra_result result = DRA_RESULT_SUCCESS;

    memset(pDevice, 0, sizeof(*pDevice));
    pDevice->pContext = pContext;
    pDevice->ownsContext = ownsContext;

    pDevice->pBackendDevice = dra_backend_device_open(pContext->pBackend, type, deviceID, channels, sampleRate, latencyInMilliseconds);
    if (pDevice->pBackendDevice == NULL) {
        result = DRA_RESULT_NO_BACKEND_DEVICE;
        goto on_error;
    }

    pDevice->channels = pDevice->pBackendDevice->channels;
    pDevice->sampleRate = pDevice->pBackendDevice->sampleRate;


    pDevice->mutex = dra_mutex_create();
    if (pDevice->mutex == NULL) {
        result = DRA_RESULT_UNKNOWN_ERROR;  // TODO: Change this to the return value of dra_mutex_create().
        goto on_error;
    }

    pDevice->threadEventSem = dra_semaphore_create(0);
    if (pDevice->threadEventSem == NULL) {
        result = DRA_RESULT_UNKNOWN_ERROR;  // TODO: Change this to the return value of dra_semaphore_create().
        goto on_error;
    }

    result = dra_mixer_create(pDevice, &pDevice->pMasterMixer);
    if (result != DRA_RESULT_SUCCESS) {
        goto on_error;
    }


    pDevice->eventQueue.lock = dra_mutex_create();
    if (pDevice->eventQueue.lock == NULL) {
        result = DRA_RESULT_UNKNOWN_ERROR;  // TODO: Change this to the return value of dra_mutex_create().
        goto on_error;
    }


    // Create the thread last to ensure the device is in a valid state as soon as the entry procedure is run.
    pDevice->thread = dra_thread_create(dra_device__thread_proc, pDevice);
    if (pDevice->thread == NULL) {
        result = DRA_RESULT_UNKNOWN_ERROR;  // TODO: Change this to the return value of dra_thread_create().
        goto on_error;
    }

    return result;

on_error:
    if (pDevice != NULL) {
        if (pDevice->pMasterMixer != NULL) dra_mixer_delete(pDevice->pMasterMixer);
        if (pDevice->pBackendDevice != NULL) dra_backend_device_close(pDevice->pBackendDevice);
        if (pDevice->threadEventSem != NULL) dra_semaphore_delete(pDevice->threadEventSem);
        if (pDevice->mutex != NULL) dra_mutex_delete(pDevice->mutex);

        if (pDevice->ownsContext) {
            dra_context_uninit(pDevice->pContext);
            free(pDevice->pContext);
        }
    }

    return result;
}

dra_result dra_device_init(dra_context* pContext, dra_device_type type, dra_device* pDevice)
{
    return dra_device_init_ex(pContext, type, 0, 0, DR_AUDIO_DEFAULT_SAMPLE_RATE, DR_AUDIO_DEFAULT_LATENCY, pDevice);
}

void dra_device_uninit(dra_device* pDevice)
{
    if (pDevice == NULL || pDevice->pContext == NULL) return;

    // Mark the device as closed in order to prevent other threads from doing work after closing.
    dra_device__lock(pDevice);
    {
        pDevice->isClosed = DR_TRUE;
    }
    dra_device__unlock(pDevice);

    // Stop playback before doing anything else.
    dra_device__stop(pDevice);

    // The background thread needs to be terminated at this point.
    dra_device__post_event(pDevice, dra_thread_event_type_terminate);
    dra_thread_wait_and_delete(pDevice->thread);


    // At this point the device is marked as closed which should prevent voice's and mixers from being created and deleted. We now need
    // to delete the master mixer which in turn will delete all of the attached voices and submixers.
    if (pDevice->pMasterMixer != NULL) {
        dra_mixer_delete(pDevice->pMasterMixer);
    }


    if (pDevice->pBackendDevice != NULL) {
        dra_backend_device_close(pDevice->pBackendDevice);
    }

    if (pDevice->threadEventSem != NULL) {
        dra_semaphore_delete(pDevice->threadEventSem);
    }

    if (pDevice->mutex != NULL) {
        dra_mutex_delete(pDevice->mutex);
    }


    if (pDevice->eventQueue.pEvents) {
        free(pDevice->eventQueue.pEvents);
    }


    if (pDevice->ownsContext) {
        dra_context_uninit(pDevice->pContext);
        free(pDevice->pContext);
    }
}


dra_result dra_device_create_ex(dra_context* pContext, dra_device_type type, unsigned int deviceID, unsigned int channels, unsigned int sampleRate, unsigned int latencyInMilliseconds, dra_device** ppDevice)
{
    if (ppDevice == NULL) return DRA_RESULT_INVALID_ARGS;
    *ppDevice = NULL;

    dra_device* pDevice = (dra_device*)malloc(sizeof(*pDevice));
    if (pDevice == NULL) {
        return DRA_RESULT_OUT_OF_MEMORY;
    }

    dra_result result = dra_device_init_ex(pContext, type, deviceID, channels, sampleRate, latencyInMilliseconds, pDevice);
    if (result != DRA_RESULT_SUCCESS) {
        free(pDevice);
        return result;
    }

    *ppDevice = pDevice;
    return DRA_RESULT_SUCCESS;
}

dra_result dra_device_create(dra_context* pContext, dra_device_type type, dra_device** ppDevice)
{
    return dra_device_create_ex(pContext, type, 0, 0, DR_AUDIO_DEFAULT_SAMPLE_RATE, DR_AUDIO_DEFAULT_LATENCY, ppDevice);
}

void dra_device_delete(dra_device* pDevice)
{
    if (pDevice == NULL) return;

    dra_device_uninit(pDevice);
    free(pDevice);
}

dra_result dra_device_start(dra_device* pDevice)
{
    if (pDevice == NULL || pDevice->pBackendDevice->type == dra_device_type_playback) return DRA_RESULT_INVALID_ARGS;

    dra_device__play(pDevice);
    return DRA_RESULT_SUCCESS;
}

dra_result dra_device_stop(dra_device* pDevice)
{
    if (pDevice == NULL || pDevice->pBackendDevice->type == dra_device_type_playback) return DRA_RESULT_INVALID_ARGS;

    dra_device__stop(pDevice);
    return DRA_RESULT_SUCCESS;
}

void dra_device_set_samples_processed_callback(dra_device* pDevice, dra_samples_processed_proc proc, void* pUserData)
{
    if (pDevice == NULL) return;
    pDevice->onSamplesProcessed = proc;
    pDevice->pUserDataForOnSamplesProcessed = pUserData;
}


dra_result dra_mixer_create(dra_device* pDevice, dra_mixer** ppMixer)
{
    if (ppMixer == NULL) return DRA_RESULT_INVALID_ARGS;
    *ppMixer = NULL;

    if (pDevice == NULL) return DRA_RESULT_INVALID_ARGS;


    // There needs to be two blocks of memory at the end of the mixer - one for the staging buffer and another for the buffer that
    // will store the float32 samples of the voice currently being mixed.
    size_t extraDataSize = (size_t)pDevice->pBackendDevice->samplesPerFragment * sizeof(float) * 2;
    dra_mixer* pMixer = (dra_mixer*)calloc(1, sizeof(*pMixer) + extraDataSize);
    if (pMixer == NULL) {
        return DRA_RESULT_OUT_OF_MEMORY;
    }

    pMixer->pDevice = pDevice;
    pMixer->linearVolume = 1;

    pMixer->pStagingBuffer = pMixer->pData;
    pMixer->pNextSamplesToMix = pMixer->pStagingBuffer + pDevice->pBackendDevice->samplesPerFragment;

    // Attach the mixer to the master mixer by default. If the master mixer is null it means we're creating the master mixer itself.
    if (pDevice->pMasterMixer != NULL) {
        dra_mixer_attach_submixer(pDevice->pMasterMixer, pMixer);
    }

    *ppMixer = pMixer;
    return DRA_RESULT_SUCCESS;
}

void dra_mixer_delete(dra_mixer* pMixer)
{
    if (pMixer == NULL) return;

    dra_mixer_detach_all_submixers(pMixer);
    dra_mixer_detach_all_voices(pMixer);

    if (pMixer->pParentMixer != NULL) {
        dra_mixer_detach_submixer(pMixer->pParentMixer, pMixer);
    }

    free(pMixer);
}

void dra_mixer_attach_submixer(dra_mixer* pMixer, dra_mixer* pSubmixer)
{
    if (pMixer == NULL || pSubmixer == NULL) {
        return;
    }

    if (pSubmixer->pParentMixer != NULL) {
        dra_mixer_detach_submixer(pSubmixer->pParentMixer, pSubmixer);
    }


    pSubmixer->pParentMixer = pMixer;

    if (pMixer->pFirstChildMixer == NULL) {
        pMixer->pFirstChildMixer = pSubmixer;
        pMixer->pLastChildMixer  = pSubmixer;
        return;
    }

    assert(pMixer->pLastChildMixer != NULL);
    pMixer->pLastChildMixer->pNextSiblingMixer = pSubmixer;
    pSubmixer->pPrevSiblingMixer = pMixer->pLastChildMixer;
    pSubmixer->pNextSiblingMixer = NULL;
    pMixer->pLastChildMixer = pSubmixer;
}

void dra_mixer_detach_submixer(dra_mixer* pMixer, dra_mixer* pSubmixer)
{
    if (pMixer == NULL || pSubmixer == NULL) {
        return;
    }

    if (pSubmixer->pParentMixer != pMixer) {
        return; // Doesn't have the same parent.
    }


    // Detach from parent.
    if (pSubmixer->pParentMixer->pFirstChildMixer == pSubmixer) {
        pSubmixer->pParentMixer->pFirstChildMixer = pSubmixer->pNextSiblingMixer;
    }
    if (pSubmixer->pParentMixer->pLastChildMixer == pSubmixer) {
        pSubmixer->pParentMixer->pLastChildMixer = pSubmixer->pPrevSiblingMixer;
    }

    pSubmixer->pParentMixer = NULL;


    // Detach from siblings.
    if (pSubmixer->pPrevSiblingMixer) {
        pSubmixer->pPrevSiblingMixer->pNextSiblingMixer = pSubmixer->pNextSiblingMixer;
    }
    if (pSubmixer->pNextSiblingMixer) {
        pSubmixer->pNextSiblingMixer->pPrevSiblingMixer = pSubmixer->pPrevSiblingMixer;
    }

    pSubmixer->pNextSiblingMixer = NULL;
    pSubmixer->pPrevSiblingMixer = NULL;
}

void dra_mixer_detach_all_submixers(dra_mixer* pMixer)
{
    if (pMixer == NULL) {
        return;
    }

    while (pMixer->pFirstChildMixer != NULL) {
        dra_mixer_detach_submixer(pMixer, pMixer->pFirstChildMixer);
    }
}

void dra_mixer_attach_voice(dra_mixer* pMixer, dra_voice* pVoice)
{
    if (pMixer == NULL || pVoice == NULL) {
        return;
    }

    if (pVoice->pMixer != NULL) {
        dra_mixer_detach_voice(pVoice->pMixer, pVoice);
    }

    pVoice->pMixer = pMixer;

    if (pMixer->pFirstVoice == NULL) {
        pMixer->pFirstVoice = pVoice;
        pMixer->pLastVoice  = pVoice;
        return;
    }

    // Attach the voice to the end of the list.
    pVoice->pPrevVoice = pMixer->pLastVoice;
    pVoice->pNextVoice = NULL;

    pMixer->pLastVoice->pNextVoice = pVoice;
    pMixer->pLastVoice = pVoice;
}

void dra_mixer_detach_voice(dra_mixer* pMixer, dra_voice* pVoice)
{
    if (pMixer == NULL || pVoice == NULL) {
        return;
    }


    // Detach from mixer.
    if (pMixer->pFirstVoice == pVoice) {
        pMixer->pFirstVoice = pMixer->pFirstVoice->pNextVoice;
    }
    if (pMixer->pLastVoice == pVoice) {
        pMixer->pLastVoice = pMixer->pLastVoice->pPrevVoice;
    }

    pVoice->pMixer = NULL;


    // Remove from list.
    if (pVoice->pNextVoice) {
        pVoice->pNextVoice->pPrevVoice = pVoice->pPrevVoice;
    }
    if (pVoice->pPrevVoice) {
        pVoice->pPrevVoice->pNextVoice = pVoice->pNextVoice;
    }

    pVoice->pNextVoice = NULL;
    pVoice->pPrevVoice = NULL;

}

void dra_mixer_detach_all_voices(dra_mixer* pMixer)
{
    if (pMixer == NULL) {
        return;
    }

    while (pMixer->pFirstVoice) {
        dra_mixer_detach_voice(pMixer, pMixer->pFirstVoice);
    }
}

void dra_mixer_set_volume(dra_mixer* pMixer, float linearVolume)
{
    if (pMixer == NULL) {
        return;
    }

    pMixer->linearVolume = linearVolume;
}

float dra_mixer_get_volume(dra_mixer* pMixer)
{
    if (pMixer == NULL) {
        return 0;
    }

    return pMixer->linearVolume;
}

size_t dra_mixer_mix_next_frames(dra_mixer* pMixer, size_t frameCount)
{
    if (pMixer == NULL) {
        return 0;
    }

    if (pMixer->pFirstVoice == NULL && pMixer->pFirstChildMixer == NULL) {
        return 0;
    }

    if (dra_mixer_is_paused(pMixer)) {
        return 0;
    }


    size_t framesMixed = 0;

    // Mixing works by simply adding together the sample data of each voice and submixer. We just start at 0 and then
    // just accumulate each one.
    memset(pMixer->pStagingBuffer, 0, frameCount * pMixer->pDevice->channels * sizeof(float));

    // Voices first. Doesn't really matter if we do voices or submixers first.
    for (dra_voice* pVoice = pMixer->pFirstVoice; pVoice != NULL; pVoice = pVoice->pNextVoice)
    {
        if (pVoice->isPlaying) {
            size_t framesJustRead = dra_voice__next_frames(pVoice, frameCount, pMixer->pNextSamplesToMix);
            for (size_t i = 0; i < framesJustRead * pMixer->pDevice->channels; ++i) {
                pMixer->pStagingBuffer[i] += (pMixer->pNextSamplesToMix[i] * pVoice->linearVolume);
            }

            // Has the end of the voice's buffer been reached?
            if (framesJustRead < frameCount)
            {
                // We'll get here if the end of the voice's buffer has been reached. The voice needs to be forcefully stopped to
                // ensure the device is aware of it and is able to put itself into a dormant state if necessary. Also note that
                // the playback position is moved back to start. The rationale for this is that it's a little bit more useful than
                // just leaving the playback position sitting on the end. Also it allows an application to restart playback with
                // a single call to dra_voice_play() without having to explicitly set the playback position.
                pVoice->currentReadPos = 0;
                dra_voice_stop(pVoice);
            }

            if (framesMixed < framesJustRead) {
                framesMixed = framesJustRead;
            }
        }
    }

    // Submixers.
    for (dra_mixer* pSubmixer = pMixer->pFirstChildMixer; pSubmixer != NULL; pSubmixer = pSubmixer->pNextSiblingMixer)
    {
        size_t framesJustMixed = dra_mixer_mix_next_frames(pSubmixer, frameCount);
        for (size_t i = 0; i < framesJustMixed * pMixer->pDevice->channels; ++i) {
            pMixer->pStagingBuffer[i] += pSubmixer->pStagingBuffer[i];
        }

        if (framesMixed < framesJustMixed) {
            framesMixed = framesJustMixed;
        }
    }


    // At this point the mixer's effects and volume need to be applied to each sample.
    size_t samplesMixed = framesMixed * pMixer->pDevice->channels;
    for (size_t i = 0; i < samplesMixed; ++i) {
        pMixer->pStagingBuffer[i] *= pMixer->linearVolume;
    }


    // Finally we need to ensure every samples is clamped to -1 to 1. There are two easy ways to do this: clamp or normalize. For now I'm just
    // clamping to keep it simple, but it might be valuable to make this configurable.
    for (size_t i = 0; i < framesMixed * pMixer->pDevice->channels; ++i)
    {
        // TODO: Investigate using SSE here (MINPS/MAXPS)
        // TODO: Investigate if the backends clamp the samples themselves, thus making this redundant.
        if (pMixer->pStagingBuffer[i] < -1) {
            pMixer->pStagingBuffer[i] = -1;
        } else if (pMixer->pStagingBuffer[i] > 1) {
            pMixer->pStagingBuffer[i] = 1;
        }
    }

    return framesMixed;
}

size_t dra_mixer_count_attached_voices(dra_mixer* pMixer)
{
    if (pMixer == NULL) {
        return 0;
    }

    size_t count = 0;
    for (dra_voice* pVoice = pMixer->pFirstVoice; pVoice != NULL; pVoice = pVoice->pNextVoice) {
        count += 1;
    }

    return count;
}

size_t dra_mixer_count_attached_voices_recursive(dra_mixer* pMixer)
{
    if (pMixer == NULL) {
        return 0;
    }

    size_t count = dra_mixer_count_attached_voices(pMixer);

    // Children.
    for (dra_mixer* pChildMixer = pMixer->pFirstChildMixer; pChildMixer != NULL; pChildMixer = pChildMixer->pNextSiblingMixer) {
        count += dra_mixer_count_attached_voices_recursive(pChildMixer);
    }

    return count;
}

size_t dra_mixer_gather_attached_voices(dra_mixer* pMixer, dra_voice** ppVoicesOut)
{
    if (pMixer == NULL) {
        return 0;
    }

    if (ppVoicesOut == NULL) {
        return dra_mixer_count_attached_voices(pMixer);
    }

    size_t count = 0;
    for (dra_voice* pVoice = pMixer->pFirstVoice; pVoice != NULL; pVoice = pVoice->pNextVoice) {
        ppVoicesOut[count] = pVoice;
        count += 1;
    }

    return count;
}

size_t dra_mixer_gather_attached_voices_recursive(dra_mixer* pMixer, dra_voice** ppVoicesOut)
{
    if (pMixer == NULL) {
        return 0;
    }

    if (ppVoicesOut == NULL) {
        return dra_mixer_count_attached_voices_recursive(pMixer);
    }

    size_t count = dra_mixer_gather_attached_voices(pMixer, ppVoicesOut);

    // Children.
    for (dra_mixer* pChildMixer = pMixer->pFirstChildMixer; pChildMixer != NULL; pChildMixer = pChildMixer->pNextSiblingMixer) {
        count += dra_mixer_gather_attached_voices_recursive(pChildMixer, ppVoicesOut + count);
    }

    return count;
}


void dra_mixer_pause(dra_mixer* pMixer)
{
    if (pMixer == NULL) return;
    pMixer->flags |= DRA_MIXER_FLAG_PAUSED;
}

void dra_mixer_resume(dra_mixer* pMixer)
{
    if (pMixer == NULL) return;
    pMixer->flags &= ~DRA_MIXER_FLAG_PAUSED;

    // When the mixer was paused it may have resulted in no audio being played which means dr_audio will have stopped the device
    // to save CPU usage. We need to make sure we wake up the device.
    dra_device__play(pMixer->pDevice);
}

dr_bool32 dra_mixer_is_paused(dra_mixer* pMixer)
{
    if (pMixer == NULL) return DR_FALSE;
    return (pMixer->flags & DRA_MIXER_FLAG_PAUSED) != 0;
}



dra_result dra_voice_create(dra_device* pDevice, dra_format format, unsigned int channels, unsigned int sampleRate, size_t sizeInBytes, const void* pInitialData, dra_voice** ppVoice)
{
    if (ppVoice == NULL) return DRA_RESULT_INVALID_ARGS;
    *ppVoice = NULL;

    if (pDevice == NULL || sizeInBytes == 0) return DRA_RESULT_INVALID_ARGS;


    // The number of bytes must be a multiple of the size of a frame.
    size_t bytesPerSample = dra_get_bytes_per_sample_by_format(format);
    if ((sizeInBytes % (bytesPerSample * channels)) != 0) {
        return DRA_RESULT_INVALID_ARGS;
    }


    dra_voice* pVoice = (dra_voice*)calloc(1, sizeof(*pVoice) + sizeInBytes);
    if (pVoice == NULL) {
        return DRA_RESULT_OUT_OF_MEMORY;
    }

    pVoice->pDevice = pDevice;
    pVoice->pMixer = NULL;
    pVoice->format = format;
    pVoice->channels = channels;
    pVoice->sampleRate = sampleRate;
    pVoice->linearVolume = 1;
    pVoice->isPlaying = DR_FALSE;
    pVoice->isLooping = DR_FALSE;
    pVoice->frameCount = sizeInBytes / (bytesPerSample * channels);
    pVoice->currentReadPos = 0;
    pVoice->sizeInBytes = sizeInBytes;
    pVoice->pNextVoice = NULL;
    pVoice->pPrevVoice = NULL;

    if (pInitialData != NULL) {
        memcpy(pVoice->pData, pInitialData, sizeInBytes);
    } else {
        //memset(pVoice->pData, 0, sizeInBytes); // <-- This is already zeroed by the calloc() above, but leaving this comment here for emphasis.
    }


    // Sample rate conversion.
    if (sampleRate == pDevice->sampleRate) {
        pVoice->src.algorithm = dra_src_algorithm_none;
    } else {
        pVoice->src.algorithm = dra_src_algorithm_linear;
    }


    // Attach the voice to the master mixer by default.
    if (pDevice->pMasterMixer != NULL) {
        dra_mixer_attach_voice(pDevice->pMasterMixer, pVoice);
    }

    *ppVoice = pVoice;
    return DRA_RESULT_SUCCESS;
}

dra_result dra_voice_create_compatible(dra_device* pDevice, size_t sizeInBytes, const void* pInitialData, dra_voice** ppVoice)
{
    return dra_voice_create(pDevice, dra_format_f32, pDevice->channels, pDevice->sampleRate, sizeInBytes, pInitialData, ppVoice);
}

void dra_voice_delete(dra_voice* pVoice)
{
    if (pVoice == NULL) return;

    // The voice needs to be stopped...
    dra_voice_stop(pVoice);

    // ... and all pending events need to be cancelled to ensure the application isn't notified of an event of a deleted voice.
    dra_event_queue__cancel_events_of_voice(&pVoice->pDevice->eventQueue, pVoice);

    if (pVoice->pMixer != NULL) {
        dra_mixer_detach_voice(pVoice->pMixer, pVoice);
    }

    free(pVoice);
}

void dra_voice_play(dra_voice* pVoice, dr_bool32 loop)
{
    if (pVoice == NULL) {
        return;
    }

    if (!dra_voice_is_playing(pVoice)) {
        dra_device__voice_playback_count_inc(pVoice->pDevice);
    } else {
        if (dra_voice_is_looping(pVoice) == loop) {
            return;     // Nothing has changed - don't need to do anything.
        }
    }

    pVoice->isPlaying = DR_TRUE;
    pVoice->isLooping = loop;

    dra_event_queue__schedule_event(&pVoice->pDevice->eventQueue, &pVoice->playEvent);

    // When playing a voice we need to ensure the backend device is playing.
    dra_device__play(pVoice->pDevice);
}

void dra_voice_stop(dra_voice* pVoice)
{
    if (pVoice == NULL) {
        return;
    }

    if (!dra_voice_is_playing(pVoice)) {
        return; // The voice is already stopped.
    }

    dra_device__voice_playback_count_dec(pVoice->pDevice);

    pVoice->isPlaying = DR_FALSE;
    pVoice->isLooping = DR_FALSE;

    dra_event_queue__schedule_event(&pVoice->pDevice->eventQueue, &pVoice->stopEvent);
}

dr_bool32 dra_voice_is_playing(dra_voice* pVoice)
{
    if (pVoice == NULL) {
        return DR_FALSE;
    }

    return pVoice->isPlaying;
}

dr_bool32 dra_voice_is_looping(dra_voice* pVoice)
{
    if (pVoice == NULL) {
        return DR_FALSE;
    }

    return pVoice->isLooping;
}


void dra_voice_set_volume(dra_voice* pVoice, float linearVolume)
{
    if (pVoice == NULL) {
        return;
    }

    pVoice->linearVolume = linearVolume;
}

float dra_voice_get_volume(dra_voice* pVoice)
{
    if (pVoice == NULL) {
        return 0;
    }

    return pVoice->linearVolume;
}


void dra_f32_to_f32(float* pOut, const float* pIn, size_t sampleCount)
{
    memcpy(pOut, pIn, sampleCount * sizeof(float));
}

void dra_s32_to_f32(float* pOut, const dr_int32* pIn, size_t sampleCount)
{
    // TODO: Try SSE-ifying this.
    for (size_t i = 0; i < sampleCount; ++i) {
        pOut[i] = pIn[i] / 2147483648.0f;
    }
}

void dra_s24_to_f32(float* pOut, const dr_uint8* pIn, size_t sampleCount)
{
    // TODO: Try SSE-ifying this.
    for (size_t i = 0; i < sampleCount; ++i) {
        dr_uint8 s0 = pIn[i*3 + 0];
        dr_uint8 s1 = pIn[i*3 + 1];
        dr_uint8 s2 = pIn[i*3 + 2];

        dr_int32 sample32 = (dr_int32)((s0 << 8) | (s1 << 16) | (s2 << 24));
        pOut[i] = sample32 / 2147483648.0f;
    }
}

void dra_s16_to_f32(float* pOut, const dr_int16* pIn, size_t sampleCount)
{
    // TODO: Try SSE-ifying this.
    for (size_t i = 0; i < sampleCount; ++i) {
        pOut[i] = pIn[i] / 32768.0f;
    }
}

void dra_u8_to_f32(float* pOut, const dr_uint8* pIn, size_t sampleCount)
{
    // TODO: Try SSE-ifying this.
    for (size_t i = 0; i < sampleCount; ++i) {
        pOut[i] = (pIn[i] / 127.5f) - 1;
    }
}


// Generic sample format conversion function. To add basic, unoptimized support for a new format, just add it to this function.
// Format-specific optimizations need to be implemented specifically for each format, at a higher level.
void dra_to_f32(float* pOut, const void* pIn, size_t sampleCount, dra_format format)
{
    switch (format)
    {
        case dra_format_f32:
        {
            dra_f32_to_f32(pOut, (float*)pIn, sampleCount);
        } break;

        case dra_format_s32:
        {
            dra_s32_to_f32(pOut, (dr_int32*)pIn, sampleCount);
        } break;

        case dra_format_s24:
        {
            dra_s24_to_f32(pOut, (dr_uint8*)pIn, sampleCount);
        } break;

        case dra_format_s16:
        {
            dra_s16_to_f32(pOut, (dr_int16*)pIn, sampleCount);
        } break;

        case dra_format_u8:
        {
            dra_u8_to_f32(pOut, (dr_uint8*)pIn, sampleCount);
        } break;

        default: break; // Unknown or unsupported format.
    }
}


// Notes on channel shuffling.
//
// Channels are shuffled frame-by-frame by first normalizing everything to floats. Then, a shuffling function is called to
// shuffle the channels in a particular way depending on the destination and source channel assignments.
void dra_shuffle_channels__generic_inc(float* pOut, const float* pIn, unsigned int channelsOut, unsigned int channelsIn)
{
    // This is the generic function for taking a frame with a smaller number of channels and expanding it to a frame with
    // a greater number of channels. This just copies the first channelsIn samples to the output and silences the remaing
    // channels.
    assert(channelsOut > channelsIn);

    for (unsigned int i = 0; i < channelsIn; ++i) {
        pOut[i] = pIn[i];
    }

    // Silence the left over.
    for (unsigned int i = channelsIn; i < channelsOut; ++i) {
        pOut[i] = 0;
    }
}

void dra_shuffle_channels__generic_dec(float* pOut, const float* pIn, unsigned int channelsOut, unsigned int channelsIn)
{
    // This is the opposite of dra_shuffle_channels__generic_inc() - it decreases the number of channels in the input stream
    // by simply stripping the excess channels.
    assert(channelsOut < channelsIn);
    (void)channelsIn;

    // Just copy the first channelsOut.
    for (unsigned int i = 0; i < channelsOut; ++i) {
        pOut[i] = pIn[i];
    }
}

void dra_shuffle_channels(float* pOut, const float* pIn, unsigned int channelsOut, unsigned int channelsIn)
{
    assert(channelsOut != 0);
    assert(channelsIn  != 0);

    if (channelsOut == channelsIn) {
        for (unsigned int i = 0; i < channelsOut; ++i) {
            pOut[i] = pIn[i];
        }
    } else {
        switch (channelsIn)
        {
            case 1:
            {
                // Mono input. This is a simple case - just copy the value of the mono channel to every output channel.
                for (unsigned int i = 0; i < channelsOut; ++i) {
                    pOut[i] = pIn[0];
                }
            } break;

            case 2:
            {
                // Stereo input.
                if (channelsOut == 1)
                {
                    // For mono output, just average.
                    pOut[0] = (pIn[0] + pIn[1]) * 0.5f;
                }
                else
                {
                    // TODO: Do a specialized implementation for all major formats, in particluar 5.1.
                    dra_shuffle_channels__generic_inc(pOut, pIn, channelsOut, channelsIn);
                }
            } break;

            default:
            {
                if (channelsOut == 1)
                {
                    // For mono output, just average each sample.
                    float total = 0;
                    for (unsigned int i = 0; i < channelsIn; ++i) {
                        total += pIn[i];
                    }

                    pOut[0] = total / channelsIn;
                }
                else
                {
                    if (channelsOut > channelsIn) {
                        dra_shuffle_channels__generic_inc(pOut, pIn, channelsOut, channelsIn);
                    } else {
                        dra_shuffle_channels__generic_dec(pOut, pIn, channelsOut, channelsIn);
                    }
                }
            } break;
        }
    }
}

float dra_voice__get_sample_rate_factor(dra_voice* pVoice)
{
    if (pVoice == NULL) {
        return 1;
    }

    return pVoice->pDevice->sampleRate / (float)pVoice->sampleRate;
}

void dra_voice__unsignal_playback_events(dra_voice* pVoice)
{
    // This function will be called when the voice has looped back to the start. In this case the playback notification events need
    // to be marked as unsignaled so that they're able to be fired again.
    for (size_t i = 0; i < pVoice->playbackEventCount; ++i) {
        pVoice->playbackEvents[i].hasBeenSignaled = DR_FALSE;
    }
}

float* dra_voice__next_frame(dra_voice* pVoice)
{
    if (pVoice == NULL) {
        return NULL;
    }


    if (pVoice->format == dra_format_f32 && pVoice->sampleRate == pVoice->pDevice->sampleRate && pVoice->channels == pVoice->pDevice->channels)
    {
        // Fast path.
        if (!pVoice->isLooping && pVoice->currentReadPos == pVoice->frameCount) {
            return NULL;    // At the end of a non-looping voice.
        }

        float* pOut = (float*)pVoice->pData + (pVoice->currentReadPos * pVoice->channels);

        pVoice->currentReadPos += 1;
        if (pVoice->currentReadPos == pVoice->frameCount && pVoice->isLooping) {
            pVoice->currentReadPos = 0;
            dra_voice__unsignal_playback_events(pVoice);
        }

        return pOut;
    }
    else
    {
        size_t bytesPerSample = dra_get_bytes_per_sample_by_format(pVoice->format);

        if (pVoice->sampleRate == pVoice->pDevice->sampleRate)
        {
            // Same sample rate. This path isn't ideal, but it's not too bad since there is no need for sample rate conversion.
            if (!pVoice->isLooping && pVoice->currentReadPos == pVoice->frameCount) {
                return NULL;    // At the end of a non-looping voice.
            }

            float* pOut = pVoice->convertedFrame;

            unsigned int channelsIn  = pVoice->channels;
            unsigned int channelsOut = pVoice->pDevice->channels;
            float tempFrame[DR_AUDIO_MAX_CHANNEL_COUNT];
            dr_uint64 sampleOffset = pVoice->currentReadPos * channelsIn;

            // The conversion is done differently depending on the format of the voice.
            if (pVoice->format == dra_format_f32) {
                dra_shuffle_channels(pOut, (float*)pVoice->pData + sampleOffset, channelsOut, channelsIn);
            } else {
                sampleOffset = pVoice->currentReadPos * (channelsIn * bytesPerSample);
                dra_to_f32(tempFrame, (dr_uint8*)pVoice->pData + sampleOffset, channelsIn, pVoice->format);
                dra_shuffle_channels(pOut, tempFrame, channelsOut, channelsIn);
            }

            pVoice->currentReadPos += 1;
            if (pVoice->currentReadPos == pVoice->frameCount && pVoice->isLooping) {
                pVoice->currentReadPos = 0;
                dra_voice__unsignal_playback_events(pVoice);
            }

            return pOut;
        }
        else
        {
            // Different sample rate. This is the truly slow path.
            unsigned int sampleRateIn  = pVoice->sampleRate;
            unsigned int sampleRateOut = pVoice->pDevice->sampleRate;
            unsigned int channelsIn    = pVoice->channels;
            unsigned int channelsOut   = pVoice->pDevice->channels;

            float factor = (float)sampleRateOut / (float)sampleRateIn;
            float invfactor = 1 / factor;

            if (!pVoice->isLooping && pVoice->currentReadPos >= (pVoice->frameCount * factor)) {
                return NULL;    // At the end of a non-looping voice.
            }

            float* pOut = pVoice->convertedFrame;

            if (pVoice->src.algorithm == dra_src_algorithm_linear) {
                // Linear filtering.
                float timeIn = pVoice->currentReadPos * invfactor;
                dr_uint64 prevFrameIndexIn = (dr_uint64)(timeIn);
                dr_uint64 nextFrameIndexIn = prevFrameIndexIn + 1;
                if (nextFrameIndexIn >= pVoice->frameCount) {
                    nextFrameIndexIn  = pVoice->frameCount-1;
                }

                if (prevFrameIndexIn != pVoice->src.data.linear.prevFrameIndex)
                {
                    dr_uint64 sampleOffset = prevFrameIndexIn * (channelsIn * bytesPerSample);
                    dra_to_f32(pVoice->src.data.linear.prevFrame, (dr_uint8*)pVoice->pData + sampleOffset, channelsIn, pVoice->format);

                    sampleOffset = nextFrameIndexIn * (channelsIn * bytesPerSample);
                    dra_to_f32(pVoice->src.data.linear.nextFrame, (dr_uint8*)pVoice->pData + sampleOffset, channelsIn, pVoice->format);

                    pVoice->src.data.linear.prevFrameIndex = prevFrameIndexIn;
                }

                float alpha = timeIn - prevFrameIndexIn;
                float frame[DR_AUDIO_MAX_CHANNEL_COUNT];
                for (unsigned int i = 0; i < pVoice->channels; ++i) {
                    frame[i] = dra_mixf(pVoice->src.data.linear.prevFrame[i], pVoice->src.data.linear.nextFrame[i], alpha);
                }

                dra_shuffle_channels(pOut, frame, channelsOut, channelsIn);
            }


            pVoice->currentReadPos += 1;
            if (pVoice->currentReadPos >= (pVoice->frameCount * factor) && pVoice->isLooping) {
                pVoice->currentReadPos = 0;
                dra_voice__unsignal_playback_events(pVoice);
            }

            return pOut;
        }
    }
}

size_t dra_voice__next_frames(dra_voice* pVoice, size_t frameCount, float* pSamplesOut)
{
    // TODO: Check for the fast path and do a bulk copy rather than frame-by-frame. Don't forget playback event handling.

    size_t framesRead = 0;

    dr_uint64 prevReadPosLocal = pVoice->currentReadPos * pVoice->channels;

    float* pNextFrame = NULL;
    while ((framesRead < frameCount) && (pNextFrame = dra_voice__next_frame(pVoice)) != NULL) {
        memcpy(pSamplesOut, pNextFrame, pVoice->pDevice->channels * sizeof(float));
        pSamplesOut += pVoice->pDevice->channels;
        framesRead += 1;
    }

    float sampleRateFactor = dra_voice__get_sample_rate_factor(pVoice);
    dr_uint64 totalSampleCount = (dr_uint64)((pVoice->frameCount * pVoice->channels) * sampleRateFactor);

    // Now we need to check if we've got past any notification events and post events for them if so.
    dr_uint64 currentReadPosLocal = (prevReadPosLocal + (framesRead * pVoice->channels)) % totalSampleCount;
    for (size_t i = 0; i < pVoice->playbackEventCount; ++i) {
        dra__event* pEvent = &pVoice->playbackEvents[i];
        if (!pEvent->hasBeenSignaled && pEvent->sampleIndex*sampleRateFactor <= currentReadPosLocal) {
            dra_event_queue__schedule_event(&pVoice->pDevice->eventQueue, pEvent);  // <-- TODO: Check that this really needs to be scheduled. Can probably call it directly and avoid a mutex lock/unlock.
            pEvent->hasBeenSignaled = DR_TRUE;
        }
    }

    return framesRead;
}


void dra_voice_set_on_stop(dra_voice* pVoice, dra_event_proc proc, void* pUserData)
{
    if (pVoice == NULL) {
        return;
    }

    pVoice->stopEvent.id = DR_AUDIO_EVENT_ID_STOP;
    pVoice->stopEvent.pUserData = pUserData;
    pVoice->stopEvent.sampleIndex = 0;
    pVoice->stopEvent.proc = proc;
    pVoice->stopEvent.pVoice = pVoice;
}

void dra_voice_set_on_play(dra_voice* pVoice, dra_event_proc proc, void* pUserData)
{
    if (pVoice == NULL) {
        return;
    }

    pVoice->playEvent.id = DR_AUDIO_EVENT_ID_PLAY;
    pVoice->playEvent.pUserData = pUserData;
    pVoice->playEvent.sampleIndex = 0;
    pVoice->playEvent.proc = proc;
    pVoice->playEvent.pVoice = pVoice;
}

dr_bool32 dra_voice_add_playback_event(dra_voice* pVoice, dr_uint64 sampleIndex, dr_uint64 eventID, dra_event_proc proc, void* pUserData)
{
    if (pVoice == NULL) {
        return DR_FALSE;
    }

    if (pVoice->playbackEventCount >= DR_AUDIO_MAX_EVENT_COUNT) {
        return DR_FALSE;
    }

    pVoice->playbackEvents[pVoice->playbackEventCount].id = eventID;
    pVoice->playbackEvents[pVoice->playbackEventCount].pUserData = pUserData;
    pVoice->playbackEvents[pVoice->playbackEventCount].sampleIndex = sampleIndex;
    pVoice->playbackEvents[pVoice->playbackEventCount].proc = proc;
    pVoice->playbackEvents[pVoice->playbackEventCount].pVoice = pVoice;

    pVoice->playbackEventCount += 1;
    return DR_TRUE;
}

void dra_voice_remove_playback_event(dra_voice* pVoice, dr_uint64 eventID)
{
    if (pVoice == NULL) {
        return;
    }

    for (size_t i = 0; i < pVoice->playbackEventCount; /* DO NOTHING */) {
        if (pVoice->playbackEvents[i].id == eventID) {
            memmove(&pVoice->playbackEvents[i], &pVoice->playbackEvents[i + 1], (pVoice->playbackEventCount - (i+1)) * sizeof(dra__event));
            pVoice->playbackEventCount -= 1;
        } else {
            i += 1;
        }
    }
}


dr_uint64 dra_voice_get_playback_position(dra_voice* pVoice)
{
    if (pVoice == NULL) {
        return 0;
    }

    return (dr_uint64)((pVoice->currentReadPos * pVoice->channels) / dra_voice__get_sample_rate_factor(pVoice));
}

void dra_voice_set_playback_position(dra_voice* pVoice, dr_uint64 sampleIndex)
{
    if (pVoice == NULL) {
        return;
    }

    // When setting the playback position it's important to consider sample-rate conversion. Sample rate conversion will often depend on
    // previous and next frames in order to calculate the next frame. Therefore, depending on the type of SRC we're using, we'll need to
    // seek a few frames earlier and then re-fill the delay-line buffer used for a particular SRC algorithm.
    dr_uint64 localFramePos = sampleIndex / pVoice->channels;
    pVoice->currentReadPos = (dr_uint64)(localFramePos * dra_voice__get_sample_rate_factor(pVoice));

    if (pVoice->sampleRate != pVoice->pDevice->sampleRate) {
        if (pVoice->src.algorithm == dra_src_algorithm_linear) {
            // Linear filtering just requires the previous frame. However, this is handled at mixing time for linear SRC so all we need to
            // do is ensure the mixing function is aware that the previous frame need to be re-read. This is done by simply resetting the
            // variable the mixer uses to determine whether or not the previous frame needs to be re-read.
            pVoice->src.data.linear.prevFrameIndex = 0;
        }
    }

    // TODO: Normalize the hasBeenSignaled properties of events.
}


void* dra_voice_get_buffer_ptr_by_sample(dra_voice* pVoice, dr_uint64 sample)
{
    if (pVoice == NULL) {
        return NULL;
    }

    dr_uint64 totalSampleCount = pVoice->frameCount * pVoice->channels;
    if (sample > totalSampleCount) {
        return NULL;
    }

    return pVoice->pData + (sample * dra_get_bytes_per_sample_by_format(pVoice->format));
}

void dra_voice_write_silence(dra_voice* pVoice, dr_uint64 sampleOffset, dr_uint64 sampleCount)
{
    void* pData = dra_voice_get_buffer_ptr_by_sample(pVoice, sampleOffset);
    if (pData == NULL) {
        return;
    }

    dr_uint64 totalSamplesRemaining = (pVoice->frameCount * pVoice->channels) - sampleOffset;
    if (sampleCount > totalSamplesRemaining) {
        sampleCount = totalSamplesRemaining;
    }

    memset(pData, 0, (size_t)(sampleCount * dra_get_bytes_per_sample_by_format(pVoice->format)));
}






//// Other APIs ////

void dra_free(void* p)
{
    free(p);
}

unsigned int dra_get_bits_per_sample_by_format(dra_format format)
{
    unsigned int lookup[] = {
        8,      // dra_format_u8
        16,     // dra_format_s16
        24,     // dra_format_s24
        32,     // dra_format_s32
        32      // dra_format_f32
    };

    return lookup[format];
}

unsigned int dra_get_bytes_per_sample_by_format(dra_format format)
{
    return dra_get_bits_per_sample_by_format(format) / 8;
}


//// STDIO ////

#ifndef DR_AUDIO_NO_STDIO
static FILE* dra__fopen(const char* filePath)
{
    FILE* pFile;
#ifdef _MSC_VER
    if (fopen_s(&pFile, filePath, "rb") != 0) {
        return NULL;
    }
#else
    pFile = fopen(filePath, "rb");
    if (pFile == NULL) {
        return NULL;
    }
#endif

    return (FILE*)pFile;
}
#endif  //DR_AUDIO_NO_STDIO


//// Decoder APIs ////

#ifdef DR_AUDIO_HAS_WAV
size_t dra_decoder_on_read__wav(void* pUserData, void* pDataOut, size_t bytesToRead)
{
    dra_decoder* pDecoder = (dra_decoder*)pUserData;
    assert(pDecoder != NULL);
    assert(pDecoder->onRead != NULL);

    return pDecoder->onRead(pDecoder->pUserData, pDataOut, bytesToRead);
}
drwav_bool32 dra_decoder_on_seek__wav(void* pUserData, int offset, drwav_seek_origin origin)
{
    dra_decoder* pDecoder = (dra_decoder*)pUserData;
    assert(pDecoder != NULL);
    assert(pDecoder->onSeek != NULL);

    return pDecoder->onSeek(pDecoder->pUserData, offset, (origin == drwav_seek_origin_start) ? dra_seek_origin_start : dra_seek_origin_current);
}

void dra_decoder_on_delete__wav(void* pBackendDecoder)
{
    drwav* pWav = (drwav*)pBackendDecoder;
    assert(pWav != NULL);

    drwav_close(pWav);
}

dr_uint64 dra_decoder_on_read_samples__wav(void* pBackendDecoder, dr_uint64 samplesToRead, float* pSamplesOut)
{
    drwav* pWav = (drwav*)pBackendDecoder;
    assert(pWav != NULL);

    return drwav_read_f32(pWav, samplesToRead, pSamplesOut);
}

dr_bool32 dra_decoder_on_seek_samples__wav(void* pBackendDecoder, dr_uint64 sample)
{
    drwav* pWav = (drwav*)pBackendDecoder;
    assert(pWav != NULL);

    return drwav_seek_to_sample(pWav, sample);
}


void dra_decoder_init__wav(dra_decoder* pDecoder, drwav* pWav)
{
    assert(pDecoder != NULL);
    assert(pWav != NULL);

    pDecoder->channels = pWav->channels;
    pDecoder->sampleRate = pWav->sampleRate;
    pDecoder->totalSampleCount = pWav->totalSampleCount;

    pDecoder->pBackendDecoder = pWav;
    pDecoder->onDelete = dra_decoder_on_delete__wav;
    pDecoder->onReadSamples = dra_decoder_on_read_samples__wav;
    pDecoder->onSeekSamples = dra_decoder_on_seek_samples__wav;
}

dr_bool32 dra_decoder_open__wav(dra_decoder* pDecoder)
{
    drwav* pWav = drwav_open(dra_decoder_on_read__wav, dra_decoder_on_seek__wav, pDecoder);
    if (pWav == NULL) {
        return DR_FALSE;
    }

    dra_decoder_init__wav(pDecoder, pWav);
    return DR_TRUE;
}

dr_bool32 dra_decoder_open_memory__wav(dra_decoder* pDecoder, const void* pData, size_t dataSize)
{
    drwav* pWav = drwav_open_memory(pData, dataSize);
    if (pWav == NULL) {
        return DR_FALSE;
    }

    dra_decoder_init__wav(pDecoder, pWav);
    return DR_TRUE;
}

#ifdef DR_AUDIO_HAS_WAV_STDIO
dr_bool32 dra_decoder_open_file__wav(dra_decoder* pDecoder, const char* filePath)
{
    drwav* pWav = drwav_open_file(filePath);
    if (pWav == NULL) {
        return DR_FALSE;
    }

    dra_decoder_init__wav(pDecoder, pWav);
    return DR_TRUE;
}
#endif
#endif  //WAV

#ifdef DR_AUDIO_HAS_FLAC
size_t dra_decoder_on_read__flac(void* pUserData, void* pDataOut, size_t bytesToRead)
{
    dra_decoder* pDecoder = (dra_decoder*)pUserData;
    assert(pDecoder != NULL);
    assert(pDecoder->onRead != NULL);

    return pDecoder->onRead(pDecoder->pUserData, pDataOut, bytesToRead);
}
drflac_bool32 dra_decoder_on_seek__flac(void* pUserData, int offset, drflac_seek_origin origin)
{
    dra_decoder* pDecoder = (dra_decoder*)pUserData;
    assert(pDecoder != NULL);
    assert(pDecoder->onSeek != NULL);

    return pDecoder->onSeek(pDecoder->pUserData, offset, (origin == drflac_seek_origin_start) ? dra_seek_origin_start : dra_seek_origin_current);
}

void dra_decoder_on_delete__flac(void* pBackendDecoder)
{
    drflac* pFlac = (drflac*)pBackendDecoder;
    assert(pFlac != NULL);

    drflac_close(pFlac);
}

dr_uint64 dra_decoder_on_read_samples__flac(void* pBackendDecoder, dr_uint64 samplesToRead, float* pSamplesOut)
{
    drflac* pFlac = (drflac*)pBackendDecoder;
    assert(pFlac != NULL);

    dr_uint64 samplesRead = drflac_read_s32(pFlac, samplesToRead, (dr_int32*)pSamplesOut);

    dra_s32_to_f32(pSamplesOut, (dr_int32*)pSamplesOut, (size_t)samplesRead);
    return samplesRead;
}

dr_bool32 dra_decoder_on_seek_samples__flac(void* pBackendDecoder, dr_uint64 sample)
{
    drflac* pFlac = (drflac*)pBackendDecoder;
    assert(pFlac != NULL);

    return drflac_seek_to_sample(pFlac, sample);
}


void dra_decoder_init__flac(dra_decoder* pDecoder, drflac* pFlac)
{
    assert(pDecoder != NULL);
    assert(pFlac != NULL);

    pDecoder->channels = pFlac->channels;
    pDecoder->sampleRate = pFlac->sampleRate;
    pDecoder->totalSampleCount = pFlac->totalSampleCount;

    pDecoder->pBackendDecoder = pFlac;
    pDecoder->onDelete = dra_decoder_on_delete__flac;
    pDecoder->onReadSamples = dra_decoder_on_read_samples__flac;
    pDecoder->onSeekSamples = dra_decoder_on_seek_samples__flac;
}

dr_bool32 dra_decoder_open__flac(dra_decoder* pDecoder)
{
    drflac* pFlac = drflac_open(dra_decoder_on_read__flac, dra_decoder_on_seek__flac, pDecoder);
    if (pFlac == NULL) {
        return DR_FALSE;
    }

    dra_decoder_init__flac(pDecoder, pFlac);
    return DR_TRUE;
}

dr_bool32 dra_decoder_open_memory__flac(dra_decoder* pDecoder, const void* pData, size_t dataSize)
{
    drflac* pFlac = drflac_open_memory(pData, dataSize);
    if (pFlac == NULL) {
        return DR_FALSE;
    }

    dra_decoder_init__flac(pDecoder, pFlac);
    return DR_TRUE;
}

#ifdef DR_AUDIO_HAS_FLAC_STDIO
dr_bool32 dra_decoder_open_file__flac(dra_decoder* pDecoder, const char* filePath)
{
    drflac* pFlac = drflac_open_file(filePath);
    if (pFlac == NULL) {
        return DR_FALSE;
    }

    dra_decoder_init__flac(pDecoder, pFlac);
    return DR_TRUE;
}
#endif
#endif  //FLAC

#ifdef DR_AUDIO_HAS_VORBIS
void dra_decoder_on_delete__vorbis(void* pBackendDecoder)
{
    stb_vorbis* pVorbis = (stb_vorbis*)pBackendDecoder;
    assert(pVorbis != NULL);

    stb_vorbis_close(pVorbis);
}

dr_uint64 dra_decoder_on_read_samples__vorbis(void* pBackendDecoder, dr_uint64 samplesToRead, float* pSamplesOut)
{
    stb_vorbis* pVorbis = (stb_vorbis*)pBackendDecoder;
    assert(pVorbis != NULL);

    stb_vorbis_info info = stb_vorbis_get_info(pVorbis);
    return (dr_uint64)stb_vorbis_get_samples_float_interleaved(pVorbis, info.channels, pSamplesOut, (int)samplesToRead) * info.channels;
}

dr_bool32 dra_decoder_on_seek_samples__vorbis(void* pBackendDecoder, dr_uint64 sample)
{
    stb_vorbis* pVorbis = (stb_vorbis*)pBackendDecoder;
    assert(pVorbis != NULL);

    return stb_vorbis_seek(pVorbis, (unsigned int)sample) != 0;
}


void dra_decoder_init__vorbis(dra_decoder* pDecoder, stb_vorbis* pVorbis)
{
    assert(pDecoder != NULL);
    assert(pVorbis != NULL);

    stb_vorbis_info info = stb_vorbis_get_info(pVorbis);

    pDecoder->channels = info.channels;
    pDecoder->sampleRate = info.sample_rate;
    pDecoder->totalSampleCount = stb_vorbis_stream_length_in_samples(pVorbis) * info.channels;

    pDecoder->pBackendDecoder = pVorbis;
    pDecoder->onDelete = dra_decoder_on_delete__vorbis;
    pDecoder->onReadSamples = dra_decoder_on_read_samples__vorbis;
    pDecoder->onSeekSamples = dra_decoder_on_seek_samples__vorbis;
}

dr_bool32 dra_decoder_open__vorbis(dra_decoder* pDecoder)
{
    // TODO: Add support for the push API.

    // Not currently supporting callback based decoding.
    (void)pDecoder;
    return DR_FALSE;
}

dr_bool32 dra_decoder_open_memory__vorbis(dra_decoder* pDecoder, const void* pData, size_t dataSize)
{
    stb_vorbis* pVorbis = stb_vorbis_open_memory((const unsigned char*)pData, (int)dataSize, NULL, NULL);
    if (pVorbis == NULL) {
        return DR_FALSE;
    }

    dra_decoder_init__vorbis(pDecoder, pVorbis);
    return DR_TRUE;
}

#ifdef DR_AUDIO_HAS_VORBIS_STDIO
dr_bool32 dra_decoder_open_file__vorbis(dra_decoder* pDecoder, const char* filePath)
{
    stb_vorbis* pVorbis = stb_vorbis_open_filename(filePath, NULL, NULL);
    if (pVorbis == NULL) {
        return DR_FALSE;
    }

    dra_decoder_init__vorbis(pDecoder, pVorbis);
    return DR_TRUE;
}
#endif
#endif  //Vorbis

dra_result dra_decoder_open(dra_decoder* pDecoder, dra_decoder_on_read_proc onRead, dra_decoder_on_seek_proc onSeek, void* pUserData)
{
    if (pDecoder == NULL) return DRA_RESULT_INVALID_ARGS;
    memset(pDecoder, 0, sizeof(*pDecoder));

    if (onRead == NULL || onSeek == NULL) return DRA_RESULT_INVALID_ARGS;

    pDecoder->onRead = onRead;
    pDecoder->onSeek = onSeek;
    pDecoder->pUserData = pUserData;

#ifdef DR_AUDIO_HAS_WAV_STDIO
    if (dra_decoder_open__wav(pDecoder)) {
        return DRA_RESULT_SUCCESS;
    }
    onSeek(pUserData, 0, dra_seek_origin_start);
#endif
#ifdef DR_AUDIO_HAS_FLAC_STDIO
    if (dra_decoder_open__flac(pDecoder)) {
        return DRA_RESULT_SUCCESS;
    }
    onSeek(pUserData, 0, dra_seek_origin_start);
#endif
#ifdef DR_AUDIO_HAS_VORBIS_STDIO
    if (dra_decoder_open__vorbis(pDecoder)) {
        return DRA_RESULT_SUCCESS;
    }
    onSeek(pUserData, 0, dra_seek_origin_start);
#endif

    // If we get here it means we were unable to open a decoder.
    return DRA_RESULT_NO_DECODER;
}


size_t dra_decoder__on_read_memory(void* pUserData, void* pDataOut, size_t bytesToRead)
{
    dra__memory_stream* memoryStream = (dra__memory_stream*)pUserData;
    assert(memoryStream != NULL);
    assert(memoryStream->dataSize >= memoryStream->currentReadPos);

    size_t bytesRemaining = memoryStream->dataSize - memoryStream->currentReadPos;
    if (bytesToRead > bytesRemaining) {
        bytesToRead = bytesRemaining;
    }

    if (bytesToRead > 0) {
        memcpy(pDataOut, memoryStream->data + memoryStream->currentReadPos, bytesToRead);
        memoryStream->currentReadPos += bytesToRead;
    }

    return bytesToRead;
}
dr_bool32 dra_decoder__on_seek_memory(void* pUserData, int offset, dra_seek_origin origin)
{
    dra__memory_stream* memoryStream = (dra__memory_stream*)pUserData;
    assert(memoryStream != NULL);
    assert(offset > 0 || (offset == 0 && origin == dra_seek_origin_start));

    if (origin == dra_seek_origin_current) {
        if (memoryStream->currentReadPos + offset <= memoryStream->dataSize) {
            memoryStream->currentReadPos += offset;
        } else {
            memoryStream->currentReadPos = memoryStream->dataSize;  // Trying to seek too far forward.
        }
    } else {
        if ((dr_uint32)offset <= memoryStream->dataSize) {
            memoryStream->currentReadPos = offset;
        } else {
            memoryStream->currentReadPos = memoryStream->dataSize;  // Trying to seek too far forward.
        }
    }

    return DR_TRUE;
}

dra_result dra_decoder_open_memory(dra_decoder* pDecoder, const void* pData, size_t dataSize)
{
    if (pDecoder == NULL) return DRA_RESULT_INVALID_ARGS;
    memset(pDecoder, 0, sizeof(*pDecoder));

    if (pData == NULL || dataSize == 0) return DRA_RESULT_INVALID_ARGS;

    // Prefer the backend's native APIs.
#if defined(DR_AUDIO_HAS_WAV)
    if (dra_decoder_open_memory__wav(pDecoder, pData, dataSize)) {
        return DRA_RESULT_SUCCESS;
    }
#endif
#if defined(DR_AUDIO_HAS_FLAC)
    if (dra_decoder_open_memory__flac(pDecoder, pData, dataSize)) {
        return DRA_RESULT_SUCCESS;
    }
#endif
#if defined(DR_AUDIO_HAS_VORBIS)
    if (dra_decoder_open_memory__vorbis(pDecoder, pData, dataSize)) {
        return DRA_RESULT_SUCCESS;
    }
#endif

    // If we get here it means the backend does not have a native memory loader so we'll need to it ourselves.
    dra__memory_stream memoryStream;
    memoryStream.data = (const unsigned char*)pData;
    memoryStream.dataSize = dataSize;
    memoryStream.currentReadPos = 0;
    dra_result result = dra_decoder_open(pDecoder, dra_decoder__on_read_memory, dra_decoder__on_seek_memory, &memoryStream);
    if (result != DRA_RESULT_SUCCESS) {
        return result;
    }

    pDecoder->memoryStream = memoryStream;
    pDecoder->pUserData = &pDecoder->memoryStream;

    return DRA_RESULT_SUCCESS;
}


#ifndef DR_AUDIO_NO_STDIO
size_t dra_decoder__on_read_stdio(void* pUserData, void* pDataOut, size_t bytesToRead)
{
    return fread(pDataOut, 1, bytesToRead, (FILE*)pUserData);
}
dr_bool32 dra_decoder__on_seek_stdio(void* pUserData, int offset, dra_seek_origin origin)
{
    return fseek((FILE*)pUserData, offset, (origin == dra_seek_origin_current) ? SEEK_CUR : SEEK_SET) == 0;
}

dra_result dra_decoder_open_file(dra_decoder* pDecoder, const char* filePath)
{
    if (pDecoder == NULL) return DRA_RESULT_INVALID_ARGS;
    memset(pDecoder, 0, sizeof(*pDecoder));

    if (filePath == NULL) return DR_FALSE;

    // When opening a decoder from a file it's preferrable to use the backend's native file IO APIs if it has them.
#if defined(DR_AUDIO_HAS_WAV) && defined(DR_AUDIO_HAS_WAV_STDIO)
    if (dra_decoder_open_file__wav(pDecoder, filePath)) {
        return DRA_RESULT_SUCCESS;
    }
#endif
#if defined(DR_AUDIO_HAS_FLAC) && defined(DR_AUDIO_HAS_FLAC_STDIO)
    if (dra_decoder_open_file__flac(pDecoder, filePath)) {
        return DRA_RESULT_SUCCESS;
    }
#endif
#if defined(DR_AUDIO_HAS_VORBIS) && defined(DR_AUDIO_HAS_VORBIS_STDIO)
    if (dra_decoder_open_file__vorbis(pDecoder, filePath)) {
        return DRA_RESULT_SUCCESS;
    }
#endif

    // If we get here it means we were unable to open the decoder using any of the backends' native file IO
    // APIs. In this case we fall back to a generic method.
    FILE* pFile = dra__fopen(filePath);
    if (pFile == NULL) {
        return DRA_RESULT_FAILED_TO_OPEN_FILE;
    }

    dra_result result = dra_decoder_open(pDecoder, dra_decoder__on_read_stdio, dra_decoder__on_seek_stdio, pFile);
    if (result != DRA_RESULT_SUCCESS) {
        fclose(pFile);
        return result;
    }

    return DRA_RESULT_SUCCESS;
}
#endif

void dra_decoder_close(dra_decoder* pDecoder)
{
    if (pDecoder == NULL) {
        return;
    }

    if (pDecoder->onDelete) {
        pDecoder->onDelete(pDecoder->pBackendDecoder);
    }

#ifndef DR_AUDIO_NO_STDIO
    if (pDecoder->onRead == dra_decoder__on_read_stdio) {
        fclose((FILE*)pDecoder->pUserData);
    }
#endif
}

dr_uint64 dra_decoder_read_f32(dra_decoder* pDecoder, dr_uint64 samplesToRead, float* pSamplesOut)
{
    if (pDecoder == NULL || pSamplesOut == NULL) {
        return 0;
    }

    return pDecoder->onReadSamples(pDecoder->pBackendDecoder, samplesToRead, pSamplesOut);
}

dr_bool32 dra_decoder_seek_to_sample(dra_decoder* pDecoder, dr_uint64 sample)
{
    if (pDecoder == NULL) {
        return DR_FALSE;
    }

    return pDecoder->onSeekSamples(pDecoder->pBackendDecoder, sample);
}


float* dra_decoder__full_decode_and_close(dra_decoder* pDecoder, unsigned int* channelsOut, unsigned int* sampleRateOut, dr_uint64* totalSampleCountOut)
{
    assert(pDecoder != NULL);

    float* pSampleData = NULL;
    dr_uint64 totalSampleCount = pDecoder->totalSampleCount;

    if (totalSampleCount == 0)
    {
        float buffer[4096];

        size_t sampleDataBufferSize = sizeof(buffer);
        pSampleData = (float*)malloc(sampleDataBufferSize);
        if (pSampleData == NULL) {
            goto on_error;
        }

        dr_uint64 samplesRead;
        while ((samplesRead = (dr_uint64)dra_decoder_read_f32(pDecoder, sizeof(buffer)/sizeof(buffer[0]), buffer)) > 0)
        {
            if (((totalSampleCount + samplesRead) * sizeof(float)) > sampleDataBufferSize) {
                sampleDataBufferSize *= 2;
                float* pNewSampleData = (float*)realloc(pSampleData, sampleDataBufferSize);
                if (pNewSampleData == NULL) {
                    free(pSampleData);
                    goto on_error;
                }

                pSampleData = pNewSampleData;
            }

            memcpy(pSampleData + totalSampleCount, buffer, (size_t)(samplesRead*sizeof(float)));
            totalSampleCount += samplesRead;
        }

        // At this point everything should be decoded, but we just want to fill the unused part buffer with silence - need to
        // protect those ears from random noise!
        memset(pSampleData + totalSampleCount, 0, (size_t)(sampleDataBufferSize - totalSampleCount*sizeof(float)));
    }
    else
    {
        dr_uint64 dataSize = totalSampleCount * sizeof(float);
        if (dataSize > SIZE_MAX) {
            goto on_error;  // The decoded data is too big.
        }

        pSampleData = (float*)malloc((size_t)dataSize);    // <-- Safe cast as per the check above.
        if (pSampleData == NULL) {
            goto on_error;
        }

        dr_uint64 samplesDecoded = dra_decoder_read_f32(pDecoder, pDecoder->totalSampleCount, pSampleData);
        if (samplesDecoded != pDecoder->totalSampleCount) {
            free(pSampleData);
            goto on_error;  // Something went wrong when decoding the FLAC stream.
        }
    }


    if (channelsOut) *channelsOut = pDecoder->channels;
    if (sampleRateOut) *sampleRateOut = pDecoder->sampleRate;
    if (totalSampleCountOut) *totalSampleCountOut = totalSampleCount;

    dra_decoder_close(pDecoder);
    return pSampleData;

on_error:
    dra_decoder_close(pDecoder);
    return NULL;
}

float* dra_decoder_open_and_decode_f32(dra_decoder_on_read_proc onRead, dra_decoder_on_seek_proc onSeek, void* pUserData, unsigned int* channels, unsigned int* sampleRate, dr_uint64* totalSampleCount)
{
    // Safety.
    if (channels) *channels = 0;
    if (sampleRate) *sampleRate = 0;
    if (totalSampleCount) *totalSampleCount = 0;

    dra_decoder decoder;
    dra_result result = dra_decoder_open(&decoder, onRead, onSeek, pUserData);
    if (result != DRA_RESULT_SUCCESS) {
        return NULL;
    }

    return dra_decoder__full_decode_and_close(&decoder, channels, sampleRate, totalSampleCount);
}

float* dra_decoder_open_and_decode_memory_f32(const void* pData, size_t dataSize, unsigned int* channels, unsigned int* sampleRate, dr_uint64* totalSampleCount)
{
    // Safety.
    if (channels) *channels = 0;
    if (sampleRate) *sampleRate = 0;
    if (totalSampleCount) *totalSampleCount = 0;

    dra_decoder decoder;
    dra_result result = dra_decoder_open_memory(&decoder, pData, dataSize);
    if (result != DRA_RESULT_SUCCESS) {
        return NULL;
    }

    return dra_decoder__full_decode_and_close(&decoder, channels, sampleRate, totalSampleCount);
}

#ifndef DR_AUDIO_NO_STDIO
float* dra_decoder_open_and_decode_file_f32(const char* filePath, unsigned int* channels, unsigned int* sampleRate, dr_uint64* totalSampleCount)
{
    // Safety.
    if (channels) *channels = 0;
    if (sampleRate) *sampleRate = 0;
    if (totalSampleCount) *totalSampleCount = 0;

    dra_decoder decoder;
    dra_result result = dra_decoder_open_file(&decoder, filePath);
    if (result != DRA_RESULT_SUCCESS) {
        return NULL;
    }

    return dra_decoder__full_decode_and_close(&decoder, channels, sampleRate, totalSampleCount);
}
#endif



//// High Level Helper APIs ////

#ifndef DR_AUDIO_NO_STDIO
dra_result dra_voice_create_from_file(dra_device* pDevice, const char* filePath, dra_voice** ppVoice)
{
    if (pDevice == NULL || filePath == NULL) return DRA_RESULT_INVALID_ARGS;

    unsigned int channels;
    unsigned int sampleRate;
    dr_uint64 totalSampleCount;
    float* pSampleData = dra_decoder_open_and_decode_file_f32(filePath, &channels, &sampleRate, &totalSampleCount);
    if (pSampleData == NULL) {
        return DRA_RESULT_UNKNOWN_ERROR;
    }

    dra_result result = dra_voice_create(pDevice, dra_format_f32, channels, sampleRate, (size_t)totalSampleCount * sizeof(float), pSampleData, ppVoice);

    free(pSampleData);
    return result;
}
#endif


//// High Level World APIs ////

dra_sound_world* dra_sound_world_create(dra_device* pPlaybackDevice)
{
    dra_sound_world* pWorld = (dra_sound_world*)calloc(1, sizeof(*pWorld));
    if (pWorld == NULL) {
        goto on_error;
    }

    pWorld->pPlaybackDevice = pPlaybackDevice;
    if (pWorld->pPlaybackDevice == NULL) {
        dra_result result = dra_device_create(NULL, dra_device_type_playback, &pWorld->pPlaybackDevice);
        if (result != DRA_RESULT_SUCCESS) {
            return NULL;
        }

        pWorld->ownsPlaybackDevice = DR_TRUE;
    }




    return pWorld;


on_error:
    dra_sound_world_delete(pWorld);
    return NULL;
}

void dra_sound_world_delete(dra_sound_world* pWorld)
{
    if (pWorld == NULL) {
        return;
    }

    if (pWorld->ownsPlaybackDevice) {
        dra_device_delete(pWorld->pPlaybackDevice);
    }

    free(pWorld);
}


void dra_sound_world__on_inline_sound_stop(dr_uint64 eventID, void* pUserData)
{
    (void)eventID;

    dra_sound* pSound = (dra_sound*)pUserData;
    assert(pSound != NULL);

    dra_sound_delete(pSound);
}

void dra_sound_world_play_inline(dra_sound_world* pWorld, dra_sound_desc* pDesc, dra_mixer* pMixer)
{
    if (pWorld == NULL || pDesc == NULL) {
        return;
    }

    // An inline sound is just like any other, except it never loops and it's deleted automatically when it stops playing. Therefore what
    // we need to do is attach an event handler to the voice's stop callback which is where the sound will be deleted.
    dra_sound* pSound = dra_sound_create(pWorld, pDesc);
    if (pSound == NULL) {
        return;
    }

    if (pMixer != NULL) {
        dra_sound_attach_to_mixer(pSound, pMixer);
    }

    dra_voice_set_on_stop(pSound->pVoice, dra_sound_world__on_inline_sound_stop, pSound);
    dra_sound_play(pSound, DR_FALSE);
}

void dra_sound_world_play_inline_3f(dra_sound_world* pWorld, dra_sound_desc* pDesc, dra_mixer* pMixer, float xPos, float yPos, float zPos)
{
    if (pWorld == NULL || pDesc == NULL) {
        return;
    }

    // TODO: Implement 3D positioning once the effects framework is in.
    (void)xPos;
    (void)yPos;
    (void)zPos;
    dra_sound_world_play_inline(pWorld, pDesc, pMixer);
}

void dra_sound_world_stop_all_sounds(dra_sound_world* pWorld)
{
    if (pWorld == NULL) {
        return;
    }

    // When sounds are stopped the on_stop event handler will be fired. It is possible for the implementation of this event handler to
    // delete the sound, so we'll first need to gather the sounds into a separate list.
    size_t voiceCount = dra_mixer_count_attached_voices_recursive(pWorld->pPlaybackDevice->pMasterMixer);
    if (voiceCount == 0) {
        return;
    }

    dra_voice** ppVoices = (dra_voice**)malloc(voiceCount * sizeof(*ppVoices));
    if (ppVoices == NULL) {
        return;
    }

    voiceCount = dra_mixer_gather_attached_voices_recursive(pWorld->pPlaybackDevice->pMasterMixer, ppVoices);
    assert(voiceCount != 0);

    for (size_t iVoice = 0; iVoice < voiceCount; ++iVoice) {
        dra_voice_stop(ppVoices[iVoice]);
    }

    free(ppVoices);
}

void dra_sound_world_set_listener_position(dra_sound_world* pWorld, float xPos, float yPos, float zPos)
{
    if (pWorld == NULL) {
        return;
    }

    // TODO: Implement me.
    (void)xPos;
    (void)yPos;
    (void)zPos;
}

void dra_sound_world_set_listener_orientation(dra_sound_world* pWorld, float xForward, float yForward, float zForward, float xUp, float yUp, float zUp)
{
    if (pWorld == NULL) {
        return;
    }

    // TODO: Implement me.
    (void)xForward;
    (void)yForward;
    (void)zForward;
    (void)xUp;
    (void)yUp;
    (void)zUp;
}


dr_bool32 dra_sound__is_streaming(dra_sound* pSound)
{
    assert(pSound != NULL);
    return pSound->desc.dataSize == 0 || pSound->desc.pData == NULL;
}

dr_bool32 dra_sound__read_next_chunk(dra_sound* pSound, dr_uint64 outputSampleOffset)
{
    assert(pSound != NULL);
    if (pSound->desc.onRead == NULL) {
        return DR_FALSE;
    }

    dr_uint64 chunkSizeInSamples = (pSound->pVoice->frameCount * pSound->pVoice->channels) / 2;
    assert(chunkSizeInSamples > 0);

    dr_uint64 samplesRead = pSound->desc.onRead(pSound, chunkSizeInSamples, dra_voice_get_buffer_ptr_by_sample(pSound->pVoice, outputSampleOffset));
    if (samplesRead == 0 && !pSound->isLooping) {
        dra_voice_write_silence(pSound->pVoice, outputSampleOffset, chunkSizeInSamples);
        return DR_FALSE;   // Ran out of samples in a non-looping buffer.
    }

    if (samplesRead == chunkSizeInSamples) {
        return DR_TRUE;
    }

    assert(samplesRead > 0);
    assert(samplesRead < chunkSizeInSamples);

    // Ran out of samples. If the sound is not looping it simply means the end of the data has been reached. The remaining samples need
    // to be zeroed out to create silence.
    if (!pSound->isLooping) {
        dra_voice_write_silence(pSound->pVoice, outputSampleOffset + samplesRead, chunkSizeInSamples - samplesRead);
        return DR_TRUE;
    }

    // At this point the sound will not be looping. We want to continuously loop back to the start and keep reading samples until the
    // chunk is filled.
    while (samplesRead < chunkSizeInSamples) {
        if (!pSound->desc.onSeek(pSound, 0)) {
            return DR_FALSE;
        }

        dr_uint64 samplesRemaining = chunkSizeInSamples - samplesRead;
        dr_uint64 samplesJustRead = pSound->desc.onRead(pSound, samplesRemaining, dra_voice_get_buffer_ptr_by_sample(pSound->pVoice, outputSampleOffset + samplesRead));
        if (samplesJustRead == 0) {
            return DR_FALSE;
        }

        samplesRead += samplesJustRead;
    }

    return DR_TRUE;
}

void dra_sound__on_read_next_chunk(dr_uint64 eventID, void* pUserData)
{
    dra_sound* pSound = (dra_sound*)pUserData;
    assert(pSound != NULL);

    if (pSound->stopOnNextChunk) {
        pSound->stopOnNextChunk = DR_FALSE;
        dra_sound_stop(pSound);
        return;
    }

    // The event ID is the index of the sample to write to.
    dr_uint64 sampleOffset = eventID;
    if (!dra_sound__read_next_chunk(pSound, sampleOffset)) {
        pSound->stopOnNextChunk = DR_TRUE;
    }
}


dra_sound* dra_sound_create(dra_sound_world* pWorld, dra_sound_desc* pDesc)
{
    if (pWorld == NULL) {
        return NULL;
    }

    dr_bool32 isStreaming = DR_FALSE;
    dra_result result = DRA_RESULT_SUCCESS;

    dra_sound* pSound = (dra_sound*)calloc(1, sizeof(*pSound));
    if (pSound == NULL) {
        result = DRA_RESULT_OUT_OF_MEMORY;
        goto on_error;
    }

    pSound->pWorld = pWorld;
    pSound->desc   = *pDesc;

    isStreaming = dra_sound__is_streaming(pSound);
    if (!isStreaming) {
        result = dra_voice_create(pWorld->pPlaybackDevice, pDesc->format, pDesc->channels, pDesc->sampleRate, pDesc->dataSize, pDesc->pData, &pSound->pVoice);
    } else {
        size_t streamingBufferSize = (pDesc->sampleRate * pDesc->channels) * 2;   // 2 seconds total, 1 second chunks. Keep total an even number and a multiple of the channel count.
        result = dra_voice_create(pWorld->pPlaybackDevice, pDesc->format, pDesc->channels, pDesc->sampleRate, streamingBufferSize * dra_get_bytes_per_sample_by_format(pDesc->format), NULL, &pSound->pVoice);

        // Streaming buffers require 2 playback events. As one is being played, the other is filled. The event ID is set to the sample
        // index of the next chunk that needs updating and is used in determining where to place new data.
        dra_voice_add_playback_event(pSound->pVoice, 0, streamingBufferSize/2, dra_sound__on_read_next_chunk, pSound);
        dra_voice_add_playback_event(pSound->pVoice, streamingBufferSize/2, 0, dra_sound__on_read_next_chunk, pSound);
    }

    if (result != DRA_RESULT_SUCCESS) {
        goto on_error;
    }

    pSound->pVoice->pUserData = pSound;


    // Streaming buffers need to have an initial chunk of data loaded before returning. This ensures the internal buffer contains valid audio data in
    // preparation for being played for the first time.
    if (isStreaming) {
        if (!dra_sound__read_next_chunk(pSound, 0)) {
            goto on_error;
        }
    }

    return pSound;

on_error:
    dra_sound_delete(pSound);
    return NULL;
}



void dra_sound__on_delete_decoder(dra_sound* pSound)
{
    dra_decoder* pDecoder = (dra_decoder*)pSound->desc.pUserData;
    assert(pDecoder != NULL);

    dra_decoder_close(pDecoder);
    free(pDecoder);
}

dr_uint64 dra_sound__on_read_decoder(dra_sound* pSound, dr_uint64 samplesToRead, void* pSamplesOut)
{
    dra_decoder* pDecoder = (dra_decoder*)pSound->desc.pUserData;
    assert(pDecoder != NULL);

    return dra_decoder_read_f32(pDecoder, samplesToRead, (float*)pSamplesOut);
}

dr_bool32 dra_sound__on_seek_decoder(dra_sound* pSound, dr_uint64 sample)
{
    dra_decoder* pDecoder = (dra_decoder*)pSound->desc.pUserData;
    assert(pDecoder != NULL);

    return dra_decoder_seek_to_sample(pDecoder, sample);
}

#ifndef DR_AUDIO_NO_STDIO
dra_sound* dra_sound_create_from_file(dra_sound_world* pWorld, const char* filePath)
{
    if (pWorld == NULL || filePath == NULL) {
        return NULL;
    }

    dra_decoder* pDecoder = (dra_decoder*)malloc(sizeof(*pDecoder));
    if (pDecoder == NULL) {
        return NULL;
    }

    dra_result result = dra_decoder_open_file(pDecoder, filePath);
    if (result != DRA_RESULT_SUCCESS) {
        free(pDecoder);
        return NULL;
    }

    dra_sound_desc desc;
    desc.format = dra_format_f32;
    desc.channels = pDecoder->channels;
    desc.sampleRate = pDecoder->sampleRate;
    desc.dataSize = 0;
    desc.pData = NULL;
    desc.onDelete = dra_sound__on_delete_decoder;
    desc.onRead = dra_sound__on_read_decoder;
    desc.onSeek = dra_sound__on_seek_decoder;
    desc.pUserData = pDecoder;

    dra_sound* pSound = dra_sound_create(pWorld, &desc);

    // After creating the sound, the audio data of a non-streaming voice can be deleted.
    if (desc.pData != NULL) {
        free(desc.pData);
    }

    return pSound;
}
#endif

void dra_sound_delete(dra_sound* pSound)
{
    if (pSound == NULL) {
        return;
    }

    if (pSound->pVoice != NULL) {
        dra_voice_delete(pSound->pVoice);
    }

    if (pSound->desc.onDelete) {
        pSound->desc.onDelete(pSound);
    }

    free(pSound);
}


void dra_sound_play(dra_sound* pSound, dr_bool32 loop)
{
    if (pSound == NULL) {
        return;
    }

    // The voice is always set to loop for streaming sounds.
    if (dra_sound__is_streaming(pSound)) {
        dra_voice_play(pSound->pVoice, DR_TRUE);
    } else {
        dra_voice_play(pSound->pVoice, loop);
    }

    pSound->isLooping = loop;
}

void dra_sound_stop(dra_sound* pSound)
{
    if (pSound == NULL) {
        return;
    }

    dra_voice_stop(pSound->pVoice);
}


void dra_sound_attach_to_mixer(dra_sound* pSound, dra_mixer* pMixer)
{
    if (pSound == NULL) {
        return;
    }

    if (pMixer == NULL) {
        pMixer = pSound->pWorld->pPlaybackDevice->pMasterMixer;
    }

    dra_mixer_attach_voice(pMixer, pSound->pVoice);
}


void dra_sound_set_on_stop(dra_sound* pSound, dra_event_proc proc, void* pUserData)
{
    dra_voice_set_on_stop(pSound->pVoice, proc, pUserData);
}

void dra_sound_set_on_play(dra_sound* pSound, dra_event_proc proc, void* pUserData)
{
    dra_voice_set_on_play(pSound->pVoice, proc, pUserData);
}

#endif  //DR_AUDIO_IMPLEMENTATION


// TODO
//
// - Forward declare every backend function and document them.
// - Add support for the push API in stb_vorbis.

// DEVELOPMENT NOTES AND BRAINSTORMING
//
// This is just random brainstorming and is likely very out of date and often just outright incorrect.
//
//
// Latency
//
// When a device is created it'll create a "hardware buffer" which is basically the buffer that the hardware
// device will read from when it needs to play audio. The hardware buffer is divided into two halves. As the
// buffer plays, it moves the playback pointer forward through the buffer and loops. When it hits the half
// way point it notifies the application that it needs more data to continue playing. Once one half starts
// playing the data within it is committed and cannot be changed. The size of each half determines the latency
// of the device.
//
// It sounds tempting to set this to something small like 1ms, but making it too small will
// increase the chance that the CPU won't be able to keep filling it with fresh data. In addition it will
// incrase overall CPU usage because operating system's scheduler will need to wake up the thread more often.
// Increasing the latency will increase memory usage and make playback of new sound sources take longer to
// begin playing. For example, if the latency was set to something like 1 second, a sound effect in a game
// may take up to a whole second to start playing. A balance needs to be made when setting the latency, and
// it can be configured when the device is created.
//
// (mention the fragments system to help avoiding the CPU running out of time to fill new audio data)
//
//
// Mixing
//
// Mixing is done via dra_mixer objects. Buffers can be attached to a mixer, but not more than one at a time.
// By default buffers are attached to a master mixer. Effects like volume can be applied on a per-buffer and
// per-mixer basis.
//
// Mixers can be chained together in a hierarchial manner. Child mixers will be mixed first with the result
// then passed on to higher level mixing. The master mixer is always the top level mixer.
//
// An obvious use case for mixers in games is to have separate volume controls for different categories of
// sounds, such as music, voices and sounds effects. Another example may be in music production where you may
// want to have separate mixers for vocal tracks, percussion tracks, etc.
//
// A mixer can be thought of as a complex buffer - it can be played/stopped/paused and have effects such as
// volume apllied to it. All of this affects all attached buffers and sub-mixers. You can, for example, pause
// every buffer attached to the mixer by simply pausing the mixer. This is an efficient and easy way to pause
// a group of audio buffers at the same time, such as when the user hits the pause button in a game.
//
// Every device includes a master mixer which is the one that buffers are automatically attached to. This one
// is intentionally hidden from the public facing API in order to keep it simpler. For basic audio playback
// using the master mixer will work just fine, however for more complex sound interactions you'll want to use
// your own mixers. Mixers, like buffers, are attached to the master mixer by default
//
//
// Thread Safety
//
// Everything in dr_audio should be thread-safe.
//
// Backends are implemented as if they are always run from a single thread. It's up to the cross-platform
// section to ensure thread-safety. This is an important property because if each backend is responsible for
// their own thread safety it increases the likelyhood of subtle backend-specific bugs.
//
// Every device has their own thread for asynchronous playback. This thread is created when the device is
// created, and deleted when the device is deleted.
//
// (Note edge cases when thread-safety may be an issue)


/*
This is free and unencumbered software released into the public domain.

Anyone is free to copy, modify, publish, use, compile, sell, or
distribute this software, either in source code form or as a compiled
binary, for any purpose, commercial or non-commercial, and by any
means.

In jurisdictions that recognize copyright laws, the author or authors
of this software dedicate any and all copyright interest in the
software to the public domain. We make this dedication for the benefit
of the public at large and to the detriment of our heirs and
successors. We intend this dedication to be an overt act of
relinquishment in perpetuity of all present and future rights to this
software under copyright law.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND,
EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF
MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT.
IN NO EVENT SHALL THE AUTHORS BE LIABLE FOR ANY CLAIM, DAMAGES OR
OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE,
ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR
OTHER DEALINGS IN THE SOFTWARE.

For more information, please refer to <http://unlicense.org/>
*/
