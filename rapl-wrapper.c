/*
 * rapl-wrapper.c
 *
 *  Created on: May 24, 2015
 *      Author: millad
 */

#include "rapl-wrapper.h"

int msr_open(int core) {

	char filename[BUFSIZ];
	int f;

	sprintf(filename, "/dev/cpu/%d/msr", core);
	f = open(filename, O_RDONLY);
	if (f < 0) {
		if ( errno == ENXIO) {
			fprintf(stderr, "rdmsr: No CPU %d\n", core);
			exit(2);
		} else if ( errno == EIO) {
			fprintf(stderr, "rdmsr: CPU %d doesn't support MSRs\n", core);
			exit(3);
		} else {
			perror("rdmsr:open");
			fprintf(stderr, "Trying to open %s\n", filename);
			exit(127);
		}
	}

	return f;
}

long long msr_read(int f, int which) {

	uint64_t data;

	if (pread(f, &data, sizeof data, which) != sizeof data) {
		perror("rdmsr:pread");
		exit(127);
	}

	return (long long) data;
}

int detect_cpu() {

	FILE *f;

	int family, model = -1;
	char buffer[BUFSIZ], *result;
	char vendor[BUFSIZ];

	f = fopen("/proc/cpuinfo", "r");
	if (f == NULL)
		return -1;

	while (1) {
		result = fgets(buffer, BUFSIZ, f);
		if (result == NULL)
			break;

		if (!strncmp(result, "vendor_id", 8)) {
			sscanf(result, "%*s%*s%s", vendor);

			if (strncmp(vendor, "GenuineIntel", 12)) {
				printf("%s not an Intel chip\n", vendor);
				return -1;
			}
		}

		if (!strncmp(result, "cpu family", 10)) {
			sscanf(result, "%*s%*s%*s%d", &family);
			if (family != 6) {
				printf("Wrong CPU family %d\n", family);
				return -1;
			}
		}

		if (!strncmp(result, "model", 5)) {
			sscanf(result, "%*s%*s%d", &model);
		}

	}

	fclose(f);

	return model;
}

void print_cpu_model() {
	int model = detect_cpu();

	switch (model) {
	case CPU_SANDYBRIDGE:
		printf("Found Sandybridge CPU\n");
		break;
	case CPU_SANDYBRIDGE_EP:
		printf("Found Sandybridge-EP CPU\n");
		break;
	case CPU_IVYBRIDGE:
		printf("Found Ivybridge CPU\n");
		break;
	case CPU_IVYBRIDGE_EP:
		printf("Found Ivybridge-EP CPU\n");
		break;
	case CPU_HASWELL:
		printf("Found Haswell CPU\n");
		break;
	default:
		printf("Unsupported model %d\n", model);
		break;
	}
}

PETU_t validate_PETU(int msr_handler, PETU_t *pet) {
	return (pet == 0) ? msr_calculate_units(msr_handler) : *pet;
}

PETU_t msr_calculate_units(int msr_handler) {
	  /* Calculate the units used */
	  long long result=msr_read(msr_handler, MSR_RAPL_POWER_UNIT);

	  PETU_t pet;

	  pet.power_units 	= pow(0.5, (double) (result & 0xf));
	  pet.energy_units 	= pow(0.5, (double) ((result >> 8) & 0x1f));
	  pet.time_units 	= pow(0.5, (double) ((result >> 16) & 0xf));

	  return pet;
}

PackagePowerInfo_t msr_get_package_power_info(int msr_handler, PETU_t *pet) {
	/* Show package power info */
	long long result=msr_read(msr_handler, MSR_PKG_POWER_INFO);

	PackagePowerInfo_t ppi;
	PETU_t p = validate_PETU(msr_handler, pet);

	ppi.thermal_spec_power = p.power_units * (double) (result & 0x7fff);
	ppi.minimum_power = p.power_units * (double) ((result >> 16) & 0x7fff);
	ppi.maximum_power = p.power_units * (double) ((result >> 32) & 0x7fff);
	ppi.time_window = p.time_units * (double) ((result >> 48) & 0x7fff);

	return ppi;
}

PackagePowerInfo_t msr_get_package_power_info_no_pet(int msr_handler) {
	return msr_get_package_power_info(msr_handler, 0);
}

PackagePowerLimit_t msr_get_package_power_limit(int msr_handler, PETU_t *pet) {
	/* Show package power limit */
	long long result=msr_read(msr_handler, MSR_PKG_RAPL_POWER_LIMIT);

	PackagePowerLimit_t ppl;
	PETU_t p = validate_PETU(msr_handler, pet);

	ppl.power_limit_is_locked = (result >> 63);

	ppl.pkg_power_limit_1 = p.power_units*(double)((result>>0)&0x7FFF);
	ppl.pkg_time_window_1 = p.time_units*(double)((result>>17)&0x007F);
	ppl.pkg_is_enabled_1 = (result & (1LL<<15));
	ppl.pkg_is_clamped_1 = (result & (1LL<<16));
	ppl.pkg_power_limit_2 = p.power_units*(double)((result>>32)&0x7FFF);
	ppl.pkg_time_window_2 = p.time_units*(double)((result>>49)&0x007F);
	ppl.pkg_is_enabled_1 = (result & (1LL<<47));
	ppl.pkg_is_clamped_1 = (result & (1LL<<48));

	return ppl;
}
PackagePowerLimit_t msr_get_package_power_limit_no_pet(int msr_handler) {
	return msr_get_package_power_limit(msr_handler, 0);
}

double msr_get_package_energy(int msr_handler, PETU_t *pet) {
	long long result=msr_read(msr_handler, MSR_PKG_ENERGY_STATUS);
	PETU_t p = validate_PETU(msr_handler, pet);
	return (double)result*p.energy_units;
}

double msr_get_package_energy_no_pet(int msr_handler) {
	return msr_get_package_energy(msr_handler, 0);
}

double msr_get_accum_pkg_throttled_time(int msr_handler, PETU_t *pet) {
	int cpu_model = detect_cpu();
	/* only available on *Bridge-EP */
	if ((cpu_model==CPU_SANDYBRIDGE_EP) || (cpu_model==CPU_IVYBRIDGE_EP))
	{
		long long result=msr_read(msr_handler, MSR_PKG_PERF_STATUS);
		PETU_t p = validate_PETU(msr_handler, pet);
		return (double)result*p.time_units;
	}
	return 0.0;
}

double msr_get_accum_pkg_throttled_time_no_pet(int msr_handler) {
	return msr_get_accum_pkg_throttled_time(msr_handler, 0);
}

PowerPlane_EnergyPolicy_t msr_get_energy_policy_for_core(int msr_handler, PETU_t *pet) {
	PowerPlane_EnergyPolicy_t pp;
	PETU_t p = validate_PETU(msr_handler, pet);
	long long result;

	result = msr_read(msr_handler, MSR_PP0_ENERGY_STATUS);
	pp.energy = (double)result*p.energy_units;

	result = msr_read(msr_handler, MSR_PP0_POLICY);
	pp.policy = (int)result&0x001f;

	pp.accum_throttled_time = 0.0;

	int cpu_model = detect_cpu();
	/* only available on *Bridge-EP */
	if ((cpu_model==CPU_SANDYBRIDGE_EP) || (cpu_model==CPU_IVYBRIDGE_EP))
	{
		result=msr_read(msr_handler, MSR_PP0_PERF_STATUS);
		pp.accum_throttled_time = (double)result*p.time_units;
	}

	return pp;
}

PowerPlane_EnergyPolicy_t msr_get_energy_policy_for_core_no_pet(int msr_handler) {
	return msr_get_energy_policy_for_core(msr_handler, 0);
}

PowerPlane_EnergyPolicy_t msr_get_energy_policy_for_integ_gpu(int msr_handler, PETU_t *pet) {
	PowerPlane_EnergyPolicy_t pp;
	pp.energy = 0.0;
	pp.policy = -1;
	pp.accum_throttled_time = 0.0;
	PETU_t p = validate_PETU(msr_handler, pet);

	int cpu_model = detect_cpu();
	/* not available on *Bridge-EP */
	if ((cpu_model==CPU_SANDYBRIDGE) || (cpu_model==CPU_IVYBRIDGE) || (cpu_model==CPU_HASWELL)) {
		long long result;
		result = msr_read(msr_handler, MSR_PP1_ENERGY_STATUS);
		PETU_t p = validate_PETU(msr_handler, pet);
		pp.energy = (double)result*p.energy_units;
		result = msr_read(msr_handler, MSR_PP1_POLICY);
		pp.policy = (int)result&0x001f;
	}

	return pp;
}
PowerPlane_EnergyPolicy_t msr_get_energy_policy_for_integ_gpu_no_pet(int msr_handler) {
	return msr_get_energy_policy_for_integ_gpu(msr_handler, 0);
}

double msr_get_energy_for_DRAM(int msr_handler, PETU_t *pet) {

	int cpu_model = detect_cpu();
	/* Despite documentation saying otherwise, it looks like */
	/* You can get DRAM readings on regular Haswell          */
	if ((cpu_model==CPU_SANDYBRIDGE_EP) || (cpu_model==CPU_IVYBRIDGE_EP) || (cpu_model==CPU_HASWELL)) {
		long long result = msr_read(msr_handler,MSR_DRAM_ENERGY_STATUS);
		PETU_t p = validate_PETU(msr_handler, pet);
		return (double)result*p.energy_units;
	}
	return 0;
}
double msr_get_energy_for_DRAM_no_pet(int msr_handler) {
	return msr_get_energy_for_DRAM(msr_handler, 0);
}
