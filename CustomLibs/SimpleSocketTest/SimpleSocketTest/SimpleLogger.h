#pragma once

#define _CRT_SECURE_NO_WARNINGS


#ifndef DISABLE_LOGGER

#include <chrono>
#include <iostream>
#include <string>
#include <fstream>
#include <ctime>
#include <filesystem>
#include<direct.h>
#include <deque>

///TODO Fix UTC delay in LOGS
#define UTC 3 //unied time const
#define CUR_TIME std::chrono::system_clock::now().time_since_epoch().count()+UTC*36000000000
#define STRT_TIME std::chrono::system_clock::now().time_since_epoch().count()-start_time

namespace SS{

	enum DataSaveType
	{
		DD_MM_YYYY = 0,
		MM_DD_YYYY = 1
	};

	enum MsgType 
	{
		Warn=0,
		Crit=1,
		Err=2,
		Inf=3
		
	};

	void Log(std::string message, MsgType type = Warn);

	void ChangePath(std::string NewPath);

	void ChangePath(DataSaveType NewSaveFormat);

	std::string ConvertMsecToStr(long long int msec, char splitter);
	
}



#endif
