/* * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * *
 *                                                                                 *
 * ALPS Project: Algorithms and Libraries for Physics Simulations                  *
 *                                                                                 *
 * ALPS Libraries                                                                  *
 *                                                                                 *
 * Copyright (C) 2010 - 2012 by Lukas Gamper <gamperl@gmail.com>                   *
 *                                                                                 *
 * ALPS Project: https://alps.comp-phys.org/                                       *
 * SPDX-License-Identifier: MIT                                                    *
 *                                                                                 *
 * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * * */

#include <gtest/gtest.h>
#include <alps/testing/temporary_directory.hpp>

#include <iostream>
#include <alps/hdf5/archive.hpp>
#include <alps/hdf5/vector.hpp>
#include <alps/hdf5/complex.hpp>

#include <filesystem>

#include <vector>
#include <complex>

using namespace std;

TEST(Hdf5, Vecvecdbl)
{
    alps::testing::TemporaryDirectory temporary;
    if (std::filesystem::exists(std::filesystem::path((temporary.path() / "vvdbl.h5").string())))
        std::filesystem::remove(std::filesystem::path((temporary.path() / "vvdbl.h5").string()));
    {
        vector<vector<double> > v;
        for(int i = 0; i < 3; ++i)
            v.push_back(vector<double>(i+1, 2*i));
        alps::hdf5::archive ar((temporary.path() / "vvdbl.h5").string(), "w");
        ar["/spectrum/sectors/5/results/cdag-c/mean/value"] = v;

    }
	 {
        vector<vector<double> > v;
        alps::hdf5::archive ar((temporary.path() / "vvdbl.h5").string(), "r");
        ar["/spectrum/sectors/5/results/cdag-c/mean/value"] >> v;
        ASSERT_EQ(v.size(), 3u);
        for (int i = 0; i < 3; ++i) EXPECT_EQ(v[i], vector<double>(i + 1, 2 * i));
    }
    std::filesystem::remove(std::filesystem::path((temporary.path() / "vvdbl.h5").string()));
}

TEST(Hdf5, VecvecdblEmptyInner)
{
    alps::testing::TemporaryDirectory temporary;
    std::string const filename = (temporary.path() / "vvdbl_empty.h5").string();
    vector<vector<double> > const v(3);
    vector<vector<vector<double> > > const vvv(2, vector<vector<double> >(1));
    {
        alps::hdf5::archive ar(filename, "w");
        ar["/v"] << v;
        ar["/vvv"] << vvv;
    }
    {
        vector<vector<double> > w;
        vector<vector<vector<double> > > www;
        alps::hdf5::archive ar(filename, "r");
        ar["/v"] >> w;
        ar["/vvv"] >> www;
        EXPECT_EQ(w, v);
        EXPECT_EQ(www, vvv);
    }
}
