from django.shortcuts import render, redirect
from currency.models import Currency
from datetime import date
import requests

def UpdateCurrency(request):
    currency = Currency()
    currency.Date = date.today()

    
    key1 = '077dbea3a01e2c601af7d870ea30191c'  # mu.niu.525@gmail.com   19900525
    key2 = 'b6ec9d9dd5efa56c094fc370fd68fbc8'  # cowtony@163.com        19900525
    key3 = 'af07896d862782074e282611f63bc64b'  # mniu@umich.edu         19900525

    URL = 'http://data.fixer.io/api/' + currency.Date.strftime('%Y-%m-%d')
    PARAMS = {'access_key': key2, 'base': 'EUR', 'symbols': 'USD,CNY,EUR,GBP'}
    response = requests.get(url=URL, params=PARAMS)
    # extracting data in json format
    data = response.json()
    print(data)
    if data['success']:
        currency.USD = data['rates']['USD']
        currency.CNY = data['rates']['CNY']
        currency.GBP = data['rates']['GBP']

        currency.save()

    return redirect('home')
