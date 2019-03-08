// Public domain. See "unlicense" statement at the end of this file.

//
// QUICK NOTES
//
// If you've stumbled across this library, be aware that this is very, very early in development. A lot of APIs
// are very temporary, in particular the effects API which at the moment are tied very closely to DirectSound.
//
// Currently, the only backend available is DirectSound while I figure out the API.
//
// General
// - This library is NOT thread safe. Functions can be called from any thread, but it is up to the host application
//   to do synchronization.
// - This library is only concerned with playback and recording of raw audio data. It does not load audio files
//   such as WAV, OGG and MP3.
// - Before you can create an output (playback) or input (recording) device you need to first create a context.
// - Each backend (DirectSound, ALSA, etc.) has it's own context. Using draudio_create_context() will find
//   a backend implementation based on the platform dr_audio has been compiled for.
// - A context for a specific backend can be created as well. For example, draudio_create_context_dsound() will
//   create a context which uses DirectSound as it's backend.
// - Currently, devices are enumerated once when the context is created. Thus, when a device is plugged in or
//   unplugged it will not be detected by dr_audio and the context will need to be deleted and re-created.
// - Currently, dr_audio will only consider the first DRAUDIO_MAX_DEVICE_COUNT output and input devices, which
//   is currently set to 16 and should be plenty for the vast majority of cases. Feel free to increase (or decrease)
//   this number to suit your own requirements.
//
// Events
// - Events are handled via callbacks. The different types of events include stop, pause, play and markers.
// - The Stop event is fired when an output buffer is stopped, either from finishing it's playback or if it was
//   stopped manually.
// - The Pause event is fired when the output buffer is paused.
// - The Play event is fired when the output buffer begins being played from either a stopped or paused state.
// - A Marker event is fired when the playback position of an output buffer reaches a certain point within the
//   buffer. This is useful for streaming audio data because it can tell you when a particular section of the
//   buffer can be filled with new data.
// - Due to the inherent multi-threaded nature of audio playback, events can be fired from any thread. It is up
//   to the application to ensure events are handled safely.
// - Currently, the maximum number of markers is set by DRAUDIO_MAX_MARKER_COUNT which is set to 4 by default. This
//   can be increased, however doing so increases memory usage for each sound buffer.
//
// Performance Considerations
// - Creating and deleting buffers should be considered an expensive operation because there is quite a bit of thread
//   management being performed under the hood. Prefer caching sound buffers.
//

//
// OPTIONS
//
// #define DRAUDIO_NO_DIRECTSOUND
//   Disables support for the DirectSound backend. Note that at the moment this is the only backend available for
//   Windows platforms, so you will likely not want to set this. DirectSound will only be compiled on Win32 builds.
//

//
// TODO
//
// - DirectSound: Consider using Win32 critical sections instead of events where possible.
// - DirectSound: Remove the semaphore and replace with an auto-reset event.
// - Implement a better error handling API.
// - Implement effects
// - Implement velocity
// - Implement cones
// - Implement attenuation min/max distances
//

#ifndef dr_audio_h
#define dr_audio_h

#ifdef __cplusplus
extern "C" {
#endif

#include <stdbool.h>
#include <stdint.h>


#define DRAUDIO_MAX_DEVICE_COUNT          16
#define DRAUDIO_MAX_MARKER_COUNT          4
#define DRAUDIO_MAX_MESSAGE_QUEUE_SIZE    1024        // The maximum number of messages that can be cached in the internal message queues.


#if defined(_WIN32) && !defined(DRAUDIO_NO_DIRECTSOUND)
#define DRAUDIO_BUILD_DSOUND
#endif


#define DRAUDIO_EVENT_ID_STOP     0xFFFFFFFF
#define DRAUDIO_EVENT_ID_PAUSE    0xFFFFFFFE
#define DRAUDIO_EVENT_ID_PLAY     0xFFFFFFFD
#define DRAUDIO_EVENT_ID_MARKER   0

#define DRAUDIO_ENABLE_3D         (1 << 0)
#define DRAUDIO_RELATIVE_3D       (1 << 1)        // <-- Uses relative 3D positioning by default instead of absolute. Only used if DRAUDIO_ENABLE_3D is also specified.


// Data formats.
typedef enum
{
    draudio_format_pcm,
    draudio_format_float

} draudio_format;

// Playback states.
typedef enum
{
    draudio_stopped,
    draudio_paused,
    draudio_playing

} draudio_playback_state;

// Effect types.
typedef enum
{
    draudio_effect_type_none,
    draudio_effect_type_i3dl2reverb,
    draudio_effect_type_chorus,
    draudio_effect_type_compressor,
    draudio_effect_type_distortion,
    draudio_effect_type_echo,
    draudio_effect_type_flanger

} draudio_effect_type;

// 3D processing modes.
typedef enum
{
    draudio_3d_mode_absolute,
    draudio_3d_mode_relative,
    draudio_3d_mode_disabled

} draudio_3d_mode;


typedef struct draudio_context draudio_context;
typedef struct draudio_device draudio_device;
typedef struct draudio_buffer draudio_buffer;

typedef void (* draudio_event_callback_proc)(draudio_buffer* pBuffer, unsigned int eventID, void *pUserData);

typedef struct
{
    /// The callback function.
    draudio_event_callback_proc callback;

    /// The user data.
    void* pUserData;

} draudio_event_callback;

typedef struct
{
    /// The description of the device.
    char description[256];

} draudio_device_info;

typedef struct
{
    /// Boolean flags.
    ///   DRAUDIO_ENABLE_3D: Enable 3D positioning
    unsigned int flags;

    /// The data format.
    draudio_format format;

    /// The number of channels. This should be 1 for mono, 2 for stereo.
    unsigned int channels;

    /// The sample rate.
    unsigned int sampleRate;

    /// The number of bits per sample.
    unsigned int bitsPerSample;

    /// The size in bytes of the data.
    size_t sizeInBytes;

    /// A pointer to the initial data.
    void* pData;

} draudio_buffer_desc;

typedef struct
{
    /// The effect type.
    draudio_effect_type type;

    struct
    {
        float room;
        float roomHF;
        float roomRolloffFactor;
        float decayTime;
        float reflections;
        float reflectionsDelay;
        float reverb;
        float reverbDelay;
        float diffusion;
        float density;
        float hfReference;
    } i3dl2reverb;

    struct
    {
        int waveform;
        int phase;
        float depth;
        float feedback;
        float frequency;
        float delay;
    } chorus;

    struct
    {
        float gain;
        float attack;
        float release;
        float threshold;
        float ratio;
        float predelay;
    } compressor;

    struct
    {
        float gain;
        float edge;
        float postEQCenterFrequency;
        float postEQBandwidth;
        float preLowpassCutoff;
    } distortion;

    struct
    {
        float wetDryMix;
        float feedback;
        float leftDelay;
        float rightDelay;
        float panDelay;
    } echo;

    struct
    {
        float wetDryMix;
        float depth;
        float feedback;
        float frequency;
        float waveform;
        float delay;
        float phase;
    } flanger;

} draudio_effect;


/// Creates a context which chooses an appropriate backend based on the given platform.
draudio_context* draudio_create_context();

#ifdef DRAUDIO_BUILD_DSOUND
/// Creates a context that uses DirectSound for it's backend.
draudio_context* draudio_create_context_dsound();
#endif

/// Deletes the given context.
void draudio_delete_context(draudio_context* pContext);




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// OUTPUT / PLAYBACK
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

/// Retrieves the number of output devices that were enumerated when the context was created.
unsigned int draudio_get_output_device_count(draudio_context* pContext);

/// Retrieves information about the device at the given index.
bool draudio_get_output_device_info(draudio_context* pContext, unsigned int deviceIndex, draudio_device_info* pInfoOut);


/// Creates a output device.
///
/// @param pContext    [in] A pointer to the main context.
/// @param deviceIndex [in] The index of the device.
///
/// @remarks
///     Use a device index of 0 to use the default output device.
draudio_device* draudio_create_output_device(draudio_context* pContext, unsigned int deviceIndex);

/// Deletes the given output device.
void draudio_delete_output_device(draudio_device* pDevice);


/// Create a buffer.
///
/// @remarks
///     This will fail if 3D positioning is requested when the sound has more than 1 channel.
draudio_buffer* draudio_create_buffer(draudio_device* pDevice, draudio_buffer_desc* pBufferDesc, size_t extraDataSize);

/// Deletes the given buffer.
void draudio_delete_buffer(draudio_buffer* pBuffer);


/// Retrieves the size in bytes of the given buffer's extra data.
unsigned int draudio_get_buffer_extra_data_size(draudio_buffer* pBuffer);

/// Retrieves a pointer to the given buffer's extra data.
void* draudio_get_buffer_extra_data(draudio_buffer* pBuffer);


/// Sets the audio data of the given buffer.
void draudio_set_buffer_data(draudio_buffer* pBuffer, size_t offset, const void* pData, size_t dataSizeInBytes);


/// Begins or resumes playing the given buffer.
///
/// @remarks
///     If the sound is already playing, it will continue to play, but the \c loop setting will be replaced with that specified
///     by the most recent call.
void draudio_play(draudio_buffer* pBuffer, bool loop);

/// Pauses playback of the given buffer.
void draudio_pause(draudio_buffer* pBuffer);

/// Stops playback of the given buffer.
void draudio_stop(draudio_buffer* pBuffer);

/// Retrieves the playback state of the given buffer.
draudio_playback_state draudio_get_playback_state(draudio_buffer* pBuffer);

/// Determines whether or not the given audio buffer is looping.
bool draudio_is_looping(draudio_buffer* pBuffer);


/// Sets the playback position for the given buffer.
void draudio_set_playback_position(draudio_buffer* pBuffer, unsigned int position);

/// Retrieves hte playback position of the given buffer.
unsigned int draudio_get_playback_position(draudio_buffer* pBuffer);


/// Sets the pan of the given buffer.
///
/// @remarks
///     This does nothing for 3D sounds.
void draudio_set_pan(draudio_buffer* pBuffer, float pan);

/// Retrieves the pan of the given buffer.
float draudio_get_pan(draudio_buffer* pBuffer);


/// Sets the volume of the given buffer.
///
/// @param volume [in] The new volume.
///
/// @remarks
///     Amplificiation is not currently supported, so the maximum value is 1. A value of 1 represents the volume of the original
///     data.
void draudio_set_volume(draudio_buffer* pBuffer, float volume);

/// Retrieves the volume of the sound.
float draudio_get_volume(draudio_buffer* pBuffer);


/// Removes every marker.
void draudio_remove_markers(draudio_buffer* pBuffer);

/// Registers the callback to fire when the playback position hits a certain position in the given buffer.
///
/// @param eventID [in] The event ID that will be passed to the callback and can be used to identify a specific marker.
///
/// @remarks
///     This will fail if the buffer is not in a stopped state.
///     @par
///     Set the event ID to DRAUDIO_EVENT_ID_MARKER + n, where "n" is your own application-specific identifier.
bool draudio_register_marker_callback(draudio_buffer* pBuffer, size_t offsetInBytes, draudio_event_callback_proc callback, unsigned int eventID, void* pUserData);

/// Registers the callback to fire when the buffer stops playing.
///
/// @remarks
///     This will fail if the buffer is not in a stopped state and the callback is non-null. It is fine to call this
///     with a null callback while the buffer is in the middle of playback in which case the callback will be cleared.
///     @par
///     The will replace any previous callback.
bool draudio_register_stop_callback(draudio_buffer* pBuffer, draudio_event_callback_proc callback, void* pUserData);

/// Registers the callback to fire when the buffer is paused.
///
/// @remarks
///     This will fail if the buffer is not in a stopped state and the callback is non-null. It is fine to call this
///     with a null callback while the buffer is in the middle of playback in which case the callback will be cleared.
///     @par
///     The will replace any previous callback.
bool draudio_register_pause_callback(draudio_buffer* pBuffer, draudio_event_callback_proc callback, void* pUserData);

/// Registers the callback to fire when the buffer begins playing from either a stopped or paused state.
///
/// @remarks
///     This will fail if the buffer is not in a stopped state and the callback is non-null. It is fine to call this
///     with a null callback while the buffer is in the middle of playback in which case the callback will be cleared.
///     @par
///     The will replace any previous callback.
bool draudio_register_play_callback(draudio_buffer* pBuffer, draudio_event_callback_proc callback, void* pUserData);


/// Retrieves the callback that is currently set for the stop event.
draudio_event_callback draudio_get_stop_callback(draudio_buffer* pBuffer);

/// Retrieves the callback that is currently set for the pause event.
draudio_event_callback draudio_get_pause_callback(draudio_buffer* pBuffer);

/// Retrieves the callback that is currently set for the play event.
draudio_event_callback draudio_get_play_callback(draudio_buffer* pBuffer);


/// Sets the position of the given buffer in 3D space.
///
/// @remarks
///     This does nothing for buffers that do not support 3D positioning.
void draudio_set_position(draudio_buffer* pBuffer, float x, float y, float z);

/// Retrieves the position of the given buffer in 3D space.
void draudio_get_position(draudio_buffer* pBuffer, float* pPosOut);

/// Sets the position of the listener for the given output device.
void draudio_set_listener_position(draudio_device* pDevice, float x, float y, float z);

/// Retrieves the position of the listner for the given output device.
void draudio_get_listener_position(draudio_device* pDevice, float* pPosOut);

/// Sets the orientation of the listener for the given output device.
void draudio_set_listener_orientation(draudio_device* pDevice, float forwardX, float forwardY, float forwardZ, float upX, float upY, float upZ);

/// Retrieves the orientation of the listener for the given output device.
void draudio_get_listener_orientation(draudio_device* pDevice, float* pForwardOut, float* pUpOut);


/// Sets the 3D processing mode (absolute, relative or disabled).
///
/// @remarks
///     This applies to position, orientation and velocity.
void draudio_set_3d_mode(draudio_buffer* pBuffer, draudio_3d_mode mode);

/// Retrieves the 3D processing mode (absolute, relative or disabled).
draudio_3d_mode draudio_get_3d_mode(draudio_buffer* pBuffer);



///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// INPUT / RECORDING
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////





///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// HIGH-LEVEL API
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// SYNCHRONIZATION ////

typedef void* draudio_mutex;

/// Creates a mutex object.
draudio_mutex draudio_create_mutex();

/// Deletes a mutex object.
void draudio_delete_mutex(draudio_mutex mutex);

/// Locks the given mutex.
void draudio_lock_mutex(draudio_mutex mutex);

/// Unlocks the given mutex.
void draudio_unlock_mutex(draudio_mutex mutex);




//// STREAMING ////
typedef int draudio_bool;

typedef draudio_bool (* draudio_stream_read_proc)(void* pUserData, void* pDataOut, size_t bytesToRead, size_t* bytesReadOut);
typedef draudio_bool (* draudio_stream_seek_proc)(void* pUserData, size_t offsetInBytesFromStart);

typedef struct
{
    /// A pointer to the user data to pass to each callback.
    void* pUserData;

    /// A pointer to the function to call when more data needs to be read.
    draudio_stream_read_proc read;

    /// Seeks source data from the beginning of the file.
    draudio_stream_seek_proc seek;

} draudio_streaming_callbacks;


/// Creates a buffer that's pre-configured for use for streaming audio data.
///
/// @remarks
///     This function is just a high-level convenience wrapper. The returned buffer is just a regular buffer with pre-configured
///     markers attached to the buffer. This will attach 3 markers in total which means there is only DRAUDIO_MAX_MARKER_COUNT - 3
///     marker slots available to the application.
///     @par
///     You must play the buffer with draudio_play_streaming_buffer() because the underlying buffer management is slightly different
///     to a regular buffer.
///     @par
///     Looping and stop callbacks may be inaccurate by up to half a second.
///     @par
///     Callback functions use bytes to determine how much data to process. This is always a multiple of samples * channels, however.
///     @par
///     The first chunk of data is not loaded until the buffer is played with draudio_play_streaming_buffer().
draudio_buffer* draudio_create_streaming_buffer(draudio_device* pDevice, draudio_buffer_desc* pBufferDesc, draudio_streaming_callbacks callbacks, unsigned int extraDataSize);

/// Retrieves the size of the extra data of the given streaming buffer..
size_t draudio_get_streaming_buffer_extra_data_size(draudio_buffer* pBuffer);

/// Retrieves a pointer to the extra data of the given streaming buffer.
void* draudio_get_streaming_buffer_extra_data(draudio_buffer* pBuffer);

/// Begins playing the given streaming buffer.
bool draudio_play_streaming_buffer(draudio_buffer* pBuffer, bool loop);

/// Determines whether or not the given streaming buffer is looping.
bool draudio_is_streaming_buffer_looping(draudio_buffer* pBuffer);





///////////////////////////////////////////////////////////////////////////////
//
// Sound World
//
// When a sound is created, whether or not it will be streamed is determined by
// whether or not a pointer to some initial data is specified when creating the
// sound. When initial data is specified, the sound data will be loaded into
// buffer once at creation time. If no data is specified, sound data will be
// loaded dynamically at playback time.
//
///////////////////////////////////////////////////////////////////////////////

typedef struct draudio_sound draudio_sound;
typedef struct draudio_world draudio_world;

typedef void (* draudio_on_sound_delete_proc)   (draudio_sound* pSound);
typedef bool (* draudio_on_sound_read_data_proc)(draudio_sound* pSound, void* pDataOut, size_t bytesToRead, size_t* bytesReadOut);
typedef bool (* draudio_on_sound_seek_data_proc)(draudio_sound* pSound, size_t offsetInBytesFromStart);

/// The structure that is used for creating a sound object.
typedef struct
{
    /// Boolean flags.
    ///   DRAUDIO_ENABLE_3D: Enable 3D positioning
    unsigned int flags;

    /// The data format.
    draudio_format format;

    /// The number of channels. This should be 1 for mono, 2 for stereo.
    unsigned int channels;

    /// The sample rate.
    unsigned int sampleRate;

    /// The number of bits per sample.
    unsigned int bitsPerSample;

    /// The size in bytes of the data. When this is non-zero, and pInitialData is non-null, the onRead and onSeek streaming
    /// callbacks are not used, and instead the sound's audio data is made up exclusively with this data.
    size_t sizeInBytes;

    /// A pointer to the initial data. Can be null, in which case the audio data is streamed with the onRead and onSeek
    /// callbacks below. It is an error for this to be null in addition to onRead and onSeek.
    void* pData;


    /// A pointer to the function to call when the sound is being deleted. This gives the application the opportunity
    /// to delete internal objects that are used for streaming or whatnot.
    draudio_on_sound_delete_proc onDelete;

    /// A pointer to the function to call when dr_audio needs to request a chunk of audio data. This is only used when
    /// streaming data.
    draudio_on_sound_read_data_proc onRead;

    /// A pointer to the function to call when dr_audio needs to seek the audio data. This is only used when streaming
    /// data.
    draudio_on_sound_seek_data_proc onSeek;


    /// The size of the extra data to associate with the sound. Extra data is how an application can link custom data to the
    /// sound object.
    unsigned int extraDataSize;

    /// A pointer to a buffer containing the initial extra data. This buffer is copied when the sound is initially created,
    /// and can be null.
    const void* pExtraData;

} draudio_sound_desc;

struct draudio_sound
{
    /// A pointer to the world that owns the sound.
    draudio_world* pWorld;

    /// A pointer to the audio buffer for playback.
    draudio_buffer* pBuffer;


    /// [Internal Use Only] The state of the buffer's playback at the time the associated world overwrote it.
    draudio_playback_state prevPlaybackState;

    /// [Internal Use Only] A pointer to the next sound in the local list.
    draudio_sound* pNextSound;

    /// [Internal Use Only] A pointer ot the previous sound in the local list.
    draudio_sound* pPrevSound;

    /// [Internal Use Only] Keeps track of whether or not a streaming buffer is being used.
    bool isUsingStreamingBuffer;

    /// [Internal Use Only] Keeps track of whether or not the sound has been marked for deletion. This is used to
    /// ensure onRead and onSeek are never called after the sound has been deleted. This scenario is possible because
    /// these functions are called in response to the sound buffer hitting markers which can be slightly delayed
    /// due to multi-threading synchronization.
    bool markedForDeletion;

    /// [Internal Use Only] the onDelete function. Can be null.
    draudio_on_sound_delete_proc onDelete;

    /// [Internal Use Only] The onRead streaming function. Can be null, in which case streaming will not be used.
    draudio_on_sound_read_data_proc onRead;

    /// [Internal Use Only] The onSeek streaming function. Can be null, in which case streaming will not be used.
    draudio_on_sound_seek_data_proc onSeek;
};

struct draudio_world
{
    /// A pointer to the dr_audio device to output audio to.
    draudio_device* pDevice;

    /// The global playback state of the world.
    draudio_playback_state playbackState;

    /// A pointer to the first sound in the local list of sounds.
    draudio_sound* pFirstSound;

    /// Mutex for thread-safety.
    draudio_mutex lock;
};


/// Creates a new sound world which will output audio from the given device.
draudio_world* draudio_create_world(draudio_device* pDevice);

/// Deletes a sound world that was previously created with draudio_create_world().
void draudio_delete_world(draudio_world* pWorld);


/// Creates a sound in 3D space.
draudio_sound* draudio_create_sound(draudio_world* pWorld, draudio_sound_desc desc);

/// Deletes a sound that was previously created with draudio_create_sound().
void draudio_delete_sound(draudio_sound* pSound);

/// Deletes every sound from the given world.
void draudio_delete_all_sounds(draudio_world* pWorld);


/// Retrieves the size in bytes of the given sound's extra data.
size_t draudio_get_sound_extra_data_size(draudio_sound* pSound);

/// Retrieves a pointer to the buffer containing the given sound's extra data.
void* draudio_get_sound_extra_data(draudio_sound* pSound);


/// Plays or resumes the given sound.
void draudio_play_sound(draudio_sound* pSound, bool loop);

/// Pauses playback the given sound.
void draudio_pause_sound(draudio_sound* pSound);

/// Stops playback of the given sound.
void draudio_stop_sound(draudio_sound* pSound);

/// Retrieves the playback state of the given sound.
draudio_playback_state draudio_get_sound_playback_state(draudio_sound* pSound);

/// Determines if the given sound is looping.
bool draudio_is_sound_looping(draudio_sound* pSound);


/// Begins playing a sound using the given streaming callbacks.
void draudio_play_inline_sound(draudio_world* pWorld, draudio_sound_desc desc);

/// Begins playing the given sound at the given position.
void draudio_play_inline_sound_3f(draudio_world* pWorld, draudio_sound_desc desc, float posX, float posY, float posZ);


/// Stops playback of all sounds in the given world.
void draudio_stop_all_sounds(draudio_world* pWorld);

/// Pauses playback of all sounds in the given world.
void draudio_pause_all_sounds(draudio_world* pWorld);

/// Resumes playback of all sounds in the given world.
void draudio_resume_all_sounds(draudio_world* pWorld);


/// Sets the callback for the stop event for the given sound.
void draudio_set_sound_stop_callback(draudio_sound* pSound, draudio_event_callback_proc callback, void* pUserData);

/// Sets the callback for the pause event for the given sound.
void draudio_set_sound_pause_callback(draudio_sound* pSound, draudio_event_callback_proc callback, void* pUserData);

/// Sets the callback for the play event for the given sound.
void draudio_set_sound_play_callback(draudio_sound* pSound, draudio_event_callback_proc callback, void* pUserData);


/// Sets the position of the given sound.
void draudio_set_sound_position(draudio_sound* pSound, float posX, float posY, float posZ);


/// Sets the 3D mode of the given sound (absolute positioning, relative positioning, no positioning).
void draudio_set_sound_3d_mode(draudio_sound* pSound, draudio_3d_mode mode);

/// Retrieves the 3D mode of the given sound.
draudio_3d_mode draudio_get_sound_3d_mode(draudio_sound* pSound);



#ifdef __cplusplus
}
#endif

#endif  //dr_audio_h

///////////////////////////////////////////////////////////////////////////////
//
// IMPLEMENTATION
//
///////////////////////////////////////////////////////////////////////////////
#ifdef DR_AUDIO_IMPLEMENTATION
#include <assert.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include <errno.h>


// Annotations
#ifndef DRAUDIO_PRIVATE
#define DRAUDIO_PRIVATE
#endif


////////////////////////////////////////////////////////
// Utilities

// strcpy()
int draudio_strcpy(char* dst, size_t dstSizeInBytes, const char* src)
{
#if defined(_MSC_VER)
    return strcpy_s(dst, dstSizeInBytes, src);
#else
    if (dst == 0) {
        return EINVAL;
    }
    if (dstSizeInBytes == 0) {
        return ERANGE;
    }
    if (src == 0) {
        dst[0] = '\0';
        return EINVAL;
    }

    char* iDst = dst;
    const char* iSrc = src;
    size_t remainingSizeInBytes = dstSizeInBytes;
    while (remainingSizeInBytes > 0 && iSrc[0] != '\0')
    {
        iDst[0] = iSrc[0];

        iDst += 1;
        iSrc += 1;
        remainingSizeInBytes -= 1;
    }

    if (remainingSizeInBytes > 0) {
        iDst[0] = '\0';
    } else {
        dst[0] = '\0';
        return ERANGE;
    }

    return 0;
#endif
}


typedef void                   (* draudio_delete_context_proc)(draudio_context* pContext);
typedef draudio_device*        (* draudio_create_output_device_proc)(draudio_context* pContext, unsigned int deviceIndex);
typedef void                   (* draudio_delete_output_device_proc)(draudio_device* pDevice);
typedef unsigned int           (* draudio_get_output_device_count_proc)(draudio_context* pContext);
typedef bool                   (* draudio_get_output_device_info_proc)(draudio_context* pContext, unsigned int deviceIndex, draudio_device_info* pInfoOut);
typedef draudio_buffer*        (* draudio_create_buffer_proc)(draudio_device* pDevice, draudio_buffer_desc* pBufferDesc, size_t extraDataSize);
typedef void                   (* draudio_delete_buffer_proc)(draudio_buffer* pBuffer);
typedef unsigned int           (* draudio_get_buffer_extra_data_size_proc)(draudio_buffer* pBuffer);
typedef void*                  (* draudio_get_buffer_extra_data_proc)(draudio_buffer* pBuffer);
typedef void                   (* draudio_set_buffer_data_proc)(draudio_buffer* pBuffer, size_t offset, const void* pData, size_t dataSizeInBytes);
typedef void                   (* draudio_play_proc)(draudio_buffer* pBuffer, bool loop);
typedef void                   (* draudio_pause_proc)(draudio_buffer* pBuffer);
typedef void                   (* draudio_stop_proc)(draudio_buffer* pBuffer);
typedef draudio_playback_state (* draudio_get_playback_state_proc)(draudio_buffer* pBuffer);
typedef void                   (* draudio_set_playback_position_proc)(draudio_buffer* pBuffer, unsigned int position);
typedef unsigned int           (* draudio_get_playback_position_proc)(draudio_buffer* pBuffer);
typedef void                   (* draudio_set_pan_proc)(draudio_buffer* pBuffer, float pan);
typedef float                  (* draudio_get_pan_proc)(draudio_buffer* pBuffer);
typedef void                   (* draudio_set_volume_proc)(draudio_buffer* pBuffer, float volume);
typedef float                  (* draudio_get_volume_proc)(draudio_buffer* pBuffer);
typedef void                   (* draudio_remove_markers_proc)(draudio_buffer* pBuffer);
typedef bool                   (* draudio_register_marker_callback_proc)(draudio_buffer* pBuffer, size_t offsetInBytes, draudio_event_callback_proc callback, unsigned int eventID, void* pUserData);
typedef bool                   (* draudio_register_stop_callback_proc)(draudio_buffer* pBuffer, draudio_event_callback_proc callback, void* pUserData);
typedef bool                   (* draudio_register_pause_callback_proc)(draudio_buffer* pBuffer, draudio_event_callback_proc callback, void* pUserData);
typedef bool                   (* draudio_register_play_callback_proc)(draudio_buffer* pBuffer, draudio_event_callback_proc callback, void* pUserData);
typedef void                   (* draudio_set_position_proc)(draudio_buffer* pBuffer, float x, float y, float z);
typedef void                   (* draudio_get_position_proc)(draudio_buffer* pBuffer, float* pPosOut);
typedef void                   (* draudio_set_listener_position_proc)(draudio_device* pDevice, float x, float y, float z);
typedef void                   (* draudio_get_listener_position_proc)(draudio_device* pDevice, float* pPosOut);
typedef void                   (* draudio_set_listener_orientation_proc)(draudio_device* pDevice, float forwardX, float forwardY, float forwardZ, float upX, float upY, float upZ);
typedef void                   (* draudio_get_listener_orientation_proc)(draudio_device* pDevice, float* pForwardOut, float* pUpOut);
typedef void                   (* draudio_set_3d_mode_proc)(draudio_buffer* pBuffer, draudio_3d_mode mode);
typedef draudio_3d_mode        (* draudio_get_3d_mode_proc)(draudio_buffer* pBuffer);

struct draudio_context
{
    // Callbacks.
    draudio_delete_context_proc delete_context;
    draudio_create_output_device_proc create_output_device;
    draudio_delete_output_device_proc delete_output_device;
    draudio_get_output_device_count_proc get_output_device_count;
    draudio_get_output_device_info_proc get_output_device_info;
    draudio_create_buffer_proc create_buffer;
    draudio_delete_buffer_proc delete_buffer;
    draudio_get_buffer_extra_data_size_proc get_buffer_extra_data_size;
    draudio_get_buffer_extra_data_proc get_buffer_extra_data;
    draudio_set_buffer_data_proc set_buffer_data;
    draudio_play_proc play;
    draudio_pause_proc pause;
    draudio_stop_proc stop;
    draudio_get_playback_state_proc get_playback_state;
    draudio_set_playback_position_proc set_playback_position;
    draudio_get_playback_position_proc get_playback_position;
    draudio_set_pan_proc set_pan;
    draudio_get_pan_proc get_pan;
    draudio_set_volume_proc set_volume;
    draudio_get_volume_proc get_volume;
    draudio_remove_markers_proc remove_markers;
    draudio_register_marker_callback_proc register_marker_callback;
    draudio_register_stop_callback_proc register_stop_callback;
    draudio_register_pause_callback_proc register_pause_callback;
    draudio_register_play_callback_proc register_play_callback;
    draudio_set_position_proc set_position;
    draudio_get_position_proc get_position;
    draudio_set_listener_position_proc set_listener_position;
    draudio_get_listener_position_proc get_listener_position;
    draudio_set_listener_orientation_proc set_listener_orientation;
    draudio_get_listener_orientation_proc get_listener_orientation;
    draudio_set_3d_mode_proc set_3d_mode;
    draudio_get_3d_mode_proc get_3d_mode;
};

struct draudio_device
{
    /// The context that owns this device.
    draudio_context* pContext;

    /// Whether or not the device is marked for deletion. A device is not always deleted straight away, so we use this
    /// to determine whether or not it has been marked for deletion.
    bool markedForDeletion;
};

struct draudio_buffer
{
    /// The device that owns this buffer.
    draudio_device* pDevice;

    /// The stop callback.
    draudio_event_callback stopCallback;

    /// The pause callback.
    draudio_event_callback pauseCallback;

    /// The play callback.
    draudio_event_callback playCallback;

    /// Whether or not playback is looping.
    bool isLooping;

    /// Whether or not the buffer is marked for deletion. A buffer is not always deleted straight away, so we use this
    /// to determine whether or not it has been marked for deletion.
    bool markedForDeletion;
};


draudio_context* draudio_create_context()
{
    draudio_context* pContext = NULL;

#ifdef DRAUDIO_BUILD_DSOUND
    pContext = draudio_create_context_dsound();
    if (pContext != NULL) {
        return pContext;
    }
#endif


    // If we get here it means we weren't able to create a context, so return NULL. Later on we're going to have a null implementation so that
    // we don't ever need to return NULL here.
    assert(pContext == NULL);
    return pContext;
}

void draudio_delete_context(draudio_context* pContext)
{
    if (pContext == NULL) {
        return;
    }

    pContext->delete_context(pContext);
}





///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// OUTPUT
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

unsigned int draudio_get_output_device_count(draudio_context* pContext)
{
    if (pContext == NULL) {
        return 0;
    }

    return pContext->get_output_device_count(pContext);
}

bool draudio_get_output_device_info(draudio_context* pContext, unsigned int deviceIndex, draudio_device_info* pInfoOut)
{
    if (pContext == NULL) {
        return false;
    }

    if (pInfoOut == NULL) {
        return false;
    }

    return pContext->get_output_device_info(pContext, deviceIndex, pInfoOut);
}


draudio_device* draudio_create_output_device(draudio_context* pContext, unsigned int deviceIndex)
{
    if (pContext == NULL) {
        return NULL;
    }

    draudio_device* pDevice = pContext->create_output_device(pContext, deviceIndex);
    if (pDevice != NULL)
    {
        pDevice->markedForDeletion = false;
    }

    return pDevice;
}

void draudio_delete_output_device(draudio_device* pDevice)
{
    if (pDevice == NULL) {
        return;
    }

    // If the device is already marked for deletion we just return straight away. However, this is an erroneous case so we trigger a failed assertion in this case.
    if (pDevice->markedForDeletion) {
        assert(false);
        return;
    }

    pDevice->markedForDeletion = true;

    assert(pDevice->pContext != NULL);
    pDevice->pContext->delete_output_device(pDevice);
}


draudio_buffer* draudio_create_buffer(draudio_device* pDevice, draudio_buffer_desc* pBufferDesc, size_t extraDataSize)
{
    if (pDevice == NULL) {
        return NULL;
    }

    if (pBufferDesc == NULL) {
        return NULL;
    }

    assert(pDevice->pContext != NULL);

    draudio_buffer* pBuffer = pDevice->pContext->create_buffer(pDevice, pBufferDesc, extraDataSize);
    if (pBuffer != NULL)
    {
        draudio_event_callback nullcb = {NULL, NULL};

        pBuffer->pDevice           = pDevice;
        pBuffer->stopCallback      = nullcb;
        pBuffer->pauseCallback     = nullcb;
        pBuffer->playCallback      = nullcb;
        pBuffer->isLooping         = false;
        pBuffer->markedForDeletion = false;
    }

    return pBuffer;
}

void draudio_delete_buffer(draudio_buffer* pBuffer)
{
    if (pBuffer == NULL) {
        return;
    }

    // We don't want to do anything if the buffer is marked for deletion.
    if (pBuffer->markedForDeletion) {
        assert(false);
        return;
    }

    pBuffer->markedForDeletion = true;


    // The sound needs to be stopped first.
    draudio_stop(pBuffer);

    // Now we need to remove every event.
    draudio_remove_markers(pBuffer);
    draudio_register_stop_callback(pBuffer, NULL, NULL);
    draudio_register_pause_callback(pBuffer, NULL, NULL);
    draudio_register_play_callback(pBuffer, NULL, NULL);


    assert(pBuffer->pDevice != NULL);
    assert(pBuffer->pDevice->pContext != NULL);
    pBuffer->pDevice->pContext->delete_buffer(pBuffer);
}


unsigned int draudio_get_buffer_extra_data_size(draudio_buffer* pBuffer)
{
    if (pBuffer == NULL) {
        return 0;
    }

    assert(pBuffer->pDevice != NULL);
    assert(pBuffer->pDevice->pContext != NULL);
    return pBuffer->pDevice->pContext->get_buffer_extra_data_size(pBuffer);
}

void* draudio_get_buffer_extra_data(draudio_buffer* pBuffer)
{
    if (pBuffer == NULL) {
        return NULL;
    }

    assert(pBuffer->pDevice != NULL);
    assert(pBuffer->pDevice->pContext != NULL);
    return pBuffer->pDevice->pContext->get_buffer_extra_data(pBuffer);
}


void draudio_set_buffer_data(draudio_buffer* pBuffer, size_t offset, const void* pData, size_t dataSizeInBytes)
{
    if (pBuffer == NULL) {
        return;
    }

    if (pData == NULL) {
        return;
    }

    assert(pBuffer->pDevice != NULL);
    assert(pBuffer->pDevice->pContext != NULL);
    pBuffer->pDevice->pContext->set_buffer_data(pBuffer, offset, pData, dataSizeInBytes);
}

void draudio_play(draudio_buffer* pBuffer, bool loop)
{
    if (pBuffer == NULL) {
        return;
    }

    pBuffer->isLooping = loop;

    assert(pBuffer->pDevice != NULL);
    assert(pBuffer->pDevice->pContext != NULL);
    pBuffer->pDevice->pContext->play(pBuffer, loop);
}

void draudio_pause(draudio_buffer* pBuffer)
{
    if (pBuffer == NULL) {
        return;
    }

    assert(pBuffer->pDevice != NULL);
    assert(pBuffer->pDevice->pContext != NULL);
    pBuffer->pDevice->pContext->pause(pBuffer);
}

void draudio_stop(draudio_buffer* pBuffer)
{
    if (pBuffer == NULL) {
        return;
    }

    assert(pBuffer->pDevice != NULL);
    assert(pBuffer->pDevice->pContext != NULL);
    pBuffer->pDevice->pContext->stop(pBuffer);
}

draudio_playback_state draudio_get_playback_state(draudio_buffer* pBuffer)
{
    if (pBuffer == NULL) {
        return draudio_stopped;
    }

    assert(pBuffer->pDevice != NULL);
    assert(pBuffer->pDevice->pContext != NULL);
    return pBuffer->pDevice->pContext->get_playback_state(pBuffer);
}

bool draudio_is_looping(draudio_buffer* pBuffer)
{
    if (pBuffer == NULL) {
        return false;
    }

    return pBuffer->isLooping;
}


void draudio_set_playback_position(draudio_buffer* pBuffer, unsigned int position)
{
    if (pBuffer == NULL) {
        return;
    }

    assert(pBuffer->pDevice != NULL);
    assert(pBuffer->pDevice->pContext != NULL);
    pBuffer->pDevice->pContext->set_playback_position(pBuffer, position);
}

unsigned int draudio_get_playback_position(draudio_buffer* pBuffer)
{
    if (pBuffer == NULL) {
        return 0;
    }

    assert(pBuffer->pDevice != NULL);
    assert(pBuffer->pDevice->pContext != NULL);
    return pBuffer->pDevice->pContext->get_playback_position(pBuffer);
}


void draudio_set_pan(draudio_buffer* pBuffer, float pan)
{
    if (pBuffer == NULL) {
        return;
    }

    assert(pBuffer->pDevice != NULL);
    assert(pBuffer->pDevice->pContext != NULL);
    pBuffer->pDevice->pContext->set_pan(pBuffer, pan);
}

float draudio_get_pan(draudio_buffer* pBuffer)
{
    if (pBuffer == NULL) {
        return 0.0f;
    }

    assert(pBuffer->pDevice != NULL);
    assert(pBuffer->pDevice->pContext != NULL);
    return pBuffer->pDevice->pContext->get_pan(pBuffer);
}


void draudio_set_volume(draudio_buffer* pBuffer, float volume)
{
    if (pBuffer == NULL) {
        return;
    }

    assert(pBuffer->pDevice != NULL);
    assert(pBuffer->pDevice->pContext != NULL);
    pBuffer->pDevice->pContext->set_volume(pBuffer, volume);
}

float draudio_get_volume(draudio_buffer* pBuffer)
{
    if (pBuffer == NULL) {
        return 1.0f;
    }

    assert(pBuffer->pDevice != NULL);
    assert(pBuffer->pDevice->pContext != NULL);
    return pBuffer->pDevice->pContext->get_volume(pBuffer);
}


void draudio_remove_markers(draudio_buffer* pBuffer)
{
    if (pBuffer == NULL) {
        return;
    }

    assert(pBuffer->pDevice != NULL);
    assert(pBuffer->pDevice->pContext != NULL);
    pBuffer->pDevice->pContext->remove_markers(pBuffer);
}

bool draudio_register_marker_callback(draudio_buffer* pBuffer, size_t offsetInBytes, draudio_event_callback_proc callback, unsigned int eventID, void* pUserData)
{
    if (pBuffer == NULL) {
        return false;
    }

    if (eventID == DRAUDIO_EVENT_ID_STOP ||
        eventID == DRAUDIO_EVENT_ID_PAUSE ||
        eventID == DRAUDIO_EVENT_ID_PLAY)
    {
        return false;
    }

    if (draudio_get_playback_state(pBuffer) != draudio_stopped) {
        return false;
    }

    assert(pBuffer->pDevice != NULL);
    assert(pBuffer->pDevice->pContext != NULL);
    return pBuffer->pDevice->pContext->register_marker_callback(pBuffer, offsetInBytes, callback, eventID, pUserData);
}

bool draudio_register_stop_callback(draudio_buffer* pBuffer, draudio_event_callback_proc callback, void* pUserData)
{
    if (pBuffer == NULL) {
        return false;
    }

    if (callback != NULL && draudio_get_playback_state(pBuffer) != draudio_stopped) {
        return false;
    }

    pBuffer->stopCallback.callback  = callback;
    pBuffer->stopCallback.pUserData = pUserData;

    assert(pBuffer->pDevice != NULL);
    assert(pBuffer->pDevice->pContext != NULL);
    return pBuffer->pDevice->pContext->register_stop_callback(pBuffer, callback, pUserData);
}

bool draudio_register_pause_callback(draudio_buffer* pBuffer, draudio_event_callback_proc callback, void* pUserData)
{
    if (pBuffer == NULL) {
        return false;
    }

    if (callback != NULL && draudio_get_playback_state(pBuffer) != draudio_stopped) {
        return false;
    }

    pBuffer->pauseCallback.callback  = callback;
    pBuffer->pauseCallback.pUserData = pUserData;

    assert(pBuffer->pDevice != NULL);
    assert(pBuffer->pDevice->pContext != NULL);
    return pBuffer->pDevice->pContext->register_pause_callback(pBuffer, callback, pUserData);
}

bool draudio_register_play_callback(draudio_buffer* pBuffer, draudio_event_callback_proc callback, void* pUserData)
{
    if (pBuffer == NULL) {
        return false;
    }

    if (callback != NULL && draudio_get_playback_state(pBuffer) != draudio_stopped) {
        return false;
    }

    pBuffer->playCallback.callback  = callback;
    pBuffer->playCallback.pUserData = pUserData;

    assert(pBuffer->pDevice != NULL);
    assert(pBuffer->pDevice->pContext != NULL);
    return pBuffer->pDevice->pContext->register_play_callback(pBuffer, callback, pUserData);
}


draudio_event_callback draudio_get_stop_callback(draudio_buffer* pBuffer)
{
    if (pBuffer != NULL) {
        return pBuffer->stopCallback;
    } else {
        draudio_event_callback result = {NULL, NULL};
        return result;
    }
}

draudio_event_callback draudio_get_pause_callback(draudio_buffer* pBuffer)
{
    if (pBuffer != NULL) {
        return pBuffer->pauseCallback;
    } else {
        draudio_event_callback result = {NULL, NULL};
        return result;
    }
}

draudio_event_callback draudio_get_play_callback(draudio_buffer* pBuffer)
{
    if (pBuffer != NULL) {
        return pBuffer->playCallback;
    } else {
        draudio_event_callback result = {NULL, NULL};
        return result;
    }
}


void draudio_set_position(draudio_buffer* pBuffer, float x, float y, float z)
{
    if (pBuffer == NULL) {
        return;
    }

    assert(pBuffer->pDevice != NULL);
    assert(pBuffer->pDevice->pContext != NULL);
    pBuffer->pDevice->pContext->set_position(pBuffer, x, y, z);
}

void draudio_get_position(draudio_buffer* pBuffer, float* pPosOut)
{
    if (pBuffer == NULL) {
        return;
    }

    if (pPosOut == NULL) {
        return;
    }

    assert(pBuffer->pDevice != NULL);
    assert(pBuffer->pDevice->pContext != NULL);
    pBuffer->pDevice->pContext->get_position(pBuffer, pPosOut);
}


void draudio_set_listener_position(draudio_device* pDevice, float x, float y, float z)
{
    if (pDevice == NULL) {
        return;
    }

    pDevice->pContext->set_listener_position(pDevice, x, y, z);
}

void draudio_get_listener_position(draudio_device* pDevice, float* pPosOut)
{
    if (pDevice == NULL || pPosOut == NULL) {
        return;
    }

    pDevice->pContext->get_listener_position(pDevice, pPosOut);
}

void draudio_set_listener_orientation(draudio_device* pDevice, float forwardX, float forwardY, float forwardZ, float upX, float upY, float upZ)
{
    if (pDevice == NULL) {
        return;
    }

    pDevice->pContext->set_listener_orientation(pDevice, forwardX, forwardY, forwardZ, upX, upY, upZ);
}

void draudio_get_listener_orientation(draudio_device* pDevice, float* pForwardOut, float* pUpOut)
{
    if (pDevice == NULL) {
        return;
    }

    pDevice->pContext->get_listener_orientation(pDevice, pForwardOut, pUpOut);
}


void draudio_set_3d_mode(draudio_buffer* pBuffer, draudio_3d_mode mode)
{
    if (pBuffer == NULL) {
        return;
    }

    assert(pBuffer->pDevice != NULL);
    assert(pBuffer->pDevice->pContext != NULL);
    pBuffer->pDevice->pContext->set_3d_mode(pBuffer, mode);
}

draudio_3d_mode draudio_get_3d_mode(draudio_buffer* pBuffer)
{
    if (pBuffer == NULL) {
        return draudio_3d_mode_disabled;
    }

    assert(pBuffer->pDevice != NULL);
    assert(pBuffer->pDevice->pContext != NULL);
    return pBuffer->pDevice->pContext->get_3d_mode(pBuffer);
}




///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// INPUT
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////








///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// HIGH-LEVEL API
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

//// SYNCHRONIZATION ////

#if defined(_WIN32)
#include <windows.h>

draudio_mutex draudio_create_mutex()
{
    draudio_mutex mutex = malloc(sizeof(CRITICAL_SECTION));
    if (mutex != NULL)
    {
        InitializeCriticalSection((LPCRITICAL_SECTION)mutex);
    }

    return mutex;
}

void draudio_delete_mutex(draudio_mutex mutex)
{
    DeleteCriticalSection((LPCRITICAL_SECTION)mutex);
    free(mutex);
}

void draudio_lock_mutex(draudio_mutex mutex)
{
    EnterCriticalSection((LPCRITICAL_SECTION)mutex);
}

void draudio_unlock_mutex(draudio_mutex mutex)
{
    LeaveCriticalSection((LPCRITICAL_SECTION)mutex);
}
#else
#include <pthread.h>

draudio_mutex draudio_create_mutex()
{
    pthread_mutex_t* mutex = malloc(sizeof(pthread_mutex_t));
    if (pthread_mutex_init(mutex, NULL) != 0) {
        free(mutex);
        mutex = NULL;
    }

    return mutex;
}

void draudio_delete_mutex(draudio_mutex mutex)
{
    pthread_mutex_destroy(mutex);
}

void draudio_lock_mutex(draudio_mutex mutex)
{
    pthread_mutex_lock(mutex);
}

void draudio_unlock_mutex(draudio_mutex mutex)
{
    pthread_mutex_unlock(mutex);
}
#endif



//// STREAMING ////

#define DRAUDIO_STREAMING_MARKER_0    DRAUDIO_EVENT_ID_MARKER + 0
#define DRAUDIO_STREAMING_MARKER_1    DRAUDIO_EVENT_ID_MARKER + 1

#define DRAUDIO_STREAMING_CHUNK_INVALID   0

typedef struct
{
    /// The steaming buffer callbacks.
    draudio_streaming_callbacks callbacks;

    /// Keeps track of whether or not we are at the start of the playback.
    bool atStart;

    /// Keeps track of whether or not we should stop at the end of the next chunk.
    bool stopAtEndOfCurrentChunk;

    /// Keeps track of whether or not the sound should loop.
    bool isLoopingEnabled;

    /// The size of the extra data.
    size_t extraDataSize;

    /// The size of an individual chunk. A chunk is half the size of the buffer.
    size_t chunkSize;

    /// A pointer to the temporary buffer for loading chunk data.
    unsigned char pTempChunkData[1];

} ea_streaming_buffer_data;


bool ea_streaming_buffer_load_next_chunk(draudio_buffer* pBuffer, ea_streaming_buffer_data* pStreamingData, size_t offset, size_t chunkSize)
{
    assert(pStreamingData != NULL);
    assert(pStreamingData->callbacks.read != NULL);
    assert(pStreamingData->callbacks.seek != NULL);
    assert(pStreamingData->chunkSize >= chunkSize);

    // A chunk size of 0 is valid, but we just return immediately.
    if (chunkSize == 0) {
        return true;
    }

    size_t bytesRead;
    if (!pStreamingData->callbacks.read(pStreamingData->callbacks.pUserData, pStreamingData->pTempChunkData, chunkSize, &bytesRead))
    {
        // There was an error reading the data. We might have run out of data.
        return false;
    }


    pStreamingData->stopAtEndOfCurrentChunk = false;

    draudio_set_buffer_data(pBuffer, offset, pStreamingData->pTempChunkData, bytesRead);

    if (chunkSize > bytesRead)
    {
        // The number of bytes read is less than our chunk size. This is our cue that we've reached the end of the steam. If we're looping, we
        // just seek back to the start and read more data. There is a chance the data total size of the streaming data is smaller than our
        // chunk, so we actually want to do this recursively.
        //
        // If we're not looping, we fill the remaining data with silence.
        if (pStreamingData->isLoopingEnabled)
        {
            pStreamingData->callbacks.seek(pStreamingData->callbacks.pUserData, 0);
            return ea_streaming_buffer_load_next_chunk(pBuffer, pStreamingData, offset + bytesRead, chunkSize - bytesRead);
        }
        else
        {
            memset(pStreamingData->pTempChunkData + bytesRead, 0, chunkSize - bytesRead);
            draudio_set_buffer_data(pBuffer, offset + bytesRead, pStreamingData->pTempChunkData + bytesRead, chunkSize - bytesRead);

            pStreamingData->stopAtEndOfCurrentChunk = true;
        }
    }

    return true;
}

void ea_steaming_buffer_marker_callback(draudio_buffer* pBuffer, unsigned int eventID, void *pUserData)
{
    ea_streaming_buffer_data* pStreamingData = (ea_streaming_buffer_data*)pUserData;
    assert(pStreamingData != NULL);

    size_t offset = 0;
    if (eventID == DRAUDIO_STREAMING_MARKER_0) {
        offset = pStreamingData->chunkSize;
    }

    if (pStreamingData->stopAtEndOfCurrentChunk)
    {
        if (!pStreamingData->atStart) {
            draudio_stop(pBuffer);
        }
    }
    else
    {
        ea_streaming_buffer_load_next_chunk(pBuffer, pStreamingData, offset, pStreamingData->chunkSize);
    }

    pStreamingData->atStart = false;
}


draudio_buffer* draudio_create_streaming_buffer(draudio_device* pDevice, draudio_buffer_desc* pBufferDesc, draudio_streaming_callbacks callbacks, unsigned int extraDataSize)
{
    if (callbacks.read == NULL) {
        return NULL;
    }

    if (pBufferDesc == NULL) {
        return NULL;
    }


    // We are determining for ourselves what the size of the buffer should be. We need to create our own copy rather than modify the input descriptor.
    draudio_buffer_desc bufferDesc = *pBufferDesc;
    bufferDesc.sizeInBytes  = pBufferDesc->sampleRate * pBufferDesc->channels * (pBufferDesc->bitsPerSample / 8);
    bufferDesc.pData        = NULL;

    size_t chunkSize = bufferDesc.sizeInBytes / 2;

    draudio_buffer* pBuffer = draudio_create_buffer(pDevice, &bufferDesc, sizeof(ea_streaming_buffer_data) - sizeof(unsigned char) + chunkSize + extraDataSize);
    if (pBuffer == NULL) {
        return NULL;
    }


    ea_streaming_buffer_data* pStreamingData = (ea_streaming_buffer_data*)draudio_get_buffer_extra_data(pBuffer);
    assert(pStreamingData != NULL);

    pStreamingData->callbacks               = callbacks;
    pStreamingData->atStart                 = true;
    pStreamingData->stopAtEndOfCurrentChunk = false;
    pStreamingData->isLoopingEnabled        = false;
    pStreamingData->chunkSize               = chunkSize;

    // Register two markers - one for the first half and another for the second half. When a half is finished playing we need to
    // replace it with new data.
    draudio_register_marker_callback(pBuffer, 0,         ea_steaming_buffer_marker_callback, DRAUDIO_STREAMING_MARKER_0, pStreamingData);
    draudio_register_marker_callback(pBuffer, chunkSize, ea_steaming_buffer_marker_callback, DRAUDIO_STREAMING_MARKER_1, pStreamingData);


    return pBuffer;
}


size_t draudio_get_streaming_buffer_extra_data_size(draudio_buffer* pBuffer)
{
    if (pBuffer == NULL) {
        return 0;
    }

    ea_streaming_buffer_data* pStreamingData = (ea_streaming_buffer_data*)draudio_get_buffer_extra_data(pBuffer);
    assert(pStreamingData != NULL);

    return pStreamingData->extraDataSize;
}

void* draudio_get_streaming_buffer_extra_data(draudio_buffer* pBuffer)
{
    if (pBuffer == NULL) {
        return NULL;
    }

    ea_streaming_buffer_data* pStreamingData = (ea_streaming_buffer_data*)draudio_get_buffer_extra_data(pBuffer);
    assert(pStreamingData != NULL);

    return ((char*)pStreamingData->pTempChunkData) + pStreamingData->chunkSize;
}


bool draudio_play_streaming_buffer(draudio_buffer* pBuffer, bool loop)
{
    if (pBuffer == NULL) {
        return false;
    }


    ea_streaming_buffer_data* pStreamingData = (ea_streaming_buffer_data*)draudio_get_buffer_extra_data(pBuffer);
    assert(pStreamingData != NULL);

    // If the buffer was previously in a paused state, we just play like normal. If it was in a stopped state we need to start from the beginning.
    if (draudio_get_playback_state(pBuffer) == draudio_stopped)
    {
        // We need to load some initial data into the first chunk.
        pStreamingData->atStart = true;
        pStreamingData->callbacks.seek(pStreamingData->callbacks.pUserData, 0);

        if (!ea_streaming_buffer_load_next_chunk(pBuffer, pStreamingData, 0, pStreamingData->chunkSize))
        {
            // There was an error loading the initial data.
            return false;
        }
    }


    pStreamingData->isLoopingEnabled = loop;
    draudio_play(pBuffer, true);      // <-- Always loop on a streaming buffer. Actual looping is done a bit differently for streaming buffers.

    return true;
}

bool draudio_is_streaming_buffer_looping(draudio_buffer* pBuffer)
{
    if (pBuffer == NULL) {
        return false;
    }

    ea_streaming_buffer_data* pStreamingData = (ea_streaming_buffer_data*)draudio_get_buffer_extra_data(pBuffer);
    assert(pStreamingData != NULL);

    return pStreamingData->isLoopingEnabled;
}



///////////////////////////////////////////////////////////////////////////////
//
// Sound World
//
///////////////////////////////////////////////////////////////////////////////

DRAUDIO_PRIVATE draudio_bool draudio_on_sound_read_callback(void* pUserData, void* pDataOut, size_t bytesToRead, size_t* bytesReadOut)
{
    draudio_sound* pSound = (draudio_sound*)pUserData;
    assert(pSound != NULL);
    assert(pSound->onRead != NULL);

    bool result = false;
    draudio_lock_mutex(pSound->pWorld->lock);
    {
        if (!pSound->markedForDeletion) {
            result = pSound->onRead(pSound, pDataOut, bytesToRead, bytesReadOut);
        }
    }
    draudio_unlock_mutex(pSound->pWorld->lock);

    return result;
}

DRAUDIO_PRIVATE static draudio_bool draudio_on_sound_seek_callback(void* pUserData, size_t offsetInBytesFromStart)
{
    draudio_sound* pSound = (draudio_sound*)pUserData;
    assert(pSound != NULL);
    assert(pSound->onRead != NULL);

    bool result = false;
    draudio_lock_mutex(pSound->pWorld->lock);
    {
        if (!pSound->markedForDeletion) {
            result = pSound->onSeek(pSound, offsetInBytesFromStart);
        }
    }
    draudio_unlock_mutex(pSound->pWorld->lock);

    return result;
}


DRAUDIO_PRIVATE static void draudio_inline_sound_stop_callback(draudio_buffer* pBuffer, unsigned int eventID, void *pUserData)
{
    (void)pBuffer;
    (void)eventID;

    assert(pBuffer != NULL);
    assert(eventID == DRAUDIO_EVENT_ID_STOP);
    assert(pUserData != NULL);

    draudio_sound* pSound = (draudio_sound*)pUserData;
    draudio_delete_sound(pSound);
}


DRAUDIO_PRIVATE void draudio_prepend_sound(draudio_sound* pSound)
{
    assert(pSound != NULL);
    assert(pSound->pWorld != NULL);
    assert(pSound->pPrevSound == NULL);

    draudio_lock_mutex(pSound->pWorld->lock);
    {
        pSound->pNextSound = pSound->pWorld->pFirstSound;

        if (pSound->pNextSound != NULL) {
            pSound->pNextSound->pPrevSound = pSound;
        }

        pSound->pWorld->pFirstSound = pSound;
    }
    draudio_unlock_mutex(pSound->pWorld->lock);
}

DRAUDIO_PRIVATE void draudio_remove_sound_nolock(draudio_sound* pSound)
{
    assert(pSound != NULL);
    assert(pSound->pWorld != NULL);

    if (pSound == pSound->pWorld->pFirstSound) {
        pSound->pWorld->pFirstSound = pSound->pNextSound;
    }

    if (pSound->pNextSound != NULL) {
        pSound->pNextSound->pPrevSound = pSound->pPrevSound;
    }

    if (pSound->pPrevSound != NULL) {
        pSound->pPrevSound->pNextSound = pSound->pNextSound;
    }
}


DRAUDIO_PRIVATE bool draudio_is_inline_sound(draudio_sound* pSound)
{
    assert(pSound != NULL);
    return draudio_get_stop_callback(pSound->pBuffer).callback == draudio_inline_sound_stop_callback;
}


draudio_world* draudio_create_world(draudio_device* pDevice)
{
    draudio_world* pWorld = (draudio_world*)malloc(sizeof(*pWorld));
    if (pWorld != NULL)
    {
        pWorld->pDevice       = pDevice;
        pWorld->playbackState = draudio_playing;
        pWorld->pFirstSound   = NULL;
        pWorld->lock          = draudio_create_mutex();
    }

    return pWorld;
}

void draudio_delete_world(draudio_world* pWorld)
{
    if (pWorld == NULL) {
        return;
    }

    // Delete every sound first.
    draudio_delete_all_sounds(pWorld);

    // Delete the lock after deleting every sound because we still need thread-safety at this point.
    draudio_delete_mutex(pWorld->lock);

    // Free the world last.
    free(pWorld);
}


draudio_sound* draudio_create_sound(draudio_world* pWorld, draudio_sound_desc desc)
{
    if (pWorld == NULL) {
        return NULL;
    }

    if ((desc.pData == NULL || desc.sizeInBytes == 0) && (desc.onRead == NULL || desc.onSeek == NULL)) {
        // When streaming is not being used, the initial data must be valid at creation time.
        return NULL;
    }

    draudio_sound* pSound = (draudio_sound*)malloc(sizeof(*pSound));
    if (pSound == NULL) {
        return NULL;
    }

    pSound->pWorld                 = pWorld;
    pSound->prevPlaybackState      = draudio_stopped;
    pSound->pNextSound             = NULL;
    pSound->pPrevSound             = NULL;
    pSound->isUsingStreamingBuffer = desc.sizeInBytes == 0 || desc.pData == NULL;
    pSound->markedForDeletion      = false;
    pSound->onDelete               = desc.onDelete;
    pSound->onRead                 = desc.onRead;
    pSound->onSeek                 = desc.onSeek;

    draudio_buffer_desc bufferDesc;
    bufferDesc.flags         = desc.flags;
    bufferDesc.format        = desc.format;
    bufferDesc.channels      = desc.channels;
    bufferDesc.sampleRate    = desc.sampleRate;
    bufferDesc.bitsPerSample = desc.bitsPerSample;
    bufferDesc.sizeInBytes   = desc.sizeInBytes;
    bufferDesc.pData         = desc.pData;

    if (pSound->isUsingStreamingBuffer)
    {
        draudio_streaming_callbacks streamingCallbacks;
        streamingCallbacks.pUserData = pSound;
        streamingCallbacks.read      = draudio_on_sound_read_callback;
        streamingCallbacks.seek      = draudio_on_sound_seek_callback;

        pSound->pBuffer = draudio_create_streaming_buffer(pWorld->pDevice, &bufferDesc, streamingCallbacks, desc.extraDataSize);
        if (pSound->pBuffer != NULL && desc.pExtraData != NULL)
        {
            memcpy(draudio_get_streaming_buffer_extra_data(pSound->pBuffer), desc.pExtraData, desc.extraDataSize);
        }
    }
    else
    {
        pSound->pBuffer = draudio_create_buffer(pWorld->pDevice, &bufferDesc, desc.extraDataSize);
        if (pSound->pBuffer != NULL && desc.pExtraData != NULL)
        {
            memcpy(draudio_get_buffer_extra_data(pSound->pBuffer), desc.pExtraData, desc.extraDataSize);
        }
    }


    // Return NULL if we failed to create the internal audio buffer.
    if (pSound->pBuffer == NULL) {
        free(pSound);
        return NULL;
    }


    // Only attach the sound to the internal list at the end when we know everything has worked.
    draudio_prepend_sound(pSound);

    return pSound;
}

void draudio_delete_sound(draudio_sound* pSound)
{
    if (pSound == NULL) {
        return;
    }


    draudio_lock_mutex(pSound->pWorld->lock);
    {
        if (pSound->markedForDeletion) {
            assert(false);
            return;
        }

        pSound->markedForDeletion = true;


        // Remove the sound from the internal list first.
        draudio_remove_sound_nolock(pSound);


        // If we're deleting an inline sound, we want to remove the stop event callback. If we don't do this, we'll end up trying to delete
        // the sound twice.
        if (draudio_is_inline_sound(pSound)) {
            draudio_register_stop_callback(pSound->pBuffer, NULL, NULL);
        }


        // Let the application know that the sound is being deleted. We want to do this after removing the stop event just to be sure the
        // application doesn't try to explicitly stop the sound in this callback - that would be a problem for inlined sounds because they
        // are configured to delete themselves upon stopping which we are already in the process of doing.
        if (pSound->onDelete != NULL) {
            pSound->onDelete(pSound);
        }


        // Delete the internal audio buffer before letting the host application know about the deletion.
        draudio_delete_buffer(pSound->pBuffer);
    }
    draudio_unlock_mutex(pSound->pWorld->lock);


    // Only free the sound after the application has been made aware the sound is being deleted.
    free(pSound);
}

void draudio_delete_all_sounds(draudio_world* pWorld)
{
    if (pWorld == NULL) {
        return;
    }

    while (pWorld->pFirstSound != NULL) {
        draudio_delete_sound(pWorld->pFirstSound);
    }
}


size_t draudio_get_sound_extra_data_size(draudio_sound* pSound)
{
    if (pSound == NULL) {
        return 0;
    }

    if (pSound->isUsingStreamingBuffer) {
        return draudio_get_streaming_buffer_extra_data_size(pSound->pBuffer);
    } else {
        return draudio_get_buffer_extra_data_size(pSound->pBuffer);
    }
}

void* draudio_get_sound_extra_data(draudio_sound* pSound)
{
    if (pSound == NULL) {
        return NULL;
    }

    if (pSound->isUsingStreamingBuffer) {
        return draudio_get_streaming_buffer_extra_data(pSound->pBuffer);
    } else {
        return draudio_get_buffer_extra_data(pSound->pBuffer);
    }
}


void draudio_play_sound(draudio_sound* pSound, bool loop)
{
    if (pSound != NULL) {
        if (pSound->isUsingStreamingBuffer) {
            draudio_play_streaming_buffer(pSound->pBuffer, loop);
        } else {
            draudio_play(pSound->pBuffer, loop);
        }
    }
}

void draudio_pause_sound(draudio_sound* pSound)
{
    if (pSound != NULL) {
        draudio_pause(pSound->pBuffer);
    }
}

void draudio_stop_sound(draudio_sound* pSound)
{
    if (pSound != NULL) {
        draudio_stop(pSound->pBuffer);
    }
}

draudio_playback_state draudio_get_sound_playback_state(draudio_sound* pSound)
{
    if (pSound == NULL) {
        return draudio_stopped;
    }

    return draudio_get_playback_state(pSound->pBuffer);
}

bool draudio_is_sound_looping(draudio_sound* pSound)
{
    if (pSound == NULL) {
        return false;
    }

    if (pSound->isUsingStreamingBuffer) {
        return draudio_is_streaming_buffer_looping(pSound->pBuffer);
    } else {
        return draudio_is_looping(pSound->pBuffer);
    }
}



void draudio_play_inline_sound(draudio_world* pWorld, draudio_sound_desc desc)
{
    if (pWorld == NULL) {
        return;
    }

    // We need to explicitly ensure 3D positioning is disabled.
    desc.flags &= ~DRAUDIO_ENABLE_3D;

    draudio_sound* pSound = draudio_create_sound(pWorld, desc);
    if (pSound != NULL)
    {
        // For inline sounds we set a callback for when the sound is stopped. When this callback is fired, the sound is deleted.
        draudio_set_sound_stop_callback(pSound, draudio_inline_sound_stop_callback, pSound);

        // Start playing the sound once everything else has been set up.
        draudio_play_sound(pSound, false);
    }
}

void draudio_play_inline_sound_3f(draudio_world* pWorld, draudio_sound_desc desc, float posX, float posY, float posZ)
{
    if (pWorld == NULL) {
        return;
    }

    draudio_sound* pSound = draudio_create_sound(pWorld, desc);
    if (pSound != NULL)
    {
        // For inline sounds we set a callback for when the sound is stopped. When this callback is fired, the sound is deleted.
        draudio_set_sound_stop_callback(pSound, draudio_inline_sound_stop_callback, pSound);

        // Set the position before playing anything.
        draudio_set_sound_position(pSound, posX, posY, posZ);

        // Start playing the sound once everything else has been set up.
        draudio_play_sound(pSound, false);
    }
}


void draudio_stop_all_sounds(draudio_world* pWorld)
{
    if (pWorld == NULL) {
        return;
    }

    bool wasPlaying = pWorld->playbackState == draudio_playing;
    if (pWorld->playbackState != draudio_stopped)
    {
        // We need to loop over every sound and stop them. We also need to keep track of their previous playback state
        // so that when resume_all_sounds() is called, it can be restored correctly.
        for (draudio_sound* pSound = pWorld->pFirstSound; pSound != NULL; pSound = pSound->pNextSound)
        {
            if (wasPlaying) {
                pSound->prevPlaybackState = draudio_get_sound_playback_state(pSound);
            }

            draudio_stop_sound(pSound);
        }
    }
}

void draudio_pause_all_sounds(draudio_world* pWorld)
{
    if (pWorld == NULL) {
        return;
    }

    if (pWorld->playbackState == draudio_playing)
    {
        // We need to loop over every sound and stop them. We also need to keep track of their previous playback state
        // so that when resume_all_sounds() is called, it can be restored correctly.
        for (draudio_sound* pSound = pWorld->pFirstSound; pSound != NULL; pSound = pSound->pNextSound)
        {
            pSound->prevPlaybackState = draudio_get_sound_playback_state(pSound);
            draudio_pause_sound(pSound);
        }
    }
}

void draudio_resume_all_sounds(draudio_world* pWorld)
{
    if (pWorld == NULL) {
        return;
    }

    if (pWorld->playbackState != draudio_playing)
    {
        // When resuming playback, we use the previous playback state to determine how to resume.
        for (draudio_sound* pSound = pWorld->pFirstSound; pSound != NULL; pSound = pSound->pNextSound)
        {
            if (pSound->prevPlaybackState == draudio_playing) {
                draudio_play_sound(pSound, draudio_is_sound_looping(pSound));
            }
        }
    }
}


void draudio_set_sound_stop_callback(draudio_sound* pSound, draudio_event_callback_proc callback, void* pUserData)
{
    if (pSound != NULL) {
        draudio_register_stop_callback(pSound->pBuffer, callback, pUserData);
    }
}

void draudio_set_sound_pause_callback(draudio_sound* pSound, draudio_event_callback_proc callback, void* pUserData)
{
    if (pSound != NULL) {
        draudio_register_pause_callback(pSound->pBuffer, callback, pUserData);
    }
}

void draudio_set_sound_play_callback(draudio_sound* pSound, draudio_event_callback_proc callback, void* pUserData)
{
    if (pSound != NULL) {
        draudio_register_play_callback(pSound->pBuffer, callback, pUserData);
    }
}


void draudio_set_sound_position(draudio_sound* pSound, float posX, float posY, float posZ)
{
    if (pSound != NULL) {
        draudio_set_position(pSound->pBuffer, posX, posY, posZ);
    }
}


void draudio_set_sound_3d_mode(draudio_sound* pSound, draudio_3d_mode mode)
{
    if (pSound != NULL) {
        draudio_set_3d_mode(pSound->pBuffer, mode);
    }
}

draudio_3d_mode draudio_get_sound_3d_mode(draudio_sound* pSound)
{
    if (pSound == NULL) {
        return draudio_3d_mode_disabled;
    }

    return draudio_get_3d_mode(pSound->pBuffer);
}





///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////
//
// BACKENDS
//
///////////////////////////////////////////////////////////////////////////////
///////////////////////////////////////////////////////////////////////////////

///////////////////////////////////////////////////////////////////////////////
//
// DirectSound
//
// The DirectSound backend is mostly simple, except for event handling. Events
// are achieved through the use of Win32 event objects and waiting on them to
// be put into a signaled state by DirectSound. Due to this mechanic we need to
// create a worker thread that waits on each event.
//
// The worker thread waits on three general types of events. The first is an
// event that is signaled when the thread needs to be terminated. The second
// is an event that is signaled when a new set of events need to be waited on.
// The third is a set of events that correspond to an output buffer event (such
// as stop, pause, play and marker events.)
//
///////////////////////////////////////////////////////////////////////////////
#ifdef DRAUDIO_BUILD_DSOUND
#include <windows.h>
#include <dsound.h>
#include <mmreg.h>
#include <stdio.h>      // For testing and debugging with printf(). Delete this later.

// Define a NULL GUID for use later on. If we don't do this and use GUID_NULL we'll end
// up with a link error.
GUID DRAUDIO_GUID_NULL = {0x00000000, 0x0000, 0x0000, {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00}};


//// Message Queue ////

#define DRAUDIO_MESSAGE_ID_UNKNOWN            0
#define DRAUDIO_MESSAGE_ID_EVENT              1
#define DRAUDIO_MESSAGE_ID_DELETE_BUFFER      2
#define DRAUDIO_MESSAGE_ID_DELETE_DEVICE      3
#define DRAUDIO_MESSAGE_ID_TERMINATE_THREAD   4

/// Structure representing an individual message
typedef struct
{
    /// The message ID.
    int id;

    /// A pointer to the relevant buffer.
    draudio_buffer* pBuffer;


    // The message-specific data.
    union
    {
        struct
        {
            /// A pointer to the callback function.
            draudio_event_callback_proc callback;

            /// The event ID.
            unsigned int eventID;

            /// The callback user data.
            void* pUserData;

        } callback_event;


        struct
        {
            /// A pointer to the DirectSound buffer object.
            LPDIRECTSOUNDBUFFER8 pDSBuffer;

            /// A pointer to the 3D DirectSound buffer object. This will be NULL if 3D positioning is disabled for the buffer.
            LPDIRECTSOUND3DBUFFER pDSBuffer3D;

            /// A pointer to the object for handling notification events.
            LPDIRECTSOUNDNOTIFY pDSNotify;

        } delete_buffer;


        struct
        {
            /// A pointer to the DIRECTSOUND object that was created with DirectSoundCreate8().
            LPDIRECTSOUND8 pDS;

            /// A pointer to the DIRECTSOUNDBUFFER object for the primary buffer.
            LPDIRECTSOUNDBUFFER pDSPrimaryBuffer;

            /// A pointer to the DIRECTSOUND3DLISTENER8 object associated with the device.
            LPDIRECTSOUND3DLISTENER pDSListener;

            /// A pointer to the device object being deleted.
            draudio_device* pDevice;

        } delete_device;

    } data;

} draudio_message_dsound;


/// Structure representing the main message queue.
///
/// The message queue is implemented as a fixed-sized cyclic array which means there should be no significant data
/// movement and fast pushing and popping.
typedef struct
{
    /// The buffer containing the list of events.
    draudio_message_dsound messages[DRAUDIO_MAX_MESSAGE_QUEUE_SIZE];

    /// The number of active messages in the queue.
    unsigned int messageCount;

    /// The index of the first message in the queue.
    unsigned int iFirstMessage;

    /// The mutex for synchronizing access to message pushing and popping.
    draudio_mutex queueLock;

    /// The semaphore that's used for blocking draudio_next_message_dsound(). The maximum value is set to DRAUDIO_MAX_MESSAGE_QUEUE_SIZE.
    HANDLE hMessageSemaphore;

    /// The message handling thread.
    HANDLE hMessageHandlingThread;

    /// Keeps track of whether or not the queue is deleted. We use this to ensure a thread does not try to post an event.
    bool isDeleted;

} draudio_message_queue_dsound;


/// The function to run on the message handling thread. This is implemented down the bottom.
DWORD WINAPI MessageHandlingThread_DSound(draudio_message_queue_dsound* pQueue);

/// Posts a new message. This is thread safe.
void draudio_post_message_dsound(draudio_message_queue_dsound* pQueue, draudio_message_dsound msg);



/// Initializes the given mesasge queue.
bool draudio_init_message_queue_dsound(draudio_message_queue_dsound* pQueue)
{
    if (pQueue == NULL) {
        return false;
    }

    pQueue->messageCount  = 0;
    pQueue->iFirstMessage = 0;

    pQueue->queueLock = draudio_create_mutex();
    if (pQueue->queueLock == NULL) {
        return false;
    }

    pQueue->hMessageSemaphore = CreateSemaphoreA(NULL, 0, DRAUDIO_MAX_MESSAGE_QUEUE_SIZE, NULL);
    if (pQueue->hMessageSemaphore == NULL)
    {
        draudio_delete_mutex(pQueue->queueLock);
        return false;
    }

    pQueue->hMessageHandlingThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)MessageHandlingThread_DSound, pQueue, 0, NULL);
    if (pQueue->hMessageHandlingThread == NULL)
    {
        CloseHandle(pQueue->hMessageSemaphore);
        draudio_delete_mutex(pQueue->queueLock);
        return false;
    }

    pQueue->isDeleted = false;

    return true;
}

/// Uninitializes the given message queue.
void draudio_uninit_message_queue_dsound(draudio_message_queue_dsound* pQueue)
{
    // We need to make sure the thread is closed properly before returning from here. To do this we just post an DRAUDIO_MESSAGE_ID_TERMINATE_THREAD
    // event to the message queue and wait for the thread to finish.
    draudio_message_dsound msg;
    msg.id = DRAUDIO_MESSAGE_ID_TERMINATE_THREAD;
    draudio_post_message_dsound(pQueue, msg);


    // Once we posted the event we just wait for the thread to process it and terminate.
    WaitForSingleObject(pQueue->hMessageHandlingThread, INFINITE);

    // At this point the thread has been terminated and we can clear everything.
    CloseHandle(pQueue->hMessageHandlingThread);
    pQueue->hMessageHandlingThread = NULL;

    CloseHandle(pQueue->hMessageSemaphore);
    pQueue->hMessageSemaphore = NULL;


    pQueue->isDeleted = true;
    draudio_lock_mutex(pQueue->queueLock);
    {
        pQueue->messageCount  = 0;
        pQueue->iFirstMessage = 0;
    }
    draudio_unlock_mutex(pQueue->queueLock);

    draudio_delete_mutex(pQueue->queueLock);
    pQueue->queueLock = NULL;
}


void draudio_post_message_dsound(draudio_message_queue_dsound* pQueue, draudio_message_dsound msg)
{
    assert(pQueue != NULL);

    if (pQueue->isDeleted) {
        return;
    }

    draudio_lock_mutex(pQueue->queueLock);
    {
        if (pQueue->messageCount < DRAUDIO_MAX_MESSAGE_QUEUE_SIZE)
        {
            pQueue->messages[(pQueue->iFirstMessage + pQueue->messageCount) % DRAUDIO_MAX_MESSAGE_QUEUE_SIZE] = msg;
            pQueue->messageCount += 1;

            ReleaseSemaphore(pQueue->hMessageSemaphore, 1, NULL);
        }
    }
    draudio_unlock_mutex(pQueue->queueLock);
}


/// Retrieves the next message in the queue.
///
/// @remarks
///     This blocks until a message is available. This will return false when it receives the DRAUDIO_MESSAGE_ID_TERMINATE_THREAD message.
bool draudio_next_message_dsound(draudio_message_queue_dsound* pQueue, draudio_message_dsound* pMsgOut)
{
    if (WaitForSingleObject(pQueue->hMessageSemaphore, INFINITE) == WAIT_OBJECT_0)   // Wait for a message to become available.
    {
        draudio_message_dsound msg;
        msg.id = DRAUDIO_MESSAGE_ID_UNKNOWN;

        draudio_lock_mutex(pQueue->queueLock);
        {
            assert(pQueue->messageCount > 0);

            msg = pQueue->messages[pQueue->iFirstMessage];

            pQueue->iFirstMessage = (pQueue->iFirstMessage + 1) % DRAUDIO_MAX_MESSAGE_QUEUE_SIZE;
            pQueue->messageCount -= 1;
        }
        draudio_unlock_mutex(pQueue->queueLock);


        if (pMsgOut != NULL) {
            pMsgOut[0] = msg;
        }

        return msg.id != DRAUDIO_MESSAGE_ID_TERMINATE_THREAD;
    }

    return false;
}


DWORD WINAPI MessageHandlingThread_DSound(draudio_message_queue_dsound* pQueue)
{
    assert(pQueue != NULL);

    draudio_message_dsound msg;
    while (draudio_next_message_dsound(pQueue, &msg))
    {
        assert(msg.id != DRAUDIO_MESSAGE_ID_TERMINATE_THREAD);        // <-- draudio_next_message_dsound() will return false when it receives DRAUDIO_MESSAGE_ID_TERMINATE_THREAD.

        switch (msg.id)
        {
            case DRAUDIO_MESSAGE_ID_EVENT:
            {
                assert(msg.data.callback_event.callback != NULL);

                msg.data.callback_event.callback(msg.pBuffer, msg.data.callback_event.eventID, msg.data.callback_event.pUserData);
                break;
            }

            case DRAUDIO_MESSAGE_ID_DELETE_BUFFER:
            {
                if (msg.data.delete_buffer.pDSNotify != NULL) {
                    IDirectSoundNotify_Release(msg.data.delete_buffer.pDSNotify);
                }

                if (msg.data.delete_buffer.pDSBuffer3D != NULL) {
                    IDirectSound3DBuffer_Release(msg.data.delete_buffer.pDSBuffer3D);
                }

                if (msg.data.delete_buffer.pDSBuffer != NULL) {
                    IDirectSoundBuffer8_Release(msg.data.delete_buffer.pDSBuffer);
                }

                free(msg.pBuffer);
                break;
            }

            case DRAUDIO_MESSAGE_ID_DELETE_DEVICE:
            {
                if (msg.data.delete_device.pDSListener != NULL) {
                    IDirectSound3DListener_Release(msg.data.delete_device.pDSListener);
                }

                if (msg.data.delete_device.pDSPrimaryBuffer != NULL) {
                    IDirectSoundBuffer_Release(msg.data.delete_device.pDSPrimaryBuffer);
                }

                if (msg.data.delete_device.pDS != NULL) {
                    IDirectSound_Release(msg.data.delete_device.pDS);
                }

                free(msg.data.delete_device.pDevice);
                break;
            }

            default:
            {
                // Should never hit this.
                assert(false);
                break;
            }
        }
    }

    return 0;
}




/// Deactivates (but does not delete) every event associated with the given buffer.
void draudio_deactivate_buffer_events_dsound(draudio_buffer* pBuffer);


//// Event Management ////

typedef struct draudio_event_manager_dsound draudio_event_manager_dsound;
typedef struct draudio_event_dsound draudio_event_dsound;

struct draudio_event_dsound
{
    /// A pointer to the event manager that owns this event.
    draudio_event_manager_dsound* pEventManager;

    /// The event.
    HANDLE hEvent;

    /// The callback.
    draudio_event_callback_proc callback;

    /// A pointer to the applicable buffer.
    draudio_buffer* pBuffer;

    /// The event ID. For on_stop events, this will be set to DRAUDIO_EVENT_STOP
    unsigned int eventID;

    /// A pointer to the user data.
    void* pUserData;

    /// The marker offset. Only used for marker events. Should be set to 0 for non-markers.
    DWORD markerOffset;

    /// Events are stored in a linked list. This is a pointer to the next event in the list.
    draudio_event_dsound* pNextEvent;

    /// A pointer to the previous event.
    draudio_event_dsound* pPrevEvent;
};

struct draudio_event_manager_dsound
{
    /// A pointer to the message queue where messages will be posted for event processing.
    draudio_message_queue_dsound* pMessageQueue;


    /// A handle to the event worker thread.
    HANDLE hThread;

    /// A handle to the terminator event object.
    HANDLE hTerminateEvent;

    /// A handle to the refresher event object.
    HANDLE hRefreshEvent;

    /// The mutex to use when refreshing the worker thread. This is used to ensure only one refresh is done at a time.
    draudio_mutex refreshMutex;

    /// The synchronization lock.
    draudio_mutex mainLock;

    /// The event object for notifying dr_audio when an event has finished being handled by the event handling thread.
    HANDLE hEventCompletionLock;


    /// The first event in a list.
    draudio_event_dsound* pFirstEvent;

    /// The last event in the list of events.
    draudio_event_dsound* pLastEvent;
};


/// Locks the event manager.
void draudio_lock_events_dsound(draudio_event_manager_dsound* pEventManager)
{
    draudio_lock_mutex(pEventManager->mainLock);
}

/// Unlocks the event manager.
void draudio_unlock_events_dsound(draudio_event_manager_dsound* pEventManager)
{
    draudio_unlock_mutex(pEventManager->mainLock);
}


/// Removes the given event from the event lists.
///
/// @remarks
///     This will be used when moving the event to a new location in the list or when it is being deleted.
void draudio_remove_event_dsound_nolock(draudio_event_dsound* pEvent)
{
    assert(pEvent != NULL);

    draudio_event_manager_dsound* pEventManager = pEvent->pEventManager;
    assert(pEventManager != NULL);

    if (pEventManager->pFirstEvent == pEvent) {
        pEventManager->pFirstEvent = pEvent->pNextEvent;
    }

    if (pEventManager->pLastEvent == pEvent) {
        pEventManager->pLastEvent = pEvent->pPrevEvent;
    }


    if (pEvent->pPrevEvent != NULL) {
        pEvent->pPrevEvent->pNextEvent = pEvent->pNextEvent;
    }

    if (pEvent->pNextEvent != NULL) {
        pEvent->pNextEvent->pPrevEvent = pEvent->pPrevEvent;
    }

    pEvent->pNextEvent = NULL;
    pEvent->pPrevEvent = NULL;
}

/// @copydoc draudio_remove_event_dsound_nolock()
void draudio_remove_event_dsound(draudio_event_dsound* pEvent)
{
    assert(pEvent != NULL);

    draudio_event_manager_dsound* pEventManager = pEvent->pEventManager;
    draudio_lock_events_dsound(pEventManager);
    {
        draudio_remove_event_dsound_nolock(pEvent);
    }
    draudio_unlock_events_dsound(pEventManager);
}

/// Adds the given event to the end of the internal list.
void draudio_append_event_dsound(draudio_event_dsound* pEvent)
{
    assert(pEvent != NULL);

    draudio_event_manager_dsound* pEventManager = pEvent->pEventManager;
    draudio_lock_events_dsound(pEventManager);
    {
        draudio_remove_event_dsound_nolock(pEvent);

        assert(pEvent->pNextEvent == NULL);

        if (pEventManager->pLastEvent != NULL) {
            pEvent->pPrevEvent = pEventManager->pLastEvent;
            pEvent->pPrevEvent->pNextEvent = pEvent;
        }

        if (pEventManager->pFirstEvent == NULL) {
            pEventManager->pFirstEvent = pEvent;
        }

        pEventManager->pLastEvent = pEvent;
    }
    draudio_unlock_events_dsound(pEventManager);
}

void draudio_refresh_worker_thread_event_queue(draudio_event_manager_dsound* pEventManager)
{
    assert(pEventManager != NULL);

    // To refresh the worker thread we just need to signal the refresh event. We then just need to wait for
    // processing to finish which we can do by waiting on another event to become signaled.

    draudio_lock_mutex(pEventManager->refreshMutex);
    {
        // Signal a refresh.
        SetEvent(pEventManager->hRefreshEvent);

        // Wait for refreshing to complete.
        WaitForSingleObject(pEventManager->hEventCompletionLock, INFINITE);
    }
    draudio_unlock_mutex(pEventManager->refreshMutex);
}


/// Closes the Win32 event handle of the given event.
void draudio_close_win32_event_handle_dsound(draudio_event_dsound* pEvent)
{
    assert(pEvent != NULL);
    assert(pEvent->pEventManager != NULL);


    // At the time of calling this function, pEvent should have been removed from the internal list. The issue is that
    // the event notification thread is waiting on it. Thus, we need to refresh the worker thread to ensure the event
    // have been flushed from that queue. To do this we just signal a special event that's used to trigger a refresh.
    draudio_refresh_worker_thread_event_queue(pEvent->pEventManager);

    // The worker thread should not be waiting on the event so we can go ahead and close the handle now.
    CloseHandle(pEvent->hEvent);
    pEvent->hEvent = NULL;
}


/// Updates the given event to use the given callback and user data.
void draudio_update_event_dsound(draudio_event_dsound* pEvent, draudio_event_callback_proc callback, void* pUserData)
{
    assert(pEvent != NULL);

    pEvent->callback  = callback;
    pEvent->pUserData = pUserData;

    draudio_refresh_worker_thread_event_queue(pEvent->pEventManager);
}

/// Creates a new event, but does not activate it.
///
/// @remarks
///     When an event is created, it just sits dormant and will never be triggered until it has been
///     activated with draudio_activate_event_dsound().
draudio_event_dsound* draudio_create_event_dsound(draudio_event_manager_dsound* pEventManager, draudio_event_callback_proc callback, draudio_buffer* pBuffer, unsigned int eventID, void* pUserData)
{
    draudio_event_dsound* pEvent = (draudio_event_dsound*)malloc(sizeof(draudio_event_dsound));
    if (pEvent != NULL)
    {
        pEvent->pEventManager = pEventManager;
        pEvent->hEvent        = CreateEventA(NULL, FALSE, FALSE, NULL);
        pEvent->callback      = NULL;
        pEvent->pBuffer       = pBuffer;
        pEvent->eventID       = eventID;
        pEvent->pUserData     = NULL;
        pEvent->markerOffset  = 0;
        pEvent->pNextEvent    = NULL;
        pEvent->pPrevEvent    = NULL;

        // Append the event to the internal list.
        draudio_append_event_dsound(pEvent);

        // This roundabout way of setting the callback and user data is to ensure the worker thread is made aware that it needs
        // to refresh it's local event data.
        draudio_update_event_dsound(pEvent, callback, pUserData);
    }

    return pEvent;
}

/// Deletes an event, and deactivates it.
///
/// @remarks
///     This will not return until the event has been deleted completely.
void draudio_delete_event_dsound(draudio_event_dsound* pEvent)
{
    assert(pEvent != NULL);

    // Set everything to NULL so the worker thread is aware that the event is about to get deleted.
    pEvent->pBuffer      = NULL;
    pEvent->callback     = NULL;
    pEvent->eventID      = 0;
    pEvent->pUserData    = NULL;
    pEvent->markerOffset = 0;

    // Remove the event from the list.
    draudio_remove_event_dsound(pEvent);

    // Close the Win32 event handle.
    if (pEvent->hEvent != NULL) {
        draudio_close_win32_event_handle_dsound(pEvent);
    }


    // At this point everything has been closed so we can safely free the memory and return.
    free(pEvent);
}


/// Gathers the event handles and callback data for all of the relevant buffer events.
unsigned int draudio_gather_events_dsound(draudio_event_manager_dsound *pEventManager, HANDLE* pHandlesOut, draudio_event_dsound** ppEventsOut, unsigned int outputBufferSize)
{
    assert(pEventManager != NULL);
    assert(pHandlesOut != NULL);
    assert(ppEventsOut != NULL);
    assert(outputBufferSize >= 2);

    unsigned int i = 2;
    draudio_lock_events_dsound(pEventManager);
    {
        pHandlesOut[0] = pEventManager->hTerminateEvent;
        ppEventsOut[0] = NULL;

        pHandlesOut[1] = pEventManager->hRefreshEvent;
        ppEventsOut[1] = NULL;


        draudio_event_dsound* pEvent = pEventManager->pFirstEvent;
        while (i < outputBufferSize && pEvent != NULL)
        {
            if (pEvent->hEvent != NULL)
            {
                pHandlesOut[i] = pEvent->hEvent;
                ppEventsOut[i] = pEvent;

                i += 1;
            }

            pEvent = pEvent->pNextEvent;
        }
    }
    draudio_unlock_events_dsound(pEventManager);

    return i;
}

/// The entry point to the event worker thread.
DWORD WINAPI DSound_EventWorkerThreadProc(draudio_event_manager_dsound *pEventManager)
{
    if (pEventManager != NULL)
    {
        HANDLE hTerminateEvent = pEventManager->hTerminateEvent;
        HANDLE hRefreshEvent   = pEventManager->hRefreshEvent;

        HANDLE eventHandles[1024];
        draudio_event_dsound* events[1024];
        unsigned int eventCount = draudio_gather_events_dsound(pEventManager, eventHandles, events, 1024);   // <-- Initial gather.

        bool requestedRefresh = false;
        for (;;)
        {
            if (requestedRefresh)
            {
                eventCount = draudio_gather_events_dsound(pEventManager, eventHandles, events, 1024);

                // Refreshing is done, so now we need to let other threads know about it.
                SetEvent(pEventManager->hEventCompletionLock);
                requestedRefresh = false;
            }



            DWORD rc = WaitForMultipleObjects(eventCount, eventHandles, FALSE, INFINITE);
            if (rc >= WAIT_OBJECT_0 && rc < eventCount)
            {
                const unsigned int eventIndex = rc - WAIT_OBJECT_0;
                HANDLE hEvent = eventHandles[eventIndex];

                if (hEvent == hTerminateEvent)
                {
                    // The terminator event was signaled. We just return from the thread immediately.
                    return 0;
                }

                if (hEvent == hRefreshEvent)
                {
                    assert(hRefreshEvent == pEventManager->hRefreshEvent);

                    // This event will get signaled when a new set of events need to be waited on, such as when a new event has been registered on a buffer.
                    requestedRefresh = true;
                    continue;
                }


                // If we get here if means we have hit a callback event.
                draudio_event_dsound* pEvent = events[eventIndex];
                if (pEvent->callback != NULL)
                {
                    assert(pEvent->hEvent == hEvent);

                    // The stop event will be signaled by DirectSound when IDirectSoundBuffer::Stop() is called. The problem is that we need to call that when the
                    // sound is paused as well. Thus, we need to check if we got the stop event, and if so DON'T call the callback function if it is in a non-stopped
                    // state.
                    bool isStopEventButNotStopped = pEvent->eventID == DRAUDIO_EVENT_ID_STOP && draudio_get_playback_state(pEvent->pBuffer) != draudio_stopped;
                    if (!isStopEventButNotStopped)
                    {
                        // We don't call the callback directly. Instead we post a message to the message handling thread for processing later.
                        draudio_message_dsound msg;
                        msg.id      = DRAUDIO_MESSAGE_ID_EVENT;
                        msg.pBuffer = pEvent->pBuffer;
                        msg.data.callback_event.callback  = pEvent->callback;
                        msg.data.callback_event.eventID   = pEvent->eventID;
                        msg.data.callback_event.pUserData = pEvent->pUserData;
                        draudio_post_message_dsound(pEventManager->pMessageQueue, msg);
                    }
                }
            }
        }
    }

    return 0;
}


/// Initializes the event manager by creating the thread and event objects.
bool draudio_init_event_manager_dsound(draudio_event_manager_dsound* pEventManager, draudio_message_queue_dsound* pMessageQueue)
{
    assert(pEventManager != NULL);
    assert(pMessageQueue != NULL);

    pEventManager->pMessageQueue = pMessageQueue;

    HANDLE hTerminateEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
    if (hTerminateEvent == NULL) {
        return false;
    }

    HANDLE hRefreshEvent = CreateEventA(NULL, FALSE, FALSE, NULL);
    if (hRefreshEvent == NULL)
    {
        CloseHandle(hTerminateEvent);
        return false;
    }

    draudio_mutex refreshMutex = draudio_create_mutex();
    if (refreshMutex == NULL)
    {
        CloseHandle(hTerminateEvent);
        CloseHandle(hRefreshEvent);
        return false;
    }

    draudio_mutex mainLock = draudio_create_mutex();
    if (mainLock == NULL)
    {
        CloseHandle(hTerminateEvent);
        CloseHandle(hRefreshEvent);
        draudio_delete_mutex(refreshMutex);
        return false;
    }

    HANDLE hEventCompletionLock = CreateEventA(NULL, FALSE, FALSE, NULL);
    if (hEventCompletionLock == NULL)
    {
        CloseHandle(hTerminateEvent);
        CloseHandle(hRefreshEvent);
        draudio_delete_mutex(refreshMutex);
        draudio_delete_mutex(mainLock);
        return false;
    }


    HANDLE hThread = CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DSound_EventWorkerThreadProc, pEventManager, 0, NULL);
    if (hThread == NULL)
    {
        CloseHandle(hTerminateEvent);
        CloseHandle(hRefreshEvent);
        draudio_delete_mutex(refreshMutex);
        draudio_delete_mutex(mainLock);
        CloseHandle(hEventCompletionLock);
        return false;
    }


    pEventManager->hTerminateEvent      = hTerminateEvent;
    pEventManager->hRefreshEvent        = hRefreshEvent;
    pEventManager->refreshMutex         = refreshMutex;
    pEventManager->mainLock             = mainLock;
    pEventManager->hEventCompletionLock = hEventCompletionLock;
    pEventManager->hThread              = hThread;

    pEventManager->pFirstEvent   = NULL;
    pEventManager->pLastEvent    = NULL;

    return true;
}

/// Shuts down the event manager by closing the thread and event objects.
///
/// @remarks
///     This does not return until the worker thread has been terminated completely.
///     @par
///     This will delete every event, so any pointers to events will be made invalid upon calling this function.
void draudio_uninit_event_manager_dsound(draudio_event_manager_dsound* pEventManager)
{
    assert(pEventManager != NULL);


    // Cleanly delete every event first.
    while (pEventManager->pFirstEvent != NULL) {
        draudio_delete_event_dsound(pEventManager->pFirstEvent);
    }



    // Terminate the thread and wait for the thread to finish executing before freeing the context for real.
    SignalObjectAndWait(pEventManager->hTerminateEvent, pEventManager->hThread, INFINITE, FALSE);

    // Only delete the thread after it has returned naturally.
    CloseHandle(pEventManager->hThread);
    pEventManager->hThread = NULL;


    // Once the thread has been terminated we can delete the terminator and refresher events.
    CloseHandle(pEventManager->hTerminateEvent);
    pEventManager->hTerminateEvent = NULL;

    CloseHandle(pEventManager->hRefreshEvent);
    pEventManager->hRefreshEvent = NULL;

    draudio_delete_mutex(pEventManager->refreshMutex);
    pEventManager->refreshMutex = NULL;

    draudio_delete_mutex(pEventManager->mainLock);
    pEventManager->mainLock = NULL;


    CloseHandle(pEventManager->hEventCompletionLock);
    pEventManager->hEventCompletionLock = NULL;
}


//// End Event Management ////

static GUID _g_DSListenerGUID                       = {0x279AFA84, 0x4981, 0x11CE, {0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60}};
static GUID _g_DirectSoundBuffer8GUID               = {0x6825a449, 0x7524, 0x4d82, {0x92, 0x0f, 0x50, 0xe3, 0x6a, 0xb3, 0xab, 0x1e}};
static GUID _g_DirectSound3DBuffer8GUID             = {0x279AFA86, 0x4981, 0x11CE, {0xA5, 0x21, 0x00, 0x20, 0xAF, 0x0B, 0xE5, 0x60}};
static GUID _g_DirectSoundNotifyGUID                = {0xb0210783, 0x89cd, 0x11d0, {0xaf, 0x08, 0x00, 0xa0, 0xc9, 0x25, 0xcd, 0x16}};
static GUID _g_KSDATAFORMAT_SUBTYPE_PCM_GUID        = {0x00000001, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};
static GUID _g_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT_GUID = {0x00000003, 0x0000, 0x0010, {0x80, 0x00, 0x00, 0xaa, 0x00, 0x38, 0x9b, 0x71}};

#ifdef __cplusplus
static GUID g_DSListenerGUID                        = _g_DSListenerGUID;
static GUID g_DirectSoundBuffer8GUID                = _g_DirectSoundBuffer8GUID;
static GUID g_DirectSound3DBuffer8GUID              = _g_DirectSound3DBuffer8GUID;
static GUID g_DirectSoundNotifyGUID                 = _g_DirectSoundNotifyGUID;
static GUID g_KSDATAFORMAT_SUBTYPE_PCM_GUID         = _g_KSDATAFORMAT_SUBTYPE_PCM_GUID;
static GUID g_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT_GUID  = _g_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT_GUID;
#else
static GUID* g_DSListenerGUID                       = &_g_DSListenerGUID;
static GUID* g_DirectSoundBuffer8GUID               = &_g_DirectSoundBuffer8GUID;
static GUID* g_DirectSound3DBuffer8GUID             = &_g_DirectSound3DBuffer8GUID;
static GUID* g_DirectSoundNotifyGUID                = &_g_DirectSoundNotifyGUID;
static GUID* g_KSDATAFORMAT_SUBTYPE_PCM_GUID        = &_g_KSDATAFORMAT_SUBTYPE_PCM_GUID;
static GUID* g_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT_GUID = &_g_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT_GUID;
#endif


typedef HRESULT (WINAPI * pDirectSoundCreate8Proc)(LPCGUID pcGuidDevice, LPDIRECTSOUND8 *ppDS8, LPUNKNOWN pUnkOuter);
typedef HRESULT (WINAPI * pDirectSoundEnumerateAProc)(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext);
typedef HRESULT (WINAPI * pDirectSoundCaptureCreate8Proc)(LPCGUID pcGuidDevice, LPDIRECTSOUNDCAPTURE8 *ppDSC8, LPUNKNOWN pUnkOuter);
typedef HRESULT (WINAPI * pDirectSoundCaptureEnumerateAProc)(LPDSENUMCALLBACKA pDSEnumCallback, LPVOID pContext);

typedef struct
{
    /// A pointer to the GUID of the device. This will be set to all zeros for the default device.
    GUID guid;

    /// The description of the device.
    char description[256];

    /// The module name of the DirectSound driver corresponding to this device.
    char moduleName[256];

} draudio_device_info_dsound;

typedef struct
{
    /// The base context data. This must always be the first item in the struct.
    draudio_context base;

    /// A handle to the dsound.dll file that was loaded by LoadLibrary().
    HMODULE hDSoundDLL;

    // DirectSound APIs.
    pDirectSoundCreate8Proc pDirectSoundCreate8;
    pDirectSoundEnumerateAProc pDirectSoundEnumerateA;
    pDirectSoundCaptureCreate8Proc pDirectSoundCaptureCreate8;
    pDirectSoundCaptureEnumerateAProc pDirectSoundCaptureEnumerateA;


    /// The number of output devices that were iterated when the context was created. This is static, so if the user was to unplug
    /// a device one would need to re-create the context.
    unsigned int outputDeviceCount;

    /// The buffer containing the list of enumerated output devices.
    draudio_device_info_dsound outputDeviceInfo[DRAUDIO_MAX_DEVICE_COUNT];


    /// The number of capture devices that were iterated when the context was created. This is static, so if the user was to unplug
    /// a device one would need to re-create the context.
    unsigned int inputDeviceCount;

    /// The buffer containing the list of enumerated input devices.
    draudio_device_info_dsound inputDeviceInfo[DRAUDIO_MAX_DEVICE_COUNT];


    /// The event manager.
    draudio_event_manager_dsound eventManager;


    /// The message queue.
    draudio_message_queue_dsound messageQueue;

} draudio_context_dsound;

typedef struct
{
    /// The base device data. This must always be the first item in the struct.
    draudio_device base;

    /// A pointer to the DIRECTSOUND object that was created with DirectSoundCreate8().
    LPDIRECTSOUND8 pDS;

    /// A pointer to the DIRECTSOUNDBUFFER object for the primary buffer.
    LPDIRECTSOUNDBUFFER pDSPrimaryBuffer;

    /// A pointer to the DIRECTSOUND3DLISTENER8 object associated with the device.
    LPDIRECTSOUND3DLISTENER pDSListener;

} draudio_device_dsound;

typedef struct
{
    /// The base buffer data. This must always be the first item in the struct.
    draudio_buffer base;

    /// A pointer to the DirectSound buffer object.
    LPDIRECTSOUNDBUFFER8 pDSBuffer;

    /// A pointer to the 3D DirectSound buffer object. This will be NULL if 3D positioning is disabled for the buffer.
    LPDIRECTSOUND3DBUFFER pDSBuffer3D;

    /// A pointer to the object for handling notification events.
    LPDIRECTSOUNDNOTIFY pDSNotify;

    /// The current playback state.
    draudio_playback_state playbackState;


    /// The number of marker events that have been registered. This will never be more than DRAUDIO_MAX_MARKER_COUNT.
    unsigned int markerEventCount;

    /// The marker events.
    draudio_event_dsound* pMarkerEvents[DRAUDIO_MAX_MARKER_COUNT];

    /// The event to trigger when the sound is stopped.
    draudio_event_dsound* pStopEvent;

    /// The event to trigger when the sound is paused.
    draudio_event_dsound* pPauseEvent;

    /// The event to trigger when the sound is played or resumed.
    draudio_event_dsound* pPlayEvent;


    /// The size in bytes of the buffer's extra data.
    unsigned int extraDataSize;

    /// The buffer's extra data.
    unsigned char pExtraData[1];

} draudio_buffer_dsound;


void draudio_activate_buffer_events_dsound(draudio_buffer* pBuffer)
{
    draudio_buffer_dsound* pBufferDS = (draudio_buffer_dsound*)pBuffer;
    assert(pBufferDS != NULL);

    unsigned int dwPositionNotifies = 0;
    DSBPOSITIONNOTIFY n[DRAUDIO_MAX_MARKER_COUNT + 1];        // +1 because we use this array for the markers + stop event.

    // Stop
    if (pBufferDS->pStopEvent != NULL)
    {
        LPDSBPOSITIONNOTIFY pN = n + dwPositionNotifies;
        pN->dwOffset     = DSBPN_OFFSETSTOP;
        pN->hEventNotify = pBufferDS->pStopEvent->hEvent;

        dwPositionNotifies += 1;
    }

    // Markers
    for (unsigned int iMarker = 0; iMarker < pBufferDS->markerEventCount; ++iMarker)
    {
        LPDSBPOSITIONNOTIFY pN = n + dwPositionNotifies;
        pN->dwOffset     = pBufferDS->pMarkerEvents[iMarker]->markerOffset;
        pN->hEventNotify = pBufferDS->pMarkerEvents[iMarker]->hEvent;

        dwPositionNotifies += 1;
    }


    HRESULT hr = IDirectSoundNotify_SetNotificationPositions(pBufferDS->pDSNotify, dwPositionNotifies, n);
#if 0
    if (FAILED(hr)) {
        printf("WARNING: FAILED TO CREATE DIRECTSOUND NOTIFIERS\n");
    }
#else
    (void)hr;
#endif
}

void draudio_deactivate_buffer_events_dsound(draudio_buffer* pBuffer)
{
    draudio_buffer_dsound* pBufferDS = (draudio_buffer_dsound*)pBuffer;
    assert(pBufferDS != NULL);


    HRESULT hr = IDirectSoundNotify_SetNotificationPositions(pBufferDS->pDSNotify, 0, NULL);
#if 0
    if (FAILED(hr)) {
        printf("WARNING: FAILED TO CLEAR DIRECTSOUND NOTIFIERS\n");
    }
#else
    (void)hr;
#endif
}


void draudio_delete_context_dsound(draudio_context* pContext)
{
    draudio_context_dsound* pContextDS = (draudio_context_dsound*)pContext;
    assert(pContextDS != NULL);

    draudio_uninit_event_manager_dsound(&pContextDS->eventManager);

    // The message queue needs to uninitialized after the DirectSound marker notification thread.
    draudio_uninit_message_queue_dsound(&pContextDS->messageQueue);

    FreeLibrary(pContextDS->hDSoundDLL);
    free(pContextDS);
}


unsigned int draudio_get_output_device_count_dsound(draudio_context* pContext)
{
    draudio_context_dsound* pContextDS = (draudio_context_dsound*)pContext;
    assert(pContextDS != NULL);

    return pContextDS->outputDeviceCount;
}

bool draudio_get_output_device_info_dsound(draudio_context* pContext, unsigned int deviceIndex, draudio_device_info* pInfoOut)
{
    draudio_context_dsound* pContextDS = (draudio_context_dsound*)pContext;
    assert(pContextDS != NULL);
    assert(pInfoOut != NULL);

    if (deviceIndex >= pContextDS->outputDeviceCount) {
        return false;
    }


    draudio_strcpy(pInfoOut->description, sizeof(pInfoOut->description), pContextDS->outputDeviceInfo[deviceIndex].description);

    return true;
}


draudio_device* draudio_create_output_device_dsound(draudio_context* pContext, unsigned int deviceIndex)
{
    draudio_context_dsound* pContextDS = (draudio_context_dsound*)pContext;
    assert(pContextDS != NULL);

    if (deviceIndex >= pContextDS->outputDeviceCount) {
        return NULL;
    }


    LPDIRECTSOUND8 pDS;

    // Create the device.
    HRESULT hr;
    if (deviceIndex == 0) {
        hr = pContextDS->pDirectSoundCreate8(NULL, &pDS, NULL);
    } else {
        hr = pContextDS->pDirectSoundCreate8(&pContextDS->outputDeviceInfo[deviceIndex].guid, &pDS, NULL);
    }

    if (FAILED(hr)) {
        return NULL;
    }


    // Set the cooperative level. Must be done before anything else.
    hr = IDirectSound_SetCooperativeLevel(pDS, GetForegroundWindow(), DSSCL_EXCLUSIVE);
    if (FAILED(hr)) {
        IDirectSound_Release(pDS);
        return NULL;
    }


    // Primary buffer.
    DSBUFFERDESC descDSPrimary;
    memset(&descDSPrimary, 0, sizeof(DSBUFFERDESC));
    descDSPrimary.dwSize          = sizeof(DSBUFFERDESC);
    descDSPrimary.dwFlags         = DSBCAPS_PRIMARYBUFFER | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRL3D;
    descDSPrimary.guid3DAlgorithm = DRAUDIO_GUID_NULL;

    LPDIRECTSOUNDBUFFER pDSPrimaryBuffer;
    hr = IDirectSound_CreateSoundBuffer(pDS, &descDSPrimary, &pDSPrimaryBuffer, NULL);
    if (FAILED(hr)) {
        IDirectSound_Release(pDS);
        return NULL;
    }


    WAVEFORMATIEEEFLOATEX wf = {0};
    wf.Format.cbSize               = sizeof(wf);
    wf.Format.wFormatTag           = WAVE_FORMAT_EXTENSIBLE;
    wf.Format.nChannels            = 2;
    wf.Format.nSamplesPerSec       = 48000;
    wf.Format.wBitsPerSample       = 32;
    wf.Format.nBlockAlign          = (wf.Format.nChannels * wf.Format.wBitsPerSample) / 8;
    wf.Format.nAvgBytesPerSec      = wf.Format.nBlockAlign * wf.Format.nSamplesPerSec;
    wf.Samples.wValidBitsPerSample = wf.Format.wBitsPerSample;
    wf.dwChannelMask               = 0;
    wf.SubFormat                   = _g_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT_GUID;
    hr = IDirectSoundBuffer_SetFormat(pDSPrimaryBuffer, (WAVEFORMATEX*)&wf);
    if (FAILED(hr)) {
        IDirectSoundBuffer_Release(pDSPrimaryBuffer);
        IDirectSound_Release(pDS);
        return NULL;
    }


    // Listener.
    LPDIRECTSOUND3DLISTENER pDSListener = NULL;
    hr = IDirectSound3DListener_QueryInterface(pDSPrimaryBuffer, g_DSListenerGUID, (LPVOID*)&pDSListener);
    if (FAILED(hr)) {
        IDirectSoundBuffer_Release(pDSPrimaryBuffer);
        IDirectSound_Release(pDS);
        return NULL;
    }


    draudio_device_dsound* pDeviceDS = (draudio_device_dsound*)malloc(sizeof(draudio_device_dsound));
    if (pDeviceDS != NULL)
    {
        pDeviceDS->base.pContext    = pContext;
        pDeviceDS->pDS              = pDS;
        pDeviceDS->pDSPrimaryBuffer = pDSPrimaryBuffer;
        pDeviceDS->pDSListener      = pDSListener;

        return (draudio_device*)pDeviceDS;
    }
    else
    {
        IDirectSound3DListener_Release(pDSListener);
        IDirectSoundBuffer_Release(pDeviceDS->pDSPrimaryBuffer);
        IDirectSound_Release(pDS);
        return NULL;
    }
}

void draudio_delete_output_device_dsound(draudio_device* pDevice)
{
    draudio_device_dsound* pDeviceDS = (draudio_device_dsound*)pDevice;
    assert(pDeviceDS != NULL);

    draudio_context_dsound* pContextDS = (draudio_context_dsound*)pDevice->pContext;
    assert(pContextDS != NULL);

    // The device id not deleted straight away. Instead we post a message to the message for delayed processing. The reason for this is that buffer
    // deletion is also delayed which means we want to ensure any delayed processing of buffers is handled before deleting the device.
    draudio_message_dsound msg;
    msg.id      = DRAUDIO_MESSAGE_ID_DELETE_DEVICE;
    msg.pBuffer = NULL;
    msg.data.delete_device.pDSListener      = pDeviceDS->pDSListener;
    msg.data.delete_device.pDSPrimaryBuffer = pDeviceDS->pDSPrimaryBuffer;
    msg.data.delete_device.pDS              = pDeviceDS->pDS;
    msg.data.delete_device.pDevice          = pDevice;
    draudio_post_message_dsound(&pContextDS->messageQueue, msg);

#if 0
    IDirectSound3DListener_Release(pDeviceDS->pDSListener);
    IDirectSoundBuffer_Release(pDeviceDS->pDSPrimaryBuffer);
    IDirectSound_Release(pDeviceDS->pDS);
    free(pDeviceDS);
#endif
}


draudio_buffer* draudio_create_buffer_dsound(draudio_device* pDevice, draudio_buffer_desc* pBufferDesc, size_t extraDataSize)
{
    draudio_device_dsound* pDeviceDS = (draudio_device_dsound*)pDevice;
    assert(pDeviceDS != NULL);
    assert(pBufferDesc != NULL);

    // 3D is only valid for mono sounds.
    if (pBufferDesc->channels > 1 && (pBufferDesc->flags & DRAUDIO_ENABLE_3D) != 0) {
        return NULL;
    }

    WAVEFORMATIEEEFLOATEX wf = {0};
    wf.Format.cbSize = sizeof(wf);
    wf.Format.wFormatTag = WAVE_FORMAT_EXTENSIBLE;
    wf.Format.nChannels = (WORD)pBufferDesc->channels;
    wf.Format.nSamplesPerSec = pBufferDesc->sampleRate;
    wf.Format.wBitsPerSample = (WORD)pBufferDesc->bitsPerSample;
    wf.Format.nBlockAlign = (wf.Format.nChannels * wf.Format.wBitsPerSample) / 8;
    wf.Format.nAvgBytesPerSec = wf.Format.nBlockAlign * wf.Format.nSamplesPerSec;
    wf.Samples.wValidBitsPerSample = wf.Format.wBitsPerSample;
    wf.dwChannelMask = 0;

    if (pBufferDesc->format == draudio_format_pcm) {
        wf.SubFormat = _g_KSDATAFORMAT_SUBTYPE_PCM_GUID;
    } else if (pBufferDesc->format == draudio_format_float) {
        wf.SubFormat = _g_KSDATAFORMAT_SUBTYPE_IEEE_FLOAT_GUID;
    } else {
        return NULL;
    }



    // We want to try and create a 3D enabled buffer, however this will fail whenever the number of channels is > 1. In this case
    // we do not want to attempt to create a 3D enabled buffer because it will just fail anyway. Instead we'll just create a normal
    // buffer with panning enabled.
    DSBUFFERDESC descDS;
    memset(&descDS, 0, sizeof(DSBUFFERDESC));
    descDS.dwSize          = sizeof(DSBUFFERDESC);
    descDS.dwFlags         = DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLPOSITIONNOTIFY | DSBCAPS_GETCURRENTPOSITION2 | DSBCAPS_GLOBALFOCUS;
    descDS.dwBufferBytes   = (DWORD)pBufferDesc->sizeInBytes;
    descDS.lpwfxFormat     = (WAVEFORMATEX*)&wf;

    LPDIRECTSOUNDBUFFER8  pDSBuffer   = NULL;
    LPDIRECTSOUND3DBUFFER pDSBuffer3D = NULL;
    if ((pBufferDesc->flags & DRAUDIO_ENABLE_3D) == 0)
    {
        // 3D Disabled.
        descDS.dwFlags |= DSBCAPS_CTRLPAN;

        LPDIRECTSOUNDBUFFER pDSBufferTemp;
        HRESULT hr = IDirectSound_CreateSoundBuffer(pDeviceDS->pDS, &descDS, &pDSBufferTemp, NULL);
        if (FAILED(hr)) {
            return NULL;
        }

        hr = IDirectSoundBuffer_QueryInterface(pDSBufferTemp, g_DirectSoundBuffer8GUID, (void**)&pDSBuffer);
        if (FAILED(hr)) {
            IDirectSoundBuffer_Release(pDSBufferTemp);
            return NULL;
        }
        IDirectSoundBuffer_Release(pDSBufferTemp);
    }
    else
    {
        // 3D Enabled.
        descDS.dwFlags |= DSBCAPS_CTRL3D;
        descDS.guid3DAlgorithm = DS3DALG_DEFAULT;

        LPDIRECTSOUNDBUFFER pDSBufferTemp;
        HRESULT hr = IDirectSound_CreateSoundBuffer(pDeviceDS->pDS, &descDS, &pDSBufferTemp, NULL);
        if (FAILED(hr)) {
            return NULL;
        }

        hr = IDirectSoundBuffer_QueryInterface(pDSBufferTemp, g_DirectSoundBuffer8GUID, (void**)&pDSBuffer);
        if (FAILED(hr)) {
            IDirectSoundBuffer_Release(pDSBufferTemp);
            return NULL;
        }
        IDirectSoundBuffer_Release(pDSBufferTemp);


        hr = IDirectSoundBuffer_QueryInterface(pDSBuffer, g_DirectSound3DBuffer8GUID, (void**)&pDSBuffer3D);
        if (FAILED(hr)) {
            return NULL;
        }

        IDirectSound3DBuffer_SetPosition(pDSBuffer3D, 0, 0, 0, DS3D_IMMEDIATE);

        if ((pBufferDesc->flags & DRAUDIO_RELATIVE_3D) != 0) {
            IDirectSound3DBuffer_SetMode(pDSBuffer3D, DS3DMODE_HEADRELATIVE, DS3D_IMMEDIATE);
        }
    }



    // We need to create a notification object so we can notify the host application when the playback buffer hits a certain point.
    LPDIRECTSOUNDNOTIFY pDSNotify;
    HRESULT hr = IDirectSoundBuffer8_QueryInterface(pDSBuffer, g_DirectSoundNotifyGUID, (void**)&pDSNotify);
    if (FAILED(hr)) {
        IDirectSound3DBuffer_Release(pDSBuffer3D);
        IDirectSoundBuffer8_Release(pDSBuffer);
        return NULL;
    }


    draudio_buffer_dsound* pBufferDS = (draudio_buffer_dsound*)malloc(sizeof(draudio_buffer_dsound) - sizeof(pBufferDS->pExtraData) + extraDataSize);
    if (pBufferDS == NULL) {
        IDirectSound3DBuffer_Release(pDSBuffer3D);
        IDirectSoundBuffer8_Release(pDSBuffer);
        return NULL;
    }

    pBufferDS->base.pDevice      = pDevice;
    pBufferDS->pDSBuffer         = pDSBuffer;
    pBufferDS->pDSBuffer3D       = pDSBuffer3D;
    pBufferDS->pDSNotify         = pDSNotify;
    pBufferDS->playbackState     = draudio_stopped;

    pBufferDS->markerEventCount  = 0;
    memset(pBufferDS->pMarkerEvents, 0, sizeof(pBufferDS->pMarkerEvents));
    pBufferDS->pStopEvent        = NULL;
    pBufferDS->pPauseEvent       = NULL;
    pBufferDS->pPlayEvent        = NULL;



    // Fill with initial data, if applicable.
    if (pBufferDesc->pData != NULL) {
        draudio_set_buffer_data((draudio_buffer*)pBufferDS, 0, pBufferDesc->pData, pBufferDesc->sizeInBytes);
    }

    return (draudio_buffer*)pBufferDS;
}

void draudio_delete_buffer_dsound(draudio_buffer* pBuffer)
{
    draudio_buffer_dsound* pBufferDS = (draudio_buffer_dsound*)pBuffer;
    assert(pBufferDS != NULL);
    assert(pBuffer->pDevice != NULL);

    draudio_context_dsound* pContextDS = (draudio_context_dsound*)pBuffer->pDevice->pContext;
    assert(pContextDS != NULL);


    // Deactivate the DirectSound notify events for sanity.
    draudio_deactivate_buffer_events_dsound(pBuffer);


    draudio_message_dsound msg;
    msg.id      = DRAUDIO_MESSAGE_ID_DELETE_BUFFER;
    msg.pBuffer = pBuffer;
    msg.data.delete_buffer.pDSNotify   = pBufferDS->pDSNotify;
    msg.data.delete_buffer.pDSBuffer3D = pBufferDS->pDSBuffer3D;
    msg.data.delete_buffer.pDSBuffer   = pBufferDS->pDSBuffer;
    draudio_post_message_dsound(&pContextDS->messageQueue, msg);

#if 0
    if (pBufferDS->pDSNotify != NULL) {
        IDirectSoundNotify_Release(pBufferDS->pDSNotify);
    }

    if (pBufferDS->pDSBuffer3D != NULL) {
        IDirectSound3DBuffer_Release(pBufferDS->pDSBuffer3D);
    }

    if (pBufferDS->pDSBuffer != NULL) {
        IDirectSoundBuffer8_Release(pBufferDS->pDSBuffer);
    }

    free(pBufferDS);
#endif
}


unsigned int draudio_get_buffer_extra_data_size_dsound(draudio_buffer* pBuffer)
{
    draudio_buffer_dsound* pBufferDS = (draudio_buffer_dsound*)pBuffer;
    assert(pBufferDS != NULL);

    return pBufferDS->extraDataSize;
}

void* draudio_get_buffer_extra_data_dsound(draudio_buffer* pBuffer)
{
    draudio_buffer_dsound* pBufferDS = (draudio_buffer_dsound*)pBuffer;
    assert(pBufferDS != NULL);

    return pBufferDS->pExtraData;
}


void draudio_set_buffer_data_dsound(draudio_buffer* pBuffer, size_t offset, const void* pData, size_t dataSizeInBytes)
{
    draudio_buffer_dsound* pBufferDS = (draudio_buffer_dsound*)pBuffer;
    assert(pBufferDS != NULL);
    assert(pData != NULL);

    LPVOID lpvWrite;
    DWORD dwLength;
    HRESULT hr = IDirectSoundBuffer8_Lock(pBufferDS->pDSBuffer, (DWORD)offset, (DWORD)dataSizeInBytes, &lpvWrite, &dwLength, NULL, NULL, 0);
    if (FAILED(hr)) {
        return;
    }

    assert(dataSizeInBytes <= dwLength);
    memcpy(lpvWrite, pData, dataSizeInBytes);

    hr = IDirectSoundBuffer8_Unlock(pBufferDS->pDSBuffer, lpvWrite, dwLength, NULL, 0);
    if (FAILED(hr)) {
        return;
    }
}


void draudio_play_dsound(draudio_buffer* pBuffer, bool loop)
{
    draudio_buffer_dsound* pBufferDS = (draudio_buffer_dsound*)pBuffer;
    assert(pBufferDS != NULL);

    bool postEvent = true;
    if (pBufferDS->playbackState == draudio_playing) {
        postEvent = false;
    }


    // Events need to be activated.
    if (pBufferDS->playbackState == draudio_stopped) {
        draudio_activate_buffer_events_dsound(pBuffer);
    }


    DWORD dwFlags = 0;
    if (loop) {
        dwFlags |= DSBPLAY_LOOPING;
    }

    pBufferDS->playbackState = draudio_playing;
    IDirectSoundBuffer8_Play(pBufferDS->pDSBuffer, 0, 0, dwFlags);

    // If we have a play event we need to signal the event which will cause the worker thread to call the callback function.
    if (pBufferDS->pPlayEvent != NULL && postEvent) {
        SetEvent(pBufferDS->pPlayEvent->hEvent);
    }
}

void draudio_pause_dsound(draudio_buffer* pBuffer)
{
    draudio_buffer_dsound* pBufferDS = (draudio_buffer_dsound*)pBuffer;
    assert(pBufferDS != NULL);

    if (pBufferDS->playbackState == draudio_playing)
    {
        pBufferDS->playbackState = draudio_paused;
        IDirectSoundBuffer8_Stop(pBufferDS->pDSBuffer);

        // If we have a pause event we need to signal the event which will cause the worker thread to call the callback function.
        if (pBufferDS->pPlayEvent != NULL) {
            SetEvent(pBufferDS->pPauseEvent->hEvent);
        }
    }
}

void draudio_stop_dsound(draudio_buffer* pBuffer)
{
    draudio_buffer_dsound* pBufferDS = (draudio_buffer_dsound*)pBuffer;
    assert(pBufferDS != NULL);

    if (pBufferDS->playbackState == draudio_playing)
    {
        pBufferDS->playbackState = draudio_stopped;
        IDirectSoundBuffer8_Stop(pBufferDS->pDSBuffer);
        IDirectSoundBuffer8_SetCurrentPosition(pBufferDS->pDSBuffer, 0);
    }
    else if (pBufferDS->playbackState == draudio_paused)
    {
        pBufferDS->playbackState = draudio_stopped;
        IDirectSoundBuffer8_SetCurrentPosition(pBufferDS->pDSBuffer, 0);

        if (pBufferDS->pStopEvent != NULL) {
            SetEvent(pBufferDS->pStopEvent->hEvent);
        }
    }
}

draudio_playback_state draudio_get_playback_state_dsound(draudio_buffer* pBuffer)
{
    draudio_buffer_dsound* pBufferDS = (draudio_buffer_dsound*)pBuffer;
    assert(pBufferDS != NULL);

    return pBufferDS->playbackState;
}


void draudio_set_playback_position_dsound(draudio_buffer* pBuffer, unsigned int position)
{
    draudio_buffer_dsound* pBufferDS = (draudio_buffer_dsound*)pBuffer;
    assert(pBufferDS != NULL);

    IDirectSoundBuffer8_SetCurrentPosition(pBufferDS->pDSBuffer, position);
}

unsigned int draudio_get_playback_position_dsound(draudio_buffer* pBuffer)
{
    draudio_buffer_dsound* pBufferDS = (draudio_buffer_dsound*)pBuffer;
    assert(pBufferDS != NULL);

    DWORD position;
    HRESULT hr = IDirectSoundBuffer8_GetCurrentPosition(pBufferDS->pDSBuffer, &position, NULL);
    if (FAILED(hr)) {
        return 0;
    }

    return position;
}


void draudio_set_pan_dsound(draudio_buffer* pBuffer, float pan)
{
    draudio_buffer_dsound* pBufferDS = (draudio_buffer_dsound*)pBuffer;
    assert(pBufferDS != NULL);

    LONG panDB;
    if (pan == 0) {
        panDB = DSBPAN_CENTER;
    } else {
        if (pan > 1) {
            panDB = DSBPAN_RIGHT;
        } else if (pan < -1) {
            panDB = DSBPAN_LEFT;
        } else {
            if (pan < 0) {
                panDB =  (LONG)((20*log10f(1 + pan)) * 100);
            } else {
                panDB = -(LONG)((20*log10f(1 - pan)) * 100);
            }
        }
    }

    IDirectSoundBuffer_SetPan(pBufferDS->pDSBuffer, panDB);
}

float draudio_get_pan_dsound(draudio_buffer* pBuffer)
{
    draudio_buffer_dsound* pBufferDS = (draudio_buffer_dsound*)pBuffer;
    assert(pBufferDS != NULL);

    LONG panDB;
    HRESULT hr = IDirectSoundBuffer_GetPan(pBufferDS->pDSBuffer, &panDB);
    if (FAILED(hr)) {
        return 0;
    }


    if (panDB < 0) {
        return -(1 - (float)(1.0f / powf(10.0f, -panDB / (20.0f*100.0f))));
    }

    if (panDB > 0) {
        return  (1 - (float)(1.0f / powf(10.0f,  panDB / (20.0f*100.0f))));
    }

    return 0;
}


void draudio_set_volume_dsound(draudio_buffer* pBuffer, float volume)
{
    draudio_buffer_dsound* pBufferDS = (draudio_buffer_dsound*)pBuffer;
    assert(pBufferDS != NULL);

    LONG volumeDB;
    if (volume > 0) {
        if (volume < 1) {
            volumeDB = (LONG)((20*log10f(volume)) * 100);
        } else {
            volumeDB = DSBVOLUME_MAX;
        }
    } else {
        volumeDB = DSBVOLUME_MIN;
    }

    IDirectSoundBuffer_SetVolume(pBufferDS->pDSBuffer, volumeDB);
}

float draudio_get_volume_dsound(draudio_buffer* pBuffer)
{
    draudio_buffer_dsound* pBufferDS = (draudio_buffer_dsound*)pBuffer;
    assert(pBufferDS != NULL);

    LONG volumeDB;
    HRESULT hr = IDirectSoundBuffer_GetVolume(pBufferDS->pDSBuffer, &volumeDB);
    if (FAILED(hr)) {
        return 1;
    }

    return (float)(1.0f / powf(10.0f, -volumeDB / (20.0f*100.0f)));
}


void draudio_remove_markers_dsound(draudio_buffer* pBuffer)
{
    draudio_buffer_dsound* pBufferDS = (draudio_buffer_dsound*)pBuffer;
    assert(pBufferDS != NULL);

    for (unsigned int iMarker = 0; iMarker < pBufferDS->markerEventCount; ++iMarker)
    {
        if (pBufferDS->pMarkerEvents[iMarker] != NULL) {
            draudio_delete_event_dsound(pBufferDS->pMarkerEvents[iMarker]);
            pBufferDS->pMarkerEvents[iMarker] = NULL;
        }
    }

    pBufferDS->markerEventCount = 0;
}

bool draudio_register_marker_callback_dsound(draudio_buffer* pBuffer, size_t offsetInBytes, draudio_event_callback_proc callback, unsigned int eventID, void* pUserData)
{
    draudio_buffer_dsound* pBufferDS = (draudio_buffer_dsound*)pBuffer;
    assert(pBufferDS != NULL);
    assert(pBufferDS->markerEventCount <= DRAUDIO_MAX_MARKER_COUNT);

    if (pBufferDS->markerEventCount == DRAUDIO_MAX_MARKER_COUNT) {
        // Too many markers.
        return false;
    }

    draudio_context_dsound* pContextDS = (draudio_context_dsound*)pBuffer->pDevice->pContext;
    assert(pContextDS != NULL);

    draudio_event_dsound* pEvent = draudio_create_event_dsound(&pContextDS->eventManager, callback, pBuffer, eventID, pUserData);
    if (pEvent == NULL) {
        return false;
    }

    // draudio_create_event_dsound() will initialize the marker offset to 0, so we'll need to set it manually here.
    pEvent->markerOffset = (DWORD)offsetInBytes;

    pBufferDS->pMarkerEvents[pBufferDS->markerEventCount] = pEvent;
    pBufferDS->markerEventCount += 1;

    return true;
}

bool draudio_register_stop_callback_dsound(draudio_buffer* pBuffer, draudio_event_callback_proc callback, void* pUserData)
{
    draudio_buffer_dsound* pBufferDS = (draudio_buffer_dsound*)pBuffer;
    assert(pBufferDS != NULL);

    if (callback == NULL)
    {
        if (pBufferDS->pStopEvent != NULL) {
            draudio_delete_event_dsound(pBufferDS->pStopEvent);
            pBufferDS->pStopEvent = NULL;
        }

        return true;
    }
    else
    {
        draudio_context_dsound* pContextDS = (draudio_context_dsound*)pBuffer->pDevice->pContext;

        // If we already have a stop event, just replace the existing one.
        if (pBufferDS->pStopEvent != NULL) {
            draudio_update_event_dsound(pBufferDS->pStopEvent, callback, pUserData);
        } else {
            pBufferDS->pStopEvent = draudio_create_event_dsound(&pContextDS->eventManager, callback, pBuffer, DRAUDIO_EVENT_ID_STOP, pUserData);
        }

        return pBufferDS->pStopEvent != NULL;
    }
}

bool draudio_register_pause_callback_dsound(draudio_buffer* pBuffer, draudio_event_callback_proc callback, void* pUserData)
{
    draudio_buffer_dsound* pBufferDS = (draudio_buffer_dsound*)pBuffer;
    assert(pBufferDS != NULL);

    if (callback == NULL)
    {
        if (pBufferDS->pPauseEvent != NULL) {
            draudio_delete_event_dsound(pBufferDS->pPauseEvent);
            pBufferDS->pPauseEvent = NULL;
        }

        return true;
    }
    else
    {
        draudio_context_dsound* pContextDS = (draudio_context_dsound*)pBuffer->pDevice->pContext;

        // If we already have a stop event, just replace the existing one.
        if (pBufferDS->pPauseEvent != NULL) {
            draudio_update_event_dsound(pBufferDS->pPauseEvent, callback, pUserData);
        } else {
            pBufferDS->pPauseEvent = draudio_create_event_dsound(&pContextDS->eventManager, callback, pBuffer, DRAUDIO_EVENT_ID_PAUSE, pUserData);
        }

        return pBufferDS->pPauseEvent != NULL;
    }
}

bool draudio_register_play_callback_dsound(draudio_buffer* pBuffer, draudio_event_callback_proc callback, void* pUserData)
{
    draudio_buffer_dsound* pBufferDS = (draudio_buffer_dsound*)pBuffer;
    assert(pBufferDS != NULL);

    if (callback == NULL)
    {
        if (pBufferDS->pPlayEvent != NULL) {
            draudio_delete_event_dsound(pBufferDS->pPlayEvent);
            pBufferDS->pPlayEvent = NULL;
        }

        return true;
    }
    else
    {
        draudio_context_dsound* pContextDS = (draudio_context_dsound*)pBuffer->pDevice->pContext;

        // If we already have a stop event, just replace the existing one.
        if (pBufferDS->pPlayEvent != NULL) {
            draudio_update_event_dsound(pBufferDS->pPlayEvent, callback, pUserData);
        } else {
            pBufferDS->pPlayEvent = draudio_create_event_dsound(&pContextDS->eventManager, callback, pBuffer, DRAUDIO_EVENT_ID_PLAY, pUserData);
        }

        return pBufferDS->pPlayEvent != NULL;
    }
}



void draudio_set_position_dsound(draudio_buffer* pBuffer, float x, float y, float z)
{
    draudio_buffer_dsound* pBufferDS = (draudio_buffer_dsound*)pBuffer;
    assert(pBufferDS != NULL);

    if (pBufferDS->pDSBuffer3D != NULL) {
        IDirectSound3DBuffer_SetPosition(pBufferDS->pDSBuffer3D, x, y, z, DS3D_IMMEDIATE);
    }
}

void draudio_get_position_dsound(draudio_buffer* pBuffer, float* pPosOut)
{
    draudio_buffer_dsound* pBufferDS = (draudio_buffer_dsound*)pBuffer;
    assert(pBufferDS != NULL);
    assert(pPosOut != NULL);

    if (pBufferDS->pDSBuffer3D != NULL)
    {
        D3DVECTOR pos;
        IDirectSound3DBuffer_GetPosition(pBufferDS->pDSBuffer3D, &pos);

        pPosOut[0] = pos.x;
        pPosOut[1] = pos.y;
        pPosOut[2] = pos.z;
    }
    else
    {
        pPosOut[0] = 0;
        pPosOut[1] = 1;
        pPosOut[2] = 2;
    }
}


void draudio_set_listener_position_dsound(draudio_device* pDevice, float x, float y, float z)
{
    draudio_device_dsound* pDeviceDS = (draudio_device_dsound*)pDevice;
    assert(pDeviceDS != NULL);

    IDirectSound3DListener_SetPosition(pDeviceDS->pDSListener, x, y, z, DS3D_IMMEDIATE);
}

void draudio_get_listener_position_dsound(draudio_device* pDevice, float* pPosOut)
{
    draudio_device_dsound* pDeviceDS = (draudio_device_dsound*)pDevice;
    assert(pDeviceDS != NULL);
    assert(pPosOut != NULL);

    D3DVECTOR pos;
    IDirectSound3DListener_GetPosition(pDeviceDS->pDSListener, &pos);

    pPosOut[0] = pos.x;
    pPosOut[1] = pos.y;
    pPosOut[2] = pos.z;
}


void draudio_set_listener_orientation_dsound(draudio_device* pDevice, float forwardX, float forwardY, float forwardZ, float upX, float upY, float upZ)
{
    draudio_device_dsound* pDeviceDS = (draudio_device_dsound*)pDevice;
    assert(pDeviceDS != NULL);

    IDirectSound3DListener_SetOrientation(pDeviceDS->pDSListener, forwardX, forwardY, forwardZ, upX, upY, upZ, DS3D_IMMEDIATE);
}

void draudio_get_listener_orientation_dsound(draudio_device* pDevice, float* pForwardOut, float* pUpOut)
{
    draudio_device_dsound* pDeviceDS = (draudio_device_dsound*)pDevice;
    assert(pDeviceDS != NULL);
    assert(pForwardOut != NULL);
    assert(pUpOut != NULL);

    D3DVECTOR forward;
    D3DVECTOR up;
    IDirectSound3DListener_GetOrientation(pDeviceDS->pDSListener, &forward, &up);

    pForwardOut[0] = forward.x;
    pForwardOut[1] = forward.y;
    pForwardOut[2] = forward.z;

    pUpOut[0] = up.x;
    pUpOut[1] = up.y;
    pUpOut[2] = up.z;
}

void draudio_set_3d_mode_dsound(draudio_buffer* pBuffer, draudio_3d_mode mode)
{
    draudio_buffer_dsound* pBufferDS = (draudio_buffer_dsound*)pBuffer;
    assert(pBufferDS != NULL);

    if (pBufferDS->pDSBuffer3D == NULL) {
        return;
    }


    DWORD dwMode = DS3DMODE_NORMAL;
    if (mode == draudio_3d_mode_relative) {
        dwMode = DS3DMODE_HEADRELATIVE;
    } else if (mode == draudio_3d_mode_disabled) {
        dwMode = DS3DMODE_DISABLE;
    }

    IDirectSound3DBuffer_SetMode(pBufferDS->pDSBuffer3D, dwMode, DS3D_IMMEDIATE);
}

draudio_3d_mode draudio_get_3d_mode_dsound(draudio_buffer* pBuffer)
{
    draudio_buffer_dsound* pBufferDS = (draudio_buffer_dsound*)pBuffer;
    assert(pBufferDS != NULL);

    if (pBufferDS->pDSBuffer3D == NULL) {
        return draudio_3d_mode_disabled;
    }


    DWORD dwMode;
    if (FAILED(IDirectSound3DBuffer_GetMode(pBufferDS->pDSBuffer3D, &dwMode))) {
        return draudio_3d_mode_disabled;
    }


    if (dwMode == DS3DMODE_NORMAL) {
        return draudio_3d_mode_absolute;
    }

    if (dwMode == DS3DMODE_HEADRELATIVE) {
        return draudio_3d_mode_relative;
    }

    return draudio_3d_mode_disabled;
}


static BOOL CALLBACK DSEnumCallback_OutputDevices(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext)
{
    // From MSDN:
    //
    // The first device enumerated is always called the Primary Sound Driver, and the lpGUID parameter of the callback is
    // NULL. This device represents the preferred output device set by the user in Control Panel.

    draudio_context_dsound* pContextDS = (draudio_context_dsound*)lpContext;
    assert(pContextDS != NULL);

    if (pContextDS->outputDeviceCount < DRAUDIO_MAX_DEVICE_COUNT)
    {
        if (lpGuid != NULL) {
            memcpy(&pContextDS->outputDeviceInfo[pContextDS->outputDeviceCount].guid, lpGuid, sizeof(GUID));
        } else {
            memset(&pContextDS->outputDeviceInfo[pContextDS->outputDeviceCount].guid, 0, sizeof(GUID));
        }

        draudio_strcpy(pContextDS->outputDeviceInfo[pContextDS->outputDeviceCount].description, 256, lpcstrDescription);
        draudio_strcpy(pContextDS->outputDeviceInfo[pContextDS->outputDeviceCount].moduleName,  256, lpcstrModule);

        pContextDS->outputDeviceCount += 1;
        return TRUE;
    }
    else
    {
        // Ran out of device slots.
        return FALSE;
    }
}

static BOOL CALLBACK DSEnumCallback_InputDevices(LPGUID lpGuid, LPCSTR lpcstrDescription, LPCSTR lpcstrModule, LPVOID lpContext)
{
    // From MSDN:
    //
    // The first device enumerated is always called the Primary Sound Driver, and the lpGUID parameter of the callback is
    // NULL. This device represents the preferred output device set by the user in Control Panel.

    draudio_context_dsound* pContextDS = (draudio_context_dsound*)lpContext;
    assert(pContextDS != NULL);

    if (pContextDS->inputDeviceCount < DRAUDIO_MAX_DEVICE_COUNT)
    {
        if (lpGuid != NULL) {
            memcpy(&pContextDS->inputDeviceInfo[pContextDS->inputDeviceCount].guid, lpGuid, sizeof(GUID));
        } else {
            memset(&pContextDS->inputDeviceInfo[pContextDS->inputDeviceCount].guid, 0, sizeof(GUID));
        }

        draudio_strcpy(pContextDS->inputDeviceInfo[pContextDS->inputDeviceCount].description, 256, lpcstrDescription);
        draudio_strcpy(pContextDS->inputDeviceInfo[pContextDS->inputDeviceCount].moduleName,  256, lpcstrModule);

        pContextDS->inputDeviceCount += 1;
        return TRUE;
    }
    else
    {
        // Ran out of device slots.
        return FALSE;
    }
}

draudio_context* draudio_create_context_dsound()
{
    // Load the DLL.
    HMODULE hDSoundDLL = LoadLibraryW(L"dsound.dll");
    if (hDSoundDLL == NULL) {
        return NULL;
    }


    // Retrieve the APIs.
    pDirectSoundCreate8Proc pDirectSoundCreate8 = (pDirectSoundCreate8Proc)GetProcAddress(hDSoundDLL, "DirectSoundCreate8");
    if (pDirectSoundCreate8 == NULL){
        FreeLibrary(hDSoundDLL);
        return NULL;
    }

    pDirectSoundEnumerateAProc pDirectSoundEnumerateA = (pDirectSoundEnumerateAProc)GetProcAddress(hDSoundDLL, "DirectSoundEnumerateA");
    if (pDirectSoundEnumerateA == NULL){
        FreeLibrary(hDSoundDLL);
        return NULL;
    }

    pDirectSoundCaptureCreate8Proc pDirectSoundCaptureCreate8 = (pDirectSoundCaptureCreate8Proc)GetProcAddress(hDSoundDLL, "DirectSoundCaptureCreate8");
    if (pDirectSoundCaptureCreate8 == NULL) {
        FreeLibrary(hDSoundDLL);
        return NULL;
    }

    pDirectSoundCaptureEnumerateAProc pDirectSoundCaptureEnumerateA = (pDirectSoundCaptureEnumerateAProc)GetProcAddress(hDSoundDLL, "DirectSoundCaptureEnumerateA");
    if (pDirectSoundCaptureEnumerateA == NULL ){
        FreeLibrary(hDSoundDLL);
        return NULL;
    }



    // At this point we can almost certainly assume DirectSound is usable so we'll now go ahead and create the context.
    draudio_context_dsound* pContext = (draudio_context_dsound*)malloc(sizeof(draudio_context_dsound));
    if (pContext != NULL)
    {
        pContext->base.delete_context             = draudio_delete_context_dsound;
        pContext->base.create_output_device       = draudio_create_output_device_dsound;
        pContext->base.delete_output_device       = draudio_delete_output_device_dsound;
        pContext->base.get_output_device_count    = draudio_get_output_device_count_dsound;
        pContext->base.get_output_device_info     = draudio_get_output_device_info_dsound;
        pContext->base.create_buffer              = draudio_create_buffer_dsound;
        pContext->base.delete_buffer              = draudio_delete_buffer_dsound;
        pContext->base.get_buffer_extra_data_size = draudio_get_buffer_extra_data_size_dsound;
        pContext->base.get_buffer_extra_data      = draudio_get_buffer_extra_data_dsound;
        pContext->base.set_buffer_data            = draudio_set_buffer_data_dsound;
        pContext->base.play                       = draudio_play_dsound;
        pContext->base.pause                      = draudio_pause_dsound;
        pContext->base.stop                       = draudio_stop_dsound;
        pContext->base.get_playback_state         = draudio_get_playback_state_dsound;
        pContext->base.set_playback_position      = draudio_set_playback_position_dsound;
        pContext->base.get_playback_position      = draudio_get_playback_position_dsound;
        pContext->base.set_pan                    = draudio_set_pan_dsound;
        pContext->base.get_pan                    = draudio_get_pan_dsound;
        pContext->base.set_volume                 = draudio_set_volume_dsound;
        pContext->base.get_volume                 = draudio_get_volume_dsound;
        pContext->base.remove_markers             = draudio_remove_markers_dsound;
        pContext->base.register_marker_callback   = draudio_register_marker_callback_dsound;
        pContext->base.register_stop_callback     = draudio_register_stop_callback_dsound;
        pContext->base.register_pause_callback    = draudio_register_pause_callback_dsound;
        pContext->base.register_play_callback     = draudio_register_play_callback_dsound;
        pContext->base.set_position        = draudio_set_position_dsound;
        pContext->base.get_position        = draudio_get_position_dsound;
        pContext->base.set_listener_position      = draudio_set_listener_position_dsound;
        pContext->base.get_listener_position      = draudio_get_listener_position_dsound;
        pContext->base.set_listener_orientation   = draudio_set_listener_orientation_dsound;
        pContext->base.get_listener_orientation   = draudio_get_listener_orientation_dsound;
        pContext->base.set_3d_mode                = draudio_set_3d_mode_dsound;
        pContext->base.get_3d_mode                = draudio_get_3d_mode_dsound;

        pContext->hDSoundDLL                      = hDSoundDLL;
        pContext->pDirectSoundCreate8             = pDirectSoundCreate8;
        pContext->pDirectSoundEnumerateA          = pDirectSoundEnumerateA;
        pContext->pDirectSoundCaptureCreate8      = pDirectSoundCaptureCreate8;
        pContext->pDirectSoundCaptureEnumerateA   = pDirectSoundCaptureEnumerateA;

        // Enumerate output devices.
        pContext->outputDeviceCount = 0;
        pContext->pDirectSoundEnumerateA(DSEnumCallback_OutputDevices, pContext);

        // Enumerate input devices.
        pContext->inputDeviceCount = 0;
        pContext->pDirectSoundCaptureEnumerateA(DSEnumCallback_InputDevices, pContext);

        // The message queue and marker notification thread.
        if (!draudio_init_message_queue_dsound(&pContext->messageQueue) || !draudio_init_event_manager_dsound(&pContext->eventManager, &pContext->messageQueue))
        {
            // Failed to initialize the event manager.
            FreeLibrary(hDSoundDLL);
            free(pContext);

            return NULL;
        }
    }

    return (draudio_context*)pContext;
}
#endif  // !DRAUDIO_BUILD_DSOUND


///////////////////////////////////////////////////////////////////////////////
//
// XAudio2
//
///////////////////////////////////////////////////////////////////////////////

#if 0
#define uuid(x)
#define DX_BUILD
#define INITGUID 1
#include <xaudio2.h>
#endif
#endif

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
