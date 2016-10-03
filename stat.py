import numpy
import pylab

##########################################################
#
# Holds statistics about a specific clock cycle
#
##########################################################
class CStat:
	def __init__ (self):
		self.id = 0 # what clock am I?
		self.drop_count = 0 # number of dropped packets this clock
		self.packet_count = 0 # total number of packets in flight
		self.rate_count = 0 # packets issued in this clock
		self.retire_count = 0 # packets retired in this clock

infile = open('test.txt','r')

indata= []
clock_list = []

clcount = 0 #clock count
pcount = 0 #packet count
dcount = 0 #drop count
rcount = 0 #packet rate count
ecount = 0 #packet retire coune

##########################################################
#
# Parse the file and collect statistics
#
for line in infile:
	temp1 = []
	line = line.rstrip()
	for item in line.split(' '):
		temp1.append(item)
	
	mystat = CStat()

	if 'clock' in temp1:

		#print 'clock {}'.format(clcount)

		clcount = int(temp1[1])
		mystat.id = clcount
		mystat.drop_count = dcount
		mystat.packet_count = pcount
		mystat.rate_count = rcount
		mystat.retire_count = ecount
		#print 'clock {} {}'.format(clcount, pcount)

		clock_list.append(mystat)

		dcount = 0
		rcount = 0
		ecount = 0
			
	if 'drop' in temp1:
		dcount = dcount + 1
		pcount = pcount - 1
		#print 'drop {}'.format(pcount)
		#print temp1

	if 'txqueue_write' in temp1:
		pcount = pcount + 1
		rcount = rcount + 1
		#print 'fire {}'.format(pcount)

	if 'retire' in temp1:
		pcount = pcount - 1
		ecount = ecount + 1
		#print 'retire {}'.format(pcount)

	# list of raw file data
	#
	indata.append(temp1)

x = []
y1 = []
y2 = []
y3 = []

ravg = 0 #average clock rate for this simulation
davg = 0 #average drop rate for this simulation
eavg = 0 #average retire rate for this simulation
c = 0 #clock count
for myclock in clock_list:
	
	if (myclock.drop_count == 0) | (myclock.packet_count == 0):
		temp = 0
	else:
		temp = float(myclock.packet_count)/float(myclock.drop_count)
#		print temp

	y3.append(temp)

	ravg = ravg + float(myclock.rate_count)
	davg = davg + float(myclock.drop_count)
	eavg = eavg + float(myclock.retire_count)
	
	c = c + 1

ravg = ravg/float(c)
davg = davg/float(c)
eavg = eavg/float(c)

if (ravg == 0) or (davg == 0):
	avg = 0
else:
	avg = ravg/davg


print '{} {} {}'.format(ravg, davg, eavg)



#pylab.plot(x, y3, 'ok-')                     # do a line plot of data
#pylab.plot(x, y2, 'ok--')                    # plot the other data with a dashed line
#pylab.show()
