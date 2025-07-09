// pid_bench_c_pthread.cpp
#include <benchmark/benchmark.h>
#include <math.h>
#include <pthread.h>
#include <stdbool.h>
#include <stdlib.h>

typedef struct {
  pthread_mutex_t mtx;
  double value;
} SharedData;

typedef struct {
  double kp, ki, kd;
  double prev_error;
  double integral;
} PID;

typedef struct {
  SharedData *data;
  bool *running;
  int iterations;
} ThreadArgs;

typedef struct {
  SharedData *data;
  bool *running;
  int iterations;
  PID *pid;
} ControlArgs;

void pid_init(PID *pid, double kp, double ki, double kd) {
  pid->kp = kp;
  pid->ki = ki;
  pid->kd = kd;
  pid->prev_error = 0;
  pid->integral = 0;
}

double pid_compute(PID *pid, double setpoint, double measurement) {
  double error = setpoint - measurement;
  pid->integral += error;
  double derivative = error - pid->prev_error;
  pid->prev_error = error;
  return pid->kp * error + pid->ki * pid->integral + pid->kd * derivative;
}

void *sensor_thread(void *arg) {
  ThreadArgs *args = (ThreadArgs *)arg;
  for (int i = 0; i < args->iterations && *(args->running); ++i) {
    pthread_mutex_lock(&args->data->mtx);
    args->data->value = i * 0.001;
    pthread_mutex_unlock(&args->data->mtx);
  }
  return NULL;
}

void *control_thread(void *arg) {
  ControlArgs *args = (ControlArgs *)arg;
  for (int i = 0; i < args->iterations && *(args->running); ++i) {
    double val;
    pthread_mutex_lock(&args->data->mtx);
    val = args->data->value;
    pthread_mutex_unlock(&args->data->mtx);
    pid_compute(args->pid, 5.0, val);
  }
  return NULL;
}

static void BM_C_PThread_PID(benchmark::State &state) {
  const int iterations = (int)state.range(0);

  for (auto _ : state) {
    SharedData data;
    pthread_mutex_init(&data.mtx, NULL);
    data.value = 0;

    bool running = true;
    PID pid;
    pid_init(&pid, 1.0, 0.1, 0.05);

    pthread_t tid1, tid2;
    ThreadArgs s_args = {&data, &running, iterations};
    ControlArgs c_args = {&data, &running, iterations, &pid};

    pthread_create(&tid1, NULL, sensor_thread, &s_args);
    pthread_create(&tid2, NULL, control_thread, &c_args);

    pthread_join(tid1, NULL);
    pthread_join(tid2, NULL);

    pthread_mutex_destroy(&data.mtx);
    running = false;
  }
}

BENCHMARK(BM_C_PThread_PID)->Arg(1000000);
BENCHMARK_MAIN();
// g++ -std=c11 -O2 -pthread pid_benchmark_chatgpt.c -lbenchmark -o c_bench
