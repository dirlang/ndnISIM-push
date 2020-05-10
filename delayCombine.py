#!/usr/bin/python2

# file delayCombine.py
# combine columns from muti similar two-column file
# Usage: python delayCombine.py  f1.dat f2.dat
# file1: multi comlumns
# file2: must contain 2 columns ## Note: file name not duplicate
# Both: column 1 is SeqNo
# Output: updated file1

import sys, os
from pandas import Series
import pandas as pd


if len(sys.argv) < 3:
    sys.exit("less argv!!!")

f1 = sys.argv[1]
f2 = sys.argv[2]
(f1_name, ext) = os.path.splitext(f1) # separate extention
(f2_name, ext) = os.path.splitext(f2) # separate extention

df1 = pd.read_csv(f1, sep='\t')
if len(df1.columns)==2: # first file is two-column
    df1.columns = ['SeqNo', f1_name] 

df2=pd.read_csv(f2, sep='\t')
df2.columns = ['SeqNo', f2_name]

df3 = pd.concat([df1, df2], sort=True)

## remove duplicates
df4=df3.drop_duplicates(subset='SeqNo', keep=False)

## intersections
df5 = pd.merge(df1, df2)

### add inter
df6 = pd.concat([df4, df5],sort=True)

# Sorted
df7 = df6.sort_values(by='SeqNo')

df8 = df7.fillna(0) # replace nan by 0

df8.to_csv('delay.csv', sep = '\t', index=None)

print df8







