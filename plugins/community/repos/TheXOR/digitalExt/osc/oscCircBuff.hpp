#pragma once
#ifdef OSC_ENABLE
#define OSCBUFFER_SIZE	 (256)
#define SERVERUP_POS  (0)      // (Int32) : indica se il server e' up o down
#define START_OF_BUFFER  (SERVERUP_POS + sizeof(uint32_t))    // prima locazione di memoria utile per il buffer circolare

class oscCircBuffer
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

	oscCircBuffer(int bufferSize, void *acc, int offset)
	{
		WR_PTR_POS = offset;
		P0 = WR_PTR_POS + bufferOverhead();   // posizione 0 memoria effettiva (dove salvare e leggere i dati, dopo i primi 8 bytes riservati ai puntatori rd/wr)
		PN = P0 + bufferSize - bufferOverhead();
		mapAccess = (uint8_t *)acc;
		Clear();
	}

	~oscCircBuffer()
	{
		mapAccess = NULL;
	}

	void WriteChunk(OSCMsg *msg)
	{
		uint8_t *ptr = (uint8_t *)msg;
		int cur_wrPtr = wrPtr_get();
		for(int k = 0; k < (int)sizeof(OSCMsg); k++)
			cur_wrPtr = Put(*ptr++, cur_wrPtr);
		wrPtr_set(cur_wrPtr);	// finalizza IN SOLIDO il chunk appena letto
	}

	bool ReadChunk(OSCMsg *msg)
	{
		if(data_available())
		{
			uint8_t *p = (uint8_t *)msg;
			for(int k = 0; k < (int)sizeof(OSCMsg); k++)
				*p++ = Get();

			return true;
		}

		return false;
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
#endif
