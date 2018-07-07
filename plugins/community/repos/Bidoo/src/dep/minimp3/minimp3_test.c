/*#define MINIMP3_ONLY_MP3*/
/*#define MINIMP3_ONLY_SIMD*/
/*#define MINIMP3_NONSTANDARD_BUT_LOGICAL*/
#define MINIMP3_IMPLEMENTATION
#include "minimp3.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#if defined(_MSC_VER)
    #define strcasecmp(str1, str2) _strnicmp(str1, str2, strlen(str2))
#else
    #include <strings.h>
#endif

static unsigned short read16le(const void *p)
{
    const unsigned char *src = (const unsigned char *)p;
    return ((src[0]) << 0) | ((src[1]) << 8);
}

#ifndef MINIMP3_NO_WAV
static char *wav_header(int hz, int ch, int bips, int data_bytes)
{
    static char hdr[44] = "RIFFsizeWAVEfmt \x10\0\0\0\1\0ch_hz_abpsbabsdatasize";
    unsigned long nAvgBytesPerSec = bips*ch*hz >> 3;
    unsigned int nBlockAlign      = bips*ch >> 3;

    *(int *  )(void*)(hdr + 0x04) = 44 + data_bytes - 8;   /* File size - 8 */
    *(short *)(void*)(hdr + 0x14) = 1;                     /* Integer PCM format */
    *(short *)(void*)(hdr + 0x16) = ch;
    *(int *  )(void*)(hdr + 0x18) = hz;
    *(int *  )(void*)(hdr + 0x1C) = nAvgBytesPerSec;
    *(short *)(void*)(hdr + 0x20) = nBlockAlign;
    *(short *)(void*)(hdr + 0x22) = bips;
    *(int *  )(void*)(hdr + 0x28) = data_bytes;
    return hdr;
}
#endif

static unsigned char *preload(FILE *file, int *data_size)
{
    unsigned char *data;
    *data_size = 0;
    if (!file)
        return 0;
    fseek(file, 0, SEEK_END);
    *data_size = (int)ftell(file);
    fseek(file, 0, SEEK_SET);
    data = (unsigned char*)malloc(*data_size);
    if (!data)
        return 0;
    if ((int)fread(data, 1, *data_size, file) != *data_size)
        exit(1);
    return data;
}

static void decode_file(const unsigned char *buf_mp3, int mp3_size, const unsigned char *buf_ref, int ref_size, FILE *file_out, const int wave_out)
{
    static mp3dec_t mp3d;
    mp3dec_frame_info_t info;
    int i, data_bytes, samples, total_samples = 0, maxdiff = 0;
    double MSE = 0.0, psnr;

    mp3dec_init(&mp3d);
    memset(&info, 0, sizeof(info));
#ifndef MINIMP3_NO_WAV
    if (wave_out && file_out)
        fwrite(wav_header(0, 0, 0, 0), 1, 44, file_out);
#endif
    do
    {
        short pcm[MINIMP3_MAX_SAMPLES_PER_FRAME];
#ifdef MP4_MODE
        int free_format_bytes = 0, frame_size = 0;
        i = mp3d_find_frame(buf_mp3, mp3_size, &free_format_bytes, &frame_size);
        buf_mp3  += i;
        mp3_size -= i;
        if (i && !frame_size)
        {
            printf("warning: skipping %d bytes, frame_size=%d\n", i, frame_size);
            continue;
        }
        if (frame_size > mp3_size)
        {
            printf("error: demux mp3 frame failed: i=%d, frame_size=%d\n", i, frame_size);
            exit(1);
        }
        if (!frame_size)
            break;
        samples = mp3dec_decode_frame(&mp3d, buf_mp3, frame_size, pcm, &info);
#else
        samples = mp3dec_decode_frame(&mp3d, buf_mp3, mp3_size, pcm, &info);
#endif
        if (samples)
        {
            total_samples += samples*info.channels;
            if (buf_ref && ref_size >= samples*info.channels*2)
            {
                for (i = 0; i < samples*info.channels; i++)
                {
                    int MSEtemp = abs((int)pcm[i] - (int)(short)read16le(&buf_ref[i*sizeof(short)]));
                    if (MSEtemp > maxdiff)
                        maxdiff = MSEtemp;
                    MSE += (float)MSEtemp*(float)MSEtemp;
                }
                buf_ref  += samples*info.channels*2;
                ref_size -= samples*info.channels*2;
            }
            if (file_out)
                fwrite(pcm, samples, 2*info.channels, file_out);
        }
        buf_mp3  += info.frame_bytes;
        mp3_size -= info.frame_bytes;
    } while (info.frame_bytes);
#ifndef LIBFUZZER
    MSE /= total_samples ? total_samples : 1;
    if (0 == MSE)
        psnr = 99.0;
    else
        psnr = 10.0*log10(((double)0x7fff*0x7fff)/MSE);
    printf("rate=%d samples=%d max_diff=%d PSNR=%f\n", info.hz, total_samples, maxdiff, psnr);
    if (psnr < 96)
    {
        printf("PSNR compliance failed\n");
        exit(1);
    }
#endif
#ifndef MINIMP3_NO_WAV
    if (wave_out && file_out)
    {
        data_bytes = ftell(file_out) - 44;
        rewind(file_out);
        fwrite(wav_header(info.hz, info.channels, 16, data_bytes), 1, 44, file_out);
    }
#endif
#ifdef MP4_MODE
    if (!total_samples)
    {
        printf("error: mp4 test should decode some samples\n");
        exit(1);
    }
#endif
}

#ifdef LIBFUZZER
int LLVMFuzzerTestOneInput(const uint8_t *Data, size_t Size)
{
    decode_file(Data, Size, 0, 0, 0, 0);
    return 0;
}
#else

#if defined(__arm__) || defined(__aarch64__) || defined(__PPC__)
int main2(int argc, char *argv[]);
int main2(int argc, char *argv[])
#else
int main(int argc, char *argv[])
#endif
{
    int wave_out = 0, ref_size, mp3_size;
    char *ref_file_name    = (argc > 2) ? argv[2] : NULL;
    char *output_file_name = (argc > 3) ? argv[3] : NULL;
    FILE *file_out = NULL;
    if (output_file_name)
    {
        file_out = fopen(output_file_name, "wb");
#ifndef MINIMP3_NO_WAV
        char *ext = strrchr(output_file_name, '.');
        if (ext && !strcasecmp(ext + 1, "wav"))
            wave_out = 1;
#endif
    }
    FILE *file_ref = ref_file_name ? fopen(ref_file_name, "rb") : NULL;
    unsigned char *buf_ref = preload(file_ref, &ref_size);
    if (file_ref)
        fclose(file_ref);
#ifdef __AFL_HAVE_MANUAL_CONTROL
    __AFL_INIT();
    while (__AFL_LOOP(1000)) {
#endif
    char *input_file_name  = (argc > 1) ? argv[1] : NULL;
    if (!input_file_name)
    {
        printf("error: no file names given\n");
        return 1;
    }
    FILE *file_mp3 = fopen(input_file_name, "rb");
    unsigned char *buf_mp3 = preload(file_mp3, &mp3_size);
    if (file_mp3)
        fclose(file_mp3);
    if (!buf_mp3 || !mp3_size)
    {
        printf("error: no input data\n");
        return 1;
    }
    decode_file(buf_mp3, mp3_size, buf_ref, ref_size, file_out, wave_out);
    if (buf_mp3)
        free(buf_mp3);
#ifdef __AFL_HAVE_MANUAL_CONTROL
    }
#endif
    if (buf_ref)
        free(buf_ref);
    if (file_out)
        fclose(file_out);
    return 0;
}

#if defined(__arm__) || defined(__aarch64__) || defined(__PPC__)
static const char *g_files[] = {
    "vectors/ILL2_center2.bit",
    "vectors/ILL2_dual.bit",
    "vectors/ILL2_dynx22.bit",
    "vectors/ILL2_dynx31.bit",
    "vectors/ILL2_dynx32.bit",
    "vectors/ILL2_ext_switching.bit",
    "vectors/ILL2_layer1.bit",
    "vectors/ILL2_layer3.bit",
    "vectors/ILL2_mono.bit",
    "vectors/ILL2_multilingual.bit",
    "vectors/ILL2_overalloc1.bit",
    "vectors/ILL2_overalloc2.bit",
    "vectors/ILL2_prediction.bit",
    "vectors/ILL2_samples.bit",
    "vectors/ILL2_scf63.bit",
    "vectors/ILL2_tca21.bit",
    "vectors/ILL2_tca30.bit",
    "vectors/ILL2_tca30_PC.bit",
    "vectors/ILL2_tca31_mtx0.bit",
    "vectors/ILL2_tca31_mtx2.bit",
    "vectors/ILL2_tca31_PC.bit",
    "vectors/ILL2_tca32_PC.bit",
    "vectors/ILL2_wrongcrc.bit",
    "vectors/ILL4_ext_id1.bit",
    "vectors/ILL4_sync.bit",
    "vectors/ILL4_wrongcrc.bit",
    "vectors/ILL4_wrong_length1.bit",
    "vectors/ILL4_wrong_length2.bit",
    "vectors/l1-fl1.bit",
    "vectors/l1-fl2.bit",
    "vectors/l1-fl3.bit",
    "vectors/l1-fl4.bit",
    "vectors/l1-fl5.bit",
    "vectors/l1-fl6.bit",
    "vectors/l1-fl7.bit",
    "vectors/l1-fl8.bit",
    "vectors/l2-fl10.bit",
    "vectors/l2-fl11.bit",
    "vectors/l2-fl12.bit",
    "vectors/l2-fl13.bit",
    "vectors/l2-fl14.bit",
    "vectors/l2-fl15.bit",
    "vectors/l2-fl16.bit",
    "vectors/l2-nonstandard-fl1_fl2_ff.bit",
    "vectors/l2-nonstandard-free_format.bit",
    "vectors/l2-nonstandard-test32-size.bit",
    "vectors/l2-test32.bit",
    "vectors/l3-compl.bit",
    "vectors/l3-he_32khz.bit",
    "vectors/l3-he_44khz.bit",
    "vectors/l3-he_48khz.bit",
    "vectors/l3-hecommon.bit",
    "vectors/l3-he_free.bit",
    "vectors/l3-he_mode.bit",
    "vectors/l3-nonstandard-big-iscf.bit",
    "vectors/l3-nonstandard-compl-sideinfo-bigvalues.bit",
    "vectors/l3-nonstandard-compl-sideinfo-blocktype.bit",
    "vectors/l3-nonstandard-compl-sideinfo-size.bit",
    "vectors/l3-nonstandard-sideinfo-size.bit",
    "vectors/l3-si.bit",
    "vectors/l3-si_block.bit",
    "vectors/l3-si_huff.bit",
    "vectors/l3-sin1k0db.bit",
    "vectors/l3-test45.bit",
    "vectors/l3-test46.bit",
    "vectors/M2L3_bitrate_16_all.bit",
    "vectors/M2L3_bitrate_22_all.bit",
    "vectors/M2L3_bitrate_24_all.bit",
    "vectors/M2L3_compl24.bit",
    "vectors/M2L3_noise.bit"
};
int main()
{
    size_t i;
    char buf[256];
    char *v[3];
    v[2] = buf;
    for (i = 0; i < sizeof(g_files)/sizeof(g_files[0]); i++)
    {
        int ret;
        const char *file = g_files[i];
        size_t len = strlen(file);
        strcpy(buf, file);
        buf[len - 3] = 'p';
        buf[len - 2] = 'c';
        buf[len - 1] = 'm';
        v[1] = (char*)file;
        printf("%s\n", file);
        ret = main2(3, v);
        if (ret)
            return ret;
    }
    return 0;
}
#endif
#endif
