/* A wrapper for functions based on the rapl-read.c file by           */
/*       Vince Weaver                                                 */
/*                                                                    */
/* Author: Millad Ghane -- mghane2 @ uh.edu -- May 24, 2015           */
/*                                                                    */
/* Reading the RAPL registers from a sandybridge-ep machine           */
/*                                                                    */
/* The /dev/cpu/<CPUNUM>/msr driver must be enabled and permissions   */
/* set to allow read access for this to work. To do this, you should  */
/* be root or be a member of group root.                              */
/*                                                                    */
/* To load the driver, run following command:                         */
/*         $ modprobe msr                                             */
/*                                                                    */
/* Also note that if you do not have the MSR access at all, it is     */
/* better to install following package:                               */
/*         $ apt-get install msr-tools                                */
/*                                                                    */
/* To see whether your linux supports the MSR or not, use following   */
/* command:                                                           */
/*         $ cat /boot/config-<some_numbers> | grep MSR               */
/* and if you see something like this, you have the MSR support.      */
/* Otherwise, you should recompile the kernel.                        */
/*         CONFIG_X86_MSR=m (or y)                                    */
/*                                                                    */
/* Note: Energy measurements can overflow in almost 60s.              */
/* ====                                                               */
/* Better to sample the counters more often than that.                */
/*                                                                    */
/*                                                                    */
/*                                                                    */
/* Additional contributions by:                                       */
/*   Romain Dolbeau -- romain @ dolbeau.org                           */
/*   Vince Weaver -- vincent.weaver @ maine.edu -- 29 November 2013   */

#ifndef RAPL_WRAPPER_H_
#define RAPL_WRAPPER_H_

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <fcntl.h>
#include <errno.h>
#include <inttypes.h>
#include <unistd.h>
#include <math.h>
#include <string.h>

#define MSR_RAPL_POWER_UNIT		0x606

/*
 * Platform specific RAPL Domains.
 * Note that PP1 RAPL Domain is supported on 062A only
 * And DRAM RAPL Domain is supported on 062D only
 */
/* Package RAPL Domain */
#define MSR_PKG_RAPL_POWER_LIMIT	0x610
#define MSR_PKG_ENERGY_STATUS		0x611
#define MSR_PKG_PERF_STATUS		0x613
#define MSR_PKG_POWER_INFO		0x614

/* PP0 RAPL Domain */
#define MSR_PP0_POWER_LIMIT		0x638
#define MSR_PP0_ENERGY_STATUS		0x639
#define MSR_PP0_POLICY			0x63A
#define MSR_PP0_PERF_STATUS		0x63B

/* PP1 RAPL Domain, may reflect to uncore devices */
#define MSR_PP1_POWER_LIMIT		0x640
#define MSR_PP1_ENERGY_STATUS		0x641
#define MSR_PP1_POLICY			0x642

/* DRAM RAPL Domain */
#define MSR_DRAM_POWER_LIMIT		0x618
#define MSR_DRAM_ENERGY_STATUS		0x619
#define MSR_DRAM_PERF_STATUS		0x61B
#define MSR_DRAM_POWER_INFO		0x61C

/* RAPL UNIT BITMASK */
#define POWER_UNIT_OFFSET	0
#define POWER_UNIT_MASK		0x0F

#define ENERGY_UNIT_OFFSET	0x08
#define ENERGY_UNIT_MASK	0x1F00

#define TIME_UNIT_OFFSET	0x10
#define TIME_UNIT_MASK		0xF000

#define CPU_SANDYBRIDGE		42
#define CPU_SANDYBRIDGE_EP	45
#define CPU_IVYBRIDGE		58
#define CPU_IVYBRIDGE_EP	62
#define CPU_HASWELL		60

/* Basic Functions */
int msr_open(int core);
long long msr_read(int f, int which);

/* Basic Functions for CPU detection */
int detect_cpu();
void print_cpu_model();


/* Basic Units for Power, Energy, and Time */
typedef struct PETU {
	double power_units;			// in Watts
	double energy_units;		// in Joules
	double time_units;			// in Seconds
} PETU_t;

PETU_t msr_calculate_units(int msr_handler);



/*
 * Package Power Information
 */
typedef struct PackagePowerInfo {
	double thermal_spec_power;	// in Watts
	double minimum_power;		// in Watts
	double maximum_power;		// in Watts
	double time_window;			// in Seconds
} PackagePowerInfo_t;

PackagePowerInfo_t msr_get_package_power_info(int msr_handler, PETU_t *pet);
PackagePowerInfo_t msr_get_package_power_info_no_pet(int msr_handler);



/*
 * Package Power Limitations
 */
typedef struct PackagePowerLimit {
	int power_limit_is_locked;	// 1 = locked  ---  0 = unlocked
	double pkg_power_limit_1;	// in Watts
	double pkg_time_window_1;	// in Seconds
	int pkg_is_enabled_1;
	int pkg_is_clamped_1;
	double pkg_power_limit_2;	// in Watts
	double pkg_time_window_2;	// in Seconds
	int pkg_is_enabled_2;
	int pkg_is_clamped_2;
} PackagePowerLimit_t;

PackagePowerLimit_t msr_get_package_power_limit(int msr_handler, PETU_t *pet);
PackagePowerLimit_t msr_get_package_power_limit_no_pet(int msr_handler);



/*
 * Getting Energy of the Package
 */
double msr_get_package_energy(int msr_handler, PETU_t *pet);
double msr_get_package_energy_no_pet(int msr_handler);

/*
 * Getting total execution time
 */
double msr_get_accum_pkg_throttled_time(int msr_handler, PETU_t *pet);
double msr_get_accum_pkg_throttled_time_no_pet(int msr_handler);



/*
 * Getting information regarding different power planes on core - Core only or Integrated GPUs
 */
typedef struct PowerPlane_EnergyPolicy {
	double energy;					// in Joules
	int policy;
	double accum_throttled_time;	// in Seconds; only for Cores (PP0) and not Integrated GPUs (PP1)
} PowerPlane_EnergyPolicy_t;

PowerPlane_EnergyPolicy_t msr_get_energy_policy_for_core(int msr_handler, PETU_t *pet);
PowerPlane_EnergyPolicy_t msr_get_energy_policy_for_core_no_pet(int msr_handler);

PowerPlane_EnergyPolicy_t msr_get_energy_policy_for_integ_gpu(int msr_handler, PETU_t *pet);
PowerPlane_EnergyPolicy_t msr_get_energy_policy_for_integ_gpu_no_pet(int msr_handler);



/*
 * Getting DRAM energy consumption
 */
double msr_get_energy_for_DRAM(int msr_handler, PETU_t *pet);
double msr_get_energy_for_DRAM_no_pet(int msr_handler);

#endif /* RAPL_WRAPPER_H_ */


