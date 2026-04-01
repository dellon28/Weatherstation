import time
import requests

now = int(time.time())
dayAgo = now - 86400
url = f'http://localhost:8080/api/weather/get/{dayAgo}/{now}'

try:
    r = requests.get(url)
    print(f'Status: {r.status_code}')
    data = r.json()
    records = data.get('data', [])
    print(f'Records found: {len(records)}')
    if records:
        print(f'Date range: {time.ctime(records[0].get("timestamp"))} to {time.ctime(records[-1].get("timestamp"))}')
        print(f'First record: {records[0]}')
    else:
        print('No historical data in database')
except Exception as e:
    print(f'Error: {e}')
