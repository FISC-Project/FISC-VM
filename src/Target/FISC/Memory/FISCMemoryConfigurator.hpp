/*                                                                 
                           __  __           _       _                                                  
                          |  \/  |         | |     | |     _                                           
                          | \  / | ___   __| |_   _| | ___(_)                                          
                          | |\/| |/ _ \ / _` | | | | |/ _ \                                            
                          | |  | | (_) | (_| | |_| | |  __/_                                           
                          |_|  |_|\___/ \__,_|\__,_|_|\___(_)                                          
  __  __                                    _____             __ _                       _             
 |  \/  |                                  / ____|           / _(_)                     | |            
 | \  / | ___ _ __ ___   ___  _ __ _   _  | |     ___  _ __ | |_ _  __ _ _   _ _ __ __ _| |_ ___  _ __ 
 | |\/| |/ _ | '_ ` _ \ / _ \| '__| | | | | |    / _ \| '_ \|  _| |/ _` | | | | '__/ _` | __/ _ \| '__|
 | |  | |  __| | | | | | (_) | |  | |_| | | |___| (_) | | | | | | | (_| | |_| | | | (_| | || (_) | |   
 |_|  |_|\___|_| |_| |_|\___/|_|   \__, |  \_____\___/|_| |_|_| |_|\__, |\__,_|_|  \__,_|\__\___/|_|   
                                    __/ |                           __/ |                              
                                   |___/                           |___/                               
*/

/*----------------------------------------------------
- FILE NAME: FISCMemoryConfigurator.cpp
- MODULE NAME: Memory Configurator
- PURPOSE: To define the structure of the Main Memory
- AUTHOR: MIGUEL SANTOS
------------------------------------------------------*/

#pragma once
#include <fvm/Pass.h>
#include <fvm/Utils/Cmdline.h>
#include <fvm/Utils/String.h>
#include <fvm/Utils/IO/File.h>
#include <fvm/Utils/ELFLoader.h>
#include <fstream>
#include <vector>
#include <bitset>
#include <stdint.h>

namespace FISC {

class MemoryConfigurator : public ConfigPass {
#pragma region REGION 1: THE MEMORY CONFIGURATION DATA
public:
    /* Pass properties */
    #define MEMORY_CONFIGURATOR_PRIORITY 2 /* The execution priority of this module */
    #define MEMORY_PROGRAM_FILE_IOS_MODE std::ios::in | std::ios::binary /* The mode the program file will be opened with */

    /* List of permissions for external Passes that want to use the resources of this Pass */
    #define WHITELIST_MEM_CONFIG {DECL_WHITELIST_ALL(MemoryModule)}

    /* Command line flags */
    #define MEMORY_FLAG_BOOT_SHORT 'b'
    #define MEMORY_FLAG_BOOT_LONG "boot"

    /* Implementation properties */
    #define MEMORY_WIDTH   8        /* The width of the memory */
    #define MEMORY_DEPTH   33554432 /* Size of memory in bytes */
    #define MEMORY_LOADLOC 0        /* Where to load the program on startup */
#pragma endregion

#pragma region REGION 2: THE MEMORY STRUCTURE DEFINITION (IMPL. SPECIFIC)
public:
    std::vector<std::bitset<MEMORY_WIDTH> > theMemory; /* The actual main memory */
    /* The following vector will hold the initial bootloader program in ELF file format, 
       which will be then copied into the main memory once parsed and relocated */
    std::vector<std::bitset<MEMORY_WIDTH> > theBootloaderMemory;
    uint64_t loadedProgramSize; /* Size of the loaded program */
    File programFile; /* The file being loaded into memory */
    /* DISCLAIMER: In the future, we might want to care about a singular
       file being loaded into memory as the absolute program being loaded. 
       In other words, the first program that is loaded might be responsible for
       loading other files, thus rendering this variable useless. */
    elfsection_list_t elfsection_list;
#pragma endregion

#pragma region REGION 3: THE MEMORY CONFIGURATION IMPLEMENTATION (IMPL SPECIFIC)
public:
    uint64_t getMemSize()
    {
        return theMemory.size();
    }

    uint64_t getProgSize()
    {
        return loadedProgramSize;
    }
#pragma endregion

#pragma region REGION 4: THE MEMORY CONFIGURATION IMPLEMENTATION (GENERIC VM FUNCTIONS)
public:
    MemoryConfigurator() : ConfigPass(MEMORY_CONFIGURATOR_PRIORITY),
        loadedProgramSize(0), theMemory(MEMORY_DEPTH, -1), programFile(NULLSTR, 0)
    {
        setWhitelist(WHITELIST_MEM_CONFIG);
    }
    
    enum PassRetcode init()
    {
        enum PassRetcode success = PASS_RET_ERR;

        /* Setup program file */
        if (cmdHasOpt(MEMORY_FLAG_BOOT_SHORT)) {
            std::pair<char, std::string> fileName = cmdQuery(MEMORY_FLAG_BOOT_SHORT);

            DEBUG(DINFO, "Program name: %s", fileName.second.c_str());
            /* Create file only if it exists */
            if (programFile.create(fileName.second, MEMORY_PROGRAM_FILE_IOS_MODE))
                success = PASS_RET_OK;
            else
                DEBUG(DERROR, "Could not load program '%s'!", fileName.second.c_str());
        } else {
            if (cmdHasOpt(MEMORY_FLAG_BOOT_LONG)) {
                std::pair<std::string, std::string> fileName = cmdQuery(MEMORY_FLAG_BOOT_LONG);
                /* Create file only if it exists */
                if (programFile.create(fileName.second, MEMORY_PROGRAM_FILE_IOS_MODE))
                    success = PASS_RET_OK;
                else
                    DEBUG(DERROR, "Could not load program '%s'!", fileName.second.c_str());
            }
            else {
                DEBUG(DERROR, "Expected a bootloader program with argument -b <filename> or --boot <filename>. There is nothing to do for the VM.");
            }
        }

        /* If the variable success is equal to PASS_RET_ERR,
           we must tell the top layer (VM) that this module
           will not be able to continue, thus forcing every other
           single module (on this target only) to cancel its current
           operations. */

        return success;
    }

    enum PassRetcode finit()
    {
        /* Nothing to do for now */
        return PASS_RET_OK;
    }

    enum PassRetcode run()
    {
        /* For now we don't have anything to configure.
           In the future, we might want to read and parse a config file (using init() function and not run())
           and select the desired configurations on the parsed results here.
           These config files could contain information about memory size (no need to hardcode them here),
           memory access speeds, restrictions, and just about any memory related property. */
        return PASS_RET_OK;
    }

    enum PassRetcode watchdog()
    {
        return PASS_RET_OK;
    }
#pragma endregion
};
}
/*
              ______           _          __   __  __           _       _                              
             |  ____|         | |        / _| |  \/  |         | |     | |     _                       
             | |__   _ __   __| |   ___ | |_  | \  / | ___   __| |_   _| | ___(_)                      
             |  __| | '_ \ / _` |  / _ \|  _| | |\/| |/ _ \ / _` | | | | |/ _ \                        
             | |____| | | | (_| | | (_) | |   | |  | | (_) | (_| | |_| | |  __/_                       
             |______|_| |_|\__,_|  \___/|_|   |_|  |_|\___/ \__,_|\__,_|_|\___(_)                      
  __  __                                    _____             __ _                       _             
 |  \/  |                                  / ____|           / _(_)                     | |            
 | \  / | ___ _ __ ___   ___  _ __ _   _  | |     ___  _ __ | |_ _  __ _ _   _ _ __ __ _| |_ ___  _ __ 
 | |\/| |/ _ | '_ ` _ \ / _ \| '__| | | | | |    / _ \| '_ \|  _| |/ _` | | | | '__/ _` | __/ _ \| '__|
 | |  | |  __| | | | | | (_) | |  | |_| | | |___| (_) | | | | | | | (_| | |_| | | | (_| | || (_) | |   
 |_|  |_|\___|_| |_| |_|\___/|_|   \__, |  \_____\___/|_| |_|_| |_|\__, |\__,_|_|  \__,_|\__\___/|_|   
                                    __/ |                           __/ |                              
                                   |___/                           |___/                               
*/