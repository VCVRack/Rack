#!/usr/bin/python2.5
#
# Copyright 2012 Olivier Gillet.
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
# Generates .cc and .h files for string, lookup tables, etc.

"""Compiles python string tables/arrays into .cc and .h files."""

import os
import string
import sys


class ResourceEntry(object):
  
  def __init__(self, index, key, value, dupe_of, table, in_ram):
    self._index = index
    self._in_ram = in_ram
    self._key = key
    self._value = value
    self._dupe_of = self._key if dupe_of is None else dupe_of
    self._table = table

  @property
  def variable_name(self):
    return '%s_%s' % (self._table.prefix.lower(), self._dupe_of)

  @property
  def declaration(self):
    c_type = self._table.c_type
    name = self.variable_name
    storage = ' IN_RAM' if self._in_ram else ''
    return 'const %(c_type)s %(name)s[]%(storage)s' % locals()
    
  def Declare(self, f):
    if self._dupe_of == self._key:
      # Dupes are not declared.
      f.write('extern %s;\n' % self.declaration)

  def DeclareAlias(self, f):
    prefix = self._table.prefix
    key = self._key.upper()
    index = self._index
    if self._table.python_type == str:
      comment = '  // %s' % self._value
      size = None
    else:
      comment = ''
      size = len(self._value)
    f.write('#define %(prefix)s_%(key)s %(index)d%(comment)s\n' % locals())
    if not size is None:
      f.write('#define %(prefix)s_%(key)s_SIZE %(size)d\n' % locals())
  
  def Compile(self, f):
    # Do not create declaration for dupes.
    if self._dupe_of != self._key:
      return
    
    declaration = self.declaration
    if self._table.python_type == float:
      f.write('%(declaration)s = {\n' % locals())
      n_elements = len(self._value)
      for i in xrange(0, n_elements, 4):
        f.write('  ');
        f.write(', '.join(
            '% 16.9e' % self._value[j] \
            for j in xrange(i, min(n_elements, i + 4))))
        f.write(',\n');
      f.write('};\n')
    elif self._table.python_type == str:
      value = self._value
      f.write('static %(declaration)s = "%(value)s";\n' % locals())
    else:
      f.write('%(declaration)s = {\n' % locals())
      n_elements = len(self._value)
      for i in xrange(0, n_elements, 4):
        f.write('  ');
        f.write(', '.join(
            '%6d' % self._value[j] if self._value[j] < 1 << 31 else \
                '%6dUL' % self._value[j] \
            for j in xrange(i, min(n_elements, i + 4))))
        f.write(',\n');
      f.write('};\n')
    

class ResourceTable(object):
  
  def __init__(self, resource_tuple):
    self.name = resource_tuple[1]
    self.prefix = resource_tuple[2]
    self.c_type = resource_tuple[3]
    self.python_type = resource_tuple[4]
    self.ram_based_table = resource_tuple[5]
    self.entries = []
    self._ComputeIdentifierRewriteTable()
    keys = set()
    values = {}
    for index, entry in enumerate(resource_tuple[0]):
      if self.python_type == str:
        # There is no name/value for string entries
        key, value = entry, entry.strip()
      else:
        key, value = entry

      # Add a prefix to avoid key duplicates.
      in_ram = 'IN_RAM' in key
      key = key.replace('IN_RAM', '')
      key = self._MakeIdentifier(key)
      while key in keys:
        key = '_%s' % key
      keys.add(key)
      hashable_value = tuple(value)
      self.entries.append(ResourceEntry(index, key, value,
          values.get(hashable_value, None), self, in_ram))
      if not hashable_value in values:
        values[hashable_value] = key
  
  def _ComputeIdentifierRewriteTable(self):
    in_chr = ''.join(map(chr, range(256)))
    out_chr = [ord('_')] * 256
    # Tolerated characters.
    for i in string.uppercase + string.lowercase + string.digits:
      out_chr[ord(i)] = ord(i.lower())

    # Rewritten characters.
    in_rewritten = '\x00\x01\x02\x03\x04\x05\x06\x07\x08\x09~*+=><^"|'
    out_rewritten = '0123456789TPSeglxpv'
    for rewrite in zip(in_rewritten, out_rewritten):
      out_chr[ord(rewrite[0])] = ord(rewrite[1])

    table = string.maketrans(in_chr, ''.join(map(chr, out_chr)))
    bad_chars = '\t\n\r-:()[]"\',;'
    self._MakeIdentifier = lambda s:s.translate(table, bad_chars)
  
  def DeclareEntries(self, f):
    if self.python_type != str:
      for entry in self.entries:
        entry.Declare(f)

  def DeclareAliases(self, f):
    for entry in self.entries:
      entry.DeclareAlias(f)
  
  def Compile(self, f):
    # Write a declaration for each entry.
    for entry in self.entries:
      entry.Compile(f)
    
    # Write the resource pointer table.
    c_type = self.c_type
    name = self.name
    f.write(
        '\n\nconst %(c_type)s* %(name)s_table[] = {\n' % locals())
    for entry in self.entries:
      f.write('  %s,\n' % entry.variable_name)
    f.write('};\n\n')


class ResourceLibrary(object):
  
  def __init__(self, root):
    self._tables = []
    self._root = root
    # Create resource table objects for all resources.
    for resource_tuple in root.resources:
      # Split a multiline string into a list of strings
      if resource_tuple[-2] == str:
        resource_tuple = list(resource_tuple)
        resource_tuple[0] = [x for x in resource_tuple[0].split('\n') if x]
        resource_tuple = tuple(resource_tuple)
      self._tables.append(ResourceTable(resource_tuple))

  @property
  def max_num_entries(self):
    max_num_entries = 0
    for table in self._tables:
      max_num_entries = max(max_num_entries, len(table.entries))
    return max_num_entries

  def _OpenNamespace(self, f):
    if self._root.namespace:
      f.write('\nnamespace %s {\n\n' % self._root.namespace)

  def _CloseNamespace(self, f):
    if self._root.namespace:
      f.write('\n}  // namespace %s\n' % self._root.namespace)

  def _DeclareTables(self, f):
    for table in self._tables:
      f.write('extern const %s* %s_table[];\n\n' % (table.c_type, table.name)) 

  def _DeclareEntries(self, f):
    for table in self._tables:
      table.DeclareEntries(f)

  def _DeclareAliases(self, f):
    for table in self._tables:
      table.DeclareAliases(f)
  
  def _CompileTables(self, f):
    for table in self._tables:
      table.Compile(f)
  
  def GenerateHeader(self):
    root = self._root
    f = file(os.path.join(root.target, 'resources.h'), 'wb')
    # Write header and header guard
    header_guard = root.target.replace(os.path.sep, '_').upper()
    header_guard = '%s_RESOURCES_H_' % header_guard
    f.write(root.header + '\n\n')
    f.write('#ifndef %s\n' % header_guard)
    f.write('#define %s\n\n' % header_guard)
    f.write(root.includes + '\n\n')
    self._OpenNamespace(f)
    f.write('typedef %s ResourceId;\n\n' % \
        root.types[self.max_num_entries > 255])
    self._DeclareTables(f)
    self._DeclareEntries(f)
    self._DeclareAliases(f)
    self._CloseNamespace(f)
    f.write('\n#endif  // %s\n' % (header_guard))
    f.close()
    
  def GenerateCc(self):
    root = self._root
    file_name = os.path.join(self._root.target, 'resources.cc')
    f = file(file_name, 'wb')
    f.write(self._root.header + '\n\n')
    f.write('#include "%s"\n' % file_name.replace('.cc', '.h'))
    self._OpenNamespace(f)
    self._CompileTables(f)
    self._CloseNamespace(f)
    f.close()


def Compile(path):
  # A hacky way of loading the py file passed as an argument as a module +
  # a descent along the module path.
  base_name = os.path.splitext(path)[0]
  sys.path += [os.path.abspath('.')]
  resource_module = __import__(base_name.replace('/', '.'))
  for part in base_name.split('/')[1:]:
    resource_module = getattr(resource_module, part)

  library = ResourceLibrary(resource_module)
  library.GenerateHeader()
  library.GenerateCc()


def main(argv):
  for i in xrange(1, len(argv)):
    Compile(argv[i])


if __name__ == '__main__':
  main(sys.argv)
