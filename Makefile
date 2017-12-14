# PPS LP Makefile
# Szilagyi Istvan
# istvan.szilagyi.ext@eon.com
# 2016

# Valtozok:
FILES 		:=  ./main.cpp $(wildcard code/*.cpp)
ORACLE_INCLUDE	:= /u01/app/oracle/product/11.2.0/xe/rdbms/public/
LOCAL_INCLUDE	:= ./header/
LINKS		:= -lboost_iostreams -lboost_system -locci -lclntsh -lnnz11 -ldl -lz -lbz2 -pthread -lssl -lcrypto -lresolv -lrt -lpugixml -lnsl -laio
LIBRARY		:= /u01/app/oracle/product/11.2.0/xe/lib/
OBJECTS		:= ./lib/librsaqrgen.a
OPTS		:= -Wall -Wreorder -Wpedantic
CONFIG		:= $(wildcard ./*.conf)

# Vegrehajtas:
all:
	g++-4.8 -std=c++11 ${OPTS} ${FILES} -o splitter -I${ORACLE_INCLUDE} -I${LOCAL_INCLUDE} ${OBJECTS} -L${LIBRARY} ${LINKS}
