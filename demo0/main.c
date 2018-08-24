#include<stdio.h>
#include<stdlib.h>
#include<string.h>

#ifdef MAC
#include<OpenCL/cl.h>
#else
#include<CL/cl.h>
#endif


int main()
{
	cl_platform_id* platforms;
	cl_uint numberOfPlatforms;

	char* platformName;
	size_t platformNameSize;

	char* platformVendor;
	size_t platformVendorSize;

	char* platformVersion;
	size_t platformVersionSize;

	cl_device_id* devices;
	unsigned int numberOfDevices;

	cl_device_type deviceType;

	char* nameData;
	size_t nameSize;

	cl_uint maxComputeUnit;
	size_t maxWorkGroupSize;
	cl_uint maxDimention;
	size_t maxWorkItemSizes[3];

	//access platforms
	int err = clGetPlatformIDs(5, NULL, &numberOfPlatforms);
	if (err < 0)
	{
		perror("couldn't find any platform");
		exit(1);
	}
	platforms = (cl_platform_id*)malloc(sizeof(cl_platform_id) * numberOfPlatforms);
	clGetPlatformIDs(numberOfPlatforms, platforms, NULL);

	printf("there are %d platforms on this computer\n\n", numberOfPlatforms);

	for (int i = 0; i < numberOfPlatforms; i++)
	{
		//access platform information
		//get platform name
		err = clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, NULL, NULL, &platformNameSize);
		if (err < 0)
		{
			perror("could not read platform name");
			exit(1);
		}
		platformName = (char*)malloc(platformNameSize);
		clGetPlatformInfo(platforms[i], CL_PLATFORM_NAME, platformNameSize, platformName, NULL);



		//get platform vendor
		err = clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, NULL, NULL, &platformVendorSize);
		if (err < 0)
		{
			perror("could not read platform vendor");
			exit(1);
		}
		platformVendor = (char*)malloc(platformVendorSize);
		clGetPlatformInfo(platforms[i], CL_PLATFORM_VENDOR, platformVendorSize, platformVendor, NULL);

		//get platform version
		err = clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION, NULL, NULL, &platformVersionSize);
		if (err < 0)
		{
			perror("could not read platform version");
			exit(1);
		}
		platformVersion = (char*)malloc(platformVersionSize);
		clGetPlatformInfo(platforms[i], CL_PLATFORM_VERSION, platformVersionSize, platformVersion, NULL);



		printf("PLATFORM %d\nPLATFORM NAME:%s\nPLATFORM VENDOR:%s\nPLATFORM VERSION:%s\n\n", i, platformName, platformVendor, platformVersion);
		
		///////////////////////////////////////////////////////////////
		//access device information
		err = clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, NULL, NULL, &numberOfDevices);
		if (err < 0)
		{
			perror("couldn't acess any device");
			exit(1);
		}
		devices = (cl_device_id*)malloc(sizeof(cl_device_id)*numberOfDevices);
		clGetDeviceIDs(platforms[i], CL_DEVICE_TYPE_ALL, numberOfDevices, devices, NULL);


		printf("NUMBER OF AVAILABLE DEVICES:%d\n\n", numberOfDevices);

		for (int a = 0; a < numberOfDevices; a++)
		{
			//get device type
			clGetDeviceInfo(devices[a], CL_DEVICE_TYPE, sizeof(deviceType), &deviceType, NULL);
			if (deviceType & CL_DEVICE_TYPE_CPU)
				printf("DEVICE TYPE: CPU\n");
			if (deviceType & CL_DEVICE_TYPE_GPU)
				printf("DEVICE TYPE: GPU\n");



			//access device name
			err = clGetDeviceInfo(devices[a], CL_DEVICE_NAME, NULL, NULL, &nameSize);
			if (err < 0)
			{
				perror("couldn't read device name");
				exit(1);
			}
			nameData = (char*)malloc(nameSize);
			clGetDeviceInfo(devices[a], CL_DEVICE_NAME, nameSize, nameData, NULL);


			//get number of compute units
			err = clGetDeviceInfo(devices[a], CL_DEVICE_MAX_COMPUTE_UNITS, NULL, NULL, &maxComputeUnit);
			if (err < 0)
			{
				perror("could not read compute units");
				exit(1);
			}

			//get max work group size
			err = clGetDeviceInfo(devices[a], CL_DEVICE_MAX_WORK_GROUP_SIZE, NULL, NULL, &maxWorkGroupSize);
			if (err < 0)
			{
				perror("could not read max work group size");
				exit(1);
			}


			//get max work dimentions
			err = clGetDeviceInfo(devices[a], CL_DEVICE_MAX_WORK_ITEM_DIMENSIONS, NULL, NULL, &maxDimention);
			if (err < 0)
			{
				perror("could not read work dimentions");
				exit(1);
			}

			//get max work item sizes
			err = clGetDeviceInfo(devices[a], CL_DEVICE_MAX_WORK_ITEM_SIZES, sizeof(maxWorkItemSizes), &maxWorkItemSizes, NULL);
			if (err < 0)
			{
				perror("could not read max work item sizes");
				exit(1);
			}


			//print
			printf("DEVICE NAME:%s\nNUMBER OF COMPUTE UNITS:%d\nMAX WORK GROUP SIZE:%d\nMAX WORK DIMENTIONS:%d\nMAX WORK ITEM SIZES:(%d, %d, %d)\n\n",
				nameData, maxComputeUnit, maxWorkGroupSize, maxDimention, maxWorkItemSizes[0], maxWorkItemSizes[1], maxWorkItemSizes[2]);
		}
		
		printf("--------------------------\n");

	}
	///////////////////////////////////////////////
	cl_device_id aDevice;
	cl_context context;
	cl_command_queue commandQueue;

	//Create a context with one device (if a GPU is available use the GPU device, otherwise use the CPU). 
	err = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_GPU, 1, &aDevice, NULL);
	if (err == CL_DEVICE_NOT_FOUND)
	{
		err = clGetDeviceIDs(platforms[0], CL_DEVICE_TYPE_CPU, 1, &aDevice, NULL);
	}
	if (err < 0)
	{
		perror("could not access device");
		exit(1);
	}
	
	context = clCreateContext(NULL, 1, &aDevice, NULL, NULL, &err);
	if (err < 0)
	{
		perror("could not create context");
		exit(1);
	}

	//Create a command queue for the context and device. 
	commandQueue = clCreateCommandQueue(context, aDevice, NULL, &err);
	if (err < 0)
	{
		perror("could not create command queue");
		exit(1);
	}

	///////////////////////////////////////////////
	//Read the program source code from the provided ¡°source.cl¡± file and build the program.
	const char* fileName = "source.cl";
	FILE* fstream; 
	cl_program program;
	char *programBuffer, *programLog;
	size_t programSize, logSize;

	fstream = fopen(fileName, "r");
	if (fstream == NULL)
	{
		perror("could not find program file");
		exit(1);
	}
	fseek(fstream, 0, SEEK_END);//go to the end of file to get file size
	programSize = ftell(fstream);
	rewind(fstream);//go back to the start of file and read file 
	programBuffer = (char*)malloc(programSize + 1);
	programBuffer[programSize] = '\0';
	fread(programBuffer, sizeof(char), programSize, fstream);
	fclose(fstream);


	//create program from file
	program = clCreateProgramWithSource(context, 1, (const char**)&programBuffer, &programSize, &err);
	if (err < 0)
	{
		perror("could not create program");
		exit(1);
	}
	free(programBuffer);

	//Display whether or not the program built successfully and display the program build log.
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err < 0)
	{
		clGetProgramBuildInfo(program, aDevice, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
		programLog = (char*)malloc(logSize + 1);
		programLog[logSize] = '\0';
		clGetProgramBuildInfo(program, aDevice, CL_PROGRAM_BUILD_LOG, logSize, programLog, NULL);
		
		printf("%s\n", programLog);

		getchar();

		free(programLog);
		exit(1);
	}
	printf("the program builds successfully\n");

	///////////////////////////////////////////////
	cl_kernel* kernels;
	cl_uint numberOfKernels;
	
	char* kernelName;
	size_t kernelNameSize;


	//Find and display the number of kernels in the program. 
	err = clCreateKernelsInProgram(program, 0, NULL, &numberOfKernels);
	if (err < 0)
	{
		perror("could not find any kernels");
		exit(1);
	}
	kernels = (cl_kernel*)malloc(sizeof(cl_kernel)*numberOfKernels);
	clCreateKernelsInProgram(program, numberOfKernels, kernels, NULL);

	printf("there are %d kernels in the program: ", numberOfKernels);

	//display all the kernel function names
	for (int i = 0; i < numberOfKernels; i++)
	{
		err = clGetKernelInfo(kernels[i], CL_KERNEL_FUNCTION_NAME, 0, NULL, &kernelNameSize);
		if (err < 0)
		{
			perror("could not read kernel name");
			exit(1);
		}
		kernelName = (char*)malloc(kernelNameSize);
		clGetKernelInfo(kernels[i], CL_KERNEL_FUNCTION_NAME, kernelNameSize, kernelName, NULL);

		printf("%s , ", kernelName);

	}

	printf("\n");
	///////////////////////////////////////////////

	getchar();

	free(platforms);
	return 0;


}