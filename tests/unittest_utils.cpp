#include <stdlib.h>
#include <string.h>

#include <gtest/gtest.h>

#include <NativeUtils.h>


TEST(Utils, MinMax)
{
	size_t mini, maxi;
	long minil, maxil;
	
	mini = 16;
	maxi = 129;
	minil = native_min( mini, maxi );
	maxil = native_max( mini, maxi );
	EXPECT_EQ( minil, mini);
	EXPECT_EQ( maxil, maxi);
	EXPECT_EQ( native_clamp( 1, mini, maxi), 16);
	EXPECT_EQ( native_clamp( 17, mini, maxi), 17);
	EXPECT_EQ( native_clamp( 177, mini, maxi), 129);
}
struct constStrMacro{
        const char * str;
        const int len;
};

TEST(Common, ConstStrLenMacro)
{
        {
#define MYSTR NULL
        struct constStrMacro test = { CONST_STR_LEN(MYSTR)};
        EXPECT_EQ(test.len, 0);
        EXPECT_TRUE(test.str == NULL);
#undef MYSTR
        }
        {
#define MYSTR "HELLO"
        struct constStrMacro test = { CONST_STR_LEN(MYSTR)};
        EXPECT_EQ(test.len, 5);
        EXPECT_TRUE(strcmp(test.str, MYSTR) == 0 );
#undef MYSTR
        }
        {
#define MYSTR "H\0ELLO"
        struct constStrMacro test = { CONST_STR_LEN(MYSTR)};
        EXPECT_EQ(test.len, 6);
        EXPECT_TRUE(strcmp(test.str, "H") == 0 );
#undef MYSTR
        }

}

