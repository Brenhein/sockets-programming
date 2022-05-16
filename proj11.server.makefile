project = proj11.server

.PHONY: all
all: 
	g++ -o $(project) proj11.server.c

.PHONY: clean
clean:
	rm -f $(project)
	rm -f *.o

