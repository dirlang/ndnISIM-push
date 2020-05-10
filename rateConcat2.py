#!/usr/bin/python2

import sys, os
import pandas as pd
from pandas import DataFrame

f1 = sys.argv[1]
f2 = sys.argv[2]
tep = sys.argv[3]

df1 = pd.read_csv(f1, sep='\t')

df2 = pd.read_csv(f2, sep='\t')

df3 = pd.concat([df1, df2])

df3.to_csv('averageSatisfiedRate'+'-'+tep+'.csv',sep='\t',index=None)