import folium
import csv
from branca.element import Template, MacroElement


map = folium.Map(
    location=[40.4316091393354, -3.6860279526293813],
    zoom_start=12,
    tiles='Stamen Terrain'
)


with open('../Mobility/substations.csv', newline='') as File:
    reader = csv.reader(File)

    for index, item in enumerate(reader):
        htmlSub = '<h5><b>Substation-' + str(index) + "</b></h5><br><b>Latitude</b>: " + item[0] + "; <b>Longitude</b>: " + item[1]
        popupSub = folium.Popup(htmlSub, max_width=300)
        icon = folium.Icon(icon='wifi', prefix='fa')
        folium.Marker([float(item[0]), float(item[1])], popup=popupSub, tooltip="<b>Substation-" + str(index) + "</b>", icon=icon).add_to(map)

routes = [[[] for x in range(3)] for y in range(3)]
route_color = ["orange", "blue", "yellow"]

with open('../Results/traces-1m.csv', newline='') as File:
    reader = csv.reader(File)
    next(reader, None)
    for index, item in enumerate(reader):
        latitude = float(item[4])
        longitude = float(item[5])

        if len(routes[int(item[1])][int(item[0])]) == 0:
            icon = folium.Icon(icon='flag', prefix='fa', color="green")
            folium.Marker([latitude, longitude], icon=icon, tooltip="<h5><b>Start Route</b></h5>").add_to(map)

        routes[int(item[1])][int(item[0])].append((latitude, longitude))

        #if index < 300:
        icon = folium.features.CustomIcon('./icon.png', icon_size=(6, 6))
        htmlMark = "<h5><b>Truck_" + item[0] + "_" + str(int(item[1])) + "</b></h5><br><b>Latitude</b>: " + item[4] + "; <b>Longitude</b>: " + item[5] + "<br>" + "<b>Temp</b>: " + item[
            2] + " ºC<br>" + "<b>Hum</b>: " + item[3] + "%"
        popupMark = folium.Popup(htmlMark, max_width=300)
        folium.Marker([latitude, longitude], popup=popupMark, tooltip="<b>Truck_" + item[0] + "_" + str(int(item[1])) + "</b>", icon=icon).add_to(map)

for i in range(3):
    for j in range(3):
        folium.PolyLine(routes[i][j],
                        color=route_color[i],
                        weight=8,
                        opacity=0.8).add_to(map)

        icon = folium.Icon(icon='flag-checkered', prefix='fa', color="red")
        # print(routes[i][j][-1][0])
        folium.Marker([routes[i][j][-1][0], routes[i][j][-1][1]], icon=icon, tooltip="<h5><b>End Route</b></h5>").add_to(map)

legend_info = ""
index = 0
for i in route_color:
    legend_info = legend_info + "<li><span style='background:"+ i +";opacity:0.7;'></span>&nbsp;Route " + str(index) + "</li>\n"
    index = index + 1

template = """
{% macro html(this, kwargs) %}

<!doctype html>
<html lang="en">
<head>
  <meta charset="utf-8">
  <meta name="viewport" content="width=device-width, initial-scale=1">
  <title>EMT Routes</title>
  <link rel="stylesheet" href="//code.jquery.com/ui/1.12.1/themes/base/jquery-ui.css">

  <script src="https://code.jquery.com/jquery-1.12.4.js"></script>
  <script src="https://code.jquery.com/ui/1.12.1/jquery-ui.js"></script>

  <script>
  $( function() {
    $( "#maplegend" ).draggable({
                    start: function (event, ui) {
                        $(this).css({
                            right: "auto",
                            top: "auto",
                            bottom: "auto"
                        });
                    }
                });
    $( "#maplegend" ).resizable();
});

  </script>
</head>
<body>


<div id='maplegend' class='maplegend' 
    style='position: absolute; z-index:9999; border:2px solid grey; background-color:rgba(255, 255, 255, 0.8);
     border-radius:6px; padding: 10px; font-size:14px; right: 20px; bottom: 20px;'>

<div class='legend-title'>Legend</div>
<div class='legend-scale'>
  <ul class='legend-labels'>\n""" + \
    legend_info + """<li><i class="fa fa-flag fa-2x"></i>&nbsp;Start</li>
    <li><i class="fa fa-flag-checkered fa-2x"></i>&nbsp;End</li>
    <li><i class="fa fa-wifi fa-2x"></i>&nbsp;Substation</li>
    <li><img src='./images/icon.png' width="10px" height="10px"/>&nbsp;Marker</li> 
  </ul>
</div>
</div>

</body>
</html>

<style type='text/css'>
  .maplegend .legend-title {
    text-align: left;
    margin-bottom: 5px;
    font-weight: bold;
    font-size: 90%;
    }
  .maplegend .legend-scale ul {
    margin: 0;
    margin-bottom: 5px;
    padding: 0;
    float: left;
    list-style: none;
    }
  .maplegend .legend-scale ul li {
    font-size: 80%;
    list-style: none;
    margin-left: 0;
    line-height: 18px;
    margin-bottom: 2px;
    }
  .maplegend ul.legend-labels li span {
    display: block;
    float: left;
    height: 16px;
    width: 30px;
    margin-right: 5px;
    margin-left: 0;
    border: 1px solid #999;
    }
  .maplegend .legend-source {
    font-size: 80%;
    color: #777;
    clear: both;
    }
  .maplegend a {
    color: #777;
    }
</style>
{% endmacro %}"""

macro = MacroElement()
macro._template = Template(template)

map.get_root().add_child(macro)

map.save('map.html')