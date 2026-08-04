#pragma once

#include <cstdint>
#include <vector>
#ifndef STRATAKV_LINEAR_TABLE_H
#define STRATAKV_LINEAR_TABLE_H



// Define our Entry Struct
struct TableEntry {
  struct TableEntry* next;
  void* key;

  union {
      void* val;
      uint64_t u64;
      int64_t s64;
      double d;
  } v;

};



namespace stratakv {

    class LinearTable {

        LinearTable(size_t table_size) : table_size_(table_size) {
            table_ = (TableEntry**)malloc(sizeof(TableEntry*) * table_size);

            for (int i = 0; i < table_size; ++i) {
                table_[i] = (TableEntry*)malloc(sizeof(TableEntry));
            }

            for (int i = 0; i < table_size_; ++i) {
                if (i == table_size_ - 1) continue;
                table_[i]->next = table_[i+1];
            }
        }

        inline bool insert(void* key, void* val) {

            if (key == NULL || val == NULL) return false;

            size_t idx = (size_t)key % table_size_;

            table_[idx]->key = key;
            table_[idx]->v.val = val;


            return true;
        }

        inline bool erase(void* key) {
            if (key == NULL) return false;

            size_t idx = (size_t)key % table_size_;

            if (table_[idx] == NULL) return false;


        }




        private:
        TableEntry** table_;

        size_t table_size_;


    };



}



#endif
