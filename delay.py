#!/usr/bin/python2

import sys

if len(sys.argv) != 3:
    print("Usage: delay.py <inputfile> <outputfile>")
    sys.exit()

inputfile = sys.argv[1]
outputfile = sys.argv[2]

print "Input file: ", inputfile
print "output file: ", outputfile
        
f=open(inputfile, "r")
#f=open(sys.argv[1], "r")
# Write another file
#fo=open("native-delay.dat", 'w')
fo=open(outputfile, 'w')

line=f.readline()

#ll="Node\tType\tDelayMS\tRetxCount\tHopCount\n"  # title line
ll="SeqNo\tFullDelayS\n"  # title line

fo.write(ll)

line=f.readline()
line=line.strip('\n')
while line:
    ll=line.split('\t')
    if ll[4]=="FullDelay" :
        s = [ll[3], ll[5]] # get SeqNo and DelayS

        s = '\t'.join(s)  # joing elements with '\t' 
        s = s + '\n'      # add '\n' at the end
        print s
        fo.write(s)
    line=f.readline()
    line=line.strip('\n')

f.close()
fo.close()





