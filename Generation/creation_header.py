import csv, sys
import numpy as np


def main():

	with open(sys.argv[1]) as file:

		template = csv.reader(file, delimiter='|')

		rows = [r for r in template]
		line_count = len(rows)

		for line in rows:
			if "number_cloud" in line:
				num_datacenter = int(line[1])
			if "number_fog" in line:
			    num_fog = int(line[1])
			if "number_edge" in line:
			    num_edge = int(line[1])
			if "number_iot" in line:
			    num_iot = int(line[1])
			    break

		header = open(str(sys.argv[2]), "w+")

		header.write("#include <string.h>\n")
		header.write("#include <math.h>\n")
		header.write("#include <stdio.h>\n")
		header.write("#include <stdlib.h>\n")
		header.write('#include "simgrid/plugins/energy.h"\n')
		header.write('#include "simgrid/msg.h"\n')
		header.write('#include "xbt/synchro.h"\n\n')

		header.write('#define MAX_REQUESTS 100000\n\n')

		header.write('#define MAX_DATACENTERS ' + str(num_datacenter) + '\n')
		header.write('#define MAX_FOG ' + str(num_fog) + '\n')
		header.write('#define MAX_EDGE ' + str(num_edge) + '\n')
		header.write('#define MAX_IOT ' + str(num_iot) + '\n')

		nodes_cloud = []
		devices_fog = []
		nodes_edge = []
		iot = []

		for i in range(num_datacenter):
			comp_cl = "devices_cloud" + str(i)
			pos = 0

			for line in rows:
			    if comp_cl in line:
			        break
			    pos = pos + 1

			nodes_cloud.append(int(rows[pos][1]))

		for i in range(num_fog):
			comp_cl = "devices_fog" + str(i)
			pos = 0

			for line in rows:
			    if comp_cl in line:
			        break
			    pos = pos + 1

			devices_fog.append(int(rows[pos][1]))

		for i in range(num_edge):
			comp_cl = "devices_edge" + str(i)
			pos = 0

			for line in rows:
			    if comp_cl in line:
			        break
			    pos = pos + 1

			nodes_edge.append(int(rows[pos][1]))

		for i in range(num_iot):
			comp_cl = "devices_iot" + str(i)
			pos = 0

			for line in rows:
			    if comp_cl in line:
			        break
			    pos = pos + 1

			iot.append(int(rows[pos][1]))

		if num_datacenter != 0:	
			max_nodes_cloud = np.max(nodes_cloud)
			header.write('#define MAX_SERVERS ' + str(max_nodes_cloud) + '\n')
		if num_fog != 0:
			max_devices_fog = np.max(devices_fog)
			header.write('#define MAX_DEVICES_FOG ' + str(max_devices_fog) + '\n')
		if num_edge != 0:
			max_nodes_edge = np.max(nodes_edge)
			header.write('#define MAX_NODES_EDGE ' + str(max_nodes_edge) + '\n')
		if num_iot != 0:
			max_nodes_iot = np.max(iot)
			header.write('#define MAX_DEVICES_IOT ' + str(max_nodes_iot) + '\n\n\n')


		header.write('struct TaskClouds\n{\n\t')
		header.write('xbt_dynar_t client_requests[MAX_REQUESTS];\n\t')
		header.write('xbt_mutex_t mutex[MAX_SERVERS];\n\t')
		header.write('xbt_cond_t cond[MAX_SERVERS];\n\t')
		header.write('int EmptyQueue[MAX_SERVERS];\n\t')
		header.write('int Nqueue[MAX_SERVERS];\n\t')
		header.write('double Navgqueue[MAX_SERVERS];\n\t')
		header.write('int Nsystem[MAX_SERVERS];\n\t')
		header.write('double Navgsystem[MAX_SERVERS];\n};\n\n')
		header.write('struct TaskClouds tasksManagement[MAX_DATACENTERS];\n\n')

		header.write('struct ClientRequest\n{\n\t')
		header.write('char id_task[40];\n\t')
		header.write('int n_task;\n\t')
		header.write('double t_arrival;\n\t')
		header.write('double t_service;\n\t')
		header.write('int iot_cluster;\n\t')
		header.write('int device;\n\t')
		header.write('double edge_usage;\n\t')
		header.write('double fog_usage;\n\t')
		header.write('int size_task;\n};\n\n')

		header.write('struct ServerResponse\n{\n\t')
		header.write('char id_task[40];\n\t')
		header.write('char response[40];\n\t')
		header.write('int server_cluster;\n\t')
		header.write('int server;\n\t')
		header.write('int iot_cluster;\n\t')
		header.write('int device;\n\t')
		header.write('double t_arrival;\n};\n\n')

		header.write('struct ControllerResponse\n{\n\t')
		header.write('int iot_cluster;\n\t')
		header.write('int device;\n\t')
		header.write('int finish;\n};\n\n')

		if num_datacenter != 0:
			header.write('struct StatisticsCloud\n{\n\t')
			header.write('double totalEnergy[MAX_SERVERS];\n\t')
			header.write('double avEnergy[MAX_SERVERS];\n\t')
			header.write('int numTasks[MAX_SERVERS];\n\t')
			header.write('double executionTime[MAX_SERVERS];\n\t')
			header.write('double dataProcessed[MAX_SERVERS];\n};\n\n')
			header.write('struct StatisticsCloud statsCloud[MAX_DATACENTERS];\n\n')

		if num_fog != 0:
			header.write('struct StatisticsFog\n{\n\t')
			header.write('double totalEnergy[MAX_DEVICES_FOG];\n\t')
			header.write('double avEnergy[MAX_DEVICES_FOG];\n\t')
			header.write('int numTasks[MAX_DEVICES_FOG];\n\t')
			header.write('double executionTime[MAX_DEVICES_FOG];\n\t')
			header.write('double dataProcessed[MAX_DEVICES_FOG];\n};\n\n')
			header.write('struct StatisticsFog statsFog[MAX_FOG];\n\n')

		if num_edge != 0:
			header.write('struct StatisticsEdge\n{\n\t')
			header.write('double totalEnergy[MAX_EDGE_NODE];\n\t')
			header.write('double avEnergy[MAX_EDGE_NODE];\n\t')
			header.write('int numTasks[MAX_EDGE_NODE];\n\t')
			header.write('double executionTime[MAX_EDGE_NODE];\n\t')
			header.write('double dataProcessed[MAX_EDGE_NODE];\n};\n\n')
			header.write('struct StatisticsEdge statsEdge[MAX_EDGE];\n\n')

		if num_iot != 0:
			header.write('struct StatisticsIot\n{\n\t')
			header.write('double totalEnergy[MAX_DEVICES_IOT];\n\t')
			header.write('double avEnergy[MAX_DEVICES_IOT];\n\t')
			header.write('int numTasks[MAX_DEVICES_IOT];\n\t')
			header.write('double avTime[MAX_DEVICES_IOT];\n};\n\n')
			header.write('struct StatisticsIot statsIot[MAX_IOT];\n\n')

		if num_datacenter != 0:
			for i in range(num_datacenter):
				header.write('int cloud' + str(i) + '(int argc, char *argv[]);\n')
				header.write('int dispatcherCloud' + str(i) + '(int argc, char *argv[]);\n')
		if num_fog != 0:
			for i in range(num_fog):
				header.write('int fog' + str(i) + '(int argc, char *argv[]);\n')
		if num_edge != 0:
			for i in range(num_edge):
				header.write('int edge' + str(i) + '(int argc, char *argv[]);\n')
		if num_iot != 0:
			for i in range(num_iot):
				header.write('int iot' + str(i) + '(int argc, char *argv[]);\n')


		header.write('int controller(int argc, char *argv[]);\n')
		header.write('void test_all(char *file);\n')

		header.close()


if __name__ == "__main__":
	main()