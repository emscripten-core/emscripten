/*
Bullet Continuous Collision Detection and Physics Library
Copyright (c) 2003-2007 Erwin Coumans  http://bulletphysics.com

This software is provided 'as-is', without any express or implied warranty.
In no event will the authors be held liable for any damages arising from the use of this software.
Permission is granted to anyone to use this software for any purpose, 
including commercial applications, and to alter it and redistribute it freely, 
subject to the following restrictions:

1. The origin of this software must not be misrepresented; you must not claim that you wrote the original software. If you use this software in a product, an acknowledgment in the product documentation would be appreciated but is not required.
2. Altered source versions must be plainly marked as such, and must not be misrepresented as being the original software.
3. This notice may not be removed or altered from any source distribution.
*/


#ifndef BT_SPU_LIBSPE2_SUPPORT_H
#define BT_SPU_LIBSPE2_SUPPORT_H

#include <LinearMath/btScalar.h> //for uint32_t etc.

#ifdef USE_LIBSPE2

#include <stdlib.h>
#include <stdio.h>
//#include "SpuNarrowPhaseCollisionTask/SpuGatheringCollisionTask.h"
#include "PlatformDefinitions.h"


//extern struct SpuGatherAndProcessPairsTaskDesc;

enum
{
	Spu_Mailbox_Event_Nothing = 0,
	Spu_Mailbox_Event_Task = 1,
	Spu_Mailbox_Event_Shutdown = 2,
	
	Spu_Mailbox_Event_ForceDword = 0xFFFFFFFF
	
};

enum
{
	Spu_Status_Free = 0,
	Spu_Status_Occupied = 1,
	Spu_Status_Startup = 2,
	
	Spu_Status_ForceDword = 0xFFFFFFFF
	
};


struct btSpuStatus
{
	uint32_t	m_taskId;
	uint32_t	m_commandId;
	uint32_t	m_status;

	addr64 m_taskDesc;
	addr64 m_lsMemory;
	
}
__attribute__ ((aligned (128)))
;



#ifndef __SPU__

#include "LinearMath/btAlignedObjectArray.h"
#include "SpuCollisionTaskProcess.h"
#include "SpuSampleTaskProcess.h"
#include "btThreadSupportInterface.h"
#include <libspe2.h>
#include <pthread.h>
#include <sched.h>

#define MAX_SPUS 4 

typedef struct ppu_pthread_data 
{
	spe_context_ptr_t context;
	pthread_t pthread;
	unsigned int entry;
	unsigned int flags;
	addr64 argp;
	addr64 envp;
	spe_stop_info_t stopinfo;
} ppu_pthread_data_t;


static void *ppu_pthread_function(void *arg)
{
    ppu_pthread_data_t * datap = (ppu_pthread_data_t *)arg;
    /*
    int rc;
    do 
    {*/
        spe_context_run(datap->context, &datap->entry, datap->flags, datap->argp.p, datap->envp.p, &datap->stopinfo);
        if (datap->stopinfo.stop_reason == SPE_EXIT) 
        {
           if (datap->stopinfo.result.spe_exit_code != 0) 
           {
             perror("FAILED: SPE returned a non-zero exit status: \n");
             exit(1);
           }
         } 
        else 
         {
           perror("FAILED: SPE abnormally terminated\n");
           exit(1);
         }
        
        
    //} while (rc > 0); // loop until exit or error, and while any stop & signal
    pthread_exit(NULL);
}






///SpuLibspe2Support helps to initialize/shutdown libspe2, start/stop SPU tasks and communication
class SpuLibspe2Support : public btThreadSupportInterface
{

	btAlignedObjectArray<btSpuStatus>	m_activeSpuStatus;
	
public:
	//Setup and initialize SPU/CELL/Libspe2
	SpuLibspe2Support(spe_program_handle_t *speprog,int numThreads);
	
	// SPE program handle ptr.
	spe_program_handle_t *program;
	
	// SPE program data
	ppu_pthread_data_t data[MAX_SPUS];
	
	//cleanup/shutdown Libspe2
	~SpuLibspe2Support();

	///send messages to SPUs
	void sendRequest(uint32_t uiCommand, uint32_t uiArgument0, uint32_t uiArgument1=0);

	//check for messages from SPUs
	void waitForResponse(unsigned int *puiArgument0, unsigned int *puiArgument1);

	//start the spus (can be called at the beginning of each frame, to make sure that the right SPU program is loaded)
	virtual void startSPU();

	//tell the task scheduler we are done with the SPU tasks
	virtual void stopSPU();

	virtual void setNumTasks(int numTasks)
	{
		//changing the number of tasks after initialization is not implemented (yet)
	}

private:
	
	///start the spus (can be called at the beginning of each frame, to make sure that the right SPU program is loaded)
	void internal_startSPU();


	
	
	int numThreads;

};

#endif // NOT __SPU__

#endif //USE_LIBSPE2

#endif //BT_SPU_LIBSPE2_SUPPORT_H




