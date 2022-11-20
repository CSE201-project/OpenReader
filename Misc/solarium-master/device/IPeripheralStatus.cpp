//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/09/02.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#include "IPeripheralStatus.h"

void IPeripheralStatus::setOnlineStatus()
{
    _currentStatus = Status::Online;
}

void IPeripheralStatus::setErrorStatus(const std::string &msg)
{
    _currentStatus = Status::Error;
    _errorMsg = msg;
}

const std::string &IPeripheralStatus::getErrorMessage() const
{
    return _errorMsg;
}

bool IPeripheralStatus::isOnline() const
{
    return _currentStatus == Status::Online;
}
