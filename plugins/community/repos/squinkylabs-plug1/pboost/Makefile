
LDFLAGS += -static -lws2_32 -lPsapi

pboost.exe : build/pboost.obj
	$(CXX) -o $@ $^ $(LDFLAGS)

build/pboost.obj : pboost.cpp
	mkdir build -p
	$(CC) $(CFLAGS) -c -o $@ $<
