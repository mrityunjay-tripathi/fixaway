INCS := -I/usr/local/include -I./include
LDFLAGS := -L/usr/local/lib -L./lib
LIBS := -lvrb -lssl -lcrypto
CXXFLAGS := -std=c++2a
INSTALL_DIR=/usr/local

debug := 0
ifeq (${debug}, 1)
	CXXFLAGS += -g -O0
else
	CXXFLAGS += -O3
endif

.PHONY: test
test:
	g++ ${CXXFLAGS} ${INCS} test/example.cpp -o bin/example ${LDFLAGS} ${LIBS}
	g++ ${CXXFLAGS} ${INCS} test/fixtest.cpp -o bin/fixtest ${LDFLAGS} ${LIBS}

benchmark:
	g++ ${CXXFLAGS} ${INCS} benchmark/fixbenchmark.cpp -o bin/fixbenchmark -lbenchmark -lpthread ${LDFLAGS} ${LIBS}

clean:
	rm -rf bin/*

install:
	rm -rf ${INSTALL_DIR}/include/fixaway
	mkdir -p ${INSTALL_DIR}/include/fixaway
	cp -rf include/* ${INSTALL_DIR}/include/
