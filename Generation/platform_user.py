import xml.etree.ElementTree as xml
import xml.dom.minidom as xmldom

IoT_Clusters = int(input("Enter number of IoT clusters:  "))
Datacenters = int(input("Enter number of Datacenters:  "))
Fogs = int(input("Enter number of Fog clusters:  "))



filename = "../platform_user.xml"
platform = xml.Element("platform")
platform.set('version','4.1')

AS = xml.SubElement(platform, 'AS')
AS.set('id','AS0')
AS.set('routing','Full')

dev = []
ser = []

for i in range(IoT_Clusters):
	id = "IoT"+str(i)

	print("")
	print("----------------------------------------------")
	print("-------------------IOT CLUSTER "+str(i)+"--------------")
	print("----------------------------------------------")
	print("")

	devices = input("Number of devices: ")
	dev.append(int(devices))
	speed = input("Speed: ")
	core = input("Cores: ")
	bw = input("Bandwidth: ")
	latency = input("Latency: ")
	bb_bw = input("Backbone bandwidth: ")
	bb_lat = input("Backbone latency: ")
	wp = input("Wattage per state (A:B:C): ")
	woff = input("Wattage off: ")

	cluster = xml.SubElement(AS, 'cluster')
	cluster.set('id',id)
	prefix = "iot-"+str(i)+"-"
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
	cluster.set('router_id', "IoT_cluster"+str(i))

	propIoT = xml.SubElement(cluster, 'prop')
	propIoT.set('id', 'wattage_per_state') 
	propIoT.set('value', wp)
	
	propIoT = xml.SubElement(cluster, 'prop')
	propIoT.set('id', 'wattage_off') 
	propIoT.set('value', woff)



for i in range(Datacenters):
	id = "Dispatcher"+str(i)

	print("")
	print("----------------------------------------------")
	print("----------------DISPATCHER "+str(i)+"------------------")
	print("----------------------------------------------")
	print("")
	speed = input("Speed: ")
	core = input("Cores: ")
	bw = input("Bandwidth: ")
	latency = input("Latency: ")
	bb_bw = input("Backbone bandwidth: ")
	bb_lat = input("Backbone latency: ")
	wp = input("Wattage per state (A:B:C): ")
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
	cluster.set('router_id', "Dispatcher_cluster"+str(i))

	propIoT = xml.SubElement(cluster, 'prop')
	propIoT.set('id', 'wattage_per_state') 
	propIoT.set('value', wp)
	
	propIoT = xml.SubElement(cluster, 'prop')
	propIoT.set('id', 'wattage_off') 
	propIoT.set('value', woff)


for i in range(Datacenters):
	id = "Datacenter"+str(i)
	
	print("")
	print("----------------------------------------------")
	print("----------------DATACENTER "+str(i)+"------------------")
	print("----------------------------------------------")
	print("")
	servers = input("Number of servers: ")
	ser.append(servers)
	speed = input("Speed: ")
	core = input("Cores: ")
	bw = input("Bandwidth: ")
	latency = input("Latency: ")
	bb_bw = input("Backbone bandwidth: ")
	bb_lat = input("Backbone latency: ")
	wp = input("Wattage per state (A:B:C): ")
	woff = input("Wattage off: ")

	cluster = xml.SubElement(AS, 'cluster')
	cluster.set('id', id)
	prefix = "s-"+str(i)+"-"
	cluster.set('prefix', prefix)
	cluster.set('suffix', "")
	cluster.set('radical', '0-'+str(int(servers)-1))
	cluster.set('speed', speed)
	cluster.set('core', core)
	cluster.set('bw', bw)
	cluster.set('lat', latency)
	cluster.set('bb_bw', bb_bw)
	cluster.set('bb_lat', bb_lat)
	cluster.set('router_id', "Datacenter_cluster"+str(i))

	propIoT = xml.SubElement(cluster, 'prop')
	propIoT.set('id', 'wattage_per_state') 
	propIoT.set('value', wp)
	
	propIoT = xml.SubElement(cluster, 'prop')
	propIoT.set('id', 'wattage_off') 
	propIoT.set('value', woff)



""" CONTROLLER """

controller = xml.SubElement(AS, 'cluster')
controller.set('id', 'Controller')
prefix = "cont-"
controller.set('prefix', prefix)
controller.set('suffix', "")
controller.set('radical', '0-1')
controller.set('speed', '4Gf')
controller.set('bw', "125GBps")
controller.set('lat', '0us')
controller.set('bb_bw', '10GBps')
controller.set('bb_lat', '10us')
controller.set('router_id', "Controller_cluster")


""" CONTROLLER """




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







""" LINKS BETWEEN CONTROLLER AND DISPATCHERS """


bandwidthLink = '10GBps'
latencyLink = '50us'

for i in range(Datacenters):
	link = xml.SubElement(AS, 'link')
	id = "linkController" + "Dispatcher" + str(i) 
	link.set('id', id)
	link.set('bandwidth', bandwidthLink)
	link.set('latency', latencyLink)



""" LINKS BETWEEN IOT AND CONTROLLER """


for i in range(IoT_Clusters):
	link = xml.SubElement(AS, 'link')
	id = "linkIoT" + str(i) + "Controller" 
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
		ASroute.set('gw_src', 'IoT_cluster' + str(j))
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




for i in range(IoT_Clusters):
	for j in range(Datacenters):
		ASroute = xml.SubElement(AS, 'ASroute')
		ASroute.set('src', 'Datacenter' + str(j))
		ASroute.set('dst', 'IoT' + str(i))
		ASroute.set('gw_src', 'Datacenter_cluster' + str(j))
		ASroute.set('gw_dst', 'IoT_cluster' + str(i))

		link_ctn = xml.SubElement(ASroute, 'link_ctn')
		link_ctn.set('id', 'linkDatacenter' + str(j) + 'IoT' + str(i))






for i in range(IoT_Clusters):

	""" ASRoute BETWEEN IOT AND CONTROLLER """

	ASroute = xml.SubElement(AS, 'ASroute')
	ASroute.set('src', 'IoT' + str(i))
	ASroute.set('dst', 'Controller')
	ASroute.set('gw_src', 'IoT_cluster' + str(i))
	ASroute.set('gw_dst', 'Controller_cluster')

	link_ctn = xml.SubElement(ASroute, 'link_ctn')
	link_ctn.set('id', 'linkIoT' + str(i) + 'Controller')








for i in range(Datacenters):

	""" ASRoute BETWEEN CONTROLLER AND DATACENTERS """
	ASroute = xml.SubElement(AS, 'ASroute')
	ASroute.set('src', 'Controller')
	ASroute.set('dst', 'Dispatcher' + str(i))
	ASroute.set('gw_src', 'Controller_cluster')
	ASroute.set('gw_dst', 'Dispatcher_cluster' + str(i))

	link_ctn = xml.SubElement(ASroute, 'link_ctn')
	link_ctn.set('id', 'linkController' + 'Dispatcher' + str(i))






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
arrival_rate = []
print("")


for i in range(Datacenters):
	rs = input("Insert size of responses to receive from Datacenter" + str(i) + ": ")
	sizeResponse.append(rs)

print("")


for i in range(IoT_Clusters):
	t = input("Insert number of tasks to create on IoT" + str(i) + ": ")
	tasks.append(t)
	r = input("Insert size of requests to create on IoT" + str(i) + ": ")
	sizeRequest.append(r)
	p = input("Insert percentage of tasks to execute on IoT" + str(i) + " (between 0 and 1): ")
	percentage.append(p)
	ar = input("Insert arrival rate on IoT" + str(i) + " (between 0 and 1): ")
	arrival_rate.append(ar)
	print("")

print("----------------------------------------------")
print ("The commands are:")
print("")
command = "./generation_main.sh " + str(Datacenters)


for i in range(Datacenters):
	command = command + " " + ser[i] + " " + sizeResponse[i]


command = command + " " + str(IoT_Clusters) + " "


for i in range(IoT_Clusters):
	command = command + str(dev[i]) + " " + str(tasks[i]) + " " + str(sizeRequest[i]) + " " + str(percentage[i]) + " " + str(arrival_rate[i]) + " "

print (command)
print("make")
print("./main platform-cluster.xml")
print("----------------------------------------------")

