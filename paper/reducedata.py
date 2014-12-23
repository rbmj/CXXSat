#!/usr/bin/env python3
import sys
import statistics
import math

numpoints = -1
data = []

for line in sys.stdin:
    if numpoints == -1:
        data = [(x.strip(), []) for x in line.split(',')]
    else:
        points = [float(x) for x in line.split(',')]
        for i in range(len(points)):
            data[i][1].append(points[i])
    numpoints = numpoints + 1

for x in data:
    mean = statistics.mean(x[1])
    error = statistics.stdev(x[1], mean)/math.sqrt(numpoints)
    error = error * 1.96 # 95% confidence interval
    print("%s %f %f" % (x[0], mean, error))
