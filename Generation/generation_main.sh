#!/bin/bash



#COMANDO GENERACION
# ./generation_main.sh num_datacenter [servers output_data] num_iot_clusters [devices tasks size_data percentage arrival_rate] 

args=("$@")

DATACENTERS=$1
IOT_CLUSTERS=${args[$((($1*2)+1))]}

ARGUMENTS=$(($DATACENTERS*2 + $IOT_CLUSTERS*5 + 2))

if [ "$arg" == "--help" ] || [ "$arg" == "-h" ]  || [ "$ARGUMENTS" != "$#" ]
    then
    echo "./generation_main.sh num_datacenter [servers output_data] num_iot_clusters [devices tasks size_data percentage]"
   	exit 1
fi
rm ../main.c



echo "#include \"./Headers/simulation.h\"" 													>> ../main.c
echo ""																						>> ../main.c
echo "void test_all(char *file)"															>> ../main.c
echo "{"																					>> ../main.c
echo -e "\tint argc, i, j;"																	>> ../main.c
echo -e "\tchar str[50];"																	>> ../main.c
echo -e "\tmsg_process_t p;"																>> ../main.c
echo ""																						>> ../main.c
echo -e "\tMSG_create_environment(file);"													>> ../main.c
echo -e "\tMSG_function_register(\"iot\", iot);"											>> ../main.c
echo -e "\tMSG_function_register(\"dispatcher\", dispatcher);"								>> ../main.c
echo -e "\tMSG_function_register(\"datacenter\", datacenter);" 								>> ../main.c
echo -e "\tMSG_function_register(\"dispatcherDatacenter\", dispatcherDatacenter);"			>> ../main.c
echo -e "\tMSG_function_register(\"controller\", controller);"								>> ../main.c
echo -e ""																					>> ../main.c
echo -e ""																					>> ../main.c



#SERVERS


echo -e "\tint request_data, num_tasks, num_datacenters, output_data;"																>> ../main.c											
echo -e "\tdouble arrival, percentage;"																								>> ../main.c



for (( c=0; c<$DATACENTERS; c++ ))
do
	SERVERS=${args[$((1 + ($c*2)))]}
	echo -e ""																														>> ../main.c
	echo -e "\t//Datacenter $c"																										>> ../main.c
	echo -e ""																														>> ../main.c
	echo -e "\ti = $c;"																												>> ../main.c
	echo -e ""																														>> ../main.c

	echo -e "\tfor(j = 0; j < $SERVERS; j++)"																						>> ../main.c
	echo -e "\t{"																													>> ../main.c
	echo -e "\t\tsprintf(str, \"s-%d-%d\", i, j);"																					>> ../main.c
	echo -e "\t\targc = 2;"																											>> ../main.c
	echo -e "\t\tchar **argvc = xbt_new(char *, 3);"																				>> ../main.c
	echo ""																															>> ../main.c
	echo -e "\t\targvc[0] = bprintf(\"%d\",i);"																						>> ../main.c
	echo -e "\t\targvc[1] = bprintf(\"%d\",j);"																						>> ../main.c
	echo -e "\t\targvc[2] = NULL;"																									>> ../main.c
	echo -e ""																														>> ../main.c
	echo -e "\t\tp = MSG_process_create_with_arguments(str, datacenter, NULL, MSG_get_host_by_name(str), argc, argvc);"				>> ../main.c
	echo -e "\t\tif(p == NULL)"																										>> ../main.c
	echo -e "\t\t{"																													>> ../main.c
	echo -e "\t\t\tprintf(\"Error en ......... s-%d-%d\", i, j);"																	>> ../main.c
	echo  "printf(\"\\n\");"																										>> ../main.c
	echo -e "\t\t\texit(0);"																										>> ../main.c
	echo -e "\t\t}"																													>> ../main.c
	echo -e "\t}"																													>> ../main.c
	
done

#DISPATCHER SERVERS

for (( c=0; c<$DATACENTERS; c++ ))
do
	echo -e ""																														>> ../main.c
	SERVERS=${args[$((1 + ($c*2)))]}
	OUTPUT=${args[$((2+$c+($c)))]}

	echo -e "\t//Dispatcher Datacenter $c"																							>> ../main.c
	echo -e ""																														>> ../main.c
	echo -e "\ti = $c;"																												>> ../main.c
	echo -e ""																														>> ../main.c
	echo -e "\tfor(j = 0; j < $SERVERS; j++)"																						>> ../main.c
	echo -e "\t{"																													>> ../main.c
	echo -e "\t\tsprintf(str, \"s-%d-%d\", i, j);"																					>> ../main.c
	echo -e "\t\targc = 3;"																											>> ../main.c
	echo -e "\t\tchar **argvc = xbt_new(char *, 4);"																				>> ../main.c
	echo ""																															>> ../main.c
	echo -e "\t\toutput_data = $OUTPUT;"																							>> ../main.c											
	echo -e "\t\targvc[0] = bprintf(\"%d\",i);"																						>> ../main.c
	echo -e "\t\targvc[1] = bprintf(\"%d\",j);"																						>> ../main.c
	echo -e "\t\targvc[2] = bprintf(\"%d\",output_data);"																			>> ../main.c
	echo -e "\t\targvc[3] = NULL;"																									>> ../main.c
	echo -e ""																														>> ../main.c
	echo -e "\t\tp = MSG_process_create_with_arguments(str, dispatcherDatacenter, NULL, MSG_get_host_by_name(str), argc, argvc);"	>> ../main.c
	echo -e "\t\tif(p == NULL)"																										>> ../main.c
	echo -e "\t\t{"																													>> ../main.c
	echo -e "\t\t\tprintf(\"Error en ......... s-%d-%d\", i, j);"																	>> ../main.c
	echo  "printf(\"\\n\");"																										>> ../main.c
	echo -e "\t\t\texit(0);"																										>> ../main.c
	echo -e "\t\t}"																													>> ../main.c
	echo -e "\t}"																													>> ../main.c
done

#IOT DEVICES


for (( c=0; c<$IOT_CLUSTERS; c++ ))
do
	echo -e ""																														>> ../main.c
	DEVICES=${args[$((($1*2)+2+($c*5)))]}
	TASKS=${args[$((($1*2)+3+($c*5)))]}
	REQUEST=${args[$((($1*2)+4+($c*5)))]}
	PERCENTAGE=${args[$((($1*2)+5+($c*5)))]}
	ARRIVAL=${args[$((($1*2)+6+($c*5)))]}

	echo -e "\t//IoT Devices from cluster $c"																						>> ../main.c
	echo -e ""																														>> ../main.c
	echo -e "\ti = $c;"																												>> ../main.c	
	echo -e "\trequest_data = $REQUEST;"																							>> ../main.c											
	echo -e "\tnum_tasks = $TASKS;"																									>> ../main.c
	echo -e "\tpercentage = $PERCENTAGE;"																							>> ../main.c
	echo -e "\tnum_datacenters = $DATACENTERS;"																						>> ../main.c	
	echo -e "\tarrival = $ARRIVAL * MAX_SERVERS;"																					>> ../main.c	

	echo -e ""																														>> ../main.c	
	
	echo -e "\tfor(j = 0; j < $DEVICES; j++)"																						>> ../main.c
	echo -e "\t{"																													>> ../main.c
	echo -e "\t\tsprintf(str, \"iot-%d-%d\", i, j);"																					>> ../main.c
	echo -e "\t\targc = 7;"																											>> ../main.c
	echo -e "\t\tchar **argvc = xbt_new(char *, 8);"																				>> ../main.c
	echo ""																															>> ../main.c
	echo -e "\t\targvc[0] = bprintf(\"%d\",i);"																						>> ../main.c
	echo -e "\t\targvc[1] = bprintf(\"%d\",j);"																						>> ../main.c
	echo -e "\t\targvc[2] = bprintf(\"%d\",request_data);"																			>> ../main.c
	echo -e "\t\targvc[3] = bprintf(\"%d\",num_tasks);"																				>> ../main.c
	echo -e "\t\targvc[4] = bprintf(\"%g\",percentage);"																			>> ../main.c
	echo -e "\t\targvc[5] = bprintf(\"%d\",num_datacenters);"																		>> ../main.c
	echo -e "\t\targvc[6] = bprintf(\"%g\",arrival);"																				>> ../main.c	
	echo -e "\t\targvc[7] = NULL;"																									>> ../main.c
	echo -e ""																														>> ../main.c
	echo -e "\t\tp = MSG_process_create_with_arguments(str, iot, NULL, MSG_get_host_by_name(str), argc, argvc);"					>> ../main.c
	echo -e "\t\tif(p == NULL)"																										>> ../main.c
	echo -e "\t\t{"																													>> ../main.c
	echo -e "\t\t\tprintf(\"Error en ......... iot-%d-%d\", i, j);"																	>> ../main.c
	echo  "printf(\"\\n\");"																										>> ../main.c
	echo -e "\t\t\texit(0);"																										>> ../main.c
	echo -e "\t\t}"																													>> ../main.c
	echo -e "\t}"																													>> ../main.c
done


#DISPATCHERS


echo -e "\tint nservers;"																											>> ../main.c											



for (( c=0; c<$DATACENTERS; c++ ))
do
	echo -e ""																														>> ../main.c
	SERVERS=${args[$((1 + ($c*2)))]}

	echo -e "\t//Dispatcher $c"																										>> ../main.c
	echo -e ""																														>> ../main.c
	echo -e "\ti = $c;"																												>> ../main.c
	echo -e ""																														>> ../main.c
	echo -e "\tsprintf(str, \"d-%d-0\", i);"																						>> ../main.c
	echo -e "\targc = 2;"																											>> ../main.c
	echo -e "\tchar **argvc$c = xbt_new(char *, 3);"																				>> ../main.c
	echo ""																															>> ../main.c
	echo -e "\tnservers = $SERVERS;"																								>> ../main.c											
	echo -e "\targvc$c[0] = bprintf(\"%d\",i);"																						>> ../main.c
	echo -e "\targvc$c[1] = bprintf(\"%d\",nservers);"																				>> ../main.c
	echo -e "\targvc$c[2] = NULL;"																									>> ../main.c
	echo -e ""																														>> ../main.c
	echo -e "\tp = MSG_process_create_with_arguments(str, dispatcher, NULL, MSG_get_host_by_name(str), argc, argvc$c);"				>> ../main.c
	echo -e "\tif(p == NULL)"																										>> ../main.c
	echo -e "\t{"																													>> ../main.c
	echo -e "\t\tprintf(\"Error en ......... d-%d-0\", i);"																			>> ../main.c
	echo  "printf(\"\\n\");"																										>> ../main.c
	echo -e "\t\texit(0);"																											>> ../main.c
	echo -e "\t}"																													>> ../main.c
done





for (( j=0; j<$IOT_CLUSTERS; j++ ))
do
	TOTAL_IOT=$(($TOTAL_IOT+${args[$((($1*2)+2+($j*5)))]}))
done

echo -e ""																														>> ../main.c
echo -e ""																														>> ../main.c
echo -e "\t//Controller"																										>> ../main.c
echo -e ""																														>> ../main.c
echo -e "\tint total_devices = $TOTAL_IOT;"																						>> ../main.c
echo -e "\tint total_datacenters = $DATACENTERS;"																				>> ../main.c
echo -e ""																														>> ../main.c
echo -e "\tsprintf(str, \"cont-0\");"																							>> ../main.c
echo -e "\targc = 2;"																											>> ../main.c
echo -e "\tchar **argvc$c = xbt_new(char *, 3);"																				>> ../main.c
echo ""																															>> ../main.c
echo -e "\targvc$c[0] = bprintf(\"%d\", total_devices);"																		>> ../main.c
echo -e "\targvc$c[1] = bprintf(\"%d\", total_datacenters);"																	>> ../main.c
echo -e "\targvc$c[2] = NULL;"																									>> ../main.c
echo -e ""																														>> ../main.c
echo -e "\tp = MSG_process_create_with_arguments(str, controller, NULL, MSG_get_host_by_name(str), argc, argvc$c);"				>> ../main.c
echo -e "\tif(p == NULL)"																										>> ../main.c
echo -e "\t{"																													>> ../main.c
echo -e "\t\tprintf(\"Error en ......... cont-0\");"																			>> ../main.c
echo  "printf(\"\\n\");"																										>> ../main.c
echo -e "\t\texit(0);"																											>> ../main.c
echo -e "\t}"																													>> ../main.c



echo -e "\treturn;"																													>> ../main.c
echo -e "}"																															>> ../main.c

echo -e ""																															>> ../main.c
echo -e ""																															>> ../main.c
echo -e ""																															>> ../main.c
echo -e ""																															>> ../main.c


echo -e "int main(int argc, char *argv[])"																							>> ../main.c
echo -e "{"																															>> ../main.c
echo -e "\tmsg_error_t res = MSG_OK;"																								>> ../main.c
echo -e "\tint i, j;"																												>> ../main.c
echo -e "\tdouble t_medio_servicio = 0.0;"																							>> ../main.c
echo -e "\tdouble q_medio = 0.0;"																									>> ../main.c
echo -e "\tdouble n_medio = 0.0;"																									>> ../main.c
echo -e ""																															>> ../main.c
echo -e "\tif (argc < 2)"																											>> ../main.c
echo -e "\t{"																														>> ../main.c
echo -e "\t\tprintf(\"Usage: %s platform_file\", argv[0]);"																			>> ../main.c
echo  "printf(\"\n\");"																											>> ../main.c
echo -e "\t\texit(1);"																												>> ../main.c
echo -e "\t}"																														>> ../main.c
echo -e ""																															>> ../main.c
echo -e ""																															>> ../main.c

echo -e "\tseed((int)time(NULL));"																									>> ../main.c
echo -e "\tsg_host_energy_plugin_init();"																							>> ../main.c
echo -e "\tMSG_init(&argc, argv);"																									>> ../main.c
echo -e ""																															>> ../main.c

for (( c=0; c<$DATACENTERS; c++ ))
do
	echo -e ""																														>> ../main.c
	SERVERS=${args[$((1 + ($c*2)))]}
	echo ""																															>> ../main.c
	echo ""																															>> ../main.c
	echo -e "\tfor(j = 0; j < $SERVERS; j++)"																						>> ../main.c
	echo -e "\t{"																													>> ../main.c
	echo -e "\t\ttasksManagement[$c].Nqueue[j] = 0;"																				>> ../main.c
	echo -e "\t\ttasksManagement[$c].Nsystem[j] = 0;"																				>> ../main.c
	echo -e "\t\ttasksManagement[$c].EmptyQueue[j] = 0;"																			>> ../main.c
	echo -e "\t\ttasksManagement[$c].mutex[j] = xbt_mutex_init();"																	>> ../main.c
	echo -e "\t\ttasksManagement[$c].cond[j] = xbt_cond_init();"																	>> ../main.c											
	echo -e "\t\ttasksManagement[$c].client_requests[j] = xbt_dynar_new(sizeof(struct ClientRequest *), NULL);"						>> ../main.c																													>> ../main.c
	echo -e "\t}"																													>> ../main.c
done

echo ""																																>> ../main.c
echo -e "\ttest_all(argv[1]);"																										>> ../main.c
echo -e "\tres = MSG_main();"																										>> ../main.c
echo ""																															>> ../main.c
echo ""																															>> ../main.c
echo -e "\tFILE *fp = fopen(\"./results.csv\", \"w+\");"																										>> ../main.c

echo -e "\tchar h[30];"																										>> ../main.c
echo -e "\tmsg_host_t host;"																										>> ../main.c


echo "fprintf(fp, \"Server,Tasks Executed,Energy Consumed,Average Energy Consumed,Average Time\n\");"																										>> ../main.c


for (( c=0; c<$DATACENTERS; c++ ))
do
	echo -e ""																														>> ../main.c
	SERVERS=${args[$((1 + ($c*2)))]}
	echo ""																															>> ../main.c
	echo ""																															>> ../main.c
	echo -e "\ti = $c;"																												>> ../main.c
	echo -e "\tfor(j = 0; j < $SERVERS; j++)"																						>> ../main.c
	echo -e "\t{"																													>> ../main.c
	echo -e "\t\tq_medio = q_medio + tasksManagement[$c].Navgqueue[j];"																				>> ../main.c
	echo -e "\t\tn_medio = n_medio + tasksManagement[$c].Navgsystem[j];"																			>> ../main.c
	echo -e "\t\tsprintf(h, \"s-%d-%d\", i, j);"																									>> ../main.c
	echo -e "\t\thost = MSG_host_by_name(h);"																										>> ../main.c
	echo "fprintf(fp,\"%s,%d,%.6f,%g\n\",MSG_host_get_name(host), statsDatacenter[i].numTasks[j], statsDatacenter[i].totalEnergy[j], statsDatacenter[i].avEnergy[j], statsDatacenter[i].avTime[j]);"													>> ../main.c


	echo -e "\t}"																																	>> ../main.c
	
	echo ""																																			>> ../main.c
	echo -e "\tt_medio_servicio = avServTime[i].avServiceTime / (avServTime[i].numTasks);"															>> ../main.c
	echo "fprintf(fp,\"\n\");"																														>> ../main.c
	echo "fprintf(fp,\"DATACENTER,tiempoMedioServicio,TamañoMediocola,TareasMediasEnElSistema,tareas\n\");"											>> ../main.c
	echo "fprintf(fp,\"%i,%g,%g,%g,%d\n\n\", i, t_medio_servicio, q_medio, n_medio, avServTime[i].numTasks);"										>> ../main.c

	echo ""																																			>> ../main.c


	echo -e "\tt_medio_servicio = 0;"																												>> ../main.c
	echo -e "\tq_medio = 0;"																														>> ../main.c
	echo -e "\tn_medio = 0;"																														>> ../main.c
done

echo -e ""																															>> ../main.c
echo "fprintf(fp,\"\n\n\n\n\");"																														>> ../main.c
echo -e ""																															>> ../main.c
echo -e ""																															>> ../main.c

echo "fprintf(fp, \"IoT Device,Tasks Executed,Energy Consumed,Average Energy Consumed,Average Time\n\");"																										>> ../main.c
echo -e ""																															>> ../main.c
echo -e ""																															>> ../main.c
echo -e ""																															>> ../main.c


for (( c=0; c<$IOT_CLUSTERS; c++ ))
do
	echo -e ""																														>> ../main.c
	DEVICES=${args[$((($1*2)+2+($c*5)))]}
	echo ""																															>> ../main.c
	echo ""																															>> ../main.c
	echo -e "\ti = $c;"																												>> ../main.c
	echo -e "\tfor(j = 0; j < $DEVICES; j++)"																						>> ../main.c
	echo -e "\t{"																													>> ../main.c
	echo -e "\t\tsprintf(h, \"iot-%d-%d\", i, j);"																									>> ../main.c
	echo -e "\t\thost = MSG_host_by_name(h);"																										>> ../main.c
	echo "fprintf(fp,\"%s,%d,%.6f,%g\n\",MSG_host_get_name(host), statsIoT[i].numTasks[j], statsIoT[i].totalEnergy[j], statsIoT[i].avEnergy[j], statsIoT[i].avTime[j]);"													>> ../main.c


	echo -e "\t}"																																	>> ../main.c
	
	echo ""																																			>> ../main.c
	echo ""																																			>> ../main.c
done

echo -e ""																															>> ../main.c
echo "fprintf(fp,\"\n\n\n\");"																														>> ../main.c

echo -e ""																															>> ../main.c
echo -e ""																															>> ../main.c
echo -e ""																															>> ../main.c



echo "fprintf(fp,\"Simulation time %g\n\", MSG_get_clock());"																		>> ../main.c
echo -e "\tfclose(fp);"																												>> ../main.c

for (( c=0; c<$DATACENTERS; c++ ))
do
	echo -e ""																														>> ../main.c
	SERVERS=${args[$((1 + ($c*2)))]}																								>> ../main.c
	echo ""																															>> ../main.c
	echo ""																															>> ../main.c
	echo -e "\tfor(j = 0; j < $SERVERS; j++)"																						>> ../main.c
	echo -e "\t{"																													>> ../main.c
	echo -e "\t\txbt_dynar_free(&tasksManagement[$c].client_requests[j]);"															>> ../main.c
	echo -e "\t}"																													>> ../main.c
done

echo ""																																>> ../main.c
echo ""																																>> ../main.c

echo -e "\tif (res == MSG_OK) return 0;"																							>> ../main.c
echo -e "\telse return 1;"																											>> ../main.c

echo -e "}"																															>> ../main.c

