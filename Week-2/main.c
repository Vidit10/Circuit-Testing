#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "verilog_parser.h"
#include "gate_logic.h"

// [Keep all the existing defines and global structures - SimSignal, all_signals, etc.]
#define MAX_TOTAL_UNIQUE_SIGNALS (MAX_PORTS + MAX_SIGNALS * 3 + MAX_GATE_INSTANCES * (MAX_GATE_INPUTS + 1))

typedef struct {
    char name[MAX_NAME_LENGTH];
    SignalValue value;
    bool is_primary_input;
    bool is_primary_output;
} SimSignal;

SimSignal all_signals[MAX_TOTAL_UNIQUE_SIGNALS];
int total_unique_signal_count = 0;

// [Keep the find_or_add_sim_signal function unchanged]
int find_or_add_sim_signal(const char* signal_name) {
    for (int i = 0; i < total_unique_signal_count; i++) {
        if (strcmp(all_signals[i].name, signal_name) == 0) {
            return i;
        }
    }
    if (total_unique_signal_count < MAX_TOTAL_UNIQUE_SIGNALS) {
        strncpy(all_signals[total_unique_signal_count].name, signal_name, MAX_NAME_LENGTH - 1);
        all_signals[total_unique_signal_count].name[MAX_NAME_LENGTH - 1] = '\0';
        all_signals[total_unique_signal_count].value = LOGIC_X;
        all_signals[total_unique_signal_count].is_primary_input = false;
        all_signals[total_unique_signal_count].is_primary_output = false;
        total_unique_signal_count++;
        return total_unique_signal_count - 1;
    } else {
        fprintf(stderr, "Error: Exceeded MAX_TOTAL_UNIQUE_SIGNALS. Increase the define.\n");
        exit(EXIT_FAILURE);
    }
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <verilog_file>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    printf("Attempting to parse Verilog file: %s\n\n", filename);

    // 1. Parse the Verilog file
    if (parse_verilog_module(filename) != 0) {
        fprintf(stderr, "An error occurred during Verilog parsing.\n");
        return 1;
    }

    // Display Parsed Header Info
    printf("## Parsed Verilog Structure (Summary)\n");
    printf("Module: %s\n", module_name);
    printf("Inputs: %d, Outputs: %d, Wires: %d, Gates: %d\n\n",
           input_count, output_count, wire_count, parsed_gate_count);

    // 2. Populate 'all_signals' with unique signal names and mark primary I/O
    total_unique_signal_count = 0;

    // Add primary inputs
    for (int i = 0; i < input_count; i++) {
        int idx = find_or_add_sim_signal(input_signals[i].name);
        all_signals[idx].is_primary_input = true;
    }
    // Add primary outputs
    for (int i = 0; i < output_count; i++) {
        int idx = find_or_add_sim_signal(output_signals[i].name);
        all_signals[idx].is_primary_output = true;
    }
    // Add wires
    for (int i = 0; i < wire_count; i++) {
        find_or_add_sim_signal(wire_signals[i].name);
    }
    // Add signals from gate ports
    for (int i = 0; i < parsed_gate_count; i++) {
        find_or_add_sim_signal(parsed_gates[i].output_signal);
        for (int j = 0; j < parsed_gates[i].input_signal_count; j++) {
            find_or_add_sim_signal(parsed_gates[i].input_signals[j].name);
        }
    }
    printf("Total unique signals identified: %d\n\n", total_unique_signal_count);

    // 3. Get values for primary inputs from the user
    printf("## Enter Primary Input Values (0 or 1):\n");
    for (int i = 0; i < total_unique_signal_count; i++) {
        if (all_signals[i].is_primary_input) {
            char input_buffer[10];
            int val = -1;
            while (val != 0 && val != 1) {
                printf("  %s: ", all_signals[i].name);
                if (fgets(input_buffer, sizeof(input_buffer), stdin) != NULL) {
                    input_buffer[strcspn(input_buffer, "\n")] = 0;
                    if (strcmp(input_buffer, "0") == 0) {
                        val = 0;
                    } else if (strcmp(input_buffer, "1") == 0) {
                        val = 1;
                    } else {
                        printf("    Invalid input. Please enter 0 or 1.\n");
                    }
                } else {
                    fprintf(stderr, "Error reading input.\n");
                    exit(EXIT_FAILURE);
                }
            }
            all_signals[i].value = (val == 1) ? LOGIC_1 : LOGIC_0;
        }
    }
    printf("\n");

    // 4. Simulation Loop
    printf("## Simulating...\n");
    int max_iterations = total_unique_signal_count * 2;
    if (max_iterations < 50) max_iterations = 50;
    if (parsed_gate_count == 0 && input_count > 0 && output_count > 0) {
        for(int i=0; i < total_unique_signal_count; ++i){
            if(all_signals[i].is_primary_output){
                for(int j=0; j < total_unique_signal_count; ++j){
                    if(all_signals[j].is_primary_input && strcmp(all_signals[i].name, all_signals[j].name) == 0){
                        all_signals[i].value = all_signals[j].value;
                        break;
                    }
                }
            }
        }
         printf("No gates to simulate. Outputs reflect direct connections if any.\n");
    }

    for (int iter = 0; iter < max_iterations; iter++) {
        bool changed_in_iteration = false;
        if (parsed_gate_count == 0) break;

        for (int i = 0; i < parsed_gate_count; i++) {
            GateInstance *gate = &parsed_gates[i];
            SignalValue gate_inputs[MAX_GATE_INPUTS];
            int gate_input_count = gate->input_signal_count;

            // Gather current values for this gate's inputs
            for (int j = 0; j < gate_input_count; j++) {
                int input_signal_idx = find_or_add_sim_signal(gate->input_signals[j].name);
                gate_inputs[j] = all_signals[input_signal_idx].value;
            }

            SignalValue new_output_value = LOGIC_X;
            switch (gate->type) {
                case GATE_AND:  new_output_value = evaluate_and(gate_inputs, gate_input_count); break;
                case GATE_NAND: new_output_value = evaluate_nand(gate_inputs, gate_input_count); break;
                case GATE_OR:   new_output_value = evaluate_or(gate_inputs, gate_input_count); break;
                case GATE_NOR:  new_output_value = evaluate_nor(gate_inputs, gate_input_count); break;
                case GATE_XOR:
                    if (gate_input_count == 2) new_output_value = evaluate_xor2(gate_inputs[0], gate_inputs[1]);
                    else new_output_value = LOGIC_X;
                    break;
                case GATE_XNOR:
                    if (gate_input_count == 2) new_output_value = evaluate_xnor2(gate_inputs[0], gate_inputs[1]);
                    else new_output_value = LOGIC_X;
                    break;
                case GATE_NOT:
                    if (gate_input_count == 1) new_output_value = evaluate_not1(gate_inputs[0]);
                    else new_output_value = LOGIC_X;
                    break;
                case GATE_BUFF:
                    if (gate_input_count == 1) new_output_value = evaluate_buff1(gate_inputs[0]);
                    else new_output_value = LOGIC_X;
                    break;
                case GATE_UNKNOWN:
                default:
                    new_output_value = LOGIC_X;
                    break;
            }

            int output_signal_idx = find_or_add_sim_signal(gate->output_signal);
            if (all_signals[output_signal_idx].value != new_output_value) {
                all_signals[output_signal_idx].value = new_output_value;
                changed_in_iteration = true;
            }
        }

        if (!changed_in_iteration) {
            printf("Circuit stabilized in %d iterations.\n", iter + 1);
            break;
        }
        if (iter == max_iterations - 1) {
            printf("Warning: Circuit did not stabilize within %d iterations. Results might be unstable or from an oscillating circuit.\n", max_iterations);
        }
    }
    printf("\n");

    // *** ADD THIS NEW SECTION: Print all intermediate node values ***
    printf("## All Node Values (Intermediate & Final):\n");
    printf("%-15s | %-6s\n", "Node Name", "Value");
    printf("----------------+-------\n");
    for (int i = 0; i < total_unique_signal_count; i++) {
        printf("%-15s | %-6c\n", all_signals[i].name, signal_value_to_char(all_signals[i].value));
    }
    printf("\n");

    // 5. Display values of primary outputs
    printf("## Primary Output Values:\n");
    bool found_any_output = false;
    for (int i = 0; i < total_unique_signal_count; i++) {
        if (all_signals[i].is_primary_output) {
            printf("  %s: %c\n", all_signals[i].name, signal_value_to_char(all_signals[i].value));
            found_any_output = true;
        }
    }
    if (!found_any_output) {
        printf("  No primary outputs defined or found in the module.\n");
    }

    return 0;
}
