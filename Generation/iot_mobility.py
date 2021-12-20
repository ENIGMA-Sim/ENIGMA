import sys, random
 
def main():
	length = len(sys.argv) - 1

	if length < 2:
		print("Error: ./python3 iot_mobility.py iot_cluster iot0 iotN grid_size_x grid_sixe_y number_positions")
		exit(0)	

	iot_cluster = int(sys.argv[1])
	size_grid_x = int(sys.argv[2 + int(iot_cluster)])
	size_grid_y = int(sys.argv[2 + int(iot_cluster) + 1])
	number_positions = int(sys.argv[2 + int(iot_cluster) + 2])

	if int(size_grid_x) < 500 or int(size_grid_y) < 500:
		print("Error: Minimum grid size is 500")
		exit(0)

	if int(number_positions) < 50:
		print("Error: Minimum position number is 50")
		exit(0)

	for i in range(iot_cluster):
		number_sensors = int(sys.argv[2 + int(i)])
		
		main = open("./sensors-" + str(i) +".txt", "w+")

		for j in range(number_sensors):
			for k in range(number_positions):
				value_x = random.randint(1, size_grid_x)
				value_y = random.randint(1, size_grid_y)
				coordinates = str(value_x) + "," + str(value_y) + ";"
				main.write(coordinates.rstrip('\n'))
			main.write("\n")
		main.close()




	#print(str(iot_cluster) + " " + str(size_grid_x) + " " + str(size_grid_y))



if __name__ == '__main__':
	main()