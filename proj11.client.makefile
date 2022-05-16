project = proj11.client

.PHONY: all
all: 
	g++ -o $(project) proj11.client.c

.PHONY: clean
clean:
	rm -f $(project)
	rm -f *.o

