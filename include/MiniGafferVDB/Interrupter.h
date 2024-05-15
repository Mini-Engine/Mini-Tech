#pragma once

#include "IECore/Canceller.h"

namespace MiniGafferVDB
{

class Interrupter {
public:
    Interrupter(const IECore::Canceller *canceller);
    void start(const char* name = nullptr);
    void end();
    bool wasInterrupted(int percent=-1);
private:
    const IECore::Canceller* canceller;
    bool interrupted;

};

}