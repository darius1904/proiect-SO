CC = gcc
CFLAGS = -Wall

# Executables
EXEC_MAIN = treasure_hunter
EXEC_HUB = treasure_hub
EXEC_MONITOR = treasure_monitor
EXEC_CALCULATOR = calculator

# Sources and Objects
SRC_MAIN = src/treasure_manager.c src/treasure.c
OBJ_MAIN = build/treasure_manager.o build/treasure.o

SRC_HUB = src/hub/shell.c
OBJ_HUB = build/shell.o

SRC_MONITOR = src/hub/monitor.c
OBJ_MONITOR = build/monitor.o

SRC_CALCULATOR = src/calculator/calculator.c
OBJ_CALCULATOR = build/calculator.o

# All targets
all: $(EXEC_MAIN) $(EXEC_HUB) $(EXEC_MONITOR) $(EXEC_CALCULATOR)

# Rules for each executable
$(EXEC_MAIN): $(OBJ_MAIN)
	$(CC) $(OBJ_MAIN) -o $@

$(EXEC_HUB): $(OBJ_HUB)
	$(CC) $(OBJ_HUB) -o $@

$(EXEC_MONITOR): $(OBJ_MONITOR)
	$(CC) $(OBJ_MONITOR) -o $@

$(EXEC_CALCULATOR): $(OBJ_CALCULATOR)
	$(CC) $(OBJ_CALCULATOR) -o $@

# Generic rules for compiling .c into build/*.o, including subdirectories
build/%.o: src/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: src/hub/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

build/%.o: src/calculator/%.c | build
	$(CC) $(CFLAGS) -c $< -o $@

# Ensure build directory exists
build:
	mkdir -p build

# Clean
clean:
	rm -f $(OBJ_MAIN) $(OBJ_HUB) $(OBJ_MONITOR) $(OBJ_CALCULATOR) \
	      $(EXEC_MAIN) $(EXEC_HUB) $(EXEC_MONITOR) $(EXEC_CALCULATOR)
