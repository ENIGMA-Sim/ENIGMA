import csv, sys


def main():
    with open(sys.argv[1]) as file, open(sys.argv[2]) as appfile:

        template = csv.reader(file, delimiter='|')
        templateApp = csv.reader(appfile, delimiter='|')

        rows = [r for r in template]
        rowsApp = [i for i in templateApp]

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

        total_iot = 0
        total_fog = 0
        total_edge = 0
        total_cloud = 0

        sim = open(str(sys.argv[3]), "w+")

        sim.write('#include "./simulation.h"\n\n')
        sim.write('#include "./rand.h"\n\n')

        for i in range(num_datacenter):
            devices = "devices_cloud" + str(i)
            pos = 0

            for line in rows:
                if devices in line:
                    break
                pos = pos + 1
            sim.write('#define NODESDATACENTER' + str(i) + " " + str(rows[pos][1]) + "\n")
            total_cloud += int(rows[pos][1])

        for i in range(num_fog):
            devices = "devices_fog" + str(i)
            pos = 0

            for line in rows:
                if devices in line:
                    break
                pos = pos + 1
            sim.write('#define FOG' + str(i) + " " + str(rows[pos][1]) + "\n")
            total_fog += int(rows[pos][1])

        for i in range(num_edge):
            devices = "devices_edge" + str(i)
            pos = 0

            for line in rows:
                if devices in line:
                    break
                pos = pos + 1
            sim.write('#define EDGE' + str(i) + " " + str(rows[pos][1]) + "\n")
            total_edge += int(rows[pos][1])

        for i in range(num_iot):
            devices = "devices_iot" + str(i)
            pos = 0

            for line in rows:
                if devices in line:
                    break
                pos = pos + 1
            sim.write('#define IOT' + str(i) + " " + str(rows[pos][1]) + "\n")
            total_iot += int(rows[pos][1])

        sim.write('\n\nint init = 0;\n')
        sim.write('xbt_mutex_t m;\n')
        sim.write('xbt_cond_t con;\n')

        sim.write('int iot_finished = 0;\n')
        sim.write('int edge_finished = 0;\n')
        sim.write('int fog_finished = 0;\n')

        sim.write('\n\n')

        list_iot = []
        for i in range(num_iot):
            length_table = len(rowsApp)
            iot_string = 'iot-' + str(i)
            for iot_row in range(length_table):
                if iot_string in rowsApp[iot_row][0]:
                    list_iot.append(iot_row)


        for i in range(num_iot):
            sim.write('int iot' + str(i) + '(int argc, char *argv[])\n{\n')
            sim.write('\tdouble task_comp_size = 0, task_comm_size = 0, t_arrival, t, percentage_edge, percentage_fog, flops;\n')
            sim.write('\tint my_iot_cluster, my_device, tasks, size, num_edge, num_fog, num_nodes, res;\n')
            sim.write('\tchar sprintf_buffer[40], mailbox[256], buf[64];\n')
            sim.write('\tmsg_task_t task = NULL, ans_data_task = NULL, fin_task = NULL;\n')
            sim.write('\tstruct ServerResponse *resServer;\n')
            sim.write('\tstruct ControllerResponse *con;\n\n')

            sim.write('\tmy_iot_cluster = atoi(argv[0]);\n')
            sim.write('\tmy_device = atoi(argv[1]);\n')

            devices = "IoT" + str(i)
            pos = 0

            for line in rowsApp:
                if devices in line:
                    break
                pos = pos + 1
                
            sim.write('\ttasks = ' + str(rowsApp[pos][1]) + ';\n\n')

            sim.write('\tsprintf(buf, "iot-%d-%d", my_iot_cluster, my_device);\n')
            sim.write('\tMSG_mailbox_set_async(buf);\n')
            sim.write('\tmsg_host_t host = MSG_host_by_name(buf);\n')
            sim.write('\tMSG_host_set_pstate(host, 2);\n\n')
            sim.write('\tfor(int i = 0; i < tasks; i++)\n\t{\n')


            percentageEdge = "edge_percentage_iot" + str(i)
            pos = 0

            for line in rows:
                if percentageEdge in line:
                    break
                pos = pos + 1
            perEdge = str(rows[pos][1])
            perFog = str(rows[pos+1][1])

            for j in range(len(list_iot)):
                if 'iot-' + str(i) + '-' + str(j) in rowsApp[list_iot[0]][0]:
                    valueiot = list_iot.pop(0)
                    output = rowsApp[valueiot][5]
                    stream_output = 0

                    if 'GB/s' in output:
                        result = output.split("GB/s", 1)[0]
                        stream_output = int(result) * 1024 * 1024

                    if 'MB/s' in output:
                        result = output.split("MB/s", 1)[0]
                        stream_output = int(result) * 1024

                    if 'KB/s' in output:
                        result = output.split("KB/s", 1)[0]
                        stream_output = int(result)

                    list_dest = rowsApp[valueiot][6].split(";")

                    value_output = []
                    device_dest = []
                    total_stream = 0

                    for k in range(len(list_dest)):
                        device_dest.append(list_dest[k].split(":", 1)[1])

                        if 'GB/s' in list_dest[k]:
                            insert = int(list_dest[k].split("GB/s", 1)[0]) * 1024 * 1024
                            value_output.append(insert)
                            total_stream = total_stream + int(insert)

                        if 'MB/s' in list_dest[k]:
                            insert = int(list_dest[k].split("MB/s", 1)[0]) * 1024
                            value_output.append(insert)
                            total_stream = total_stream + int(insert)

                        if 'KB/s' in list_dest[k]:
                            insert = int(list_dest[k].split("KB/s", 1)[0])
                            value_output.append(insert)
                            total_stream = total_stream + int(insert)

                    if total_stream != stream_output:
                        print('Error: The output value does not match the output stream')
                        print(str(total_stream) + " " + str(stream_output))
                        exit(-1)

                    sim.write('\t\tif (strcmp(buf, "' + rowsApp[valueiot][0] + '") == 0)\n\t\t{\n')
                    sim.write('\t\t\tMSG_host_set_pstate(host, 0);\n\n')

                    for k in range(len(list_dest)):
                        sim.write('\t\t\tstruct ClientRequest *req' + str(k) + ';\n')
                        sim.write('\t\t\treq' + str(k) + ' = (struct ClientRequest *)malloc(sizeof(struct ClientRequest));\n\n')
                        sim.write('\t\t\tsprintf(sprintf_buffer, "Data");\n')
                        sim.write('\t\t\tsprintf(req' + str(k) + '->id_task, "%s", sprintf_buffer);\n')
                        sim.write('\t\t\treq' + str(k) + '->device = my_device;\n')
                        sim.write('\t\t\treq' + str(k) + '->edge_usage = ' + perEdge + ';\n')
                        sim.write('\t\t\treq' + str(k) + '->fog_usage = ' + perFog + ';\n')
                        sim.write('\t\t\treq' + str(k) + '->n_task = i;\n')
                        sim.write('\t\t\treq' + str(k) + '->size_task = ' + str(value_output[k]) + ';\n')
                        sim.write('\t\t\ttask = MSG_task_create("Data", 0, 0, NULL);\n')
                        sim.write('\t\t\tMSG_task_set_data(task, (void *)req' + str(k) + ');\n\n')

                        ''' Search for the task receiving device '''
                        length_table = len(rowsApp)
                        for dest in range(length_table):
                            if device_dest[k] in rowsApp[dest][1]:
                                sim.write('\t\t\tsprintf(mailbox, "' + rowsApp[dest][0] + '");\n')
                                break

                        sim.write('\t\t\tMSG_task_send(task, mailbox);\n\n')

                        if "True" in rowsApp[valueiot][2]:
                            sim.write('\t\t\tres = MSG_task_receive(&(task), MSG_host_get_name(MSG_host_self()));\n\n')
                            sim.write('\t\t\tif (res == MSG_OK)\n\t\t\t{\n')
                            sim.write('\t\t\t\tresServer = MSG_task_get_data(task);\n')
                            sim.write('\t\t\t\tif(resServer->iot_cluster != my_iot_cluster || resServer->device != my_device)\n\t\t\t\t{\n')
                            sim.write('\t\t\t\t\tprintf("There was an error receiving the task.\\n");\n\t\t\t\t}\n')
                            sim.write('\t\t\t\tfree(resServer);\n\t\t\t\tMSG_task_destroy(task);\n\t\t\t\ttask = NULL;\n\t\t\t}\n\n')

                    sim.write('\t\t}\n\n')

            sim.write('\t\tMSG_host_set_pstate(host, 2);\n')
            sim.write('\t\ttask = NULL;\n')
            sim.write('\t\tstatsIoT[my_iot_cluster].numTasks[my_device] += 1;\n')
            sim.write('\t}\n\n\tiot_terminated();\n')
            sim.write('\tstatsIoT[my_iot_cluster].totalEnergy[my_device] = sg_host_get_consumed_energy(host);\n')
            sim.write('\tstatsIoT[my_iot_cluster].avEnergy[my_device] = sg_host_get_consumed_energy(host) / statsIoT[my_iot_cluster].numTasks[my_device];\n')
            sim.write('\tstatsIoT[my_iot_cluster].avTime[my_device] = statsIoT[my_iot_cluster].avTime[my_device] / statsIoT[my_iot_cluster].numTasks[my_device];\n')
            sim.write('\treturn 0;\n}\n\n')

        list_fog = []
        for i in range(num_fog):
            length_table = len(rowsApp)
            fog_string = 'fog-' + str(i)
            for fog_row in range(length_table):
                if fog_string in rowsApp[fog_row][0]:
                    list_fog.append(fog_row)

        for i in range(num_fog):
            sim.write('int fog' + str(i) + '(int argc, char *argv[])\n{\n')
            sim.write('\tint res, my_d, server = 0, length = 0, my_dis;\n')
            sim.write('\tchar buf[64], buf2[40], mailbox[64];\n')
            sim.write('\tmsg_task_t task = NULL, new_task = NULL, host = NULL;\n')
            sim.write('\tClientRequest *req, *status;, reqAux;\n')
            sim.write('\tstruct ServerResponse *resServer;\n')

            sim.write('\tmy_d = atoi(argv[0]);\n')
            sim.write('\tmy_dis = atoi(argv[1]);\n')
            sim.write('\tsprintf(buf, "fog-%d-%d", my_d, my_dis);\n')
            sim.write('\tMSG_mailbox_set_async(buf);\n')
            sim.write('\thost = MSG_host_by_name(buf);\n\n')
            sim.write('\tstatsFog[my_d].numTasks[my_dis] = 0;\n')
            sim.write('\tstatsFog[my_d].executionTime[my_dis] = 0.0;\n')
            sim.write('\tstatsFog[my_d].dataProcessed[my_dis] = 0.0;\n')
            sim.write('\tMSG_host_set_pstate(host, 2);\n\n')

            sim.write('\twhile (1)\n\t{\n')
            sim.write('\t\tres = MSG_task_receive(&(task), MSG_host_get_name(MSG_host_self()));\n')
            sim.write('\t\tif (res != MSG_OK) break;\n\n')

            sim.write('\t\tif (strcmp(MSG_task_get_name(task), "Finalize") == 0)\n\t\t{\n')
            sim.write('\t\t\tMSG_task_destroy(task);\n')
            sim.write('\t\t\ttask = NULL;\n')
            sim.write('\t\t\tbreak;\n\t\t}\n\n')

            sim.write('\t\tif (strcmp(MSG_task_get_name(task), "Data") == 0)\n\t\t{\n')
            sim.write('\t\t\tdouble c = MSG_get_clock();\n')
            sim.write('\t\t\treq = MSG_task_get_data(task);\n')
            sim.write('\t\t\treqAux = *req;\n')
            sim.write('\t\t\tfree(req);\n\n')

            for j in range(len(list_fog)):
                if 'fog-' + str(i) + '-' + str(j) in rowsApp[list_fog[0]][0]:

                    sim.write('\t\t\tif strcmp(buf, ' + '"fog-' + str(i) + '-' + str(j) + '") == 0)\n\t\t\t{\n')

                    sim.write('\t\t\t\tif (reqAux.fog_usage > 0)\n\t\t\t\t{\n')
                    sim.write('\t\t\t\t\tMSG_host_set_pstate(host,0);\n')
                    sim.write('\t\t\t\t\tnew_task = NULL;\n')

                    valuefog = list_fog.pop(0)
                    processing = rowsApp[valuefog][3]
                    insert = 0

                    if 'GFlops' in processing:
                        insert = float(processing.split("GFlops", 1)[0]) * pow(10, 9)
                        sim.write('\t\t\t\t\tdouble proc = ' + str(insert) + ' * reqAux.fog_usage;\n')

                    if 'MFlops' in processing:
                        insert = float(processing.split("MFlops", 1)[0]) * pow(10, 6)
                        sim.write('\t\t\t\t\tdouble proc = ' + str(insert) + ' * reqAux.fog_usage;\n')

                    sim.write('\t\t\t\t\tdouble data = reqAux.size_task;\n')
                    sim.write('\t\t\t\t\tnew_task = MSG_task_create("Data", proc, data, NULL);\n')
                    sim.write('\t\t\t\t\tMSG_task_execute(new_task);\n')
                    sim.write('\t\t\t\t\tMSG_task_destroy(new_task);\n')
                    sim.write('\t\t\t\t\tMSG_host_set_pstate(host, 2);\n\t\t\t\t}\n\n')

                    sim.write('\t\t\t\tMSG_task_destroy(task);\n')
                    sim.write('\t\t\t\ttask = NULL;\n')

                    output_fog = rowsApp[valuefog][5]
                    stream_output_fog = 0

                    if 'GB/s' in output_fog:
                        result = output_fog.split("GB/s", 1)[0]
                        stream_output_fog = int(result) * 1024 * 1024

                    if 'MB/s' in output_fog:
                        result = output_fog.split("MB/s", 1)[0]
                        stream_output_fog = int(result) * 1024

                    if 'KB/s' in output_fog:
                        result = output_fog.split("KB/s", 1)[0]
                        stream_output_fog = int(result)

                    list_dest_fog = rowsApp[valuefog][6].split(";")

                    value_output_fog = []
                    device_dest_fog = []
                    total_stream_fog = 0

                    for k in range(len(list_dest_fog)):
                        device_dest_fog.append(list_dest_fog[k].split(":", 1)[1])

                        if 'GB/s' in list_dest_fog[k]:
                            insert = int(list_dest_fog[k].split("GB/s", 1)[0]) * 1024 * 1024
                            value_output_fog.append(insert)
                            total_stream_fog = total_stream_fog + int(insert)

                        if 'MB/s' in list_dest_fog[k]:
                            insert = int(list_dest_fog[k].split("MB/s", 1)[0]) * 1024
                            value_output_fog.append(insert)
                            total_stream_fog = total_stream_fog + int(insert)

                        if 'KB/s' in list_dest_fog[k]:
                            insert = int(list_dest_fog[k].split("KB/s", 1)[0])
                            value_output_fog.append(insert)
                            total_stream_fog = total_stream_fog + int(insert)

                    if total_stream_fog != stream_output_fog:
                        print('Error: The output value does not match the output stream')
                        print(str(total_stream_fog) + " " + str(stream_output_fog))
                        exit(-1)

                    for k in range(len(list_dest_fog)):
                        sim.write('\t\t\t\tstruct ClientRequest *req' + str(k) + ';\n')
                        sim.write('\t\t\t\treq' + str(k) + ' = (struct ClientRequest *)malloc(sizeof(struct ClientRequest));\n\n')
                        sim.write('\t\t\t\t*req' + str(k) + ' = reqAux;\n')
                        sim.write('\t\t\t\treq' + str(k) + '->t_arrival = MSG_get_clock();\n')
                        sim.write('\t\t\t\treq' + str(k) + '->t_service = ' + str(insert) + ' * (1 - req' + str(k) + '->fog_usage);\n')
                        sim.write('\t\t\t\treq' + str(k) + '->size_task = req' + str(k) + '->size_task * (1 - req' + str(k) + '->fog_usage);\n')

                        sim.write('\t\t\t\tnew_task = NULL;\n')
                        sim.write('\t\t\t\tnew_task = MSG_task_create("Data", req' + str(k) + '->t_service, req' + str(k) + '->size_task, NULL);\n')
                        sim.write('\t\t\t\tMSG_task_set_data(task, (void *)req' + str(k) + ');\n\n')

                        ''' Search for the task receiving device '''
                        length_table = len(rowsApp)
                        for dest in range(length_table):
                            if device_dest_fog[k] in rowsApp[dest][1]:
                                sim.write('\t\t\t\tsprintf(mailbox, "' + rowsApp[dest][0] + '");\n')
                                break

                        sim.write('\t\t\t\tMSG_task_send(task, mailbox);\n\t\t\t}\n\n')

                        if "True" in rowsApp[valuefog][2]:
                            sim.write('\t\t\tres = MSG_task_receive(&(task), MSG_host_get_name(MSG_host_self()));\n\n')
                            sim.write('\t\t\tif (res == MSG_OK)\n\t\t\t{\n')
                            sim.write('\t\t\t\tresServer = MSG_task_get_data(task);\n')
                            sim.write('\t\t\t\tif(resServer->iot_cluster != my_iot_cluster || resServer->device != my_device)\n\t\t\t\t{\n')
                            sim.write('\t\t\t\t\tprintf("There was an error receiving the task.\\n");\n\t\t\t\t}\n')
                            sim.write('\t\t\t\tfree(resServer);\n\t\t\t\tMSG_task_destroy(task);\n\t\t\t\ttask = NULL;\n\t\t\t}\n\n')

            sim.write('\t\t\tstatsFog[my_d].numTasks[my_dis] += 1;\n')
            sim.write('\t\t\tstatsFog[my_d].executionTime[my_dis] += (MSG_get_clock() - c);\n')
            sim.write('\t\t\tstatsFog[my_d].dataProcessed[my_dis] += reqAux.size_task * reqAux.fog_usage;\n\t\t}\n\n')

            sim.write('\n\t}\n\tstatsFog[my_d].avEnergy[my_dis] = (sg_host_get_consumed_energy(host) / statsFog[my_d].numTasks[my_dis]);\n')
            sim.write('\tstatsFog[my_d].totalEnergy[my_dis] = sg_host_get_consumed_energy(host);\n')
            sim.write('\tif (statsFog[my_d].numTasks[my_dis] == 0) statsFog[my_d].executionTime[my_dis] = 0;\n')
            sim.write('\tfog_terminated();\n\n')
            sim.write('\treturn 0;\n}\n\n')

        list_edge = []
        for i in range(num_edge):
            length_table = len(rowsApp)
            edge_string = 'edge-' + str(i)
            for edge_row in range(length_table):
                if edge_string in rowsApp[edge_row][0]:
                    list_edge.append(edge_row)


        for i in range(num_edge):
            sim.write('int edge' + str(i) + '(int argc, char *argv[])\n{\n')
            sim.write('\tint res, my_d, server = 0, length = 0, my_dis;\n')
            sim.write('\tchar buf[64], buf2[40], mailbox[64];\n')
            sim.write('\tmsg_task_t task = NULL, new_task = NULL, host = NULL;\n')
            sim.write('\tClientRequest *req, *status;, reqAux;\n')
            sim.write('\tstruct ServerResponse *resServer;\n')

            sim.write('\tmy_d = atoi(argv[0]);\n')
            sim.write('\tmy_dis = atoi(argv[1]);\n')
            sim.write('\tsprintf(buf, "edge-%d-%d", my_d, my_dis);\n')
            sim.write('\tMSG_mailbox_set_async(buf);\n')
            sim.write('\thost = MSG_host_by_name(buf);\n\n')
            sim.write('\tstatsEdge[my_d].numTasks[my_dis] = 0;\n')
            sim.write('\tstatsEdge[my_d].executionTime[my_dis] = 0.0;\n')
            sim.write('\tstatsEdge[my_d].dataProcessed[my_dis] = 0.0;\n')
            sim.write('\tMSG_host_set_pstate(host, 2);\n\n')

            sim.write('\twhile (1)\n\t{\n')
            sim.write('\t\tres = MSG_task_receive(&(task), MSG_host_get_name(MSG_host_self()));\n')
            sim.write('\t\tif (res != MSG_OK) break;\n\n')

            sim.write('\t\tif strcmp(MSG_task_get_name(task), "Finalize") == 0)\n\t\t{\n')
            sim.write('\t\t\tMSG_task_destroy(task);\n')
            sim.write('\t\t\ttask = NULL;\n')
            sim.write('\t\t\tbreak;\n\t\t}\n\n')

            sim.write('\t\tif (strcmp(MSG_task_get_name(task), "Data") == 0)\n\t\t{\n')
            sim.write('\t\t\tdouble c = MSG_get_clock();\n')
            sim.write('\t\t\treq = MSG_task_get_data(task);\n')
            sim.write('\t\t\treqAux = *req;\n')
            sim.write('\t\t\tfree(req);\n\n')

            for j in range(len(list_edge)):
                if 'fog-' + str(i) + '-' + str(j) in rowsApp[list_edge[0]][0]:

                    sim.write('\t\t\tif strcmp(buf, ' + '"fog-' + str(i) + '-' + str(j) + '") == 0)\n\t\t\t{\n')

                    sim.write('\t\t\t\tif (reqAux.fog_usage > 0)\n\t\t\t\t{\n')
                    sim.write('\t\t\t\t\tMSG_host_set_pstate(host,0);\n')
                    sim.write('\t\t\t\t\tnew_task = NULL;\n')

                    valueedge = list_fog.pop(0)
                    processing = rowsApp[valueedge][3]
                    insert = 0

                    if 'GFlops' in processing:
                        insert = float(processing.split("GFlops", 1)[0]) * pow(10, 9)
                        sim.write('\t\t\t\t\tdouble proc = ' + str(insert) + ' * reqAux.fog_usage;\n')

                    if 'MFlops' in processing:
                        insert = float(processing.split("MFlops", 1)[0]) * pow(10, 6)
                        sim.write('\t\t\t\t\tdouble proc = ' + str(insert) + ' * reqAux.fog_usage;\n')

                    sim.write('\t\t\t\t\tdouble data = reqAux.size_task;\n')
                    sim.write('\t\t\t\t\tnew_task = MSG_task_create("Data", proc, data, NULL);\n')
                    sim.write('\t\t\t\t\tMSG_task_execute(new_task);\n')
                    sim.write('\t\t\t\t\tMSG_task_destroy(new_task);\n')
                    sim.write('\t\t\t\t\tMSG_host_set_pstate(host, 2);\n\t\t\t\t}\n\n')

                    sim.write('\t\t\t\tMSG_task_destroy(task);\n')
                    sim.write('\t\t\t\ttask = NULL;\n')

                    output_edge = rowsApp[valueedge][5]
                    stream_output_edge = 0

                    if 'GB/s' in output_edge:
                        result = output_edge.split("GB/s", 1)[0]
                        stream_output_edge = int(result) * 1024 * 1024

                    if 'MB/s' in output_edge:
                        result = output_edge.split("MB/s", 1)[0]
                        stream_output_edge = int(result) * 1024

                    if 'KB/s' in output_edge:
                        result = output_edge.split("KB/s", 1)[0]
                        stream_output_edge = int(result)

                    list_dest_edge = rowsApp[valueedge][6].split(";")

                    value_output_edge = []
                    device_dest_edge = []
                    total_stream_edge = 0

                    for k in range(len(list_dest_edge)):
                        device_dest_edge.append(list_dest_fog[k].split(":", 1)[1])

                        if 'GB/s' in list_dest_edge[k]:
                            insert = int(list_dest_edge[k].split("GB/s", 1)[0]) * 1024 * 1024
                            value_output_edge.append(insert)
                            total_stream_edge = total_stream_edge + int(insert)

                        if 'MB/s' in list_dest_edge[k]:
                            insert = int(list_dest_edge[k].split("MB/s", 1)[0]) * 1024
                            value_output_edge.append(insert)
                            total_stream_edge = total_stream_edge + int(insert)

                        if 'KB/s' in list_dest_edge[k]:
                            insert = int(list_dest_edge[k].split("KB/s", 1)[0])
                            value_output_edge.append(insert)
                            total_stream_edge = total_stream_edge + int(insert)

                    if total_stream_edge != stream_output_edge:
                        print('Error: The output value does not match the output stream')
                        print(str(total_stream_edge) + " " + str(stream_output_edge))
                        exit(-1)

                    for k in range(len(list_dest_edge)):
                        sim.write('\t\t\t\tstruct ClientRequest *req' + str(k) + ';\n')
                        sim.write('\t\t\t\treq' + str(
                            k) + ' = (struct ClientRequest *)malloc(sizeof(struct ClientRequest));\n\n')
                        sim.write('\t\t\t\t*req' + str(k) + ' = reqAux;\n')
                        sim.write('\t\t\t\treq' + str(k) + '->t_arrival = MSG_get_clock();\n')
                        sim.write('\t\t\t\treq' + str(k) + '->t_service = ' + str(insert) + ' * (1 - req' + str(
                            k) + '->edge_usage);\n')
                        sim.write(
                            '\t\t\t\treq' + str(k) + '->size_task = req' + str(k) + '->size_task * (1 - req' + str(
                                k) + '->edge_usage);\n')

                        sim.write('\t\t\t\tnew_task = NULL;\n')
                        sim.write('\t\t\t\tnew_task = MSG_task_create("Data", req' + str(k) + '->t_service, req' + str(
                            k) + '->size_task, NULL);\n')
                        sim.write('\t\t\t\tMSG_task_set_data(task, (void *)req' + str(k) + ');\n\n')

                        ''' Search for the task receiving device '''
                        length_table = len(rowsApp)
                        for dest in range(length_table):
                            if device_dest_edge[k] in rowsApp[dest][1]:
                                sim.write('\t\t\t\tsprintf(mailbox, "' + rowsApp[dest][0] + '");\n')
                                break

                        sim.write('\t\t\t\tMSG_task_send(task, mailbox);\n\t\t\t}\n\n')

                        if "True" in rowsApp[valueedge][2]:
                            sim.write('\t\t\tres = MSG_task_receive(&(task), MSG_host_get_name(MSG_host_self()));\n\n')
                            sim.write('\t\t\tif (res == MSG_OK)\n\t\t\t{\n')
                            sim.write('\t\t\t\tresServer = MSG_task_get_data(task);\n')
                            sim.write(
                                '\t\t\t\tif(resServer->iot_cluster != my_iot_cluster || resServer->device != my_device)\n\t\t\t\t{\n')
                            sim.write('\t\t\t\t\tprintf("There was an error receiving the task.\\n");\n\t\t\t\t}\n')
                            sim.write(
                                '\t\t\t\tfree(resServer);\n\t\t\t\tMSG_task_destroy(task);\n\t\t\t\ttask = NULL;\n\t\t\t}\n\n')

            sim.write('\t\t\tstatsEdge[my_d].numTasks[my_dis] += 1;\n')
            sim.write('\t\t\tstatsEdge[my_d].executionTime[my_dis] += (MSG_get_clock() - c);\n')
            sim.write('\t\t\tstatsEdge[my_d].dataProcessed[my_dis] += reqAux.size_task * reqAux.fog_usage;\n\t\t}\n\n')

            sim.write(
                '\tstatsEdge[my_d].avEnergy[my_dis] = (sg_host_get_consumed_energy(host) / statsEdge[my_d].numTasks[my_dis]);\n')
            sim.write('\tstatsEdge[my_d].totalEnergy[my_dis] = sg_host_get_consumed_energy(host);\n')
            sim.write('\tif (statsEdge[my_d].numTasks[my_dis] == 0) statsEdge[my_d].executionTime[my_dis] = 0;\n')
            sim.write('\tedge_terminated();\n\n')
            sim.write('\treturn 0;\n}\n\n')

        list_cloud = []
        for i in range(num_datacenter):
            length_table = len(rowsApp)
            cloud_string = 'cloud-' + str(i)
            for cloud_row in range(length_table):
                if cloud_string in rowsApp[cloud_row][0]:
                    list_cloud.append(cloud_row)

        for i in range(num_datacenter):

            sim.write('int cloud' + str(i) + '(int argc, char *argv[])\n{\n')
            sim.write('\tmsg_task_t task = NULL, new_task = NULL, ans_task = NULL;\n')
            sim.write('\tstruct ClientRequest *req;\n')
            sim.write('\tint res, my_datacenter = atoi(argv[1]), my_server = atoi(argv[2]);\n')
            sim.write('\tchar buf[40], hostS[30], mailbox[64];\n\n')
            sim.write('\tsprintf(buf, "%s", argv[0]);\n')
            sim.write('\tmsg_host_t host = MSG_host_by_name(buf);\n')
            sim.write('\tstatsCloud[my_datacenter].numTasks[my_server] = 0;\n')
            sim.write('\tstatsCloud[my_datacenter].executionTime[my_server] = 0.0;\n')
            sim.write('\tstatsCloud[my_datacenter].dataProcessed[my_server] = 0.0;\n')
            sim.write('\tMSG_host_set_pstate(host, 2);\n\n')
            sim.write('\twhile (1)\n\t{\n')
            sim.write('\t\tres = MSG_task_receive(&(task), MSG_host_get_name(MSG_host_self()));\n')
            sim.write('\t\tif(res != MSG_OK) break;\n')
            sim.write('\t\tif(strcmp(MSG_task_get_name(task), "Finalize") == 0)\n\t\t{\n')
            sim.write('\t\t\tMSG_task_destroy(task);\n\t\t\ttask = NULL;\n\t\t\tbreak;\n\t\t}\n')
            sim.write('\t\tif(strcmp(MSG_task_get_name(task), "Data") == 0)\n\t\t{\n')
            sim.write('\t\t\treq = MSG_task_get_data(task);\n')
            sim.write('\t\t\tMSG_task_destroy(task);\n')
            sim.write('\t\t\ttask = NULL;\n\n')
            sim.write('\t\t\txbt_mutex_acquire(tasksManagement[my_datacenter].mutex[my_server]);\n')
            sim.write('\t\t\ttasksManagement[my_datacenter].Nqueue[my_server]++;\n')
            sim.write('\t\t\ttasksManagement[my_datacenter].Nsystem[my_server]++;\n')
            sim.write('\t\t\txbt_dynar_push(tasksManagement[my_datacenter].client_requests[my_server], (const char *)&req);\n');
            sim.write('\t\t\txbt_cond_signal(tasksManagement[my_datacenter].cond[my_server]);\n')
            sim.write('\t\t\txbt_mutex_release(tasksManagement[my_datacenter].mutex[my_server]);\n\t\t}\n\t}\n\n')

            sim.write('\txbt_mutex_acquire(tasksManagement[my_datacenter].mutex[my_server]);\n')
            sim.write('\ttasksManagement[my_datacenter].EmptyQueue[my_server] = 1;\n')
            sim.write('\txbt_cond_signal(tasksManagement[my_datacenter].cond[my_server]);\n')
            sim.write('\txbt_mutex_release(tasksManagement[my_datacenter].mutex[my_server]);\n')
            sim.write('\treturn 0;\n}\n\n')

            sim.write('int dispatcherCloud' + str(i) + '(int argc, char *argv[])\n{\n')
            sim.write('\tchar mailbox[64], hostS[30], vm_name[30];\n')
            sim.write('\tstruct ClientRequest *req, reqAux;\n')
            sim.write('\tstruct ServerResponse *resServer;\n')
            sim.write('\tmsg_task_t task = NULL;\n')
            sim.write('\tdouble Nqueue_avg = 0.0, Nsystem_avg = 0.0, c = 0.0, wait = 0.0;\n')
            sim.write('\tint n_tasks = 0, my_datacenter = atoi(argv[1]), my_server = atoi(argv[2]), opt_vm = 0, res = 0;\n\n')
            sim.write('\tsprintf(hostS, "%s", argv[0]);\n')
            sim.write('\tmsg_host_t host = MSG_host_by_name(hostS);\n')
            sim.write('\tdouble c1 = MSG_get_clock();\n')
            sim.write('\tdouble ca = 0.0;\n\n')
            sim.write('\twhile(1)\n\t{\n')
            sim.write('\t\txbt_mutex_acquire(tasksManagement[my_datacenter].mutex[my_server]);\n\n')
            sim.write('\t\twhile((tasksManagement[my_datacenter].Nqueue[my_server] == 0) && (tasksManagement[my_datacenter].EmptyQueue[my_server] == 0))\n\t\t{\n')
            sim.write('\t\t\txbt_cond_wait(tasksManagement[my_datacenter].cond[my_server], tasksManagement[my_datacenter].mutex[my_server]);\n\t\t}\n\n')
            sim.write('\t\tif((tasksManagement[my_datacenter].EmptyQueue[my_server] == 1) && (tasksManagement[my_datacenter].Nqueue[my_server] == 0))\n\t\t{\n')
            sim.write('\t\t\tstatsCloud[my_datacenter].avEnergy[my_server] = (sg_host_get_consumed_energy(host)/statsCloud[my_datacenter].numTasks[my_server]);\n')
            sim.write('\t\t\tstatsCloud[my_datacenter].totalEnergy[my_server] = sg_host_get_consumed_energy(host);\n')
            sim.write('\t\t\txbt_mutex_release(tasksManagement[my_datacenter].mutex[my_server]);\n\t\t\tbreak;\n\t\t}\n\n')

            sim.write('\t\txbt_dynar_shift(tasksManagement[my_datacenter].client_requests[my_server], (char *)&req);\n')
            sim.write('\t\ttasksManagement[my_datacenter].Nqueue[my_server]--;\n\t\tn_tasks++;\n')
            sim.write('\t\ttasksManagement[my_datacenter].Navgqueue[my_server] = (tasksManagement[my_datacenter].Navgqueue[my_server] * (n_tasks - 1) + tasksManagement[my_datacenter].Nqueue[my_server]) / n_tasks;\n')
            sim.write('\t\ttasksManagement[my_datacenter].Navgsystem[my_server] = (tasksManagement[my_datacenter].Navgsystem[my_server] * (n_tasks - 1) + tasksManagement[my_datacenter].Nsystem[my_server]) / n_tasks;\n')
            sim.write('\t\txbt_mutex_release(tasksManagement[my_datacenter].mutex[my_server]);\n\n\t\tMSG_host_set_pstate(host, 0);\n')
            sim.write('\t\tmsg_vm_t vm;\n\t\tsize_t ram;\n')
            sim.write('\t\tsprintf(vm_name, "vm-%d-%d",my_datacenter,my_server);\n\t\tvm = MSG_vm_create_multicore(host, vm_name, 16);\n')
            sim.write('\t\tram = 32 * 1024;\n\t\tMSG_vm_set_ramsize(vm, ram);\n\t\tMSG_vm_start(vm);\n')
            sim.write('\t\treqAux = *req;\n\t\tfree(req);\n\n')
            sim.write('\t\ttask = MSG_task_create("Data", reqAux.t_service, reqAux.size_task, &reqAux);\n')
            sim.write('\t\tc = MSG_get_clock();\n\t\tMSG_task_execute(task);\n\t\tca = ca + MSG_get_clock() - c;\n')
            sim.write('\t\tMSG_task_destroy(task);\n\t\ttask = NULL;\n\t\tMSG_host_set_pstate(host, 2);\n')
            sim.write('\t\twait = MSG_get_clock();\n\n\t\tMSG_vm_shutdown(vm);\n\t\tMSG_vm_destroy(vm);\n\n')

            for i in range(len(list_cloud)):
                valuecloud = list_cloud.pop(0)

                if ':' in rowsApp[valuecloud][6]:

                    output_cloud = rowsApp[valuecloud][5]
                    stream_output_cloud = 0

                    if 'GB/s' in output_cloud:
                        result = output_cloud.split("GB/s", 1)[0]
                        stream_output_cloud = int(result) * 1024 * 1024

                    if 'MB/s' in output_cloud:
                        result = output_cloud.split("MB/s", 1)[0]
                        stream_output_cloud = int(result) * 1024

                    if 'KB/s' in output_cloud:
                        result = output_cloud.split("KB/s", 1)[0]
                        stream_output_cloud = int(result)

                    list_dest_cloud = rowsApp[valuecloud][6].split(";")

                    value_output_cloud = []
                    device_dest_cloud = []
                    total_stream_cloud = 0

                    for k in range(len(list_dest_cloud)):
                        device_dest_cloud.append(list_dest_cloud[k].split(":", 1)[1])

                        if 'GB/s' in list_dest_cloud[k]:
                            insert = int(list_dest_cloud[k].split("GB/s", 1)[0]) * 1024 * 1024
                            value_output_cloud.append(insert)
                            total_stream_cloud = total_stream_cloud + int(insert)

                        if 'MB/s' in list_dest_cloud[k]:
                            insert = int(list_dest_cloud[k].split("MB/s", 1)[0]) * 1024
                            value_output_cloud.append(insert)
                            total_stream_cloud = total_stream_cloud + int(insert)

                        if 'KB/s' in list_dest_cloud[k]:
                            insert = int(list_dest_cloud[k].split("KB/s", 1)[0])
                            value_output_cloud.append(insert)
                            total_stream_cloud = total_stream_cloud + int(insert)

                    if total_stream_cloud != stream_output_cloud:
                        print('Error: The output value does not match the output stream')
                        print(str(total_stream_cloud) + " " + str(stream_output_cloud))
                        exit(-1)


                    for k in range(len(list_dest_cloud)):
                        sim.write('\t\tif(strcmp(argv[0], "' + rowsApp[valuecloud][0] + '") == 0)\n\t\t{\n')
                        sim.write('\t\t\tmsg_task_t new_task = NULL;\n')
                        sim.write('\t\t\tstruct ClientRequest *req' + str(k) + ' = (struct ClientRequest *) malloc(sizeof(struct ClientRequest));\n')
                        sim.write('\t\t\t*req' + str(k) + ' = reqAux;\n\n')
                        sim.write('\t\t\tnew_task = MSG_task_create("Data", req' + str(k) + '->t_service, req' + str(k) + '->size_task, req' + str(k) + ');\n')

                        ''' Search for the task receiving device '''
                        length_table = len(rowsApp)
                        for dest in range(length_table):
                            if device_dest_cloud[k] in rowsApp[dest][1]:
                                sim.write('\t\t\tsprintf(mailbox, "' + rowsApp[dest][0] + '");\n')
                                break

                        sim.write('\t\t\tMSG_task_send(task, mailbox);\n\t\t}\n')

                """else:
                    sim.write('\t\tif (strcmp(argv[0]), "' + rowsApp[valuecloud][0] + '") == 0)\n\t\t{\n')
                    sim.write('\t\t\txbt_mutex_acquire(m);\n')
                    sim.write('\t\t\tdata_processed += 1;\n')
                    sim.write('\t\t\txbt_cond_signal(con);\n')
                    sim.write('\t\t\txbt_mutex_release(m);\n\t\t}\n')"""

            sim.write('\t\txbt_mutex_acquire(tasksManagement[my_datacenter].mutex[my_server]);\n')
            sim.write('\t\ttasksManagement[my_datacenter].Nsystem[my_server]--;\n')
            sim.write('\t\tstatsCloud[my_datacenter].numTasks[my_server] += 1;\n')
            sim.write('\t\tstatsCloud[my_datacenter].dataProcessed[my_server] += reqAux.size_task;\n')
            sim.write('\t\txbt_mutex_release(tasksManagement[my_datacenter].mutex[my_server]);\n\n')


            sim.write('\t}\n\n')
 
            sim.write('\tdouble c2 = MSG_get_clock() - c1;  // felix\n')
            sim.write('\tstatsCloud[my_datacenter].executionTime[my_server] = ca;\n')
            sim.write('\tprintf("--------------------->  %s   %g\\n", hostS, ca / c2);  //felix  este es el % de uso de CPU\n}\n\n\n')

        sim.write('int controller(int argc, char *argv[])\n{\n')
        sim.write('\tint res, devices_finished = 0, total_iot = ' + str(total_iot) + ', total_nodes = ' + str(total_cloud) + ', total_edge_nodes = '+str(total_edge)+', total_fog_devices = ' + str(total_fog) + ';\n')
        sim.write('\tmsg_task_t task = NULL;\n')
        sim.write('\tchar mailbox[64], buffer[64], node[30];\n')
        sim.write('\tstruct ClientRequest *req;\n')
        sim.write('\tstruct ControllerResponse *response;\n\n')
        sim.write('\tsprintf(mailbox, "cont-0");\n')
        sim.write('\tMSG_mailbox_set_async(mailbox);\n')
        sim.write('\tinit_m();\n\n')

        sim.write('\txbt_mutex_acquire(m);\n')
        sim.write('\twhile(total_iot != iot_finished)\n\t{\n')
        sim.write('\t\txbt_cond_wait(con,m);\n\t}\n')
        sim.write('\txbt_mutex_release(m);\n\n')

        sim.write('\treq = (struct ClientRequest *)malloc(sizeof(struct ClientRequest));\n')
        sim.write('\tsprintf(buffer, "Finalize");\n')
        sim.write('\tsprintf(req->id_task, "%s", buffer);\n\n')

        for i in range(num_edge):
            sim.write('\tfor (int i = 0; i < EDGE' + str(i) + '; i++)\n\t{\n')
            sim.write('\t\ttask = MSG_task_create("Finalize", 0, 0, NULL);\n')
            sim.write('\t\tMSG_task_set_data(task, (void *)req);\n')
            sim.write('\t\tsprintf(node, "edge-' + str(i) + '-%d", i);\n')
            sim.write('\t\tMSG_task_send(task, node);\n\t}\n\n')

        if num_edge != 0:
            sim.write('\txbt_mutex_acquire(m);\n')
            sim.write('\twhile(total_edge_nodes != edge_finished)\n\t{\n')
            sim.write('\t\txbt_cond_wait(con,m);\n\t}\n')
            sim.write('\txbt_mutex_release(m);\n\n')

        for i in range(num_fog):
            sim.write('\tfor (int i = 0; i < FOG' + str(i) + '; i++)\n\t{\n')
            sim.write('\t\ttask = MSG_task_create("Finalize", 0, 0, NULL);\n')
            sim.write('\t\tMSG_task_set_data(task, (void *)req);\n')
            sim.write('\t\tsprintf(node, "fog-' + str(i) + '-%d", i);\n')
            sim.write('\t\tMSG_task_send(task, node);\n\t}\n\n')

        if num_fog != 0:
            sim.write('\txbt_mutex_acquire(m);\n')
            sim.write('\twhile(total_fog_devices != fog_finished)\n\t{\n')
            sim.write('\t\txbt_cond_wait(con,m);\n\t}\n')
            sim.write('\txbt_mutex_release(m);\n\n')

        for i in range(num_datacenter):
            sim.write('\tfor (int i = 0; i < NODESDATACENTER' + str(i) + '; i++)\n\t{\n')
            sim.write('\t\ttask = MSG_task_create("Finalize", 0, 0, NULL);\n')
            sim.write('\t\tMSG_task_set_data(task, (void *)req);\n')
            sim.write('\t\tsprintf(node, "cloud-' + str(i) + '-%d", i);\n')
            sim.write('\t\tMSG_task_send(task, node);\n\t}\n\n')

        sim.write('\treturn 0;\n}\n\n')

        sim.write('void init_m(void)\n{\n')
        sim.write('\tif(init == 0)\n\t{\n')
        sim.write('\t\tinit = 1;\n')
        sim.write('\t\tm = xbt_mutex_init();\n')
        sim.write('\t\tcon = xbt_cond_init();\n\t}\n}\n\n')


        sim.write('void iot_terminated (void)\n{\n')
        sim.write('\txbt_mutex_acquire(m);\n')
        sim.write('\tiot_finished += 1;\n')
        sim.write('\txbt_cond_signal(con);\n')
        sim.write('\txbt_mutex_release(m);\n}\n\n')

        if num_edge != 0:
            sim.write('void edge_terminated (void)\n{\n')
            sim.write('\txbt_mutex_acquire(m);\n')
            sim.write('\tedge_finished += 1;\n')
            sim.write('\txbt_cond_signal(con);\n')
            sim.write('\txbt_mutex_release(m);\n}\n\n')

        if num_fog != 0:
            sim.write('void fog_terminated (void)\n{\n')
            sim.write('\txbt_mutex_acquire(m);\n')
            sim.write('\tfog_finished += 1;\n')
            sim.write('\txbt_cond_signal(con);\n')
            sim.write('\txbt_mutex_release(m);\n}\n\n')

        sim.close()

if __name__ == "__main__":
	main()