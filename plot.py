from numpy import *
import pylab
from matplotlib.pyplot import *


#infile = open('drop_rate.txt','r')
infile = open('e1.txt','r')

x = []
y1 = []
y2 = []
tlist = []

for line in infile:
	tup = [] #used to create a tuple
	temp1 = []
	line = line.rstrip()
	for item in line.split(' '):
		temp1.append(item)

	x.append(float(temp1[0]))
	y1.append(float(temp1[1]))
#	y2.append(float(temp1[3]))

pylab.title("C++ Simulator Results")                   # set plot title
pylab.xlabel("Packet Issue Rate")                         # add axis labels
pylab.ylabel("Drop Rate")    

coefficients = polyfit(x, y1, 3)
polynomial = poly1d(coefficients) 
         
xs = arange(0, 5, 0.1)
ys = polynomial(xs)

myplot = pylab.plot(x, y1, 'o') 
plot(xs, ys)
pylab.show()

