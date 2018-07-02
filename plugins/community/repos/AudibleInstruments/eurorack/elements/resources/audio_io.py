#!/usr/bin/python2.6
#
# Author: Olivier Gillet (ol.gillet@gmail.com)

"""Class and functions to read and write numpy array from and to audio files."""

import copy
import logging
import numpy
import struct
import sys

sys.path.append('.')


# Constant used when converting between unsigned char and float. A different
# value is used in both directions to avoid clipping.
_UNSIGNED_CHAR_TO_FLOAT_SCALE = 128.0
_FLOAT_TO_UNSIGNED_CHAR_SCALE = 127.0

_DATA_CHUNK_HEADER_SIZE = 8
_FMT_CHUNK_DATA_SIZE = 16
_FMT_CHUNK_HEADER_SIZE = 8
_RIFF_FORMAT_DESCRIPTOR_SIZE = 4


class AudioIoException(Exception):
  """An error indicating a failure in audio file reading/writing."""

  def __init__(self, message):
    """Initializes an AudioIoException object."""
    Exception.__init__(self, 'Audio IO error: %s' % message)


def _ReadBytesOrFail(file_object, num_bytes, error_message):
  """Read a given number of bytes from the file or raise an error.

  Args:
    file_object: file object.
    num_bytes: int. number of bytes to read.
    error_message: string. text message of the exception thrown when the number
        of bytes could not be read (for example, identifying which section the
        caller attempted to read.)

  Returns:
    String with the bytes read from the file.

  Raises:
    AudioIoException:
      - The required number of bytes could not be read from the file.
  """
  read = file_object.read(num_bytes)
  if len(read) < num_bytes:
    raise AudioIoException(error_message)
  return read


def _GoToIffChunk(file_object, iff_chunk_id):
  """Jump to a named chunk in a (R)IFF file.

  Args:
     file_object: file object.
     iff_chunk_id: 4 chars ID of the chunk.

  Returns:
    length of the chunk in bytes. -1 if the chunk has not been found.
    If the chunk is found, file_object is positioned at the beginning of the
    chunk. Otherwise, it is positioned at the end of the file.
  """
  while True:
    chunk_id = file_object.read(4)
    if len(chunk_id) < 4:
      return -1
    chunk_size = file_object.read(4)
    if len(chunk_size) < 4:
      return -1
    chunk_size = struct.unpack('<L', chunk_size)
    if iff_chunk_id == chunk_id:
      return chunk_size[0]
    else:
      file_object.seek(chunk_size, 1)


def ReadWavFile(file_name, scale=True):
  """Read a .wav file into a numpy array.

  Note: the FFmpeg based AudioDecoder is more generic, use this only as a
        low-level alternative to AudioDecoder.

  Args:
    file_name: string. name of the local file to load.
    scale: boolean. if True, returns float data in the [-1, 1] range instead
        of integers.

  Returns:
    2-dimensional numpy array of size (num_samples, num_channels)

  Raises:
    AudioIoException:
      - The file header is corrupted.
      - The file uses an unsupported sampling rate, bitdepth or codec.
  """
  f = file(file_name, 'r')
  header = f.read(12)
  if len(header) < 12 or header[:4] != 'RIFF' or header[8:] != 'WAVE':
    raise AudioIoException('Corrupted header')

  format_header_size = _GoToIffChunk(f, 'fmt ')
  if format_header_size < 0 or format_header_size != 16:
    raise AudioIoException('Invalid header size')

  format_header = _ReadBytesOrFail(f, 16, 'Corrupted header')

  compression, num_channels, sample_rate, _, _, bitdepth = struct.unpack(
      '<HHLLHH', format_header)

  if compression != 1:
    raise AudioIoException('Unknown .wav codec: %d' % compression)

  if not num_channels:
    raise AudioIoException('Wrong number of channels')

  if sample_rate < 1000 or sample_rate > 96000:
    raise AudioIoException('Invalid sample rate')

  if bitdepth != 8 and bitdepth != 16:
    raise AudioIoException('Unsupported bit depth')

  sample_data_size = _GoToIffChunk(f, 'data')
  num_samples = sample_data_size / (bitdepth / 8)
  # Make sure we are reading a number of samples which is a multiple of the
  # number of channels. Some corrupted stereo .wav files may contain 5 samples!
  num_samples -= num_samples % num_channels
  if bitdepth == 8:
    samples = numpy.fromfile(f, dtype=numpy.ubyte, count=num_samples)
    if scale:
      samples = (samples / _UNSIGNED_CHAR_TO_FLOAT_SCALE) - 1.0
  else:
    bytes = bitdepth / 8
    samples = numpy.fromfile(f, dtype='<i%d' % bytes, count=num_samples)
    if scale:
      # Semantics of x = x / y and x /= y are different when x and y are
      # numpy arrays of a different type. x /= y casts to y's type, while
      # x = x / y casts to x's type.
      # pylint: disable-msg=C6407
      samples = samples / float(1 << (bitdepth - 1))
  return (samples.reshape(-1, num_channels), sample_rate)


def Quantize(signal, bitdepth, normalize=True):
  """Convert an array of float to an array of integers.

  Args:
    signal: numpy array. source signal.
    bitdepth: int. size of the integer in bits.
    normalize: boolean. whether samples should be scaled to use all the
        available dynamic range.

  Returns:
    array of integers.
  """
  norm = numpy.abs(signal).max()

  # Normalization or clipping.
  if normalize and norm > 0:
    scaled_signal = signal / norm
  else:
    scaled_signal = copy.copy(signal)
    if norm > 1.0:
      logging.warning('Some samples will be clipped.')
      # Clip samples above 1 and below -1.
      scaled_signal[scaled_signal < -1] = -1
      scaled_signal[scaled_signal > 1] = 1

  if bitdepth == 8:
    scaled_signal = (scaled_signal + 1.0) * _FLOAT_TO_UNSIGNED_CHAR_SCALE
    scaled_signal = numpy.array(scaled_signal, dtype=numpy.uint8)
  else:
    scale = (1 << (bitdepth - 1)) - 1
    # pylint: disable-msg=C6407
    scaled_signal = scaled_signal * scale
    scaled_signal = numpy.array(scaled_signal, dtype='i%d' % (bitdepth / 8))

  return scaled_signal


def WriteWavFile(signal, sample_rate, file_name, bitdepth=16, normalize=True):
  """Write a .wav file from a numpy array.

  Args:
    signal: 2-dimensional numpy array, of size (num_samples, num_channels).
    sample_rate: int. sample rate of the signal in Hz.
    file_name: string. name of the destination file.
    bitdepth: int. bitdepth in bits (default 16).
    normalize: boolean. if set to True, scale the data to the [-1, 1] range
        before writing.
  """
  if signal.dtype == numpy.uint8 or signal.dtype == numpy.int16:
    bitdepth = signal.dtype.itemsize * 8
    scaled_signal = signal
  else:
    scaled_signal = Quantize(signal, bitdepth, normalize=normalize)

  if scaled_signal.ndim == 1:
    num_channels = 1
  else:
    num_channels = scaled_signal.shape[1]

  # Compute the total size of the output .wav file, minus the size of the
  # first two fields of the RIFF header.

  # RIFF Format.
  total_size = _RIFF_FORMAT_DESCRIPTOR_SIZE
  # 'fmt ' chunk.
  total_size += _FMT_CHUNK_HEADER_SIZE + _FMT_CHUNK_DATA_SIZE
  # 'data' chunk.
  total_size += _DATA_CHUNK_HEADER_SIZE + scaled_signal.nbytes

  f = file(file_name, 'w')
  try:
    f.write('RIFF')
    f.write(struct.pack('<L', total_size))
    f.write('WAVEfmt ')
    bitrate = sample_rate * num_channels * (bitdepth / 8)
    bits_per_sample = num_channels * (bitdepth / 8)
    f.write(struct.pack('<LHHLLHH', 16, 1, num_channels, sample_rate, bitrate,
                        bits_per_sample, bitdepth))
    f.write('data')
    f.write(struct.pack('<L', scaled_signal.nbytes))
    scaled_signal.tofile(f)
  finally:
    f.close()
