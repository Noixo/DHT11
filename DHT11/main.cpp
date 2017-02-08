/*
;
;	DHT11 temperature and humidity read test.
;	Range & accuracy:
;		Humidity: 20-90%RH, +- 5%RH
;		Temperature: 0-50*C +- 2*C
;
*/
/*
		TO-DO
	--optimise setup()
	-split up into separate .cpp files
	-convert to a library
	-Add option to not output result/not save data
	-more conversion algorithms? (kind of pointless due to the poor accuracy of the device)
*/

#include <iostream>
#include <wiringPi.h>
#include <string>

#include <time.h>
#include <fstream>

#include <cstdlib>
#include <cstdio>

using namespace std;

#define PIN 4				// 7 or 4
#define POLL_RATE 10000		// x > 0

void save_data(int humidity, int temperature, int checksum)
{
	time_t rawtime;
	struct tm * timeinfo;
	time(&rawtime);
	timeinfo = localtime(&rawtime);

	ofstream DHT11_data;
	if (!ifstream("DHT11_data.txt"))
	{
		DHT11_data.open("DHT11_data.txt", ios_base::app);
		DHT11_data << "Local time		Humidity(%RH)		Temperature(*C)		Checksum" << endl;
		DHT11_data.close();
	}
	DHT11_data.open("DHT11_data.txt", ios_base::app);
	DHT11_data << asctime(timeinfo) << "		" << humidity << "		" << temperature << "		" << checksum << endl;

	DHT11_data.close();
}

double cTof(double temperature)
{
	return (temperature*1.8 + 32);
}
double f_to_c(double temperature)
{
	return (temperature - 32) * (5.0 / 9);
}

double c_to_k(double temperature)
{

}
double saturation_vapour_density(int humidity)
{
	double SVP = humidity * 1.0 / 18.01528;
	//return (humidity*);
}

void apparent_temperature(int temperature, int humidity)	//Heat index. Only works for 44.4* > temp > 26.6* &  humidity > 13% otherwise need adjustments
{
	temperature = cTof(temperature);
	double result = -42.379 + 2.04901523*temperature + 10.14333127*humidity - .22475541*temperature*humidity - .00683783*temperature*temperature - .05481717*humidity*humidity + .00122874*temperature*temperature*humidity + .00085282*temperature*humidity *humidity - .00000199*temperature*temperature*humidity *humidity;
	double c_temp = f_to_c(result);
	cout << "apparent temp *C: " << c_temp << endl;
}

void dew_point(int temperature, int humidity)	//Simple approximation due to DHT11 +- 5% RH
{
	cout << "Dew point: " << temperature - ((100 - humidity) / 5) << endl;
}

int setup()
{
	int count = 0;
	char array[40];

	string temp;
	string humidity;
	string check;

	int humidityint = 0, tempint = 0, checkint = 0;

	bool state = LOW;

	pinMode(PIN, INPUT);
	
	while (digitalRead(PIN) == LOW)
	{
		if (count > 40)
		{
			cout << "Device is not on." << endl;
			return 0;
		}
		count++;
		delayMicroseconds(1);
	}
	count = 0;

	pinMode(PIN, OUTPUT);
	digitalWrite(PIN, LOW);
	delay(20);
	digitalWrite(PIN, HIGH);
	delayMicroseconds(20);

	pinMode(PIN, INPUT);

	//----------------------

	for (int i = 0; i < 2; i++)	//Check signals low and high 80us
	{
		while (digitalRead(PIN) == state)
		{
			count++;
			if (count > 350)
			{
				cout << "Error" << endl;
				return 0;
			}
			delayMicroseconds(1);
		}
		state = digitalRead(PIN);
		count = 0;
		
	}

	//------------------------------
	for (int i = 0, j = 0; i < 40; i++)
	{
		while (digitalRead(PIN) == LOW)
		{
			delayMicroseconds(1);
		}

		while (digitalRead(PIN) == HIGH)
		{
			count++;
			delayMicroseconds(1);
			if (count > 200)
			{
				cout << "Error 2" << endl;
				return 0;
			}
		}
		if (count < 20)
		{
			array[i] = '0';
		}
		else
		{
			array[i] = '1';
		}
		count = 0;
	}
	for (int i = 0; i < 8; i++)
		humidity += array[i];
	for (int i = 16; i < 24; i++)
		temp += array[i];
	for (int i = 32; i < 40; i++)
		check += array[i];

	humidityint = stoi(humidity, nullptr, 2);
	tempint = stoi(temp, nullptr, 2);
	checkint = stoi(check, nullptr, 2);

	if (checkint != (humidityint + tempint))
	{
		cout << "Bad checksum" << endl;
	}
	else
	{
		cout << endl << "Humidity: " << humidityint << "%RH +- 5%" << endl;
		cout << "Temperature: " << tempint << "*C +/- 2*C" << endl << endl;

		//cout << "f: " << cTof(tempint) << endl; 
		dew_point(tempint, humidityint);
		apparent_temperature(tempint, humidityint);
		save_data(humidityint, tempint, checkint);
	}
}
	//------------------------------
int main()
{
	if (wiringPiSetupGpio() == -1)
		exit(1);
	while (true)
	{
		//cout << "Attempting to read DHT11\n";
		setup();
		delay(POLL_RATE);
	}
	return 0;
}
