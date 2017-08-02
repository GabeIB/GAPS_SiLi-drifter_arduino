#Gabriel Brown
#GAPS Summer 2017
#parses drift data from labview to remove outlier data points

import csv
import numpy as np
import matplotlib.pyplot as plt
import sys

file_name = sys.argv[1]
file = open(file_name, 'r')
file = file.readlines()
output = open('parsed_drift.txt', 'w')

#reading from labview txt file
lc = 3
temp = 4 
time = 1 
hv = 2 

#maximum acceptable slope for data point to have from last point
temp_jump = .2
lc_jump = .00001
hv_jump = 3

header = '***End_of_Header***\t\t\t\t\t\t\t\t\r\n'

#finds where the actual data starts being recorded and returns the line number
def dataStart(file, header):
	start = 0
	ii = 0
	for line in file:
		if(line == header):
			start = ii
			break
		ii += 1
	return start

#turns the file into a 2 dimensional list storing all data
def listParse(file, start):
	ii = 0
	returnList = []
	for line in file:
		if(ii > start+1):
			storage = line.split('\t')
			returnList.append(storage)
		ii += 1
	return returnList

#processes data to remove points that are significantly differant from the point before
#these points are caused by occasional unreliability in the analog to digital converter in the hardware of the drifter
def removeJumps(data):
	global lc, temp, time, hv, temp_jump, lc_jump, hv_jump
	previousLine = data[0]
	smoothList = []
	ranOnce=False
	for line in data:
		if(ranOnce):
			time_change = float(line[time])-float(previousLine[time])
			lc_change = abs(float(previousLine[lc])-float(line[lc]))/time_change
			hv_change = abs(float(previousLine[hv])-float(line[hv]))/time_change
			temp_change = abs(float(previousLine[temp])-float(line[temp]))/time_change
			if(lc_change < lc_jump and hv_change < hv_jump and temp_change < temp_jump):
				smoothList.append(line)
				previousLine=line
		ranOnce=True
	return smoothList

#processes data to be turned into graphs
#also converts time from seconds to hours
def pullData(data, first, second):
	x_data = []
	y_data = []
	package = []
	for line in data:
		if(first==time):
			x_data.append(float(line[first])/360) #convert time from seconds to hours
		else:
			x_data.append(float(line[first]))
		y_data.append(float(line[second]))
	package.append(x_data)
	package.append(y_data)
	return package

#shows all the graphs in one window
def showGraphs(graph1, graph2, graph3, graph4, graph5):
	var_array = [graph1, graph2, graph3, graph4, graph5]
	axis_labels = [["leakage current(Amps)", "time(hr)"], ["voltage(V)", "time(hr)"], ["temperture(c)", "time(hr)"], ["leakage current(Amps)", "temperture(c)"], ["leakage current(Amps)", "voltage(V)"]]
	left  = 0.125  # the left side of the subplots of the figure
	right = 0.9    # the right side of the subplots of the figure
	bottom = 0.1   # the bottom of the subplots of the figure
	top = 0.9      # the top of the subplots of the figure
	wspace = 0.3   # the amount of width reserved for blank space between subplots
	hspace = 0.3   # the amount of height reserved for white space between subplots
	ii=0
	for graph in var_array:
		ii+=1
		plt.subplot(2,3,ii)
		plt.plot(graph[0],graph[1])
		plt.ylabel(axis_labels[ii-1][0])
		plt.xlabel(axis_labels[ii-1][1])
		if(axis_labels[ii-1][0]=="leakage current(Amps)"):
			plt.ticklabel_format(style='sci', axis='y', scilimits=(0,0))
	plt.subplots_adjust(left, bottom, right, top, wspace, hspace)
	plt.show()

#main
start_index = dataStart(file, header)
data = listParse(file, start_index)
data = removeJumps(data)
lc_time = pullData(data, time, lc)
hv_time = pullData(data, time, hv)
temp_time = pullData(data, time, temp)
lc_temp = pullData(data, temp, lc)
lc_hv = pullData(data, hv, lc)
showGraphs(lc_time, hv_time, temp_time, lc_temp, lc_hv)
