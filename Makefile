TARGETS = server client server-api
CPPFLAGS = -g -Wall -Werror -pthread

all: $(TARGETS)

server: server.cpp
	$(CXX) $(CPPFLAGS) $^ -o $@

client: client.cpp
	$(CXX) $(CPPFLAGS) $^ -o $@
	
server-api: server-api.cpp
	$(CXX) $(CPPFLAGS) $^ -o $@

.PHONY : clean
clean::
	rm -fv $(TARGETS) *~ *.o