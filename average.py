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
from pandas import DataFrame
import numpy as np


f1 = sys.argv[1]
# f2 = sys.argv[2]
# (f1_name, ext) = os.path.splitext(f1) # separate extention
#(f2_name, ext) = os.path.splitext(f2) # separate extention

df1 = pd.read_csv(f1, sep='\t')

print(df1.columns[1][:-1])
columns2=df1.columns[1][:-1]
df2=DataFrame(columns=['SeqNo',columns2])
df2['SeqNo']=df1['SeqNo']
df5=[]
for x in range(0,len(df1)):   
	### make every row of delay.csv to a list
	df3=df1.iloc[[x]]
	df4=np.array(df3)
	df3_list=df4.tolist()
	df3_list=df3_list[0]
	
	df3_list=map(float,df3_list)
	del df3_list[0]
	# df3_list.remove(x)    
	   
	###  average
	y=0
	sum=0
	for i in df3_list:
		if i!=0:
			y+=1
			sum+=i
	df5.append(sum/y)

df2[columns2]=df5

df2.to_csv(columns2+'.dat', sep = '\t', index=None)

print df2



#df2['columns2']=df3


	
#print df2


# if len(df1.columns)==2: # first file is two-column
#     df1.columns = ['SeqNo', f1_name] 

# df2=pd.read_csv(f2, sep='\t')
# df2.columns = ['SeqNo', f2_name]

# df3 = pd.concat([df1, df2], sort=True)

# ## remove duplicates
# df4=df3.drop_duplicates(subset='SeqNo', keep=False)

# ## intersections
# df5 = pd.merge(df1, df2)

# ### add inter
# df6 = pd.concat([df4, df5],sort=True)

# # Sorted
# df7 = df6.sort_values(by='SeqNo')

# df8 = df7.fillna(0) # replace nan by 0

# df8.to_csv('delay.csv', sep = '\t', index=None)

#print df8







