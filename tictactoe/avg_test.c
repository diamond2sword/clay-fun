#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>


float avg_n_of(float * data, int n_data)
{
	float sum = 0;
	for (int i = 0; i < n_data; i++)
	{
		sum += data[i];
	}

	return sum / n_data;
}

float avg_2_of(float * data, int n_data)
{
	float avg = 0;
	if (n_data > 0)
	{
		avg = data[0];
		for (int i = 1; i < n_data; i++)
		{
			avg = (avg + data[i]) / 2;
		}
	}
	return avg;
}

void avg_c_of(float * data, int n_data, int c_max)
{
	static int c_index = 0;
	float c_data[c_max];
	for (int i = 0; i < c_max; i++)
	{
		c_data[i] = 0;
	}
	for (int i = 0; i < n_data; i++)
	{
		c_data[c_index] = data[i];
		c_index = c_index + 1 < c_max ? c_index + 1 : 0;
		float c_sum = 0;
		for (int j = 0; j < c_max; j++)
		{
			c_sum += c_data[j];
			printf("%d ", (int)c_data[j]);
		}
		float c_avg = c_sum / c_max;
		printf("in data[%d] avg_c: %f\n", i, c_avg);
	}
}

float deltaTime;
float averageDeltaTime;
#define cycleMax 20
void updateAverageDeltaTime()
{
	static float cycleData[cycleMax];
	static int cycleIndex = 0;
	static bool isCycleFull = false;
	static float deltaTimeSum = 0;
	
	if (isCycleFull)
	{
		deltaTimeSum += deltaTime - cycleData[cycleIndex];
		averageDeltaTime = deltaTimeSum / cycleMax;
	}
	else
	{
		deltaTimeSum += deltaTime;
		averageDeltaTime = deltaTimeSum / (cycleIndex + 1);
		isCycleFull = cycleIndex + 1 == cycleMax;
	}
	cycleData[cycleIndex] = deltaTime;
	cycleIndex = cycleIndex + 1 == cycleMax ? 0 : cycleIndex + 1;
}

float randint(float min, float max)
{
	return (float)rand() / RAND_MAX * (max - min) + min;
}

int main()
{
	float data[] = {0, 1, 10, 9, 4, 2, 6, 0, 3};
	const float avg_n = avg_n_of(data, 3);
	const float avg_2 = avg_2_of(data, 3);
	printf("avg_n: %f\n", avg_n);
	printf("avg_2: %f\n", avg_2);

	avg_c_of(data, 9, 4);

	while (true)
	{
		deltaTime = randint(0.001f, 0.5f);
		updateAverageDeltaTime();
		printf("deltaTime: %f, averageDeltaTime:%f\n", deltaTime, averageDeltaTime);
	}

	return 0;
}
