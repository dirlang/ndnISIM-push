#!/usr/bin/python2

import sys, os
import pandas as pd
from pandas import DataFrame
import numpy as np

from string import digits


f1 = sys.argv[1]
f2 = sys.argv[2]

df1 = pd.read_csv(f1, sep='\t')
df2 = pd.read_csv(f2, sep='\t')

(f1_name, ext) = os.path.splitext(f1)



header = f1_name.translate(None, digits)
# print df1['SeqNo'],type(df1['SeqNo'])

lis1=list(df1['FullDelayS'])
lis2=list(df2['FullDelayS'])

df3=pd.DataFrame({'FullDelayS':lis1})

df4=pd.DataFrame({'FullDelayS':lis2})

df5=df3.append(df4)

df5.to_csv('delay.csv', sep = '\t',  index=None)