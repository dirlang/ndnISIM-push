#!/usr/bin/python2

import sys
from pandas import DataFrame



inputfile = sys.argv[1]
#outputfile = sys.argv[2]

print "Input file: ", inputfile
#print "output file: ", outputfile
        
f=open(inputfile, "r")
#f=open(sys.argv[1], "r")
# Write another file
#fo=open("native-delay.dat", 'w')
#fo=open(outputfile, 'w')

line=f.readline()

#ll="Node\tType\tDelayMS\tRetxCount\tHopCount\n"  # title line


#fo.write(ll)
listtemp=[]

line=f.readline()
line=line.strip('\n')
while line:
    ll=line.split('\t')
    if ll[4]=="FullDelay" :
        s = ll[3] # get SeqNo and DelayS

        listtemp.append(s)
        #s = '\t'.join(s)  # joing elements with '\t' 
        #s = s + '\n'      # add '\n' at the end
        #print s
        
    line=f.readline()
    line=line.strip('\n')



df2=DataFrame(columns=['SeqNo'])
df2['SeqNo']=listtemp


rate=float(len(df2['SeqNo']))/20

ll="SatisfiedRate\n"
fo=open('rate.dat','a+')
if fo.readline()!="SatisfiedRate\n":
    fo.write(ll)
fo.write(str(rate)+'\n')
print rate

f.close()
fo.close()





