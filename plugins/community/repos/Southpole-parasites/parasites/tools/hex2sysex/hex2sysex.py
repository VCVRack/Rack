#!/usr/bin/python2.5
#
# Copyright 2009 Olivier Gillet.
#
# Author: Olivier Gillet (ol.gillet@gmail.com)
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.
#
# -----------------------------------------------------------------------------
#
# Hex2SysEx utility

"""Hex2SysEx utility.

usage:
  python hex2sysex.py \
    [--page_size 128] \
    [--delay 250] \
    [--syx] \
    [--output_file path_to/firmware.mid] \
    path_to/firmware.hex
"""

import logging
import optparse
import os
import struct
import sys

# Allows the code to be run from the project root directory
sys.path.append('.')

from tools.midi import midifile
from tools.hexfile import hexfile


def CreateMidifile(
    input_file_name,
    data,
    output_file,
    options):
  size = len(data)
  page_size = options.page_size
  delay = options.delay
  _, input_file_name = os.path.split(input_file_name)
  comments = [
      'Warning: contains OS data!',
      'Created from %(input_file_name)s' % locals(),
      'Size: %(size)d' % locals(),
      'Page size: %(page_size)d' % locals(),
      'Delay: %(delay)d ms' % locals()]
  m = midifile.Writer()
  if options.write_comments:
    for comment in comments:
      m.AddTrack().AddEvent(0, midifile.TextEvent(comment))
  t = m.AddTrack()
  t.AddEvent(0, midifile.TempoEvent(120.0))
  page_size *= 2  # Converts from words to bytes
  # The first SysEx block must not start at 0! Sequencers like Logic play the
  # first SysEx block everytime stop/play is pressed.
  time = 1
  syx_data = []
  for i in xrange(0, size, page_size):
    block = ''.join(map(chr, data[i:i+page_size]))
    padding = page_size - len(block)
    block += '\x00' * padding
    mfr_id = options.manufacturer_id if not \
        options.force_obsolete_manufacturer_id else '\x00\x20\x77'
    event = midifile.SysExEvent(
        mfr_id,
        struct.pack('>h', options.device_id),
        options.update_command + midifile.Nibblize(block))
    t.AddEvent(time, event)
    syx_data.append(event.raw_message)
    # ms -> s -> beats -> ticks
    time += int(delay / 1000.0 / 0.5 * 96)
  event = midifile.SysExEvent(
      mfr_id,
      struct.pack('>h', options.device_id),
      options.reset_command)
  t.AddEvent(time, event)
  syx_data.append(event.raw_message)
  
  f = file(output_file, 'wb')
  if options.syx:
    f.write(''.join(syx_data))
  else:
    m.Write(f, format=1)
  f.close()


if __name__ == '__main__':
  parser = optparse.OptionParser()
  parser.add_option(
      '-p',
      '--page_size',
      dest='page_size',
      type='int',
      default=128,
      help='Flash page size in words')
  parser.add_option(
      '-d',
      '--delay',
      dest='delay',
      type='int',
      default=250,
      help='Delay between pages in milliseconds')
  parser.add_option(
      '-o',
      '--output_file',
      dest='output_file',
      default=None,
      help='Write output file to FILE',
      metavar='FILE')
  parser.add_option(
      '-m',
      '--manufacturer_id',
      dest='manufacturer_id',
      default='\x00\x21\x02',
      help='Manufacturer ID to use in SysEx message')
  parser.add_option(
      '-b',
      '--obsolete_manufacturer_id',
      dest='force_obsolete_manufacturer_id',
      default=False,
      action='store_true',
      help='Force the use of the manufacturer ID used in early products')
  parser.add_option(
      '-v',
      '--device_id',
      dest='device_id',
      type='int',
      default=2,
      help='Device ID to use in SysEx message')
  parser.add_option(
      '-u',
      '--update_command',
      dest='update_command',
      default='\x7e\x00',
      help='OS update SysEx command')
  parser.add_option(
      '-r',
      '--reset_command',
      dest='reset_command',
      default='\x7f\x00',
      help='Post-OS update reset SysEx command')
  parser.add_option(
      '-s',
      '--syx',
      dest='syx',
      action='store_true',
      default=False,
      help='Produces a .syx file instead of a MIDI file')
  parser.add_option(
      '-c',
      '--comments',
      dest='write_comments',
      action='store_true',
      default=False,
      help='Store additional technical gibberish')

  options, args = parser.parse_args()
  if len(args) != 1:
    logging.fatal('Specify one, and only one firmware .hex file!')
    sys.exit(1)

  if args[0].endswith('.bin'):
    data = map(ord, file(args[0], 'rb').read())
  else:
    data = hexfile.LoadHexFile(file(args[0]))
    if not data:
      logging.fatal('Error while loading .hex file')
      sys.exit(2)

  output_file = options.output_file
  if not output_file:
    if '.hex' in args[0]:
      output_file = args[0].replace('.hex', '.mid')
    else:
      output_file = args[0] + '.mid'

  CreateMidifile(
      args[0],
      data,
      output_file,
      options)
