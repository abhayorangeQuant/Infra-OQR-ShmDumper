#include "iostream"

#include "SettingFileReader.h"
#include "Strategy.h"

using namespace std;

uint64_t getMyTime()
{
	struct timespec tp;
	clock_gettime( CLOCK_REALTIME, &tp ) ;
	std::cout << tp.tv_sec << "|" << tp.tv_nsec << std::endl;
	return (((uint64_t)tp.tv_sec*1000000000)+tp.tv_nsec) ;
}

char szMailids[1000];
int EmailAlert(char* szSubject, char* szBody)
{
        char command[1000];
        sprintf(command, "echo \"%s\" | mail -s \"%s\" \"%s\"", szBody, szSubject, szMailids);
        if(!system(command))
        {
                std::cout << "Mail alert sent" << std::endl;
        }
        else
        {
                std::cout << "Failed to send email alert " << command << std::endl;
        }
        return 0;
}

int main(int argc, char* argv[])
{
	std::cout << "Starting market data recorder...." << std::endl;

	if(argc < 2)
	{
		std::cout << "MDRecorder configFile" << std::endl;
		return 0;
	}

	std::string filename(argv[1]);
    bool CheckSanity = argv[2];
	SettingsReader* reader = new SettingsReader();
	reader->ReadFile(filename);

	Strategy* strat = new Strategy(reader, CheckSanity);

	strat->Run();

	std::cout << "Exiting..." << std::endl;
	return 0;
}

