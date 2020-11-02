#include <stdint.h>
#include <unistd.h>
#include <fcntl.h>

void read_file(const char *path, void *data, size_t size) {
	int fd = open( path, O_RDONLY );
	int nr = read( fd, data, size );
	close( fd );//TODO: error checking
}

void write_file(const char *path, const void *data, size_t size) {
	int fd = open( path, O_WRONLY | O_CREAT );
	int nr = write( fd, data, size );
	close( fd );//TODO: error checking
}
