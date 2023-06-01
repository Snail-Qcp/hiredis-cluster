//
// Created by qicp on 2023/5/30.
//
#include <stdint.h>
#include <stdlib.h>
#include <assert.h>
#include <string>
#include <thread>
#include <iostream>
#include <list>
#include <vector>
#include "../hircluster.h"

std::vector<std::string> gListKeys;
int32_t gKeyNum = 10000;

#define ENTER_CLUSTER_TEST(test_name) \
    std::cout << "test " << test_name << " ... " << std::endl; \
    gen_keys();                                  \
    auto hc = redisClusterConnect("192.168.58.9:6380", HIRCLUSTER_FLAG_ROUTE_USE_SLOTS, "123456");\
    if (nullptr == hc)\
       return false;

#define LEAVE_CLUSTER_TEST(test_name) \
    redisClusterFree(hc);                                  \
    std::cout << "test " << test_name << " success!!!" << std::endl;

void gen_keys() {
    std::cout << "gen keys ..." << std::endl;
    gListKeys.clear();
    int32_t i = 0;
    int32_t baseV = 0;
    while (i++ < gKeyNum) {
        baseV += rand() % 1000 + 1;
        gListKeys.push_back(std::string("key").append(std::to_string(baseV)));
    }
    std::cout << "gen keys success!!!" << std::endl;
}

bool test_hash()
{
    return true;
}

bool test_mget_mset() {
    ENTER_CLUSTER_TEST("mget mset")

    std::vector<std::string> vecVals;
    vecVals.clear();

    //gen vals
    for (int32_t i = 0; i < gKeyNum; ++i) {
        vecVals.push_back(std::to_string(rand() + 1));
    }

    //test mset
    int32_t key = 0;
    int32_t step = 0;
    for (int32_t mset_start = 0; mset_start < gKeyNum; mset_start += step) {
        step = rand() % 10 + 1;
        if ((gKeyNum - mset_start) < step)
            step = gKeyNum - mset_start;

        //make mset command
        std::string set_cmd = "mset";
        for (int32_t i = 0; i < step; ++i) {
            set_cmd.append(" ").append(gListKeys[mset_start + i]).append(" ").append(vecVals[mset_start + i]);
        }

        //execute mset command
        auto set_reply = (redisReply*) redisClusterCommand(hc, set_cmd.c_str());
        assert(nullptr != set_reply);
        assert(hc->err == REDIS_OK);
        freeReplyObject(set_reply);
    }

    //test mget
    key = 0; step = 0;
    for (int32_t start = 0; start < gKeyNum; start += step) {
        step = rand() % 10 + 1;
        if ((gKeyNum - start) < step)
            step = gKeyNum - start;

        //make mget command
        std::string get_cmd = "mget";
        for (int32_t i = 0; i < step; ++i) {
            get_cmd.append(" ").append(gListKeys[start + i]);
        }

        auto getReply = (redisReply *) redisClusterCommand(hc, get_cmd.c_str());
        assert(nullptr != getReply);
        assert(hc->err == REDIS_OK);
        assert(getReply->elements == step);
        for (int32_t k = 0; k < step; ++k) {
            if (vecVals[start + k].compare(getReply->element[k]->str) != 0) {
                assert(false);
            }
        }
        freeReplyObject(getReply);
    }

    LEAVE_CLUSTER_TEST("mget mset")
    return true;
}

bool test_getset()
{
    ENTER_CLUSTER_TEST("getset")

    int32_t i = 0;
    for (auto key : gListKeys)
    {
        int32_t val = rand();
        auto setReply = (redisReply *)redisClusterCommand(hc, "set %s %d", key.c_str(), val);
        assert(nullptr != setReply);
        freeReplyObject(setReply);

        auto getReply = (redisReply *)redisClusterCommand(hc, "get %s", key.c_str());
        assert(nullptr != getReply);
        if (getReply->type == REDIS_REPLY_INTEGER) {
            assert(val == getReply->integer);
        } else if (getReply->type == REDIS_REPLY_STRING) {
            assert(std::to_string(val).compare(getReply->str) == 0);
        } else {
            assert(false);
        }
        freeReplyObject(getReply);
    }

    LEAVE_CLUSTER_TEST("getset")

    return true;
}

bool test_reconnect() {
    ENTER_CLUSTER_TEST("reconnect");

    for (auto &key : gListKeys) {
        auto reply = (redisReply*) redisClusterCommand(hc, "set %s %d", key.c_str(), 1);
        assert(nullptr != reply);
        freeReplyObject(reply);
    }

    std::cout << "please simulate disconnect redis server and press any key after restart redis server" << std::endl;
    char c;
    std::cin >> c;

    std::cout << "run get command after restart redis server..." << std::endl;

    redisClusterKeepAlive(hc);

    for (auto &key : gListKeys) {
        auto reply = (redisReply*) redisClusterCommand(hc, "get %s", key.c_str());
        assert(nullptr != reply);
        assert(hc->err == REDIS_OK);
        freeReplyObject(reply);
    }

    LEAVE_CLUSTER_TEST("reconnect");
    return true;
}

void init_test() {
    srand((unsigned int)time(NULL));

#ifdef WIN32
    WSADATA wsaData;
    WSAStartup(MAKEWORD(2, 2), &wsaData);
#endif
}

int main(int argc, char **argv)
{
    init_test();

    test_reconnect();
    assert(test_mget_mset());
    assert(test_getset());
//    assert(test_hash());
}
