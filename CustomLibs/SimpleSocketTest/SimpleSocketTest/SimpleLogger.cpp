#include "SimpleLogger.h"

std::string path = "";
std::ofstream fout;
std::string inst_path = "";
long long int start_time = std::chrono::system_clock::now().time_since_epoch().count();



void SS::Log(std::string message, MsgType type ) {

	std::string formed_message=ConvertMsecToStr(STRT_TIME,':') + " [";

	switch (type)
	{
	case SS::Warn:
		formed_message.append("WARN");
		break;
	case SS::Crit:
		formed_message.append("CRIT");
		break;
	case SS::Err:
		formed_message.append("ERR");
		break;
	case SS::Inf:
		formed_message.append("INFO");
		break;
	default:
		break;
	}
	
	formed_message.append("]: ");

	formed_message.append(message);

	std::string fullpath = path;

	if (fullpath != "") {

		fullpath.append("/");

	}
	fullpath.append("Log");
	_mkdir(fullpath.c_str());
	fullpath.append("/");
	std::time_t result = std::time(nullptr);
	
	char* str_date = std::asctime(std::localtime(&result));

	std::string today_folder;
	for (int  i = 0; i < 10; i++)
	{
		today_folder.push_back(str_date[i]);
	}

	str_date = nullptr;
	fullpath.append('/'+ today_folder);
	int md=_mkdir(fullpath.c_str());
	fullpath.append("/"+ ConvertMsecToStr(CUR_TIME, '-') + ".log");
	if (inst_path == "") {

		inst_path = fullpath;
	}
	fout.open(inst_path, std::ios_base::app);

	
	fout << formed_message<<std::endl;
	
	fout.close();
}

void SS::ChangePath(std::string NewPath) {

	path = NewPath;

}

void SS::ChangePath(DataSaveType NewSaveFormat) {

}

std::string SS::ConvertMsecToStr(long long int msec, char splitter) {
	msec = msec / 100000;

	int p_msec=msec % 100;
	long long int sec;
	sec = msec / 100;
	long long int min = floorf(float(sec/60));
	sec = sec % 60;
	long long int hour = floorf(float(min / 60));
	min = min % 60;
	hour = hour % 24;
	hour += UTC;

	std::string prop_time;


	if (hour<10){
		prop_time.push_back('0');
	}
	prop_time.append(std::to_string(hour) + splitter);

	if (min < 10) {
		prop_time.push_back('0');
	}
	prop_time.append(std::to_string(min)+splitter);

	if (sec < 10) {
		prop_time.push_back('0');
	}
	prop_time.append(std::to_string(sec) + splitter);

	if (p_msec < 10) {

		prop_time.push_back('0');
		prop_time.push_back('0');

	}
	else if (p_msec < 100) {

		prop_time.push_back('0');

	}
	prop_time.append(std::to_string(p_msec));

	return prop_time;

};