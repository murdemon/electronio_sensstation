// This #include statement was automatically added by the Particle IDE.
#include <time.h>    
#include "Adafruit_DHT.h"

#define SAMPLETIME 1800
#define DHTPIN D2     // what pin we're connected to
#define DHTTYPE DHT22		// DHT 22 (AM2302)

DHT dht(DHTPIN, DHTTYPE);

SYSTEM_MODE(SEMI_AUTOMATIC);


float MoistFilt[10];
float TempFilt[10];

int Timeout = 0;
int mesuring_done = 0;
int CellCon = 0;
int ReadyToSleep = 0;
int PublishDone = 0;
String myID;

void setup()
{
 myID = System.deviceID();
 Time.zone(-4);
 Serial.begin(9600);
 Serial.println("Setup.....");

 dht.begin();

 Particle.subscribe(myID, myHandler);
}

void myHandler(const char *event, const char *data)
{
if (strcmp(event,myID)==0) {ReadyToSleep = 1;}
}

float Temperature()
{
int A1_Raw;
float tmp;
float Temp;
float Sum;
float y;


for (int i = 0; i < 10 ; i ++)
{
	 A1_Raw = analogRead(A1);
	 y =  A1_Raw;
	 tmp = (3.3*y)/4096;	 
	 Temp = tmp*75.006 - 40;
	 TempFilt[i] = Temp;
}
	 Sum = 0;
	 for (int i = 0; i < 10 ; i++) {Sum = Sum + TempFilt[i];}
	 Sum = Sum / 10;
 return Sum;

}

float Moisture()
{
int A0_Raw;
float tmp;
float Moist;
float Sum;
float y;


for (int i = 0; i < 10 ; i ++)
{
	 A0_Raw = analogRead(A0);
	 y =  A0_Raw;
	 tmp = (3.3*y)/4096;	 
	 if (tmp > 0 and tmp <= 1.1){  Moist = 10*tmp-1; }
	 if (tmp > 1.1 and tmp <= 1.3){  Moist = 25*tmp-17.5; }
	 if (tmp > 1.3 and tmp <= 1.82){  Moist = 48.08*tmp-47.5; }
	 if (tmp > 1.82){  Moist = 26.32*tmp-7.89; }
	 MoistFilt[i] = Moist;
}
	 Sum = 0;
	 for (int i = 0; i < 10 ; i++) {Sum = Sum + MoistFilt[i];}
	 Sum = Sum / 10;
 return Sum;
}

void loop()
{
   Serial.println("Looping call");

   int index = 0;
   float value = 0;
   String stringOne = "";
   String sensorRecordId = "232312";
   int intervalSeconds = SAMPLETIME;
   bool sent = false;
   String startDateTime = Time.format(Time.local()-SAMPLETIME*48, "%Y-%m-%d  %H:%M:%S");// "2016-08-05 00:00:00";
   String userrecordid = "411337";
   String orgnum = "8";
   FuelGauge fuel;

   delay(5000);

   if (PublishDone == 1) {Timeout = Timeout + 1;}
   if (Timeout > 10000) {ReadyToSleep = 1;}

   EEPROM.get(2000, index);

   if (index > 48) {index = 0; EEPROM.put(2000, index);}

   if (mesuring_done == 0) 
   {


	   value = Moisture();
	   EEPROM.put(index*4, value);
	   Serial.println(String::format("Moist = %3.1f",value));   
	   Serial.println(index);

	   value = Temperature();
	   EEPROM.put(index*4+300, value);
	   Serial.println(String::format("Soil_Temp = %3.1f",value));   
	   Serial.println(index);

	   value = dht.getHumidity();
	   EEPROM.put(index*4+600, value);
	   Serial.println(String::format("Humid = %3.1f",value));   
	   Serial.println(index);

	   value = dht.getTempFarenheit();
	   EEPROM.put(index*4+900, value);
	   Serial.println(String::format("Air_Temp = %3.1f",value));   
	   Serial.println(index);


	   index = index + 1;
	   EEPROM.put(2000, index);
	   mesuring_done = 1;           
   }
 
   if (index > 47)
   {
     
     if (Particle.connected() == false) 
		{
		        
			if (CellCon = 0) { Cellular.on(); CellCon = 1;}
			
			if (!Cellular.ready()) {Cellular.connect();}
		        if (Cellular.ready())  {Particle.connect();}

		}

     
     if (Particle.connected() == true && PublishDone == 0 &&  System.updatesPending() == false)
     {
        Particle.process();
	Serial.println("Cloud ready");


        float tx_total;
        float rx_total;
                
        CellularSignal sig = Cellular.RSSI();
        
        EEPROM.get(1200,tx_total);
        EEPROM.get(1204,rx_total);

        //stringOne = String::format("Tx=%6.0f,Rx==%6.0f,Bat==%3.1f,RSSI=%i,Qual=%i",tx_total,rx_total, fuel.getSoC(),sig.rssi,sig.qual);
        //sent = Particle.publish("Stat", stringOne);

	    stringOne = "1,"+String(SAMPLETIME)+",";

	     for(int i=1;i<49;i++)
		{
			EEPROM.get(i*4,value);
			stringOne = stringOne + String::format("%3.1f",value) +",";
		}

	     stringOne = stringOne.remove(stringOne.length()-1);

	     Serial.println(stringOne);
	    
	     stringOne = String(stringOne);

	     Serial.println(stringOne);
	     sent = Particle.publish("D", stringOne);
	     delay(2000);

	     stringOne = "2,"+String(SAMPLETIME)+",";

	     for(int i=1;i<49;i++)
		{
			EEPROM.get(i*4+300,value);
			stringOne = stringOne + String::format("%3.1f",value) +",";
		}

	     stringOne = stringOne.remove(stringOne.length()-1);

	     Serial.println(stringOne);
	    
	     stringOne = String(stringOne);

	     Serial.println(stringOne);
	     sent = Particle.publish("D", stringOne);
	     delay(2000);

	     stringOne = "3,"+String(SAMPLETIME)+",";

	     for(int i=1;i<49;i++)
		{
			EEPROM.get(i*4+600,value);
			stringOne = stringOne + String::format("%3.1f",value) +",";
		}

	     stringOne = stringOne.remove(stringOne.length()-1);

	     Serial.println(stringOne);
	    
	     stringOne = String(stringOne);

	     Serial.println(stringOne);
	     sent = Particle.publish("D", stringOne);
	     delay(2000);

	     stringOne = "4,"+String(SAMPLETIME)+",";

	     for(int i=1;i<49;i++)
		{
			EEPROM.get(i*4+900,value);
			stringOne = stringOne + String::format("%3.1f",value) +",";
		}

	     stringOne = stringOne.remove(stringOne.length()-1);

	     Serial.println(stringOne);
	    
	     stringOne = String(stringOne);

	     Serial.println(stringOne);
	     sent = Particle.publish("D", stringOne);
	     delay(2000);
            
             stringOne = "10,"+String(SAMPLETIME)+",";

	     stringOne = stringOne + String::format("%3.1f",fuel.getSoC()) + ",";
	     stringOne = stringOne.remove(stringOne.length()-1);
	     Serial.println(stringOne);
    
	     stringOne = String(stringOne);

	     Serial.println(stringOne);
	     sent = Particle.publish("D", stringOne);
	     delay(2000);
        
	     sent = Particle.publish(myID, "Done");
	     delay(2000);
         
//	     if ( sent == true) 	     {
	     Serial.println("Data send to Cloud good");	 
	     PublishDone = 1;
//		}
     }
   
   
    if (ReadyToSleep == 1  &&  System.updatesPending() == false)
	{
	     index = 0; 
	     EEPROM.put(2000, index);   
	     Particle.disconnect();
 	     Serial.println("Go to sleep!");
	     delay(5000);
	}
    }



	if (index < 48)  
	{
        float tx_total;
        float rx_total;
        
	CellularData data;
        if (!Cellular.getDataUsage(data)) {Serial.print("Error! Not able to get data.");}
        
        EEPROM.get(1200,tx_total);
        EEPROM.get(1204,rx_total);

        tx_total = tx_total + (float)data.tx_total;
        rx_total = rx_total + (float)data.rx_total;    
		
	EEPROM.put(1200,tx_total);
        EEPROM.put(1204,rx_total);

	delay(5000);
        Particle.disconnect();
	System.sleep(SLEEP_MODE_DEEP, SAMPLETIME);
	}

}
