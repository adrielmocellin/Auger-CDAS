#include <stdio.h>
#include <stdlib.h>
#include <netinet/in.h>
#include <string.h>

void usage(char *name) {
	printf("Usage: %s [-v] <station number|file name>\n   (-v for verbose)\n",name);
	exit(1);
}

int main(int argc, char **argv) {
	FILE *f;
	unsigned char buf;
	int totsize,i,j,nb,size,verbose=0,tmp;
	unsigned short sbuf;
	char *file,fname[2048];
	int bytesRead,previousOffset;;

	if (argc==1) usage(argv[0]);
	for (i=1;i<argc;i++) {
		if (strcmp(argv[i],"-v")==0) verbose=1;
		else file=argv[i];
	}
	f=fopen(file,"r");
	if (f==NULL) {
		sprintf(fname,"/Raid/var/trash/station%s",file);
		f=fopen(fname,"r");
		if (f==NULL) {
			printf("No such file %s or %s\n\n",file,fname);
			usage(argv[0]);
		}
	}
	bytesRead=0;
	while(fread(&buf,1,1,f)) {
		totsize=buf;previousOffset=bytesRead++;
		if (verbose) printf("size: %d %x\n",buf,buf);
		fread(&buf,1,1,f);bytesRead++;
		if (verbose) printf("frame: %d %x\n",buf,buf);
		fread(&buf,1,1,f);bytesRead++;
		if (verbose) printf("reserved: %d %x\n",buf,buf);
		fread(&buf,1,1,f);bytesRead++;
		if (verbose) printf("reserved: %d %x\n",buf,buf);
		fread(&sbuf,2,1,f);bytesRead++;bytesRead++;
		if (verbose) printf("source: %d %x\n",ntohs(sbuf),sbuf);
		fread(&buf,1,1,f);bytesRead++;
		nb=buf;
		printf("+ %d(%x) message(s):\n",buf,buf);
		totsize-=6;
		for(i=0;i<nb;i++) {
			fread(&buf,1,1,f);bytesRead++;
			totsize--;
			size=buf;
			printf("+ %.2d +\n",i+1);
			if (verbose) printf("     + size: %d\n",buf);
			fread(&buf,1,1,f);bytesRead++;
			if (buf!=0) {
				printf("     + completion (not 0)=%d\n",buf);
				printf("          + completion type: %d\n",buf>>6);
				printf("          + slice number: %d\n",buf&0x3f);
			} else if (verbose) printf("     + completion =%.2x\n",buf);
			fread(&buf,1,1,f);bytesRead++;
			printf("     + message type: %d\n",buf);
			fread(&buf,1,1,f);bytesRead++;
			tmp=buf;
			if (verbose) printf("     + message info: %d\n",tmp);
			if (verbose) printf("         + #message: %d\n",tmp>>2);
			if (verbose) printf("         + version: %d\n",tmp%4);
			printf("     + contents: ");
			totsize-=3;
			for(j=0;j<size-3;j++) {
				fread(&buf,1,1,f);bytesRead++;
				totsize--;
				printf("%.2x ",buf);
			}
			printf("\n");
		}
		if (verbose) printf("bytes read up to there : %d \n",bytesRead);
		if (totsize==1) {
			fread(&buf,1,1,f);bytesRead++;
			totsize--;
			/* New LSX does not 00 pad
			if (buf!=0)
				printf("******* WARNING, NOT 00 PADED ***** buf=%.2x\n",buf);
				*/
		}
		if (totsize!=0) {
			printf(" problem: bytes : %d\n",bytesRead);
			printf("ARGLLL, totsize=%d\n",totsize);
			fread(&buf,1,1,f);bytesRead++;
			printf("ARGLLL, value=%.2x\n",buf);
			printf("trying to go back \n");
			fseek(f,previousOffset,SEEK_SET);
			bytesRead=previousOffset;
			fread(&buf,1,1,f);bytesRead++;
			totsize=buf;
			printf("discarding %d bytes:\n",totsize+1);
			for(i=0;i<totsize;i++)
			{fread(&buf,1,1,f);bytesRead++;printf("%.2x ",buf);}
			printf("\n");
		}
	}
	return 0;
}
