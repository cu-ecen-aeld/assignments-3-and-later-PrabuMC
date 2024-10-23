#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <syslog.h>
int stlen(char* str) {
	int len = 0;
	if (str == NULL)
		return 0;
	for (len = 0; str[len] != '\0'; len++);
	return (len+1);
}
int main(int argc, char* argv[]) {
        fprintf(stderr, "args %d", argc);
	if (argc < 3) {
		fprintf(stderr, "Less Args, exit 1 %d", argc);
		syslog(LOG_ERR, "Less Arguments, exit 1");
		return 1;
	}
	int file_fd = open(argv[1], O_CREAT|O_RDWR, S_IRWXU);
	syslog(LOG_INFO, "Writing %s to %s", argv[1], argv[2]);
	int res = write(file_fd, argv[2], stlen(argv[2]));
	if (res < 0)
		syslog(LOG_ERR, "Write failed");
	return 0;
}
