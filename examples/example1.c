/*
 * f1.c
 *
 *  Created on: May 26, 2015
 *      Author: millad
 */

#include "rapl-wrapper.h"

void computation() {
	double s[1000][1000];
	int i, j;

	for(i=0;i<1000;i++)
		for(j=0;j<1000;j++) {
			s[i][j] = 0;
		}

	for(i=0;i<1000;i++)
		for(j=0;j<1000;j++) {
			s[i][j] = i*j;
		}
}

int main() {
	int cpu_model = detect_cpu();
	if (cpu_model < 0) {
		printf("Unsupported CPU type\n");
		return -1;
	}

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

	computation();

	e0 = msr_get_package_energy(core0, &pet0) - e0;
	e1 = msr_get_package_energy(core1, &pet1) - e1;
	printf("Computation ends\n");

	printf("Energy consumed: Core 0: %.3fJ\n", e0);
	printf("Energy consumed: Core 1: %.3fJ\n", e1);

	return 0;
}

