import xml.etree.ElementTree as xml
import xml.dom.minidom as xmldom

IoT_Clusters = int(input("Enter number of IoT clusters:  "))
Datacenters = int(input("Enter number of Datacenters:  "))
Fogs = int(input("Enter number of Fog clusters:  "))



filename = "./platform_user.xml"
platform = xml.Element("platform")
platform.set('version','4.1')

AS = xml.SubElement(platform, 'AS')
AS.set('id','AS0')
AS.set('routing','Full')

for i in range(IoT_Clusters):
	id = "IoT"+str(i)

	print("")
	print("----------------------------------------------")
	print("------------IOT CLUSTER "+str(i)+"------------")
	print("")

	devices = input("Number of devices: ")
	speed = input("Speed (A,B,C): ")
	core = input("Cores: ")
	bw = input("Bandwidth: ")
	latency = input("Latency: ")
	bb_bw = input("Backbone bandwidth: ")
	bb_lat = input("Backbone latency: ")
	wp = input("Wattage per state (A:B:C, D:E:F, G:H:I): ")
	woff = input("Wattage off: ")

	cluster = xml.SubElement(AS, 'cluster')
	cluster.set('id',id)
	prefix = "c-"+str(i)+"-"
	cluster.set('prefix', prefix)
	cluster.set('suffix', "")
	radical = int(devices)-1
	cluster.set('radical', '0-'+str(radical))
	cluster.set('speed', speed)
	cluster.set('core', core)
	cluster.set('bw', bw)
	cluster.set('lat', latency)
	cluster.set('bb_bw', bb_bw)
	cluster.set('bb_lat', bb_lat)
	cluster.set('router_id', "IoT_Cluster"+str(i))

	propIoT = xml.SubElement(cluster, 'prop')
	propIoT.set('id', 'watt_per_state') 
	propIoT.set('value', wp)
	
	propIoT = xml.SubElement(cluster, 'prop')
	propIoT.set('id', 'wattage_off') 
	propIoT.set('value', woff)



for i in range(Datacenters):
	id = "Dispatcher"+str(i)

	print("")
	print("----------------------------------------------")
	print("------------DISPATCHER "+str(i)+"-------------")
	print("")
	speed = input("Speed (A,B,C): ")
	core = input("Cores: ")
	bw = input("Bandwidth: ")
	latency = input("Latency: ")
	bb_bw = input("Backbone bandwidth: ")
	bb_lat = input("Backbone latency: ")
	wp = input("Wattage per state (A:B:C, D:E:F, G:H:I): ")
	woff = input("Wattage off: ")

	cluster = xml.SubElement(AS, 'cluster')
	cluster.set('id', id)
	prefix = "d-"+str(i)+"-"
	cluster.set('prefix', prefix)
	cluster.set('suffix', "")
	cluster.set('radical', '0-'+str(1))
	cluster.set('speed', speed)
	cluster.set('core', core)
	cluster.set('bw', bw)
	cluster.set('lat', latency)
	cluster.set('bb_bw', bb_bw)
	cluster.set('bb_lat', bb_lat)
	cluster.set('router_id', "IoT_Cluster"+str(i))

	propIoT = xml.SubElement(cluster, 'prop')
	propIoT.set('id', 'watt_per_state') 
	propIoT.set('value', wp)
	
	propIoT = xml.SubElement(cluster, 'prop')
	propIoT.set('id', 'wattage_off') 
	propIoT.set('value', woff)


for i in range(Datacenters):
	id = "Datacenter"+str(i)
	
	print("")
	print("----------------------------------------------")
	print("------------DATACENTER "+str(i)+"-------------")
	print("")
	servers = input("Number of servers: ")
	speed = input("Speed (A,B,C): ")
	core = input("Cores: ")
	bw = input("Bandwidth: ")
	latency = input("Latency: ")
	bb_bw = input("Backbone bandwidth: ")
	bb_lat = input("Backbone latency: ")
	wp = input("Wattage per state (A:B:C, D:E:F, G:H:I): ")
	woff = input("Wattage off: ")

	cluster = xml.SubElement(AS, 'cluster')
	cluster.set('id', id)
	prefix = "d-"+str(i)+"-"
	cluster.set('prefix', prefix)
	cluster.set('suffix', "")
	cluster.set('radical', '0-'+str(int(servers)-1))
	cluster.set('speed', speed)
	cluster.set('core', core)
	cluster.set('bw', bw)
	cluster.set('lat', latency)
	cluster.set('bb_bw', bb_bw)
	cluster.set('bb_lat', bb_lat)
	cluster.set('router_id', "IoT_Cluster"+str(i))

	propIoT = xml.SubElement(cluster, 'prop')
	propIoT.set('id', 'watt_per_state') 
	propIoT.set('value', wp)
	
	propIoT = xml.SubElement(cluster, 'prop')
	propIoT.set('id', 'wattage_off') 
	propIoT.set('value', woff)




print("")
print("-----------------------------------------------")
print("---Links between IoT clusters and Dispatchers--")
print("-----------------------------------------------")
print("")

bandwidthLink = input ("Enter link bandwidth: ")
latencyLink = input ("Enter link latency: ")

for i in range(Datacenters):
	for j in range(IoT_Clusters):
		link = xml.SubElement(AS, 'link')
		id = "linkIoT" + str(j) + "Dispatcher" + str(i)
		link.set('id', id)
		link.set('bandwidth', bandwidthLink)
		link.set('latency', latencyLink)



print("")
print("-----------------------------------------------")
print("---Links between Dispatchers and Datacenters---")
print("-----------------------------------------------")
print("")

bandwidthLink = input ("Enter link bandwidth: ")
latencyLink = input ("Enter link latency: ")

for i in range(Datacenters):
	link = xml.SubElement(AS, 'link')
	id = "linkDispatcher" + str(i) + "Datacenter" + str(i)
	link.set('id', id)
	link.set('bandwidth', bandwidthLink)
	link.set('latency', latencyLink)




print("")
print("----------------------------------------------")
print("--Links between Datacenters and IoT Clusters--")
print("----------------------------------------------")
print("")

bandwidthLink = input ("Enter link bandwidth: ")
latencyLink = input ("Enter link latency: ")

for i in range(Datacenters):
	for j in range(IoT_Clusters):
		link = xml.SubElement(AS, 'link')
		id = "linkDatacenter" + str(i) + "IoT" + str(j)
		link.set('id', id)
		link.set('bandwidth', bandwidthLink)
		link.set('latency', latencyLink)



print("----------------------------------------------")
print("--------CREATING PLATFORM_CLUSTER.XML---------")
print("----------------------------------------------")



for i in range(Datacenters):
	for j in range(IoT_Clusters):
		ASroute = xml.SubElement(AS, 'ASroute')
		ASroute.set('src', 'IoT' + str(j))
		ASroute.set('dst', 'Dispatcher' + str(i))
		ASroute.set('gw_src', 'Iot_cluster' + str(j))
		ASroute.set('gw_dst', 'Dispatcher_cluster' + str(i))

		link_ctn = xml.SubElement(ASroute, 'link_ctn')
		link_ctn.set('id', 'linkIoT' + str(j) + 'Dispatcher' + str(i))



for i in range(Datacenters):
	
	ASroute = xml.SubElement(AS, 'ASroute')
	ASroute.set('src', 'Dispatcher' + str(i))
	ASroute.set('dst', 'Datacenter' + str(i))
	ASroute.set('gw_src', 'Dispatcher_cluster' + str(i))
	ASroute.set('gw_dst', 'Datacenter_cluster' + str(i))

	link_ctn = xml.SubElement(ASroute, 'link_ctn')
	link_ctn.set('id', 'linkDispatcher' + str(i) + 'Datacenter' + str(i))




for i in range(Datacenters):
	for j in range(IoT_Clusters):
		ASroute = xml.SubElement(AS, 'ASroute')
		ASroute.set('src', 'Datacenter' + str(i))
		ASroute.set('dst', 'IoT' + str(j))
		ASroute.set('gw_src', 'Datacenter_cluster' + str(j))
		ASroute.set('gw_dst', 'IoT_cluster' + str(i))

		link_ctn = xml.SubElement(ASroute, 'link_ctn')
		link_ctn.set('id', 'linkDatacenter' + str(i) + 'IoT' + str(j))



tree = xml.ElementTree(platform)


with open(filename, "wb") as fh:
	fh.write("<?xml version='1.0'?><!DOCTYPE platform SYSTEM \"http://simgrid.gforge.inria.fr/simgrid/simgrid.dtd\">".encode('utf-8'))

with open(filename, "ab") as fh: 
   	tree.write(fh, encoding='utf-8')

dom = xmldom.parse(filename)
string = dom.toprettyxml()

f = open(filename, "w")
f.write(string)
f.close

print("----------------------------------------------")
print("---------PLATFORM_CLUSTER.XML CREATED---------")
print("----------------------------------------------")


tasks = []
sizeRequest = []
sizeResponse = []
percentage = []


for i in range(IoT_Clusters):
	t = input("Insert number of tasks to create on IoT" + str(i) + ": ")
	tasks.append(t)
	r = input("Insert size of requests to create on IoT" + str(i) + ": ")
	sizeRequest.append(r)
	rs = input("Insert size of responses to receive on IoT" + str(i) + " (between 0 and 1): ")
	sizeResponse.append(rs)
	p = input("Insert percentage of tasks to execute on IoT" + str(i) + " (between 0 and 1): ")
	percentage.append(p)
	print("")

arrival_rate = input("Insert arrival rate (between 0 and 1): ")

print("----------------------------------------------")
print ("The command is:")

command = "./simulation platform-cluster.xml " + arrival_rate + " " + str(IoT_Clusters) + " "

for i in range(IoT_Clusters):
	command = command + tasks[i] + " " + sizeRequest[i] + " " + sizeResponse[i] + " " + percentage[i]

print (command)
print("----------------------------------------------")

