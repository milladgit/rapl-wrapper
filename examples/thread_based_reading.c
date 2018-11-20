/*
 * f1.c
 *
 *  Created on: Oct 04, 2016
 *      Author: millad
 */

#include "rapl-wrapper.h"

void computation() {
	int n = 10000;
	double *s = (double *) malloc(n * n * sizeof(double));
	int i, j;

	for(i=0;i<n;i++) {
		for(j=0;j<n;j++) {
			s[i * n + j]  = sin(i*j*1.0) * sin(i*j*1.0);
			s[i * n + j] += cos(i*j*1.0) * cos(i*j*1.0);
		}
	}

	for(i=0;i<n;i++) {
		for(j=0;j<n;j++) {
			s[i * n + j] = sqrt(s[i * n + j]) * exp(s[i * n + j]);
		}
	}

	sleep(2);

	free(s);

}


static struct timeval tv1, tv2;
static double en0, en1;

static int core0;
static PETU_t pet0;

static int exiting = 0;


void *measuring_code() {

	while(!exiting) {
		gettimeofday(&tv2, NULL);
		double duration_in_us = (tv2.tv_sec - tv1.tv_sec) * 1.0E6 + (tv2.tv_usec - tv1.tv_usec);
		en1 = msr_get_package_energy(core0, &pet0);

		double delta_e = en1 - en0;

		en1 = en0;
		tv2 = tv1;

		printf("--- Power: %.3f\n", delta_e / duration_in_us * 1E6);

		usleep(50000);
	}

	return NULL;
}

int main() {
	int cpu_model = detect_cpu();
	if (cpu_model < 0) {
		printf("Unsupported CPU type\n");
		return -1;
	}


	printf("CPU Model: %d\n", cpu_model);

	core0 = msr_open(0);

	pet0 = msr_calculate_units(core0);

	PackagePowerInfo_t ppi0 = msr_get_package_power_info(core0, &pet0);

	printf("PP Core 0: %.3f %.3f\n", ppi0.minimum_power, ppi0.maximum_power);



	gettimeofday(&tv1, NULL);
	en0 = msr_get_package_energy(core0, &pet0);

	pthread_t th;
	pthread_create(&th, NULL, measuring_code, NULL);



	printf("Computation begins\n");
	double e0 = msr_get_package_energy(core0, &pet0);

	computation();

	e0 = msr_get_package_energy(core0, &pet0) - e0;
	printf("Computation ends\n");

	printf("Energy consumed: Core 0: %.3fJ\n", e0);


	exiting = 1;
	pthread_join(th, NULL);

	return 0;
}

