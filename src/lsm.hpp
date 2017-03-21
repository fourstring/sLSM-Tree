//
//  lsm.hpp
//  lsm-tree
//
//  Created by Aron Szanto on 3/3/17.
//  Copyright © 2017 Aron Szanto. All rights reserved.
//

#ifndef LSM_H
#define LSM_H

#include "run.hpp"
#include "skipList.hpp"
#include "bloom.hpp"
#include "diskLevel.hpp"
#include <cstdio>
#include <cstdint>
#include <vector>

const double BF_FP_RATE = .05;


template <class K, class V>
class LSM {
    
    typedef SkipList<K,V> RunType;
    
    
    
public:
    vector<Run<K,V> *> C_0;
    
    vector<BloomFilter<K> *> filters;
    DiskLevel<K,V> disk_level;
    
    LSM<K,V>(size_t initialSize, size_t runSize, double sizeRatio):_sizeRatio(sizeRatio),_runSize(runSize),_initialSize(initialSize), _num_runs(initialSize / runSize), disk_level((runSize / sizeof(KVPair<K, V>)) * (initialSize / runSize) * sizeRatio, 1){
        _activeRun = 0;
        _eltsPerRun = _runSize / sizeof(KVPair<K, V>);
        _bfFalsePositiveRate = BF_FP_RATE;
        
        for (int i = 0; i < _num_runs; i++){
            RunType * run = new RunType(INT32_MIN,INT32_MAX);
            run->set_size(runSize);
            C_0.push_back(run);
            
            BloomFilter<K> * bf = new BloomFilter<K>(_eltsPerRun, _bfFalsePositiveRate);
            filters.push_back(bf);
        }
    }
    
    void insert_key(K key, V value) {
        
        if (C_0[_activeRun]->num_elements() >= _eltsPerRun)
            ++_activeRun;
        
        if (_activeRun > _num_runs){
            
            do_merge();
        }
        
        
        C_0[_activeRun]->insert_key(key,value);
        filters[_activeRun]->add(&key, sizeof(K));
    }
    
    V lookup(K key){
        // TODO keep track of min/max in runs?
//        cout << "looking for key " << key << endl;
        for (int i = _activeRun; i >= 0; --i){
//            cout << "... in run/filter " << i << endl;
            if (!filters[i]->mayContain(&key, sizeof(K)))
                continue;
            
            V lookupRes = C_0[i]->lookup(key);
            if (lookupRes)
                return lookupRes;
        }
        return NULL;
    }
    
    
    unsigned long long num_elements(){
        unsigned long long total = 0;
        for (int i = 0; i <= _activeRun; ++i)
            total += C_0[i]->num_elements();
        return total;
    }
    
    // how do you do disk stuff?
//private: // TODO MAKE PRIVATE
    double _sizeRatio;
    size_t _runSize;
    size_t _initialSize;
    unsigned int _activeRun;
    unsigned int _eltsPerRun;
    double _bfFalsePositiveRate;
    unsigned int _num_runs;
    double _frac_runs_merged;
    
    void do_merge(){
        int num_to_merge = _frac_runs_merged * _num_runs;
        vector<KVPair<K, V>> to_merge = vector<KVPair<K,V>>();
        to_merge.reserve(_eltsPerRun * num_to_merge);
        for (int i = 0; i < num_to_merge; i++){
            auto all = C_0[i]->get_all();
            to_merge.insert(to_merge.begin(), all.begin(), all.end());
            delete C_0[i];
        }
        disk_level.merge(&to_merge[0], to_merge.size());
        C_0.erase(C_0.begin(), C_0.begin() + num_to_merge);
        _activeRun -= num_to_merge;
        for (int i = _activeRun; i < _num_runs; i++){
            RunType * run = new RunType(INT32_MIN,INT32_MAX);
            run->set_size(_runSize);
            C_0.push_back(run);
            
            BloomFilter<K> * bf = new BloomFilter<K>(_eltsPerRun, _bfFalsePositiveRate);
            filters.push_back(bf);
        }
        

    }
    
};




#endif /* lsm_h */

