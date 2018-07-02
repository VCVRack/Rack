#!/usr/bin/python2.5
#
# Copyright 2012 Olivier Gillet.
#
# Author: Olivier Gillet (olivier@mutable-instruments.net)
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
# Lookup table definitions.

import numpy

lookup_tables = []
lookup_tables_32 = []

"""----------------------------------------------------------------------------
Timer oscillator pitch table.
----------------------------------------------------------------------------"""

notes = numpy.arange(0, 12 * 128 + 16, 16) / 128.0 + 24
frequencies = 2 ** ((notes - 69) / 12) * 440.0
values = 32000000.0 / (2 * 8 * frequencies)
lookup_tables.append(
    ('timer_count', numpy.round(values))
)

"""----------------------------------------------------------------------------
Digital oscillator pitch table.
----------------------------------------------------------------------------"""

sample_rate = 32000000 / 666.0
a4_midi = 69
a4_pitch = 440.0
excursion = 65536.0
notes = numpy.arange(116 * 128.0, 128 * 128.0 + 16, 16)
pitches = 2 * a4_pitch * 2 ** ((notes - a4_midi * 128) / (128 * 12))
increments = excursion / sample_rate * pitches

lookup_tables.append(
    ('oscillator_increments', increments.astype(int))
)

"""----------------------------------------------------------------------------
Bitcrusher pitch table.
----------------------------------------------------------------------------"""
ratios = [1] * 128
ratios = ratios + list(numpy.linspace(1.0, 64.0, 128))
increments = 65536 / numpy.array(ratios)
lookup_tables.append(
    ('bitcrusher_increments', increments.astype(int))
)
