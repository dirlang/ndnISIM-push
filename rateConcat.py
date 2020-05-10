#!/usr/bin/python2

import sys, os
import pandas as pd
from pandas import DataFrame

if len(sys.argv)==6:
	
	f1 = sys.argv[1]
	f2 = sys.argv[2]
	f3 = sys.argv[3]
	f4 = sys.argv[4]
	frequency=sys.argv[5]

	df1 = pd.read_csv(f1, sep='\t')

	df2 = pd.read_csv(f2, sep='\t')

	df3 = pd.read_csv(f3, sep='\t')

	df4 = pd.read_csv(f4, sep='\t')



	df_temp=df1.join(df2)
	df_temp=df_temp.join(df3)
	df_temp=df_temp.join(df4)

	fre=DataFrame({"frequency":[frequency]})
	df_temp=fre.join(df_temp)

	print df_temp

	df_temp.to_csv('averageSatisfiedRate'+'-'+frequency+'.csv',sep='\t', index=None)


if len(sys.argv)==4:
	f1 = sys.argv[1]
	f2 = sys.argv[2]
	frequency=sys.argv[3]

	df1 = pd.read_csv(f1, sep='\t')

	df2 = pd.read_csv(f2, sep='\t')

	df_temp=df1.join(df2)

	fre=DataFrame({"frequency":[frequency]})
	df_temp=fre.join(df_temp)

	print df_temp

	df_temp.to_csv('averageSatisfiedRate'+'-'+frequency+'.csv',sep='\t', index=None)