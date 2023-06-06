//
// Created by qicp on 2023/5/30.
//
#include <stdint.h>
#include <stdlib.h>
#include <string>
#include <cstring>
#include <iostream>
#include <list>
#include <vector>
#include <algorithm>
#include <thread>
#include "../hircluster.h"

//test config
#define DEF_TEST_CLUSTER_HOST "192.168.58.9:6380"
#define DEF_TEST_CLUSTER_AUTH "123456"

int32_t gKeyNum = 10000;

#define ASSERT_TRUE(val) \
    if (!(val)) {               \
        std::cout << hc->errstr << std::endl; \
        return;                          \
    }

#define ENTER_CLUSTER_TEST(test_name) \
    std::cout << "============== test " << test_name << " ==============" << std::endl; \
    std::vector<std::string> lstKeys;\
    auto _key = std::string(test_name); \
    gen_keys(_key, lstKeys);                                  \
    auto hc = redisClusterConnect(DEF_TEST_CLUSTER_HOST, HIRCLUSTER_FLAG_ROUTE_USE_SLOTS, DEF_TEST_CLUSTER_AUTH);\
    ASSERT_TRUE(nullptr != hc);

#define LEAVE_CLUSTER_TEST(test_name) \
    redisClusterFree(hc);                                  \
    std::cout << "test " << test_name << " success!!!" << std::endl;

#define ADD_TEST_CASE(v, func) v.emplace_back(std::thread(func))

void trim_space(std::string &prefix) {
    prefix.erase(std::remove_if(prefix.begin(), prefix.end(), isspace), prefix.end());
}

void gen_keys(std::string prefix, std::vector<std::string> &lstKeys) {
    std::cout << "gen keys ..." << std::endl;
    trim_space(prefix);
    lstKeys.clear();
    int32_t i = 0;
    int32_t baseV = 0;
    while (i++ < gKeyNum) {
        baseV += rand() % 1000 + 1;
        auto key = prefix.append(std::to_string(baseV));
        lstKeys.push_back(key);
    }
    std::cout << "gen keys success!!!" << std::endl;
}

void test_hash()
{
    ENTER_CLUSTER_TEST("hash");
    //test hmset
    std::cout << "test hmset..." << std::endl;
    for (auto key : lstKeys) {
        auto reply = (redisReply *) redisClusterCommand(hc, "hmset %s name hashtest age 10 sex 1", key.c_str());
        ASSERT_TRUE(nullptr != reply && hc->err == REDIS_OK);
        freeReplyObject(reply);
    }

    //test hmget
    std::cout << "test hmget..." << std::endl;
    for (auto key : lstKeys) {
        auto reply = (redisReply*) redisClusterCommand(hc, "hmget %s name age sex", key.c_str());
        ASSERT_TRUE(nullptr != reply && hc->err == REDIS_OK && reply->type == REDIS_REPLY_ARRAY && reply->elements == 3);
        ASSERT_TRUE(strcmp(reply->element[0]->str, "hashtest") == 0);
        ASSERT_TRUE(strcmp(reply->element[1]->str, "10") == 0);
        ASSERT_TRUE(strcmp(reply->element[2]->str, "1") == 0);
        freeReplyObject(reply);
    }

    //test hset
    std::cout << "test hset..." << std::endl;
    for (auto key : lstKeys) {
        auto reply = (redisReply*) redisClusterCommand(hc, "hset %s country china", key.c_str());
        ASSERT_TRUE(nullptr != reply && reply->type == REDIS_REPLY_INTEGER && reply->integer == 1);
        freeReplyObject(reply);
    }

    //test hget
    std::cout << "test hget..." << std::endl;
    for (auto key : lstKeys) {
        auto reply = (redisReply *) redisClusterCommand(hc, "hget %s name", key.c_str());
        ASSERT_TRUE(nullptr != reply && hc->err == REDIS_OK);
        ASSERT_TRUE(strcmp(reply->str, "hashtest") == 0);
        freeReplyObject(reply);

        reply = (redisReply *) redisClusterCommand(hc, "hget %s age", key.c_str());
        ASSERT_TRUE(nullptr != reply && hc->err == REDIS_OK);
        ASSERT_TRUE(strcmp(reply->str, "10") == 0);
        freeReplyObject(reply);

        reply = (redisReply *) redisClusterCommand(hc, "hget %s sex", key.c_str());
        ASSERT_TRUE(nullptr != reply && hc->err == REDIS_OK);
        ASSERT_TRUE(strcmp(reply->str, "1") == 0);
        freeReplyObject(reply);

        reply = (redisReply *) redisClusterCommand(hc, "hget %s country", key.c_str());
        ASSERT_TRUE(nullptr != reply && hc->err == REDIS_OK);
        ASSERT_TRUE(strcmp(reply->str, "china") == 0);
        freeReplyObject(reply);
    }

    //test hgetall
    std::cout << "test hgetall..." << std::endl;
    for (auto key : lstKeys) {
        auto reply = (redisReply*) redisClusterCommand(hc, "hgetall %s", key.c_str());
        ASSERT_TRUE(nullptr != reply && hc->err == REDIS_OK && reply->type == REDIS_REPLY_ARRAY && reply->elements == 8);
        for (int32_t i = 0; i < reply->elements; i += 2) {
            std::string key(reply->element[i]->str);
            std::string val(reply->element[i+1]->str);
            if (key.compare("name") == 0) {
                ASSERT_TRUE(val.compare("hashtest") == 0);
            } else if (key.compare("age") == 0) {
                ASSERT_TRUE(val.compare("10") == 0);
            } else if (key.compare("sex") == 0) {
                ASSERT_TRUE(val.compare("1") == 0);
            } else if (key.compare("country") == 0) {
                ASSERT_TRUE(val.compare("china") == 0);
            } else {
                ASSERT_TRUE(false);
            }
        }
        freeReplyObject(reply);
    }

    //test hexists
    std::cout << "test hexists..." << std::endl;
    for (auto key : lstKeys) {
        auto reply = (redisReply*) redisClusterCommand(hc, "hexists %s name", key.c_str());
        ASSERT_TRUE(reply != nullptr && reply->type == REDIS_REPLY_INTEGER && reply->integer == 1);
        freeReplyObject(reply);

        reply = (redisReply*) redisClusterCommand(hc, "hexists %s age", key.c_str());
        ASSERT_TRUE(reply != nullptr && reply->type == REDIS_REPLY_INTEGER && reply->integer == 1);
        freeReplyObject(reply);

        reply = (redisReply*) redisClusterCommand(hc, "hexists %s sex", key.c_str());
        ASSERT_TRUE(reply != nullptr && reply->type == REDIS_REPLY_INTEGER && reply->integer == 1);
        freeReplyObject(reply);

        reply = (redisReply*) redisClusterCommand(hc, "hexists %s country", key.c_str());
        ASSERT_TRUE(reply != nullptr && reply->type == REDIS_REPLY_INTEGER && reply->integer == 1);
        freeReplyObject(reply);
    }

    //test hdel
    std::cout << "test hdel..." << std::endl;
    for (auto key : lstKeys) {
        auto reply = (redisReply*) redisClusterCommand(hc, "hdel %s name", key.c_str());
        ASSERT_TRUE(reply != nullptr && reply->type == REDIS_REPLY_INTEGER && reply->integer == 1);
        freeReplyObject(reply);

        reply = (redisReply*) redisClusterCommand(hc, "hdel %s age sex country", key.c_str());
        ASSERT_TRUE(reply != nullptr && reply->type == REDIS_REPLY_INTEGER && reply->integer == 3);
        freeReplyObject(reply);

        reply = (redisReply*) redisClusterCommand(hc, "hexists %s name", key.c_str());
        ASSERT_TRUE(reply != nullptr && reply->type == REDIS_REPLY_INTEGER && reply->integer == 0);
        freeReplyObject(reply);

        reply = (redisReply*) redisClusterCommand(hc, "hexists %s age", key.c_str());
        ASSERT_TRUE(reply != nullptr && reply->type == REDIS_REPLY_INTEGER && reply->integer == 0);
        freeReplyObject(reply);

        reply = (redisReply*) redisClusterCommand(hc, "hexists %s sex", key.c_str());
        ASSERT_TRUE(reply != nullptr && reply->type == REDIS_REPLY_INTEGER && reply->integer == 0);
        freeReplyObject(reply);

        reply = (redisReply*) redisClusterCommand(hc, "hexists %s country", key.c_str());
        ASSERT_TRUE(reply != nullptr && reply->type == REDIS_REPLY_INTEGER && reply->integer == 0);
        freeReplyObject(reply);
    }

    LEAVE_CLUSTER_TEST("hash");
}

void test_mget_mset() {
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
            set_cmd.append(" ").append(lstKeys[mset_start + i]).append(" ").append(vecVals[mset_start + i]);
        }

        //execute mset command
        auto set_reply = (redisReply*) redisClusterCommand(hc, set_cmd.c_str());
        ASSERT_TRUE(nullptr != set_reply);
        ASSERT_TRUE(hc->err == REDIS_OK);
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
            get_cmd.append(" ").append(lstKeys[start + i]);
        }

        auto getReply = (redisReply *) redisClusterCommand(hc, get_cmd.c_str());
        ASSERT_TRUE(nullptr != getReply);
        ASSERT_TRUE(hc->err == REDIS_OK);
        ASSERT_TRUE(getReply->elements == step);
        for (int32_t k = 0; k < step; ++k) {
            if (vecVals[start + k].compare(getReply->element[k]->str) != 0) {
                ASSERT_TRUE(false);
            }
        }
        freeReplyObject(getReply);
    }

    LEAVE_CLUSTER_TEST("mget mset")
}

void test_getset()
{
    ENTER_CLUSTER_TEST("getset")

    int32_t i = 0;
    for (auto key : lstKeys)
    {
        int32_t val = rand();
        auto setReply = (redisReply *)redisClusterCommand(hc, "set %s %d", key.c_str(), val);
        ASSERT_TRUE(nullptr != setReply);
        freeReplyObject(setReply);

        auto getReply = (redisReply *)redisClusterCommand(hc, "get %s", key.c_str());
        ASSERT_TRUE(nullptr != getReply);
        if (getReply->type == REDIS_REPLY_INTEGER) {
            ASSERT_TRUE(val == getReply->integer);
        } else if (getReply->type == REDIS_REPLY_STRING) {
            ASSERT_TRUE(std::to_string(val).compare(getReply->str) == 0);
        } else {
            ASSERT_TRUE(false);
        }
        freeReplyObject(getReply);
    }

    LEAVE_CLUSTER_TEST("getset")
}

void test_reconnect() {
    ENTER_CLUSTER_TEST("reconnect");

    for (auto &key : lstKeys) {
        auto reply = (redisReply*) redisClusterCommand(hc, "set %s %d", key.c_str(), 1);
        ASSERT_TRUE(nullptr != reply);
        freeReplyObject(reply);
    }

    std::cout << "please simulate disconnect redis server and then press any key to continue" << std::endl;
    char c;
    std::cin >> c;

    std::cout << "run get command after restart redis server..." << std::endl;

    redisClusterPing(hc);

    for (auto &key : lstKeys) {
        auto reply = (redisReply*) redisClusterCommand(hc, "get %s", key.c_str());
        ASSERT_TRUE(nullptr != reply);
        ASSERT_TRUE(hc->err == REDIS_OK);
        freeReplyObject(reply);
    }

    LEAVE_CLUSTER_TEST("reconnect");
}

void test_exists_del() {
    ENTER_CLUSTER_TEST("exists and del");
    //set all keys
    std::cout << "write keys..." << std::endl;
    for (auto &key : lstKeys) {
        auto reply = (redisReply *)redisClusterCommand(hc, "set %s %d", key.c_str(), 1);
        ASSERT_TRUE(nullptr != reply && hc->err == REDIS_OK);
        freeReplyObject(reply);
    }

    //test exists
    std::cout << "test exists..." << std::endl;
    for (auto &key : lstKeys) {
        auto reply = (redisReply *) redisClusterCommand(hc, "exists %s", key.c_str());
        ASSERT_TRUE(nullptr != reply && reply->type == REDIS_REPLY_INTEGER && reply->integer==1);
        freeReplyObject(reply);
    }

    //test del
    std::cout << "test del..." << std::endl;
    for (auto &key : lstKeys) {
        auto reply = (redisReply *) redisClusterCommand(hc, "del %s", key.c_str());
        ASSERT_TRUE(nullptr != reply && reply->type == REDIS_REPLY_INTEGER && reply->integer==1);
        freeReplyObject(reply);
    }

    //retest exists
    std::cout << "retest exists..." << std::endl;
    for (auto &key : lstKeys) {
        auto reply = (redisReply *) redisClusterCommand(hc, "exists %s", key.c_str());
        ASSERT_TRUE(nullptr != reply && reply->type == REDIS_REPLY_INTEGER && reply->integer==0);
        freeReplyObject(reply);
    }

    LEAVE_CLUSTER_TEST("exists and del");
}

void test_sortedset() {
    ENTER_CLUSTER_TEST("sortedsest");
    std::cout << "test sorted set..." << std::endl;
    for (auto key : lstKeys) {
        auto field = "score";
        for (int32_t i = 0; i < 10; ++i) {
            auto v = rand();
            auto reply = (redisReply*) redisClusterCommand(hc, "zadd %s %d %s%d", key.c_str(), v, field, i);
            ASSERT_TRUE(reply != nullptr && reply->type == REDIS_REPLY_INTEGER);
            freeReplyObject(reply);
        }

        std::list<int32_t> lstValues;
        auto replyRevrange = (redisReply *) redisClusterCommand(hc, "zrevrange %s %d %d withscores", key.c_str(), 0, INT32_MAX);
        ASSERT_TRUE(replyRevrange != nullptr && replyRevrange->type == REDIS_REPLY_ARRAY);
        for (int32_t i = 0; i < replyRevrange->elements; i = i + 2) {
            lstValues.push_back(atoi(replyRevrange->element[i + 1]->str));
        }
        freeReplyObject(replyRevrange);
        ASSERT_TRUE(lstValues.size() == 10);
        ASSERT_TRUE(*lstValues.begin() >= *lstValues.rbegin());

        lstValues.clear();
        auto replyRange = (redisReply *) redisClusterCommand(hc, "zrange %s %d %d withscores", key.c_str(), 0, INT32_MAX);
        ASSERT_TRUE(replyRange != nullptr && replyRange->type == REDIS_REPLY_ARRAY);
        for (int32_t j = 0; j < replyRange->elements; j = j + 2) {
            lstValues.push_back(atoi(replyRange->element[j + 1]->str));
        }
        freeReplyObject(replyRange);
        ASSERT_TRUE(lstValues.size() == 10);
        ASSERT_TRUE(*lstValues.begin() <= *lstValues.rbegin());
    }
    LEAVE_CLUSTER_TEST("sortedset");
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

#if 0
    std::list<std::thread> lstThreads;
    ADD_TEST_CASE(lstThreads, test_mget_mset);
    ADD_TEST_CASE(lstThreads, test_getset);
    ADD_TEST_CASE(lstThreads, test_exists_del);
    ADD_TEST_CASE(lstThreads, test_hash);
    ADD_TEST_CASE(lstThreads, test_sortedset);

    for (auto &th : lstThreads) {
        th.join();
    }
#else
    test_mget_mset();
    test_getset();
    test_exists_del();
    test_hash();
    test_sortedset();
#endif

    test_reconnect();

    return  EXIT_SUCCESS;
}
