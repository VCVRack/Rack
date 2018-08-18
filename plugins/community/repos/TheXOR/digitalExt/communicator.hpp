#pragma once
#ifdef LAUNCHPAD
#include <memory.h>
#include <windows.h>
#include "driver_include.hpp"

#pragma pack(push,1)
struct __attribute__((aligned(1), packed)) _launchpadMessage
{
	uint8_t status;
	uint8_t cmd;
	uint8_t key;
	uint8_t currentScene;
	uint8_t lpNumber;
	uint8_t shiftDown;
	short param0;
	short param1;
	
	#pragma GCC diagnostic ignored "-Wmaybe-uninitialized"
	void FromMessage(LaunchpadMessage *src)
	{
		status = (uint8_t)src->status;
		cmd = (uint8_t)src->cmd;
		key = (uint8_t)src->key;
		currentScene = (uint8_t)src->currentScene;
		lpNumber = (uint8_t)src->lpNumber;
		shiftDown = (uint8_t)(src->shiftDown ? 1 : 0);
		param0 = src->param0;
		param1 = src->param1;
	}

	LaunchpadMessage Reconstruct()
	{
		LaunchpadMessage rv;
		rv.status = (LaunchpadKeyStatus)status;
		rv.cmd = (LaunchpadCommand)cmd;
		rv.key = (LaunchpadKey)key;
		rv.currentScene = (LaunchpadScene)currentScene;
		rv.lpNumber = lpNumber;
		rv.shiftDown = shiftDown != 0;
		rv.param0 = param0;
		rv.param1 = param1;

		return rv;
	}
};
#pragma pack(pop)

class communicator
{
private:
	class circBuffer
	{
	private:
		uint8_t * mapAccess;
		int rdPtr;
		int WR_PTR_POS;
		int P0;
		int PN;

	public:
		static int bufferOverhead() { return sizeof(uint32_t); }        // n. di bytes EXTRA richiesti per la gestione del buffer circolare

		void Clear()
		{
			rdPtr = wrPtr_get();
		}

		circBuffer(int bufferSize, void *acc, int offset)
		{
			WR_PTR_POS = offset;
			P0 = WR_PTR_POS + bufferOverhead();   // posizione 0 memoria effettiva (dove salvare e leggere i dati, dopo i primi 8 bytes riservati ai puntatori rd/wr)
			PN = P0 + bufferSize - bufferOverhead();
			mapAccess = (uint8_t*)acc;
			Clear();
		}

		~circBuffer()
		{
			mapAccess = NULL;
		}

		void WriteChunk(LaunchpadMessage *_msg)
		{
#ifdef DEBUG
			info("send message %i; tx size is %i", _msg->cmd, sizeof(_launchpadMessage));
#endif
			_launchpadMessage msg;
			msg.FromMessage(_msg);

			uint8_t *ptr = (uint8_t *)&msg;
			int cur_wrPtr = wrPtr_get();
			for(int k = 0; k < (int)sizeof(_launchpadMessage); k++)
				cur_wrPtr = Put(*ptr++, cur_wrPtr);
			wrPtr_set(cur_wrPtr);	// finalizza IN SOLIDO il chunk appena letto
		}

		LaunchpadMessage ReadChunk()
		{
			if(data_available())
			{
				_launchpadMessage msg;
				uint8_t *p = (uint8_t *)&msg;
				for(int k = 0; k < (int)sizeof(_launchpadMessage); k++)
					*p++ = Get();

				return msg.Reconstruct();
			}

			LaunchpadMessage rv;
			rv.status = LaunchpadKeyStatus::keyNone;
			return rv;
		}

	private:
		int wrPtr_get()
		{
			return *((uint32_t *)(mapAccess + WR_PTR_POS));
		}
		void wrPtr_set(uint32_t v)
		{
			uint32_t *ptr = (uint32_t *)(mapAccess + WR_PTR_POS);
			*ptr = v;
		}
		bool data_available() { return rdPtr != wrPtr_get(); }

		int incPtr(int ptr)
		{
			if(++ptr >= PN)
				ptr = P0;

			return ptr;
		}

		int Put(uint8_t b, int ptr)
		{
			mapAccess[ptr] = b;
			return incPtr(ptr);
		}

		uint8_t Get()
		{
			int trdPtr = rdPtr;
			uint8_t rv = mapAccess[trdPtr];
			rdPtr = incPtr(trdPtr);
			return rv;
		}
	};

public:
	communicator()
	{
		hMapFile = NULL;
		syncMutex = NULL;
		pmemory = NULL;
		pCommonMemory = NULL;
		rdBuffer = wrBuffer = NULL;
	}

	~communicator()
	{
		deinit();
	}

	void clear()
	{
		if(pmemory != NULL)
		{
			rdBuffer->Clear();
			wrBuffer->Clear();
		}
	}

	bool Connected() { return pmemory != NULL; }

	bool Open()
	{
		if(pmemory == NULL)
		{
			try
			{
				init();
			} catch(_exception ex)
			{
				deinit();
			}
		}

		return pmemory != NULL;
	}

	void Write(LaunchpadMessage msg)
	{
		if(checkServer())
		{
			if(WaitForSingleObject(syncMutex, 500) == WAIT_OBJECT_0)
			{
				wrBuffer->WriteChunk(&msg);
				ReleaseMutex(syncMutex);
			}
		}
	}

	LaunchpadMessage Read()
	{
		if(checkServer())
			return rdBuffer->ReadChunk();

		LaunchpadMessage rv;
		rv.status = LaunchpadKeyStatus::keyNone;
		return rv;
	}

private:
	void *pmemory;
	uint32_t *pCommonMemory;
	circBuffer *rdBuffer;
	circBuffer *wrBuffer;
	HANDLE hMapFile;
	HANDLE syncMutex;

	void init()
	{
		const int BUFFER_SIZE = 100;
		const int SERVERUP_POS = 0;      // (Int32) : indica se il server e' up o down
		const int START_OF_BUFFER = SERVERUP_POS + sizeof(uint32_t);    // prima locazione di memoria utile per il buffer circolare

		int b_l = BUFFER_SIZE * sizeof(_launchpadMessage) + circBuffer::bufferOverhead();
#ifdef DEBUG
		info("Open file mapping");
#endif
		hMapFile = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE, TRUE, "launchpad_mem");
		if(hMapFile != NULL)
		{
#ifdef DEBUG
			info("file mapping opened");
#endif
			void *p = MapViewOfFile(hMapFile, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, 2 * b_l + sizeof(uint32_t)/*flag SERVER_POS*/);
#ifdef DEBUG
			info("MapViewOfFile success");
#endif
			syncMutex = OpenMutex(MUTEX_ALL_ACCESS, FALSE, "launchpad_mtx");
#ifdef DEBUG
			if(syncMutex != NULL) info("mutex OK"); else info("*** no mutexxxxxxx!!!!!! ****");
#endif


			wrBuffer = new circBuffer(b_l, p, START_OF_BUFFER);
			rdBuffer = new circBuffer(b_l, p, START_OF_BUFFER + b_l);
#ifdef DEBUG
			info("circBuffer created");
#endif

			pmemory = p;
			pCommonMemory = (uint32_t *)p;
#ifdef DEBUG
			info("Opened OK");
#endif
		}
	}

	bool checkServer()
	{
		bool rv = false;
		if(pCommonMemory != NULL)
		{
			uint32_t server = *pCommonMemory;
			if(server == 0)
			{
#ifdef DEBUG
				info("*** connection lost ***");
#endif
				deinit();
			} else
				rv = true;
		}

		return rv;
	}

	void deinit()
	{
		void *p = pmemory;
		pmemory = NULL;
		pCommonMemory = NULL;

		if(p != NULL)
			UnmapViewOfFile(p);

		if(hMapFile != NULL)
			CloseHandle(hMapFile);
		hMapFile = NULL;

		if(rdBuffer != NULL)
		{
			delete rdBuffer;
			rdBuffer = NULL;
		}
		if(wrBuffer != NULL)
		{
			delete wrBuffer;
			wrBuffer = NULL;
		}
		if(syncMutex != NULL)
		{
			CloseHandle(syncMutex);
			syncMutex = NULL;
		}
	}
};
#endif
