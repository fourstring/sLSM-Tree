//
//  diskLevel.hpp
//  lsm-tree
//
//  Created by Aron Szanto on 3/20/17.
//  Copyright © 2017 Aron Szanto. All rights reserved.
//

#ifndef diskLevel_h
#define diskLevel_h
#include <vector>
#include <cstdint>
#include <string>
#include "run.hpp"
#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/mman.h>
#include <cassert>
#include <algorithm>
#include "imsort.hpp"

#define PAGESIZE 4096

using namespace std;

template <class K, class V>
class DiskLevel {
public:
    typedef KVPair<K,V> KVPair_t;
    KVPair_t *map;
    int fd;

    DiskLevel<K,V> (unsigned long long capacity, int level):_capacity(capacity),_numElts(0),_level(level), _iMaxFP(0) {
        
        _filename = ("C_" + to_string(level) + ".txt").c_str();
        
        size_t filesize = capacity * sizeof(KVPair_t);

        long result;
        
        fd = open(_filename, O_RDWR | O_CREAT | O_TRUNC, (mode_t) 0600);
        if (fd == -1) {
            perror("Error opening file for writing");
            exit(EXIT_FAILURE);
        }
        
        /* Stretch the file size to the size of the (mmapped) array of KVPairs
         */
        result = lseek(fd, filesize - 1, SEEK_SET);
        if (result == -1) {
            close(fd);
            perror("Error calling lseek() to 'stretch' the file");
            exit(EXIT_FAILURE);
        }
        
        
        result = write(fd, "", 1);
        if (result != 1) {
            close(fd);
            perror("Error writing last byte of the file");
            exit(EXIT_FAILURE);
        }
        
        
        map = (KVPair<K, V>*) mmap(0, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (map == MAP_FAILED) {
            close(fd);
            perror("Error mmapping the file");
            exit(EXIT_FAILURE);
        }

        
        
    }
    ~DiskLevel<K,V>(){
        doUnmap();
    }
    
    void merge(KVPair_t *run, unsigned long long len) {
        while (len + _numElts > _capacity){
            // TODO: FOR NOW: JUST INCREASE CAPACITY BY DOUBLING
            doubleSize();
        }

        memcpy(map + _numElts, run, len * sizeof(KVPair_t));
        inplace_merge(map, map + _numElts, map + _numElts + len);
        _numElts += len;
        
        // redo fence pointers
        _fencePointers.resize(0);
        _iMaxFP = 0;
        for (int j = 0; j * PAGESIZE < _numElts; j++) {
            _fencePointers.push_back(map[j * PAGESIZE].key);
            _iMaxFP++;
        }
        
        
        
        
    }
    
private:
    unsigned long long _capacity;
    unsigned long long _numElts;
    const char  *_filename;
    int _level;
    vector<K> _fencePointers;
    unsigned _iMaxFP;
    
    void doMap(){
        
        size_t filesize = _capacity * sizeof(KVPair_t);
        
        fd = open(_filename, O_RDWR | O_CREAT | O_TRUNC, (mode_t) 0600);
        if (fd == -1) {
            perror("Error opening file for writing");
            exit(EXIT_FAILURE);
        }
        
    
        map = (KVPair<K, V>*) mmap(0, filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (map == MAP_FAILED) {
            close(fd);
            perror("Error mmapping the file");
            exit(EXIT_FAILURE);
        }
    }
    
    void doUnmap(){
        size_t filesize = _capacity * sizeof(KVPair_t);

        if (munmap(map, filesize) == -1) {
            perror("Error un-mmapping the file");
        }
        
        close(fd);
        fd = -5;
    }
    
    void doubleSize(){
        unsigned long long new_capacity = _capacity * 2;
        
        size_t new_filesize = new_capacity * sizeof(KVPair_t);
        int result = lseek(fd, new_filesize - 1, SEEK_SET);
        if (result == -1) {
            close(fd);
            perror("Error calling lseek() to 'stretch' the file");
            exit(EXIT_FAILURE);
        }
        
        result = write(fd, "", 1);
        if (result != 1) {
            close(fd);
            perror("Error writing last byte of the file");
            exit(EXIT_FAILURE);
        }
        
        map = (KVPair<K, V>*) mmap(0, new_filesize, PROT_READ | PROT_WRITE, MAP_SHARED, fd, 0);
        if (map == MAP_FAILED) {
            close(fd);
            perror("Error mmapping the file");
            exit(EXIT_FAILURE);
        }
        
        _capacity = new_capacity;
    }
    
    
    
    
};
#endif /* diskLevel_h */
