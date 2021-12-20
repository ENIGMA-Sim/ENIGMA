import csv, sys


def main():
    with open(sys.argv[1]) as file:

        template = csv.reader(file, delimiter='|')
        #, open(sys.argv[2]) as appfiletemplateApp = csv.reader(appfile, delimiter='|')

        rows = [r for r in template]
        #rowsApp = [i for i in templateApp]

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

        main = open(str(sys.argv[2]), "w+")

        main.write('#include "./simulation.h"\n\n')

        for i in range(num_datacenter):
            devices = "devices_cloud" + str(i)
            pos = 0

            for line in rows:
                if devices in line:
                    break
                pos = pos + 1
            main.write('#define NODESDATACENTER' + str(i) + " " + str(rows[pos][1]) + "\n")

        for i in range(num_fog):
            devices = "devices_fog" + str(i)
            pos = 0

            for line in rows:
                if devices in line:
                    break
                pos = pos + 1
            main.write('#define FOG' + str(i) + " " + str(rows[pos][1]) + "\n")

        for i in range(num_edge):
            devices = "devices_edge" + str(i)
            pos = 0

            for line in rows:
                if devices in line:
                    break
                pos = pos + 1
            main.write('#define EDGE' + str(i) + " " + str(rows[pos][1]) + "\n")

        for i in range(num_iot):
            devices = "devices_iot" + str(i)
            pos = 0

            for line in rows:
                if devices in line:
                    break
                pos = pos + 1
            main.write('#define IOT' + str(i) + " " + str(rows[pos][1]) + "\n")

        main.write("\nvoid test_all(char *file)\n{\n")
        main.write('\tint argc, num_edge, cluster_edge, num_datacenters, nodes_datacenter, tasks[' + str(num_iot) + '];\n')
        main.write("\tchar str[50];\n")
        '''main.write('\tdouble percentage_fog[' + str(num_iot) + '], percentage_edge[' + str(num_iot) + '];\n')'''
        main.write("\tmsg_process_t p;\n\n")

        main.write("\tMSG_create_environment(file);\n")

        sum = 0

        if num_datacenter != 0:
            for i in range(num_datacenter):
                main.write('\tMSG_function_register("cloud' + str(i) + '", cloud' + str(i) + ');\n')
                main.write('\tMSG_function_register("dispatcherCloud' + str(i) + '", dispatcherCloud' + str(i) + ');\n')
        if num_fog != 0:
            for i in range(num_fog):
                main.write('\tMSG_function_register("fog' + str(i) + '", fog' + str(i) + ');\n')
        if num_edge != 0:
            for i in range(num_edge):
                main.write('\tMSG_function_register("edge' + str(i) + '", edge' + str(i) + ');\n')
        if num_iot != 0:
            for i in range(num_iot):
                main.write('\tMSG_function_register("iot' + str(i) + '", iot' + str(i) + ');\n')

        main.write('\tMSG_function_register("controller", controller);\n')        

        main.write('\n\t/*IOT*/\n\n')

        for i in range(num_iot):
            '''
            item_iot = num_datacenter * 12 + num_fog * 12 + num_edge * 12 + i * 13
            size = rowsApp[1 + i][2]
            tasks = rowsApp[1 + i][1]
            percentage_fog = rows[item_iot + 13][1]
            percentage_edge = rows[item_iot + 12][1]

            main.write('\tsize_data[' + str(i) + '] = ' + str(size) + ' * 1024;\n')
            main.write('\tnum_tasks[' + str(i) + '] = ' + str(tasks) + ';\n')
            main.write('\tpercentage_fog[' + str(i) + '] = ' + str(percentage_fog) + ';\n')
            main.write('\tpercentage_edge[' + str(i) + '] = ' + str(percentage_edge) + ';\n\n')'''

            #tasks = rowsApp[1 + i][1]
            #main.write('\ttasks[' + str(i) + '] = ' + str(tasks) + ';\n')

        for i in range(num_iot):
            main.write('\tfor(int i = 0; i < IOT' + str(i) + '; i++)\n\t{\n')
            main.write('\t\tsprintf(str, "iot-' + str(i) + '-%d", i);\n')
            main.write('\t\tprintf("Creating %s\\n", str);\n')
            main.write('\t\targc = 2;\n')
            main.write('\t\tchar **argvc' + str(sum) + ' = xbt_new(char *, 4);\n')
            main.write('\t\targvc' + str(sum) + '[0] = bprintf("%d", ' + str(i) + ');\n')
            main.write('\t\targvc' + str(sum) + '[1] = bprintf("%d", i);\n')
            #main.write('\t\targvc' + str(sum) + '[2] = bprintf("%d", size_data[' + str(i) + ']);\n')
            #main.write('\t\targvc' + str(sum) + '[2] = bprintf("%d", num_tasks[' + str(i) + ']);\n')
            main.write('\t\targvc' + str(sum) + '[2] = NULL;\n')
            main.write('\t\tp = MSG_process_create_with_arguments(str, iot' + str(
                i) + ', NULL, MSG_get_host_by_name(str), argc, argvc' + str(sum) + ');\n\n')
            main.write('\t\tif( p == NULL )\n\t\t{\n')
            main.write('\t\t\tprintf("Error en %s\\n", str);\n')
            main.write('\t\t\texit(0);\n\t\t}\n')
            main.write('\t}\n\n')
            sum = sum + 1

        if num_edge != 0:

            main.write('\n\t/*EDGE*/\n\n')

            for i in range(num_edge):
                main.write('\tfor(int i = 0; i < EDGE' + str(i) + '; i++)\n\t{\n')
                main.write('\t\tsprintf(str, "edge-' + str(i) + '-%d", i);\n')
                main.write('\t\tprintf("Creating %s\\n", str);\n')
                main.write('\t\targc = 2;\n')
                main.write('\t\tchar **argvc' + str(sum) + ' = xbt_new(char *, 3);\n')
                main.write('\t\targvc' + str(sum) + '[0] = bprintf("%d", ' + str(i) + ');\n')
                main.write('\t\targvc' + str(sum) + '[1] = bprintf("%d", i);\n')
                main.write('\t\targvc' + str(sum) + '[2] = NULL;\n')
                main.write('\t\tp = MSG_process_create_with_arguments(str, edge' + str(
                    i) + ', NULL, MSG_get_host_by_name(str), argc, argvc' + str(sum) + ');\n\n')
                main.write('\t\tif( p == NULL )\n\t\t{\n')
                main.write('\t\t\tprintf("Error en %s\\n", str);\n')
                main.write('\t\t\texit(0);\n\t\t}\n')
                main.write('\t}\n\n')
                sum = sum + 1

        if num_fog != 0:

            main.write('\n\t/*FOG*/\n\n')

            for i in range(num_fog):
                main.write('\tfor(int i = 0; i < FOG' + str(i) + '; i++)\n\t{\n')
                main.write('\t\tsprintf(str, "fog-' + str(i) + '-%d", i);\n')
                main.write('\t\tprintf("Creating %s\\n", str);\n')
                main.write('\t\targc = 2;\n')
                main.write('\t\tchar **argvc' + str(sum) + ' = xbt_new(char *, 3);\n')
                main.write('\t\targvc' + str(sum) + '[0] = bprintf("%d", ' + str(i) + ');\n')
                main.write('\t\targvc' + str(sum) + '[1] = bprintf("%d", i);\n')
                main.write('\t\targvc' + str(sum) + '[2] = NULL;\n')
                main.write('\t\tp = MSG_process_create_with_arguments(str, fog' + str(
                    i) + ', NULL, MSG_get_host_by_name(str), argc, argvc' + str(sum) + ');\n\n')
                main.write('\t\tif( p == NULL )\n\t\t{\n')
                main.write('\t\t\tprintf("Error en %s\\n", str);\n')
                main.write('\t\t\texit(0);\n\t\t}\n')
                main.write('\t}\n\n')
                sum = sum + 1

        if num_datacenter != 0:

            for i in range(num_datacenter):
                main.write('\n\t/*DATACENTER' + str(i) + '*/\n\n')

                main.write('\tfor(int i = 0; i < NODESDATACENTER' + str(i) + '; i++)\n\t{\n')
                main.write('\t\tsprintf(str, "cloud-' + str(i) + '-%d", i);\n')
                main.write('\t\tprintf("Creating %s\\n", str);\n')
                main.write('\t\targc = 3;\n')
                main.write('\t\tchar **argvc' + str(sum) + ' = xbt_new(char *, 4);\n')
                main.write('\t\targvc' + str(sum) + '[0] = bprintf("%s", str);\n')
                main.write('\t\targvc' + str(sum) + '[1] = bprintf("%d", ' + str(i) + ');\n')
                main.write('\t\targvc' + str(sum) + '[2] = bprintf("%d", i);\n')
                main.write('\t\targvc' + str(sum) + '[3] = NULL;\n')
                main.write('\t\tp = MSG_process_create_with_arguments(str, cloud' + str(
                    i) + ', NULL, MSG_get_host_by_name(str), argc, argvc' + str(sum) + ');\n\n')
                main.write('\t\tif( p == NULL )\n\t\t{\n')
                main.write('\t\t\tprintf("Error en %s\\n", str);\n')
                main.write('\t\t\texit(0);\n\t\t}\n')
                main.write('\t}\n\n')
                sum = sum + 1

                main.write('\n\t/*DISPATCHER' + str(i) + '*/\n\n')

                main.write('\tfor(int i = 0; i < NODESDATACENTER' + str(i) + '; i++)\n\t{\n')
                main.write('\t\tsprintf(str, "cloud-' + str(i) + '-%d", i);\n')
                main.write('\t\tprintf("Creating %s\\n", str);\n')
                main.write('\t\targc = 3;\n')
                main.write('\t\tchar **argvc' + str(sum) + ' = xbt_new(char *, 4);\n')
                main.write('\t\targvc' + str(sum) + '[0] = bprintf("%s", str);\n')
                main.write('\t\targvc' + str(sum) + '[1] = bprintf("%d", ' + str(i) + ');\n')
                main.write('\t\targvc' + str(sum) + '[2] = bprintf("%d", i);\n')
                main.write('\t\targvc' + str(sum) + '[3] = NULL;\n')
                main.write('\t\tp = MSG_process_create_with_arguments(str, dispatcherCloud' + str(
                    i) + ', NULL, MSG_get_host_by_name(str), argc, argvc' + str(sum) + ');\n\n')
                main.write('\t\tif( p == NULL )\n\t\t{\n')
                main.write('\t\t\tprintf("Error en %s\\n", str);\n')
                main.write('\t\t\texit(0);\n\t\t}\n')
                main.write('\t}\n\n')
                sum = sum + 1

        main.write('\tsprintf(str, "cont-0");\n')
        main.write('\tprintf("Creating %s\\n", str);\n')
        main.write('\targc = 0;\n')
        main.write('\tchar **argvc' + str(sum) + ' = xbt_new(char *, 1);\n')
        main.write('\targvc' + str(sum) + '[0] = NULL;\n')
        main.write(
            '\tp = MSG_process_create_with_arguments(str, controller, NULL, MSG_get_host_by_name(str), argc, argvc' + str(sum) + ');\n\n')
        main.write('\tif( p == NULL )\n\t{\n')
        main.write('\t\tprintf("Error en %s\\n", str);\n')
        main.write('\t\texit(0);\n\t}\n')

        main.write('\treturn;\n\n}')

        '''Main Function'''

        main.write('\n\nint main(int argc, char *argv[])\n{\n')
        main.write('\tmsg_error_t res = MSG_OK;\n')
        main.write('\tint taskExecuted = 0, days, min, hours;\n')
        main.write('\tdouble q_medio = 0.0, n_medio = 0.0, t, t_s;\n\n')

        main.write('\tif (argc < 2)\n\t{\n')
        main.write('\t\tprintf("Usage: %s platform_file.xml\\n", argv[0]);\n')
        main.write('\t\texit(1);\n\t}\n')

        main.write('\tt = (double)time(NULL);\n')
        main.write('\tseed((int)time(NULL));\n')
        main.write('\tsg_host_energy_plugin_init(t);\n')
        main.write('\tMSG_init(&argc, argv);\n\n')

        main.write('\tfor(int i = 0; i < MAX_DATACENTERS; i++)\n\t{\n')
        main.write('\t\tfor(int j = 0; j < MAX_SERVERS; j++)\n\t\t{\n')
        main.write('\t\t\ttasksManagement[i].Nqueue[j] = 0;\n')
        main.write('\t\t\ttasksManagement[i].Nsystem[j] = 0;\n')
        main.write('\t\t\ttasksManagement[i].EmptyQueue[j] = 0;\n')
        main.write('\t\t\ttasksManagement[i].mutex[j] = xbt_mutex_init();\n')
        main.write('\t\t\ttasksManagement[i].cond[j] = xbt_cond_init();\n')
        main.write('\t\t\ttasksManagement[i].client_requests[j] =')
        main.write(' xbt_dynar_new(sizeof(struct ClientRequest *), NULL);\n\t\t}\n\t}\n\n')

        main.write('\ttest_all(argv[1]);\n')
        main.write('\tres = MSG_main();\n')
        main.write('\tFILE *fp = fopen("./Results/tests.csv", "w+");\n')
        main.write('\tchar h[30];\n')
        main.write('\tmsg_host_t host;\n\n')
        main.write('\tfprintf(fp, "Server;Tasks Executed;Data Processed (MB);Energy Consumed;')
        main.write('Average Energy Consumed;Total Time;Execution Time;CPU Usage %\\n");\n')

        for i in range(num_datacenter):
            main.write('\n\t/*STATISTICS DATACENTER' + str(i) + '*/\n\n')
            main.write('\tfor(int i = 0; i < NODESDATACENTER' + str(i) + '; i++)\n\t{\n')
            main.write('\t\tq_medio = q_medio + tasksManagement[' + str(i) + '].Navgqueue[i];\n')
            main.write('\t\tn_medio = n_medio + tasksManagement[' + str(i) + '].Navgsystem[i];\n')
            main.write('\t\tsprintf(h, "d-' + str(i) + '-%d", i);\n')
            main.write('\t\thost = MSG_host_by_name(h);\n')
            main.write('\t\tfprintf(fp,"%s;%d;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f\\n"')
            main.write(',MSG_host_get_name(host), statsDatacenter[' + str(i) + '].numTasks[i], ')
            main.write('statsDatacenter[' + str(i) + '].dataProcessed[i]/(1024*1024), statsDatacenter[')
            main.write(str(i) + '].totalEnergy[i], ')
            main.write('statsDatacenter[' + str(i) + '].avEnergy[i], ')
            main.write('MSG_get_clock(), statsDatacenter[' + str(i) + '].executionTime[')
            main.write(str(i) + '], statsDatacenter[' + str(i) + '].executionTime[')
            main.write('i]*100/MSG_get_clock());\n')
            main.write('\t\ttaskExecuted += statsDatacenter[' + str(i) + '].numTasks[i];\n\t}\n')

        main.write('\tfprintf(fp, "\\n\\n\\n");\n')
        main.write('\tfprintf(fp, "Edge;Tasks Executed;Data Processed (MB);Energy Consumed;Average Energy Consumed;')
        main.write('Total Time;Execution Time;CPU Usage %\\n");\n\n')

        for i in range(num_edge):
            main.write('\n\t/*STATISTICS EDGE' + str(i) + '*/\n\n')
            main.write('\tfor (int j = 0; j < EDGE' + str(i) + '; j++)\n\t{\n')
            main.write('\t\tsprintf(h, edge-' + str(i) + '-%d", j);\n')
            main.write('\t\thost = MSG_host_by_name(h);\n')
            main.write('\t\tfprintf(fp,"%s;%d;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f\\n",MSG_host_get_name(host),')
            main.write('statsEdge[' + str(i) + '].numTasks[j], statsEdge[' + str(i) + '].dataProcessed[j]/(1024*1024),')
            main.write('statsEdge[' + str(i) + '].totalEnergy[j], statsEdge[' + str(i) + '].avEnergy[j], MSG_get_clock(),')
            main.write('statsEdge[' + str(i) + '].executionTime[j], statsEdge[' + str(i) + '].executionTime[j]*100/MSG_get_clock());\n')
            main.write('\t\ttaskExecuted += statsEdge[' + str(i) + '].numTasks[j];\n\t}\n')

        main.write('\tfprintf(fp, "\\n\\n\\n");\n')
        main.write('\tfprintf(fp, "Fog;Tasks Executed;Data Processed (MB);Energy Consumed;Average Energy Consumed;')
        main.write('Total Time;Execution Time;CPU Usage %\\n");\n\n')

        for i in range(num_fog):
            main.write('\n\t/*STATISTICS FOG' + str(i) + '*/\n\n')
            main.write('\tfor (int j = 0; j < FOG' + str(i) + '; j++)\n\t{\n')
            main.write('\t\tsprintf(h, "fog-' + str(i) + '-%d", j);\n')
            main.write('\t\thost = MSG_host_by_name(h);\n')
            main.write('\t\tfprintf(fp,"%s;%d;%.2f;%.2f;%.2f;%.2f;%.2f;%.2f\\n",MSG_host_get_name(host),')
            main.write('statsFog[' + str(i) + '].numTasks[j], statsFog[' + str(i) + '].dataProcessed[j]/(1024*1024),')
            main.write('statsFog[' + str(i) + '' + str(i) + '].totalEnergy[j], statsFog[' + str(i) + '].avEnergy[j], MSG_get_clock(),')
            main.write('statsFog[' + str(i) + '].executionTime[j], statsFog[' + str(i) + '].executionTime[j]*100/MSG_get_clock());\n')
            main.write('\t\ttaskExecuted += statsFog[' + str(i) + '].numTasks[j];\n\t}\n')

        main.write('\tfprintf(fp, "\\n\\n\\n");\n')
        main.write('\tfprintf(fp, "IoT Device;Tasks Executed;Energy Consumed;Average Energy Consumed\\n");\n\n')
        #main.write('Total Time;Execution Time;CPU Usage %\\n");\n\n')

        for i in range(num_iot):
            main.write('\n\t/*STATISTICS IOT' + str(i) + '*/\n\n')
            main.write('\tfor (int j = 0; j < IOT' + str(i) + '; j++)\n\t{\n')
            main.write('\t\tsprintf(h, "iot-' + str(i) + '-%d", j);\n')
            main.write('\t\thost = MSG_host_by_name(h);\n')
            main.write('\t\tfprintf(fp,"%s;%d;%.2f;%.2f\\n",MSG_host_get_name(host), statsIoT[' + str(i) + '].numTasks[j],')
            main.write('statsIoT[' + str(i) + '].totalEnergy[j], statsIoT[' + str(i) + '].avEnergy[j]);\n\t}\n\n')

        main.write('\tfprintf(fp, "\\n\\n\\n");\n')

        main.write('\tt_s = MSG_get_clock();\n')
        main.write('\tdays = (int) (t_s / (24 * 3600));\n')
        main.write('\tt_s -= (days * 24 * 3600);\n')
        main.write('\thours = (int) (t_s / 3600);\n')
        main.write('\tt_s -= (hours * 3600);\n')
        main.write('\tmin = (int) (t_s / 60);\n')
        main.write('\tt_s -= (min * 60);\n')
        main.write(
            '\tfprintf(fp, "Simulation time:; %d days; %d hours; %d min; %d seconds\\n", days, hours, min, (int)round(t_s));\n\n')

        main.write('\tt = (double)time(NULL) - t;\n')
        main.write('\tdays = (int) (t / (24 * 3600));\n')
        main.write('\tt -= (days * 24 * 3600);\n')
        main.write('\thours = (int) (t / 3600);\n')
        main.write('\tt -= (hours * 3600);\n')
        main.write('\tmin = (int) (t / 60);\n')
        main.write('\tt -= (min * 60);\n')
        main.write(
            '\tfprintf(fp, "Simulation time:; %d days; %d hours; %d min; %d seconds\\n", days, hours, min, (int)round(t));\n\n')

        for i in range(num_datacenter):
            main.write('\tfor(int i = 0; i < NODESDATACENTER' + str(i) + '; i++)\n\t{\n')
            main.write('\t\txbt_dynar_free(&tasksManagement[' + str(i) + '].client_requests[i]);\n')
            main.write('\t}\n\n')

        main.write('\tif (res == MSG_OK) return 0;\n')
        main.write('\telse return 1; \n')
        main.write('}\n')

        main.close()


if __name__ == "__main__":
    main()
