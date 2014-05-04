#  ======================================================================================
#  File         : Makefile
#  Author       : Adrien Oliva 
#  Last Change  : 01/02/2013 | 18:53:35 PM | Wednesday,January
#  Description  : Build script for FlieIO library (Unit test part)
#  ======================================================================================
UNIT_TESTS	= avl_tests.x
TEST_DEPEND	= ../avl.h ../syslog.h minunit.h

include ../Makefile.global

# Link
avl_tests.x: 	avl_tests.o\
				avl_test01.o\
				avl_test02.o\
				avl_test03.o\
				avl_test04.o\
				avl_test05.o\
				avl_test06.o\
				avl_test07.o\
				avl_test08.o\
				avl_test09.o\
				avl_test10.o\
				avl_test11.o\
				../avl.o

# Dependencies
avl_tests.o: $(TEST_DEPEND)
avl_test01.o: $(TEST_DEPEND)
avl_test02.o: $(TEST_DEPEND)
avl_test03.o: $(TEST_DEPEND)
avl_test04.o: $(TEST_DEPEND)
avl_test05.o: $(TEST_DEPEND)
avl_test06.o: $(TEST_DEPEND)
avl_test07.o: $(TEST_DEPEND)
avl_test08.o: $(TEST_DEPEND)
avl_test09.o: $(TEST_DEPEND)
avl_test10.o: $(TEST_DEPEND)
avl_test11.o: $(TEST_DEPEND)
