/*
 * IConnector.h
 *
 *  Created on: Fev 24, 2014
 *      Author: fets
 */

#ifndef ICONNECTOR_H
#define ICONNECTOR_H

#include <boost/signals2.hpp>
#include <boost/bind.hpp>
#include <sys/time.h>

#include "IProtocol.h"
#include "INode.h"

#include "SettingsParser.h"
#include "DomoasterLog.h"
#include "ListPtr.h"

namespace domoaster {

    class IConnector {
    public:
        virtual std::string Name() = 0;
        virtual std::string Class() = 0;

        virtual void Init() = 0;
        virtual void Start() = 0;

        virtual void Receive() = 0;
        virtual void Send(bool) = 0;
        virtual void Send(std::vector<uint8_t> &) = 0;

    public:

        IConnector() {
        };

        virtual ~IConnector() {
        };

        typedef ListPtr<IProtocol> ProtocolList;

        void RegisterProtocol(IProtocol * p) {
            this->_protocols.push_back(p);
        }
        ProtocolList & Protocols();

        void Dispatch(uint8_t rx_value) {
            ProtocolList & l = _protocols;
            for (ProtocolList::iterator it = l.begin(); it != l.end(); ++it) {
                IProtocol * pt = *it;
                pt->Receive(rx_value);
            }
        }

        void Dispatch(std::vector<uint16_t> & rx_values) {
            ProtocolList & l = _protocols;
            for (ProtocolList::iterator it = l.begin(); it != l.end(); ++it) {
                IProtocol * pt = *it;
                pt->Receive(rx_values);
            }
        }
    private:
        ProtocolList _protocols;
    };

}; // namespace domoaster

#endif // ICONNECTOR_H