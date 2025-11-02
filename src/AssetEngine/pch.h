#ifndef PCH_H
#define PCH_H

// ----------------------------------------------------------------------------
// Defines
// ----------------------------------------------------------------------------
#define MAX_BUFFER_SIZE 1024        
#define MAX_FILENAME_LENGTH 256     

// ----------------------------------------------------------------------------
// STL C++ Standard Library
// ----------------------------------------------------------------------------

#include <iostream>   
#include <iomanip>      
#include <cstdio>   
#include <string>      

#include <vector>    
#include <unordered_map>

#include <algorithm>   
#include <random>       
#include <chrono>      

#include <cstdint>     

#include <cassert>     

// ----------------------------------------------------------------------------
// Windows System API
// ----------------------------------------------------------------------------
#include <malloc.h>    
#include <sys/stat.h>  
#include <direct.h>     

// ----------------------------------------------------------------------------
// C++17 Filesystem
// ----------------------------------------------------------------------------
#include <filesystem>   

// ----------------------------------------------------------------------------
// Project Includes
// ----------------------------------------------------------------------------
#include "Types.hpp"       
#include "File.h"        
#include "Blob.h"          
#include "Memory.h"       
#include "SafeFormat.h"  
#include "Archive.h"      
#include "DebugUtils.hpp"   

#endif // PCH_H

