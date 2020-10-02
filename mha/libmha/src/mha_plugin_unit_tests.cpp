// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2006 2008 2011 2013 2015 2018 2020 HörTech gGmbH
// All rights reserved.

#include "mha_plugin.hh"
#include <gtest/gtest.h>
#include <thread>

class test_cfg_t {
public:
    int i;
    static int instances;
    explicit test_cfg_t(int n) : i(n) {++instances;}
    ~test_cfg_t() {--instances;}
};
int test_cfg_t::instances = 0;

class Test_mha_plugin_rtcfg_t : public ::testing::Test
{   
    MHAPlugin::config_t<test_cfg_t> t;
public:
    void SetUp() {test_cfg_t::instances = 0;}
    void test_initial_state();
    void test_push_config(void);
    void test_poll_config(void);
    void test_peek_config(void);
    void test_cleanup_unused_cfg(void);
    void test_remove_all_cfg(void);
};

void Test_mha_plugin_rtcfg_t::test_initial_state()
{
    ASSERT_TRUE(t.cfg == 0);
    ASSERT_TRUE(t.cfg_root != 0);
    ASSERT_TRUE(t.cfg_root.load()->data == 0);
    ASSERT_TRUE(t.cfg_root.load()->next == 0);
    ASSERT_EQ(false, t.cfg_root.load()->not_in_use);
    ASSERT_THROW(t.poll_config(), MHA_Error);
}
TEST_F(Test_mha_plugin_rtcfg_t,test_initial_state) {
    test_initial_state();
}
void Test_mha_plugin_rtcfg_t::test_push_config(void)
{
    // First push pushes new data behind NULL object
    test_cfg_t * c = new test_cfg_t(0);
    t.push_config(c);
    ASSERT_TRUE(t.cfg == 0);
    ASSERT_EQ(c, t.cfg_root.load()->next.load()->data);
    ASSERT_TRUE(t.cfg_root.load()->next.load()->next == 0);
    ASSERT_EQ(false, t.cfg_root.load()->next.load()->not_in_use);

    // Next push pushes new data behind previous object
    t.push_config(new test_cfg_t(1));
    ASSERT_TRUE(t.cfg_root.load()->next.load()->next != 0);
    ASSERT_EQ(1, t.cfg_root.load()->next.load()->next.load()->data->i);
    ASSERT_TRUE(t.cfg_root.load()->next.load()->next.load()->next == 0);
    ASSERT_EQ(false, t.cfg_root.load()->not_in_use);
    ASSERT_EQ(false, t.cfg_root.load()->next.load()->not_in_use);
    ASSERT_EQ(false, t.cfg_root.load()->next.load()->next.load()->not_in_use);

    // Push deletes no longer in use data
    ASSERT_TRUE(t.cfg_root.load()->data == 0);
    t.cfg_root.load()->not_in_use = true;
    t.cfg_root.load()->next.load()->not_in_use = true;
    ASSERT_EQ(2, test_cfg_t::instances);
    t.push_config(0);
    ASSERT_EQ(1, t.cfg_root.load()->data->i);
    ASSERT_EQ(1, test_cfg_t::instances);
}
TEST_F(Test_mha_plugin_rtcfg_t,test_push_config) {test_push_config();}
void Test_mha_plugin_rtcfg_t::test_poll_config(void)
{
    ASSERT_THROW(t.poll_config(), MHA_Error);
    t.push_config(new test_cfg_t(0));
    ASSERT_NO_THROW(t.poll_config());
    ASSERT_EQ(true, t.cfg_root.load()->not_in_use);
    ASSERT_TRUE(t.cfg != 0);
    ASSERT_EQ(0, t.cfg->i);
}
TEST_F(Test_mha_plugin_rtcfg_t,test_poll_config) {
    test_poll_config();
}
void Test_mha_plugin_rtcfg_t::test_peek_config(void)
{
    t.push_config(new test_cfg_t(0));
    ASSERT_NO_THROW(t.peek_config());
    ASSERT_EQ(false, t.cfg_root.load()->not_in_use);
    ASSERT_TRUE(t.peek_config() != 0);
    ASSERT_EQ(0, t.peek_config()->i);
}
TEST_F(Test_mha_plugin_rtcfg_t,test_peek_config) {test_peek_config();}
void Test_mha_plugin_rtcfg_t::test_cleanup_unused_cfg(void)
{
    t.push_config(new test_cfg_t(0));
    t.push_config(new test_cfg_t(1));
    t.push_config(new test_cfg_t(2));
    t.poll_config();
    ASSERT_EQ(3, test_cfg_t::instances);
    t.cleanup_unused_cfg();
    ASSERT_EQ(1, test_cfg_t::instances);
}
TEST_F(Test_mha_plugin_rtcfg_t,test_cleanup_unused_cfg) {
    test_cleanup_unused_cfg();
}
void Test_mha_plugin_rtcfg_t::test_remove_all_cfg(void)
{
    t.push_config(new test_cfg_t(0));
    t.push_config(new test_cfg_t(1));
    t.push_config(new test_cfg_t(2));
    t.poll_config();
    ASSERT_EQ(3, test_cfg_t::instances);
    t.remove_all_cfg();
    ASSERT_EQ(0, test_cfg_t::instances);
    ASSERT_EQ(nullptr, t.cfg_root);
}
TEST_F(Test_mha_plugin_rtcfg_t,test_remove_all_cfg) {test_remove_all_cfg();}

// Local Variables:
// compile-command: "make -C .. unit-tests"
// coding: utf-8-unix
// c-basic-offset: 4
// indent-tabs-mode: nil
// End:
