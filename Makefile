SRC_DIR = src
OBJ_DIR = cashe
RES_DIR = results

CC      = gcc
LDLIBS  = -lm

STATIONS ?= 300
SLOT_MS  ?= 10
SLOTS    ?= 10000
G_COUNT  ?= 17

CFLAGS += -DNUM_STATIONS=$(STATIONS)
CFLAGS += -DSLOT_SIZE_MS=$(SLOT_MS)
CFLAGS += -DSIMULATION_SLOTS=$(SLOTS)
CFLAGS += -DNUM_G_VALUES=$(G_COUNT)

TARGET = $(OBJ_DIR)/channel_sim

.PHONY: all clean run results

all:
	$(MAKE) clean
	$(MAKE) run

$(TARGET): $(SRC_DIR)/channel_sim.c
	@mkdir -p $(OBJ_DIR)
	$(CC) $< -o $@ $(LDLIBS) $(CFLAGS)

clean:
	rm -rf $(OBJ_DIR) $(RES_DIR)
	rm -f channel_sim_results.dat protocol_throughput_plot.png

run: $(TARGET)
	@mkdir -p $(RES_DIR)
	@echo ">>> Starting ALOHA & CSMA Simulation..."
	@time -f ">>> Total Execution Time: %E" ./$(TARGET)
	@echo ">>> Generating Plots..."
	@python3 ./src/plot.py
	@# Organize output files into the results directory
	@mv -f channel_sim_results.dat $(RES_DIR)/
	@mv -f protocol_throughput_plot.png $(RES_DIR)/
	@echo ">>> Simulation finished. Check the '$(RES_DIR)' folder."
	@$(MAKE) results

results:
	@echo ">>> Generated files in '$(RES_DIR)':"
	@ls -lh $(RES_DIR)

help:
	@echo "==============================================================="
	@echo "Wireless Holiday Makefile Help"
	@echo "================================================================="
	@echo " make help : Prints this help."
	@echo " make : Cleans the previous build and runs a full build."
	@echo " make run : Trims C code and performs chair and Python plotting."
	@echo " make clean : Deletes transmitted binaries, logs, and file files."
	@echo " Create results: Check the list of generated result files."
	@echo "================================================================"