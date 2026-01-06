#define ALPHA 1.2
//#include "adaptive_batchsize8_RF_100.h"
#include "RF_100.h"
#include "RF_100_testset.h"
#include "hardware_check.h"

Eloquent::ML::Port::RandomForest clf;

int pred = 0;
int temp, start;
float correct = 0.0;
float pro;

void setup() 
{
    Serial.begin(115200);
    delay(2000);
    Serial.print("benutzter Heap-Speicher: ");
    Serial.print(GetUsedSRAM(&pro));
    Serial.println(" Bytes");

    Serial.print("Auslastung: ");
    Serial.print(pro);
    Serial.println(" %");

    Serial.print("Freier Heap-Speicher: ");
    Serial.print(GetFreeSRAM(&pro));
    Serial.println(" Bytes");
}

void loop() 
{
    correct = 0;		
    start = millis();
    for (int i = 0; i < DATASET_SIZE; i++) 
    {        
		pred = clf.predict(X[i]);
		//Serial.print(pred);
	    if (pred == y[i])
            correct += 1.0;
    }
    temp = millis() - start;
    
    Serial.print("\nIt took ");    
    Serial.print(temp);
    Serial.print("ms to infer for ");
    Serial.print(DATASET_SIZE);
    Serial.println(" samples");
   
    Serial.print("Correct: ");
    Serial.print(correct / DATASET_SIZE * 100.0);
	Serial.println("%"); 
    delay(3000); 
}