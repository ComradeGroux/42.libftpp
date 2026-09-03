BOLD  := \033[1m
GRAY  := \033[90m
GREEN := \033[32m
BLUE  := \033[34m
RESET := \033[0m
ERASE := \r\033[2K

TARGET := libftpp.a

BUILD_DIR	:= build
OBJS_DIR	:= $(BUILD_DIR)/objs

SRCS	:= $(wildcard $(SRC_DIR)/*.cpp)
OBJS	:= $(addprefix $(OBJS_DIR)/, $(notdir $(SRCS:.cpp=.o)))

CXX 		:= g++
CXXFLAGS	:= -Wall -Wextra -Werror -std=c++20

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
$(OBJS_DIR)/%.o: %.cpp | $(OBJS_DIR)
	@printf "$(GRAY)  $<...$(RESET)" && \
	 $(CXX) $(CXXFLAGS) -c $< -o $@ && \
	 printf "$(ERASE)$(GREEN)  ✓ $<$(RESET)\n"\

clean:
	@printf "$(GRAY)  Removing build objects...$(RESET)" && \
	 rm -rf $(OBJS_DIR) && \
	 printf "$(ERASE)"
	@printf "$(GREEN)  ✓ Build files cleaned$(RESET)\n"

fclean: clean
	@printf "$(GRAY)  Removing $(BUILD_DIR) and $(TARGET)...$(RESET)" && \
	 rm -rf $(BUILD_DIR) $(TARGET) && \
	 printf "$(ERASE)"
	@printf "$(GREEN)  ✓ $(TARGET) cleaned$(RESET)\n"

re: fclean all

.PHONY: all clean fclean re
