/*
 * IFrame.h
 *
 *  Created on: Fev 24, 2014
 *      Author: fets
 */

#ifndef IFRAME_H
#define IFRAME_H

#include <string.h>

#include "DomoasterLog.h"

namespace domoaster {

class IFrame
{
  public:
    uint8_t receptor;
    uint8_t sender;
    uint8_t taille;
    uint8_t data[32];

    IFrame () {
      receptor = 0;
      sender = 0;
      memset(data, 0, sizeof(data));
    } ;
    virtual ~IFrame () {} ;
    void Dump (const char * msg)
    {
      char sztmp[255];

      if (msg)
        DOMOASTER_DEBUG << msg ;

      DOMOASTER_DEBUG << "Emetteur : " << (int) sender;
      DOMOASTER_DEBUG << "Recepteur :" << (int) receptor;

      sprintf(sztmp, "Data Hex: ");
      for (int a=0; a < taille - 1; a++)
        sprintf(sztmp,"%s 0x%02X", sztmp, data[a]);
      DOMOASTER_DEBUG << sztmp;

      sprintf(sztmp, "Data Dec: ");
      for (int a=0; a < taille - 1; a++)
        sprintf(sztmp,"%s %d", sztmp, data[a]);
      DOMOASTER_DEBUG << sztmp;
    }
} ;

} ; // namespace domoaster

#endif // IFRAME_H