CXX=g++
CFLAGS=-c -Wall -std=c++20
LDFLAGS=
SOURCES_DIR=src
SOURCES=$(wildcard src/*.cpp)
OBJECTS_DIR=build
OBJECTS=$(patsubst $(SOURCES_DIR)/%.cpp,$(OBJECTS_DIR)/%.o,$(SOURCES))
OUTPUT_DIR=build
EXECUTABLE=$(OUTPUT_DIR)/gaijin-test-client

all: $(SOURCES) $(EXECUTABLE)

$(EXECUTABLE): $(OBJECTS)
	mkdir -p $(OUTPUT_DIR)
	$(CXX) $(LDFLAGS) $^ -o $@

$(OBJECTS_DIR)/%.o: $(SOURCES_DIR)/%.cpp
	mkdir -p $(OBJECTS_DIR)
	$(CXX) $(CFLAGS) $< -o $@

clean:
	rm -rf $(OBJECTS_DIR)/*.o $(EXECUTABLE)
