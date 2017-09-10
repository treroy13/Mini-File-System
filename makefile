#Kristjana Popovski, Treasa Roy, Atsuko Shimizu, Marchrista Jones

all:	ssfs_mkdsk ssfs

ssfs_mkdsk: ssfs_mkdsk.o
	g++ ssfs_mkdsk.o -o ssfs_mkdsk

ssfs_mkdsk.o: ssfs_mkdsk.cpp
	g++ -c ssfs_mkdsk.cpp -std=c++11

ssfs: ssfs.o
	g++ ssfs.o -o ssfs

ssfs.o: ssfs.cpp
	g++ -c ssfs.cpp -std=c++11

clean:
	rm -f *.o ssfs_mkdsk ssfs
