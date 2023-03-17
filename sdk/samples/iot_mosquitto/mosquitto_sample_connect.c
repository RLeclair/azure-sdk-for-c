#include <stdlib.h>
#include <stdio.h>
#include "mosquitto.h"
#include "time.h"
#include "pthread.h"
#include "signal.h"
#include "string.h"
#include "unistd.h"

// Mosquitto Lib documentation is available at: https://mosquitto.org/api/files/mosquitto-h.html

#define MOSQUITTO_LOCAL_HOSTNAME "0.0.0.0"
#define MOSQUITTO_TOPIC "test/multiple"
#define MOSQUITTO_CLEAN 0

#define NUM_THREADS 100
#define WAIT_TIME_CREATE_THREAD_MS 10
#define WAIT_TIME_MESSAGE_MS 100

//#define PUBLISH_LOGS

volatile bool running = true;

void on_connect(struct mosquitto *mosq, void *obj, int reason_code);
void on_disconnect(struct mosquitto *mosq, void *obj, int rc);
void on_publish(struct mosquitto *mosq, void *obj, int mid);
void on_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos);
void on_unsubscribe(struct mosquitto *mosq, void *obj, int mid);
void on_log(struct mosquitto *mosq, void *obj, int level, const char *str);

/* Callback called when the client receives a CONNACK message from the broker. */
void on_connect(struct mosquitto *mosq, void *obj, int reason_code)
{
  (void)obj;

  /* Print out the connection result. mosquitto_connack_string() produces an
   * appropriate string for MQTT v3.x clients, the equivalent for MQTT v5.0
   * clients is mosquitto_reason_string().
   */
  printf("on_connect: %s\n", mosquitto_connack_string(reason_code));
  if(reason_code != 0){
    /* If the connection fails for any reason, we don't want to keep on
     * retrying in this example, so disconnect. Without this, the client
     * will attempt to reconnect. */
    mosquitto_disconnect(mosq);
  }

  /* You may wish to set a flag here to indicate to your application that the
   * client is now connected. */
}

void on_disconnect(struct mosquitto *mosq, void *obj, int rc)
{
  (void)mosq; (void)obj, (void)rc;
  printf("MOSQ: DISCONNECT reason=%d\n", rc);
  running = false;
}

/* Callback called when the client knows to the best of its abilities that a
 * PUBLISH has been successfully sent. For QoS 0 this means the message has
 * been completely written to the operating system. For QoS 1 this means we
 * have received a PUBACK from the broker. For QoS 2 this means we have
 * received a PUBCOMP from the broker. */
void on_publish(struct mosquitto *mosq, void *obj, int mid)
{
  (void)mosq; (void)obj, (void)mid;
#ifdef PUBLISH_LOGS
  printf("MOSQ: Message with mid %d has been published.\n", mid);
#endif // PUBLISH_LOGS
}

void on_subscribe(struct mosquitto *mosq, void *obj, int mid, int qos_count, const int *granted_qos)
{
  (void)mosq; (void)obj; (void)mid; (void)qos_count; (void)granted_qos;

  printf("MOSQ: Subscribed with mid %d; %d topics.\n", mid, qos_count);
  for(int i = 0; i < qos_count; i++)
  {
    printf("MOSQ: \t QoS %d\n", granted_qos[i]);
  }
}

void on_unsubscribe(struct mosquitto *mosq, void *obj, int mid)
{
  (void)mosq; (void)obj; (void)mid;

  printf("MOSQ: Unsubscribing using message with mid %d.\n", mid);
}

void on_log(struct mosquitto *mosq, void *obj, int level, const char *str)
{
  (void)mosq; (void)obj;

  char* log_level;

  switch (level)
  {
    case MOSQ_LOG_INFO:
      log_level = "INFO";
      break;
    case MOSQ_LOG_NOTICE:
      log_level = "NOTI";
      break;
    case MOSQ_LOG_WARNING:
      log_level = "WARN";
      break;
    case MOSQ_LOG_ERR:
      log_level = "ERR ";
      break;
    case MOSQ_LOG_DEBUG:
      log_level = "DBUG";
      break;
    case MOSQ_LOG_SUBSCRIBE:
      log_level = "SUB ";
      break;
    case MOSQ_LOG_UNSUBSCRIBE:
      log_level = "USUB";
      break;
    case MOSQ_LOG_WEBSOCKETS:
      log_level = "WSCK";
      break;
    default:
      log_level = "UNKN";
  }
  printf("MOSQ [%s] %s\n", log_level, str);
}

void *mosquitto_client_test(void* arg)
{
  struct mosquitto *mosq = (struct mosquitto*) arg;
  int rc;
  char payload[5];

  snprintf(payload, sizeof(payload), "%d", 123);

  while(1){
    rc = mosquitto_publish(mosq, NULL, MOSQUITTO_TOPIC, strlen(payload), payload, 0, false);
    if(rc != MOSQ_ERR_SUCCESS){
      fprintf(stderr, "Error publishing: %s\n", mosquitto_strerror(rc));
    }
    usleep(WAIT_TIME_MESSAGE_MS * 1000U);
  }
}

int main(int argc, char *argv[])
{
  struct mosquitto *mosq;
  char client_name[] = "zancudo";
	int rc;

	/* Required before calling other mosquitto functions */
	mosquitto_lib_init();

  mosq = mosquitto_new(client_name, MOSQUITTO_CLEAN, NULL);
  if(mosq == NULL){
		fprintf(stderr, "Error: Out of memory.\n");
		exit(1);
	}

  mosquitto_connect_callback_set(mosq, on_connect);
	mosquitto_publish_callback_set(mosq, on_publish);
  mosquitto_disconnect_callback_set(mosq, on_disconnect);

  rc = mosquitto_connect(mosq, MOSQUITTO_LOCAL_HOSTNAME, 1883, 60);
	if(rc != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(mosq);
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
		exit(1);
	}

  rc = mosquitto_loop_start(mosq);
	if(rc != MOSQ_ERR_SUCCESS){
		mosquitto_destroy(mosq);
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
		exit(1);
	}

  rc = mosquitto_subscribe(mosq, NULL, MOSQUITTO_TOPIC, 0);
  if(rc != MOSQ_ERR_SUCCESS){
    mosquitto_destroy(mosq);
		fprintf(stderr, "Error: %s\n", mosquitto_strerror(rc));
		exit(1);
  }
  
  pthread_t thid[NUM_THREADS];
  int thcount = 0;

  for (int i = 0; i < NUM_THREADS; i++)
  {
    printf("Creating thread number: %d\r\n", i);
    if (pthread_create(&(thid[i]), NULL, mosquitto_client_test, mosq) != 0) {
      perror("pthread_create() error");
      exit(1);
    }
    usleep(WAIT_TIME_CREATE_THREAD_MS * 1000U);
  }
  
	while(1){
    sleep(10);
	}

	mosquitto_lib_cleanup();
	return 0;
}
