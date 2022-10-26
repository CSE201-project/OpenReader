//
// Created by Mario Valdivia <mario@lumedix.com> on 2020/09/02.
// Copyright (c) 2020 LumediX. All rights reserved.
//

#ifndef SOLARIUM_IPERIPHERALSTATUS_H
#define SOLARIUM_IPERIPHERALSTATUS_H

#include <string>

class IPeripheralStatus {
public:
    enum class Status { Offline, Online, Error };

    Status getStatus() const;
    bool isOnline() const;
    const std::string & getErrorMessage() const;

protected:
    Status _currentStatus = Status::Offline;
    std::string _errorMsg;

    IPeripheralStatus() = default;
    void setOnlineStatus();
    void setErrorStatus(const std::string & msg);
};


#endif //SOLARIUM_IPERIPHERALSTATUS_H
