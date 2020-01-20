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
rm ./prueba.c



echo "#include \"./Headers/simulation.h\"" 															>> prueba.c
echo ""																						>> prueba.c
echo "void test_all(char *file)"															>> prueba.c
echo "{"																					>> prueba.c
echo -e "\tint argc, i, j;"																	>> prueba.c
echo -e "\tchar str[50];"																	>> prueba.c
echo -e "\tmsg_process_t p;"																>> prueba.c
echo ""																						>> prueba.c
echo -e "\tMSG_create_environment(file);"													>> prueba.c
echo -e "\tMSG_function_register(\"iot\", iot);"											>> prueba.c
echo -e "\tMSG_function_register(\"dispatcher\", dispatcher);"								>> prueba.c
echo -e "\tMSG_function_register(\"datacenter\", datacenter);" 								>> prueba.c
echo -e "\tMSG_function_register(\"dispatcherDatacenter\", dispatcherDatacenter);"			>> prueba.c
echo -e ""																					>> prueba.c
echo -e ""																					>> prueba.c



#SERVERS


echo -e "\tint request_data, num_tasks, num_datacenters, output_data;"																						>> prueba.c											
echo -e "\tdouble arrival, percentage;"																																				>> prueba.c



for (( c=0; c<$DATACENTERS; c++ ))
do
	SERVERS=${args[$((1 + ($c*2)))]}
	echo -e ""																														>> prueba.c
	echo -e "\t//Datacenter $c"																										>> prueba.c
	echo -e ""																														>> prueba.c
	echo -e "\ti = $c;"																												>> prueba.c
	echo -e ""																														>> prueba.c

	echo -e "\tfor(j = 0; j < $SERVERS; j++)"																						>> prueba.c
	echo -e "\t{"																													>> prueba.c
	echo -e "\t\tsprintf(str, \"s-%d-%d\", i, j);"																						>> prueba.c
	echo -e "\t\targc = 2;"																											>> prueba.c
	echo -e "\t\tchar **argvc = xbt_new(char *, 3);"																				>> prueba.c
	echo ""																															>> prueba.c
	echo -e "\t\targvc[0] = bprintf(\"%d\",i);"																						>> prueba.c
	echo -e "\t\targvc[1] = bprintf(\"%d\",j);"																						>> prueba.c
	echo -e "\t\targvc[2] = NULL;"																									>> prueba.c
	echo -e ""																														>> prueba.c
	echo -e "\t\tp = MSG_process_create_with_arguments(str, datacenter, NULL, MSG_get_host_by_name(str), argc, argvc);"				>> prueba.c
	echo -e "\t\tif(p == NULL)"																										>> prueba.c
	echo -e "\t\t{"																													>> prueba.c
	echo -e "\t\t\tprintf(\"Error en ......... s-%d-%d\", i, j);"																	>> prueba.c
	echo  "printf(\"\\n\");"																										>> prueba.c
	echo -e "\t\t\texit(0);"																										>> prueba.c
	echo -e "\t\t}"																													>> prueba.c
	echo -e "\t}"																													>> prueba.c
	
done

#DISPATCHER SERVERS

for (( c=0; c<$DATACENTERS; c++ ))
do
	echo -e ""																														>> prueba.c
	SERVERS=${args[$((1 + ($c*2)))]}
	OUTPUT=${args[$((2+$c+($c)))]}

	echo -e "\t//Dispatcher Datacenter $c"																							>> prueba.c
	echo -e ""																														>> prueba.c
	echo -e "\ti = $c;"																												>> prueba.c
	echo -e ""																														>> prueba.c
	echo -e "\tfor(j = 0; j < $SERVERS; j++)"																						>> prueba.c
	echo -e "\t{"																													>> prueba.c
	echo -e "\t\tsprintf(str, \"s-%d-%d\", i, j);"																						>> prueba.c
	echo -e "\t\targc = 3;"																											>> prueba.c
	echo -e "\t\tchar **argvc = xbt_new(char *, 4);"																				>> prueba.c
	echo ""																															>> prueba.c
	echo -e "\t\toutput_data = $OUTPUT;"																							>> prueba.c											
	echo -e "\t\targvc[0] = bprintf(\"%d\",i);"																						>> prueba.c
	echo -e "\t\targvc[1] = bprintf(\"%d\",j);"																						>> prueba.c
	echo -e "\t\targvc[2] = bprintf(\"%d\",output_data);"																			>> prueba.c
	echo -e "\t\targvc[3] = NULL;"																									>> prueba.c
	echo -e ""																														>> prueba.c
	echo -e "\t\tp = MSG_process_create_with_arguments(str, dispatcherDatacenter, NULL, MSG_get_host_by_name(str), argc, argvc);"	>> prueba.c
	echo -e "\t\tif(p == NULL)"																										>> prueba.c
	echo -e "\t\t{"																													>> prueba.c
	echo -e "\t\t\tprintf(\"Error en ......... s-%d-%d\", i, j);"																	>> prueba.c
	echo  "printf(\"\\n\");"																										>> prueba.c
	echo -e "\t\t\texit(0);"																										>> prueba.c
	echo -e "\t\t}"																													>> prueba.c
	echo -e "\t}"																													>> prueba.c
done

#IOT DEVICES


for (( c=0; c<$IOT_CLUSTERS; c++ ))
do
	echo -e ""																														>> prueba.c
	DEVICES=${args[$((($1*2)+2+($c*5)))]}
	TASKS=${args[$((($1*2)+3+($c*5)))]}
	REQUEST=${args[$((($1*2)+4+($c*5)))]}
	PERCENTAGE=${args[$((($1*2)+5+($c*5)))]}
	ARRIVAL=${args[$((($1*2)+6+($c*5)))]}

	echo -e "\t//IoT Devices from cluster $c"																						>> prueba.c
	echo -e ""																														>> prueba.c
	echo -e "\ti = $c;"																												>> prueba.c	
	echo -e "\trequest_data = $REQUEST;"																							>> prueba.c											
	echo -e "\tnum_tasks = $TASKS;"																									>> prueba.c
	echo -e "\tpercentage = $PERCENTAGE;"																							>> prueba.c
	echo -e "\tnum_datacenters = $DATACENTERS;"																						>> prueba.c	
	echo -e "\tarrival = $ARRIVAL * MAX_SERVERS;"																					>> prueba.c	

	echo -e ""																														>> prueba.c	
	
	echo -e "\tfor(j = 0; j < $DEVICES; j++)"																						>> prueba.c
	echo -e "\t{"																													>> prueba.c
	echo -e "\t\tsprintf(str, \"c-%d-%d\", i, j);"																						>> prueba.c
	echo -e "\t\targc = 7;"																											>> prueba.c
	echo -e "\t\tchar **argvc = xbt_new(char *, 8);"																				>> prueba.c
	echo ""																															>> prueba.c
	echo -e "\t\targvc[0] = bprintf(\"%d\",i);"																						>> prueba.c
	echo -e "\t\targvc[1] = bprintf(\"%d\",j);"																						>> prueba.c
	echo -e "\t\targvc[2] = bprintf(\"%d\",request_data);"																			>> prueba.c
	echo -e "\t\targvc[3] = bprintf(\"%d\",num_tasks);"																				>> prueba.c
	echo -e "\t\targvc[4] = bprintf(\"%d\",percentage);"																			>> prueba.c
	echo -e "\t\targvc[5] = bprintf(\"%d\",num_datacenters);"																		>> prueba.c
	echo -e "\t\targvc[6] = bprintf(\"%g\",arrival);"																				>> prueba.c	
	echo -e "\t\targvc[7] = NULL;"																									>> prueba.c
	echo -e ""																														>> prueba.c
	echo -e "\t\tp = MSG_process_create_with_arguments(str, iot, NULL, MSG_get_host_by_name(str), argc, argvc);"					>> prueba.c
	echo -e "\t\tif(p == NULL)"																										>> prueba.c
	echo -e "\t\t{"																													>> prueba.c
	echo -e "\t\t\tprintf(\"Error en ......... c-%d-%d\", i, j);"																	>> prueba.c
	echo  "printf(\"\\n\");"																										>> prueba.c
	echo -e "\t\t\texit(0);"																										>> prueba.c
	echo -e "\t\t}"																													>> prueba.c
	echo -e "\t}"																													>> prueba.c
done


#DISPATCHERS


echo -e "\tint nservers;"																							>> prueba.c											



for (( c=0; c<$DATACENTERS; c++ ))
do
	echo -e ""																														>> prueba.c
	SERVERS=${args[$((1 + ($c*2)))]}

	echo -e "\t//Dispatcher $c"																										>> prueba.c
	echo -e ""																														>> prueba.c
	echo -e "\ti = $c;"																												>> prueba.c
	echo -e ""																														>> prueba.c
																														>> prueba.c
	echo -e "\tsprintf(str, \"d-%d-0\", i);"																						>> prueba.c
	echo -e "\targc = 2;"																											>> prueba.c
	echo -e "\tchar **argvc$c = xbt_new(char *, 3);"																				>> prueba.c
	echo ""																															>> prueba.c
	echo -e "\tnservers = $SERVERS;"																							>> prueba.c											
	echo -e "\targvc$c[0] = bprintf(\"%d\",i);"																						>> prueba.c
	echo -e "\targvc$c[1] = bprintf(\"%d\",nservers);"																				>> prueba.c
	echo -e "\targvc$c[2] = NULL;"																									>> prueba.c
	echo -e ""																														>> prueba.c
	echo -e "\tp = MSG_process_create_with_arguments(str, dispatcher, NULL, MSG_get_host_by_name(str), argc, argvc$c);"				>> prueba.c
	echo -e "\tif(p == NULL)"																										>> prueba.c
	echo -e "\t{"																													>> prueba.c
	echo -e "\t\tprintf(\"Error en ......... d-%d-0\", i);"																		>> prueba.c
	echo  "printf(\"\\n\");"																										>> prueba.c
	echo -e "\t\texit(0);"																										>> prueba.c
	echo -e "\t}"																													>> prueba.c
done

echo -e "\treturn;"																													>> prueba.c
echo -e "}"																															>> prueba.c

echo -e ""																															>> prueba.c
echo -e ""																															>> prueba.c
echo -e ""																															>> prueba.c
echo -e ""																															>> prueba.c


echo -e "int main(int argc, char *argv[])"																								>> prueba.c
echo -e "{"																															>> prueba.c
echo -e "\tmsg_error_t res = MSG_OK;"																								>> prueba.c
echo -e "\tint i, j;"																												>> prueba.c
echo -e "\tdouble t_medio_servicio = 0.0;"																							>> prueba.c
echo -e "\tdouble q_medio = 0.0;"																									>> prueba.c
echo -e "\tdouble n_medio = 0.0;"																									>> prueba.c
echo -e ""																															>> prueba.c
echo -e "\tif (argc < 2)"																											>> prueba.c
echo -e "\t{"																														>> prueba.c
echo -e "\t\tprintf(\"Usage: %s platform_file\", argv[0]);"																	>> prueba.c
echo  "printf(\"\\n\");"																											>> prueba.c
echo -e "\t\texit(1);"																												>> prueba.c
echo -e "\t}"																														>> prueba.c
echo -e ""																															>> prueba.c
echo -e ""																															>> prueba.c

echo -e "\tseed((int)time(NULL));"																									>> prueba.c
echo -e "\tsg_host_energy_plugin_init();"																							>> prueba.c
echo -e "\tMSG_init(&argc, argv);"																									>> prueba.c
echo -e ""																															>> prueba.c

for (( c=0; c<$DATACENTERS; c++ ))
do
	echo -e ""																														>> prueba.c
	SERVERS=${args[$((1 + ($c*2)))]}
	echo ""																															>> prueba.c
	echo ""																															>> prueba.c
	echo -e "\tfor(j = 0; j < $SERVERS; j++)"																						>> prueba.c
	echo -e "\t{"																													>> prueba.c
	echo -e "\t\ttasksManagement[$c].Nqueue[j] = 0;"																				>> prueba.c
	echo -e "\t\ttasksManagement[$c].Nsystem[j] = 0;"																				>> prueba.c
	echo -e "\t\ttasksManagement[$c].EmptyQueue[j] = 0;"																			>> prueba.c
	echo -e "\t\ttasksManagement[$c].mutex[j] = xbt_mutex_init();"																	>> prueba.c
	echo -e "\t\ttasksManagement[$c].cond[j] = xbt_cond_init();"																	>> prueba.c											
	echo -e "\t\ttasksManagement[$c].client_requests[j] = xbt_dynar_new(sizeof(struct ClientRequest *), NULL);"						>> prueba.c																													>> prueba.c
	echo -e "\t}"																													>> prueba.c
done

echo ""																																>> prueba.c
echo -e "\ttest_all(argv[1]);"																										>> prueba.c
echo -e "\tres = MSG_main();"																										>> prueba.c
echo -e "\tint totaltasks = 0;"																										>> prueba.c



for (( c=0; c<$DATACENTERS; c++ ))
do
	echo -e ""																														>> prueba.c
	SERVERS=${args[$((1 + ($c*2)))]}
	echo ""																															>> prueba.c
	echo ""																															>> prueba.c
	echo -e "\ti = $c;"																												>> prueba.c
	echo -e "\tfor(j = 0; j < $SERVERS; j++)"																						>> prueba.c
	echo -e "\t{"																													>> prueba.c
	echo -e "\t\tq_medio = q_medio + tasksManagement[$c].Navgqueue[j];"																				>> prueba.c
	echo -e "\t\tn_medio = n_medio + tasksManagement[$c].Navgsystem[j];"																				>> prueba.c
	echo -e "\t}"																																	>> prueba.c

	echo "printf(\"DATACENTER \t tiempoMedioServicio \t TamañoMediocola \t    TareasMediasEnElSistema  \t   tareas\n\");"							>> prueba.c
	echo "printf(\"%i \t\t %g \t\t\t %g \t\t\t  %g  \t\t\t  %d \n\n\", i, t_medio_servicio, q_medio, n_medio, avServTime[i].numTasks);"				>> prueba.c
	echo -e "\tt_medio_servicio = 0;"																												>> prueba.c
	echo -e "\tq_medio = 0;"																														>> prueba.c
	echo -e "\tn_medio = 0;"																														>> prueba.c
	echo -e "\ttotaltasks += avServTime[i].numTasks;"																								>> prueba.c
done

echo -e ""																															>> prueba.c
echo "printf(\"Simulation time %g\n\", MSG_get_clock());"																			>> prueba.c


for (( c=0; c<$DATACENTERS; c++ ))
do
	echo -e ""																														>> prueba.c
	SERVERS=${args[$((1 + ($c*2)))]}																									>> prueba.c
	echo ""																															>> prueba.c
	echo ""																															>> prueba.c
	echo -e "\tfor(j = 0; j < $SERVERS; j++)"																						>> prueba.c
	echo -e "\t{"																													>> prueba.c
	echo -e "\t\txbt_dynar_free(&tasksManagement[$c].client_requests[j]);"															>> prueba.c
	echo -e "\t}"																													>> prueba.c
done

echo ""																																>> prueba.c
echo ""																																>> prueba.c

echo -e "\tif (res == MSG_OK) return 0;"																							>> prueba.c
echo -e "\telse return 1;"																											>> prueba.c

echo -e "}"																															>> prueba.c

