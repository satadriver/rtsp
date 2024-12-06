

#include <string>

#include "utils.h"
#include <cstring>


using namespace std;

string getstring(const char* start, const  char* end, const  char* lpdata) {
	char* lphdr = 0;
	if (start == 0 || *start == 0) {
		lphdr = (char*)lpdata;
	}
	else {
		lphdr = strstr((char*)lpdata, start);
		if (lphdr) {
			lphdr += strlen(start);
		}
	}

	if (lphdr)
	{
		char* lpend = 0;
		if (end == 0 || *end == 0) {
			lpend = lphdr;
			return lphdr;
		}
		else {
			lpend = strstr(lphdr, end);
			if (lpend) {
				int len = lpend - lphdr;
				if (len > 0) {
					return string(lphdr, len);
				}
				
			}

		}
	}
	return "";
}




#ifndef _WIN32

#include <vector>
#include <sys/types.h>
#include <dirent.h>
#include <iostream>
#include <stdio.h>
#include <signal.h>

#define BUF_SIZE 1024

//获取进程pid
vector<string> GetPidFromName(std::string ProcessName) {
    vector<string> pids;
    const char* cprocessName = ProcessName.c_str();
    DIR* dir;
    struct dirent* ptr;
    FILE* fp;
    char filepath[256];//存放cmline文件路径
    char cur_task_name[256];//存放命令行文本
    char buf[1024];
    dir = opendir("/proc"); //打开路径，读取下面每一个文件夹
    if (NULL != dir)
    {
        while ((ptr = readdir(dir)) != NULL)
        {
            //如果是"."或者".."就跳过
            if ((strcmp(ptr->d_name, ".") == 0) || (strcmp(ptr->d_name, "..") == 0))
                continue;
            if (DT_DIR != ptr->d_type)
                continue;
            sprintf(filepath, "/proc/%s/status", ptr->d_name);
            fp = fopen(filepath, "r");//打开status文件
            if (NULL != fp)
            {
                if (fgets(buf, BUF_SIZE - 1, fp) == NULL) {
                    fclose(fp);
                    // printf("fgets %s error\n", filepath);
                    continue;
                }

                sscanf(buf, "%*s %s", cur_task_name);

                //status中的进程名与要杀死的进程名是否匹配
                if (!strcmp(cprocessName, cur_task_name)) {
                    // printf("Get process:%s PID:  %s\n", cprocessName,ptr->d_name);
                    pids.push_back(ptr->d_name);
                }
                fclose(fp);
            }
            else {
                // printf("fopen %s error\n", filepath);
            }
        }

        vector<string>::iterator it;
        // std::cout << cprocessName << " --pids:   ";
        for (it = pids.begin(); it != pids.end(); it++) {
            // std::cout << *it << "   ";
        }
        // std::cout << std::endl;
        closedir(dir);
    }else{
    	// printf("open dir /proc error\r\n");
    }
    return pids;

}


void KillProcess(vector<string> pids, vector<string>& pids_failed) {

    vector<string>::iterator it;
    for (it = pids.begin(); it != pids.end(); it++) {
        // std::cout << *it << "   ";
        int num;
        sscanf((*it).c_str(), "%d", &num);
        //long runtime = get_process_runtime(num, m_processName.c_str());       //get pid's running time;
        //if (runtime > m_timeout) 
        if(1)
        {

            //if (isRunning(num) == 0) 
            {
                //// printf("%d is running,start to kill it\n", num);
                int retval = kill(num, SIGKILL);  //发送SIGKILL信号给进程，要求其停止运行
                if (retval < 0)   //判断是否发生信号
                {
                    // printf("kill %d failed", num);
                    pids_failed.push_back(*it);
                }
                else
                {
                    // printf("%d killed\n", num);
                }
            }

        }
        else {
            // printf("[ %d ]runtime<timeout, this is no need to kill it\n ", num);
        }
    }
}



void KillMonolith() {
    vector<string> str = GetPidFromName("monolith");
    vector<string> fail;
    KillProcess(str, fail);
    return;
}

#endif
