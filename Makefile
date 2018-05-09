NAME = shaderPixel
CC = clang++

LIB_GLFW_NAME = .glfw-3.2.1
LIB_GLAD_NAME = .glad
LIB_GLM_NAME = .glm

SRC_PATH = ./src/
OBJ_PATH = ./obj/
LIB_PATH = $(HOME)/
INC_PATH = ./include/ $(LIB_PATH)$(LIB_GLFW_NAME)/include/ $(LIB_PATH)$(LIB_GLAD_NAME)/include/ $(LIB_PATH)$(LIB_GLM_NAME)/

CC_FLGS = -std=c++11 #-Werror -Wextra -Wall
CC_LIBS = -lglfw3 -framework AppKit -framework OpenGL -framework IOKit -framework CoreVideo

SRC_NAME = main.cpp Camera.cpp Controller.cpp Env.cpp Model.cpp Renderer.cpp Shader.cpp utils.cpp
OBJ_NAME = $(SRC_NAME:.cpp=.o)

LIB_GLFW_SRC = $(LIB_GLFW_NAME)/src
LIB_GLAD_SRC = $(LIB_GLAD_NAME)/src/glad.c

SRC = $(addprefix $(SRC_PATH), $(SRC_NAME))
OBJ = $(addprefix $(OBJ_PATH), $(OBJ_NAME))
INC = $(addprefix -I,$(INC_PATH))
LIB_GLFW = $(addprefix -L$(LIB_PATH),$(LIB_GLFW_SRC))
LIB_GLAD = $(addprefix $(LIB_PATH),$(LIB_GLAD_SRC))

all: $(NAME)

$(NAME): $(OBJ)
	$(CC) $(CC_FLGS) $(LIB_GLFW) $(LIB_GLAD) $(INC) $(OBJ) $(CC_LIBS) -o $(NAME)

$(OBJ_PATH)%.o: $(SRC_PATH)%.cpp
	mkdir -p $(OBJ_PATH)
	$(CC) $(CC_FLGS) $(INC) -o $@ -c $<

clean:
	rm -fv $(OBJ)
	rm -rf $(OBJ_PATH)

fclean: clean
	rm -fv $(NAME)

re: fclean all
