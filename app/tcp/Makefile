#预定义变量
CC = gcc
#预定义编译目录
SUBDIRS = tcp_socket \
		tcp_server \
        obj
#预定义目标
OBJS = tcp_socket.o tcp_server.o
BIN = tcp_server
OBJS_DIR = obj
BIN_DIR = bin
#传递预定义参数
export CC OBJS BIN OBJS_DIR BIN_DIR

all:CHECK_DIR $(SUBDIRS)
CHECK_DIR:
	@mkdir -p $(BIN_DIR)
$(SUBDIRS):ECHO
	@make -C $@ 

ECHO:
	@echo $(SUBDIRS)
	@echo begin compile
clean:
	@$(RM) $(OBJS_DIR)/*.o
	@rm -rf $(BIN_DIR)