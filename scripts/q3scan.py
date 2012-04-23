#!/usr/bin/python
# Parse quakestat output for player names in q3watch.lst, ignore names in q3ignore.lst,
# output the player name and server IP:port
#
import tempfile
import os
import time
import sys

os.chdir(sys.path[0])

QUAKESTAT = "quakestat"
WATCHFILE = "q3watch.lst"
IGNOREFILE = "q3ignore.lst"

def checkIgnore(playername):
	for name in ignore:
		if name in playername:
			return True
	return False

def checkWatch(playername):
	for name in watch:
		if name in playername: return True
	return False

startTime = time.time()

(tempfd, tempfilename) = tempfile.mkstemp(prefix='quakestat')
raw = os.fdopen(tempfd)
os.system(QUAKESTAT + ' -P -q3m master.urbanterror.info > ' + tempfilename)
#raw = open("/tmp/players")

watch = open(WATCHFILE).read().splitlines()
ignore = open(IGNOREFILE).read().splitlines()
rawlist = raw.read().splitlines()

servers = {}
current = None

for line in rawlist[2:]:
	if line.startswith("Q3S"):
		servinfo = line.split()
		current = servinfo[1]
		servers[current] = []
	else:
		playerinfo = line.split()
		if len(playerinfo) != 4: continue
		servers[current].append(playerinfo[3])


playerlist = {}

for server in servers:
	for player in servers[server]:
		if checkWatch(player) and not checkIgnore(player):
			if player not in playerlist: playerlist[player] = [server]
			else:
				if server not in playerlist[player]:
					playerlist[player].append(server)

# Output here, woop.

totalTime = time.time() - startTime
#print 'Search took', totalTime, 'seconds.'
for player in playerlist:
	print player, playerlist[player]

raw.close()
os.unlink(tempfilename)
