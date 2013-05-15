
import re
import string


def detectCycle(testAddress, neigh, nodes, tested):
    if neigh in tested and len(tested) > 0:
        #print('cycle')
        tested.append(neigh)
        return True
    if neigh == '1':
        #print('tr')
        return False
    result = False
    tested.append(neigh)
    for node in nodes[neigh]:
        result = result or detectCycle(testAddress, node, nodes, tested)
    return result

f = open("g:\\work\\logs\\temp\\operations.txt")
start_time = '03-22 20:17:02' 
stop_time = '03-23 08:31:09'
start = False
nodes = {}
for line in f:
       
    if re.search('..-.. ..:..:..',line):
        time = re.match('..-.. ..:..:..', line).group(0)

        if time == start_time:
            start = True
        if time == stop_time:
            start = False
            
        
        
    
    if re.search("Set Clock Source", line) is not None and start:
        
        if re.search("state= G", line) is None:
            continue
        
        l = line
        Idx = line.find('own=')
        if Idx > -1:
            line = line[Idx + 4:]
            items = string.split(line, ',')
            own = items[0].lstrip().rstrip()

        Idx = line.find('ngh:')
        if Idx > -1:
            line = line[Idx + 4:]
            items = string.split(line, ',')
            neighborAddress = items[0].lstrip().rstrip()
            
        Idx = line.find('flags:')
        if Idx > -1:
            line = line[Idx + 6:]
            items = string.split(line, ',')
            flag = items[0].lstrip().rstrip()
            

                                
        if own in nodes:
            
            if int(flag) == 1:
                nodes[own].add(neighborAddress)
#                if own == '28':
#                    print ('Add: ' + own + '<-' + neighborAddress)
#                    print(nodes[own])
            if int(flag) == 0 and neighborAddress in nodes[own]:
                nodes[own].remove(neighborAddress)
#                if own == '28':
#                    print ('Remove: ' + own + '<-' + neighborAddress)
#                    print(nodes[own])
                
        else:            
            if int(flag) == 1:
                nodes[own] = set([neighborAddress])
#                if own == '28':
#                    print ('Add: ' + own + '<-' + neighborAddress)
#                    print(nodes[own])


#print(edges)
for nodeKey in nodes:    
    for neighborKey in nodes[nodeKey]:
        print(nodeKey + "<-" + neighborKey)

for node in nodes:
    tested =[] 
    cycle = detectCycle(node, node, nodes, tested)
    print ('Node=' + node + ': ' + str(cycle))
    if cycle:
        print(tested)
f.close()