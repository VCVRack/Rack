#pragma once
#ifdef OSC_ENABLE
#define NUM_SCENES  (8)
#include <map>
#include <memory.h>
#include <windows.h>

struct OSCParam
{
	char paramType;
	int paramLen;
	union
	{
		float f;
		double d;
		unsigned char midi[4];
		int32_t i32;
		int64_t i64;
		uint64_t time;
		const char *s;
	};
};

#pragma pack(push,1)
struct __attribute__((aligned(1), packed)) OSCMsg
{
public:
	int32_t scene;
	char address[40];
	float value;
	void set(int scn, const char *addr, float v) 
	{ 
		memset(address, 0, sizeof(address)); 
		strcpy(address, addr);
		scene = scn;
		value = v;
	}
};
#pragma pack(pop)

#include "oscCircBuff.hpp"

class oscCommunicator
{
public:
	oscCommunicator(int scene_num)
	{
		hMapFile = NULL;
		pmemory = NULL;
		pCommonMemory = NULL;
		rdBuffer = wrBuffer = NULL;
		m_myScene = scene_num;
	}

	~oscCommunicator()
	{
		deinit();
	}

	void Clear()
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

	void Write(OSCMsg *msg)
	{
		if(checkServer())
		{
			wrBuffer->WriteChunk(msg);
		}
	}

	bool Read(OSCMsg *msg)
	{
		if(checkServer())
			return rdBuffer->ReadChunk(msg);

		return false;
	}

private:
	void *pmemory;
	uint32_t *pCommonMemory;
	oscCircBuffer *rdBuffer;
	oscCircBuffer *wrBuffer;
	HANDLE hMapFile;
	int m_myScene;

	void init()
	{
		int b_l = OSCBUFFER_SIZE * sizeof(OSCMsg) + oscCircBuffer::bufferOverhead();
		int rawsize = NUM_SCENES * 2 * b_l + sizeof(uint32_t) /*flag SERVER_POS*/;
		hMapFile = OpenFileMapping(FILE_MAP_READ | FILE_MAP_WRITE, TRUE, "OSC_mem");
		if(hMapFile != NULL)
		{
#ifdef DEBUG
			info("file mapping opened, buff size = %i", b_l);
#endif
			void *p = MapViewOfFile(hMapFile, FILE_MAP_READ | FILE_MAP_WRITE, 0, 0, rawsize);
#ifdef DEBUG
			if(p == NULL)
				info("MapViewOfFile failed");
			else
				info("MapViewOfFile success, my scene is %i", m_myScene);
#endif
			if(p != NULL)
			{
				int offset = b_l * 2 * (m_myScene - 1);
#ifdef DEBUG
				info("wrBuffer[%i] @ offset = %i", m_myScene - 1, START_OF_BUFFER + offset);
				info("rdBuffer[%i] @ offset = %i", m_myScene - 1, START_OF_BUFFER + b_l + offset);
#endif
				wrBuffer = new oscCircBuffer(b_l, p, START_OF_BUFFER + offset);
				rdBuffer = new oscCircBuffer(b_l, p, START_OF_BUFFER + b_l + offset);
			}
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

	}
};
#endif //OSC
