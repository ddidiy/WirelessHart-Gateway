#! /usr/bin/python

import pycurl
import cStringIO
import os
import re
from time import time
import sys
from threading import Thread
import json

class testit(Thread):
	def __init__ (self,id):
		Thread.__init__(self)
		self.id = id
		self.status = -1
	def run(self):
		c = pycurl.Curl()
		b = cStringIO.StringIO()
		c.setopt(pycurl.URL, "http://10.32.0.25/rpc.cgi")
		c.setopt(pycurl.HTTPHEADER, ["Content-Type: text/plain"])
		c.setopt(pycurl.COOKIEFILE, 'cookies.txt')
		c.setopt(pycurl.WRITEFUNCTION, b.write)
		c.setopt(pycurl.FOLLOWLOCATION, 1)
		c.setopt(pycurl.MAXREDIRS, 5)
		c.setopt(pycurl.POST, 1)

		start = time()
		c.setopt(pycurl.POSTFIELDS, '{"id":"'+self.id+'", "method":"user.login", "params":{"user":"admin","pass":"adminadmin"}}')
		c.perform()
		print self.id+'user.login %6.2fms' % ( (time() - start) * 1000)
		a=b.getvalue()
		result  = json.loads( a )
		if not result['result'] or result['error']:
			print self.id + "ERROR >>> LoginFailed"
			return
		b.close()

		print self.id+'device.search'
		b = cStringIO.StringIO()
		c.setopt(pycurl.WRITEFUNCTION, b.write)
		c.setopt(pycurl.POSTFIELDS, '{"id":"'+self.id+'", "method":"misc.dummy", "params":{}}')
		start = time()
		c.perform()
		a=b.getvalue()
		print self.id+'device.search %6.2fms ' % ( (time() - start) * 1000) +a


pinglist = []

for host in range(0,13):
   current = testit(str(host)+" ")
   pinglist.append(current)
   current.start()

for pingle in pinglist:
   pingle.join()

