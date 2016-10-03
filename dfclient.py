#!/usr/bin/python

##########################################################
#
# Digital Fabric Simulator Client
#
##########################################################

from pylab import *
from Tkinter import *
from tkMessageBox import *
from threading import Thread

import thread
import random
import sys
import socket
import Queue
import time
import copy



main = Tk()
main.title("Digital Fabric Simulator Client V1.0")

##########################################################
#
# Create the canvas.
#
##########################################################

cwid = 700
cheight = 600
canvas = Canvas(main, width=cwid, height=cheight, bg='gray')
canvas.pack()


##########################################################
#
# Network Interface
#
##########################################################
class NetInterface:
	def __init__ (self, mygui):
		self.q = Queue.Queue()
		self.empty = 1
		self.lock = thread.allocate_lock()
		self.mydraw = UIDraw(mygui, self.q)
		self.myevent_list = EventList(self.q)
		self.myplot = DrawPlots(self.myevent_list)


	def connect(self):
		self.flag2 = 0
#		host = '131.252.209.116' #fab05.cecs.pdx.edu
		host = 'localhost'
		port = 8888
		size = 1024
		self.s = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
		self.s.connect((host,port))

		self.net_client = ThreadBuffer(self.q, self.lock, self.s, self.mydraw, self.myevent_list)
		self.net_client.setDaemon(True)
		self.net_client.start()
	
	def press_start(self):
		self.s.send("test1")

	def press_stop(self):
		self.s.send("stop")
		self.net_client.stop = 1

		# wait for thread to stop
		while self.net_client.stop_flag == 0:
			x=1

		self.s.close()
	


##########################################################
#
# Thread Buffer
#
##########################################################
class ThreadBuffer(Thread):
	def __init__ (self, myq, mylock, mysock, mydraw, event):
		Thread.__init__(self)
		self.myq = myq
		self.mylock = mylock
		self.mysock = mysock
		self.mydraw = mydraw
		self.myevent = event
		self.stop = 0
		self.stop_flag = 0
		self.lcount = 0

	def run(self):
		while True:
#			print "debug stop %d " % self.stop
			if self.stop == 0:
				self.mylock.acquire()  #blocks until we get lock
				self.buffer = self.mysock.recv(256) #blocks until we get input
				#print self.buffer
				self.myq.put(self.buffer)
				self.mylock.release()
				self.myq.task_done()
				self.mysock.send("next")
	#			print "debug2"
	#			if self.buffer == "done":
				if self.buffer.find("done") == 0:
					self.myevent.add_events()
			#		self.mydraw.doredraw()
				else:
					abc=1
			else:
				self.stop_flag = 1
			time.sleep(.00000001)


##########################################################
#
# 
#
##########################################################
class FIFOInfo:
	def __init__ (self):
		self.in_time = 0			
		self.drop = 0

		
##########################################################
#
# 
#
##########################################################
class NodeInfo:
	def __init__ (self):
		
		self.id = 0		

		self.a = FIFOInfo()
		self.n = FIFOInfo()
		self.s = FIFOInfo()
		self.e = FIFOInfo()
		self.w = FIFOInfo()

		self.at_dest = 0

##########################################################
#
# 
#
##########################################################
class PacketInfo:
	def __init__ (self):
		self.timestamp = 0
		self.src_mod = 0
		self.dst_mod = 0
		self.start_time = 0
		self.stop_time = 0
		self.cur_time = 0
		self.hops = []




##########################################################
#
# database of events
#
##########################################################
class EventList:
	def __init__ (self, q):
		self.x = 0
		self.myq = q
		self.myevent = []
		self.count = 0
		self.mycount = 0

		self.packet_list = []
		self.mynode_info = NodeInfo()
		self.mypacket_info = PacketInfo()

		# for each clock store data
		self.volume = []
		self.jitter = []
		
	
	def add_events(self):
		while not self.myq.empty():
			self.buffer = ""
			self.buffer = self.myq.get()
#			print self.buffer
			self.spl = self.buffer.split()
			if self.myq.empty():
				abc=1
			else:
#				print self.mycount
				print self.buffer
				self.mycount = self.mycount + 1
				# scratchpad for node info
				self.mynode_info.n.in_time = 0
				self.mynode_info.s.in_time = 0
				self.mynode_info.e.in_time = 0
				self.mynode_info.w.in_time = 0

				del self.myevent[:]
				for i in range (0, 9):
					temp = int(self.spl[(i)])
					self.myevent.append(temp)

				#print 'event {0}'.format(self.myevent)



				#Check if this is the destination
				if (self.myevent[1] == self.myevent[6]):
					for i in range (0, len(self.packet_list)):
						if ((self.myevent[4] == self.packet_list[i].timestamp) and (self.myevent[5] == self.packet_list[i].src_mod)):
							self.packet_list[i].stop_time = self.myevent[0]
				else:
						
					if len(self.packet_list) == 0:
						# Add first packet
						self.get_packet_info()

						# Add starting point for this packet
#						self.mypacket_info.hops.append(copy.deepcopy(self.mynode_info))
						self.packet_list.append(copy.deepcopy(self.mypacket_info))

						# print 'first {0} {1}'.format(self.mypacket_info.timestamp, self.mypacket_info.src_mod)
					else:

						# Search for a match in the current list
						match1 = 0
						matchp = 0
						for i in range (0, len(self.packet_list)):
							#print 'search {0} {1} {2}'.format(self.myevent[0], self.packet_list[i].timestamp, self.packet_list[i].src_mod)
							if ((self.myevent[4] == self.packet_list[i].timestamp) and 
									(self.myevent[5] == self.packet_list[i].src_mod) and
									(self.myevent[6] == self.packet_list[i].dst_mod)):
								
								self.packet_list[i].cur_time = self.myevent[0]
								match1 = 1
								matchp = i

								# Search for match in node list
								match2 = 0
								for j in range (0, len(self.packet_list[matchp].hops)):
									if self.myevent[1] == self.packet_list[matchp].hops[j].id:
										match2 = 1
										# add new info to current node
										# print 'adding new info to current node {0}'.format(self.packet_list[matchp].hops[i].id)
										if self.myevent[2] == 1:
											self.packet_list[matchp].hops[j].n.in_time = self.myevent[0]
										elif self.myevent[2] == 2:
											self.packet_list[matchp].hops[j].s.in_time = self.myevent[0]
										elif self.myevent[2] == 3:
											self.packet_list[matchp].hops[j].e.in_time = self.myevent[0]
										elif self.myevent[2] == 4:
											self.packet_list[matchp].hops[j].w.in_time = self.myevent[0]

								if match2 == 0:
									# add new info to new node
									# print "adding new info to new node"
									self.mynode_info.id = self.myevent[1]
									if self.myevent[2] == 1:
										self.mynode_info.n.in_time = self.myevent[0]
									elif self.myevent[2] == 2:
										self.mynode_info.s.in_time = self.myevent[0]
									elif self.myevent[2] == 3:
										self.mynode_info.e.in_time = self.myevent[0]
									elif self.myevent[2] == 4:
										self.mynode_info.w.in_time = self.myevent[0]

									self.packet_list[matchp].hops.append(copy.deepcopy(self.mynode_info))



						# Add a new packet
						if match1 == 0:
							self.mypacket_info.start_time = self.myevent[0]
							self.mypacket_info.cur_time = self.myevent[0]
							self.get_packet_info()
#							self.mypacket_info.hops.append(copy.deepcopy(self.mynode_info))
							self.packet_list.append(copy.deepcopy(self.mypacket_info))
							#print 'new {0} {1}'.format(self.mypacket_info.timestamp, self.mypacket_info.src_mod)

							#print len(self.packet_list)




	#------------------------------------------------------------
	#
	#------------------------------------------------------------
	def get_packet_info(self):				
		self.mypacket_info.timestamp = self.myevent[4]
		self.mypacket_info.src_mod = self.myevent[5]
		self.mypacket_info.dst_mod = self.myevent[6]

		# Add node info
		self.mynode_info.id = self.myevent[1]
		if self.myevent[2] == 1:
			self.mynode_info.n.in_time = self.myevent[0]
		elif self.myevent[2] == 2:
			self.mynode_info.s.in_time = self.myevent[0]
		elif self.myevent[2] == 3:
			self.mynode_info.e.in_time = self.myevent[0]
		elif self.myevent[2] == 4:
			self.mynode_info.w.in_time = self.myevent[0]

	#------------------------------------------------------------
	#
	#------------------------------------------------------------
	def calc_jitter(self):
		del self.jitter[:]
		for i in range (100, 600):
			mysum = 0
			pcount = 0 # number of active packets
			for j in range (0, len(self.packet_list)):
				# Stop time can't be zero. This indicates the packet 
				# did not reach it's destination before the simulation ended
				if ((self.packet_list[j].start_time <= i) and (i < self.packet_list[j].stop_time)):
					pcount = pcount + 1
					exp_time = len(self.packet_list[j].hops) * 10 # expected time
					cur_time = self.packet_list[j].cur_time #current time
					mysum = mysum + (cur_time - exp_time)
			self.jitter.append(mysum/pcount)

	#------------------------------------------------------------
	#
	#------------------------------------------------------------
	def calc_volume(self):
		del self.volume[:]
		for i in range (100, 600):
			mysum = 0
			for j in range (0, len(self.packet_list)):
				if ((self.packet_list[j].start_time <= i) and (i <= self.packet_list[j].stop_time)):
					mysum = mysum + 1
			self.volume.append(mysum)

	#------------------------------------------------------------
	#
	#------------------------------------------------------------
	def print_data(self):
		for i in range (0, len(self.packet_list)):
#			print 'list {0} {1}'.format(self.packet_list[i].timestamp, self.packet_list[i].src_mod)

			print 'list ts {0}, src mod {1}, dst mod {2} start {3} stop {4} num hops {5}'.format(
				self.packet_list[i].timestamp, 
				self.packet_list[i].src_mod,
				self.packet_list[i].dst_mod,
				self.packet_list[i].start_time,
				self.packet_list[i].stop_time,
				len(self.packet_list[i].hops)
				)


			for j in range (0, len(self.packet_list[i].hops)):
				print 'id {0}, n {1}, s {2}, e {3}, w {4}'.format(self.packet_list[i].hops[j].id, 
					self.packet_list[i].hops[j].n.in_time,
					self.packet_list[i].hops[j].s.in_time,
					self.packet_list[i].hops[j].e.in_time,
					self.packet_list[i].hops[j].w.in_time)

##########################################################
#
# Draw plots
#
##########################################################
class DrawPlots:
	def __init__ (self, events):
		self.temp = 1
		self.events = events
		self.x = []
		self.y1 = []
		self.y2 = []

	def mydraw(self):

		#self.events.print_data()
		self.events.calc_volume()
		self.events.calc_jitter()

		print len(self.events.jitter)
		print len(self.events.volume)

		for i in range (0, len(self.events.volume)):
			self.x.append(i)
#			self.y1.append(self.events.volume)
#			self.y2.append(self.events.jitter)

#		x = [0,1,2,3]        # list of x values
#		y1 = [1,14,42,81]     # list of y values
#		y2 = [7,39,79,131]    # another list of y values
#		y3 = [53,169,307,488] # ya you get it...

		plot(self.x, self.events.volume, label="volume")                     # do a line plot of data
		plot(self.x, self.events.jitter, label="jitter")                     # do a line plot of data
#		plot(self.x, self.y2, 'ok--')                     # do a line plot of data
#		plot(x, y2, 'ok--')                    # plot the other data with a dashed line
#		plot(x, y3, 'ok-.')                    # plot the other data with a dashed line
		 
		xlabel("time (clocks)")                         # add axis labels
		ylabel("packets")              
		legend()
		xticks(self.x)                             # set x axis ticks to x values
		title("Packet Volume")                   # set plot title
#		grid(True, ls = '-', c = '#a0a0a0')   # turn on grid lines
		 
		savefig("pylab_example.svg")          # save as SVG
		 
		show()                                # show plot in GUI (optional)


##########################################################
#
# Redrawing for User Interface
#
##########################################################
class UIDraw:
	def __init__ (self, mynodes, myq):
		self.temp = 1
		self.myq = myq
		self.mynodes = mynodes

		self.colors = ["#199199199", "#333333333", "#4cc4cc4cc", "#666666666", "#800800800", 
                       "#999999999", "#b33b33b33", "#ccccccccc", "#e66e66e66", "#fffffffff"]

	def doredraw(self):
		while not self.myq.empty():
			self.buffer = ""
			self.buffer = self.myq.get()
			self.spl = self.buffer.split()
#			time.sleep(.001)
			if self.myq.empty():
				print "empty"
			else:
				abc=1
				#print self.buffer
	def imdone(self):
		self.mynodes[0].app.uconfig(self.colors[4])


##########################################################
#
# Node
#
##########################################################
class Node:
	def __init__(self, nid, x, y):
		self.a_x = x
		self.a_y = y
		self.b_x = x + 20
		self.b_y = y + 50
		self.color = 'blue'

		canvas.create_rectangle(x, y, x+100, y+100, fill='blue', outline='black')

		self.app = APP(x-20, y-20, x+20, y+20)

		self.north_in = NodePort(x+30, y, x+50, y+20)
		self.north_out = NodePort(x+50, y, x+70, y+20)

		self.south_in = NodePort(x+30, y+80, x+50, y+100)
		self.south_out = NodePort(x+50, y+80, x+70, y+100)

		self.east_in = NodePort(x+80, y+30, x+100, y+50)
		self.east_out = NodePort(x+80, y+50, x+100, y+70)

		self.west_in = NodePort(x, y+30, x+20, y+50)
		self.west_out = NodePort(x, y+50, x+20, y+70)


#		canvas.tag_bind(self.north_in.rec, '<Button-1>', self.north_in.action)

		mytext = "Node: %d" % (nid)
		mylab = Label(main, text=mytext)
		canvas.create_window(self.a_x+70, self.a_y-20, window=mylab)





##########################################################
#
# Node Port
#
##########################################################
class NodePort:

	def __init__(self, ax, ay, bx, by):
		self.a_x = ax
		self.a_y = ay
		self.b_x = bx
		self.b_y = by
		self.color = 'green'

		self.rec = canvas.create_rectangle(self.a_x, self.a_y, self.b_x, self.b_y, fill=self.color, outline='black')

	def uconfig(self,color):
		canvas.itemconfigure(self.rec, fill=color)

	def action(self, dummy):
		mylab = Label(main, text="Lenght: 8")
		canvas.create_window(self.a_x, self.a_y-5, window=mylab)



 
##########################################################
#
# APP
#
##########################################################
class APP:

	def __init__(self, ax, ay, bx, by):
		self.a_x = ax
		self.a_y = ay
		self.b_x = bx
		self.b_y = by
		self.color = 'SeaGreen2'

		self.flag = 0

		self.rec = canvas.create_rectangle(self.a_x, self.a_y, self.b_x, self.b_y, fill=self.color, outline='black')

	def uconfig(self,color):
		canvas.itemconfigure(self.rec, fill=color)

##########################################################
#
# Main Program
#
##########################################################
nodes = {}
frames = {}
text_list = {}
test_list = {}

def oval_act():
    showerror(main, "Can't connect yet.")


#mybuff = GUIBuffer()

mynet = NetInterface(test_list)


x=30;
y=30;
k=0;
for m in range (0, 4):
	for n in range (0, 4):
		test_list[(k)] = Node(k,x,y)
		x = x + 100 + 50
		k = k + 1
	y = y + 100 + 40
	x = 30;




cbut =  Button(main, text='Connect', command=mynet.connect)
canvas.create_window(650,50,window=cbut)

exbut = Button(main, text='Exit', command=sys.exit)
canvas.create_window(650,80,window=exbut)

sbut = Button(main, text='Start', command=mynet.press_start)
canvas.create_window(650,110,window=sbut)

stbut = Button(main, text='Stop', command=mynet.press_stop)
canvas.create_window(650,140,window=stbut)

stbut = Button(main, text='dropped packets', command=mynet.myplot.mydraw)
canvas.create_window(650,170,window=stbut)


main.mainloop()


