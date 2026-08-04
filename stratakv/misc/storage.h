#pragma once

#ifndef STRATAKV_STORAGE_H
#define STATAKV_STORAGE_H

#include <striped_table.h>

#include <string>

using namespace std;


namespace stratakv {

class Storage {

    StripedTable<string,string>* kv_store;


    

};
}




#endif