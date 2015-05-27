RAPL wrapper for Intel Processors using MSRs
============================================

This project is mainly based on the code written by Vince Weaver, and known as rapl-read.c file.

I tried to make it a little bit more function-oriented in order to use it in my research. Therefore, I began to represent everything as functions. Keep in mind that I wrapped the codes in the RAPL-read file with functions and did nothing systematically.

Build
-----

In order to build this package, please use:

    make all

This will generate an object file (rapl-wrapper.o file) in current folder. It is now ready to be used.

How to use
----------

In order to find out how to use this library (as an object file), please take a look at the Makefile at example folder (example/Makefile).

Structures
----------

I also present some of the informations as structures in order to transfer values between functions 
easily. Some of the structures are presented in here. 


PETU_t structure
----------------

```C
typedef struct PETU {
	double power_units;			// in Watts
	double energy_units;		// in Joules
	double time_units;			// in Seconds
} PETU_t;
```
PETU_t represents power, energy, and time units that are used extensively by MSR functions. 


PackagePowerInfo_t
------------------
```C
typedef struct PackagePowerInfo {
	double thermal_spec_power;	// in Watts
	double minimum_power;		// in Watts
	double maximum_power;		// in Watts
	double time_window;			// in Seconds
} PackagePowerInfo_t;
```
This structure is used for output of getting power info about packages.


PackagePowerLimit_t
-------------------
```C
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
```
This structure is used for output of getting power limitations in packages.



PowerPlane_EnergyPolicy_t
-------------------------
```C
typedef struct PowerPlane_EnergyPolicy {
	double energy;					// in Joules
	int policy;
	double accum_throttled_time;	// in Seconds; only for Cores (PP0) and not Integrated GPUs (PP1)
} PowerPlane_EnergyPolicy_t;
```
This structure is used for output of getting energy consumptions for cores and integrated GPUs.


Functions
---------

Function names are pretty much expressive enough to point to what they are doing. Therefore, right now, there is no comprehensive documentations regarding that. However, it will be added in the future.

Example
-------
There is simple example on how to use this package at example/ folder. Please take a look at it.

Contact
-------
If you have any questions regarding these codes, please send me an email to mghane2 at uh.edu.

