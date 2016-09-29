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
allmodels = ["geometric", "fromtop", "random", "graycode"]
allfamilies = ["murmur", "koloboke", "zobrist", "wide-zobrist", "tztabulated", "cllinear", "clquadratic", "clcubic", "cwlinear",  "cwquadratic", "cwcubic", "multiplyshift" ]
scriptlocation = os.path.dirname(os.path.abspath(__file__))

#Usage: ./param_htbenchmark.exe -l [maxloadfactor:0-1] -s [size:>0] -m [model:0-3] -H [hashfamily:0-11]
def gethisto(size, model, family):
  pipe = subprocess.Popen([scriptlocation+"/../"+"param_htbenchmark.exe", "-l", "1", "-s" , str(size),  "-m", str(model), "-H", str(family)], stdout=subprocess.PIPE, stderr=subprocess.STDOUT)
  res = pipe.communicate()[0].decode().split("\n")
  res = tuple(filter(lambda x: len(x)>0,res))
  res = tuple(filter(lambda x: not(x.startswith("#")),res))
  effectiveload = float(res[0].split()[0])
  histo = tuple(map(int,res[1].split()))
  return (effectiveload, histo)



import argparse
parser = argparse.ArgumentParser()
parser.add_argument("model", type=int, help="the model [0,4) "+str(allmodels))
parser.add_argument("size", type=int, help="the number of keys, e.g., 1000000", default=1000000)
parser.add_argument("repeat", type=int, help="how many hash tables for each hash function", default = 1024)
args = parser.parse_args()
size = args.size
model = args.model
repeat = args.repeat

def maxhistosize(allhistos):
    return max(len(h) for h in allhistos)

def valorzero(histo,i):
    if(i < len(histo)): return histo[i]
    return 0


def statsfromhistos(allhistos) :
    ms = maxhistosize(allhistos)
    maxhisto = [0 for i in range(ms)]
    for histo in allhistos:
        maxhisto[len(histo)-1] += 1
    for i in range(ms):
        print(i,maxhisto[i])

print("#model=",allmodels[model])
print("#size=",str(size))
print("#repeat=",str(repeat))
effectiveload = 0

for family in range(len(allfamilies)):
    allhistos = []
    print("# family",allfamilies[family])
    if(allfamilies[family] == "tztabulated"):
        print("#skipping")
        print()
        print()
        continue
    for test in range(repeat):
        effectiveload, histo = gethisto(size,model,family)
        allhistos.append(histo)
    print("# effectiveload=", effectiveload)
    statsfromhistos(allhistos)
    print()
    print()
