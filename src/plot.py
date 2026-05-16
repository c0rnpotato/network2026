# Wireless Network HW4(2026-1)
# 2023066980 Youngjin Kim

import pandas as pd
import matplotlib.pyplot as plt
import numpy as np
import sys

DATA_FILE = "channel_sim_results.dat"

try:
    # Read the data file containing ALOHA and CSMA results
    data = pd.read_csv(DATA_FILE, sep=" ") 
except FileNotFoundError:
    print(f"Error: '{DATA_FILE}' not found.")
    print("Please run the C simulation program first (./channel_sim)")
    sys.exit(1)

# Generate points for theoretical curves
g_theory = np.linspace(0, max(data['G']), 200)

# 1. ALOHA Theory
s_slotted_theory = g_theory * np.exp(-g_theory)
s_pure_theory = g_theory * np.exp(-2 * g_theory)

# 2. CSMA Theory (a = 0.01: 1 tic propagation delay / 100 tics per slot)
a = 0.01
# Non-persistent CSMA Formula
s_non_theory = (g_theory * np.exp(-a * g_theory)) / (g_theory * (1 + 2 * a) + np.exp(-a * g_theory))
# 1-persistent CSMA Formula
num_1 = g_theory * (1 + g_theory + a * g_theory * (1 + g_theory + (a * g_theory) / 2)) * np.exp(-g_theory * (1 + 2 * a))
den_1 = g_theory * (1 + 2 * a) - (1 - np.exp(-a * g_theory)) + (1 + a * g_theory) * np.exp(-g_theory * (1 + a))
s_1_theory = num_1 / den_1


plt.figure(figsize=(14, 8))

# Plot ALOHA Theory and Simulation
plt.plot(g_theory, s_slotted_theory, 'b-', label="Slotted ALOHA (Theory)")
plt.plot(g_theory, s_pure_theory, 'r--', label="Pure ALOHA (Theory)")
plt.scatter(data['G'], data['S_slotted'], c='blue', marker='o', s=40, label="Slotted ALOHA (Sim)")
plt.scatter(data['G'], data['S_pure'], c='red', marker='x', s=40, label="Pure ALOHA (Sim)")

# Plot CSMA Theory (Dotted lines)
plt.plot(g_theory, s_non_theory, color='darkgreen', linestyle=':', linewidth=2, label="Non-persistent CSMA (Theory)")
plt.plot(g_theory, s_1_theory, color='cyan', linestyle=':', linewidth=2, label="1-persistent CSMA (Theory)")

# Plot CSMA Simulations (Lines with markers)
plt.plot(data['G'], data['S_non'], color='darkgreen', marker='s', linestyle='-', alpha=0.7, label="Non-persistent CSMA (Sim)")
plt.plot(data['G'], data['S_01'], color='darkorange', marker='^', linestyle='-', alpha=0.7, label="0.1-persistent CSMA (Sim)")
plt.plot(data['G'], data['S_05'], color='magenta', marker='v', linestyle='-', alpha=0.7, label="0.5-persistent CSMA (Sim)")
plt.plot(data['G'], data['S_1'], color='cyan', marker='d', linestyle='-', alpha=0.7, label="1-persistent CSMA (Sim)")

# Plot configurations
plt.title("Throughput Simulation vs. Theory: ALOHA and CSMA Protocols", fontsize=16)
plt.xlabel("G (Traffic Load)", fontsize=12)
plt.ylabel("S (Throughput)", fontsize=12)
plt.legend(fontsize=10, loc='upper right', bbox_to_anchor=(1.25, 1))
plt.grid(True, linestyle=':', alpha=0.7)

# Adjust limits since CSMA throughput goes up to 1.0
plt.ylim(bottom=0, top=1.1)
plt.xlim(left=0)

plt.tight_layout()
plt.savefig("protocol_throughput_plot.png")
plt.show()