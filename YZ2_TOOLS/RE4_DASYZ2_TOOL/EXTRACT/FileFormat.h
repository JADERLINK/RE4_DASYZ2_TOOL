#pragma once
#ifndef __FileFormat
#define __FileFormat

using namespace System;

namespace EXTRACT
{
    enum class FileFormat
    {
        Null,
        DAT,
        MAP,
        UDAS,
        DAS,
        DRS,
        DECMP
    };
}

#endif