// Copyright (C) 2016 iNuron NV
//
// This file is part of Open vStorage Open Source Edition (OSE),
// as available from
//
//      http://www.openvstorage.org and
//      http://www.openvstorage.com.
//
// This file is free software; you can redistribute it and/or modify it
// under the terms of the GNU Affero General Public License v3 (GNU AGPLv3)
// as published by the Free Software Foundation, in version 3 as it comes in
// the LICENSE.txt file of the Open vStorage OSE distribution.
// Open vStorage is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY of any kind.

#include <gtest/gtest.h>
#include "../Time.h"

namespace youtilstest
{
//using namespace youtils;
class TimeTest : public testing::Test
{};


TEST_F(TimeTest, test1)
{
    std::string s(Time::getCurrentTimeAsString());

    boost::posix_time::ptime p(Time::timeFromString(s));
    (void)p;
}


}
// Local Variables: **
// mode: c++ **
// End: **
