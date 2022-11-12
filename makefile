TARGET ?= inca_on_the_mysql
SRC_DIRS ?= ./src
MYSQL_CONCPP_DIR=./src/libraries/mysql-l

CXX=g++
CXXFLAGS= -g -std=c++11 -Wall -Ofast
LDLIBS = -lmysqlcppconn8-static -lssl -lcrypto -lpthread
LDFLAGS = -L/usr/lib -L${MYSQL_CONCPP_DIR}/lib64

SRCS := $(shell find $(SRC_DIRS) -name *.cpp -or -name *.c -or -name *.s)
OBJS := $(addsuffix .o,$(basename $(SRCS)))
DEPS := $(OBJS:.o=.d)

INC_DIRS := $(shell find $(SRC_DIRS) -type d)
INC_DIRS += ${SRC_DIRS}/include
INC_FLAGS := $(addprefix -I,$(INC_DIRS))

CPPFLAGS ?= -I $(MYSQL_CONCPP_DIR)/include $(INC_FLAGS) -MMD -MP

$(TARGET): $(OBJS)
		$(CXX) $(CXXFLAGS) $(CPPFLAGS) $(LDFLAGS) -o $@ $(LOADLIBES) $(OBJS) $(LDLIBS)

.PHONY: clean
	clean:
		$(RM) $(TARGET) $(OBJS) $(DEPS)

-include $(DEPS)