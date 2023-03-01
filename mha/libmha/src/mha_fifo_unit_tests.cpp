// This file is part of the HörTech Open Master Hearing Aid (openMHA)
// Copyright © 2006 2008 2011 2013 2015 2018 2020 HörTech gGmbH
// All rights reserved.

#include "mha.hh"
#include "mha_error.hh"
#include "mha_fifo.h"
#include <gtest/gtest.h>
#include <thread>

class Test_mha_fifo_lf_t : public ::testing::Test
{
public:
    /// stress test the fifo, try to produce synchronization errors
    /// @param fifo Instance of either mha_fifo_t or mha_fifo_lf_t
    /// @param expect_read_error We would expect the stress test to fail with
    ///                       mha_fifo_t (set this to true), and to not fail
    ///                       with mha_fifo_lf_t (set this to false).  The
    ///                       way the test is set up, it would produce a read
    ///                       error as a result of insufficient documentation.
    /// @param fail_when_expected_read_errors_do_not_occur
    ///                       Failures may be difficult to reproduce.  Indicate
    ///                       here if the test should fail when we expect a
    ///                       sync failure but no sync failure happens.
    ///                       Unexpected sync failures are always errors.
    void stress(mha_fifo_t<double> & fifo, bool expect_read_error,
                bool fail_when_expected_read_errors_do_not_occur);
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

TEST(Test_mha_fifo_t,overflow_protection)
{
    // Check that allocating a fifo with MAX_UINT elements is prohibited
    // We need to allocate capacity+1 elements. The number of allocated elements
    // should still be representable as an unsigned. MAX_UINT+1 elements
    // cannot be represented in an unsigned, therefore MAX_UINT must be
    // prohibited.

    const unsigned max_uint = std::numeric_limits<unsigned>::max();
    ASSERT_THROW(mha_fifo_t<char> fifo(max_uint), MHA_Error);
}
TEST(Test_mha_fifo_t,test_size_0)
{
    mha_fifo_t<mha_real_t> fifo(0);
    ASSERT_EQ(0U, fifo.get_fill_count());
    ASSERT_EQ(0U, fifo.get_available_space());
    
    mha_real_t data = 0;
    // Cant read from nor write to a Fifo of size 0
    ASSERT_THROW(fifo.write(&data, 1), MHA_Error);
    ASSERT_THROW(fifo.read(&data, 1), MHA_Error);
    
}

TEST(Test_mha_fifo_t,test_size_1)
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

TEST(Test_mha_fifo_t,test_available_space_20)
{
    mha_fifo_t<mha_real_t> * fifo = new mha_fifo_t<mha_real_t>(20);
    ASSERT_EQ(0U, fifo->get_fill_count());
    ASSERT_EQ(20U, fifo->get_available_space());
    delete fifo;
}

TEST(Test_mha_fifo_t,test_size_200)
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

TEST(Test_mha_fifo_t,test_placement_new)
{
    constexpr size_t L = 12U;
    struct custom_type_t {
        char s[L];
    };
    // Check that we can copy custom_type by value.
    custom_type_t c1{"Hello"}, c2{c1};
    EXPECT_STREQ("Hello", c1.s);
    EXPECT_STREQ("Hello", c2.s);

    mha_fifo_t<custom_type_t> fifo(200, c1);

    EXPECT_EQ(0U, fifo.get_fill_count());
    EXPECT_EQ(200U, fifo.get_available_space());

    std::vector<custom_type_t> in =
        {custom_type_t{"0"}, custom_type_t{"1"}, custom_type_t{"2"},
         custom_type_t{"3"}, custom_type_t{"4"}, custom_type_t{"5"},
         custom_type_t{"6"}, custom_type_t{"7"}, custom_type_t{"8"},
         custom_type_t{"9"}, custom_type_t{"10"},
         custom_type_t{"11"}, custom_type_t{"12"}};
    std::vector<custom_type_t> out = {14, custom_type_t{"-14"}};

    int read_counter = 0;
    while (atoi(in[0].s) < 13*14*2) {
        fifo.write(in.data(), 13);
        for (auto & c : in) {
            int str_sz=snprintf(c.s, L-1, "%hd", short(atoi(c.s) + 13));
            if(str_sz<0)
              throw MHA_Error(__FILE__, __LINE__,
                              "Implementation bug: Encoding error in snprintf");
            if ( str_sz > static_cast<int>(L-1) ) // Static cast uncritical: L is constexpr 12
              throw MHA_Error(__FILE__, __LINE__,
                              "Implementation bug: String of size %i does not "
                              "fit in buffer.",
                              str_sz);
        }
        if (fifo.get_fill_count() >= 14) {
            fifo.read(out.data(), 14);
            for (unsigned k = 0; k < 14; ++k)
                EXPECT_EQ(read_counter++, atoi(out[k].s));
        }
    }
    EXPECT_EQ(13*14*2, read_counter);
}

TEST_F(Test_mha_fifo_lf_t,mha_fifo_lf_t)
{
    mha_fifo_lf_t<double> fifo(1000000,-1.0);
    stress(fifo, false, true);
}

void Test_mha_fifo_lf_t::
stress(mha_fifo_t<double> & fifo, bool expect_read_error,
                bool fail_when_expected_read_errors_do_not_occur) {
    const unsigned capacity = fifo.get_max_fill_count();
    const unsigned transfer_size = capacity / 3U;
    std::vector<double> in(transfer_size);
    const unsigned transfers = 1000U;
    size_t reader_error_count = 0;
    double factor = 1e-8;
    auto reader_thread = std::thread
        ([&](){
             std::vector<double> out(capacity);
             for (unsigned received_samples = 0;
                  received_samples < transfer_size * transfers;) {
                 unsigned n = fifo.get_fill_count();
                 if (n) {
                     fifo.read(out.data(),n);
                     reader_error_count +=
                         (out[n-1] != factor*(received_samples+n-1));
                     received_samples += n;
                 }
             }
         });
    for (unsigned transfer = 0; transfer < transfers; ++transfer) {
        unsigned begin = transfer * transfer_size;
        for (unsigned k = 0; k < transfer_size; ++k) {
            in[k] = factor*(begin+k);
        }
        while(fifo.get_available_space()<transfer_size)
            ;
        fifo.write(in.data(),transfer_size);
    }
    reader_thread.join();
    if (expect_read_error) {
        printf("EXPECTED FAILURES, FAILURE COUNT=%zu\n", reader_error_count);
        if (fail_when_expected_read_errors_do_not_occur) {
            EXPECT_GT(reader_error_count, 0U);
        }
    } else {
        EXPECT_EQ(0U, reader_error_count);
    }
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

    std::thread thread([&](){
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
    );

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
    std::thread thread([&](){
            unsigned k;
            unsigned k_max =
            (DATA_SIZE / dblbuf->get_inner_size() * dblbuf->get_inner_size()
             + dblbuf->get_delay())
            / dblbuf->get_outer_size() * dblbuf->get_outer_size();
            std::vector<mha_real_t> temp(dblbuf->get_inner_size());
            for (k = 0;
                 (k + dblbuf->get_inner_size()) <= k_max;
                 k += dblbuf->get_inner_size()) {
                dblbuf->input(temp.data());
                dblbuf->output(temp.data());
            }
        }
    );

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

// Local Variables:
// compile-command: "make -C .. unit-tests"
// coding: utf-8-unix
// c-basic-offset: 4
// indent-tabs-mode: nil
// End:
