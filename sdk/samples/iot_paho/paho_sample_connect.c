#include <stdlib.h>
#include <stdio.h>
#include "time.h"
#include "pthread.h"
#include "signal.h"
#include "string.h"
#include "unistd.h"
#include <MQTTClient.h>

#define PAHO_LOCAL_HOSTNAME "0.0.0.0"
#define PAHO_TOPIC "test/multiple"
#define PAHO_CLEAN 0

#define NUM_THREADS 100
#define WAIT_TIME_CREATE_THREAD_MS 10
#define WAIT_TIME_MESSAGE_MS 100

void *paho_client_test(void* arg)
{
  MQTTClient *paho = (MQTTClient*) arg;
  int rc;
  char payload[5];

  snprintf(payload, sizeof(payload), "%d", 123);

  while(1){
    rc = MQTTClient_publish(*paho, PAHO_TOPIC, strlen(payload), payload, 0, 0, NULL);
    if(rc != MQTTCLIENT_SUCCESS){
      fprintf(stderr, "Error publishing: %s\n", MQTTClient_strerror(rc));
    }
    usleep(WAIT_TIME_MESSAGE_MS * 1000U);
  }
}

int main(int argc, char *argv[])
{
  int rc;
  MQTTClient paho;
  MQTTClient_connectOptions client_options = MQTTClient_connectOptions_initializer;
  char client_name[] = "zancudo";

  rc = MQTTClient_create(&paho, PAHO_LOCAL_HOSTNAME,
   client_name, MQTTCLIENT_PERSISTENCE_NONE, NULL);

  if (rc != MQTTCLIENT_SUCCESS)
  {
    MQTTClient_destroy(&paho);
    fprintf(stderr, "Error: %s\n", MQTTClient_strerror(rc));
    exit(1);
  }

  client_options.cleansession = PAHO_CLEAN;
  client_options.keepAliveInterval = 240;
  rc = MQTTClient_connect(paho, &client_options);

  if (rc != MQTTCLIENT_SUCCESS)
  {
    MQTTClient_destroy(&paho);
    fprintf(stderr, "Error; %s\n", MQTTClient_strerror(rc));
    exit(1);
  }

  rc = MQTTClient_subscribe(paho, PAHO_TOPIC, 0);

  if (rc != MQTTCLIENT_SUCCESS)
  {
    MQTTClient_destroy(&paho);
    fprintf(stderr, "Error; %s\n", MQTTClient_strerror(rc));
    exit(1);
  }

  pthread_t thid[NUM_THREADS];
  int thcount = 0;

  for (int i = 0; i < NUM_THREADS; i++)
  {
    printf("Creating thread number: %d\r\n", i);
    if (pthread_create(&(thid[i]), NULL, paho_client_test, &paho) != 0) {
      perror("pthread_create() error");
      exit(1);
    }
    usleep(WAIT_TIME_CREATE_THREAD_MS * 1000U);
  }
  
	while(1){
    sleep(10);
	}

	return 0;
}
