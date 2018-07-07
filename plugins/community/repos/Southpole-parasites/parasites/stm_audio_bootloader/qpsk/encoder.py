#!/usr/bin/python2.5
#
# Copyright 2013 Olivier Gillet.
# 
# Author: Olivier Gillet (ol.gillet@gmail.com)
# 
# Permission is hereby granted, free of charge, to any person obtaining a copy
# of this software and associated documentation files (the "Software"), to deal
# in the Software without restriction, including without limitation the rights
# to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
# copies of the Software, and to permit persons to whom the Software is
# furnished to do so, subject to the following conditions:
# 
# The above copyright notice and this permission notice shall be included in
# all copies or substantial portions of the Software.
# 
# THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
# IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
# FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
# AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
# LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
# OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
# THE SOFTWARE.
# 
# See http://creativecommons.org/licenses/MIT/ for more information.
#
# -----------------------------------------------------------------------------
#
# Qpsk encoder for converting firmware .bin files into .

import numpy
import optparse
import zlib

from stm_audio_bootloader import audio_stream_writer


class QpskEncoder(object):
  
  def __init__(
      self,
      sample_rate=48000,
      carrier_frequency=2000,
      bit_rate=8000,
      packet_size=256):
    period = sample_rate / carrier_frequency
    symbol_time = sample_rate / (bit_rate / 2)
    
    assert (symbol_time % period == 0) or (period % symbol_time == 0)
    assert (sample_rate % carrier_frequency) == 0
    assert (sample_rate % bit_rate) == 0
    
    self._sr = sample_rate
    self._br = bit_rate
    self._carrier_frequency = carrier_frequency
    self._sample_index = 0
    self._packet_size = packet_size
    
  @staticmethod
  def _upsample(x, factor):
    return numpy.tile(x.reshape(len(x), 1), (1, factor)).ravel()
    
  def _encode_qpsk(self, symbol_stream):
    ratio = self._sr / self._br * 2
    symbol_stream = numpy.array(symbol_stream)
    bitstream_even = 2 * self._upsample(symbol_stream % 2, ratio) - 1
    bitstream_odd = 2 * self._upsample(symbol_stream / 2, ratio) - 1
    return bitstream_even / numpy.sqrt(2.0), bitstream_odd / numpy.sqrt(2.0)
  
  def _modulate(self, q_mod, i_mod):
    num_samples = len(q_mod)
    t = (numpy.arange(0.0, num_samples) + self._sample_index) / self._sr
    self._sample_index += num_samples
    phase = 2 * numpy.pi * self._carrier_frequency * t
    return (q_mod * numpy.sin(phase) + i_mod * numpy.cos(phase))

  def _encode(self, symbol_stream):
    return self._modulate(*self._encode_qpsk(symbol_stream))
    
  def _code_blank(self, duration):
    num_zeros = int(duration * self._br / 8) * 4
    symbol_stream = numpy.zeros((num_zeros, 1)).ravel().astype(int)
    return self._encode(symbol_stream)

  def _code_packet(self, data):
    assert len(data) <= self._packet_size
    if len(data) != self._packet_size:
      data = data + '\x00' * (self._packet_size - len(data))

    crc = zlib.crc32(data) & 0xffffffff

    data = map(ord, data)
    # 16x 0 for the PLL ; 8x 21 for the edge detector ; 8x 3030 for syncing
    preamble = [0] * 8 + [0x99] * 4 + [0xcc] * 4
    crc_bytes = [crc >> 24, (crc >> 16) & 0xff, (crc >> 8) & 0xff, crc & 0xff]
    bytes = preamble + data + crc_bytes
    
    symbol_stream = []
    for byte in bytes:
      symbol_stream.append((byte >> 6) & 0x3)
      symbol_stream.append((byte >> 4) & 0x3)
      symbol_stream.append((byte >> 2) & 0x3)
      symbol_stream.append((byte >> 0) & 0x3)
    
    return self._encode(symbol_stream)
  
  def code_intro(self):
    yield numpy.zeros((1.0 * self._sr, 1)).ravel()
    yield self._code_blank(1.0)
  
  def code_outro(self, duration=1.0):
    yield self._code_blank(duration)
  
  def code(self, data, page_size=1024, blank_duration=0.06):
    if len(data) % page_size != 0:
      tail = page_size - (len(data) % page_size)
      data += '\xff' * tail
    
    offset = 0
    remaining_bytes = len(data)
    num_packets_written = 0
    while remaining_bytes:
      size = min(remaining_bytes, self._packet_size)
      yield self._code_packet(data[offset:offset+size])
      num_packets_written += 1
      if num_packets_written == page_size / self._packet_size:
        yield self._code_blank(blank_duration)
        num_packets_written = 0
      remaining_bytes -= size
      offset += size

STM32F4_SECTOR_BASE_ADDRESS = [
  0x08000000,
  0x08004000,
  0x08008000,
  0x0800C000,
  0x08010000,
  0x08020000,
  0x08040000,
  0x08060000,
  0x08080000,
  0x080A0000,
  0x080C0000,
  0x080E0000
]

STM32F1_PAGE_SIZE = 1024
STM32F4_BLOCK_SIZE = 16384
STM32F4_APPLICATION_START = 0x08008000

def main():
  parser = optparse.OptionParser()
  parser.add_option(
      '-s',
      '--sample_rate',
      dest='sample_rate',
      type='int',
      default=48000,
      help='Sample rate in Hz')
  parser.add_option(
      '-c',
      '--carrier_frequency',
      dest='carrier_frequency',
      type='int',
      default=6000,
      help='Carrier frequency in Hz')
  parser.add_option(
      '-b',
      '--baud_rate',
      dest='baud_rate',
      type='int',
      default=12000,
      help='Baudrate in bps')
  parser.add_option(
      '-p',
      '--packet_size',
      dest='packet_size',
      type='int',
      default=256,
      help='Packet size in bytes')
  parser.add_option(
      '-o',
      '--output_file',
      dest='output_file',
      default=None,
      help='Write output file to FILE',
      metavar='FILE')
  parser.add_option(
      '-t',
      '--target',
      dest='target',
      default='stm32f1',
      help='Set page size and erase time for TARGET',
      metavar='TARGET')
      
  
  options, args = parser.parse_args()
  data = file(args[0], 'rb').read()
  if len(args) != 1:
    logging.fatal('Specify one, and only one firmware .bin file!')
    sys.exit(1)
  
  if options.target not in ['stm32f1', 'stm32f4']:
    logging.fatal('Unknown target: %s' % options.target)
    sys.exit(2)
  
  output_file = options.output_file
  if not output_file:
    if '.bin' in args[0]:
      output_file = args[0].replace('.bin', '.wav')
    else:
      output_file = args[0] + '.wav'

  encoder = QpskEncoder(
      options.sample_rate,
      options.carrier_frequency,
      options.baud_rate,
      options.packet_size)
  writer = audio_stream_writer.AudioStreamWriter(
      output_file,
      options.sample_rate,
      16,
      1)
  
  # INTRO
  for block in encoder.code_intro():
    writer.append(block)
  
  blank_duration = 1.0
  if options.target == 'stm32f1':
    for block in encoder.code(data, STM32F1_PAGE_SIZE, 0.06):
      if len(block):
        writer.append(block)
  elif options.target == 'stm32f4':
    for x in xrange(0, len(data), STM32F4_BLOCK_SIZE):
      address = STM32F4_APPLICATION_START + x
      block = data[x:x+STM32F4_BLOCK_SIZE]
      pause = 2.5 if address in STM32F4_SECTOR_BASE_ADDRESS else 0.2
      for block in encoder.code(block, STM32F4_BLOCK_SIZE, pause):
        if len(block):
          writer.append(block)
    blank_duration = 5.0
  for block in encoder.code_outro(blank_duration):
    writer.append(block)


if __name__ == '__main__':
  main()
