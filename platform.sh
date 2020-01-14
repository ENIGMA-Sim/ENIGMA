#!/bin/bash

rm ./platform-cluster.xml

if [ "$arg" == "--help" ] || [ "$arg" == "-h" ] || [ "$#" != 5 ]
    then
    echo "./platform.sh num_datacenters servers_on_datacenters iot_clusters iot_devices fog_devices"
   	exit 1
fi



DATACENTERS=$1
SERVERS_ON_DATACENTERS=$2
IOT_CLUSTERS=$3
IOT_DEVICES=$4
FOG_COMPUTING=$5

echo "<?xml version='1.0'?>" 																>> platform-cluster.xml
echo "<!DOCTYPE platform SYSTEM \"http://simgrid.gforge.inria.fr/simgrid/simgrid.dtd\">" 	>> platform-cluster.xml
echo "<platform version=\"4.1\">" 															>> platform-cluster.xml
echo																						>> platform-cluster.xml
echo -e "\t<AS  id=\"AS0\"  routing=\"Full\">" 												>> platform-cluster.xml


for (( c=0; c<$IOT_CLUSTERS; c++ ))
do
	echo -e "\t\t<cluster id=\"IoT$c\" prefix=\"c-$c-\" suffix=\"\" radical=\"0-$(($IOT_DEVICES-1))\"" 										>> platform-cluster.xml
	echo -e "\t\tspeed=\"4Gf,2Gf,1Gf\" core=\"4\" bw=\"10Mbps\" lat=\"10ms\" bb_bw=\"1000000000GBps\""										>> platform-cluster.xml	
	echo -e	"\t\tbb_lat=\"10ms\" router_id=\"IoT_cluster$c\">" 																				>> platform-cluster.xml
	echo -e "\t\t\t<prop id=\"watt_per_state\" value=\"100.0:120.0:200.0, 93.0:110.0:170.0, 90.0:105.0:150.0\" />"							>> platform-cluster.xml
	echo -e "\t\t\t<prop id=\"wattage_off\" value=\"10\"/>"											>> platform-cluster.xml
	echo -e "\t\t</cluster>"																		>> platform-cluster.xml
	echo																							>> platform-cluster.xml
done


for (( c=0; c<$DATACENTERS; c++ ))
do
	echo -e "\t\t<cluster id=\"Dispatcher$c\" prefix=\"d-$c-\" suffix=\"\" radical=\"0-1\"" 												>> platform-cluster.xml
	echo -e "\t\tspeed=\"8Gf,4Gf,2Gf\" core=\"4\" bw=\"125Gbps\" lat=\"0us\" bb_bw=\"1000000000GBps\""		 								>> platform-cluster.xml
	echo -e	"\t\tbb_lat=\"0us\" router_id=\"Dispatcher_cluster$c\">"																		>> platform-cluster.xml
	echo -e "\t\t\t<prop id=\"watt_per_state\" value=\"100.0:120.0:200.0, 93.0:110.0:170.0, 90.0:105.0:150.0\" />"							>> platform-cluster.xml
	echo -e "\t\t\t<prop id=\"wattage_off\" value=\"10\"/>"											>> platform-cluster.xml
	echo -e "\t\t</cluster>"																		>> platform-cluster.xml
	echo																							>> platform-cluster.xml	
done


for (( c=0; c<$DATACENTERS; c++ ))
do
	echo -e "\t\t<cluster id=\"Datacenter$c\" prefix=\"s-$c-\" suffix=\"\" radical=\"0-$(($SERVERS_ON_DATACENTERS-1))\"" 					>> platform-cluster.xml
	echo -e "\t\tspeed=\"10Gf,8Gf,4Gf\" core=\"8\" bw=\"125Gbps\" lat=\"0us\" bb_bw=\"10000000GBps\""		 								>> platform-cluster.xml
	echo -e	"\t\tbb_lat=\"0us\" router_id=\"Datacenter_cluster$c\">"																		>> platform-cluster.xml
	echo -e "\t\t\t<prop id=\"watt_per_state\" value=\"100.0:120.0:200.0, 93.0:110.0:170.0, 90.0:105.0:150.0\" />"							>> platform-cluster.xml
	echo -e "\t\t\t<prop id=\"wattage_off\" value=\"10\"/>"											>> platform-cluster.xml
	echo -e "\t\t</cluster>"																		>> platform-cluster.xml
	echo																							>> platform-cluster.xml		
done



if [ "$FOG_COMPUTING" == 0 ]
then
	for (( j=0; j<$DATACENTERS; j++ ))
	do
		for (( i=0; i<$IOT_CLUSTERS; i++ ))
		do
			echo -e "\t\t<link id=\"linkIoT$i""Dispatcher$j\" bandwidth=\"100000000GBps\" latency=\"50us\" />"						>> platform-cluster.xml
			echo 																													>> platform-cluster.xml
		done
	done

	for (( i=0; i<$DATACENTERS; i++ ))
	do
		echo -e "\t\t<link id=\"linkDispatcher$i""Datacenter$i\" bandwidth=\"100000000GBps\" latency=\"50us\" />"							>> platform-cluster.xml
		echo 																																>> platform-cluster.xml
	done	

	for (( i=0; i<$DATACENTERS; i++ ))
	do
		for (( j=0; j<$IOT_CLUSTERS; j++ ))
		do
			echo -e "\t\t<link id=\"linkDatacenter$i""IoT$j\" bandwidth=\"100000000GBps\" latency=\"50us\" />"						>> platform-cluster.xml
			echo 																													>> platform-cluster.xml
		done
	done



	for (( j=0; j<$DATACENTERS; j++ ))
	do
		for (( i=0; i<$IOT_CLUSTERS; i++ ))
		do
			echo -e "\t\t<ASroute src=\"IoT$i\" dst=\"Dispatcher$j\" gw_src=\"IoT_cluster$i\" gw_dst=\"Dispatcher_cluster$j\">"		>> platform-cluster.xml
			echo -e "\t\t\t<link_ctn id=\"linkIoT$i""Dispatcher$j\" />"																>> platform-cluster.xml
			echo -e "\t\t</ASroute>"																								>> platform-cluster.xml
			echo 																													>> platform-cluster.xml
		done
	done

	for (( i=0; i<$DATACENTERS; i++ ))
	do
		echo -e "\t\t<ASroute src=\"Dispatcher$i\" dst=\"Datacenter$i\" gw_src=\"Dispatcher_cluster$i\" gw_dst=\"Datacenter_cluster$i\">"	>> platform-cluster.xml
		echo -e "\t\t\t<link_ctn id=\"linkDispatcher$i""Datacenter$i\" />"																	>> platform-cluster.xml
		echo -e "\t\t</ASroute>"																											>> platform-cluster.xml
		echo 																																>> platform-cluster.xml
	done	

	for (( i=0; i<$DATACENTERS; i++ ))
	do
		for (( j=0; j<$IOT_CLUSTERS; j++ ))
		do
			echo -e "\t\t<ASroute src=\"Datacenter$i\" dst=\"IoT$j\" gw_src=\"Datacenter_cluster$i\" gw_dst=\"IoT_cluster$j\">"		>> platform-cluster.xml
			echo -e "\t\t\t<link_ctn id=\"linkDatacenter$i""IoT$j\" />"																>> platform-cluster.xml
			echo -e "\t\t</ASroute>"																								>> platform-cluster.xml
			echo 																													>> platform-cluster.xml
		done
	done


	str=`grep '#define NUM_DATACENTERS' simulation.c`
	sed -i "s/$str/#define NUM_DATACENTERS $DATACENTERS/g" simulation.c


	str=`grep '#define NUM_SERVERS_PER_DATACENTER' simulation.c`
	sed -i "s/$str/#define NUM_SERVERS_PER_DATACENTER $SERVERS_ON_DATACENTERS/g" simulation.c


	str=`grep '#define NUM_DISPATCHERS' simulation.c`
	sed -i "s/$str/#define NUM_DISPATCHERS $DATACENTERS/g" simulation.c
	

	str=`grep '#define NUM_IOT_CLUSTERS' simulation.c`
	sed -i "s/$str/#define NUM_IOT_CLUSTERS $IOT_CLUSTERS/g" simulation.c 


	str=`grep '#define NUM_DEVICES_PER_IOT_CLUSTER' simulation.c`
	sed -i "s/$str/#define NUM_DEVICES_PER_IOT_CLUSTER $IOT_DEVICES/g" simulation.c

fi

echo -e "\t</AS>" >> platform-cluster.xml
echo "</platform>" 	>> platform-cluster.xml