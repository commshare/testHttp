#ifndef DATAACCESS_H
#define DATAACCESS_H

#include <map>
#include <string>

namespace domoaster {

class DataAccess : public std::map <std::string, double>
{
public:
	bool	Get(std::string n, double & v) ;
	static DataAccess data ;
};

} ; // namespace domoaster

#endif // DATAACCESS_H