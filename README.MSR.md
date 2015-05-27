
Infomation about the CPUs and MSRs
==================================


/proc/cpuinfo:
--------------

	This is a textual file that contains detailed information about the CPU, its cores and so many other valuable info.



MSRs:
-----

	MSR directories could be found at the following addresses. By opening these files, we can get access to MSRs. 

		/dev/cpu/[CPUNUM]/msr

	
	You should be root or be a member of group root.

	To load the driver, run following command:

		modprobe msr

	Also note that if you do not have the MSR access at all, it is better to install following package:

		apt-get install msr-tools

	To see whether your linux supports the MSR or not, use following command:

		cat /boot/config-<some_numbers> | grep MSR

	and if you see something like this, you have the MSR support. Otherwise, you should recompile the kernel.
	
		CONFIG_X86_MSR=m (or y)






Source: http://manpages.ubuntu.com/manpages/saucy/man4/msr.4.html
Source: http://linux.koolsolutions.com/2009/09/19/howto-using-cpu-msr-tools-rdmsrwrmsr-in-debian-linux/
