#include <iostream>
#include <string>
#include <iomanip>
#include <libconfig.h++>
using namespace std;
using namespace libconfig;

#define EXIT_FAILURE -1

int main(int argc, char const *argv[])
{
	Config cfg;
	try
	{
		cfg.readFile("test.cfg"); //读配置文件
	}
	catch(const FileIOException &fioex)
	{
		 std::cerr << "I/O error while reading file." << std::endl;
		 return(EXIT_FAILURE);
	}
	catch(const ParseException &pex)
	{
		 std::cerr << "Parse error at " << pex.getFile() << ":" << pex.getLine()
		              << " - " << pex.getError() << std::endl;
		 return(EXIT_FAILURE);
	}
	const Setting &root = cfg.getRoot();
	const Setting &interfaces = root["advp"]["interfaces"];
	for(int i=0;i<interfaces.getLength();i++)
	{
		const Setting &interface = interfaces[i];
		string if_name,ip,netmask;
		int active;
		interface.lookupValue("if_name",if_name);
		interface.lookupValue("ip", ip);
		interface.lookupValue("netmask", netmask);
		interface.lookupValue("active", active);
		cout<<if_name<<endl;
		cout<<ip<<endl;
		cout<<netmask<<endl;
		cout<<active<<endl;		
	}
	return 0;
}