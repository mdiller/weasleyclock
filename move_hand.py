import requests
import json

baseUrl = "http://192.168.0.120"

data = {
	"hand": 1,
	"position": 4.35
}

data = json.dumps(data)
response = requests.post(f"{baseUrl}/moveHand?plain={data}")

print(response.status_code, response.text)