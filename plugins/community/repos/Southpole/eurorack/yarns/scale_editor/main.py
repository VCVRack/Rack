#!/usr/bin/env python
#
# Copyright 2007 Google Inc.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

import os

import webapp2

from google.appengine.ext import webapp
from google.appengine.ext.webapp import util
from google.appengine.ext.webapp import template

from music.scala import scala

TEMPLATE_PATH = os.path.join(os.path.dirname(__file__), 'templates')



class SyxHandler(webapp.RequestHandler):
  def get(self):
    header = '\xf0\x7f\x7f\x08\x09\x03\x7f\x7f'
    data = self.request.get('data')
    body = ''.join([chr(int(data[x:x+2], 16)) for x in range(0, len(data), 2)])
    footer = '\xf7'
    self.response.headers['Content-Type'] = 'application/octet-stream'
    self.response.headers['Content-disposition'] = \
        'attachment; filename=scale.syx'
    self.response.out.write(header + body + footer)



class MainHandler(webapp.RequestHandler):
  def post(self):
    return self.get()
  
  def get(self):
    if self.request.get('scl'):
      source = self.request.get('scl').decode('iso-8859-1').encode('utf8')
      hide_if_error = True
    else:
      source = self.request.get('source')
      hide_if_error = False
    
    semitones = []
    try:
      error_message = ''
      notes = scala.parse(source)
      mapping = scala.assign_to_octave(notes)
      names = ['C', 'C#', 'D', 'D#', 'E', 'F', 'F#', 'G', 'G#', 'A', 'A#', 'B']
      mappings = []
      for i, (shift, note) in enumerate(zip(mapping, names)):
        mappings.append(
          '%s = %.2f cents' % (note, shift + i * 100.0))
      mapping_14bit = [int(round(8192 + 8192 * x / 100.0)) for x in mapping]
      mapping_hex = ['%02x%02x' % (m >> 7, m & 0x7f) for m in mapping_14bit]
      link = ''.join(mapping_hex)
      
    except scala.ParseError as p:
      error_message = p.message
      notes = []
      mappings = []
      link = ''
    
    if not source:
      error_message = ''
    
    self.response.headers['Content-Type'] = 'text/html'
    template_path = os.path.join(TEMPLATE_PATH, 'index.html')
    template_data = {}
    template_data['error_message'] = error_message
    template_data['notes'] = notes
    template_data['mappings'] = mappings
    template_data['link'] = link
    template_data['source'] = '' if hide_if_error and error_message else source
    self.response.out.write(template.render(template_path, template_data))

application = webapp.WSGIApplication([('/', MainHandler), ('/syx', SyxHandler)])
