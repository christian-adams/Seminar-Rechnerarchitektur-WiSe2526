#include "RF_100_m2cgen.h"
#include "RF_100_testset.h"
#include "hardware_check.h"

int temp, start;
float correct = 0.0;
float pro;
double d[4];
double pred[3];
int predicted_class;
double max_score;

void float_to_double(float src[4], double dst[4])
{
    for (int i = 0; i < 4; i++)    
        dst[i] = (double)src[i];    
}

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
		float_to_double(X[i], d);
        model_m2cgen(d, pred);
        max_score = 0.0;
        for (int k = 0; k < 3; k++) //argmax
        {
            if (pred[k] > max_score)
            {
                max_score = pred[k];
                predicted_class = k;
            }
        }
		//Serial.print(predicted_class);
	    if (predicted_class == y[i])
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