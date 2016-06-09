#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h>

typedef struct job
{
    long initialtime;
    long time;
    char job[10];
    char addr[20];
    struct job *next;
    struct job *before;
}jobqueue;

void printjob(jobqueue *k)
{
    printf("job started time : %li, kind : %s, target address : %s " , k->initialtime, k->job, k->addr);

}

typedef struct Rows
{
    char rowbuffer[5];
    int busy;
    struct job *tq;
    struct job *rq;
    int jsize;
    long totalbusy;
    long jobend;
}Row;

Row row[8];


void addqueue(int num, jobqueue *k)
{
    if(row[num].tq == row[num].rq && row[num].jsize==0)
    {
        row[num].tq = k;
        row[num].rq = k;
        k->before = NULL;
        k->next = NULL;
        row[num].jsize = 1;
    }
    else
    {
        k->next = row[num].tq;
        row[num].tq->before = k;
        row[num].tq = k;
        row[num].jsize +=1;
    }
}

void deletequeue(int num)
{
    if(row[num].jsize==0)
    {


    }
    else if(row[num].jsize==1)
    {
        row[num].jsize-=1;
        row[num].rq = NULL;
        row[num].rq = row[num].tq;
    }
    else
    {
        //printf("::::::::::::::%d\n", row[num].rq->time);
        row[num].jsize-=1;
        row[num].rq->before->next = row[num].rq->next;
        row[num].rq = row[num].rq->before;
    }

}


void initialize()
{
	int a=0;
	for(a; a<8; a++)
	{
		strcpy(row[a].rowbuffer,"ffff\0");
		jobqueue *j = (jobqueue*)malloc(sizeof(jobqueue));
		row[a].rq = j;
		row[a].tq = j;
		row[a].busy = 0;
		row[a].jsize = 0;
	}
}


char * getrow(char* addr)
{
	char *row;
	row = (char*)malloc(10*sizeof(char));
	strncpy(row, addr+2, 4);
	return row;

}//strlen

int getbank(char* addr)
{
	char* t = (char*)malloc(1);
	strncpy(t, addr+8, 1);
	int k = 0;
	k = (int)strtol(t, NULL, 16);
	return k/4;
}

int check=0;

int access(jobqueue* k, int bank)
{

    if(row[bank].busy == 1)
    {
        k->time = 9999999999999;
        //printf("busy\n");
        addqueue(bank, k);
        if(row[bank].jsize>1)
        {

            check++;
        }
        //printf("%s\n", row[bank].rq->addr);
    }
    else
    {
        row[bank].busy = 1;
        if(strncmp(row[bank].rowbuffer,getrow(k->addr),4)==0)
        {
		printf("hit\n");
		row[bank].jobend = k->time +154;
		row[bank].totalbusy += 154;
		return 154;

        }
        else if(strncmp(row[bank].rowbuffer, "ffff",4)==0)
        {
            printf("closed\n");
            strncpy(row[bank].rowbuffer, getrow(k->addr),4);
            row[bank].rowbuffer[4] = '\0';
            row[bank].jobend = k->time +258;
            row[bank].totalbusy += 258;
            return 258;

        }
        else
        {
            strncpy(row[bank].rowbuffer, getrow(k->addr),4);
            row[bank].rowbuffer[4] = '\0';
            printf("miss\n");
            row[bank].jobend = k->time +362;
            row[bank].totalbusy += 362;
            return 362;

        }
    }

}

int rowhit(jobqueue *k, int bank)
{
    if(strncmp(row[bank].rowbuffer,getrow(k->addr),4)==0)
    {
        return 1;
    }
    else
    {
        return 0;
    }

}


void scheduling()
{
    int a=0;
    int b=0;
    int cswitch=0;
    jobqueue *j;
    j = (jobqueue *)malloc(sizeof(jobqueue));
    for(a=0; a<4; a++)
    {
        cswitch = 0;
        if(row[a].jsize<2)
        {
            continue;
        }
        if(rowhit(row[a].rq, a)==0)
        {
            j = row[a].rq->before;
            for(b=0; b<row[a].jsize-1; b++)
            {
                if(rowhit(j,a)==1)
                {
                    cswitch=1;
                    break;
                }
                j = j->before;
            }

        }

        if(cswitch==1)
        {
            if(j==row[a].tq)
            {
                row[a].tq = j->next;
                j->before = row[a].rq;
                j->next->before = NULL;
                row[a].rq->next = j;
                j->next = NULL;
                row[a].rq = j;

            }
            else if(j==row[a].rq->before)
            {
                j->before->next = row[a].rq;
                row[a].rq->before = j->before;
                row[a].rq->next = j;
                j->before = row[a].rq;
                j->next = NULL;
                row[a].rq = j;

            }
            else
            {
                printf("\n\nSwiching COMPLETE \n\n\n");

                j->before->next = j->next;
                j->next->before = j->before;
                j->before = row[a].rq;
                row[a].rq->next = j;
                j->next = NULL;
                row[a].rq = j;

            }
        }

    }

}




void syncronize(long time)
{
    //scheduling();
    int a=0;
    //printf("%d ,,,    %d\n\n", row[0].jobend, time);
    for(a=0; a<4; a++)
    {
           if(row[a].jsize==0 && row[a].jobend <= time)
           {
               row[a].busy = 0; // job can be done in time
           }
           else if(row[a].jsize!=0 && row[a].jobend <= time)
           {
               row[a].busy = 0; // then it ends first stored job, but some jobs remain
               while(1)
                {
                    row[a].rq->time = row[a].jobend;
                    int nexttime = access(row[a].rq,a);
                    deletequeue(a);
                   // row[a].jobend += nexttime;
                    if(row[a].jobend > time)
                    {
                        row[a].busy = 1;
                        break;
                    }
                    else
                    {
                        row[a].busy = 0;
                    }
                    if(row[a].jsize==0 && row[a].jobend <= time)
                    {
                        row[a].busy = 0;
                        break;
                    }
                }
            }
            else //then the job should be running in current time.
            {
                row[a].busy = 1;
            }

     }
}

void printallbanks()
{
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
    int num = 0;
    jobqueue *j;
    j = (jobqueue *)malloc(sizeof(jobqueue));
    for(num; num<4; num++)
    {
        int jnum = row[num].jsize;
        int a=0;
        printf("-------------bank %d-----------\n", num);
        {
            int k = &row[num];
            printf(" current rowbuffer : %s \n", row[num].rowbuffer);
            printf(" current jobqueue size : %d \n ", row[num].jsize);
            if(row[num].busy==1)
            {
                printf(" Row is busy \n");
            }
            else
            {
                printf(" Row is idle \n");
            }
            printf(" current job : \n");
            j = row[num].rq;
            for(a; a<jnum; a++)
            {
                printf(" %d job \n", a);
                printjob(j);
                printf("\n");
                j = j->before;

            }

        }

    }
    printf("@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n\n\n");

}


int main()
{
	FILE* f;
	int j=0;
	int counter = 0;
	int totalbusy = 0;
	initialize();
	char addr[20];
	char jobname[10];
	long randomtime, currenttime;
	char nothing1[40], nothing2[40];
	srand(time(NULL));
	char W[] = "W";
	int wtd;
	//f= fopen("gcc.trc", "r");
	f = fopen("input.txt", "r");
	while(1)
	{
	    counter++;
	    if(counter>750)
        {
            break;
        }
	    /*printf(" 1 To progress 1 work, 0 to terminate :");
	    scanf("%d", &wtd);
	    if(wtd==0)
        {
            break;
        }
        */
		if(fscanf(f, "%s", nothing1)==EOF)
		{
			break;
		}
		fscanf(f, "%s", currenttime);
		fscanf(f, "%s", jobname);
		fscanf(f, "%s", addr);
		//fscanf(f, "%s", jobname);
		/*if(!strncmp(jobname,W,1))
        {
            fscanf(f, "%s", nothing2);
            strncpy(addr, nothing2, 20);
        }
        else
        {
            fscanf(f, "%s", addr);
            strcat(addr, "0");
            fscanf(f, "%s", nothing2);
        }
        */
        addr[10] = '\0';
		//printf("%s", row[2].rowbuffer);
		jobqueue *jq = (jobqueue *)malloc(sizeof(jobqueue));
		strncpy(jq->addr, addr,20);
		//randomtime = rand()%200; // 0 to 499, random number
		randomtime=135;
		printf("--------------------------------------------------\n");
		printf("Address : %s , Job : %s , Random time : %d , Bank : %d \n Total time :%d\n", addr, jobname, randomtime, getbank(addr), currenttime);
		printf("--------------------------------------------------\n\n\n");
		//strncpy(jq->job, jobname, 10);
		strncpy(jq->job, jobname, 1);

		syncronize(currenttime);
		currenttime += randomtime;
		jq->initialtime = currenttime;
		jq->time = currenttime;
		totalbusy += access(jq, getbank(addr));
        printallbanks();

	}
	//printf("%s\n", getrow(addr));
	//printf("%d\n", getbank(addr));
	fclose(f);
	//printf("totalbusy : %li", row[4].totalbusy);
}
