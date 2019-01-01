/*
 * INode.h
 *
 *  Created on: Fev 24, 2014
 *      Author: fets
 */

#ifndef INODE_H
#define INODE_H

#include <string>
#include <list>

#include "DomoasterLog.h"

using namespace std;

namespace domoaster {

class INode
{
  public:
    virtual ~INode () {} ;

    virtual std::string Name ()  = 0 ;
    virtual std::string Class ()  = 0 ;
    virtual std::string Protocol ()  = 0 ;

    virtual void Init () = 0 ;
    virtual void Start ()  = 0 ;
} ;

} ; // namespace domoaster

#endif // INODE_H