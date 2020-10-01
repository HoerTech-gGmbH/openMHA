// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2006 2008 2011 2013 2015 2018 2020 HörTech gGmbH
// All rights reserved.

#include "mha.hh"
#include "mha_error.hh"
#include "mha_fifo.h"
#include "mha_plugin.hh" // test runtime config fifo
#include <gtest/gtest.h>
#include <thread>

class Test_mha_fifo_t : public ::testing::Test
{
    // This fixture has no data, setup nor teardown, and no friends.
};


class Test_mha_drifter_fifo_t : public ::testing::Test
{
public:
    mha_drifter_fifo_t<mha_real_t> * drifter_fifo;
    void SetUp();
    void TearDown();
    void test_constructor();
    void test_startup_write_first();
    void test_startup_read_first();
    void test_overrun();
    void test_underrun();
};

static constexpr unsigned DATA_SIZE = 1000U;

class Test_mha_fifo_lw_t: public ::testing::Test
{
public:
    size_t source_fragsize;
    size_t target_fragsize;
    mha_fifo_lw_t<mha_real_t> * fifo;
    mha_real_t source_data[DATA_SIZE];
    mha_real_t target_data[DATA_SIZE];

    void fill_source_data();
    void check_target_data();
    virtual void svc();
    std::thread thread;

    void SetUp();
    void TearDown();
    void testing_blocksize_adaptation(size_t source_fragsize,
                                      size_t target_fragsize,
                                      size_t fifo_size = 0);
};

class Test_mha_dblbuf_t: public testing::Test
{
public:
    mha_dblbuf_t<mha_fifo_lw_t<mha_real_t> > * dblbuf;
    mha_real_t source_data[DATA_SIZE];
    mha_real_t target_data[DATA_SIZE];

    void fill_source_data();
    virtual void svc();
    std::thread thread;

    void SetUp();
    void TearDown();
    void testing_blocksize_adaptation(unsigned source_fragsize,
                                      unsigned target_fragsize,
                                      unsigned delay);
};

class Test_mha_rt_fifo_t: public ::testing::Test
{
public:
    mha_rt_fifo_t<mha_real_t> * fifo;

    void SetUp();
    void TearDown();
    void test_initial_state();
    void test_push(void);
    void test_poll(void);
    void test_poll_1(void);
    void test_remove_abandonned(void);
    void test_remove_all(void);
};

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

TEST_F(Test_mha_fifo_t,test_size_0)
{
    mha_fifo_t<mha_real_t> fifo(0);
    ASSERT_EQ(0U, fifo.get_fill_count());
    ASSERT_EQ(0U, fifo.get_available_space());
    
    mha_real_t data = 0;
    // Cant read from nor write to a Fifo of size 0
    ASSERT_THROW(fifo.write(&data, 1), MHA_Error);
    ASSERT_THROW(fifo.read(&data, 1), MHA_Error);
    
}

TEST_F(Test_mha_fifo_t,test_size_1)
{
    mha_fifo_t<mha_real_t> fifo(1);
    mha_real_t data = 0.1;

    ASSERT_EQ(0U, fifo.get_fill_count());
    ASSERT_EQ(1U, fifo.get_available_space());

    // Can't read from a fresh (=empty) FIFO
    ASSERT_THROW(fifo.read(&data, 1), MHA_Error);
    ASSERT_EQ(0.1f, data);
    
    // Writing 0 instances succeeds and does not alter the FIFO
    ASSERT_NO_THROW(fifo.write(&data, 0));
    ASSERT_EQ(0U, fifo.get_fill_count());
    ASSERT_EQ(1U, fifo.get_available_space());

    // Writing 2 instances fails and does not alter FIFO
    ASSERT_THROW(fifo.write(&data,2), MHA_Error);
    ASSERT_EQ(0U, fifo.get_fill_count());
    ASSERT_EQ(1U, fifo.get_available_space());

    // Writing one instance succeeds and fills the FIFO
    ASSERT_NO_THROW(fifo.write(&data, 1));
    ASSERT_EQ(1U, fifo.get_fill_count());
    ASSERT_EQ(0U, fifo.get_available_space());
    
    // Writing more instances fails since FIFO is full
    data = 0.0;
    ASSERT_THROW(fifo.write(&data,1), MHA_Error);

    // Reading 0 instances succeeds and does not alter the fifo nor the target
    fifo.read(&data, 0);
    ASSERT_EQ(1U, fifo.get_fill_count());
    ASSERT_EQ(0U, fifo.get_available_space());
    ASSERT_EQ(0.0f, data);

    // Reading 1 instance succeeds, empties the FIFO and writes target.
    fifo.read(&data, 1);
    ASSERT_EQ(0U, fifo.get_fill_count());
    ASSERT_EQ(1U, fifo.get_available_space());
    ASSERT_EQ(0.1f, data);

    // Reading more instances fails and does not alter target
    data = 0;
    ASSERT_THROW(fifo.read(&data,1), MHA_Error);
    ASSERT_EQ(0.0f, data);
}

TEST_F(Test_mha_fifo_t,test_available_space_20)
{
    mha_fifo_t<mha_real_t> * fifo = new mha_fifo_t<mha_real_t>(20);
    ASSERT_EQ(0U, fifo->get_fill_count());
    ASSERT_EQ(20U, fifo->get_available_space());
    delete fifo;
}

TEST_F(Test_mha_fifo_t,test_size_200)
{
    mha_fifo_t<mha_real_t> fifo(200);
    ASSERT_EQ(0U, fifo.get_fill_count());
    ASSERT_EQ(200U, fifo.get_available_space());
    
    mha_real_t in[13] = {0,1,2,3,4,5,6,7,8,9,10,11,12};
    mha_real_t out[14] = {-14};

    unsigned read_counter = 0;
    while (in[0] < 13*14*2) {
        fifo.write(in, 13);
        for (unsigned k = 0; k < 13; ++k) in[k] += 13;
        if (fifo.get_fill_count() >= 14) {
            fifo.read(out, 14);
            for (unsigned k = 0; k < 14; ++k)
                ASSERT_EQ(mha_real_t(read_counter++), out[k]);
        }
    }
    ASSERT_EQ(13U*14U*2U, read_counter);
}

void Test_mha_drifter_fifo_t::SetUp()
{
    drifter_fifo = nullptr;
    drifter_fifo = new mha_drifter_fifo_t<mha_real_t>(10,15,20);
}
void Test_mha_drifter_fifo_t::TearDown()
{
    delete drifter_fifo;
    drifter_fifo = nullptr;
}

void Test_mha_drifter_fifo_t::test_constructor()
{
    ASSERT_EQ(20U, drifter_fifo->get_max_fill_count());
    ASSERT_EQ(15U, drifter_fifo->get_des_fill_count());
    ASSERT_EQ(10U, drifter_fifo->get_min_fill_count());
    ASSERT_EQ(0U,
                         drifter_fifo->
                         mha_fifo_t<mha_real_t>::get_fill_count());
    ASSERT_EQ(20U,
                         drifter_fifo->
                         mha_fifo_t<mha_real_t>::get_available_space());
    ASSERT_EQ(15U,
                         drifter_fifo->get_fill_count());
    ASSERT_EQ(5U,
                         drifter_fifo->get_available_space());
    ASSERT_EQ(false, drifter_fifo->reader_started);
    ASSERT_EQ(false, drifter_fifo->writer_started);
    ASSERT_EQ(0U, drifter_fifo->reader_xruns_total);
    ASSERT_EQ(0U, drifter_fifo->writer_xruns_total);
    ASSERT_EQ(0U, drifter_fifo->reader_xruns_since_start);
    ASSERT_EQ(0U, drifter_fifo->writer_xruns_since_start);
    ASSERT_EQ(0U, drifter_fifo->reader_xruns_in_succession);
    ASSERT_EQ(0U, drifter_fifo->writer_xruns_in_succession);
    ASSERT_EQ(10U,
                         drifter_fifo->
                         maximum_reader_xruns_in_succession_before_stop);
    ASSERT_EQ(10U,
                         drifter_fifo->
                         maximum_writer_xruns_in_succession_before_stop);
    ASSERT_EQ(mha_real_t(0), drifter_fifo->null_data);
    ASSERT_EQ(15U, drifter_fifo->startup_zeros);
}
TEST_F(Test_mha_drifter_fifo_t,test_constructor) {test_constructor();}

void Test_mha_drifter_fifo_t::test_startup_write_first()
{
    mha_real_t data = 1;
    drifter_fifo->write(&data, 1);
    ASSERT_EQ(true, drifter_fifo->writer_started);
    ASSERT_EQ(false, drifter_fifo->reader_started);
    drifter_fifo->write(&++data, 1);
    ASSERT_EQ(15U, drifter_fifo->get_fill_count());
    
    drifter_fifo->read(&data, 1);
    ASSERT_EQ(mha_real_t(0), data);
    ASSERT_EQ(14U, drifter_fifo->get_fill_count());
    data = 3;
    drifter_fifo->write(&data,1);
    ASSERT_EQ(15U, drifter_fifo->get_fill_count());
    for (unsigned i = 0; i < 14; ++i) {
        drifter_fifo->read(&data, 1);
        ASSERT_EQ(mha_real_t(0), data);
        ASSERT_EQ(14U, drifter_fifo->get_fill_count());
        data = 4 + i;
        drifter_fifo->write(&data,1);
        ASSERT_EQ(0U, drifter_fifo->reader_xruns_total);
        ASSERT_EQ(0U, drifter_fifo->writer_xruns_total);
    }
    for (unsigned i = 0; i < 5; ++i) {
        drifter_fifo->read(&data, 1);
        ASSERT_EQ(mha_real_t(i+3), data);
        ASSERT_EQ(0U, drifter_fifo->reader_xruns_total);
        ASSERT_EQ(0U, drifter_fifo->writer_xruns_total);
    }
}
TEST_F(Test_mha_drifter_fifo_t,test_startup_write_first) {
    test_startup_write_first();
}

void Test_mha_drifter_fifo_t::test_startup_read_first()
{
    mha_real_t data = 1;
    drifter_fifo->read(&data, 1);
    ASSERT_EQ(mha_real_t(0), data);
    ASSERT_EQ(true, drifter_fifo->reader_started);
    ASSERT_EQ(false, drifter_fifo->writer_started);
    drifter_fifo->read(&++data, 1);
    ASSERT_EQ(15U, drifter_fifo->get_fill_count());
    
    data = 2;
    drifter_fifo->write(&data, 1);
    ASSERT_EQ(16U, drifter_fifo->get_fill_count());
    drifter_fifo->read(&data,1);
    ASSERT_EQ(15U, drifter_fifo->get_fill_count());
    for (unsigned i = 0; i < 14; ++i) {
        data = 3 + i;
        drifter_fifo->write(&data, 1);
        ASSERT_EQ(16U, drifter_fifo->get_fill_count());
        drifter_fifo->read(&data,1);
        ASSERT_EQ(mha_real_t(0), data);
        ASSERT_EQ(0U, drifter_fifo->writer_xruns_total);
        ASSERT_EQ(0U, drifter_fifo->reader_xruns_total);
    }
    for (unsigned i = 0; i < 5; ++i) {
        drifter_fifo->read(&data, 1);
        ASSERT_EQ(mha_real_t(i+2), data);
        ASSERT_EQ(0U, drifter_fifo->writer_xruns_total);
        ASSERT_EQ(0U, drifter_fifo->reader_xruns_total);
    }
}
TEST_F(Test_mha_drifter_fifo_t,test_startup_read_first) {
    test_startup_read_first();
}

void Test_mha_drifter_fifo_t::test_overrun()
{
    // Set up the fifo so that the first write will start the fifo.
    {
        mha_real_t data;
        drifter_fifo->read(&data, 0);
    }
    // Replace the zeros with "real" data
    for (unsigned i = 0; i < 15; ++i) {
        // use up zeros
        mha_real_t data = 1+i;
        drifter_fifo->write(&data, 1);
        drifter_fifo->read(&data, 1);
    }
    ASSERT_EQ(15U, drifter_fifo->get_fill_count());

    // Writing till max succeeds
    {
        mha_real_t data[5] = {16,17,18,19,20};
        drifter_fifo->write(data, 5);
        ASSERT_EQ(20U, drifter_fifo->get_fill_count());
        ASSERT_EQ(0U, drifter_fifo->writer_xruns_total);
    }
    // Next write triggers overrun
    {
        mha_real_t data = 21;
        drifter_fifo->write(&data, 1);
        ASSERT_EQ(1U, drifter_fifo->writer_xruns_total);
        ASSERT_EQ(1U, drifter_fifo->writer_xruns_since_start);
        ASSERT_EQ(1U, drifter_fifo->writer_xruns_in_succession);
    }
    // One read & write will reset the in_succession counter but not the others
    {
        mha_real_t data = 0;
        drifter_fifo->read(&data,1);
        ASSERT_EQ(1.0f, data);
        data = 22;
        drifter_fifo->write(&data,1);
        ASSERT_EQ(20U, drifter_fifo->get_fill_count());
        ASSERT_EQ(0U, drifter_fifo->writer_xruns_in_succession);
        ASSERT_EQ(1U, drifter_fifo->writer_xruns_total);
        ASSERT_EQ(1U, drifter_fifo->writer_xruns_since_start);
    }
    // 11 xruns in succession will set the fifo to stopped.
    {
        for (unsigned i = 0; i < 10; ++i) {
            mha_real_t data = 22+i;
            drifter_fifo->write(&data, 1);
            ASSERT_EQ(i+1U,
                                 drifter_fifo->writer_xruns_in_succession);
            ASSERT_EQ(i+2U,
                                 drifter_fifo->writer_xruns_total);
            ASSERT_EQ(i+2U,
                                 drifter_fifo->writer_xruns_since_start);
        }
        ASSERT_EQ(true, drifter_fifo->writer_started);
        ASSERT_EQ(true, drifter_fifo->reader_started);
        mha_real_t data = 32;
        drifter_fifo->write(&data, 1);
        ASSERT_EQ(false, drifter_fifo->writer_started);
        ASSERT_EQ(false, drifter_fifo->reader_started);
        ASSERT_EQ(12U, drifter_fifo->writer_xruns_total);
        ASSERT_EQ(12U, drifter_fifo->writer_xruns_since_start);
        ASSERT_EQ(11U, drifter_fifo->writer_xruns_in_succession);
    }

    // But there has been no underrun
    ASSERT_EQ(0U, drifter_fifo->reader_xruns_total);
    ASSERT_EQ(0U, drifter_fifo->reader_xruns_since_start);
    ASSERT_EQ(0U, drifter_fifo->reader_xruns_in_succession);

    // one write and read will start it all again, with previous data lost.
    {
        mha_real_t data = 33;
        drifter_fifo->write(&data, 1); // this gets lost
        drifter_fifo->read(&data, 1);
        ASSERT_EQ(true, drifter_fifo->writer_started);
        ASSERT_EQ(true, drifter_fifo->reader_started);
        ASSERT_EQ(0.0f, data); // One of the initial zeros
        ASSERT_EQ(14U, drifter_fifo->get_fill_count());
        ASSERT_EQ(12U, drifter_fifo->writer_xruns_total);
        ASSERT_EQ(0U, drifter_fifo->writer_xruns_since_start);
        ASSERT_EQ(0U, drifter_fifo->writer_xruns_in_succession);
    }    
}
TEST_F(Test_mha_drifter_fifo_t,test_overrun) {test_overrun();}

void Test_mha_drifter_fifo_t::test_underrun()
{
    // Set up the fifo so that the first write will start the fifo.
    {
        mha_real_t data;
        drifter_fifo->read(&data, 0);
    }
    // Replace the zeros with "real" data
    for (unsigned i = 0; i < 15; ++i) {
        // use up zeros
        mha_real_t data = 1+i;
        drifter_fifo->write(&data, 1);
        drifter_fifo->read(&data, 1);
    }
    ASSERT_EQ(15U, drifter_fifo->get_fill_count());

    // Reading till min fill count succeeds
    {
        mha_real_t data[5];
        drifter_fifo->read(data, 5);
        ASSERT_EQ(5.0f, data[4]);
        ASSERT_EQ(10U, drifter_fifo->get_fill_count());
        ASSERT_EQ(0U, drifter_fifo->reader_xruns_total);
    }
    // Next read triggers underrun
    {
        mha_real_t data;
        drifter_fifo->read(&data, 1);
        ASSERT_EQ(1U, drifter_fifo->reader_xruns_total);
        ASSERT_EQ(1U, drifter_fifo->reader_xruns_since_start);
        ASSERT_EQ(1U, drifter_fifo->reader_xruns_in_succession);
    }
    // One read & write will reset the in_succession counter but not the others
    {
        mha_real_t data = 16;
        drifter_fifo->write(&data,1);
        drifter_fifo->read(&data,1);
        ASSERT_EQ(6.0f, data);
        ASSERT_EQ(10U, drifter_fifo->get_fill_count());
        ASSERT_EQ(0U, drifter_fifo->reader_xruns_in_succession);
        ASSERT_EQ(1U, drifter_fifo->reader_xruns_total);
        ASSERT_EQ(1U, drifter_fifo->reader_xruns_since_start);
    }
    // 11 xruns in succession will set the fifo to stopped.
    {
        for (unsigned i = 0; i < 10; ++i) {
            mha_real_t data;
            drifter_fifo->read(&data, 1);
            ASSERT_EQ(i+1U,
                                 drifter_fifo->reader_xruns_in_succession);
            ASSERT_EQ(i+2U,
                                 drifter_fifo->reader_xruns_total);
            ASSERT_EQ(i+2U,
                                 drifter_fifo->reader_xruns_since_start);
        }
        ASSERT_EQ(true, drifter_fifo->reader_started);
        ASSERT_EQ(true, drifter_fifo->reader_started);
        mha_real_t data;
        drifter_fifo->read(&data, 1);
        ASSERT_EQ(false, drifter_fifo->reader_started);
        ASSERT_EQ(false, drifter_fifo->reader_started);
        ASSERT_EQ(12U, drifter_fifo->reader_xruns_total);
        ASSERT_EQ(12U, drifter_fifo->reader_xruns_since_start);
        ASSERT_EQ(11U, drifter_fifo->reader_xruns_in_succession);
    }
    // But there has been no overrun
    ASSERT_EQ(0U, drifter_fifo->writer_xruns_total);
    ASSERT_EQ(0U, drifter_fifo->writer_xruns_since_start);
    ASSERT_EQ(0U, drifter_fifo->writer_xruns_in_succession);

    // one write and read will start it all again, with previous data lost.
    {
        mha_real_t data = 33;
        drifter_fifo->write(&data, 1); // this gets lost
        drifter_fifo->read(&data, 1);
        ASSERT_EQ(true, drifter_fifo->writer_started);
        ASSERT_EQ(true, drifter_fifo->reader_started);
        ASSERT_EQ(0.0f, data); // One of the initial zeros
        ASSERT_EQ(14U, drifter_fifo->get_fill_count());
        ASSERT_EQ(12U, drifter_fifo->reader_xruns_total);
        ASSERT_EQ(0U, drifter_fifo->reader_xruns_since_start);
        ASSERT_EQ(0U, drifter_fifo->reader_xruns_in_succession);
    }    
}
TEST_F(Test_mha_drifter_fifo_t,test_underrun) {test_underrun();}

static inline size_t gcd(size_t a, size_t b)
{
    return (b == 0) ? a : gcd(b, a % b);
}

void Test_mha_fifo_lw_t::fill_source_data()
{
    for (unsigned i = 0; i < DATA_SIZE; ++i)
        source_data[i] = rand();
}

void Test_mha_fifo_lw_t::check_target_data()
{
    for (unsigned i = 0;
         i < (DATA_SIZE / source_fragsize * source_fragsize
              / target_fragsize * target_fragsize);
         ++i) {
        ASSERT_EQ(source_data[i], target_data[i]) << "Checking at index " << i;
    }
}

void Test_mha_fifo_lw_t::svc()
{
    unsigned k;
    unsigned k_max = DATA_SIZE 
        / source_fragsize * source_fragsize
        / target_fragsize * target_fragsize;
    for (k = 0;
         (k + target_fragsize) <= k_max;
         k += target_fragsize) {
        fifo->read(target_data + k, target_fragsize);
    }
}

void Test_mha_fifo_lw_t::testing_blocksize_adaptation(size_t source_fragsize,
                                                      size_t target_fragsize,
                                                      size_t fifo_size)
{
    this->source_fragsize = source_fragsize;
    this->target_fragsize = target_fragsize;
    size_t delay = target_fragsize - gcd(target_fragsize, source_fragsize);
    if (fifo_size == 0)
        fifo_size = delay + std::max(target_fragsize, source_fragsize);
    fifo = new mha_fifo_lw_t<mha_real_t>(fifo_size);
    fill_source_data();
    thread = std::thread(&Test_mha_fifo_lw_t::svc, this);
    size_t k = 0;
    while ((k + source_fragsize) <= DATA_SIZE) {
        fifo->write(source_data + k, source_fragsize);
        k += source_fragsize;
    }
    thread.join();
    check_target_data();
    delete fifo;
    fifo = 0;
}

TEST_F(Test_mha_fifo_lw_t,test_blocksize_adaptation)
{
    testing_blocksize_adaptation(1,10);
    testing_blocksize_adaptation(10,1);
    testing_blocksize_adaptation(8,12);
    testing_blocksize_adaptation(63,64);
}

void Test_mha_fifo_lw_t::SetUp()
{
    target_fragsize = 0;
    fifo = 0;
}

void Test_mha_fifo_lw_t::TearDown()
{
    delete fifo;
}

void Test_mha_dblbuf_t::fill_source_data()
{
    for (unsigned i = 0; i < DATA_SIZE; ++i)
        source_data[i] = rand();
}

void Test_mha_dblbuf_t::svc()
{
    unsigned k;
    unsigned k_max = (DATA_SIZE
                      / dblbuf->get_inner_size() * dblbuf->get_inner_size()
                      + dblbuf->get_delay())
        / dblbuf->get_outer_size() * dblbuf->get_outer_size();
    mha_real_t * temp = new mha_real_t[dblbuf->get_inner_size()];
    for (k = 0;
         (k + dblbuf->get_inner_size()) <= k_max;
         k += dblbuf->get_inner_size()) {
        //printf("Have read %u\n", k); fflush(stdout);
        dblbuf->input(temp);
        dblbuf->output(temp);
    }
    delete [] temp;
}

void Test_mha_dblbuf_t::testing_blocksize_adaptation(unsigned source_fragsize,
                                                     unsigned target_fragsize,
                                                     unsigned delay)
{
    dblbuf = new mha_dblbuf_t<mha_fifo_lw_t<mha_real_t> >(source_fragsize,
                                                          target_fragsize,
                                                          delay,
                                                          1,
                                                          1,
                                                          0);
    ASSERT_EQ(source_fragsize, dblbuf->get_outer_size());
    ASSERT_EQ(target_fragsize, dblbuf->get_inner_size());
    ASSERT_EQ(delay, dblbuf->get_delay());
    ASSERT_EQ(std::max(source_fragsize, target_fragsize) + delay,
                         dblbuf->get_fifo_size());
    ASSERT_EQ(delay, dblbuf->get_input_fifo_fill_count());
    ASSERT_EQ(dblbuf->get_fifo_size() - delay, 
                         dblbuf->get_input_fifo_space());
    ASSERT_EQ(0U, dblbuf->get_output_fifo_fill_count());
    ASSERT_EQ(dblbuf->get_fifo_size(),
                         dblbuf->get_output_fifo_space());
    fill_source_data();
    thread = std::thread(&Test_mha_dblbuf_t::svc, this);
    unsigned k = 0;
    unsigned k_max = (DATA_SIZE / target_fragsize * target_fragsize)
        / source_fragsize * source_fragsize;
    while ((k + source_fragsize) <= k_max) {
        dblbuf->process(source_data + k, target_data + k, source_fragsize);
        k += source_fragsize;
    }
    thread.join();
    delete dblbuf;
    dblbuf = 0;
}

TEST_F(Test_mha_dblbuf_t,test_blocksize_adaptation)
{
    testing_blocksize_adaptation(1,10,9);
    testing_blocksize_adaptation(10,1,0);
    testing_blocksize_adaptation(8,12,8);
    testing_blocksize_adaptation(63,64,63);
}

void Test_mha_dblbuf_t::SetUp()
{
    dblbuf = 0;
}

void Test_mha_dblbuf_t::TearDown()
{
    delete dblbuf;
    dblbuf = 0;
}

void Test_mha_rt_fifo_t::SetUp()
{
    fifo = new mha_rt_fifo_t<float>;
}
void Test_mha_rt_fifo_t::TearDown()
{
    delete fifo; fifo = 0;
}
void Test_mha_rt_fifo_t::test_initial_state()
{
    ASSERT_TRUE(fifo->root == 0);
    ASSERT_TRUE(fifo->current == 0);
    ASSERT_EQ((float*)0, fifo->poll()); 
}
TEST_F(Test_mha_rt_fifo_t,test_initial_state) {test_initial_state();}
void Test_mha_rt_fifo_t::test_push(void)
{
    fifo->push(new float(0));
    ASSERT_TRUE(fifo->root != 0);
    ASSERT_TRUE(fifo->current == 0);
    ASSERT_TRUE(! fifo->root->abandonned);
    ASSERT_TRUE(fifo->root->next == 0);
    ASSERT_EQ(0.0f, *fifo->poll());
    fifo->push(new float(1));
    fifo->poll();
    ASSERT_EQ(0.0f, *fifo->root->data);
    fifo->push(new float(2));
    ASSERT_EQ(1.0f, *fifo->root->data);
}
TEST_F(Test_mha_rt_fifo_t,test_push) {test_push();}
void Test_mha_rt_fifo_t::test_poll(void)
{
    fifo->push(new float(0));
    fifo->push(new float(1));
    ASSERT_EQ(1.0F, *fifo->poll());
    ASSERT_TRUE(fifo->root->abandonned);
    ASSERT_EQ(1.0F, *fifo->poll());
}
TEST_F(Test_mha_rt_fifo_t,test_poll) {test_poll();}
void Test_mha_rt_fifo_t::test_poll_1(void)
{
    fifo->push(new float(0));
    fifo->push(new float(1));
    ASSERT_EQ(0.0F, *fifo->poll_1());
    ASSERT_TRUE(! fifo->root->abandonned);
    ASSERT_EQ(1.0F, *fifo->poll_1());
    ASSERT_TRUE(fifo->root->abandonned);
    ASSERT_EQ(1.0F, *fifo->poll_1());
}
TEST_F(Test_mha_rt_fifo_t,test_poll_1) {test_poll_1();}
void Test_mha_rt_fifo_t::test_remove_abandonned(void)
{
    fifo->push(new float(0));
    fifo->push(new float(1));
    fifo->poll();
    ASSERT_TRUE(fifo->root != fifo->current);
    fifo->remove_abandonned();
    ASSERT_EQ(fifo->root, fifo->current);
}
TEST_F(Test_mha_rt_fifo_t,test_remove_abandonned) {test_remove_abandonned();}
void Test_mha_rt_fifo_t::test_remove_all(void)
{
    fifo->push(new float(0));
    fifo->poll();
    fifo->remove_all();
    ASSERT_TRUE(fifo->root == 0);
    ASSERT_TRUE(fifo->current == 0);
}
TEST_F(Test_mha_rt_fifo_t,test_remove_all) {test_remove_all();}

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
    ASSERT_NO_THROW(t.push_config(c));
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
