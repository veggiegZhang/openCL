#define _CRT_SECURE_NO_WARNINGS

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>

#ifdef MAC
#include <OpenCL/cl.h>
#else  
#include <CL/cl.h>
#endif


cl_device_id createDevice()
{
	int err;

	//identify a platform
	cl_platform_id platform;
	err = clGetPlatformIDs(1, &platform, NULL);
	if (err < 0)
	{
		perror("couldn't access any platform");
		exit(1);
	}


	//access device
	cl_device_id device;
	err = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, 1, &device, NULL);
	if (err < 0)
	{
		perror("couldn't find GPU");
		exit(1);
	}

	return device;
}


cl_program readProgram(cl_device_id device, cl_context context, const char* fileName)
{
	int err;

	//read program file and place content into buffer
	FILE* programHandle;
	char *programBuffer, *programLog;
	size_t programSize, logSize;
	cl_program program;

	//read program from file
	programHandle = fopen(fileName, "r");
	if (programHandle == NULL)
	{
		perror("could not open file");
		exit(1);
	}
	fseek(programHandle, 0, SEEK_END);
	programSize = ftell(programHandle);
	rewind(programHandle);
	programBuffer = (char*)malloc(programSize + 1);
	programBuffer[programSize] = '\0';
	fread(programBuffer, sizeof(char), programSize, programHandle);
	fclose(programHandle);


	//create program
	program = clCreateProgramWithSource(context, 1, (const char**)&programBuffer, &programSize, &err);
	if (err < 0)
	{
		perror("could not create program");
		exit(1);
	}
	free(programBuffer);

	//build program
	err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
	if (err < 0)
	{
		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, 0, NULL, &logSize);
		programLog = (char*)malloc(logSize + 1);
		programLog[logSize] = '\0';

		clGetProgramBuildInfo(program, device, CL_PROGRAM_BUILD_LOG, logSize + 1, programLog, NULL);
		printf("%s\n", programLog);

		free(programLog);
		exit(1);
	}

	return program;
}


cl_mem createInputBuffer(cl_context context, size_t bufferSize, void* bufferData)
{
	cl_mem inputBuffer;
	int err;

	inputBuffer = clCreateBuffer(context, CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR, bufferSize, bufferData, &err);
	if (err < 0)
	{
		perror("could not create a buffer object for input data");
		exit(1);
	}

	return inputBuffer;
}


cl_mem createOutputBuffer(cl_context context,size_t bufferSize)
{
	cl_mem outputBuffer;
	int err;

	outputBuffer = clCreateBuffer(context, CL_MEM_WRITE_ONLY, bufferSize, NULL, &err);
	if (err < 0)
	{
		perror("could not create a buffer object for output data");
		exit(1);
	}


	return outputBuffer;
}



int main()
{
	int err = 0;


	//access a device
	cl_device_id device;
	device = createDevice();
	
	
	//create context
	cl_context context;
	context = clCreateContext(NULL, 1, &device, NULL, NULL, err);
	if (err < 0)
	{
		perror("could not create context");
		exit(1);
	}
	
	//create program
	const char* programName = "vecadd.cl";
	cl_program program = readProgram(device,context, programName);


	//create kernel 
	cl_kernel kernel;
	const char* kernelFunction = "vecadd";

	kernel = clCreateKernel(program, kernelFunction, &err);
	if (err < 0)
	{
		perror("could not create kernel");
		exit(1);
	}

	/////////////////////////////////////////////////
	//initialize values
	float a = 1.0f, b = 2.0f;
	float* h_a = &a;
	float* h_b = &b;

	float c = 0.0f;
	float* result = &c;

	//create buffer to hold I/O data
	cl_mem matrixrixBuffer, vecBuffer, resultBuffer;
	matrixrixBuffer = createInputBuffer(context, sizeof(float), h_a);
	vecBuffer = createInputBuffer(context, sizeof(float), h_b);
	resultBuffer = createOutputBuffer(context, sizeof(float));
	
	//create kernel arguments from buffers
	cl_mem CLBuffers[3] = {matrixrixBuffer,vecBuffer,resultBuffer};

	for (int i = 0; i < 3; i++)
	{
		err = clSetKernelArg(kernel, i, sizeof(cl_mem), &CLBuffers[i]);
		if (err < 0)
		{
			perror("could not set the kernel argument for buffer " + i);
			exit(1);
		}
	}
	///////////////////////////////////////////////////////////////////
	//create a CL command queue for the device
	cl_command_queue queue;

	queue = clCreateCommandQueue(context, device, 0, &err);
	if (err < 0)
	{
		perror("could not create command queue");
		exit(1);
	}

	///////////////////////////////////////////////////////////////////
	//enqueue command queue to device
	size_t workUnitPerKernel = 1;

	err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &workUnitPerKernel, NULL, 0, NULL, NULL);
	if (err < 0)
	{
		perror("could not enqueue the kernel execution");
		exit(1);
	}

	//read the result
	err = clEnqueueReadBuffer(queue, resultBuffer, CL_TRUE, 0, sizeof(float), result, 0, NULL, NULL);
	if (err < 0)
	{
		perror("could not enqueue read buffer command ");
		exit(1);
	}


	printf("%f + %f = %f\n", a, b, *result);

	getchar();

	/////////////////////////////////////////////////////////
	//clean up
	for (int i = 0; i < 3; i++)
	{
		clReleaseMemObject(CLBuffers[i]);
	}
	clReleaseKernel(kernel);
	clReleaseCommandQueue(queue);
	clReleaseProgram(program);
	clReleaseContext(context);

	return 0;

	
}