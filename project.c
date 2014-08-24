#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

/* Global Access token. Naughty Naughty Naughty */
char *access_token_GLOBAL = "288688992.1fb234f.b71d32601a6746cb9809590d6b53b181";
int num_urls_GLOBAL = 0;
char *pagination_url_GLOBAL = NULL;
char **url_array_GLOBAL = NULL;
int url_array_size_GLOBAL = 0;

/**
 * A pipe looks like: pfd[1]->[===]->pfd[0]
 * The 1 is used for writing and the 0 is used for reading.
 * This enum should make using pipes a little clearer.
 */
enum {
	READ,
	WRITE
};

/**
 * Prints a message explaining how to use this program and exits the program.
 */
void usage() {
	printf("Usage:\n"
	"   ./project USER_NAME\n"
	"Description:\n"
	"   Grabs pictures from USER_NAME's instagram recent media or feed,\n"
	"   and composes a mosaic image of his/her profile picture.\n");
	exit(EXIT_FAILURE);
}

/* Prototypes go here */
char** get_urls(char *user_id, int max_pics);
char** get_urls_from_JSON(int pipefd[2]);
char** get_id_and_pic_from_name(char *user_name);
char* get_user_pic_from_JSON(int pipefd[2]);
char* get_user_id_from_JSON(int pipefd[2]);
void* fork_stuff(char *curl_command[], void* (*do_stuff)(int pipefd[2]));
char* remove_all_char(char *str, char c);
void init_pipe(int pipefd[2]);
void unix_error(char *msg);
void dump_pipe_to_stdout(int pipefd[2]);
void addStringToArray(char *string, int loc, char ***array, int *array_size);
char* str_replace(char *orig, char *rep, char *with);
int get_number_of_pics(char *user_id);
int get_number_pics_from_JSON(int pipefd[2]);

/* Begin Main Program */
int main(int argc, char *argv[]) {

	/* Check to make sure that the program was started with at least one argument for the username */
	if (argc < 2) {
		printf("Quitting. User name was not supplied.\n");
		usage();
	}

	/* Get the user id and profile pic */
	char **user_info = get_id_and_pic_from_name(argv[1]);
	char *user_id = user_info[0];
	char *prof_pic = user_info[1];
	
	//Download profile pic
	pid_t prof_pid;
	if ( (prof_pid = fork()) == 0){
		execlp("wget", "wget", "-P","profPic/", prof_pic, NULL);
	} else {//parent process
		waitpid(prof_pid,NULL,0);
	}

	/* Allocate memory for the global array of urls */
	url_array_size_GLOBAL = 32;
	url_array_GLOBAL = malloc(url_array_size_GLOBAL * sizeof(char*));

	/* Get the number of user images */
	int pics_to_download = get_number_of_pics(user_id);
	printf("Number of pics:%d\n", pics_to_download);

	/* Get the array of URL's */
	char **urls = get_urls(user_id, pics_to_download);

	/* Download the images  */
	int i;
    pid_t url_pid[num_urls_GLOBAL];
	#pragma omp parrallel for
    for (i = 0; i < num_urls_GLOBAL; i++) {
        if((url_pid[i] = fork()) == 0) {
            execlp("wget", "wget", "-P", "downloads/", urls[i], NULL);
        }
        else {
            waitpid(url_pid[i], NULL, 0);
        }
    }

	/* Deallocate the dynamically allocated user_id string */
	free(user_id);

	/* Deallocate the array of strings containing the urls */
	for (i = 0; i < num_urls_GLOBAL; i++) {
		free( urls[i] );
	}
	free(urls);

	printf("Number of Images downloaded : %d\n", num_urls_GLOBAL);
	printf("\nBye Bye.\n");
}

int get_number_of_pics(char *user_id) {
	/* Build the URI for following command: curl "https://api.instagram.com/v1/users/208614288?access_token=288688992.1fb234f.b71d32601a6746cb9809590d6b53b181" */
	char *uri_p1 = "https://api.instagram.com/v1/users/";
	char *uri_p2 = "?access_token=";

	/* Allocate memory to store the URL string */
	int url_size = strlen(uri_p1) + strlen(user_id) + strlen(uri_p2) + strlen(access_token_GLOBAL) + 1;
	char *url = malloc(url_size);

	/* Copy the pieces of the URI into the URL string */
	snprintf(url, url_size, "%s%s%s%s", uri_p1, user_id, uri_p2, access_token_GLOBAL);

	/* Create the curl command to execute */
	char *curl_command[] = { "curl", "--silent", url, NULL };

	/* Execute the curl command and get the number of user media from the JSON */
	int pics_to_download = fork_stuff(curl_command, (void*) get_number_pics_from_JSON);

	return pics_to_download;
}

int get_number_pics_from_JSON(int pipefd[2]) {

	/* Open the pipe for reading */
	FILE *f = fdopen(pipefd[READ], "r");

	/* Read lines from the file and search for the string signifiying the id */
	char buffer[BUFSIZ];
	char *loc = NULL;
	while ( (fgets(buffer, BUFSIZ, f)) != NULL ) {

		/* If the line has the substring key in it */
		char *key = "\"media\":";
		if ( (loc = strstr(buffer, key)) != NULL ) {

			/* find the position of the start of the key value */
			char *start = loc + strlen(key);

			/* Find the ending position of the key value */
			char *end = strstr(start, ",");

			/* Null terminate the string */
			*end = '\0';

			/* Return the string */
			int lolol = atoi(start);
			return lolol;
		}
	}

	/* If substring isn't found, quit and alert user */
	fprintf(stderr, "Substring not found in get_number_pics_from_JSON.\n");
	exit(EXIT_FAILURE);
}

/**
 * LOL
 */
char** get_urls(char *user_id, int max_pics) {
	/* Build the URI for he following command: curl "https://api.instagram.com/v1/users/3/media/recent/?access_token=288688992.1fb234f.b71d32601a6746cb9809590d6b53181" */
	char *request_part1 = "https://api.instagram.com/v1/users/";
	char *request_part2 = "/media/recent/?access_token=";

	/* Allocate memory enough to store the URL string */
	int url_size = strlen(request_part1) + strlen(user_id) + strlen(request_part2) + strlen(access_token_GLOBAL) + 12;
	char *url = malloc(url_size);

	/* Copy the pieces of the URI into the URL string */
	snprintf(url, url_size, "%s%s%s%s%s", request_part1, user_id, request_part2, access_token_GLOBAL, "&count=2000");

	/* If max pics is greater than 200 use 200 */
	if (max_pics > 200)	max_pics = 200;

	char **rv;
	do {
		/* Create a curl command argument vector to be used for execv */
		char *curl_command[] = { "curl", "--silent", url, NULL };

		/* Execute the curl command, get back an array of strings containing the url's of the instagram picture feed */
		rv = fork_stuff(curl_command, (void*) get_urls_from_JSON);

		/* Free the dynamically allocated url string */
		free(url);
		url = NULL;

		/* Assign the global url for the next_page of images into url */
		url = pagination_url_GLOBAL;
		pagination_url_GLOBAL = NULL;

		printf("Number of urls found: %d\n", num_urls_GLOBAL);

	} while ( !(num_urls_GLOBAL > max_pics) );

	/* Return the array of strings */
	return rv;
}

/**
 * This function reads lines from the file and search for the string "standard_resolution":
 * The "standard_resolution" string indicates that the url we want is on the next line.
 * @param pipefd[2] The pipe to read the JSON from
 * @return The array of strings that contain the urls
 */
char** get_urls_from_JSON(int pipefd[2]) {

	/* Open the pipe for reading */
	FILE *f = fdopen(pipefd[READ], "r");
	char buffer[BUFSIZ], *pos = NULL;

	/* Look for and save the pagination URL */
	char *key = "\"next_url\":";
	while ( (fgets(buffer, BUFSIZ, f)) != NULL ) {

		/* If the line has the string "next_url": in it then save the url */
		if ( (pos = strstr(buffer, key)) != NULL ) {

			/* Find the beginning of the url */
			char *start = strstr(pos+strlen(key), "\"")+1;

			/* Find the end of the url */
			char *end = strstr(start, "\"");

			/* NUL terminate the string start where the ending quote of the nex_url is */
			*end = '\0';

			/* String replace the unicode character for the ampersand */
			pagination_url_GLOBAL = str_replace(start, "\\u0026", "&");

			/* Remove all backslashes from the url */
			remove_all_char(pagination_url_GLOBAL, '\\');
		
			/* Found the pagination url so exit the loop */
			break;
		}
	}

	/* Read lines from the file, saving only the URL's we need into the char array */
	int save_url = 0;
	while ( (fgets(buffer, BUFSIZ, f)) != NULL ) {

		/* If save_the_url flag is true, then parse the line for the url */
		if (save_url) {

			/* Find the location right after the "url:" substring */
			key = "\"url\":";
			pos = strstr(buffer, key) + strlen(key);

			/* Find the location of the start of the url */
			char *start = strstr(pos, "\"") + 1;

			/* Find the end of the url */
			char *end = strstr(start, "\"");

			/* Nul terminate the string */
			*end = '\0';

			/* Remove backslashes and spaces from the url */
			remove_all_char(start, '\\');
			remove_all_char(start, ' ');

			/* Add the urls to the array */
			addStringToArray(start, num_urls_GLOBAL, &url_array_GLOBAL, &url_array_size_GLOBAL);

			/* Increment the location in the results array */
			num_urls_GLOBAL++;

			/* Don't save the url on the next run through */
			save_url = !save_url;

		/* Or, if the line has the substring "standard_resolution": in it set the save_url flag */
		} else if ( (pos = strstr(buffer, "\"low_resolution\":")) != NULL ) {
			/* Set the save_the_url flag to true */
			save_url = !save_url;
		}
	}

	/* Return the array of urls */
	return url_array_GLOBAL;
}

/**
 * This function takes an instagram user name and returns the user id of the first
 * result from the instagram API user name search get request
 * @param user_name The user's user name on Instagram
 * @return The string containing the user id
 * NOTE:
 *     The return string was dynamically allocated and free must be called on it
 */
char** get_id_and_pic_from_name(char *user_name) {
	/* Build the URI for the following command: curl "https://api.instagram.com/v1/users/search?q=Kierchon&access_token=288688992.1fb234f.b71d32601a6746cb9809590d6b53b181" */
	char *request = "https://api.instagram.com/v1/users/search?q=";
	char *uri_variable = "&access_token=";

	/* Allocate memory for the string to store the url in */
	int url_size = strlen(request) + strlen(user_name) + strlen(uri_variable) + strlen(access_token_GLOBAL) + 1;
	char *url = malloc(url_size);

	/* Copy all the pieces of the url into the url string */
	snprintf(url, url_size, "%s%s%s%s", request, user_name, uri_variable, access_token_GLOBAL);

	/* Put together the curl command using the url */
	char *curl_command[] = { "curl", "--silent", url, NULL };

	/* Execute the curl command, get the user id from the resultant JSON, and save it in the result string */
	char *id = fork_stuff(curl_command, (void*) get_user_id_from_JSON);
	char *pic = fork_stuff(curl_command, (void*) get_user_pic_from_JSON);
	char *formattedPic = remove_all_char(pic, '\\');

	/* Free the dynamically allocated url string */
	free(url);
	char **rv = malloc(sizeof(char*)*2);
	rv[0]=id;
	rv[1]=formattedPic;
	/* Return the result string */
	return rv;
}

/**
 * This function grabs the user id number from formatted JSON, by searcing each line
 * until it finds a match for the string "id:", and does some substring copying
 * @param pipefd[2] The pipe to read the JSON out of
 * @param return The string containing the user id number
 */
char* get_user_id_from_JSON(int pipefd[2]) {

	/* Open the pipe for reading */
	FILE *f = fdopen(pipefd[READ], "r");

	/* Read lines from the file and search for the string signifiying the id */
	char buffer[BUFSIZ];
	char *loc = NULL;
	while ( (fgets(buffer, BUFSIZ, f)) != NULL ) {

		/* If the line has the substring "id" in it */
		if ( (loc = strstr(buffer, "\"id\":")) != NULL ) {

			/* find the position of the start of the id number */
			char *start = strstr(loc+4, "\"") + 1;

			/* Find the ending position of the id */
			char *end = strstr(start, "\"");

			/* Create a string to return the user id */
			int str_size = end - start;
			char *rv = malloc(str_size + 1);

			/* Copy the string into the return value */
			strncpy(rv, start, str_size);

			/* Null terminate the string */
			rv[str_size] = '\0';

			/* Return the string */
			return rv;
		}
	}

	/* If substring isn't found, quit and alert user */
	fprintf(stderr, "Substring not found in get_user_id_from_JSON.\n");
	exit(EXIT_FAILURE);
}

/**
 * This function grabs the profile picture url from formatted JSON, by searcing each line
 * until it finds a match for the string "profile_picture:", and does some substring copying
 * @param pipefd[2] The pipe to read the JSON out of
 * @param return The string containing the user prof pic
 */
char* get_user_pic_from_JSON(int pipefd[2]) {

	/* Open the pipe for reading */
	FILE *f = fdopen(pipefd[READ], "r");

	/* Read lines from the file and search for the string signifiying the id */
	char buffer[BUFSIZ];
	char *loc = NULL;
	while ( (fgets(buffer, BUFSIZ, f)) != NULL ) {

		/* If the line has the substring "profile_picture" in it */
		if ( (loc = strstr(buffer, "\"profile_picture\":")) != NULL ) {

			/* find the position of the start of the URL */
			char *start = strstr(loc+19, "\"") + 1;

			/* Find the ending position of the URL */
			char *end = strstr(start, "\"");

			/* Create a string to return the user prof pic */
			int str_size = end - start;
			char *rv = malloc(str_size + 1);

			/* Copy the string into the return value */
			strncpy(rv, start, str_size);

			/* Null terminate the string */
			rv[str_size] = '\0';

			/* Return the string */
			return rv;
		}
	}

	/* If substring isn't found, quit and alert user */
	fprintf(stderr, "Substring - profile_picture not found\n");
	exit(EXIT_FAILURE);
}

/**
 * This function will run a command that returns JSON and pipes it to the jsb beautifier from the lab.
 * Then it will pipe the formatted JSON from the beautifier to the do_stuff function that will do whatever
 * it wants and can return whatever it wants, hence the void* return type
 * @param curl_command The curl command that will be executed which returns raw JSON 
 * @param do_stuff The function that takes a pipe and does anything it wants with it and can return something
 * @return Returns the return value from the function pointed to by the do_stuff function pointer argument
 */
void* fork_stuff(char *curl_command[], void* (*do_stuff)(int pipefd[2])) {

	/* Generic return Pointer */
	void *rv = NULL;

	int child_status;
	int child_status2;

	/* Declare file descriptors for the pipe file */
	int curlfd[2], jsbfd[2], parsefd[2];

	/* Declare process id's to be used for the fork child processes */
	pid_t curl_pid, jsb_pid, parse_pid;

	init_pipe(curlfd);

	/* Fork a process that will curl the JSON for the Instagram Feed */
	if ( (curl_pid = fork()) < 0 ) {
		unix_error("Forking at curl_pid failed...\n");

	/* If the pid is 0 then this process is the child process */
	} else if (curl_pid == 0) {
		/* Reroute all output that should go to stdout, to go to the pipe instead */
		dup2(curlfd[WRITE], STDOUT_FILENO);

		/* Close the read end of the pipe. We don't need it */
		close(curlfd[READ]);

		/* Download the JSON for the user ID. */
		execv("/usr/bin/curl", curl_command);

		/* If the program reaches this point, execl didn't execute */
		unix_error("The execl command failed to execute the curl request. Exiting child...\n");

	/* Otherwise, this is the parent process */
	} else {
		/* Close parent write end of the pipe, so we can wait for the EOF from the Child */
		close(curlfd[WRITE]);

		/* Open a pipe to be used for sending JSON that must be beautified into */
		init_pipe(jsbfd);

		/* Fork a process that will beautify the JSON */
		if ( (jsb_pid = fork()) < 0 ) {
			unix_error("Forking at jsn_pid failed...\n");

		/* If the pid is 0 the this is the child process */
		} else if (jsb_pid == 0) {

			/* We will be reading from the curlfd pipe */
			dup2(curlfd[READ], STDIN_FILENO);

			/* Reroute output for stdout to the pipe instead */
			dup2(jsbfd[WRITE], STDOUT_FILENO);

			/* Call the json beautifier */
			execl("./jsb", "jsb", NULL);

			/* If the program reaches this point, execl didn't execute */
			unix_error("Try recompiling jsb.c; execl() failed to execute the jsb beautifier. Exiting child...\n");

		/* If the pid is not 0, this is the parent process */
		} else {

			/* We aren't writing to the pipe so close the write end, so we can wait for EOF from child */
			close(jsbfd[WRITE]);

			/* Get all the URL's in the JSON */
			rv = do_stuff(jsbfd);

			/* Finished reading from the pipe so close it */
			close(jsbfd[READ]);

			/* Wait for and reap the child */
			waitpid(jsb_pid, &child_status2, 0);
		}

		/* We are done reading from the pipe. Close it */
		close(curlfd[READ]);

		/* Wait for the child process to finish, and reap it */
		waitpid(curl_pid, &child_status, 0);
	}

	/* Return the result */
	return rv;
}

/**
 * Removes all instances of a specified character from a string.
 * @param str The string to be modified
 * @param c The character to be removed from the string
 * @return The modified string
 */
char* remove_all_char(char *str, char c) {

	// EXAMPLE USAGE BIZNITCH
	// int main(int argc, char *argv[]) {
	// 	FILE *f = fopen(argv[1],"r");
	// 	char buffer[BUFFER_SIZE];
	// 	while (fgets(buffer, BUFFER_SIZE, f)) {
	// 		printf("%s",remove_all_char(buffer, '\\'));
	// 	}
	// }

	char *p = str, *q = str;
	while (*q == c) q++;
	while (*q) {
		*p++ = *q++;
		while (*q == c) q++;
	}
	*p = *q;
	return str;
}

/**
 * Tries to open a pipe. Exits program if piping fails.
 * @param pipefd[2] The pipe to be opened
 */
void init_pipe(int pipefd[2]) {
	/* Open the pipe and error check the result */
	if ((pipe(pipefd)) < 0) {
		unix_error("Piping failed. Exiting...\n");
	}
}

/**
 * Prints a custom error message followed by the error message corresponding to
 * errno that is set by a failed c function
 * @param msg A char array containing the custom message
 */
void unix_error(char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

/** DEBUG thisss SHIT up in  Herrryyyeee */
void dump_pipe_to_stdout(int pipefd[2]) {
	int n;
	char buffer[BUFSIZ];
	while ( (n = read(pipefd[READ], buffer, BUFSIZ)) ) {
		if (n < 0) {
			unix_error("Error reading from pipe in function \'dump_pipe_to_stdout\'. Exiting...\n");
		}
		write(STDOUT_FILENO, buffer, n);
	}
}

/**
* Given a string, an index, an array, and its current size, append the string to the array resizing if necessary.
* @param string the string to be added to the array
* @param loc the index to add the string at in the array
* @param arr the pointer to the 2D-array
* @param size current size of the 2D-array
*/
void addStringToArray(char *string, int loc, char ***array, int *array_size) {
    // Create a new variable for the new size of the array. Makes sure it is not less than 1.
    int new_size;
    if (*array_size < 1) {
        new_size = 1;
    } else {
        new_size = *array_size;
    }

    // Double the size of the array until we can access the array at index loc
    while (loc >= new_size) {
        new_size *= 2;
    }

    // Resize the array if the new size is determined to be different than the current size of the array
    if (new_size != *array_size) {
        *array = (char **) realloc(*array, new_size * sizeof(char*));
    }

    // Copy the string into the array at index loc
    (*array)[loc] = malloc(strlen(string)+1); /* +1 because strlen doesn't count the terminating nul */

    strcpy((*array)[loc], string);
}

// You must free the result if result is non-NULL.
char* str_replace(char *orig, char *rep, char *with) {
    char *result; // the return string
    char *ins;    // the next insert point
    char *tmp;    // varies
    int len_rep;  // length of rep
    int len_with; // length of with
    int len_front; // distance between rep and end of last rep
    int count;    // number of replacements

    if (!orig)
        return NULL;
    if (!rep)
        rep = "";
    len_rep = strlen(rep);
    if (!with)
        with = "";
    len_with = strlen(with);

    ins = orig;
    for (count = 0; (tmp = strstr(ins, rep)); ++count) {
        ins = tmp + len_rep;
    }

    // first time through the loop, all the variable are set correctly
    // from here on,
    //    tmp points to the end of the result string
    //    ins points to the next occurrence of rep in orig
    //    orig points to the remainder of orig after "end of rep"
    tmp = result = malloc(strlen(orig) + (len_with - len_rep) * count + 1);

    if (!result)
        return NULL;

    while (count--) {
        ins = strstr(orig, rep);
        len_front = ins - orig;
        tmp = strncpy(tmp, orig, len_front) + len_front;
        tmp = strcpy(tmp, with) + len_with;
        orig += len_front + len_rep; // move to next "end of rep"
    }
    strcpy(tmp, orig);
    return result;
}
