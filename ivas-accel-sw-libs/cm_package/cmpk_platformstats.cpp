#include <stdio.h>
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <unistd.h>
#include <dirent.h>
#include <string.h>
#include <sys/sysinfo.h>

#include "cmpk_platformstats.hpp"

void skip_lines(FILE *fp, int numlines)
{
	int cnt; 
	char ch;

	cnt = 0;

	while((cnt < numlines) && ((ch = getc(fp)) != EOF))
	{
		if (ch == '\n')
			cnt++;
	}
}



/************************** Function Definitions *****************************/
/*****************************************************************************/
/*
 *
 * This API opens /proc/stat file to read information for each CPU and store it in struct
 * /proc/stat displays the following columns of information for any CPU
 * user: time spent on processes executing in user mode with normal priority
 * nice: time spent on processes executing in user mode with "niced" priority
 * system: time spent on processes executing in kernel mode
 * idle: time spent idling (no CPU instructions) while there were no disk I/O requests
 * outstanding.
 * iowait: time spent idling while there were outstanding disk I/O requests.
 * irq: time spent servicing interrupt requests.
 * softirq: time spent servicing softirq.
 *
 * @param	cpu_stat: store CPU stats
 * @param	cpu_id: CPU id for which the details must be caputred.
 *
 * @return	None.
 *
 * @note		Internal API only.
 *
 ******************************************************************************/
int get_stats(struct cpustat *cpu_stat, int cpu_id)
{
	FILE *fp;

	fp = fopen("/proc/stat", "r");

	if(fp == NULL)
	{
		printf("Unable to open /proc/stat. Returned errono: %d", errno);
		return(errno);
	}
	else
	{
		int lskip;
		char cpun[255];

		lskip = cpu_id+1;
		skip_lines(fp, lskip);

		fscanf(fp,"%s %ld %ld %ld %ld %ld %ld %ld", cpun,
				&(cpu_stat->user), &(cpu_stat->nice), &(cpu_stat->system),
				&(cpu_stat->idle), &(cpu_stat->iowait), &(cpu_stat->irq),
				&(cpu_stat->softirq));

		fclose(fp);
	}

	return(0);
}

/*****************************************************************************/
/*
 *
 * This API prints CPU stats stored in given structure for particular CPU id 
 *
 * @param	cpu_stat: struct that stores CPU stats
 * @param	cpu_id: CPU id for which the details must be caputred.
 *
 * @return	None.
 *
 * @note		Internal API only.
 *
 ******************************************************************************/
int print_cpu_stats(struct cpustat *st, int cpu_id)
{
	printf("CPU%d: %ld %ld %ld %ld %ld %ld %ld\n", cpu_id, (st->user), (st->nice), 
			(st->system), (st->idle), (st->iowait), (st->irq),
			(st->softirq));

	return(0);
}

/*****************************************************************************/
/*
 *
 * This API calculates CPU util in real time, by computing delta at two time instances.
 * By default the interval between two time instances is 1s if not specified. 
 *
 * @param	prev: CPU stats at T0
 * @param	curr: CPU stats at T1
 *
 * @return	cpu_util.
 *
 * @note		Internal API only.
 *
 ******************************************************************************/
double calculate_load(struct cpustat *prev, struct cpustat *curr)
{
	unsigned long idle_prev, idle_curr, nidle_prev, nidle_curr;
	unsigned long total_prev, total_curr;
	double total_delta, idle_delta, cpu_util; 

	idle_prev=(prev->idle)+(prev->iowait);
	idle_curr=(curr->idle)+(curr->iowait);

	nidle_prev = (prev->user) + (prev->nice) + (prev->system) + (prev->irq) + (prev->softirq);
	nidle_curr = (curr->user) + (curr->nice) + (curr->system) + (curr->irq) + (curr->softirq);

	total_prev = idle_prev + nidle_prev;
	total_curr = idle_curr + nidle_curr;

	total_delta = (double) total_curr - (double) total_prev;
	idle_delta = (double) idle_curr - (double) idle_prev;

	cpu_util = (1000 * (total_delta - idle_delta) / total_delta + 1) / 10;

	return (cpu_util);
}

/*****************************************************************************/
/*
 *
 * This API identifies the number of configured CPUs in the system. For each
 * active CPU it reads the CPU stats by calling get_stats and then calculates
 * load.
 *
 * @param	verbose_flag: Enable verbose prints on stdout
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
int print_cpu_utilization(int verbose_flag)
{
	int num_cpus_conf, cpu_id;
	struct cpustat *st0_0, *st0_1;

	num_cpus_conf= get_nprocs_conf();
	cpu_id=0;

	st0_0 = (cpustat *)malloc(num_cpus_conf * sizeof (struct cpustat));
	st0_1 = (cpustat *)malloc(num_cpus_conf * sizeof (struct cpustat));

	if(!st0_0 || !st0_1)
	{
		printf("Unable to allocate memory, malloc failed");
		return(errno);
	}

	printf("CPU Utilization\n");
	for(; cpu_id < num_cpus_conf; cpu_id++)
	{
		st0_0[cpu_id].total_util = 0;
		get_stats(&st0_0[cpu_id],cpu_id);

		if(verbose_flag)
		{
			printf("cpu_id=%d\nStats at t0\n",cpu_id);
			print_cpu_stats(&st0_0[cpu_id],cpu_id);
		}
	}

	sleep(1);

	cpu_id = 0;

	for(; cpu_id < num_cpus_conf; cpu_id++)
	{
		st0_1[cpu_id].total_util = 0;
		get_stats(&st0_1[cpu_id],cpu_id);
		st0_1[cpu_id].total_util = calculate_load(&st0_0[cpu_id],&st0_1[cpu_id]);

		if(verbose_flag)
		{
			printf("Stats at t1 after 1s\n");
			print_cpu_stats(&st0_1[cpu_id],cpu_id);
		}
		printf("CPU%d\t:     %lf%%\n",cpu_id,st0_1[cpu_id].total_util);
	}

	printf("\n");
	free(st0_0);
	free(st0_1);

	return(0);
}
/*****************************************************************************/
/*
 *
 * This API identifies the number of configured CPUs in the system. For each
 * active CPU it reads the CPU frequency by opening
 * /sys/devices/system/cpu/<cpuid>/cpufreq/cpuinfo_cur_freq.
 *
 * @param	verbose_flag: Enable verbose prints on stdout
 *
 * @return	cpu_freq.
 *
 * @note		Internal API.
 *
 ******************************************************************************/

int get_cpu_frequency(int cpu_id, float* cpu_freq)
{
	FILE *fp;
	char base_filepath[] = "/sys/devices/system/cpu/cpu";
	char cpu_id_str[50];
	char *filename;

	filename = (char *)malloc(255);

	sprintf(cpu_id_str,"%d",cpu_id);
	strcpy(filename,base_filepath);
	strcat(filename,cpu_id_str);
	strcat(filename,"/cpufreq/cpuinfo_cur_freq");

	fp = fopen(filename, "r");
	if(fp == NULL)
	{
		printf(" File open returned with error : %s\n", strerror(errno));
		if(errno == 13)
		{
			printf("This command has to be run under the root user \n");
		}
		exit(0);

	}

	fscanf(fp,"%f",cpu_freq);
	fclose(fp);
	return(0);
}

/*****************************************************************************/
/*
 *
 * This API identifies the number of configured CPUs in the system. For each
 * active CPU it reads the CPU frequency by calling get_cpu_freq and prints it.
 *
 * @param	verbose_flag: Enable verbose prints on stdout
 *
 * @return	None.
 *
 * @note		None.
 *
 ******************************************************************************/
int print_cpu_frequency(int verbose_flag)
{
	int num_cpus_conf, cpu_id;
	float cpu_freq = 0;

	num_cpus_conf= get_nprocs_conf();
	cpu_id=0;

	printf("CPU Frequency\n");
	for(; cpu_id < num_cpus_conf; cpu_id++)
	{
		get_cpu_frequency(cpu_id,&cpu_freq);
		printf("CPU%d\t:    %f MHz\n",cpu_id,(cpu_freq)/1000);
	}
	printf("\n");

	return(0);
}

/*****************************************************************************/
/*
 *
 * This API scans the following information about physical memory:
 * MemTotal: Total usable physical ram
 * MemFree: The amount of physical ram, in KB, left unused by the system
 * MemAvailable: An estimate on how much memory is available for starting new
 * applications, without swapping.It can be timated from MemFree, Active(file),
 * Inactive(file), and SReclaimable, as well as the "low"
 * watermarks from /proc/zoneinfo.
 *
 * @param        MemTotal: Total usable physical ram size
 * @param        MemFree: amount of RAM left unsused
 * @param        MemAvailable: estimate of amount of memory available to start a new
 * app
 *
 * @return       Error code.
 *
 * @note         Internal API.
 *
 ******************************************************************************/
int get_ram_memory_utilization(unsigned long* MemTotal, unsigned long* MemFree, unsigned long* MemAvailable)
{
	//read first three lines of file
	//print to terminal
	FILE *fp;

	fp = fopen("/proc/meminfo", "r");

	if(fp == NULL)
	{
		printf("Unable to open /proc/stat. Returned errono: %d", errno);
		return(errno);
	}
	else
	{
		char buff[80];

		fscanf(fp," %s %ld",buff,MemTotal);
		skip_lines(fp,1);

		fscanf(fp," %s %ld",buff,MemFree);
		skip_lines(fp,1);

		fscanf(fp, "%s %ld",buff,MemAvailable);

		fclose(fp);
	}

	return(0);

}

/*****************************************************************************/
/*
 *
 * This API prints the following information about physical memory:
 * MemTotal: Total usable physical ram
 * MemFree: The amount of physical ram, in KB, left unused by the system
 * MemAvailable: An estimate on how much memory is available for starting new
 * applications, without swapping.It can be timated from MemFree, Active(file),
 * Inactive(file), and SReclaimable, as well as the "low"
 * watermarks from /proc/zoneinfo.
 *
 * @param        verbose_flag: Enable verbose prints
 * @param        MemAvailable: estimate of amount of memory available to start a new
 * app
 *
 * @return       Error code.
 *
 * @note         None.
 *
 ******************************************************************************/
int print_ram_memory_utilization(int verbose_flag)
{
	unsigned long MemTotal=0, MemFree=0, MemAvailable=0;
	int mem_util_ret;

	mem_util_ret = 0;

	mem_util_ret = get_ram_memory_utilization(&MemTotal, &MemFree, &MemAvailable);

	printf("RAM Utilization\n");
	printf("MemTotal      :     %ld kB\n",MemTotal);
	printf("MemFree	      :     %ld kB\n", MemFree);
	printf("MemAvailable  :     %ld kB\n\n", MemAvailable);

	return(mem_util_ret);

}

/*****************************************************************************/
/*
 *
 * This API prints the following information about physical memory:
 * CMATotal: Total CMA information
 * CMAFree: The CMA alloc free information
 *
 * @param        verbose_flag: Enable verbose prints
 *
 * @return       Error code.
 *
 * @note         Internal API.
 *
 ******************************************************************************/
int get_cma_utilization(unsigned long* CmaTotal, unsigned long* CmaFree)
{
	FILE *fp;

	fp = fopen("/proc/meminfo", "r");

	if(fp == NULL)
	{
		printf("Unable to open /proc/stat. Returned errono: %d", errno);
		return(errno);
	}
	else
	{
		char buff[80];

		skip_lines(fp,41);
		fscanf(fp," %s %ld",buff,CmaTotal);

		skip_lines(fp,1);
		fscanf(fp," %s %ld",buff,CmaFree);

		fclose(fp);
	}

	return(0);

}

/*****************************************************************************/
/*
 *
 * This API prints the following information about physical memory:
 * CMATotal: Total CMA information
 * CMAFree: The CMA alloc free information
 *
 * @param        verbose_flag: Enable verbose prints
 *
 * @return       Error code.
 *
 * @note         None.
 *
 ******************************************************************************/
int print_cma_utilization(int verbose_flag)
{
	unsigned long CmaTotal=0, CmaFree=0;
	int cma_util_ret;

	cma_util_ret = 0;

	cma_util_ret = get_cma_utilization(&CmaTotal, &CmaFree);

	printf("CMA Mem Utilization\n");
	printf("CmaTotal   :     %ld kB\n",CmaTotal);
	printf("CmaFree    :     %ld kB\n\n", CmaFree);

	return(cma_util_ret);

}

/*****************************************************************************/
/*
 *
 * This API scans the following information about physical swap memory:
 * SwapTotal: Total usable physical swap memory
 * SwapFree: The amount of swap memory free. Memory which has been evicted from RAM, 
 * and is temporarily on the disk.
 *
 * @param        SwapTotal: Total usable physical swap size
 * @param        SwapFree: amount of swap memory free
 *
 * @return       Error code.
 *
 * @note         Internal API.
 *
 ******************************************************************************/
int get_swap_memory_utilization(unsigned long* SwapTotal, unsigned long* SwapFree)
{
	FILE *fp;

	fp = fopen("/proc/meminfo", "r");

	if(fp == NULL)
	{
		printf("Unable to open /proc/stat. Returned errono: %d", errno);
		return(errno);
	}
	else
	{
		char buff[80];

		skip_lines(fp,14);
		fscanf(fp," %s %ld",buff,SwapTotal);

		skip_lines(fp,1);
		fscanf(fp," %s %ld",buff,SwapFree);

		fclose(fp);
	}

	return(0);

}

/*****************************************************************************/
/*
 *
 * This API prints the following information about swap memory:
 * SwapTotal: Total usable physical swap memory
 * SwapFree: The amount of swap memory free. Memory which has been evicted from RAM, 
 * and is temporarily on the disk.
 *
 * @param        verbose_flag: Enable verbose prints
 *
 * @return       Error code.
 *
 * @note         None.
 *
 ******************************************************************************/
int print_swap_memory_utilization(int verbose_flag)
{
	unsigned long SwapTotal=0, SwapFree=0;
	int mem_util_ret;

	mem_util_ret = 0;

	mem_util_ret = get_swap_memory_utilization(&SwapTotal, &SwapFree);

	printf("Swap Mem Utilization\n");
	printf("SwapTotal    :    %ld kB\n",SwapTotal);
	printf("SwapFree     :    %ld kB\n\n",SwapFree);

	return(mem_util_ret);

}

/*****************************************************************************/
/*
 *
 * This API reads the sysfs enteries for a given sysfs file
 *
 * @param	filename: sysfs path
 * @param	value: value read from sysfs entry
 *
 * @return       None
 *
 * @note         None.
 *
 ******************************************************************************/
int read_sysfs_entry(char* filename, char* value)
{

	FILE *fp;

	fp = fopen(filename,"r");

	if(fp == NULL)
	{
		printf("Unable to open %s\n",filename);
		return(errno);
	}

	fscanf(fp,"%s",value);

	return(0);

}

/*****************************************************************************/
/*
 *
 * This API returns the number of hwmon devices registered under /sys/class/hwmon
 *
 * @return       num_hwmon_devices: Number of registered hwmon devices
 *
 * @note         None.
 *
 ******************************************************************************/
int count_hwmon_reg_devices()
{
	//find number of hwmon devices listed under
	int num_hwmon_devices;
	DIR *d;
	struct dirent *dir;

	num_hwmon_devices = 0;
	d = opendir("/sys/class/hwmon");

	if(!d)
	{
		printf("Unable to open /sys/class/hwmon path\n");
		return(errno);
	}

	while((dir = readdir(d)) != NULL)
	{
		if(strstr(dir->d_name, "hwmon"))
		{
			num_hwmon_devices++;
		}
	}

	closedir(d);

	return(num_hwmon_devices);
}

/*****************************************************************************/
/*
 *
 * This API returns hwmon_id of the specified device:
 *
 * @param        name: device name for which hwmon_id needs to be identified
 *
 * @return       hwmon_id
 *
 * @note         None.
 *
 ******************************************************************************/
int get_device_hwmon_id(int verbose_flag, char* name)
{
	//find number of hwmon devices listed under
	int num_hwmon_devices,hwmon_id;
	char hwmon_id_str[50];
	char *device_name;
	char *filename;

	filename = (char*)malloc(255);
	device_name = (char*)malloc(255);

	hwmon_id=-1;

	num_hwmon_devices = count_hwmon_reg_devices();

	for(hwmon_id = 0; hwmon_id < num_hwmon_devices; hwmon_id++)
	{
		sprintf(hwmon_id_str,"%d",hwmon_id);
		strcpy(filename,"/sys/class/hwmon/hwmon");
		strcat(filename,hwmon_id_str);
		strcat(filename,"/name");

		read_sysfs_entry(filename,device_name);

		if(!strcmp(name,device_name))
		{
			return(hwmon_id);
		}

		if(verbose_flag)
		{
			printf("filename %s\n",filename);
			printf("device_name = %s\n",device_name);
		}
	}

	free(filename);
	free(device_name);
	return(-1);
}

/*****************************************************************************/
/*
 *
 * This API prints the following information about power utilization for ina260:
 * in1_input: Voltage input value.
 * curr1_input: Current input value.
 * power1_input: Instantaneous power use
 *
 * @param        verbose_flag: Enable verbose prints
 *
 * @return       Error code.
 *
 * @note         None.
 *
 ******************************************************************************/
int print_ina260_power_info(int verbose_flag)
{
	int hwmon_id;
	long total_power, total_current, total_voltage;
	FILE *fp;
	char filename[255];
	char hwmon_id_str[255];

	char base_filepath[] = "/sys/class/hwmon/hwmon";

	hwmon_id = get_device_hwmon_id(verbose_flag,"ina260_u14");

	printf("Power Utilization\n");
	if(hwmon_id == -1)
	{
		printf("no hwmon device found for ina260_u14 under /sys/class/hwmon\n");
		return(0);
	}

	//printf("hwmon device found, device_id is %d\n",hwmon_id);

	sprintf(hwmon_id_str,"%d",hwmon_id);
	strcat(base_filepath,hwmon_id_str);

	//if "power" file exists then read power value
	strcpy(filename,base_filepath);
	strcat(filename,"/power1_input");

	fp = fopen(filename,"r");
	if(fp == NULL)
	{
		printf("unable to open %s\n",filename);
	}

	fscanf(fp,"%ld",&total_power);
	fclose(fp);

	//if "curr" file exists then read curr value
	strcpy(filename,base_filepath);
	strcat(filename,"/curr1_input");

	fp = fopen(filename,"r");
	if(fp == NULL)
	{
		printf("unable to open %s\n",filename);
	}

	fscanf(fp,"%ld",&total_current);
	fclose(fp);


	//if "voltage" file exists then read voltage value
	strcpy(filename,base_filepath);
	strcat(filename,"/in1_input");

	fp = fopen(filename,"r");
	if(fp == NULL)
	{
		printf("unable to open %s\n",filename);
	}

	fscanf(fp,"%ld",&total_voltage);
	fclose(fp);

	printf("SOM total power    :     %ld mW\n",(total_power)/1000);
	printf("SOM total current  :     %ld mA\n",total_current);
	printf("SOM total voltage  :     %ld mV\n\n",total_voltage);


	return(0);
}






/*****************************************************************************/
/*
 *
 * This API prints the following information about power utilization for ina260:
 * Power of SOM (mW)
 * 
 * @param        None 
 *
 * @return       Error code.
 *
 * @note         None.
 *
 ******************************************************************************/



long cmpk_getSomPowerInfo(int hwmonid)
{
	int hwmon_id;
	long total_power, total_current, total_voltage;
	FILE *fp;
	char filename[255];
	char hwmon_id_str[255];

	char base_filepath[] = "/sys/class/hwmon/hwmon";

	hwmon_id = hwmonid;

	if(hwmon_id == -1)
	{
		printf("no hwmon device found for ina260_u14 under /sys/class/hwmon\n");
		return -1;
	}

	//printf("hwmon device found, device_id is %d\n",hwmon_id);

	sprintf(hwmon_id_str,"%d",hwmon_id);
	strcat(base_filepath,hwmon_id_str);

	//if "power" file exists then read power value
	strcpy(filename,base_filepath);
	strcat(filename,"/power1_input");

	fp = fopen(filename,"r");
	if(fp == NULL)
	{
		printf("unable to open %s\n",filename);
		return -2;
	}

	fscanf(fp,"%ld",&total_power);
	fclose(fp);

	 
	return (total_power)/1000;
}


long cmpk_getSomPowerInfo()
{
	return  cmpk_getSomPowerInfo(1);
}


/**
 * @description: 获取对应的信息
 * @param {int} target 1 2 3
 * @param {int} id kv260 是 1
 * @return {*} temp * 1000
 */
long cmpk_getPlatformTempInfo(int target, int hwmon_id){
	char filename[255];
	
	char base_filepath[255];
	char hwmon_id_str[255];

	long temp;
	FILE *fp;

	if(target>2 or target <0)
	{
		return 0;
	}

	sprintf(base_filepath,"/sys/class/hwmon/hwmon%d",hwmon_id);
	sprintf(filename,"/temp%d_input",target);
	strcat(base_filepath,filename);

	fp = fopen(base_filepath,"r");
	if(fp == NULL)
	{
		printf("unable to open %s\n",base_filepath);
	}

	fscanf(fp,"%ld",&temp);
	fclose(fp);
	return temp;
}


/*****************************************************************************/
/*
 *
 * This API prints the following information from sysmon driver:
 * in1_input: Voltage input value.
 * curr1_input: Current input value.
 * power1_input: Instantaneous power use
 *
 * @param        verbose_flag: Enable verbose prints
 *
 * @return       Error code.
 *
 * @note         None.
 *
 ******************************************************************************/
int print_sysmon_power_info(int verbose_flag)
{
	int hwmon_id;
	FILE *fp;
	char filename[255];
	char hwmon_id_str[255];
	long LPD_TEMP, FPD_TEMP, PL_TEMP;
	long VCC_PSPLL, PL_VCCINT, VOLT_DDRS, VCC_PSINTFP, VCC_PS_FPD;
	long PS_IO_BANK_500, VCC_PS_GTR, VTT_PS_GTR;

	char base_filepath[] = "/sys/class/hwmon/hwmon";

	hwmon_id = get_device_hwmon_id(verbose_flag,"ams");

	if(hwmon_id == -1)
	{
		printf("no hwmon device found for ams under /sys/class/hwmon\n");
		return(0);
	}

	//printf("hwmon device found, device_id is %d \n",hwmon_id);

	sprintf(hwmon_id_str,"%d",hwmon_id);
	strcat(base_filepath,hwmon_id_str);

	//Print temperature values
	strcpy(filename,base_filepath);
	strcat(filename,"/temp1_input");

	fp = fopen(filename,"r");
	if(fp == NULL)
	{
		printf("unable to open %s\n",filename);
	}

	fscanf(fp,"%ld",&LPD_TEMP);
	fclose(fp);

	//FPD temp
	strcpy(filename,base_filepath);
	strcat(filename,"/temp2_input");

	fp = fopen(filename,"r");
	if(fp == NULL)
	{
		printf("unable to open %s\n",filename);
	}

	fscanf(fp,"%ld",&FPD_TEMP);
	fclose(fp);

	//PL temp
	strcpy(filename,base_filepath);
	strcat(filename,"/temp3_input");

	fp = fopen(filename,"r");
	if(fp == NULL)
	{
		printf("unable to open %s\n",filename);
	}

	fscanf(fp,"%ld",&PL_TEMP);
	fclose(fp);

	//VCC_PSPLL
	strcpy(filename,base_filepath);
	strcat(filename,"/in1_input");

	fp = fopen(filename,"r");
	if(fp == NULL)
	{
		printf("unable to open %s\n",filename);
	}

	fscanf(fp,"%ld",&VCC_PSPLL);
	fclose(fp);

	//PL_VCCINT
	strcpy(filename,base_filepath);
	strcat(filename,"/in3_input");

	fp = fopen(filename,"r");
	if(fp == NULL)
	{
		printf("unable to open %s\n",filename);
	}

	fscanf(fp,"%ld",&PL_VCCINT);
	fclose(fp);

	//VOLT_DDRS
	strcpy(filename,base_filepath);
	strcat(filename,"/in6_input");

	fp = fopen(filename,"r");
	if(fp == NULL)
	{
		printf("unable to open %s\n",filename);
	}

	fscanf(fp,"%ld",&VOLT_DDRS);
	fclose(fp);

	//VCC_PSINTFP
	strcpy(filename,base_filepath);
	strcat(filename,"/in7_input");

	fp = fopen(filename,"r");
	if(fp == NULL)
	{
		printf("unable to open %s\n",filename);
	}

	fscanf(fp,"%ld",&VCC_PSINTFP);
	fclose(fp);

	//VCC_PS_FPD
	strcpy(filename,base_filepath);
	strcat(filename,"/in9_input");

	fp = fopen(filename,"r");
	if(fp == NULL)
	{
		printf("unable to open %s\n",filename);
	}

	fscanf(fp,"%ld",&VCC_PS_FPD);
	fclose(fp);

	// PS_IO_BANK_500
	strcpy(filename,base_filepath);
	strcat(filename,"/in13_input");

	fp = fopen(filename,"r");
	if(fp == NULL)
	{
		printf("unable to open %s\n",filename);
	}

	fscanf(fp,"%ld",&PS_IO_BANK_500);
	fclose(fp);

	//VCC_PS_GTR
	strcpy(filename,base_filepath);
	strcat(filename,"/in16_input");

	fp = fopen(filename,"r");
	if(fp == NULL)
	{
		printf("unable to open %s\n",filename);
	}

	fscanf(fp,"%ld",&VCC_PS_GTR);
	fclose(fp);

	//VTT_PS_GTR
	strcpy(filename,base_filepath);
	strcat(filename,"/in17_input");

	fp = fopen(filename,"r");
	if(fp == NULL)
	{
		printf("unable to open %s\n",filename);
	}

	fscanf(fp,"%ld",&VTT_PS_GTR);
	fclose(fp);

	printf("AMS CTRL\n");
	printf("System PLLs voltage measurement, VCC_PSLL   		:     %ld mV\n",VCC_PSPLL);
	printf("PL internal voltage measurement, VCC_PSBATT 		:     %ld mV\n",PL_VCCINT);
	printf("Voltage measurement for six DDR I/O PLLs, VCC_PSDDR_PLL :     %ld mV\n",VOLT_DDRS);
	printf("VCC_PSINTFP_DDR voltage measurement         		:     %ld mV\n\n",VCC_PSINTFP);

	printf("PS Sysmon\n");
	printf("LPD temperature measurement 		    		:     %ld C\n",(LPD_TEMP)/1000);
	printf("FPD temperature measurement (REMOTE)  		    	:     %ld C\n",(FPD_TEMP)/1000);
	printf("VCC PS FPD voltage measurement (supply 2)   		:     %ld mV\n",VCC_PS_FPD);
	printf("PS IO Bank 500 voltage measurement (supply 6)		:     %ld mV\n",PS_IO_BANK_500);
	printf("VCC PS GTR voltage   					:     %ld mV\n",VCC_PS_GTR);
	printf("VTT PS GTR voltage    					:     %ld mV\n\n",VTT_PS_GTR);

	printf("PL Sysmon\n");
	printf("PL temperature    					:     %ld C\n\n",(PL_TEMP)/1000);

	return(0);
}
/*****************************************************************************/
/*
 *
 * This API prints the following information about power utilization for the system:
 *
 * @param        verbose_flag: Enable verbose prints
 *
 * @return       Error code.
 *
 * @note         None.
 *
 ******************************************************************************/
int print_power_utilization(int verbose_flag)
{
	print_ina260_power_info(verbose_flag);
	print_sysmon_power_info(verbose_flag);

	return(0);
}

/*****************************************************************************/
/*
 *
 * This API calls all other APIs that read, compute and print different platform
 * stats
 *
 * @param        verbose_flag: Enable verbose prints on stdout
 * and printed
 *
 * @return       None.
 *
 * @note         None.
 *
 ******************************************************************************/
void print_all_stats(int verbose_flag)
{

	print_cpu_utilization(verbose_flag);

	print_ram_memory_utilization(verbose_flag);

	print_swap_memory_utilization(verbose_flag);

	print_power_utilization(verbose_flag);

	print_cma_utilization(verbose_flag);

	print_cpu_frequency(verbose_flag);
}