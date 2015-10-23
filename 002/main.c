#include <stdlib.h>
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv, char** env)
{
	char* cdir_pwd;
	char* cdir_env;

	if((cdir_pwd = getenv("PWD")) != NULL) {
		printf("getenv(\"PWD\") return %s\n", cdir_pwd);
	} else {
		printf("getenv(\"PWD\") return NULL - not found\n");
		return -1;
	}

	while(*env) {
		if(cdir_env = strtok(*env,"=")) {
			if(!strcmp(cdir_env,"PWD")) {
				printf("${PWD} from **env is %s\n", strtok(NULL,"="));
			};
		} else {
			printf("ENV fail\n");
			return -1;
		}
		*env++;
	}

	return 0;
}