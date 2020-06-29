// Anuneet Anand
// 2018022
// OS Assignment-3 : Shell

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <unistd.h>

void parse(char* args[])
{
	// To Obtain Space Separated Arguments
	char *input = (char*)malloc(sizeof(char)*10000);
	fgets(input,10000,stdin);
	input[strlen(input)-1] ='\0';
	int w = 0,c = 0;
	while(*input!='\0')
	{
		while((*input!=' ')&&(*input!='\0'))
		{
			args[w][c]=*input;
			input++;
			c++;
		}
		if(strlen(args[w])>0){w++;} //Ignoring Empty String
		input++;
		c = 0;
	}
	args[w] = NULL;					//Appending NULL
}

void resolve(char* cmd[])
{
	// To Resolve All I/O Redirections
	char *temp[10000];
	for(int i=0;i<10000;i++){temp[i]=(char*)malloc(sizeof(char)*100);}
	int i=0,j=0;
	while(cmd[i]!=NULL)
	{
		char* file;
		
		if(!strcmp(cmd[i],">>"))						// Append
		{
			file = cmd[++i];
			close(1);
			open(file,O_APPEND|O_WRONLY|O_CREAT,0666);
		}

		else if(strchr(cmd[i],'>')!=NULL)    			
		{	
			file = strchr(cmd[i],'>') + 1;
			if (cmd[i][0]=='1')   					   // 1>Filename
			{
				close(1);
				open(file,O_WRONLY|O_CREAT,0666);
			}
			else if(cmd[i][0]=='2')					   // 2>&1
			{
				if(file[0]=='&')
				{
					close(2);
					dup(1);
				}
				else								  // 2>Filename
				{
					close(2);
					open(file,O_WRONLY|O_CREAT,0666);
				}
			}
			else               						  // Command > Filename
			{
				file = cmd[++i];
				close(1);
				open(file,O_WRONLY|O_CREAT,0666);
			}	
		}

		else if (strchr(cmd[i],'<')!=NULL)
		{
			if(strlen(cmd[i])>1)					  // 0<Filename
			{
				file = strchr(cmd[i],'<') + 1;
				if (cmd[i][0]=='0')
				{
					close(0);
					open(file,O_RDONLY,0666);
				}
			}
			else   									  // Command < Filename
			{	
				file = cmd[++i];
				close(0);
				open(file,O_RDONLY,0666);
			}
		}

		else										  // Command
		{
			temp[j]=cmd[i];
			j++;
		}
		i++;
	}

	temp[j] = NULL;
	memcpy(cmd,temp,sizeof(temp));
}

int count_pipes(char* args[])
{
	// Count No. Of Pipes In Input
	int i = 0;
	int c = 0;
	while(args[i]!=NULL)
	{
		if (!strcmp(args[i],"|")) {c++;}
		i++;
	}
	return c;
}

int main(int argc, char* argv[])
{
	char *S = "\033[32;1mAnuneet$ \033[0m";
	while (1)
	{
		write(1,S,strlen(S));
		char *args[10000];
		for(int i=0;i<10000;i++){args[i]=(char*)malloc(sizeof(char)*100);}
		parse(args);
		if (args[0]==NULL){continue;}						// Ignore Empty Line
	    else if(!strcmp(args[0],"exit")) {exit(0);}			// Exit The Shell
		else if(!strcmp(args[0],"cd")) {chdir(args[1]);}	// Change Current Directory
		else
		{
			int pid = fork();
			if (pid == 0) 									
			{
				int pc = count_pipes(args);					
				int fd[pc*2];
				for(int x=0;x<pc*2;x++) 
				{if(x%2==0){pipe(fd+x);}}					// Initialise Pipe File Descriptors

				int a,b,k,j=0,i=0,cn=0;
				int X = pc+1;								// Number Of Commands
				while(X)
				{
					j = 0;
					char *cmd[10000];
					for(int i=0;i<10000;i++){cmd[i]=(char*)malloc(sizeof(char)*100);}
					
					while( (args[i]!=NULL) && (strcmp(args[i],"|")!=0))
					{	strcpy(cmd[j++],args[i++]);}		// Copy Arguments For A Command
	
					cmd[j] = NULL;
					if(cmd[0]==NULL){continue;}
					resolve(cmd);							// Resolve I/O Redirections

					X--; i++;
					k = cn++;
					a = 2*k - 2;
					b = 2*k + 1;
					
					int fid = fork();						// Create Child Process To Execute The Command
					if (fid == 0)
					{
						if (k!=0)
						{
							close(0);
							dup(fd[a]);						// Redirect Input
							close(fd[a]);
						}
						if (k<pc)
						{
							close(1);
							dup(fd[b]);						// Redirect Output
							close(fd[b]);
						}
						for (int z=0;z<pc*2;z++) {close(fd[z]);}
						char *path = cmd[0];
						execvp(path,cmd);
						printf("%s\033[31;1m: command not found\033[0m\n",path);
						exit(0); 
					}
				}
				for (int z=0;z<pc*2;z++) {close(fd[z]);}
				while(wait(NULL)>=0);						// Wait For All Commands To Finish
				exit(0);
			} 
			else if (pid > 0) 
			{ while(wait(NULL)>=0);} 						// Wait For Child To Finish
			else
			{ printf("\033[31;1mERROR\033[0m\n"); }
		}
	}
	return 0;
}

// END OF CODE