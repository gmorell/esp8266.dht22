#include <stdint.h>
#include <stdio.h>


#include "../tinytest/tinytest.h"
#include "../user/dns_util.h"

unsigned char dns_response[] = {
    0, 0, 0, 0, 0, 0, 255, 255, 255, 255, 255, 255, // 12 bytes of random stuff for fun..
    1, 'a', 7, 'e', 'x', 'a', 'm', 'p', 'l', 'e', 3, 'c', 'o', 'm', 0,
    1, 'a', 0xc0, 14, 3, 'o', 'r', 'g', 0,
    0xc0, 36, 0
};

// this one is too short.
unsigned char dns_smallresponse[] = {
    0, 0, 0, 0,
    4, 't', 'e', 's', 't', 3, 'c', 'o', 'm', 0
};

void test_dnsname_no_compression()
{
    // no dns compression, name compare simply and it works
    ASSERT_EQUALS(0, dns_compare_name("a.example.com", dns_response, sizeof(dns_response), 12));
    // no dns compression, name compare simply and it works. Trailing dot ok.
    ASSERT_EQUALS(0, dns_compare_name("a.example.com.", dns_response, sizeof(dns_response), 12));
    // pointer to the wrong part of the name, shouldn't compare successfully, we pointed at example.com
    ASSERT_EQUALS(1, dns_compare_name("a.example.com", dns_response, sizeof(dns_response), 14));
    // pointer to the start of the message, but is only the start of the hostname, so this should fail
    ASSERT_EQUALS(1, dns_compare_name("a.example", dns_response, sizeof(dns_response), 12));

    // one segment too long....
    ASSERT_EQUALS(1, dns_compare_name("a.examplea.com.", dns_response, sizeof(dns_response), 12));
    // one segment too short....
    ASSERT_EQUALS(1, dns_compare_name("a.exampl.com.", dns_response, sizeof(dns_response), 12));
}

void test_malicious_input_rejected() {
    // no infinite loop....
    ASSERT_EQUALS(1, dns_compare_name(".", dns_response, sizeof(dns_response), 36));
}

void test_bounds_truncate() {
    ASSERT_EQUALS(0, dns_compare_name("test.com", dns_smallresponse, sizeof(dns_smallresponse), 4));
    ASSERT_EQUALS(1, dns_compare_name("test.com", dns_smallresponse, sizeof(dns_smallresponse) - 2, 4));
    ASSERT_EQUALS(1, dns_compare_name("test.com", dns_smallresponse, sizeof(dns_smallresponse) - 1, 4));

}

void test_dnsname_with_compression()
{
    // pointer to the part of the message that used DNS compression properly. False positive in stock LWIP...
    ASSERT_EQUALS(0, dns_compare_name("a.example.org", dns_response, sizeof(dns_response), 27));

    // below here is where things failed before my substantive changes.  This code code from lwip had
    // anything that has dns compression matching everything as soon as the compression kicks in..
    // pointer to the compressed a.example.com in the message, but we aren't comparing that
    //FIXME re-enable test once working...
    ASSERT_EQUALS(1, dns_compare_name("a.bing.com", dns_response, sizeof(dns_response), 27));
}

/* test runner */
int main()
{
    RUN(test_dnsname_no_compression);
    RUN(test_dnsname_with_compression);
    RUN(test_malicious_input_rejected);
    RUN(test_bounds_truncate);
    return TEST_REPORT();
}