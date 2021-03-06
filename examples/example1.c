/*
 * f1.c
 *
 *  Created on: May 26, 2015
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

int main() {
	int cpu_model = detect_cpu();
	if (cpu_model < 0) {
		printf("Unsupported CPU type\n");
		return -1;
	}

	printf("CPU Model: %d\n", cpu_model);

	int core0 = msr_open(0);
	int core1 = msr_open(1);

	PETU_t pet0 = msr_calculate_units(core0);
	PETU_t pet1 = msr_calculate_units(core1);

	PackagePowerInfo_t ppi0 = msr_get_package_power_info(core0, &pet0);
	PackagePowerInfo_t ppi1 = msr_get_package_power_info(core1, &pet1);

	printf("PP Core 0: %.3f %.3f\n", ppi0.minimum_power, ppi0.maximum_power);
	printf("PP Core 1: %.3f %.3f\n", ppi1.minimum_power, ppi1.maximum_power);

	printf("Computation begins\n");
	double e0 = msr_get_package_energy(core0, &pet0);
	double e1 = msr_get_package_energy(core1, &pet1);

	struct timeval monitor_time_tv_start, monitor_time_tv_stop;

	gettimeofday(&monitor_time_tv_start, NULL);
	computation();
	gettimeofday(&monitor_time_tv_stop, NULL);

	e0 = msr_get_package_energy(core0, &pet0) - e0;
	e1 = msr_get_package_energy(core1, &pet1) - e1;
	printf("Computation ends\n");

	printf("Energy consumed: Core 0: %.3fJ\n", e0);
	printf("Energy consumed: Core 1: %.3fJ\n", e1);

	double duration_in_us = (monitor_time_tv_stop.tv_sec - monitor_time_tv_start.tv_sec) * 1.0E6 + (monitor_time_tv_stop.tv_usec - monitor_time_tv_start.tv_usec);
	printf("Time (seconds): %.3f\n", duration_in_us * 1E-6);
	printf("Power: %.3f\n", (e0) / duration_in_us * 1E6);

	return 0;
}

