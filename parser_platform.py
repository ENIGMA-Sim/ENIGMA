import json

with open ('user_platform.json', 'r') as f:
	user_platform = json.load(f)

nD = user_platform['num_Datacenter']

for n in user_platform['Datacenter']:
	print (n['id'])