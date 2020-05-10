#!/usr/bin/python2

import sys, os
import pandas as pd
from pandas import DataFrame
import numpy as np

from string import digits


f1 = sys.argv[1]

outputfile = sys.argv[2]

#temp=sys.argv[3]

(f2_name, ext) = os.path.splitext(outputfile)

df1 = pd.read_csv(f1)

average=df1['FullDelayS'].mean()

df2=pd.DataFrame({f2_name:[average]})


print df2
df2.to_csv(outputfile, index=None)

print average