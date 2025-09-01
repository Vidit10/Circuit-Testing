#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>

#include "verilog_parser.h"
#include "gate_logic.h"
#include "circuit_node.h"

// Function to build circuit from parsed data
Circuit* build_circuit_from_parsed_data(void) {
    Circuit* circuit = create_circuit();
    if (!circuit) {
        fprintf(stderr, "Error: Failed to create circuit\n");
        return NULL;
    }
    
    printf("Building circuit from parsed data...\n");
    
    // Step 1: Add all primary input nodes
    for (int i = 0; i < input_count; i++) {
        int node_id = add_node(circuit, input_signals[i].name, NODE_PI);
        if (node_id == -1) {
            fprintf(stderr, "Error: Failed to add primary input %s\n", input_signals[i].name);
        } else {
            printf("Added PI: %s (ID: %d)\n", input_signals[i].name, node_id);
        }
    }
    
    // Step 2: Add all primary output nodes
    for (int i = 0; i < output_count; i++) {
        int node_id = add_node(circuit, output_signals[i].name, NODE_PO);
        if (node_id == -1) {
            fprintf(stderr, "Error: Failed to add primary output %s\n", output_signals[i].name);
        } else {
            printf("Added PO: %s (ID: %d)\n", output_signals[i].name, node_id);
        }
    }
    
    // Step 3: Add wire nodes and gate nodes from parsed gates
    for (int i = 0; i < parsed_gate_count; i++) {
        GateInstance* gate = &parsed_gates[i];
        
        // Add output node as gate node
        int output_node_id = add_node(circuit, gate->output_signal, NODE_GATE);
        if (output_node_id == -1) {
            fprintf(stderr, "Error: Failed to add gate output node %s\n", gate->output_signal);
            continue;
        }
        
        // Set gate properties
        circuit->nodes[output_node_id].gate_type = gate->type;
        strncpy(circuit->nodes[output_node_id].gate_instance, gate->instance_name, 
                sizeof(circuit->nodes[output_node_id].gate_instance) - 1);
        
        printf("Added Gate: %s -> %s (ID: %d, Type: %s)\n", 
               gate->instance_name, gate->output_signal, output_node_id,
               gate_type_to_string(gate->type));
        
        // Add input nodes if they don't exist
        for (int j = 0; j < gate->input_signal_count; j++) {
            int input_node_id = add_node(circuit, gate->input_signals[j].name, NODE_GATE);
            if (input_node_id == -1) {
                fprintf(stderr, "Error: Failed to add input node %s\n", gate->input_signals[j].name);
                continue;
            }
            
            // Add connection from input to gate output
            if (!add_connection(circuit, input_node_id, output_node_id)) {
                fprintf(stderr, "Error: Failed to connect %s -> %s\n", 
                        gate->input_signals[j].name, gate->output_signal);
            } else {
                printf("  Connected: %s (ID:%d) -> %s (ID:%d)\n", 
                       gate->input_signals[j].name, input_node_id,
                       gate->output_signal, output_node_id);
            }
        }
    }
    
    // Step 4: Add branch nodes for fanout points
    printf("\nAdding branch nodes for fanout points...\n");
    add_branch_nodes(circuit);
    
    printf("Circuit construction completed.\n\n");
    return circuit;
}

// Function to get user input for primary inputs
void get_user_inputs(Circuit* circuit, SignalValue* input_values) {
    printf("## Enter Primary Input Values (0 or 1):\n");
    for (int i = 0; i < circuit->pi_count; i++) {
        int node_id = circuit->primary_inputs[i];
        CircuitNode* node = &circuit->nodes[node_id];
        
        char input_buffer[10];
        int val = -1;
        
        while (val != 0 && val != 1) {
            printf("  %s: ", node->name);
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
        input_values[i] = (val == 1) ? LOGIC_1 : LOGIC_0;
    }
    printf("\n");
}

int main(int argc, char *argv[]) {
    if (argc < 2) {
        fprintf(stderr, "Usage: %s <verilog_file>\n", argv[0]);
        return 1;
    }

    const char *filename = argv[1];
    
    printf("=== ISCAS Circuit Simulator ===\n");
    printf("Parsing Verilog file: %s\n\n", filename);

    // 1. Parse the Verilog file
    if (parse_verilog_module(filename) != 0) {
        fprintf(stderr, "Error: Failed to parse Verilog file\n");
        return 1;
    }

    printf("## Parsed Verilog Structure\n");
    printf("Module: %s\n", module_name);
    printf("Inputs: %d, Outputs: %d, Wires: %d, Gates: %d\n\n",
           input_count, output_count, wire_count, parsed_gate_count);

    // 2. Build circuit from parsed data
    Circuit* circuit = build_circuit_from_parsed_data();
    if (!circuit) {
        fprintf(stderr, "Error: Failed to build circuit\n");
        return 1;
    }

    // 3. Display circuit information
    print_circuit_info(circuit);
    
    // 4. Display circuit connections (optional, comment out for large circuits)
    if (circuit->node_count <= 20) {
        print_connections(circuit);
    }

    // 5. Interactive simulation
    SignalValue input_values[MAX_SIGNALS];
    get_user_inputs(circuit, input_values);
    
    // Set inputs and simulate
    set_primary_inputs(circuit, input_values);
    
    printf("## Simulating Circuit\n");
    if (simulate_circuit(circuit)) {
        printf("Circuit simulation completed successfully.\n");
        printf("Converged in %d iterations.\n\n", circuit->iteration_count);
    } else {
        printf("Warning: Circuit did not stabilize within maximum iterations.\n\n");
    }
    
    // 6. Display results
    print_node_values(circuit);
    
    printf("## Primary Output Values:\n");
    for (int i = 0; i < circuit->po_count; i++) {
        int node_id = circuit->primary_outputs[i];
        CircuitNode* node = &circuit->nodes[node_id];
        printf("  %s: %c\n", node->name, signal_value_to_char(node->value));
    }

    // Cleanup
    destroy_circuit(circuit);
    return 0;
}