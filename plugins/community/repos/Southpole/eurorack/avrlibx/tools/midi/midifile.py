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
# Midifile Writer and Reader.

"""Midifile writer.
"""

import bisect
import math
import struct


def PackInteger(value, size=4):
  """Packs a python integer into a n-byte big endian byte sequence."""
  return struct.pack('>%s' % {1: 'B', 2: 'H', 4: 'L'}[size], value)


def UnpackInteger(value, size=4):
  """Packs a python integer into a n-byte big endian byte sequence."""
  return struct.unpack('>%s' % {1: 'B', 2: 'H', 4: 'L'}[size], value)[0]


def PackVariableLengthInteger(value):
  """Packs a python integer into a variable length byte sequence."""
  if value == 0:
    return '\x00'
  s = value
  output = []
  while value:
    to_write = value & 0x7f
    value = value >> 7
    output.insert(0, to_write)
  for i in xrange(len(output) - 1):
    output[i] |= 0x80
  output = ''.join(map(chr, output))
  return output

"""Classes representing a MIDI event, with a Serialize method which encodes
them into a string."""

class Event(object):
  def __init__(self):
    pass
    
  def Serialize(self, running_status):
    raise NotImplementedError


class MetaEvent(Event):
  def __init__(self, id, data):
    assert len(data) < 256
    self.id = id
    self.data = data
    
  def Serialize(self, running_status):
    return ''.join([
        '\xff',
        PackInteger(self.id, size=1),
        PackInteger(len(self.data), size=1),
        self.data]), None


class TextEvent(MetaEvent):
  def __init__(self, text):
    self.text = text
    super(TextEvent, self).__init__(0x01, text)


class CopyrightInfoEvent(MetaEvent):
  def __init__(self, text):
    self.text = text
    super(CopyrightInfoEvent, self).__init__(0x02, text)


class TrackNameEvent(MetaEvent):
  def __init__(self, text):
    self.text = text
    super(TrackNameEvent, self).__init__(0x03, text)


class TrackInstrumentNameEvent(MetaEvent):
  def __init__(self, text):
    self.text = text
    super(TrackInstrumentNameEvent, self).__init__(0x04, text)


class LyricEvent(MetaEvent):
  def __init__(self, text):
    self.text = text
    super(LyricEvent, self).__init__(0x05, text)


class MarkerEvent(MetaEvent):
  def __init__(self, text):
    self.text = text
    super(MarkerEvent, self).__init__(0x06, text)


class CuePointEvent(MetaEvent):
  def __init__(self, text):
    self.text = text
    super(CuePointEvent, self).__init__(0x07, text)


class EndOfTrackEvent(MetaEvent):
  def __init__(self):
    super(EndOfTrackEvent, self).__init__(0x2f, '')


class TempoEvent(MetaEvent):
  def __init__(self, bpm):
    self.bpm = bpm
    value = 60000000.0 / bpm
    data = PackInteger(int(value), size=4)[1:]
    super(TempoEvent, self).__init__(0x51, data)


class SMPTEOffsetEvent(MetaEvent):
  def __init__(self, h, m, s, f, sf):
    self.smpte_offset = (h, m, s, f, sf)
    data = ''.join(map(chr, [h, m, s, f, sf]))
    super(SMPTEOffsetEvent, self).__init__(0x54, data)


class TimeSignatureEvent(MetaEvent):
  def __init__(self, numerator, denominator):
    self.numerator = numerator
    self.denominator = denominator
    data = ''.join([
        PackInteger(numerator, size=1),
        PackInteger(int(math.log(denominator) / math.log(2)), size=1),
        '\x16\x08'])
    super(TimeSignatureEvent, self).__init__(0x58, data)


class KeyEvent(MetaEvent):
  def __init__(self, sharp_flats, major_minor):
    self.sharp_flats = sharp_flats
    self.major_minor = major_minor
    data = ''.join([
        PackInteger(sharp_flats, size=1),
        PackInteger(major_minor, size=1)])
    super(KeyEvent, self).__init__(0x59, data)


class BlobEvent(MetaEvent):
  def __init__(self, blob):
    self.data = blob
    super(BlobEvent, self).__init__(0x7f, blob)


"""Classic channel-oriented messages."""

class ChannelEvent(Event):
  def __init__(self, mask, channel, data):
    self.channel = channel
    self._status = mask | (channel - 1)
    self._data = PackInteger(self._status, size=1) + data
    
  def Serialize(self, running_status):
    if self._status == running_status:
      return self._data[1:], self._status
    else:
      return self._data, self._status


class NoteOffEvent(ChannelEvent):
  def __init__(self, channel, note, velocity):
    data = PackInteger(note, size=1) + PackInteger(velocity, size=1)
    super(NoteOffEvent, self).__init__(0x80, channel, data)
    self.note = note
    self.velocity = velocity


class NoteOnEvent(ChannelEvent):
  def __init__(self, channel, note, velocity):
    data = PackInteger(note, size=1) + PackInteger(velocity, size=1)
    super(NoteOnEvent, self).__init__(0x90, channel, data)
    self.note = note
    self.velocity = velocity


class KeyAftertouchEvent(ChannelEvent):
  def __init__(self, channel, note, aftertouch):
    data = PackInteger(note, size=1) + PackInteger(aftertouch, size=1)
    super(KeyAftertouchEvent, self).__init__(0xa0, channel, data)
    self.note = note
    self.aftertouch = aftertouch


class ControlChangeEvent(ChannelEvent):
  def __init__(self, channel, controller, value):
    data = PackInteger(controller, size=1) + PackInteger(value, size=1)
    super(ControlChangeEvent, self).__init__(0xb0, channel, data)
    self.controller = controller
    self.value = value


class ProgramChangeEvent(ChannelEvent):
  def __init__(self, channel, program_number):
    data = PackInteger(program_number, size=1)
    super(ProgramChangeEvent, self).__init__(0xc0, channel, data)
    self.program_number = program_number


class ChannelAftertouchEvent(ChannelEvent):
  def __init__(self, channel, aftertouch):
    data = PackInteger(aftertouch, size=1)
    super(ChannelAftertouchEvent, self).__init__(0xd0, channel, data)
    self.aftertouch = aftertouch


class PitchBendEvent(ChannelEvent):
  def __init__(self, channel, pitch_bend_14_bits):
    data = PackInteger(pitch_bend_14_bits >> 7, size=1) + \
        PackInteger(pitch_bend_14_bits & 0x7f, size=1)
    super(PitchBendEvent, self).__init__(0xe0, channel, data)
    self.pitch_bend = pitch_bend_14_bits


class SystemEvent(Event):
  def __init__(self, id):
    self._id = id
    
  def Serialize(self, running_status):
    return PackInteger(self._id, size=1), running_status


class ClockEvent(SystemEvent):
  def __init__(self):
    super(ClockEvent, self).__init__(0xf8)


class StartEvent(SystemEvent):
  def __init__(self):
    super(StartEvent, self).__init__(0xfa)


class ContinueEvent(SystemEvent):
  def __init__(self):
    super(ContinueEvent, self).__init__(0xfb)


class StopEvent(SystemEvent):
  def __init__(self):
    super(StopEvent, self).__init__(0xfc)


# TODO(pichenettes): also support pauses within a block transmission (F7)
class SysExEvent(Event):
  def __init__(self, manufacturer_id, device_id, data):
    self.data = data
    self.message = ''.join([
        manufacturer_id,
        device_id,
        data,
        '\xf7'])
    self.raw_message = '\xf0' + self.message
    assert all(ord(x) < 128 for x in self.message[:-1])
    self.message = ''.join([
        '\xf0',
        PackVariableLengthInteger(len(self.message)),
        self.message])

  def Serialize(self, running_status):
    return self.message, None


def Nibblize(data, add_checksum=True):
  """Converts a byte string into a nibble string. Also adds checksum"""
  output = []
  if add_checksum:
    tail = [chr(sum(ord(char) for char in data) % 256)]
  else:
    tail = []
  for char in map(ord, list(data) + tail):
    output.append(chr(char >> 4))
    output.append(chr(char & 0x0f))
  return ''.join(output)


class Track(object):
  def __init__(self):
    self._events = []
    
  def AddEvent(self, time, event):
    self._events.append((time, event))

  def Sort(self):
    self._events = sorted(self._events)

  def Serialize(self):
    self.Sort()
    last_time, last_event = self._events[-1]
    if type(last_event) != EndOfTrackEvent:
      self._events.append((last_time + 1, EndOfTrackEvent()))
    data = []
    current_time = 0
    running_status = None
    for time, event in self._events:
      delta = time - current_time
      data.append(PackVariableLengthInteger(delta))
      event_data, running_status = event.Serialize(running_status)
      data.append(event_data)
      current_time = time
    return ''.join(data)
    
  def Write(self, file_object):
    file_object.write('MTrk')
    track_data = self.Serialize()
    file_object.write(PackInteger(len(track_data)))
    file_object.write(track_data)
    
  @property
  def events(self):
    return self._events

    
class Writer(object):
  def __init__(self, ppq=96):
    self._tracks = []
    self._ppq = ppq
  
  def AddTrack(self):
    new_track = Track()
    self._tracks.append(new_track)
    return new_track
    
  def _MergeTracks(self):
    new_track = Track()
    for track in self._tracks:
      for time_event in track.events:
        new_track.AddEvent(*time_event)
    new_track.Sort()
    return new_track
    
  def Write(self, file_object, format=0):
    tracks = self._tracks
    if format == 0:
      tracks = [self._MergeTracks()]

    # File header.
    file_object.write('MThd')
    file_object.write(PackInteger(6))
    file_object.write(PackInteger(format, size=2))
    if format == 0:
      file_object.write(PackInteger(1, size=2))
    else:
      file_object.write(PackInteger(len(self._tracks), size=2))
    file_object.write(PackInteger(self._ppq, size=2))
    
    # Tracks.
    for track in tracks:
      track.Write(file_object)


class Reader(object):
  def __init__(self):
    self.tracks = []
    self.format = 0
    self.ppq = 96
    self._previous_status = 0
    
  def Read(self, f):
    assert f.read(4) == 'MThd'
    assert struct.unpack('>i', f.read(4))[0] == 6
    self.format = struct.unpack('>h', f.read(2))[0]
    assert self.format <= 2
    num_tracks = struct.unpack('>h', f.read(2))[0]
    self.ppq = struct.unpack('>h', f.read(2))[0]
    self._tempo_map = []
    
    for i in xrange(num_tracks):
      self.tracks.append(self._ReadTrack(f))
    self._CreateCumulativeTempoMap()
    
      
  def _ReadTrack(self, f):
    assert f.read(4) == 'MTrk'
    size = struct.unpack('>i', f.read(4))[0]
    t = 0
    events = []
    while size > 0:
      delta_t, e, event_size = self._ReadEvent(f)
      t += delta_t
      if e:
        events.append((t, e))
        if type(e) == TempoEvent:
          self._tempo_map.append((t, e.bpm))
      size -= event_size
    return events
    
  def _CreateCumulativeTempoMap(self):
    t = 0.0
    current_tempo = 120.0
    previous_beat = 0
    cumulative_tempo_map = [(0, 0.0, current_tempo)]
    for beat, tempo in sorted(self._tempo_map):
      beats = float(beat - previous_beat) / self.ppq
      t += beats * 60.0 / current_tempo
      cumulative_tempo_map.append((beat, t, tempo))
      current_tempo = tempo
      previous_beat = beat
    self._tempo_map = cumulative_tempo_map
    
  def AbsoluteTime(self, t):
    index = bisect.bisect_left(self._tempo_map, (t, 0, 0))
    index = max(index - 1, 0)
    start_beat, start_seconds, tempo = self._tempo_map[index]
    return start_seconds + float(t - start_beat) / self.ppq * 60.0 / tempo
    
  def _ReadVariableLengthInteger(self, f):
    v = 0
    size = 0
    while True:
      v <<= 7
      byte = UnpackInteger(f.read(1), size=1)
      size += 1
      v |= (byte & 0x7f)
      if not (byte & 0x80):
        break
    return v, size
    
  def _ReadEvent(self, f):
    delta_t, size = self._ReadVariableLengthInteger(f)
    event_byte = ord(f.read(1))
    size += 1
    if event_byte < 0x80:
      if self._previous_status:
        f.seek(f.tell() - 1)
        size -= 1
        event_byte = self._previous_status
      else:
        return delta_t, None, size
    
    event_type = event_byte & 0xf0
    channel = event_byte & 0xf
    channel += 1
    if event_type == 0x80:
      self._previous_status = event_type
      note = ord(f.read(1))
      velo = ord(f.read(1))
      event = NoteOffEvent(channel, note, velo)
      size += 2
    elif event_type == 0x90:
      self._previous_status = event_type
      event = NoteOnEvent(channel, ord(f.read(1)), ord(f.read(1)))
      size += 2
    elif event_type == 0xa0:
      self._previous_status = event_type
      event = KeyAftertouchEvent(channel, ord(f.read(1)), ord(f.read(1)))
      size += 2
    elif event_type == 0xb0:
      self._previous_status = event_type
      event = ControlChangeEvent(channel, ord(f.read(1)), ord(f.read(1)))
      size += 2
    elif event_type == 0xc0:
      self._previous_status = event_type
      event = ProgramChangeEvent(channel, ord(f.read(1)))
      size += 1
    elif event_type == 0xd0:
      self._previous_status = event_type
      event = ChannelAftertouchEvent(channel, ord(f.read(1)))
      size += 1
    elif event_type == 0xe0:
      self._previous_status = event_type
      event = PitchBendEvent(channel, (ord(f.read(1)) << 7) | ord(f.read(1)))
      size += 2
    elif event_byte == 0xff:
      event_type = ord(f.read(1))
      size += 1
      event_size, event_size_size = self._ReadVariableLengthInteger(f)
      size += event_size_size
      bytes = f.read(event_size)
      size += event_size
      if event_type == 0x01:
        event = TextEvent(bytes)
      elif event_type == 0x02:
        event = CopyrightInfoEvent(bytes)
      elif event_type == 0x03:
        event = TrackNameEvent(bytes)
      elif event_type == 0x04:
        event = TrackInstrumentNameEvent(bytes)
      elif event_type == 0x05:
        event = LyricEvent(bytes)
      elif event_type == 0x06:
        event = MarkerEvent(bytes)
      elif event_type == 0x07:
        event = CuePointEvent(bytes)
      elif event_type == 0x20:
        current_channel = ord(bytes[0])
        event = None
      elif event_type == 0x2f:
        event = EndOfTrackEvent()
      elif event_type == 0x51:
        value = UnpackInteger('\x00' + bytes, size=4)
        event = TempoEvent(60000000.0 / value)
      elif event_type == 0x54:
        event = SMPTEOffsetEvent(*map(ord, bytes))
      elif event_type == 0x58:
        event = TimeSignatureEvent(ord(bytes[0]), 2 ** ord(bytes[1]))
      elif event_type == 0x59:
        event = KeyEvent(ord(bytes[0]), ord(bytes[1]))
      elif event_type == 0x7f:
        event = BlobEvent(bytes)
    elif event_byte == 0xf0:
      event_size, event_size_size = self._ReadVariableLengthInteger(f)
      size += event_size_size
      bytes = f.read(event_size)
      size += event_size
      event = SysExEvent(bytes[0:3], bytes[3:5], bytes[5:-1])
    else:
      print event_byte, '!!'
      event = None
    return delta_t, event, size
    

if __name__ == '__main__':
  m = Writer()
  t = m.AddTrack()
  t.AddEvent(0, TempoEvent(120.0))
  
  t = m.AddTrack()
  t.AddEvent(1, SysExEvent(
      '\x00\x20\x77',
      '\x00\x01',
      '\x7f\x7f' + Nibblize('\xff\x00\xcc')))
  
  f = file('output.mid', 'wb')
  m.Write(f, format=0)
  f.close()
