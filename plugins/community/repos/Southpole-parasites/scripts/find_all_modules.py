#!/usr/bin/python

import os
import re

def findfiles(path, regex):
    regObj = re.compile(regex)
    res = []
    for root, dirs, fnames in os.walk(path):
        for fname in fnames:
            #print fname    
            if regObj.match(fname):
                res.append(os.path.join(root, fname))
    return res

def grep(filepath, regex):
    regObj = re.compile(regex)
    res = []
    with open(filepath) as f:
        for line in f:
            #print line
            if re.search('addModel',line):
                m2 = re.findall(r'\"(.+?)\"',line)
                #res.append(line)
                if len(m2) > 1:
                    print m2[0],"[",m2[1],"]"
    return res

headers = findfiles('.', r'.*cpp$')

#print headers

for filepath in headers:
#    print filepath
    res = grep( filepath, r'.*addModel.*"(.*)",')
    #print res