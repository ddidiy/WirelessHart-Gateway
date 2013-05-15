#!/usr/bin/python

import os
import sys
import socket
import SocketServer
import string
import threading
import time
import traceback
import rrdtool
import shutil
	
class MyTCPRequestHandler(SocketServer.BaseRequestHandler):

	"""
	The RequestHandler class for our server. It is instantiated once 
	per connection to the server, and must override the handle()
	method to implement communication to the client.
	"""

	WebRoot = '/var/www/'

	@classmethod
	def SetWebRoot(cls_, webRoot_):
		
		cls_.WebRoot = webRoot_
		cls_.Graphs = {}
		cls_.STOP = False	
		path = webRoot_ + '/RRD'
		if not os.path.exists(path):
			os.mkdir(path)
	
	# constructor calls handle method	
	def __init__(self, request, client_address, server):
		
		clientip = client_address[0]
		try:
			self.__graphs = MyTCPRequestHandler.Graphs[clientip]
		except:
			MyTCPRequestHandler.Graphs[clientip] = {}
			self.__graphs = MyTCPRequestHandler.Graphs[clientip]
		
		self.__buffers = {}
		self.__lastUpdate = time.time()
		self.__lastRecPacket = time.time()
		
		SocketServer.BaseRequestHandler.__init__(self, request, client_address, server)
				
	# create rrd archive name, rrd archives, graph names
	def newgraph(self, varnames_):

		# concat all var names into one name
		name = '';
		varnames = []
		for varname in varnames_:
			name += varname
			varnames.append(varname)
		
		# rd archives and graph names
		rrdb_short = self.__folder + '/' + '%s_short.rrd' % name
		rrdb_long = self.__folder + '/' + '%s_long.rrd' % name
		graphfile_short = self.__folder + '/' + '%s_short.png' % name
		graphfile_long = self.__folder + '/' + '%s_long.png' % name
			
		data_sources = []
		for varname in varnames_:
			data_sources.append("DS:%s:GAUGE:121:0:4294967295" % varname)

		rra_short = 'RRA:AVERAGE:0.5:1:60'
		rra_long = 'RRA:AVERAGE:0.5:1:1440'
				
		# create archives
		if not os.path.exists(rrdb_short):
			rrdtool.create(rrdb_short, '--start', '0', '--step', '60', data_sources, rra_short)
		if not os.path.exists(rrdb_long):
			rrdtool.create(rrdb_long, '--start', '0', '--step', '60', data_sources, rra_long)
		
		return (rrdb_short, graphfile_short, rrdb_long, graphfile_long, varnames)

	def updatehtml(self):
		
		# recreate html file
		htmlName = self.__folder + '/rrd.html' 
		html = open(htmlName, 'w')
		html.write('<head>\n')
		html.write('<meta http-equiv="refresh" content="10" > \n')
		html.write('</head>\n')
		html.write('<body>\n')
		html.write('<table>\n')
			
		for key in sorted(self.__graphs.keys ()):
			html.write(' <tr>\n')
			html.write("  <td><img src=\"%s_short.png\"/></td>\n" % str(key))
			html.write("  <td><img src=\"%s_long.png\"/></td>\n" % str(key))
			html.write(' </tr>\n')
	
		html.write('</table>\n')
		html.write('</body>\n')
		html.close()

	def adddata(self, dbname, valsums, valcounts, valtime):
				
		command = "rrdtool.update('%s', " % dbname

		command += "'%d"
		for i in range(len(valsums)):
			command += ":%d"

		command += "' % " + "(%s" % valtime
		
		for i in range(len(valsums)):	
			command += ", %d" % (valsums[i]/valcounts[i])

		command += "))"
		
		try:
			exec(command)
		except:
			print "!!! ERR: ", command

	def updatePlots(self):

		now = time.time()
		spanShort = 3600
		spanLong = 3600 * 24

		for key in self.__graphs:

			(rrdb_short, graphfile_short, rrdb_long, graphfile_long, varnames) =  self.__graphs[key]

			name = '';
			for varname in varnames:
				name += varname
			
			fileshortnew = graphfile_short + '.new'
			filelongnew = graphfile_long + '.new'

			command = "rrdtool.graph(fileshortnew, '--imgformat', 'PNG', '--width', '350', '--height', '120', "
			command += "'--start', '%d' % (now - spanShort), '--end', '%d' % now, '--vertical-label', 'hourly plot', "
			command += "'--title', '%s' % name, '--lower-limit', '0'"
			for varname in varnames:
				command += ", 'DEF:%s=%s:%s:AVERAGE' % " 
				command += "('%s', rrdb_short, '%s')" % (varname, varname)
			for varname in varnames:	
				command += ", 'AREA:%s%s:%s:STACK' % "
				variableColor = '#' + str(hex(abs(hash(str(varname) + 'abc')) % int('FFFFFF', 16))).lstrip('0x') 
				command += "('%s', '%s', '%s') " % (varname, variableColor, varname)
			command += ")"
		
			try:
				exec(command)
			except:
				print "!!! ERR: ", command

			shutil.move(fileshortnew, graphfile_short)
			
			command = "rrdtool.graph(filelongnew, '--imgformat', 'PNG', '--width', '1200', '--height', '120', "
			command += "'--start', '%d' % (now - spanLong), '--end', '%d' % now, '--vertical-label', 'daily plot', "
			command += "'--title', '%s' % name, '--lower-limit', '0'"
			for varname in varnames:
				command += ", 'DEF:%s=%s:%s:AVERAGE' % " 
				command += "('%s', rrdb_short, '%s')" % (varname, varname)
			for varname in varnames:	
				command += ", 'AREA:%s%s:%s:STACK' % "
				variableColor = '#' + str(hex(abs(hash(str(varname) + 'abc')) % int('FFFFFF', 16))).lstrip('0x') 
				command += "('%s', '%s', '%s') " % (varname, variableColor, varname)
			command += ")"

			try:
				exec(command)
			except:
				print "!!! ERR: ", command

			shutil.move(filelongnew, graphfile_long)

	def logdata(self, varnames_, varalues_, time_):
				
		(rrdb_short, graphfile_short, rrdb_long, graphfile_long) = ('', '', '', '')
		(valsums, valcounts, valtime) = ([], [], 0)

		fullname = '';
		for varname in varnames_:
			fullname += varname 

		try:
			(rrdb_short, graphfile_short, rrdb_long, graphfile_long, varnms) = self.__graphs[fullname]
			(valsums, valcounts, valtime) = self.__buffers[fullname]
			
			if time_ > valtime and len(valsums) > 0:
				
				self.adddata(rrdb_short, valsums, valcounts, valtime)
				self.adddata(rrdb_long, valsums, valcounts, valtime)
				valsums, valcounts = [], []
				for i in range(len(varalues_)):
					valsums.append(0)
					valcounts.append(0)

		except:
			valsums, valcounts = [], []
			for i in range(len(varalues_)):
				valsums.append(0)
				valcounts.append(0)

			self.__graphs[fullname] = self.newgraph(varnames_)
			self.updatehtml()
		
		valtime = time_
		for i in range(len(varalues_)):
			valsums[i] += varalues_[i]
			valcounts[i] += 1

		self.__buffers[fullname] = (valsums, valcounts, valtime)
					
		if  time.time() - self.__lastUpdate >= 5:
			self.updatePlots()
			self.__lastUpdate = time.time()	

	def handle(self):
		
		# make sure client is trusworthy
		self.data = self.request.recv(17).strip()
		if self.data != 'my name is socket':
			return
		
		# this cannot be done in init
		path = MyTCPRequestHandler.WebRoot + '/RRD/' + string.replace(str(self.client_address[0]), '.', '-') 
		if not os.path.exists(path):
			os.mkdir(path)

		self.__folder = path

		prevFragment = ''
		while True and not MyTCPRequestHandler.STOP:

			try:
				self.data = self.request.recv(1500).strip('\0')
				
				if self.data:
					
					self.data = prevFragment + self.data	
					prevFragment = ''
					
					if self.data[len(self.data) - 1] != chr(0x0a):
						
						indx = self.data.rfind('\n') 
						if indx != -1:
							prevFragment = 	self.data[indx + 1:]
							self.data = self.data[:indx + 1]
								
					self.__lastRecPacket = time.time()
					items = string.split(self.data, '\n')
					
					for item in [x for x in items if len(x) > 0]:
						
						item = item.strip('\x00')
						dataItems = string.split(item, ' ')
						
						if len(dataItems) > 2:

							varnames = []
							varvalues = []

							for i in range((len(dataItems) - 1) // 2):
								varnames.append(dataItems[2 * i])
								varvalues.append(int(dataItems[2 * i + 1]))

							self.logdata(varnames, varvalues, int(dataItems[len(dataItems) - 1]))
																		
				else:
					if time.time() - self.__lastRecPacket > 900:
						print "breaking .." 
						break
					time.sleep(3)
				
			except:
				# print exception details
				print '\n\n .. exception thrown '
				traceback.print_exc() 	
				traceback.print_stack() 

	def finish(self):
		print "closing .."
		return SocketServer.BaseRequestHandler.finish(self)

	def setup(self):
		return SocketServer.BaseRequestHandler.setup(self)


class ThreadedTCPServer(SocketServer.ThreadingMixIn, SocketServer.TCPServer):
	pass
	
if __name__ == "__main__":

	webRoot = '/var/www'

	if len(sys.argv) > 1:
		webRoot  = sys.argv[1]
	
	MyTCPRequestHandler.SetWebRoot(webRoot)

	HOST, PORT = "localhost", 4444

	try:
		# Create the server, binding to localhost on port 4444
		server = ThreadedTCPServer((HOST, PORT), MyTCPRequestHandler)
		server.allow_reuse_address = True

		# default attributes: just so we know	
		server.timeout = 300
		server. max_children = 40

		# Start a thread with the server --
		# that thread will then start one more thread for each request
		server_thread = threading.Thread(target=server.serve_forever)
		server_thread.start()
	except:
		print "Socket not released yet .. Try again later."

