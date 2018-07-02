#!/usr/bin/python2.5
#
# Copyright 2014 Olivier Gillet.
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
# Lookup table definitions.

import numpy
import optparse
import os
import pylab
import scipy.signal
import sys



HEADER = """// Copyright 2015 Olivier Gillet.
//
// Author: Olivier Gillet (ol.gillet@gmail.com)
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
// 
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
// 
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
// 
// See http://creativecommons.org/licenses/MIT/ for more information.
//
// -----------------------------------------------------------------------------
//
// Sample rate conversion filters.
"""



def plot_response(
    ir,
    ratio,
    name,
    n=8192,
    sr=96000.0,
    critical_frequency=16000.0):
  gain = 20 * numpy.log10(numpy.abs(numpy.fft.rfft(ir, n)) + 1e-30)
  f = numpy.arange(n / 2 + 1) / float(n) * sr * ratio
  bin_index = critical_frequency / ratio / sr * n
  critical_gain = gain[int(round(bin_index))]
  pylab.figure(figsize=(8, 12))
  pylab.subplot(211)
  pylab.plot(f, gain)
  pylab.xlim([0.0, sr])
  pylab.ylim([-80.0, 3.0])
  pylab.xlabel('Frequency (Hz)')
  pylab.ylabel('Gain (dB)')
  caption = 'Resampling filter, N=%d, Ratio=%d. Gain at %d Hz = %.2fdB'
  pylab.title(caption % (len(ir), ratio, critical_frequency, critical_gain))

  pylab.subplot(212)
  pylab.plot(f, gain)
  pylab.xlim([0.0, 20000.0])
  pylab.ylim([-3.0, 1.0])
  pylab.xlabel('Frequency (Hz)')
  pylab.ylabel('Gain (dB)')

  pylab.savefig(name + '.pdf')
  pylab.close()
  


def make_header_guard_name(name):
  name = name.upper()
  name = name.replace('/', '_')
  name = name.replace('.', '_')
  return name + '_' if name else ''
  


def generate_filters(options):
  output_file = options.output_file
  out = file(output_file, 'w') if output_file else sys.stderr
  guard = make_header_guard_name(output_file)
  namespace = output_file.split(os.sep)[0] if output_file else ''
  out.write(HEADER)
  
  if guard:
    out.write('\n#ifndef %s\n' % guard)
    out.write('#define %s\n' % guard)
  
  if namespace:
    out.write('\nnamespace %s {\n' % namespace)
  
  configurations = options.configurations.split(';')
  for configuration in configurations:
    tokens = configuration.split(',')
    ratio, length = map(int, tokens[:2])
    transition = float(tokens[2])
    ir = scipy.signal.remez(
        length,
        [0, transition / ratio, 0.5 / ratio, 0.5], [1, 0])
    ir /= sum(ir)
    cmd = 'scipy.signal.remez(%d, [0, %f / %d, 0.5 / %d, 0.5], [1, 0])' % \
        (length, transition, ratio, ratio)
    
    if options.plot_responses:
      name = 'src_filter_%d_%d' % (ratio, length)
      plot_response(ir, ratio, name)
    
    for up in [True, False]:
      scale = ratio if up else 1.0
      scaled = ir * scale
      out.write('\n// Generated with:\n')
      out.write('// %d * %s\n' % (scale, cmd))
      out.write('template<>\n')
      out.write('struct SRC_FIR<SRC_%s, %d, %d> {\n' % \
          ('UP' if up else 'DOWN', ratio, length))
      out.write('  template<int32_t i> inline float Read() const {\n')
      out.write('    const float h[] = {\n')
      for i in xrange(0, length / 2, 4):
        out.write('      ');
        out.write(', '.join(
            '% 16.9e' % scaled[j]  for j in xrange(i, min(length / 2, i + 4))))
        out.write(',\n');
      out.write('    };\n')
      out.write('    return h[i];\n')
      out.write('  }\n')
      out.write('};\n')
      
  if namespace:
    out.write('\n}  // namespace %s\n\n' % namespace)

  if guard:
    out.write('#endif  // %s\n' % guard)



if __name__ == '__main__':
  parser = optparse.OptionParser()
  parser.add_option(
      '-p',
      '--plot',
      dest='plot_responses',
      default=False,
      action='store_true',
      help='Plot responses')
  parser.add_option(
      '-c',
      '--configurations',
      dest='configurations',
      default='6,48,0.06;4,48,0.105;3,36,0.05',
      help='Output file name')
  parser.add_option(
      '-o',
      '--output',
      dest='output_file',
      default='warps/dsp/sample_rate_conversion_filters.h',
      help='Output file name')

  options, _ = parser.parse_args()
  generate_filters(options)
