BOLD  := \033[1m
GRAY  := \033[90m
GREEN := \033[32m
BLUE  := \033[34m
RESET := \033[0m
ERASE := \r\033[2K

TARGET := libftpp.a

BUILD_DIR	:= build
OBJS_DIR	:= $(BUILD_DIR)/objs
SRCS		:=	data_structures/data_buffer/data_buffer.cpp \
				design_pattern/memento/memento.cpp \
				io_stream/thread_safe_iostream.cpp
OBJS		:= $(addprefix $(OBJS_DIR)/, $(notdir $(SRCS:.cpp=.o)))

TEST_DIR		:= tests
OBJS_TEST_DIR	:= $(BUILD_DIR)/test_objs
TEST_SRCS		:= $(wildcard $(TEST_DIR)/src/*.cpp)
TEST_OBJS		:= $(addprefix $(OBJS_TEST_DIR)/, $(notdir $(TEST_SRCS:.cpp=.o)))
TEST_BINS		:= $(addprefix $(TEST_DIR)/, $(notdir $(TEST_SRCS:.cpp=)))

CXX 		:= g++
CXXFLAGS	:= -Wall -Wextra -Werror -std=c++17 -I.

all: $(TARGET)

$(TARGET): $(OBJS)
	@printf "$(BOLD)Linking $(TARGET)$(RESET)\n"
	@ar rcs $(TARGET) $(OBJS)
	@printf "$(GREEN)  ✓ $(TARGET) ready$(RESET)\n"

$(OBJS_DIR):
	@mkdir -p $@
$(OBJS): | $(OBJS_DIR)/.compile_start
$(OBJS_DIR)/.compile_start: $(SRCS)
	@printf "$(BOLD)Compiling$(RESET)\n"
	@touch $@
vpath %.cpp $(sort $(dir $(SRCS)))
$(OBJS_DIR)/%.o: %.cpp | $(OBJS_DIR)
	@printf "$(GRAY)  $<...$(RESET)" && \
	 $(CXX) $(CXXFLAGS) -c $< -o $@ && \
	 printf "$(ERASE)$(GREEN)  ✓ $<$(RESET)\n"\




test: all $(TEST_BINS)
	@printf "$(GREEN)  ✓ All tests built$(RESET)\n"

$(OBJS_TEST_DIR):
	@mkdir -p $@
$(TEST_OBJS): | $(OBJS_TEST_DIR)/.compile_start
$(OBJS_TEST_DIR)/.compile_start: $(TEST_SRCS)
	@printf "$(BOLD)Compiling tests$(RESET)\n"
	@touch $@
$(OBJS_TEST_DIR)/%.o: $(TEST_DIR)/src/%.cpp | $(OBJS_TEST_DIR)
	@$(CXX) $(CXXFLAGS) -c $< -o $@

$(TEST_DIR)/%: $(OBJS_TEST_DIR)/%.o $(TARGET)
	@printf "$(GRAY)  Linking $@...$(RESET)" && \
	 $(CXX) $(CXXFLAGS) $< $(TARGET) -o $@ && \
	 printf "$(ERASE)$(GREEN)  ✓ $@$(RESET)\n"



clean:
	@printf "$(GRAY)  Removing build objects...$(RESET)" && \
	 rm -rf $(OBJS_DIR) && \
	 printf "$(ERASE)"
	@printf "$(GREEN)  ✓ Build files cleaned$(RESET)\n"
	@printf "$(GRAY)  Removing test build objects...$(RESET)" && \
	 rm -rf $(OBJS_TEST_DIR) && \
	 printf "$(ERASE)"
	@printf "$(GREEN)  ✓ Test build files cleaned$(RESET)\n"

fclean: clean
	@printf "$(GRAY)  Removing $(BUILD_DIR), $(TARGET) and test binaries...$(RESET)" && \
	 rm -rf $(BUILD_DIR) $(TARGET) $(TEST_BINS) && \
	 printf "$(ERASE)"
	@printf "$(GREEN)  ✓ $(TARGET) cleaned$(RESET)\n"

re: fclean all

.PHONY: all clean fclean re test
