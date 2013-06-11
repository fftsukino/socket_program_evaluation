#include <stdio.h>
#include <unistd.h>

int main(int argc, char *argv[]){
	int ret;
	
	// ３番めがオプション
	// abc　は値をとらない
	// d:e:　は値をとる
	while( (ret=getopt(argc, argv, "abcd:e:")) != -1 ){
		switch(ret){
		case 'a':
		case 'b':
		case 'c':
			fprintf(stdout, "%c\n", ret);
			fprintf(stdout, "optind %d\n", optind);
			break;
		case 'd':
		case 'e':
			fprintf(stdout, "%c %s\n", ret, optarg);
			fprintf(stdout, "optind %d\n", optind);
			break;
		case ':':
			fprintf(stdout, "%c needs value\n", ret);
			fprintf(stdout, "optind %d\n", optind);
			break;
		case '?':
			fprintf(stdout, "unknown options\n");
			fprintf(stdout, "optind %d\n", optind);
			break;
		}
		fprintf(stdout, "argv %d\n", argc);
		fprintf(stdout, "optind %d \n", optind);
		
		for(;optind<argc;optind++){
			fprintf(stdout, "optind %d \n", optind);
			fprintf(stdout, "non option arg %s \n", argv[optind]);
		}

	}

//	fprintf(stdout, "argv %d\n", argc);
//	for(;optind<argc;optind++){
//		fprintf(stdout, "non option arg %s \n", argv[optind]);
//	}

}
