//
//  main.cpp
//  lsm-tree
//
//  Created by Aron Szanto on 3/2/17.
//  Copyright © 2017 Aron Szanto. All rights reserved.
//

#include <stdio.h>
#include <cstdio>
#include <ctime>
#include <iostream>
#include <math.h>
#include <random>
#include <algorithm>
#include "skipList.hpp"
#include "bloom.hpp"
#include "lsm.hpp"


using namespace std;


void bloomFilterTest(){
    std::random_device                  rand_dev;
    std::mt19937                        generator(rand_dev());
    std::uniform_int_distribution<int>  distribution(INT32_MIN, INT32_MAX);
  
    const int num_inserts = 10;
    double fprate = .1;
    BloomFilter<int32_t> bf = BloomFilter<int32_t>(num_inserts, fprate);
    
    std::vector<int> to_insert;
    for (int i = 0; i < num_inserts; i++) {
        int insert = distribution(generator);
        to_insert.push_back(insert);
    }
    std::clock_t    start_insert;
    std::cout << "Starting inserts" << std::endl;
    start_insert = std::clock();
    for (int i = 0; i < num_inserts; i++) {
        bf.add(&i, sizeof(i));
    }
    
    double total_insert = (std::clock() - start_insert) / (double)(CLOCKS_PER_SEC);
    
    std::cout << "Time: " << total_insert << " s" << std::endl;
    std::cout << "Inserts per second: " << (int) num_inserts / total_insert << " s" << std::endl;
    int fp = 0;
    for (int i = num_inserts; i < 2 * num_inserts; i++) {
        bool lookup = bf.mayContain(&i, sizeof(i));
        if (lookup){
            // cout << i << " found but didn't exist" << endl;
            fp++;
        }
    }
    cout << fp << endl;
    cout << "FP rate: " << ((double) fp / double(num_inserts)) << endl;

    
    
    
    
    
}
void insertLookupTest(){
    std::random_device                  rand_dev;
    std::mt19937                        generator(rand_dev());
    std::uniform_int_distribution<int>  distribution(INT32_MIN, INT32_MAX);
    
    
    const int num_inserts = 1000000;
    const int max_levels = 16;
    const int num_runs = 100;
    const int buffer_capacity = 2000 * num_runs;
    const double bf_fp = .01;
    const int pageSize = 10000;
    const int disk_run_level = 10;
    LSM<int32_t, int32_t> lsmTree = LSM<int32_t, int32_t>(buffer_capacity, num_runs, 2,.5, bf_fp, pageSize, disk_run_level);
    
    std::vector<int> to_insert;
    for (int i = 0; i < num_inserts; i++) {
//        int insert = distribution(generator);
        to_insert.push_back(i);
    }
//    shuffle(to_insert.begin(), to_insert.end(), generator);

    std::clock_t    start_insert;
    std::cout << "Starting inserts" << std::endl;
    start_insert = std::clock();
    for (int i = 0; i < num_inserts; i++) {
        if ( i % 10000 == 0 ) cout << "insert " << i << endl;
        lsmTree.insert_key(to_insert[i],i);
    }
    
    double total_insert = (std::clock() - start_insert) / (double)(CLOCKS_PER_SEC);
    
    std::cout << "Time: " << total_insert << " s" << std::endl;
    std::cout << "Inserts per second: " << (int) num_inserts / total_insert << " s" << std::endl;
    
    std::clock_t    start_lookup;
    std::cout << "Starting lookups" << std::endl;
    start_lookup = std::clock();

    for (int i = 0 ; i < num_inserts; i++) {
        if ( i % 10000 == 0 ) cout << "lookup " << i << endl;

        int lookup = lsmTree.lookup(to_insert[i]);
        if (lookup != i)
            cout << "LOOKUP TEST FAILED ON ITERATION " << i << ". Got " << lookup << " but was expecting " << i << ".\n";
    }
    double total_lookup = (std::clock() - start_lookup) / (double)(CLOCKS_PER_SEC);

    std::cout << "Time: " << total_lookup << " s" << std::endl;
    std::cout << "Lookups per second: " << (int) num_inserts / total_lookup << " s" << std::endl;
}
void runInOrderTest() {
    const int num_inserts = 1000000;
    const int max_levels = 16;
    const int num_runs = 16;
    const int buffer_capacity = 1000000;
    const double bf_fp = .2;
    const int pageSize = 4096;
    const int disk_run_level = 10;
    LSM<int32_t, int32_t> lsmTree = LSM<int32_t, int32_t>(buffer_capacity, num_runs, 2,.5, bf_fp, pageSize, disk_run_level);
    

    std::vector<int> to_insert;
    for (int i = 0; i < num_inserts; i++) {
        to_insert.push_back(100 * i);
    }
    std::clock_t    start_insert;
    std::cout << "Starting inserts" << std::endl;
    start_insert = std::clock();
    for (int i = 0; i < num_inserts; i++) {
        lsmTree.insert_key(to_insert[i], i);
    }
    
    for (int i = 0; i < num_runs; i++){
        cout << "on run " << i << endl;
        auto all = lsmTree.C_0[i]->get_all();
        
        for (int j = 0; j < lsmTree._eltsPerRun; j++){
            
            auto kv = all[j];
            cout << "K: " << kv.key << ", " << "V: " << kv.value << endl;
        }
    }
}

void diskLevelTest(){
    const int num_inserts = 10000000;
    const int max_levels = 16;
    const int num_runs = 10;
    const int buffer_capacity = 10000;
    const double bf_fp = .0001;
    const int pageSize = 4096;
    const int disk_run_level = 10;
    LSM<int32_t, int32_t> lsmTree = LSM<int32_t, int32_t>(buffer_capacity, num_runs, 2,.5, bf_fp, pageSize, disk_run_level);
    

    std::vector<int> to_insert;
    for (int i = 0; i < num_inserts; i++) {
        to_insert.push_back(i);
    }
    std::clock_t    start_insert;
    std::cout << "Starting inserts" << std::endl;
    start_insert = std::clock();
    for (int i = 0; i < num_inserts; i++) {
        lsmTree.insert_key(to_insert[i], i);
    }
    
    vector<KVPair<int32_t, int32_t>> all = lsmTree.C_0[0]->get_all();
    int capacity = num_inserts * 2;
    int numElts = all.size();
    int level = 1;
    auto disklevel = DiskLevel<int32_t,int32_t>(capacity, 4096, level);
    
}
void customTest(const int num_inserts, const int num_runs, const int buffer_capacity,  const double bf_fp, const double merge_frac, const int pageSize, const int diskRunsPerLevel){
    std::random_device                  rand_dev;
    std::mt19937                        generator(rand_dev());
    std::uniform_int_distribution<int>  distribution(INT32_MIN, INT32_MAX);
    
    
    LSM<int32_t, int32_t> lsmTree = LSM<int32_t, int32_t>(buffer_capacity, num_runs, 2,merge_frac, bf_fp, pageSize, diskRunsPerLevel);
    
    std::vector<int> to_insert;
    for (int i = 0; i < num_inserts; i++) {
        //        int insert = distribution(generator);
        to_insert.push_back(i);
    }
    shuffle(to_insert.begin(), to_insert.end(), generator);
    
    std::clock_t    start_insert;
//    std::cout << "Starting inserts" << std::endl;
    start_insert = std::clock();
    for (int i = 0; i < num_inserts; i++) {
//        if ( i % 10000 == 0 ) cout << "insert " << i << endl;
        lsmTree.insert_key(to_insert[i],i);
    }
    
    double total_insert = (std::clock() - start_insert) / (double)(CLOCKS_PER_SEC);
    
    
    std::clock_t    start_lookup;
    start_lookup = std::clock();
    
    for (int i = 0 ; i < num_inserts; i++) {
        if ( i % 10000 == 0 ) cout << "lookup " << i << endl;
        int lookup = lsmTree.lookup(to_insert[i]);
    }
    double total_lookup = (std::clock() - start_lookup) / (double)(CLOCKS_PER_SEC);
    
    double ipersec = num_inserts / total_insert;
    double lpersec = num_inserts / total_lookup;
    
    cout << num_inserts << "," << num_runs << "," << buffer_capacity << "," << bf_fp << "," << merge_frac << "," << pageSize << "," << ipersec << "," << lpersec <<  "," << total_insert << "," << total_lookup << endl;
}
void cartesianTest(){
    vector<int> numins = {100000, 1000000};
    vector<int> numruns = {25, 100, 500, 1000};
    vector<int> buffercaps = {10000, 100000};
    vector<double> bf_fp = {0.001, 0.1};
    vector<double> merge_frac = {.2, .5, .8};
    vector<int> pss = {100, 1000};
    vector<int> drpl = {5, 10, 15, 20, 50};
    
    for (int i = 0; i < numins.size(); i++)
        for(int n = 0; n < numruns.size(); n++)
            for(int b = 0; b < buffercaps.size();b++)
                for(int bf = 0; bf < bf_fp.size(); bf++)
                    for (int m = 0; m < merge_frac.size(); m++)
                        for (int p = 0; p < pss.size(); p++)
                            for (int d = 0; d < drpl.size(); d++)
                                customTest(numins[i], numruns[n], buffercaps[b], bf_fp[bf], merge_frac[m], pss[p], drpl[d]);
}
void bfPerfTest(){
    vector<double> numruns = {.00001, .000001};
    for (int i = 0; i < numruns.size(); i++)
        customTest(1000000,	100, 100000, numruns[i],	0.8,1000, 50);
}
void fencePointerTest(){

    const long num_inserts = 1000 * 1000 * 1;
    const int num_lookups = 10000000;
    const int blocks = 16;
    const long pageSize = 100;
    std::random_device                  rand_dev;
    std::mt19937                        generator(rand_dev());
    std::uniform_int_distribution<int>  distribution(0, (int) (num_inserts * 1.2));

    std::vector<KVPair<int32_t, int32_t>> to_insert;
    auto dl = DiskLevel<int32_t, int32_t>(num_inserts + 1000, pageSize, 2);

    cout << "reserving" << endl;
    to_insert.reserve(num_inserts);
    cout << "pushing" << endl;
    for (int b = 0; b < blocks; b++){
        for (int i = b * (num_inserts / blocks); i < (b + 1) * num_inserts / blocks; i++) {
            if (i % 1000000 == 0) cout << "insert " << i << endl;

            to_insert.push_back((KVPair<int32_t, int32_t>) {i, i});
        }
        dl.merge(&to_insert[0], num_inserts / blocks);
        to_insert.resize(0);
        
    }
    
    auto to_lookup = vector<int>();
    to_lookup.reserve(num_lookups);
    for (int i = 0; i< num_lookups; i++){
        to_lookup.push_back(distribution(generator));
    }
    cout << "lookups" << endl;
    std::clock_t    start_lookup;
    start_lookup = std::clock();
    
    for (int i = 0 ; i < num_lookups; i++) {
        if (i % 1000000 == 0) cout << "lookup " << i << endl;
        int lookup = dl.lookup(to_insert[to_lookup[i]].key);
    }
    double total_lookup = (std::clock() - start_lookup) / (double)(CLOCKS_PER_SEC);
    
    double lpersec = num_lookups / total_lookup;
    cout << num_inserts << "," << pageSize << "," << lpersec << "," << total_lookup << endl;

    
    
}
int main(){

    
//    fencePointerTest();
    insertLookupTest();

    
   

    
    


    return 0;
    
}
