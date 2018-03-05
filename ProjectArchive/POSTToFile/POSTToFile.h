#pragma once

#include <iostream>
#include <fstream>
#include <string>
#include <sstream>
#include <io.h>
#include <fcntl.h>

typedef long long ll;
const ll HMOD = 12582917,
	HMUL = 256;
const std::string DEFAULT_DIR = "C:/Users/Yang Yan/Desktop/Main/Active/Emilia-ta/Root/";

ll GetHash (std::string &s);
ll FastModPow (ll base, ll mod, ll pow);
std::string DecodeURLPath (std::string urlpath);