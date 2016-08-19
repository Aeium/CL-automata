/**********************************************************************
Copyright ©2015 Advanced Micro Devices, Inc. All rights reserved.

Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:

•   Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
•   Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or
 other materials provided with the distribution.

THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED
 WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE FOR ANY
 DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING
 NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
********************************************************************/
#include "SimpleMultiDevice.hpp"



int
Device::createContext()
{
    //Create context using current device's ID
    context = clCreateContext(cprops,
                              1,
                              &deviceId,
                              0,
                              0,
                              &status);
    CHECK_OPENCL_ERROR(status, "clCreateContext failed.");

    return SDK_SUCCESS;
}

int
Device::createQueue()
{
    //Create Command-Queue
    queue = clCreateCommandQueue(context,
                                 deviceId,
                                 CL_QUEUE_PROFILING_ENABLE,
                                 &status);
    CHECK_OPENCL_ERROR(status, "clCreateCommandQueue failed.");

    return SDK_SUCCESS;
}

int
Device::createBuffers()
{
    // Create input buffer
    inputBuffer = clCreateBuffer(context,
                                 CL_MEM_READ_ONLY,
                                 100 * sizeof(cl_uint),
                                 0,
                                 &status);
    CHECK_OPENCL_ERROR(status, "clCreateBuffer failed.(inputBuffer)");

    // Create output buffer
    outputBuffer = clCreateBuffer(context,
                                  CL_MEM_WRITE_ONLY,
                                  width * sizeof(cl_float),
                                  0,
                                  &status);
    CHECK_OPENCL_ERROR(status, "clCreateBuffer failed.(outputBuffer)");

    return SDK_SUCCESS;
}

int
Device::enqueueWriteBuffer(int seed)
{
	
	//std::cout <<("input:");
	
	srand(seed);
	
    for(int i = 0; i < 100; i++)
    {
        input[i] = (cl_uint) (rand() % 2);
		
		
		std::cout << input[i];
    }
	
	std::cout << "\tbefore Enqueue : " << std::endl;
	
    // Initialize input buffer
    status = clEnqueueWriteBuffer(queue,
                                  inputBuffer,
                                  1,
                                  0,
                                  100 * sizeof(cl_uint),
                                  input,
                                  0,
                                  0,
                                  0);
    CHECK_OPENCL_ERROR(status, "clEnqueueWriteBuffer failed.");

	std::cout << "\tafter Enqueue : " << std::endl;
	
    return SDK_SUCCESS;
}

int
Device::createProgram(const char **source, const size_t *sourceSize)
{
    // Create program with source
    program = clCreateProgramWithSource(context,
                                        1,
                                        source,
                                        sourceSize,
                                        &status);
    CHECK_OPENCL_ERROR(status, "clCreateProgramWithSource failed.");

    return SDK_SUCCESS;
}

int
Device::buildProgram()
{
	
	std::cout << "HELLO IS THIS BUILDING HERE WTF WHY IS THIS HERE THEN~~~~~~~~~~~~~~~~~~~~~~~~~~~~~" << std::endl;
	
    char buildOptions[50];
    sprintf(buildOptions, "-D KERNEL_ITERATIONS=%d", KERNEL_ITERATIONS);
    // Build program source
    status = clBuildProgram(program,
                            1,
                            &deviceId,
                            buildOptions,
                            0,
                            0);
    // Print build log here if build program failed
    if(status != CL_SUCCESS)
    {
        if(status == CL_BUILD_PROGRAM_FAILURE)
        {
            cl_int logStatus;
            char *buildLog = NULL;
            size_t buildLogSize = 0;
            logStatus = clGetProgramBuildInfo(program,
                                              deviceId,
                                              CL_PROGRAM_BUILD_LOG,
                                              buildLogSize,
                                              buildLog,
                                              &buildLogSize);
            CHECK_OPENCL_ERROR(status, "clGetProgramBuildInfo failed.");

            buildLog = (char*)malloc(buildLogSize);
            if(buildLog == NULL)
            {
                CHECK_ALLOCATION(buildLog, "Failed to allocate host memory. (buildLog)");
            }

            memset(buildLog, 0, buildLogSize);

            logStatus = clGetProgramBuildInfo(program,
                                              deviceId,
                                              CL_PROGRAM_BUILD_LOG,
                                              buildLogSize,
                                              buildLog,
                                              NULL);
            if(logStatus != CL_SUCCESS)
            {
                std::cout << "clGetProgramBuildInfo failed.";
                free(buildLog);
                return SDK_FAILURE;
            }

            std::cout << " \n\t\t\tBUILD LOG\n";
            std::cout << " ************************************************\n";
            std::cout << buildLog << std::endl;
            std::cout << " ************************************************\n";
            free(buildLog);
        }

        CHECK_OPENCL_ERROR(status, "clBuildProgram failed.");
    }

    return SDK_SUCCESS;
}

int
Device::createKernel()
{
    kernel = clCreateKernel(program, "multiDeviceKernel", &status);
    CHECK_OPENCL_ERROR(status, "clCreateKernel failed.");

    return SDK_SUCCESS;
}

int
Device::setKernelArgs(int run)
{
	unsigned int startVal = run * width; // width == 65536
	
	std::cout << startVal << "\n";
	
	status = clSetKernelArg(kernel, 0, sizeof(unsigned int), &startVal);
    CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.(run)");
	
    status = clSetKernelArg(kernel, 1, sizeof(cl_mem), &inputBuffer);
    CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.(inputBuffer)");
	
	status = clSetKernelArg(kernel, 2, sizeof(cl_mem), &outputBuffer);
		CHECK_OPENCL_ERROR(status, "clSetKernelArg failed.(outputBuffer)");
	
    return SDK_SUCCESS;
}

int
Device::enqueueKernel(size_t *globalThreads, size_t *localThreads)
{
    status = clEnqueueNDRangeKernel(queue,
                                    kernel,
                                    1,
                                    NULL,
                                    globalThreads,
                                    localThreads,
                                    0,
                                    NULL,
                                    &eventObject);
    CHECK_OPENCL_ERROR(status, "clEnqueueNDRangeKernel failed.");

    status = clFlush(queue);
    CHECK_OPENCL_ERROR(status, "clFlush failed.");

    return SDK_SUCCESS;
}

int
Device::waitForKernel()
{
    status = clFinish(queue);
    CHECK_OPENCL_ERROR(status, "clFinish failed.");

    return SDK_SUCCESS;
}

int
Device::getProfilingData()
{
    status = clGetEventProfilingInfo(eventObject,
                                     CL_PROFILING_COMMAND_START,
                                     sizeof(cl_ulong),
                                     &kernelStartTime,
                                     0);
    CHECK_OPENCL_ERROR(status, "clGetEventProfilingInfo failed.(start time)");

    status = clGetEventProfilingInfo(eventObject,
                                     CL_PROFILING_COMMAND_END,
                                     sizeof(cl_ulong),
                                     &kernelEndTime,
                                     0);
    CHECK_OPENCL_ERROR(status, "clGetEventProfilingInfo failed.(end time)");

    //Measure time in ms
    elapsedTime = 1e-6 * (kernelEndTime - kernelStartTime);

    return SDK_SUCCESS;
}

int
Device::enqueueReadData(unsigned int start, int gpu, int updates, Result* results)
{
    // Allocate memory
    if(output == NULL)
    {
        output = (cl_float*)malloc(width * sizeof(cl_float));
        CHECK_ALLOCATION(output, "Failed to allocate output buffer!\n");
    }
	
    status = clEnqueueReadBuffer(queue,
                                 outputBuffer,
                                 1,
                                 0,
                                 width * sizeof(cl_float),
                                 output,
                                 0, 0, 0);
    CHECK_OPENCL_ERROR(status, "clEnqueueReadBuffer failed.");
	
	
	int pos;
	unsigned int rule;
	
	std::cout << "reading result gpu : " << gpu << std::endl;
	std::cout << "update no          : " << updates << std::endl;
	std::cout << "first position     : " << ((start * 4) + gpu) * width << std::endl;
	
	for(int j = 0; j < width; j++)
	{
		rule = ((start * 4) + gpu) * width + j;
		
		pos = gpu * width + j;
		
		results[pos].autoID = rule;
		
		cl_float newscore = results[pos].score * updates;
		newscore = newscore + output[j];
		
		//if(1) std::cout << "newscore : " << results[pos].autoID << " score : " << output[j] << std::endl;
		
		newscore = newscore / (updates + 1);
		
		
		//if(1) std::cout << "automation 1 : " << results[pos].autoID << " score : " << newscore << std::endl;
		
		results[pos].score = newscore; // output[j]; 
		
		//if(1) std::cout << "automation  2: " << results[pos].autoID << " score : " << results[pos].score << std::endl;
	}
	
	/*
	float max = 0;
	int maxCount = 0;
	float goodScores[100];
	int goodRules[100];
	for(int i = 0; i < width; i++){
		if(output[i] > max * .7){
			
			if(output[i] > max) max = output[i];
			
			goodScores[maxCount % 100] = output[i];
			goodRules[maxCount % 100] = i;
			maxCount++;
			
		}
	}
	
	for(int i = 0; i < 10; i++){
		std::cout << "rule: " << goodRules[i] << "   score: " << goodScores[i] << std::endl;
	}
	
	*/
	
	//free(output);
	
	//std::cout << std::endl;
	
	
    return SDK_SUCCESS;
}

int
Device::verifyResults()
{
		
    float error = 0;
    //compare results between verificationOutput and output host buffers
    for(int i = 0; i < width; i++)
    {
        error += (output[i] - verificationOutput[i]);
    }
    error /= width;

    if(error < 0.001)
    {
        std::cout << "Passed!\n" << std::endl;
        verificationCount++;
    }
    else
    {
        std::cout << "Failed!\n" << std::endl;
        return SDK_FAILURE;
    }

    return SDK_SUCCESS;
}

int
Device::cleanupResources()
{
    int status = clReleaseCommandQueue(queue);
    CHECK_OPENCL_ERROR(status, "clReleaseCommandQueue failed.(queue)");

    status = clReleaseKernel(kernel);
    CHECK_OPENCL_ERROR(status, "clReleaseKernel failed.(kernel)");

    cl_uint programRefCount;
    status = clGetProgramInfo(program,
                              CL_PROGRAM_REFERENCE_COUNT,
                              sizeof(cl_uint),
                              &programRefCount,
                              0);
    CHECK_OPENCL_ERROR(status, "clGetProgramInfo failed.");

    if(programRefCount)
    {
        status = clReleaseProgram(program);
        CHECK_OPENCL_ERROR(status, "clReleaseProgram failed.");
    }

    cl_uint inputRefCount;
    status = clGetMemObjectInfo(inputBuffer,
                                CL_MEM_REFERENCE_COUNT,
                                sizeof(cl_uint),
                                &inputRefCount,
                                0);
    CHECK_OPENCL_ERROR(status, "clGetMemObjectInfo failed.");

    if(inputRefCount)
    {
        status = clReleaseMemObject(inputBuffer);
        CHECK_OPENCL_ERROR(status, "clReleaseMemObject failed. (inputBuffer)");
    }

    cl_uint outputRefCount;
    status = clGetMemObjectInfo(outputBuffer,
                                CL_MEM_REFERENCE_COUNT,
                                sizeof(cl_uint),
                                &outputRefCount,
                                0);
    CHECK_OPENCL_ERROR(status, "clGetMemObjectInfo failed.");

    if(outputRefCount)
    {
        status = clReleaseMemObject(outputBuffer);
        CHECK_OPENCL_ERROR(status, "clReleaseMemObject failed. (outputBuffer)");
    }

	/*
    status = clGetMemObjectInfo(outputBuffer1,
                                CL_MEM_REFERENCE_COUNT,
                                sizeof(cl_uint),
                                &outputRefCount,
                                0);
    CHECK_OPENCL_ERROR(status, "clGetMemObjectInfo failed.");

    if(outputRefCount)
    {
        status = clReleaseMemObject(outputBuffer1);
        CHECK_OPENCL_ERROR(status, "clReleaseMemObject failed. (outputBuffer)");
    }
	
    status = clGetMemObjectInfo(outputBuffer2,
                                CL_MEM_REFERENCE_COUNT,
                                sizeof(cl_uint),
                                &outputRefCount,
                                0);
    CHECK_OPENCL_ERROR(status, "clGetMemObjectInfo failed.");

    if(outputRefCount)
    {
        status = clReleaseMemObject(outputBuffer2);
        CHECK_OPENCL_ERROR(status, "clReleaseMemObject failed. (outputBuffer)");
    }
	
    status = clGetMemObjectInfo(outputBuffer3,
                                CL_MEM_REFERENCE_COUNT,
                                sizeof(cl_uint),
                                &outputRefCount,
                                0);
    CHECK_OPENCL_ERROR(status, "clGetMemObjectInfo failed.");

    if(outputRefCount)
    {
        status = clReleaseMemObject(outputBuffer3);
        CHECK_OPENCL_ERROR(status, "clReleaseMemObject failed. (outputBuffer)");
    }
	
	*/
	
    cl_uint contextRefCount;
    status = clGetContextInfo(context,
                              CL_CONTEXT_REFERENCE_COUNT,
                              sizeof(cl_uint),
                              &contextRefCount,
                              0);
    CHECK_OPENCL_ERROR(status, "clGetContextInfo failed.");

    if(contextRefCount)
    {
        status = clReleaseContext(context);
        CHECK_OPENCL_ERROR(status, "clReleaseContext failed.");
    }

    status = clReleaseEvent(eventObject);
    CHECK_OPENCL_ERROR(status, "clReleaseEvent failed.");

    return SDK_SUCCESS;
}



//Thread function for a device
void* threadFunc(void *device)
{
    Device *d = (Device*)device;

    size_t globalThreads = width;
    size_t localThreads = GROUP_SIZE;

    d->enqueueKernel(&globalThreads, &localThreads);
    d->waitForKernel();

    return NULL;
}

Device::~Device()
{
    FREE(output);
}


int runMultiGPU()
{
    int status;

    ///////////////////////////////////////////////////////////////////
    //  Case 1 : Single Context (Single Thread)
    //////////////////////////////////////////////////////////////////
    std::cout << sep << "\nMulti GPU Test 1 : Single context Single Thread\n" <<
              sep << std::endl;

    cl_context context = clCreateContextFromType(cprops,
                         CL_DEVICE_TYPE_GPU,
                         0,
                         0,
                         &status);
    CHECK_OPENCL_ERROR(status, "clCreateContext failed.");

	std::cout << "\nhello1\n"  << std::endl;
	
    size_t sourceSize = strlen(source);
    cl_program program  = clCreateProgramWithSource(context,
                          1,
                          &source,
                          (const size_t*)&sourceSize,
                          &status);
    CHECK_OPENCL_ERROR(status, "clCreateProgramWithSource failed.");

    char buildOptions[50];
    sprintf(buildOptions, "-D KERNEL_ITERATIONS=%d", KERNEL_ITERATIONS);

    //Build program for all the devices in the context
    status = clBuildProgram(program, 0, 0, buildOptions, 0, 0);
		
	std::cout << "\nhello2\n"  << std::endl;
	
    CHECK_OPENCL_ERROR(status, "clBuildProgram failed.");

    //Buffers
    cl_mem inputBuffer;
    cl_mem outputBuffer0;
	cl_mem outputBuffer1;
	cl_mem outputBuffer2;
	cl_mem outputBuffer3;
	
	//Output
	cl_float* output0 = (cl_float*)malloc(width*sizeof(cl_float));
	
	//Output
	cl_float* output1 = (cl_float*)malloc(width*sizeof(cl_float));
	
	//Output
	cl_float* output2 = (cl_float*)malloc(width*sizeof(cl_float));
	
	//Output
	cl_float* output3 = (cl_float*)malloc(width*sizeof(cl_float));
	
	if(262144 == width * 4)	std::cout << "Running 262144 automations in parallel!\n";
	
	
	Result results[262144];  //width * 4];
		
	// DB ALLOCATIONS
		
	int rc;
	MDB_env *env;
	MDB_dbi dbi;
	MDB_val key, data;
	MDB_txn *txn;
	MDB_cursor *cursor;
	
	rc = mdb_env_create(&env);
	rc = mdb_env_set_mapsize(env, (68719476736 * 2));
	rc = mdb_env_open(env, "./automationDB5", 0, 0664);
	
	
	// clear weird values from results
	
	for(int i = 0; i < width * 4; i++){
		
		results[i].autoID = 0;
		results[i].score = 0;
		
	}
	
	 std::cout << "\nhello3\n"  << std::endl;
	
	
	int seed = 60844398;
	
	for(int i = 0; i < numGPUDevices; i++){
		
		gpu[i].context = context;
		gpu[i].program = program;

		status = gpu[i].createQueue();
		CHECK_ERROR(status , SDK_SUCCESS, "Creating Commmand Queue(GPU) failed");

		status = gpu[i].createKernel();
		CHECK_ERROR(status , SDK_SUCCESS, "Creating Kernel (GPU) failed");
		
	}
	
	
	
	// 16384 runs of 4 cards
	
	for(int start = 8503; start < 16384; start++ ){   // which slice of 4x 65536 are we on
	
		std::cout << "Starting run " << start << std::endl; 
	
		for(int updates = 0; updates < 1; updates++){
	
		//Setup for all GPU devices
		for(int i = 0; i < numGPUDevices; i++)
		{


			// Create buffers
			// Create input buffer
		
			std::cout << "\tbefore create buffer : " << std::endl;
		
			inputBuffer = clCreateBuffer(context,
										CL_MEM_READ_ONLY,
										100 * sizeof(cl_uint),
										0,
										&status);
			CHECK_OPENCL_ERROR(status, "clCreateBuffer failed.(inputBuffer)");

		
			std::cout << "\tbefore enqueueWriteBufferCall : " << std::endl;
		
			//Set Buffers
			gpu[i].inputBuffer = inputBuffer;
			status = gpu[i].enqueueWriteBuffer(seed + updates);
			CHECK_ERROR(status , SDK_SUCCESS,
						"Submitting Write OpenCL Buffer (GPU) failed");
			switch(i){
				case 0:
					// Create output buffer
					outputBuffer0 = clCreateBuffer(context,
										CL_MEM_WRITE_ONLY,
										width * sizeof(cl_float),
										0,
										&status);
					CHECK_OPENCL_ERROR(status, "clCreateBuffer failed.(outputBuffer)");
					gpu[i].outputBuffer = outputBuffer0; 
				break;
				case 1:
					// Create output buffer
					outputBuffer1 = clCreateBuffer(context,
										CL_MEM_WRITE_ONLY,
										width * sizeof(cl_float),
										0,
										&status);
					CHECK_OPENCL_ERROR(status, "clCreateBuffer failed.(outputBuffer)");
					gpu[i].outputBuffer = outputBuffer1; 
				break;
				case 2:
					// Create output buffer
					outputBuffer2 = clCreateBuffer(context,
                                      CL_MEM_WRITE_ONLY,
                                      width * sizeof(cl_float),
                                      0,
                                      &status);
					CHECK_OPENCL_ERROR(status, "clCreateBuffer failed.(outputBuffer)");
					gpu[i].outputBuffer = outputBuffer2; 
				break;
				case 3:
					// Create output buffer
					outputBuffer3 = clCreateBuffer(context,
                                      CL_MEM_WRITE_ONLY,
                                      width * sizeof(cl_float),
                                      0,
                                      &status);
					CHECK_OPENCL_ERROR(status, "clCreateBuffer failed.(outputBuffer)");
					gpu[i].outputBuffer = outputBuffer3; 
				break;
			}
			//Set kernel arguments
			status = gpu[i].setKernelArgs(i + start * 4);
			CHECK_ERROR(status , SDK_SUCCESS, "Setting Kernel Args(GPU) failed");
		}

	
	
		std::cout << "\nhello5\n"  << std::endl;
	
		size_t globalThreads = width;
		size_t localThreads = GROUP_SIZE;

		//Start a host timer here
		int timer = sampleTimer.createTimer();
		sampleTimer.resetTimer(timer);
		sampleTimer.startTimer(timer);

		for(int i = 0; i < numGPUDevices; i++)
		{
			status = gpu[i].enqueueKernel(&globalThreads, &localThreads);
			CHECK_ERROR(status , SDK_SUCCESS, "Submitting Opencl Kernel (GPU) failed");
		}

		//Wait for all kernels to finish execution
		for(int i = 0; i < numGPUDevices; i++)
		{
			status = gpu[i].waitForKernel();
			CHECK_ERROR(status , SDK_SUCCESS, "Waiting for Kernel(GPU) failed");
		}

		//cl_float *C;
	
		//status = clEnqueueReadBuffer(gpu[0].queue, outputBuffer, CL_TRUE, 0, 256*sizeof(cl_float), output, 0, NULL, NULL);
		//CHECK_ERROR(status , SDK_SUCCESS, "reading output(GPU) failed");
	

		//Stop the host timer here
		sampleTimer.stopTimer(timer);

		//Measure total time
		double totalTime = sampleTimer.readTimer(timer);

		//Get individual timers
		for(int i = 0; i < numGPUDevices; i++)
		{
			status = gpu[i].getProfilingData();
			CHECK_ERROR(status , SDK_SUCCESS, "Getting Profiling Data (GPU) failed");
		}


		//Print total time and individual times
		std::cout << "Total time : " << totalTime * 1000 << std::endl;
		for(int i = 0; i < numGPUDevices; i++)
		{
			std::cout << "Time of GPU" << i << " : " << gpu[i].elapsedTime <<
                  std::endl;
		}

		if(1)
		{
			//Enqueue Read output buffer and verify results
			for(int i = 0; i < numGPUDevices; i++)
			{
				status = gpu[i].enqueueReadData(start, i, updates, results);
				CHECK_ERROR(status , SDK_SUCCESS, "Submitting Read buffer (GPU) failed");

				// Verify results
				std::cout << "Verifying results for GPU" << i << " : " << std::endl;
				gpu[i].verifyResults();
			}
		}
	
		//std::cout << "Auto Num, average: " << results[382680].autoID << "   Score: " << results[382680].score << std::endl;
	
		std::cout << "after enqueueRead" << std::endl;
	
		}  // end mulitple runs on one set of auto
	
	
	std::cout << "before db" << std::endl;
	
	// write results to db here
	char keyValue[16];
	char dataValue[16];
	
	std::cout << "db 1" << std::endl;
	
	rc = mdb_txn_begin(env, NULL, 0, &txn);
	rc = mdb_open(txn, NULL, 0, &dbi);
	
	std::cout << "db 2" << std::endl;
	
	key.mv_size = 8;
	key.mv_data = keyValue;
	data.mv_size = 16;
	data.mv_data = dataValue;
	
	unsigned int autoID;
	
	std::cout << "db 3" << std::endl;
	
	for(int i = 0; i < width * 4; i++){
	
		//std::cout << "1 writing to db #" << i << std::endl;
	
		sprintf(keyValue,  "%08x", results[i].autoID);
		sprintf(dataValue,  "%08.0f", results[i].score);
		
		//std::cout << "Key: " << keyValue << " Score: " << dataValue << std::endl;
	
		//std::cout << "2 writing to db #" << i << std::endl;
	
		rc = mdb_put(txn, dbi, &key, &data, 0);
	
	}
	rc = mdb_txn_commit(txn);
	
	if (rc) {
		fprintf(stderr, "mdb_txn_commit: (%d) %s\n", rc, mdb_strerror(rc));
		goto leave;
	}
	
	printf("MDB commit rc#: %d\n", rc);
	
	
	std::cout << "db 4" << std::endl;
	
	//mdb_txn_abort(txn);
	
	//
	
	//std::cout << "Auto Num: " << results[382680].autoID << "   Score: " << results[382680].score << std::endl;
	
	//std::cout << "before sort " << std::endl;
	
	//std::sort(results, results + width * 8, &resultComp);
	
	//std::cout << "after sort " << std::endl;
	
	//for(int i = width * 8; i > width * 8 - 100; i--){
		
	//	std::cout << "Auto Num: " << results[i-1].autoID << "   Score: " << results[i-1].score << std::endl;
		
	//}
	
	
	    //Release memory buffers
    status = clReleaseMemObject(inputBuffer);
    CHECK_OPENCL_ERROR(status, "clReleaseMemObject failed. (inputBuffer)");

    status = clReleaseMemObject(outputBuffer0);
    CHECK_OPENCL_ERROR(status,"clReleaseMemObject failed. (outputBuffer)");
	status = clReleaseMemObject(outputBuffer1);
    CHECK_OPENCL_ERROR(status,"clReleaseMemObject failed. (outputBuffer)");
	status = clReleaseMemObject(outputBuffer2);
    CHECK_OPENCL_ERROR(status,"clReleaseMemObject failed. (outputBuffer)");
	status = clReleaseMemObject(outputBuffer3);
    CHECK_OPENCL_ERROR(status,"clReleaseMemObject failed. (outputBuffer)");
	
	
	
	}  // end run on all autos
	
	
leave:
	mdb_close(env, dbi);
	mdb_env_close(env);
	return 0;
	
	

    //Release the resources on all devices
    //Release context
    status = clReleaseContext(context);
    CHECK_OPENCL_ERROR(status, "clCreateContext failed.");


    //Release Program object
    status = clReleaseProgram(program);
    CHECK_OPENCL_ERROR(status, "clReleaseProgram failed.");

    //Release Kernel object, command-queue, event object
    for(int i = 0; i < numGPUDevices; i++)
    {
        status = clReleaseKernel(gpu[i].kernel);
        CHECK_OPENCL_ERROR(status, "clReleaseCommandQueue failed.");

        status = clReleaseCommandQueue(gpu[i].queue);
        CHECK_OPENCL_ERROR(status, "clReleaseCommandQueue failed.");

        status = clReleaseEvent(gpu[i].eventObject);
        CHECK_OPENCL_ERROR(status, "clReleaseEvent failed.");
    }


    return SDK_SUCCESS;
}

/*
 * \brief Host Initialization
 *        Allocate and initialize memory
 *        on the host. Print input array.
 */
int
initializeHost(void)
{
    width = NUM_THREADS;
    input = NULL;
    verificationOutput = NULL;

    /////////////////////////////////////////////////////////////////
    // Allocate and initialize memory used by host
    /////////////////////////////////////////////////////////////////
    cl_uint sizeInBytesIn = 100 * sizeof(cl_uint);
	cl_uint sizeInBytesOut = width * sizeof(cl_uint);
    input = (cl_uint*) malloc(sizeInBytesIn);
    CHECK_ALLOCATION(input, "Error: Failed to allocate input memory on host\n");

    verificationOutput = (cl_float*) malloc(sizeInBytesOut);
    CHECK_ALLOCATION(verificationOutput,
                     "Error: Failed to allocate verificationOutput memory on host\n");

	//std::cout << "initing host wtf where is this happening ~~~~~~~~~~~~~~~~~~~~~~~~~~~`\n";
					 
    //Initilize input data
	
	//std::cout <<("input:");
    //for(int i = 0; i < 100; i++)
    //{
    //    input[i] = (cl_uint) (rand() % 2);
		
		
	//	std::cout << input[i];
    //}
	
	//for(int j = 0; j < 10; j++)
	//{
	//	std::cout << "\tinput = " << input[j]; 
	
	//}
	
	std::cout << std::endl;
	
    return SDK_SUCCESS;
}

/*
 * Converts the contents of a file into a string
 */
std::string
convertToString(const char *filename)
{
    size_t size;
    char*  str;
    std::string s;

    std::fstream f(filename, (std::fstream::in | std::fstream::binary));

    if(f.is_open())
    {
        size_t fileSize;
        f.seekg(0, std::fstream::end);
        size = fileSize = (size_t)f.tellg();
        f.seekg(0, std::fstream::beg);

        str = new char[size+1];
        if(!str)
        {
            f.close();
            return NULL;
        }

        f.read(str, fileSize);
        f.close();
        str[size] = '\0';

        s = str;
        delete[] str;
        return s;
    }
    return NULL;
}


/*
 * \brief OpenCL related initialization
 *        Create Context, Device list, Command Queue
 *        Create OpenCL memory buffer objects
 *        Load CL file, compile, link CL source
 *  Build program and kernel objects
 */
int
initializeCL(void)
{
	
	std::cout << "\tInit CL : " << std::endl;
	
    cl_int status = 0;
    /*
     * Have a look at the available platforms and pick either
     * the AMD one if available or a reasonable default.
     */

    cl_uint numPlatforms;
    status = clGetPlatformIDs(0, NULL, &numPlatforms);
    CHECK_OPENCL_ERROR(status, "clGetPlatformIDs failed.");

    if(numPlatforms > 0)
    {
		
		std::cout << "\tnum platforms : " << numPlatforms << std::endl;
		
        cl_platform_id* platforms = (cl_platform_id *)malloc(numPlatforms*sizeof(
                                        cl_platform_id));
        status = clGetPlatformIDs(numPlatforms, platforms, NULL);
        CHECK_OPENCL_ERROR(status, "clGetPlatformIDs failed.");

        for(unsigned int i=0; i < numPlatforms; ++i)
        {
			std::cout << "\tInit Platform : "  << i << std::endl;
			
            char pbuff[100];
            status = clGetPlatformInfo(
                         platforms[i],
                         CL_PLATFORM_VENDOR,
                         sizeof(pbuff),
                         pbuff,
                         NULL);
            CHECK_OPENCL_ERROR(status, "clGetPlatformInfo failed.");

            platform = platforms[i];
            if(!strcmp(pbuff, "Advanced Micro Devices, Inc."))
            {
                break;
            }
        }
        FREE(platforms);
    }

    /*
     * If we could find our platform, use it. Otherwise pass a NULL and get whatever the
     * implementation thinks we should be using.
     */
    cps[0] = CL_CONTEXT_PLATFORM;
    cps[1] = (cl_context_properties)platform;
    cps[2] = 0;

    cprops = (NULL == platform) ? NULL : cps;

    // Get Number of CPU devices available
    status = clGetDeviceIDs(platform,
                            CL_DEVICE_TYPE_CPU,
                            0,
                            0,
                            (cl_uint*)&numCPUDevices);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs failed.(numCPUDevices)");

    // Get Number of CPU devices available
    status = clGetDeviceIDs(platform,
                            CL_DEVICE_TYPE_ALL,
                            0,
                            0,
                            (cl_uint*)&numDevices);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs failed.(numDevices)");

    // Get number of GPU Devices
    numGPUDevices = numDevices - numCPUDevices;

    // If no GPU is present then exit
    if(numGPUDevices < 1)
    {
        OPENCL_EXPECTED_ERROR("Only CPU device is present. Exiting!");
    }

    // Allocate memory for list of Devices
    cpu = new Device[numCPUDevices];
    //Get CPU Device IDs
    cl_device_id* cpuDeviceIDs = new cl_device_id[numCPUDevices];
    status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_CPU, numCPUDevices,
                            cpuDeviceIDs, 0);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs failed.");

    for(int i = 0; i < numCPUDevices; i++)
    {
        cpu[i].dType = CL_DEVICE_TYPE_CPU;
        cpu[i].deviceId = cpuDeviceIDs[i];
    }

    delete[] cpuDeviceIDs;

    gpu = new Device[numGPUDevices];
    //Get GPU Device IDs
    cl_device_id* gpuDeviceIDs = new cl_device_id[numGPUDevices];
    status = clGetDeviceIDs(platform, CL_DEVICE_TYPE_GPU, numGPUDevices,
                            gpuDeviceIDs, 0);
    CHECK_OPENCL_ERROR(status, "clGetDeviceIDs failed.");

    for(int i = 0; i < numGPUDevices; i++)
    {
        gpu[i].dType = CL_DEVICE_TYPE_GPU;
        gpu[i].deviceId = gpuDeviceIDs[i];
    }

    delete[] gpuDeviceIDs;

    /////////////////////////////////////////////////////////////////
    // Load CL file
    /////////////////////////////////////////////////////////////////
    const char *filename  = "SimpleMultiDevice_Kernels.cl";
    sourceStr = convertToString(filename);
    source = sourceStr.c_str();

    return SDK_SUCCESS;
}

int
run()
{
    int status;

    // If a GPU is present then run CPU + GPU concurrently
    //if(numGPUDevices > 0 && numCPUDevices > 0)
    //{
        /* 3 tests :
         a) Single context - Single thread
         b) Multiple context - Single thread
         c) Multiple context - Multple Threads*/

        // 3 Tests * 2 devices
     //   requiredCount += 3 * 2;
     //   status = runMultiDevice();
     //   CHECK_ERROR(status , SDK_SUCCESS,
     //               "Running OpenCL Kernel in MultiDevice Failed");
    //}

    // If more than 1 GPU is present then run MultiGPU concurrently
    if(numGPUDevices > 1)
    {
        /* 3 tests :
         a) Single context - Single thread
         b) Multiple context - Single thread
         c) Multiple context - Multple Threads*/

        // 3 Tests * numGPUDevices
        requiredCount += 3 * numGPUDevices;
        status = runMultiGPU();
        CHECK_ERROR(status , SDK_SUCCESS, "Running OpenCL Kernel in MultiGPU Failed");
    }
    return SDK_SUCCESS;

}


/*
 * \brief Releases program's resources
 */
void
cleanupHost(void)
{
    if(input != NULL)
    {
        free(input);
        input = NULL;
    }
    if(verificationOutput != NULL)
    {
        free(verificationOutput);
        verificationOutput = NULL;
    }
    if(cpu != NULL)
    {
        delete[] cpu;
        cpu = NULL;
    }
    if(gpu != NULL)
    {
        delete[] gpu;
        gpu = NULL;
    }
}


/*
 * \brief Print no more than 256 elements of the given array.
 *
 *        Print Array name followed by elements.
 */
void print1DArray(
    const std::string arrayName,
    cl_float* arrayData,
    int length)
{
    cl_uint i;
    cl_uint numElementsToPrint = (256 < length) ? 256 : length;

    std::cout << std::endl;
    std::cout << arrayName << ":" << std::endl;
    for(i = 0; i < numElementsToPrint; ++i)
    {
        std::cout << arrayData[i] << " ";
    }
    std::cout << std::endl;

}

// OpenCL MAD definition for CPU
float mad(float a, float b, float c)
{
    return a * b + c;
}

// OpenCL HYPOT definition for CPU
#if (_MSC_VER != 1800)
float hypot(float a, float b)
{
    return sqrt(a * a + b * b);
}
#endif


int
CPUKernel()
{
    for(int i = 0; i < width; i++)
    {
        float a = mad(input[i], input[i], 1);
        float b = mad(input[i], input[i], 2);

        for(int j = 0; j < KERNEL_ITERATIONS; j++)
        {
            a = hypot(a, b);
            b = hypot(a, b);
        }
        verificationOutput[i] = (a + b);
    }
    return SDK_SUCCESS;
}

int 
main(int argc, char * argv[])
{
    for(int i = 1; i < argc; i++)
    {
        if(!strcmp(argv[i], "-e") || !strcmp(argv[i], "--verify"))
        {
            verify = true;
        }
        if(!strcmp(argv[i], "-h") || !strcmp(argv[i], "--help"))
        {
            printf("Usage\n");
            printf("-h, --help\tPrint this help.\n");
            printf("-e, --verify\tVerify results against reference implementation.\n");
            exit(0);
        }
		if(!strcmp(argv[i], "-v") || !strcmp(argv[i], "--version"))
        {
            std::cout << "SDK version : " << SAMPLE_VERSION << std::endl;
            exit(0);
        }
    }

    int status;

    // Initialize Host application
    if (initializeHost() != SDK_SUCCESS)
    {
        return SDK_FAILURE;
    }

    // Run host computation if verification is true
    if (CPUKernel() != SDK_SUCCESS)
    {
        return SDK_FAILURE;
    }

    // Initialize OpenCL resources
    status = initializeCL();
    if(status != SDK_SUCCESS)
    {
        if(status == SDK_EXPECTED_FAILURE)
        {
            return SDK_SUCCESS;
        }

        return SDK_FAILURE;
    }

    // Run the CL program
    if (run() != SDK_SUCCESS)
    {
        return SDK_FAILURE;
    }

    // Release host resources
    cleanupHost();

    if(verify)
    {
        // If any one test fails then print FAILED
        if(verificationCount != requiredCount)
        {
            std::cout << "\n\nFAILED!\n" << std::endl;
            return SDK_FAILURE;
        }
        else
        {
            std::cout << "\n\nPASSED!\n" << std::endl;
            return SDK_SUCCESS;
        }
    }

    return SDK_SUCCESS;
}
