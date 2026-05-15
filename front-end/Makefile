CC = g++
CFLAGS = -g -I include -I ../commons/include -I $(OBJECTS_DIR)/gen \
         -Wcast-qual -Wconversion -Wctor-dtor-privacy \
         -Wempty-body -Wformat-security -Wformat=2 -Wignored-qualifiers -Wlogical-op \
         -Wno-missing-field-initializers -Wnon-virtual-dtor -Woverloaded-virtual \
         -Wpointer-arith -Wsign-promo -Wstack-usage=8192 -Wstrict-aliasing \
         -Wstrict-null-sentinel -Wtype-limits -Wwrite-strings -fexceptions -pipe

LDFLAGS =

LEX = flex
YACC = bison
LEX_FLAGS = -o
YACC_FLAGS = -d -o

COMMONS_OBJ      = ../commons/bin
SOURCES_DIR      = src
OBJECTS_DIR      = bin
GENERATED_DIR    = $(OBJECTS_DIR)/gen
BUILD_DIR        = build
LOGS_DIR         = logs
DOT_FILES_DIR    = $(LOGS_DIR)/dot_files
IMAGES_DIR       = $(LOGS_DIR)/images

EXECUTABLE       = frontend
EXECUTABLE_PATH  = $(BUILD_DIR)/$(EXECUTABLE)

LEXER_SRC        = $(SOURCES_DIR)/lexer.l
PARSER_SRC       = $(SOURCES_DIR)/parser.y
LEXER_OUT        = $(GENERATED_DIR)/lexer.cpp
PARSER_OUT       = $(GENERATED_DIR)/parser.cpp
PARSER_HDR       = $(GENERATED_DIR)/parser.hpp

SOURCE_FILES     = $(wildcard $(SOURCES_DIR)/*.cpp)
GENERATED_FILES  = $(LEXER_OUT) $(PARSER_OUT)
OBJECT_FILES     = $(patsubst $(SOURCES_DIR)/%.cpp, $(OBJECTS_DIR)/%.o, $(SOURCE_FILES)) \
                   $(patsubst $(GENERATED_DIR)/%.cpp, $(OBJECTS_DIR)/%.o, $(GENERATED_FILES))

COMMONS_OBJ_FILES = $(wildcard $(COMMONS_OBJ)/*.o)

all: $(GENERATED_FILES) $(EXECUTABLE_PATH)

$(LEXER_OUT): $(LEXER_SRC) | $(GENERATED_DIR)
	$(LEX) $(LEX_FLAGS) $@ $<

$(PARSER_OUT) $(PARSER_HDR): $(PARSER_SRC) | $(GENERATED_DIR)
	$(YACC) $(YACC_FLAGS) $@ $<

$(GENERATED_DIR):
	mkdir -p $(GENERATED_DIR)

$(BUILD_DIR):
	mkdir -p $(BUILD_DIR)

$(OBJECTS_DIR):
	mkdir -p $(OBJECTS_DIR)
	mkdir -p $(LOGS_DIR)
	mkdir -p $(DOT_FILES_DIR)
	mkdir -p $(IMAGES_DIR)

$(OBJECTS_DIR)/%.o: $(SOURCES_DIR)/%.cpp $(OBJECTS_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(OBJECTS_DIR)/%.o: $(GENERATED_DIR)/%.cpp $(OBJECTS_DIR)
	$(CC) -c $(CFLAGS) $< -o $@

$(EXECUTABLE_PATH): $(OBJECT_FILES) $(BUILD_DIR)
	$(CC) $(COMMONS_OBJ_FILES) $(OBJECT_FILES) -o $@ $(LDFLAGS)

clean_logs:
	rm -f $(DOT_FILES_DIR)/*.dot $(IMAGES_DIR)/*.png

clean:
	rm -fr $(OBJECTS_DIR) $(BUILD_DIR) $(LOGS_DIR)

.PHONY: all clean clean_logs