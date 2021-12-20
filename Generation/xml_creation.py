import csv
import xml.etree.ElementTree as xml
import xml.dom.minidom as xmldom
import sys


def main():
    filename = str(sys.argv[2])
    platform = xml.Element("platform")
    platform.set('version', '4.1')

    AS = xml.SubElement(platform, 'AS')
    AS.set('id', 'AS0')
    AS.set('routing', 'Full')

    with open(sys.argv[1]) as csv_file:
        csv_reader = csv.reader(csv_file, delimiter='|')

        rows = [r for r in csv_reader]

        #num_datacenter = int(rows[1][1])
        #num_fog = int(rows[num_datacenter * 12 + 1][1])
        #num_edge = int(rows[num_datacenter * 12 + num_fog * 12 + 2][1])
        #num_iot = int(rows[num_datacenter * 12 + num_fog * 12 + num_edge * 12 + 3][1])

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


        list_ids = []
        dict_routerid = {}

        for i in range(num_datacenter):
            comp_cl = "name_cloud" + str(i)
            pos = 0

            for line in rows:
                if comp_cl in line:
                    break
                pos = pos + 1

            id = rows[pos][1]

            list_ids.append(id)

            servers = rows[pos + 1][1]
            core = rows[pos + 2][1]
            bw = rows[pos + 3][1]
            latency = rows[pos + 4][1]
            speed = rows[pos + 5][1]
            bb_bw = rows[pos + 6][1]
            bb_lat = rows[pos + 7][1]
            wp = rows[pos + 8][1]
            woff = rows[pos + 9][1]
            ram = rows[pos + 10][1]

            cluster = xml.SubElement(AS, 'cluster')
            cluster.set('id', id)
            prefix = "cloud-" + str(i) + "-"
            cluster.set('prefix', prefix)
            cluster.set('suffix', "")
            cluster.set('radical', '0-' + str(int(servers) - 1))
            cluster.set('speed', speed)
            cluster.set('core', core)
            cluster.set('bw', bw)
            cluster.set('lat', latency)
            cluster.set('bb_bw', bb_bw)
            cluster.set('bb_lat', bb_lat)
            cluster.set('router_id', "Cloud_platform" + str(i))

            dict_routerid[id] = "Cloud_platform" + str(i)

            prop_cloud = xml.SubElement(cluster, 'prop')
            prop_cloud.set('id', 'wattage_per_state')
            prop_cloud.set('value', wp)

            prop_cloud = xml.SubElement(cluster, 'prop')
            prop_cloud.set('id', 'wattage_off')
            prop_cloud.set('value', woff)

            prop_cloud = xml.SubElement(cluster, 'prop')
            prop_cloud.set('id', 'ram')
            prop_cloud.set('value', ram)

        for j in range(num_fog):
            comp_fog = "name_fog" + str(j)
            pos = 0

            for line in rows:
                if comp_fog in line:
                    break
                pos = pos + 1

            id_fog = rows[pos][1]

            list_ids.append(id_fog)

            servers_fog = rows[pos + 1][1]
            core_fog = rows[pos + 2][1]
            bw_fog = rows[pos + 3][1]
            latency_fog = rows[pos + 4][1]
            speed_fog = rows[pos + 5][1]
            bb_bw_fog = rows[pos + 6][1]
            bb_lat_fog = rows[pos + 7][1]
            wp_fog = rows[pos + 8][1]
            woff_fog = rows[pos + 9][1]
            ram_fog = rows[pos + 10][1]

            cluster_fog = xml.SubElement(AS, 'cluster')
            cluster_fog.set('id', id_fog)
            prefix_fog = "fog-" + str(j) + "-"
            cluster_fog.set('prefix', prefix_fog)
            cluster_fog.set('suffix', "")
            cluster_fog.set('radical', '0-' + str(int(servers_fog) - 1))
            cluster_fog.set('speed', speed_fog)
            cluster_fog.set('core', core_fog)
            cluster_fog.set('bw', bw_fog)
            cluster_fog.set('lat', latency_fog)
            cluster_fog.set('bb_bw', bb_bw_fog)
            cluster_fog.set('bb_lat', bb_lat_fog)
            cluster_fog.set('router_id', "Fog_platform" + str(j))

            dict_routerid[id_fog] = "Fog_platform" + str(j)

            prop_fog = xml.SubElement(cluster_fog, 'prop')
            prop_fog.set('id', 'wattage_per_state')
            prop_fog.set('value', wp_fog)

            prop_fog = xml.SubElement(cluster_fog, 'prop')
            prop_fog.set('id', 'wattage_off')
            prop_fog.set('value', woff_fog)

            prop_fog = xml.SubElement(cluster_fog, 'prop')
            prop_fog.set('id', 'ram')
            prop_fog.set('value', ram_fog)

        for k in range(num_edge):
            comp_edge = "name_edge" + str(k)
            pos = 0

            for line in rows:
                if comp_edge in line:
                    break
                pos = pos + 1

            id_edge = rows[pos][1]
            list_ids.append(id_edge)

            servers_edge = rows[pos + 1][1]
            core_edge = rows[pos + 2][1]
            bw_edge = rows[pos + 3][1]
            latency_edge = rows[pos + 4][1]
            speed_edge = rows[pos + 5][1]
            bb_bw_edge = rows[pos + 6][1]
            bb_lat_edge = rows[pos + 7][1]
            wp_edge = rows[pos + 8][1]
            woff_edge = rows[pos + 9][1]
            ram_edge = rows[pos + 10][1]

            cluster_edge = xml.SubElement(AS, 'cluster')
            cluster_edge.set('id', id_edge)
            prefix_edge = "edge-" + str(k) + "-"
            cluster_edge.set('prefix', prefix_edge)
            cluster_edge.set('suffix', "")
            cluster_edge.set('radical', '0-' + str(int(servers_edge) - 1))
            cluster_edge.set('speed', speed_edge)
            cluster_edge.set('core', core_edge)
            cluster_edge.set('bw', bw_edge)
            cluster_edge.set('lat', latency_edge)
            cluster_edge.set('bb_bw', bb_bw_edge)
            cluster_edge.set('bb_lat', bb_lat_edge)
            cluster_edge.set('router_id', "Edge_platform" + str(k))

            dict_routerid[id_edge] = "Edge_platform" + str(k)

            prop_edge = xml.SubElement(cluster_edge, 'prop')
            prop_edge.set('id', 'wattage_per_state')
            prop_edge.set('value', wp_edge)

            prop_edge = xml.SubElement(cluster_edge, 'prop')
            prop_edge.set('id', 'wattage_off')
            prop_edge.set('value', woff_edge)

            prop_edge = xml.SubElement(cluster_edge, 'prop')
            prop_edge.set('id', 'ram')
            prop_edge.set('value', ram_edge)

        
        for l in range(num_iot):
            comp_iot = "name_iot" + str(l)
            pos = 0

            for line in rows:
                if comp_iot in line:
                    break
                pos = pos + 1
            
            id_iot = rows[pos][1]
            list_ids.append(id_iot)

            servers_iot = rows[pos + 1][1]
            core_iot = rows[pos + 2][1]
            bw_iot = rows[pos + 3][1]
            latency_iot = rows[pos + 4][1]
            speed_iot = rows[pos + 5][1]
            bb_bw_iot = rows[pos + 6][1]
            bb_lat_iot = rows[pos + 7][1]
            wp_iot = rows[pos + 8][1]
            woff_iot = rows[pos + 9][1]
            ram_iot = rows[pos + 10][1]

            cluster_iot = xml.SubElement(AS, 'cluster')
            cluster_iot.set('id', id_iot)
            prefix_iot = "iot-" + str(l) + "-"
            cluster_iot.set('prefix', prefix_iot)
            cluster_iot.set('suffix', "")
            cluster_iot.set('radical', '0-' + str(int(servers_iot) - 1))
            cluster_iot.set('speed', speed_iot)
            cluster_iot.set('core', core_iot)
            cluster_iot.set('bw', bw_iot)
            cluster_iot.set('lat', latency_iot)
            cluster_iot.set('bb_bw', bb_bw_iot)
            cluster_iot.set('bb_lat', bb_lat_iot)
            cluster_iot.set('router_id', "IoT_Cluster" + str(l))

            dict_routerid[id_iot] = "IoT_Cluster" + str(l)

            prop_iot = xml.SubElement(cluster_iot, 'prop')
            prop_iot.set('id', 'wattage_per_state')
            prop_iot.set('value', wp_iot)

            prop_iot = xml.SubElement(cluster_iot, 'prop')
            prop_iot.set('id', 'wattage_off')
            prop_iot.set('value', woff_iot)

            prop_iot = xml.SubElement(cluster_iot, 'prop')
            prop_iot.set('id', 'ram')
            prop_iot.set('value', ram_iot)

        '''CONTROLLER'''

        controller = xml.SubElement(AS, 'cluster')
        controller.set('id', 'Controller')
        controller.set('prefix', "cont-")
        controller.set('suffix', "")
        controller.set('radical', '0-0')
        controller.set('speed', '4Gf')
        controller.set('bw', "125GBps")
        controller.set('lat', '0us')
        controller.set('bb_bw', '10GBps')
        controller.set('bb_lat', '10us')
        controller.set('router_id', "Controller_cluster")

        '''Final de los cluster'''

        links = "name_source"
        pos = 0

        for line in rows:
            if links in line:
                break
            pos = pos + 1

        '''LINKS'''
        number_links = len(rows[pos]) - 1

        for i in range(number_links):
            link = xml.SubElement(AS, 'link')
            id = "link" + str(rows[pos][1 + i]) + str(rows[pos + 1][1 + i])
            link.set('id', id)
            link.set('latency', str(rows[pos + 2][1 + i]))
            link.set('bandwidth', str(rows[pos + 3][1 + i]))

        '''LINKS CONTROLLER'''

        for i in range(len(list_ids)):
            link = xml.SubElement(AS, 'link')
            id = "linkController" + str(list_ids[i])
            link.set('id', id)
            link.set('latency', '0.0s')
            link.set('bandwidth', '1GBps')

        '''ASROUTES'''

        for i in range(number_links):
            ASroute = xml.SubElement(AS, 'ASroute')

            src = str(rows[pos][1 + i])
            dst = str(rows[pos + 1][1 + i])
            ASroute.set('src', src)
            ASroute.set('dst', dst)
            ASroute.set('gw_src', str(dict_routerid[src]))
            ASroute.set('gw_dst', str(dict_routerid[dst]))

            link_ctn = xml.SubElement(ASroute, 'link_ctn')
            link_ctn.set('id', "link" + src + dst)

        for i in range(len(list_ids)):
            ASroute = xml.SubElement(AS, 'ASroute')

            dst = str(list_ids[i])

            ASroute.set('src', "Controller")
            ASroute.set('dst', dst)
            ASroute.set('gw_src', "Controller_cluster")
            ASroute.set('gw_dst', str(dict_routerid[dst]))

            link_ctn = xml.SubElement(ASroute, 'link_ctn')
            link_ctn.set('id', "linkController" + dst)

        tree = xml.ElementTree(platform)

        with open(filename, "wb") as fh:
            fh.write(
                "<?xml version='1.0'?><!DOCTYPE platform SYSTEM 'http://simgrid.gforge.inria.fr/simgrid/simgrid.dtd'>".encode(
                    'utf-8'))

        with open(filename, "ab") as fh:
            tree.write(fh, encoding='utf-8')

        dom = xmldom.parse(filename)
        string = dom.toprettyxml()

        f = open(filename, "w")
        f.write(string)
        f.close()

"""
        with open('./Output/app.csv', "w+") as app:
            app.write('IOTCluster|#Tasks|%Edge|%Fog\n')

            for i in range(num_iot):

                item_iot = num_datacenter * 12 + num_fog * 12 + num_edge * 12 + i * 13
                edge_iot = rows[item_iot + 12][1]
                fog_iot = rows[item_iot + 13][1]

                app.write('IOT' + str(i) + '|0000|' + str(edge_iot) + '|' + str(fog_iot) + '\n')

            app.write('Node|Name|Service|ProcessingRequirement|Input|Output|OutputStream\n')

            for i in range(num_iot):

                item_iot = num_datacenter * 12 + num_fog * 12 + num_edge * 12 + i * 13
                servers_iot = rows[item_iot + 2][1]

                for j in range(int(servers_iot)):
                    app.write('iot-' + str(i) + '-' + str(j) + '||||||\n')

            for i in range(num_fog):

                item_fog = num_datacenter * 12 + i * 11
                servers_fog = rows[item_fog + 2][1]

                for j in range(int(servers_fog)):
                    app.write('fog-' + str(i) + '-' + str(j) + '||||||\n')

            for i in range(num_edge):

                item_edge = num_datacenter * 12 + num_fog * 12 + k * 11
                devices_edge = rows[item_edge + 2][1]

                for j in range(int(devices_edge)):
                    app.write('edge-' + str(i) + '-' + str(j) + '||||||\n')

            for i in range(num_datacenter):

                servers = rows[3 + i * 11][1]

                for j in range(int(servers)):
                    app.write('cloud-' + str(i) + '-' + str(j) + '||||||\n')

            app.close
    """

if __name__ == "__main__":
    main()
