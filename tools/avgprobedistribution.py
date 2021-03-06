#!/usr/bin/env python3
########################################################################
import sys
import re
import subprocess
import io
import os

# model 0 is geometric
# model 1 is fromtop
# model 2 is random
# model 3 is graycode
# hashfamily 0 is murmur
# hashfamily 1 is koloboke
# hashfamily 2 is zobrist
# hashfamily 3 is wide-zobrist
# hashfamily 4 is tztabulated
# hashfamily 5 is cllinear
# hashfamily 6 is clquadratic
# hashfamily 7 is clcubic
# hashfamily 8 is cwlinear
# hashfamily 9 is cwquadratic
# hashfamily 10 is cwcubic
# hashfamily 11 is multiplyshift
# hashfamily 12 is cyclic
# hashfamily 13 is fnv
# hashfamily 14 is identity
# hashfamily 15 is oddmultiply
# hashfamily 16 is randkolokobe

allmodels = ["geometric", "fromtop", "random", "graycode"]
allfamilies = ["murmur", "koloboke", "zobrist", "wide-zobrist", "tztabulated", "cllinear", "clquadratic", "clcubic", "cwlinear",  "cwquadratic", "cwcubic", "multiplyshift", "cyclic" , "fnv", "identity", "oddmultiply", "randkolokobe"]
scriptlocation = os.path.dirname(os.path.abspath(__file__))

#Usage: ./param_htbenchmark.exe -l [maxloadfactor:0-1] -s [size:>0] -m [model:0-3] -H [hashfamily:0-11]
def getavgprobe(size, model, family):
  pipe = subprocess.Popen([scriptlocation+"/../"+"param_htbenchmark.exe", "-l", "1", "-s" , str(size),  "-m", str(model), "-H", str(family)], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
  res = pipe.communicate()[0].decode().split("\n")
  res = tuple(filter(lambda x: len(x)>0,res))
  res = tuple(filter(lambda x: not(x.startswith("#")),res))
  effectiveload = float(res[0].split()[0])
  avgprobe = float(res[0].split()[1])
  return (effectiveload, avgprobe)



import argparse
parser = argparse.ArgumentParser()
parser.add_argument("model", type=int, help="the model [0,4) "+str(allmodels))
parser.add_argument("size", type=int, help="the number of keys, e.g., 1000000", default=1000000)
parser.add_argument("repeat", type=int, help="how many hash tables for each hash function", default = 1024)
args = parser.parse_args()
size = args.size
model = args.model
repeat = args.repeat

print("#model=",allmodels[model])
print("#size=",str(size))
print("#repeat=",str(repeat))
effectiveload = 0

for family in range(len(allfamilies)):
    maxageprobe = 0
    print("# family",allfamilies[family])
    if(allfamilies[family] == "tztabulated"):
        print("#skipping")
        print()
        print()
        continue
    for test in range(repeat):
        effectiveload, avgprobe = getavgprobe(size,model,family)
        print(avgprobe, flush=True)
        if (avgprobe > maxageprobe) :
          maxageprobe = avgprobe
    print("# effectiveload=", effectiveload)
    print("# this was the end of ", allfamilies[family], " max avg probe ", maxageprobe)
    print()
    print()
