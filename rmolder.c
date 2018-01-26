#include <sys/types.h>
#include <dirent.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/param.h>
#include <sys/stat.h>
#include <time.h>
#include <unistd.h>


#define kOneDayOfSeconds (60L * 60L * 24L)


static void Usage(void);


int main(int argc,char **argv) {
	char *			inputPath	= NULL;
	time_t			days		= 30;
	int				doit		= 1;
	int				noisy		= 1;
	int				i;
	char			path[MAXPATHLEN];
	struct stat		sb;
	DIR *			dirp;
	struct dirent *	dp;
	time_t			maxAge;

	for (i = 1; i < argc; i++) {
		if (argv[i][0] == '-') {
			switch (argv[i][1]) {
				case 'h':
					Usage();
					break;
				case 'q':
					if (doit) {
						noisy = 0;
					}
					break;
				case 'n':
					doit	= 0;
					noisy	= 1;
					break;
				case 'd':
					i++;
					if (i < argc) {
						days = atoi(argv[i]);
						if (days < 1 || days > 365) {
							Usage();
						}
					} else {
						Usage();
					}
					break;
				default:
					fprintf(stderr,"Ignored unknown option: %s\n",argv[i]);
					break;
			}
		} else {
			inputPath = argv[i];
		}
	}

	if (!inputPath || *inputPath == 0) {
		Usage();
	}

	if (noisy) {
		fprintf(stderr,"Removing files older than %ld days from \"%s\".\n",days,inputPath);
		if (!doit) {
			fprintf(stderr,"   (dry run -- not actually removing)\n");
		}
	}

	maxAge = time(NULL) - (days * kOneDayOfSeconds);

	if (!realpath(inputPath,path)) {
		fprintf(stderr,"Directory \"%s\" doesn't exist!\n",inputPath);
		exit(1);
	}

	if (stat(path,&sb) || !(sb.st_mode & S_IFDIR)) {
		fprintf(stderr,"Path \"%s\" is not a directory!\n",path);
		exit(1);
	}

	dirp = opendir(path);
	if (!dirp) {
		fprintf(stderr,"Unable to open the directory!\n");
		exit(1);
	}

	do {
		dp = readdir(dirp);
		if (dp) {
			char buffer[MAXPATHLEN * 2];
	
			strcpy(buffer,path);
			strcat(buffer,"/");
			strcat(buffer,dp->d_name);
	
			if (!stat(buffer,&sb)) {
				if (!(sb.st_mode & S_IFDIR)) {
					if (sb.st_mtimespec.tv_sec < maxAge) {
						if (noisy) {
							fprintf(stderr,"Removing %s...\n",dp->d_name);
						}
						if (doit) {
							unlink(buffer);
						}
					} else if (noisy) {
						fprintf(stderr,"Skipping %s... [%ld days old]\n",dp->d_name,
							days - ((sb.st_mtimespec.tv_sec - maxAge) / kOneDayOfSeconds));
					}
				}
			}
		}
	} while (dp);

	closedir(dirp);

	return 0;
}

static void Usage(void) {
	fprintf(stderr,"Usage: rmolder [-h] [-n] [-d days] directory\n");
	fprintf(stderr,"    -h         This information\n");
	fprintf(stderr,"    -q         Don't show progress messages (ignored when -n is present)\n");
	fprintf(stderr,"    -n         Don't actually do anything\n");
	fprintf(stderr,"    -d days    Remove all files older than this number of days (default = 30)\n");
	fprintf(stderr,"    directory  The directory to work on\n");
	exit(1);
}
